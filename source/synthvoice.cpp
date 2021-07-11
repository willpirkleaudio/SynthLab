// --- Synth Core v1.0
//
#include "synthvoice.h"

namespace SynthLab
{
	/**
	\brief
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
		if (!midiInputData)
			midiInputData.reset(new (MidiInputData));

		if (!midiInputData)
			midiOutputData.reset(new (MidiOutputData));

		// --- this happens in stand-alone mode; does not happen otherwise;
		//     the first initialized SynthLab component creates its own parameters
		if (!parameters)
			parameters = std::make_shared<SynthVoiceParameters>();

		// --- create your member modules here
		// --- create our audio buffers
		mixBuffers.reset(new SynthProcessInfo(NO_CHANNELS, STEREO_CHANNELS, blockSize));

		// --- more inits here

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

		// --- initialize sub components for new sample rate

		return true;
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
		// --- initialize all sub components that need the DLL path here
		//     it is up to you to find that path if you need it

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
		// --- update your sub-components here based on the GUI parameters (if you used them)

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

		// --- render your objects into the mix buffer output

		// --- to mains
		copyOutputToOutput(mixBuffers, synthProcessInfo, STEREO_TO_STEREO, samplesToProcess);

		// --- check for note off condition
		if (voiceIsActive)
		{
			// --- check to see if voice has expired (final output EG has terminated)
			/* if(EG is complete)
				voiceIsActive = false; */
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

		// --- send NOTE ON message to your sub objects here



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

		// --- send NOTE OFF message to your sub objects here

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