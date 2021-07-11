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
	\param config OPTIONAL argument for DM synth configuration; can be safely ignored for non DM products

	\returns the newly constructed object
	*/
	SynthEngine::SynthEngine(uint32_t blockSize, DMConfig* config)
	{
		// --- DM config file; OK if this is NULL for non-DM products
		initDMConfig(midiInputData, config);

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
		for (uint32_t i = 0; i < MAX_VOICES; i++)
		{
			// --- reset is the constructor for this kind of smartpointer
			//
			//     Pass our this pointer for the IMIDIData interface - safe
			synthVoices[i].reset(new SynthVoice(midiInputData, midiOutputData, parameters->voiceParameters, wavetableDatabase, sampleDatabase, blockSize));
		}

		// --- voice object
		voiceProcessInfo.init(0, 2, blockSize);

		// --- delay FX
		pingPongDelay.reset(new AudioDelay(midiInputData, parameters->audioDelayParameters, blockSize));

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
	Forwards custom code settings to first voice (since all voices share the same architecture)
	- 100% optional and only for those who know how to use dynamic strings with their frameworks

	*/
	void SynthEngine::setAllCustomUpdateCodes()
	{
		synthVoices[0]->setAllCustomUpdateCodes();
	}

	/**
	\brief
	Adds dynamic module cores to the voice's member obejcts.
	- only used in SynthLab-DM products

	*/
	void SynthEngine::setDynamicModules(std::vector<std::shared_ptr<SynthLab::ModuleCore>> modules,
		                                uint32_t voiceIndex)
	{
		if(voiceIndex < MAX_VOICES)
			synthVoices[voiceIndex]->setDynamicModules(modules);
	}

	/**
	\brief
	Gets module core names, four per object
	- Forwards call to first voice since all share the same architecture

	\return a vector full of module core NAME strings (standard implementation is 4 strings)
	*/
	std::vector<std::string> SynthEngine::getModuleCoreNames(uint32_t moduleType)
	{
		return synthVoices[0]->getModuleCoreNames(moduleType);
	}

	/**
	\brief
	Gets module-specific core STRINGS (e.g. waveform names for oscillators, filter types for filters, etc...
	- Forwards call to first voice since all share the same architecture

	\return a vector full of module-specific strings (standard implementation is 16 strings)
	*/
	std::vector<std::string> SynthEngine::getModuleStrings(uint32_t mask)
	{
		// --- NOTE all voices are identical so only need first voice info
		std::vector<std::string> emptyVector;

		return synthVoices[0]->getModuleStrings(mask, false);
		emptyVector.clear();
		return emptyVector;
	}

	/**
	\brief
	Gets module-specific Mod Knob label STRINGS
	- Forwards call to first voice since all share the same architecture

	\return a vector full of module core mod knob lable strings (standard implementation is 4 strings)
	*/
	std::vector<std::string> SynthEngine::getModKnobStrings(uint32_t mask)
	{
		// --- NOTE all voices are identical so only need first voice info
		std::vector<std::string> emptyVector;

		return synthVoices[0]->getModuleStrings(mask, true);
		emptyVector.clear();
		return emptyVector;
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
		for (uint32_t i = 0; i < MAX_VOICES; i++)
		{
			// --- smart poitner access looks normal (->)
			synthVoices[i]->reset(_sampleRate); // this calls reset() on the smart-pointers underlying naked pointer
		}

		// --- FX
		pingPongDelay->reset(_sampleRate);

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
		for (uint32_t i = 0; i < MAX_VOICES; i++)
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
		// --- mau do thie before?
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

		// --- -6dB per active channel to avoid clipping; I left some logic here if you want to experiment
		//     with the different synth modes
		double gainFactor = 0.5;

		//if (parameters->synthModeIndex == enumToInt(SynthMode::kPoly))
		//	gainFactor = 0.5;
		//else if (parameters->synthModeIndex == enumToInt(SynthMode::kUnison) ||
		//	     parameters->synthModeIndex == enumToInt(SynthMode::kUnisonLegato))
		//	gainFactor = 0.5;

		// --- this is important
		uint32_t samplesToProcess = synthProcessInfo.getSamplesInBlock();
		voiceProcessInfo.setSamplesInBlock(samplesToProcess);

		midiInputData->setAuxDAWDataFloat(kBPM, synthProcessInfo.BPM);
		midiInputData->setAuxDAWDataFloat(kTSNumerator, synthProcessInfo.timeSigNumerator);
		midiInputData->setAuxDAWDataUINT(kTSDenominator, synthProcessInfo.timeSigDenomintor);
		midiInputData->setAuxDAWDataFloat(kAbsBufferTime, synthProcessInfo.absoluteBufferTime_Sec);

		// --- loop through voices and render/accumulate them
		for (uint32_t i = 0; i < MAX_VOICES; i++)
		{
			// --- blend active voices
			if (synthVoices[i]->isVoiceActive())
			{
				// --- render and accumulate
				synthVoices[i]->render(voiceProcessInfo);
				accumulateVoice(synthProcessInfo, gainFactor);
			}
#ifdef SYNTHLAB_WS
			// --- sequencer status lights for voice 0 only
			if (i == 0)
				parameters->wsStatusMeters = parameters->voiceParameters->waveSequencerParameters->statusMeters;
#endif
		}

		// --- apply delay FX and other master FX here
		//
		if (parameters->enableDelayFX)
		{
			// --- copy synth output to delay input
			copySynthOutputToAudioBufferInput(synthProcessInfo, pingPongDelay->getAudioBuffers(), STEREO_TO_STEREO, samplesToProcess);

			// --- run the delay
			pingPongDelay->render(samplesToProcess);

			// --- copy to output
			copyAudioBufferOutputToSynthOutput(pingPongDelay->getAudioBuffers(), synthProcessInfo, STEREO_TO_STEREO, samplesToProcess);
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
				else // --- steal voice
					;//TRACE("-- DID NOT getFreeVoiceIndex index:%d \n", voiceIndex);

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
					if (voiceIndex >= 0)
						;// TRACE("-- Note OFF on STEAL-PENDING -> Voice:%d Note:%d Vel:%d \n", voiceIndex, event.midiData1, event.midiData2);
				}

				if (voiceIndex >= 0)
				{
					synthVoices[voiceIndex]->processMIDIEvent(event);
				}
				else
					;// TRACE("-- DID NOT FIND NOTE OFF (this is very bad) index:%d \n", voiceIndex);
				// --- this is very bad - it means we probably have a stuck note... :\

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

				if (event.midiData1 == ALL_NOTES_OFF)
				{
					midiEvent noteOffEvent = event;
					noteOffEvent.midiMessage = NOTE_OFF;
					noteOffEvent.midiData2 = 0; // 0 velocity
					noteOffEvent.midiSampleOffset = 0;
					for (uint32_t i = 0; i < MAX_VOICES; i++)
					{
						for (uint32_t j = 0; j < NUM_MIDI_NOTES; j++)
						{
							noteOffEvent.midiData1 = j;

							// --- blend active voices
							synthVoices[i]->doNoteOff(noteOffEvent);
						}
					}
				}
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

		// --- master volume maps to MIDI RPN see http://www.somascape.org/midi/tech/spec.html#usx7F0401
		double globalVolumeRaw = dB2Raw(parameters->globalVolume_dB);
		boundValue(globalVolumeRaw, 0.001, 4.0);

		// --- master volume is our own mapping -60 to +12
		uint32_t unipolarValue = mapDoubleToUINT(globalVolumeRaw, 0.001, 4.0, 0, 16383);

		// --- map 0 -> 16383 to MIDI 14-bit
		uint32_t lsb, msb = 0;
		unipolarIntToMIDI14_bit(unipolarValue, lsb, msb);
		midiInputData->setGlobalMIDIData(kMIDIMasterVolumeLSB, lsb);
		midiInputData->setGlobalMIDIData(kMIDIMasterVolumeMSB, msb);

		// --- store pitch bend range in midi data table; for a released synth, you want to decode this as SYSEX as well
		// --- sensitivity is in semitones (0 -> 127) and cents (0 -> 127)
		uint32_t pbCoarse = parameters->globalPitchBendSensCoarse;
		boundMIDIValueByte(pbCoarse);

		uint32_t pbFine = parameters->globalPitchBendSensFine;
		boundMIDIValueByte(pbFine);

		midiInputData->setGlobalMIDIData(kMIDIMasterPBSensCoarse, pbCoarse); // --- unsigned integer part of tuning, in semitones
		midiInputData->setGlobalMIDIData(kMIDIMasterPBSensFine, pbFine);		// --- fractional part of tuning, converted to cents PRIOR to this call

		// --- store master tuning info from GUI controls
		//     for a released synth, you want to decode this as SYSEX as well
		//
		// --- Master Tuning: See MIDI Spec http://www.somascape.org/midi/tech/spec.html#usx7F0401
		//
		// --- coarse (semitones): -64 to +63 maps-> 0, 127 (7-bit)
		//     This message is transmitted as )LSB, MSB) but LSB is always 0 so we can ignore
		int mtCoarse = parameters->globalTuningCoarse;
		mapIntValue(mtCoarse, -64, +63, 0, 127);		// --- MIDI specified values, not using constants here
		midiInputData->setGlobalMIDIData(kMIDIMasterTuneCoarseMSB, mtCoarse);
		midiInputData->setGlobalMIDIData(kMIDIMasterTuneCoarseLSB, 0);	// --- should never change

		// --- fine (cents): -100 to +100 maps-> -8192, 8191 (14-bit)
		int mtFine = parameters->globalTuningFine;
		mapIntValue(mtFine, -100, +100, -8192, 8191, false); // MIDI uses [-8192, 8191] range, false = no rounding (this is a big deal with MIDI)

		// --- map -8192 -> 8191 to MIDI 14-bit
		bipolarIntToMIDI14_bit(mtFine, -8192, 8191, lsb, msb);
		midiInputData->setGlobalMIDIData(kMIDIMasterTuneFineMSB, msb);
		midiInputData->setGlobalMIDIData(kMIDIMasterTuneFineLSB, lsb);

		// --- engine mode: poly, mono or unison
		parameters->voiceParameters->synthModeIndex = parameters->synthModeIndex;

		for (uint32_t i = 0; i < MAX_VOICES; i++)
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
		for (uint32_t i = 0; i < MAX_VOICES; i++)
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
	int SynthEngine::getActiveVoiceIndexInNoteOn(uint32_t midiNoteNumber)
	{
		for (uint32_t i = 0; i < MAX_VOICES; i++)
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
	int SynthEngine::getStealingVoiceIndexInNoteOn(uint32_t midiNoteNumber)
	{
		for (uint32_t i = 0; i < MAX_VOICES; i++)
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