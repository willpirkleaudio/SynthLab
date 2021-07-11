// --- Synth Core v1.0
//
#include "synthengine.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthengine.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief
	Construction: 
	- initializes global MIDI data structure with values to prevent accidental silence on first note-on
	- constructs the shared wavetable databse
	- constructs the shared PCM sample database
	- constructs the array of MAX_VOICES synth voice objects
	- constructs the (un-shared) audio delay object

	\param blockSize the block size to be used for the lifetime of operation; OK if arriving blocks are smaller than this value, NOT OK if larger

	\returns the newly constructed object
	*/
	SynthEngine::SynthEngine(uint32_t blockSize)
	{
		// --- initialize to non-zero values for volume and pan
		initMIDIInputData(midiInputData);

		// --- these databases are empty at this point
		//     will be filled by modules or cores
		//     automatically rejects duplicate entries so any module or core can 
		//     load the database at startup and all will have access to it
		if (!wavetableDatabase)
			wavetableDatabase = std::make_shared<WavetableDatabase>();

		if (!sampleDatabase)
			sampleDatabase = std::make_shared<PCMSampleDatabase>();

		// --- create the smart pointers
		for (unsigned int i = 0; i < MAX_VOICES; i++)
		{
			// --- reset is the constructor for this kind of smartpointer
			//
			//     Pass our this pointer for the IMIDIData interface - safe
			synthVoices[i].reset(new SynthVoice(midiInputData, midiOutputData, parameters->voiceParameters, wavetableDatabase, sampleDatabase, blockSize));
		}

		// --- voice object
		voiceProcessInfo.init(0, 2, blockSize);
	}

	/**
	\brief
	Destruction:
	- the PCM sample database is dynamic and is populated with arrays extracted from WAV files
	at startup time; the benefit is instant access to the samples
	- this deletes these arrays that are shared safely across voices vie the IPCMSampleDatabase 
	objects

	*/
	SynthEngine::~SynthEngine()
	{
		if (sampleDatabase)
			sampleDatabase->clearSampleSources();
	}

	/**
	\brief
	Resets all voices and the audio delay object

	\param _sampleRate the initial or newly changed sample rate

	\return true if sucessful
	*/
	bool SynthEngine::reset(double _sampleRate)
	{
		// --- reset array of voices
		for (unsigned int i = 0; i < MAX_VOICES; i++)
		{
			// --- smart poitner access looks normal (->)
			synthVoices[i]->reset(_sampleRate); // this calls reset() on the smart-pointers underlying naked pointer
		}

		return true;
	}

	/**
	\brief
	Initializes all voices with the DLL path
	- used for PCM sample oscillator cores that need to find WAV files with samples

	\param dllPath path to the folder that contains the plugin DLL

	\return true if sucessful
	*/
	bool SynthEngine::initialize(const char* dllPath)
	{
		// --- loop
		for (unsigned int i = 0; i < MAX_VOICES; i++)
		{
			// --- init
			synthVoices[i]->initialize(dllPath);
		}

		return true;
	}

	/**
	\brief
	Render a buffer of output audio samples
	- process all MIDI events at top of block
	- then renders the active voices one at a time
	- accumulates voices and applies delay FX
	- applies global gain control to final audio output stream

	\param synthProcessInfo structure containing all information needed 
	about the current block to process including:
	- MIDI messages for the block
	- pointers to the output audio buffers
	- optional pointers to input audio buffers (not used in SynthLab)

	\return true if sucessful
	*/
	bool SynthEngine::render(SynthProcessInfo& synthProcessInfo)
	{
		// --- this may or may not be done on the framwork
		synthProcessInfo.flushBuffers();

		// --- issue MIDI events for this block
		uint32_t midiEvents = synthProcessInfo.getMidiEventCount();
		for (uint32_t i = 0; i < midiEvents; i++)
		{
			// --- get the event
			midiEvent event = *synthProcessInfo.getMidiEvent(i);

			// --- process it
			processMIDIEvent(event);
		}

		// --- -12dB per active channel to avoid clipping
		double gainFactor = 1.0;
		if (parameters->synthModeIndex == enumToInt(SynthMode::kPoly))
			gainFactor = 0.5;
		else if (parameters->synthModeIndex == enumToInt(SynthMode::kUnison) ||
			     parameters->synthModeIndex == enumToInt(SynthMode::kUnisonLegato))
			gainFactor = 0.5;

		// --- this is important
		uint32_t samplesToProcess = synthProcessInfo.getSamplesInBlock();
		voiceProcessInfo.setSamplesInBlock(samplesToProcess);

		midiInputData->setAuxDAWDataFloat(kBPM, synthProcessInfo.BPM);
		midiInputData->setAuxDAWDataFloat(kTSNumerator, synthProcessInfo.timeSigNumerator);
		midiInputData->setAuxDAWDataUINT(kTSDenominator, synthProcessInfo.timeSigDenomintor);
		midiInputData->setAuxDAWDataFloat(kAbsBufferTime, synthProcessInfo.absoluteBufferTime_Sec);
		
		// --- loop through voices and render/accumulate them
		for (unsigned int i = 0; i < MAX_VOICES; i++)
		{
			// --- blend active voices
			if (synthVoices[i]->isVoiceActive())
			{
				// --- render and accumulate
				synthVoices[i]->render(voiceProcessInfo);
				accumulateVoice(synthProcessInfo, gainFactor);
			}
		}

		// --- add master volume
		applyGlobalVolume(synthProcessInfo);

		// --- note that this is const, and therefore read-only
		return true;
	}

	/**
	\brief
	Apply a single global volume control to output mix buffers

	\param synthProcessInfo structure containing all information needed
	about the current block to process including:
	- MIDI messages for the block
	- pointers to the output audio buffers
	- optional pointers to input audio buffers (not used in SynthLab)

	\return true if sucessful
	*/
	void SynthEngine::applyGlobalVolume(SynthProcessInfo& synthProcessInfo)
	{
		// --- apply global volume
		//     globalMIDIData[kMIDIMasterVolume] = 0 -> 16383
		//	   mapping to -60dB(0.001) to +12dB(4.0)
		double globalVol = midi14_bitToDouble(midiInputData->getGlobalMIDIData(kMIDIMasterVolumeLSB),
			midiInputData->getGlobalMIDIData(kMIDIMasterVolumeMSB),
			0.001, 4.0); /* mapping to -60dB(0.001) to +12dB(4.0) */

		float* synthLeft = synthProcessInfo.getOutputBuffer(LEFT_CHANNEL);
		float* synthRight = synthProcessInfo.getOutputBuffer(RIGHT_CHANNEL);
		uint32_t samplesInBlock = synthProcessInfo.getSamplesInBlock();

		for (uint32_t i = 0; i < samplesInBlock; i++)
		{
			// --- stereo
			synthLeft[i] *= globalVol;
			synthRight[i] *= globalVol;
		}
	}

	/**
	\brief
	Accumulates voice buffers into a single mix buffer for each channel.

	\param synthProcessInfo structure containing all information needed
	about the current block to process including:
	- MIDI messages for the block
	- pointers to the output audio buffers
	- optional pointers to input audio buffers (not used in SynthLab)

	\param scaling a scalar value to apply while accumulating

	\return true if sucessful
	*/
	void SynthEngine::accumulateVoice(SynthProcessInfo& synthProcessInfo, double scaling)
	{
		// --- accumulate results
		uint32_t samplesInBlock = synthProcessInfo.getSamplesInBlock();
		float* synthLeft = synthProcessInfo.getOutputBuffer(LEFT_CHANNEL);
		float* synthRight = synthProcessInfo.getOutputBuffer(RIGHT_CHANNEL);
		float* voiceLeft = voiceProcessInfo.getOutputBuffer(LEFT_CHANNEL);
		float* voiceRight = voiceProcessInfo.getOutputBuffer(RIGHT_CHANNEL);

		for (uint32_t i = 0; i < samplesInBlock; i++)
		{
			// --- stereo
			synthLeft[i] += voiceLeft[i] * scaling;
			synthRight[i] += voiceRight[i] * scaling;
		}
	}

	/**
	\brief The MIDI event handler function; for note on/off messages it finds the voices to turn on/off.
	MIDI CC information is placed in the shared CC array.

	\param event a single MIDI Event to decode and process

	\return true if handled, false otherwise
	*/
	bool SynthEngine::processMIDIEvent(midiEvent& event)
	{
		if (parameters->enableMIDINoteEvents && event.midiMessage == NOTE_ON)
		{
			// --- set current MIDI data
			midiInputData->setGlobalMIDIData(kCurrentMIDINoteNumber, event.midiData1);
			midiInputData->setGlobalMIDIData(kCurrentMIDINoteVelocity, event.midiData2);

			// --- mono mode
			if (parameters->synthModeIndex == enumToInt(SynthMode::kMono) ||
				parameters->synthModeIndex == enumToInt(SynthMode::kLegato))
			{
				// --- just use voice 0 and do the note EG variables will handle the rest
				synthVoices[0]->processMIDIEvent(event);
			}
			else if (parameters->synthModeIndex == enumToInt(SynthMode::kUnison) ||
						parameters->synthModeIndex == enumToInt(SynthMode::kUnisonLegato))
			{
				// --- UNISON mode is heavily dependent on the manufacturer's
				//     implementation and decision
				//     for the synth core, we will use 4 voices 
				synthVoices[0]->processMIDIEvent(event);
				synthVoices[1]->processMIDIEvent(event);
				synthVoices[2]->processMIDIEvent(event);
				synthVoices[3]->processMIDIEvent(event);
			}
			else if (parameters->synthModeIndex == enumToInt(SynthMode::kPoly))
			{
				// --- get index of the next available voice (for note on events)
				int voiceIndex = getFreeVoiceIndex();

				if (voiceIndex < 0)
				{
					voiceIndex = getVoiceIndexToSteal();
				}

				// --- trigger next available note
				if (voiceIndex >= 0)
				{
					synthVoices[voiceIndex]->processMIDIEvent(event);
				}
	
				// --- increment all timestamps for note-on voices
				for (int i = 0; i < MAX_VOICES; i++)
				{
					if (synthVoices[i]->isVoiceActive())
						synthVoices[i]->incrementTimestamp();
				}
			}

			// --- need to store these for things like portamento
			// --- store global data for note ON event: set previous note-on data
			midiInputData->setGlobalMIDIData(kLastMIDINoteNumber, event.midiData1);
			midiInputData->setGlobalMIDIData(kLastMIDINoteVelocity, event.midiData2);
		}
		else if (parameters->enableMIDINoteEvents && event.midiMessage == NOTE_OFF)
		{
			// --- for mono, we only use one voice, number [0]
			if (parameters->synthModeIndex == enumToInt(SynthMode::kMono) || 
				parameters->synthModeIndex == enumToInt(SynthMode::kLegato))
			{
				if (synthVoices[0]->isVoiceActive())
				{
					synthVoices[0]->processMIDIEvent(event);
					return true;
				}
			}
			else if (parameters->synthModeIndex == enumToInt(SynthMode::kPoly))
			{
				// --- find the note with this MIDI number (this implies that note numbers and voices are exclusive to each other)
				int voiceIndex = getActiveVoiceIndexInNoteOn(event.midiData1);

				if (voiceIndex < 0)
				{
					voiceIndex = getStealingVoiceIndexInNoteOn(event.midiData1);
				}

				if (voiceIndex >= 0)
				{
					synthVoices[voiceIndex]->processMIDIEvent(event);
				}
				return true;
			}
			else if (parameters->synthModeIndex == enumToInt(SynthMode::kUnison) || 
				parameters->synthModeIndex == enumToInt(SynthMode::kUnisonLegato))
			{
				// --- this will get complicated with voice stealing.
				synthVoices[0]->processMIDIEvent(event);
				synthVoices[1]->processMIDIEvent(event);
				synthVoices[2]->processMIDIEvent(event);
				synthVoices[3]->processMIDIEvent(event);

				return true;
			}
		}
		else // --- non-note stuff here!
		{
			// --- store the data in our arrays; sub-components have access to all data via safe IMIDIData pointer
			if (event.midiMessage == PITCH_BEND)
			{
				midiInputData->setGlobalMIDIData(kMIDIPitchBendDataLSB, event.midiData1);
				midiInputData->setGlobalMIDIData(kMIDIPitchBendDataMSB, event.midiData2);
			}
			if (event.midiMessage == CONTROL_CHANGE)
			{
				// --- store CC event in globally shared array
				midiInputData->setCCMIDIData(event.midiData1, event.midiData2);
			}

			// --- NOTE: this synth has GUI controls for items that may also be transmitted via SYSEX-MIDI
			//
			//           If you want your synth plugin to support these messages, you need to add the code here
			//           to handle the MIDI. See any website with MIDI specs details or
			//			 http://www.somascape.org/midi/tech/spec.html
			//
			//			 The following global MIDI slots are used in this synth core so you want to process them too;
			//			 You may also decide to add more GUI controls and/or MIDI SYSEX message handling.
			//
			//			 globalMIDIData[kMIDIMasterVolume]
			//			 globalMIDIData[kMIDIMasterPitchBendCoarse] // aka pitch bend sensitivity
			//			 globalMIDIData[kMIDIMasterPitchBendFine]	// aka pitch bend sensitivity
			//			 globalMIDIData[kMIDIMasterTuningCoarse]
			//			 globalMIDIData[kMIDIMasterTuningFine]
			//
		}

		return true;
	}

	/**
	\brief Function to update the engine and voice parameters
	- note that most pararameter update information is in shared pointers to structures
	- most updating is done at the ModuleCore level
	- converts incoming global tune and volume information into MIDI data for cores to use
	- sets up voice detuning and panning in unison mode

	\param _parameters custom parameter structure, delivered from plugin framework 
	prior to calling the render function

	\return true if handled, false otherwise
	*/
	void SynthEngine::setParameters(std::shared_ptr<SynthEngineParameters>& _parameters)
	{
		// --- store parameters
		parameters = _parameters;

		// --- engine mode: poly, mono or unison
		parameters->voiceParameters->synthModeIndex = parameters->synthModeIndex;

		for (unsigned int i = 0; i < MAX_VOICES; i++)
		{
			// --- needed for modules YES
			synthVoices[i]->update();

			if (synthVoices[i]->isVoiceActive())
			{
				// -- note the special handling for unison mode - you could probably
				//    clean this up
				if (parameters->synthModeIndex == enumToInt(SynthMode::kUnison) || 
					parameters->synthModeIndex == enumToInt(SynthMode::kUnisonLegato))
				{
					if (i == 0)
					{
						parameters->voiceParameters->unisonDetuneCents = 0.0;
						parameters->voiceParameters->unisonStartPhase = 0.0;
					}
					else if (i == 1)
					{
						parameters->voiceParameters->unisonDetuneCents = parameters->globalUnisonDetune_Cents;
						parameters->voiceParameters->unisonStartPhase = 13.0;
					}
					else if (i == 2)
					{
						parameters->voiceParameters->unisonDetuneCents = -parameters->globalUnisonDetune_Cents;
						parameters->voiceParameters->unisonStartPhase = -13.0;
					}
					else if (i == 3)
					{
						parameters->voiceParameters->unisonDetuneCents = 0.707*parameters->globalUnisonDetune_Cents;
						parameters->voiceParameters->unisonStartPhase = 37.0;
					}
				}
				else
				{
					parameters->voiceParameters->unisonStartPhase = 0.0;
					parameters->voiceParameters->unisonDetuneCents = 0.0;
				}
			}
		}
	}

	/**
	\brief Helper function to find a free voice to use

	\return index of free voice, or -1 if voice not found
	*/
	int SynthEngine::getFreeVoiceIndex()
	{
		for (unsigned int i = 0; i < MAX_VOICES; i++)
		{
			// --- return index of first free voice we find
			if (!synthVoices[i]->isVoiceActive())
				return i;
		}

		// --- didn't find any
		return -1;
	}

	/**
	\brief Helper function to find a free voice to steal based on some kind of heuristic

	\return index of the voice to be stolen, or -1 if voice not found (should never happen)
	*/
	int SynthEngine::getVoiceIndexToSteal()
	{
		// --- add your heuristic code here to return the index of the voice to steal
		// --- find oldest note
		int index = -1;
		int timestamp = -1;
		int currentTimestamp = -1;
		for (int i = 0; i < MAX_VOICES; i++)
		{
			currentTimestamp = (int)(synthVoices[i]->getTimestamp());

			// --- find index of oldest voice = highest timestamp number
			if (currentTimestamp > timestamp)
			{
				timestamp = currentTimestamp;
				index = i;
			}
		}

		// --- index should always be >= 0
		return index;
	}


	/**
	\brief Helper function to find the voice that is playing a certain MIDI note

	\param midiNoteNumber note number that a voice may or may not be plaing

	\return index of the voice playing the note or -1 if no voice is playing this note
	*/
	int SynthEngine::getActiveVoiceIndexInNoteOn(unsigned int midiNoteNumber)
	{
		for (unsigned int i = 0; i < MAX_VOICES; i++)
		{
			if (synthVoices[i]->isVoiceActive() &&
				synthVoices[i]->getVoiceState() == voiceState::kNoteOnState &&
				synthVoices[i]->getMIDINoteNumber() == midiNoteNumber)
				return i;
		}
		return -1;
	}

	/**
	\brief Helper function to find the voice that is playing a certain MIDI note 
	and that will be stolen

	\param midiNoteNumber note number that a voice may or may not be plaing

	\return index of the voice playing the note or -1 if no voice is playing this note
	*/
	int SynthEngine::getStealingVoiceIndexInNoteOn(unsigned int midiNoteNumber)
	{
		for (unsigned int i = 0; i < MAX_VOICES; i++)
		{
			if (synthVoices[i]->isVoiceActive() &&
				synthVoices[i]->getVoiceState() == voiceState::kNoteOnState &&
				synthVoices[i]->voiceIsStealing() &&
				synthVoices[i]->getStealMIDINoteNumber() == midiNoteNumber)
				return i;
		}
		return -1;
	}

}