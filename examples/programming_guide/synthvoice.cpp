// --- Synth Core v1.0
//
#include "synthvoice.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthvoice.cpp
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
	This is a template file with a minimal implementation. 

	Construction:
	- constructs each of its SynthModule objects and uses one of the members of the voice
	parameter structure as the shared parameter pointers
	- supplies wavetable database to oscillators that need it
	- supplies PCM sample database to oscillators that need it
	- sets up the modulation matrix source and destinations using the SynthModule components
	- sets up hard-wired modulation matrix routings
	- supports standalone operation using nullptrs (see documentation)

	\param _midiInputData shared pointer to MIDI input data; created on SynthEngine
	\param _midiOutputData shared pointer to MIDI input data; not used
	\param _parameters shared pointer to voice parameter structure; shared with all other voices
	\param _wavetableDatabase shared pointer to wavetable database; created on SynthEngine
	\param _sampleDatabase shared pointer to PCM sample database; created on SynthEngine
	\param blockSize the block size to be used for the lifetime of operation; OK if arriving blocks are smaller than this value, NOT OK if larger

	\returns the newly constructed object
	*/
	SynthVoice::SynthVoice(std::shared_ptr<MidiInputData> _midiInputData,
		std::shared_ptr<MidiOutputData> _midiOutputData,
		std::shared_ptr<SynthVoiceParameters> _parameters,
		std::shared_ptr<WavetableDatabase> _wavetableDatabase,
		std::shared_ptr<PCMSampleDatabase> _sampleDatabase,
		uint32_t _blockSize)
		: midiInputData(_midiInputData)		//<- set our midi dat interface value
		, midiOutputData(_midiOutputData)
		, parameters(_parameters)	//<- set our parameters
		, wavetableDatabase(_wavetableDatabase)
		, sampleDatabase(_sampleDatabase)
		, blockSize(_blockSize)
	{
		// for standalone
		if (!midiInputData)
			midiInputData.reset(new (MidiInputData));
		if (!midiOutputData)
			midiOutputData.reset(new (MidiOutputData));

		// --- this happens in stand-alone mode; does not happen otherwise;
		//     the first initialized SynthLab component creates its own parameters
		if (!parameters)
			parameters = std::make_shared<SynthVoiceParameters>();

		// --- NOTE: in standalone mode, the modules will create the wavetable and PCM databases
		//           locally, so they do not need to be checked here.

		// --- LFOs
		lfo.reset(new SynthLFO(midiInputData, parameters->lfoParameters, blockSize));

		// --- EGs
		ampEG.reset(new EnvelopeGenerator(midiInputData, parameters->ampEGParameters, blockSize));
	
		// --- wt oscillator
		wtOsc.reset(new WTOscillator(midiInputData, parameters->wtOscParameters, wavetableDatabase, blockSize));

		// --- filters
		filter.reset(new SynthFilter(midiInputData, parameters->filterParameters, blockSize));

		// --- DCA
		dca.reset(new DCA(midiInputData, parameters->dcaParameters, blockSize));

		// --- mod matrix
		modMatrix.reset(new ModMatrix(parameters->modMatrixParameters));

		// --- create our audio buffers
		mixBuffers.reset(new SynthProcessInfo(NO_CHANNELS, STEREO_CHANNELS, blockSize));

		// --- mod matrix can be reconfigured on the fly
		//
		// --- (1) clear the arrays
		modMatrix->clearModMatrixArrays();

		// --- (2) setup possible sources and destinations; can also be done on the fly
		//
		// --- add the sources
		modMatrix->addModSource(SynthLab::kSourceLFO1_Norm, lfo->getModulationOutput()->getModArrayPtr(SynthLab::kLFONormalOutput));
		modMatrix->addModSource(SynthLab::kSourceAmpEG_Norm, ampEG->getModulationOutput()->getModArrayPtr(SynthLab::kEGNormalOutput));

		// --- add the destinations
		modMatrix->addModDestination(SynthLab::kDestOsc1_fo, wtOsc->getModulationInput()->getModArrayPtr(SynthLab::kBipolarMod));
		modMatrix->addModDestination(SynthLab::kDestFilter1_fc_Bipolar, filter->getModulationInput()->getModArrayPtr(SynthLab::kBipolarMod));
		modMatrix->addModDestination(SynthLab::kDestDCA_EGMod, dca->getModulationInput()->getModArrayPtr(SynthLab::kEGMod));

		// --- hardwire the routings for now; the default hardwired intenstity is 1.0
		modMatrix->getParameters()->setMM_HardwiredRouting(SynthLab::kSourceLFO1_Norm, SynthLab::kDestOsc1_fo);
		modMatrix->getParameters()->setMM_HardwiredRouting(SynthLab::kSourceLFO1_Norm, SynthLab::kDestFilter1_fc_Bipolar);
		modMatrix->getParameters()->setMM_HardwiredRouting(SynthLab::kSourceAmpEG_Norm, SynthLab::kDestDCA_EGMod);

	}

	/**
	\brief
	Initialize the voice sub-components; this really only applies to PCM oscillators that need DLL path
	BUT, DLL path is avaialble for all modules and may be used in clever ways

	\param dllPath path to folder containing this DLL

	\returns true if sucessful
	*/
	bool SynthVoice::initialize(const char* dllPath)
	{
		// --- initialize all sub components that need the DLL path
		wtOsc->initialize(dllPath);
		
		return true;
	}

	/**
	\brief
	Reset all SynthModules on init or when sample rate changes

	\param _sampleRate sample rate

	\returns true if sucessful
	*/
	bool SynthVoice::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		currentMIDINote = -1;

		// --- reset modules
		lfo->reset(_sampleRate);
		ampEG->reset(_sampleRate);
		wtOsc->reset(_sampleRate);
		filter->reset(_sampleRate);
		dca->reset(_sampleRate);
		return true;
	}

	/**
	\brief
	Update voice specific stuff.
	- the main thing this does is to load new cores as user selects them

	\returns true if sucessful
	*/
	bool SynthVoice::update()
	{
		// --- do updates to sub-components 
		//     NOTE: this is NOT for GUI control updates for normal synth operation
		//
		// ---- sets unison mode detuning and phase from GUI controls (optional)
		wtOsc->setUnisonMode(parameters->unisonDetuneCents, parameters->unisonStartPhase);

		return true;
	}

	/**
	\brief
	Accumulate buffers into voice's mix buffers

	\param oscBuffers buffers to add to the mix buffer
	\param samplesInBlock samples in this block to accumulate
	\param scaling scaling mix coefficient if needed

	*/
	void SynthVoice::accumulateToMixBuffer(std::shared_ptr<AudioBuffer> oscBuffers, uint32_t samplesInBlock, double scaling)
	{
		float* leftOutBuffer = mixBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOutBuffer = mixBuffers->getOutputBuffer(RIGHT_CHANNEL);
		float* leftOscBuffer = oscBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOscBuffer = oscBuffers->getOutputBuffer(RIGHT_CHANNEL);

		for (uint32_t i = 0; i < samplesInBlock; i++)
		{
			// --- stereo
			leftOutBuffer[i] += leftOscBuffer[i] * scaling;
			rightOutBuffer[i] += rightOscBuffer[i] * scaling;
		}
	}

	/**
	\brief
	Write buffer into voice's mix buffers; unlike accumulate, this overwrites the audio data

	\param oscBuffers buffers to overwrite into the mix buffer
	\param samplesInBlock samples in this block to accumulate
	\param scaling scaling mix coefficient if needed

	*/
	void SynthVoice::writeToMixBuffer(std::shared_ptr<AudioBuffer> oscBuffers, uint32_t samplesInBlock, double scaling = 1.0)
	{
		float* leftOutBuffer = mixBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOutBuffer = mixBuffers->getOutputBuffer(RIGHT_CHANNEL);
		float* leftOscBuffer = oscBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOscBuffer = oscBuffers->getOutputBuffer(RIGHT_CHANNEL);

		for (uint32_t i = 0; i < samplesInBlock; i++)
		{
			// --- stereo
			leftOutBuffer[i] = leftOscBuffer[i] * scaling;
			rightOutBuffer[i] = rightOscBuffer[i] * scaling;
		}
	}

	/**
	\brief
	Render a block of audio data for an active note event
	- call the render function on modulators
	- run the modulation matrix
	- call the render function on mod targets
	- move audio output from oscillators to filters
	- move audio output from filters to DCA
	- move audio output from DCA to main mix buffers
	- check the status of the Amp EG object to see if note has expired
	- set steal-pending flag if voice is to be taken for next event

	\param synthProcessInfo structure of data needed for rendering this block.

	*/
	bool SynthVoice::render(SynthProcessInfo& synthProcessInfo)
	{
		uint32_t samplesToProcess = synthProcessInfo.getSamplesInBlock();

		// --- clear for accumulation
		mixBuffers->flushBuffers();

		// --- render LFO outputt
		lfo->render(samplesToProcess);
	
		// --- render EG output
		ampEG->render(samplesToProcess);

		// --- do the modulation routings
		modMatrix->runModMatrix();

		// --- render oscillator
		wtOsc->render(samplesToProcess);

		// --- transfer information from OSC output to filter input
		SynthLab::copyOutputToInput(wtOsc->getAudioBuffers(),
			filter->getAudioBuffers(),
			SynthLab::STEREO_TO_STEREO, blockSize);

		// --- render filter
		filter->render(samplesToProcess);

		// --- transfer information from fikter output to DCA input
		SynthLab::copyOutputToInput(filter->getAudioBuffers(),
			dca->getAudioBuffers(),
			SynthLab::STEREO_TO_STEREO, blockSize);

		// --- render DCA
		dca->render(samplesToProcess);

		// --- to mains
		copyOutputToOutput(dca->getAudioBuffers(), synthProcessInfo, STEREO_TO_STEREO, samplesToProcess);

		// --- check for note off condition
		if (voiceIsActive)
		{
			// --- has ampEG expired yet?
			if (ampEG->getState() == enumToInt(EGState::kOff))
			{				
				voiceIsActive = false;
			}
		}
		return true;
	}

	/**
	\brief
	Note-on handler for voice
	- For oscillators: start glide modulators then call note-on handlers
	- For all others: call note-on handlers
	- set and save voice state information

	\param event MIDI note event

	*/
	bool SynthVoice::doNoteOn(midiEvent& event)
	{
		// --- calculate MIDI -> pitch value
		double midiPitch = midiNoteNumberToOscFrequency(event.midiData1);
		int32_t lastMIDINote = currentMIDINote;
		currentMIDINote = (int32_t)event.midiData1;

		MIDINoteEvent noteEvent(midiPitch, event.midiData1, event.midiData2);
		// --- TEMPLATE CODE END

		// --- needed forLFO  modes
		lfo->doNoteOn(noteEvent);

		// --- EG
		ampEG->doNoteOn(noteEvent);

		// --- create glide info structure out of notes and times
		GlideInfo glideInfo(lastMIDINote, currentMIDINote, parameters->glideTime_mSec, sampleRate);

		// --- set glide mod
		wtOsc->startGlideModulation(glideInfo);
		wtOsc->doNoteOn(noteEvent);
	
		// --- filter needs note on for key track
		filter->doNoteOn(noteEvent);

		// --- DCA
		dca->doNoteOn(noteEvent);

		// --- TEMPLATE CODE CONTINUED
		// --- set the flag
		voiceIsActive = true; // we are ON
		voiceNoteState = voiceState::kNoteOnState;

		// --- this saves the midi note number and velocity so that we can identify our own note
		voiceMIDIEvent = event;

		return true;
	}

	/**
	\brief
	Note-off handler for voice
	- just forwards note-off handling to sub-objects

	\param event MIDI note event
	*/
	bool SynthVoice::doNoteOff(midiEvent& event)
	{
		// --- lookup MIDI -> pitch value
		double midiPitch = midiNoteNumberToOscFrequency(event.midiData1);

		MIDINoteEvent noteEvent(midiPitch, event.midiData1, event.midiData2);

		// --- components
		lfo->doNoteOff(noteEvent);
		ampEG->doNoteOff(noteEvent);
		wtOsc->doNoteOff(noteEvent);
		filter->doNoteOff(noteEvent);
		dca->doNoteOff(noteEvent);

		// --- set our current state; the ampEG will determine the final state
		voiceNoteState = voiceState::kNoteOffState;
		
		return true;
	}

	/**
	\brief
	MIDI Event handler
	- decodes MIDI message and processes note-on and note-off events only
	- all other MIDI messages are decoded and data stored in Synth Engine prior to calling this function

	\param event MIDI note event
	*/
	bool SynthVoice::processMIDIEvent(midiEvent& event)
	{
		// --- the voice only needs to process note on and off
		//     Other MIDI info such as CC can be found in global midi table via our midiData interface
		if (event.midiMessage == NOTE_ON)
		{
			// --- clear timestamp
			clearTimestamp();

			// --- call the subfunction
			doNoteOn(event);
		}
		else if (event.midiMessage == NOTE_OFF)
		{
			// --- call the subfunction
			doNoteOff(event);
		}

		return true;
	}
}