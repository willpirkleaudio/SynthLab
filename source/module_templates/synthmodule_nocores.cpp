#include "synthmodule_nocores.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   synthmodule_nocores.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs Digitally Controlled Amplifier module.
	- See class declaration for information on standalone operation

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	SynthModuleNoCores::SynthModuleNoCores(std::shared_ptr<MidiInputData> _midiInputData,
		std::shared_ptr<SynthModuleNoCoresParameters> _parameters,
		uint32_t blockSize) :
		SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- standalone ONLY: parameters
		if (!parameters)
			parameters.reset(new SynthModuleNoCoresParameters);

		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(SynthModuleNoCores_AUDIO_INPUTS, SynthModuleNoCores_AUDIO_OUTPUTS, blockSize));
	}

	/**
	\brief Resets object to initialized state
	- call once during initialization
	- call any time sample rate changes (after init)

	\param _sampleRate the current sample rate in Hz

	\returns true if successful, false otherwise
	*/
	bool SynthModuleNoCores::reset(double _sampleRate)
	{
		gainRaw = 1.0;			// --- unity
		panLeftGain = 0.707;	// --- center
		panRightGain = 0.707;	// --- center
		midiVelocityGain = 1.0; // --- 127
		return true;
	}

	/**
	\brief Updates object by applying GUI parameter and input modulations to the internal variables
	- calculates a set of gain values, then applies them to the left and right channels
	- basically a big gain calculator

	\returns true if successful, false otherwise
	*/
	bool SynthModuleNoCores::update()
	{
		// --- apply Max Down modulator
		double ampMod = doUnipolarModulationFromMax(modulationInput->getModValue(kMaxDownAmpMod), 0.0, 1.0);
		ampMod *= parameters->ampModIntensity;

		// --- EG modulation
		double egMod = parameters->ampEGIntensity * modulationInput->getModValue(kEGMod);

		// --- flip EG output
		if (parameters->ampEGIntensity < 0.0)
			egMod += 1.0;

		// --- support for MIDI Volume CC
		double midiVolumeGain = mmaMIDItoAtten(midiInputData->getCCMIDIData(VOLUME_CC07));

		// --- calculate the final raw gain value
		//     multiply the various gains together: MIDI Velocity * EG Mod * Amp Mod * gain_dB (from GUI, next code line)
		gainRaw = midiVelocityGain * egMod * ampMod;

		// --- apply final output gain
		if (parameters->gainValue_dB > kMinAbsoluteGain_dB)
			gainRaw *= pow(10.0, parameters->gainValue_dB / 20.0);
		else
			gainRaw = 0.0; // OFF

		// --- now process pan modifiers
		double panTotal = parameters->panValue + (parameters->panModIntensity * modulationInput->getModValue(kPanMod));

		// --- limit in case pan control is biased
		boundValueBipolar(panTotal);

		// --- equal power calculation in synthfunction.h
		calculatePanValues(panTotal, panLeftGain, panRightGain);

		return true; // handled
	}

	/**
	\brief Processes audio from the input buffers to the output buffers
	- Calls the update function first - NOTE: owning object does not need to call update()
	- samples to process should normally be the block size, but may be a partial block in some cases
	due to OS/CPU activity.
	- the input and output buffers are accessed with
		audioBuffers->getInputBuffer() and
		audioBuffers->getOutputBuffer()

	\returns true if successful, false otherwise
	*/
	bool SynthModuleNoCores::render(uint32_t samplesToProcess)
	{
		// --- update parameters for this block
		update();

		// --- SynthModuleNoCores processes every sample into output buffers
		float* leftInBuffer = audioBuffers->getInputBuffer(LEFT_CHANNEL);
		float* rightInBuffer = audioBuffers->getInputBuffer(RIGHT_CHANNEL);
		float* leftOutBuffer = audioBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOutBuffer = audioBuffers->getOutputBuffer(RIGHT_CHANNEL);

		// --- process block
		for (uint32_t i = 0; i < samplesToProcess; i++)
		{
			// --- stereo, add left pan value
			leftOutBuffer[i] = leftInBuffer[i] * gainRaw * panLeftGain;
			rightOutBuffer[i] = rightInBuffer[i] * gainRaw * panRightGain;
		}
		return true;
	}

	/**
	\brief Perform note-on operations for the component

	\return true if handled, false if not handled
	*/
	bool SynthModuleNoCores::doNoteOn(MIDINoteEvent& noteEvent)
	{
		// --- store our MIDI velocity
		midiVelocityGain = mmaMIDItoAtten(noteEvent.midiNoteVelocity);

		// --- prevent mod inputs from accidentaly killing output (e.g. volume is set to 0)
		modulationInput->initInputValues();

		return true;
	}

	/**
	\brief Perform note-off operations for the component;
		   Here there is nothing to do.

	\return true if handled, false if not handled
	*/
	bool SynthModuleNoCores::doNoteOff(MIDINoteEvent& noteEvent)
	{
		return true;
	}
}