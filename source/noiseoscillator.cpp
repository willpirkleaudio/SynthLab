#include "noiseoscillator.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   noiseoscillator.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs Noise Oscillator module.
	- See class declaration for information on standalone operation

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	NoiseOscillator::NoiseOscillator(std::shared_ptr<MidiInputData> _midiInputData, 
		std::shared_ptr<NoiseOscillatorParameters> _parameters,
		uint32_t blockSize)
		: SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- standalone ONLY: parameters
		if (!parameters)
			parameters.reset(new NoiseOscillatorParameters);
		
		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(NOISE_OSC_INPUTS, NOISE_OSC_OUTPUTS, blockSize));
	}

	/**
	\brief Resets object to initialized state
	- call once during initialization
	- call any time sample rate changes (after init)

	\param _sampleRate the current sample rate in Hz

	\returns true if successful, false otherwise
	*/
	bool NoiseOscillator::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		return true;
	}

	/**
	\brief Updates the output amplitude gain value

	\returns true if successful, false otherwise
	*/
	bool NoiseOscillator::update()
	{
		// --- scale from dB
		outputAmplitude = dB2Raw(parameters->outputAmplitude_dB);

		return true;
	}

	/**
	\brief Renders audio from the noise generator object.
	- Calls the update function first - NOTE: owning object does not need to call update()
	- samples to process should normally be the block size, but may be a partial block in some cases
	due to OS/CPU activity.

	\returns true if successful, false otherwise
	*/
	bool NoiseOscillator::render(uint32_t samplesToProcess)
	{
		// --- update parameters for this block
		update();

		float* leftOutBuffer = audioBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOutBuffer = audioBuffers->getOutputBuffer(RIGHT_CHANNEL);

		for (uint32_t i = 0; i < samplesToProcess; i++)
		{
			// --- render variable
			double oscOutput = 0.0;

			// --- use the helper functions
			if (parameters->waveform == NoiseWaveform::kWhiteNoise)
				oscOutput = noiseGen.doWhiteNoise();
			else if (parameters->waveform == NoiseWaveform::kGaussWhiteNoise)
				oscOutput = noiseGen.doGaussianWhiteNoise();
			else if (parameters->waveform == NoiseWaveform::kPinkNoise)
				oscOutput = noiseGen.doPinkNoise();

			// --- scale by gain control
			oscOutput *= outputAmplitude;

			// --- write to output buffers
			leftOutBuffer[i] = oscOutput;
			rightOutBuffer[i] = leftOutBuffer[i];
		}

		// --- rendered
		return true;
	}

	/**
	\brief Calls the note-on handler for the module

	\returns true if successful, false otherwise
	*/
	bool NoiseOscillator::doNoteOn(MIDINoteEvent& noteEvent)
	{
		return true;
	}

	/**
	\brief Calls the note-off handler for the module

	\returns true if successful, false otherwise
	*/
	bool NoiseOscillator::doNoteOff(MIDINoteEvent& noteEvent)
	{
		return true;
	}

}// namespace



