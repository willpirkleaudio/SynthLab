#include "audiodelay.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   audiodelay.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs a stereo audio delay processor
	- See class declaration for information on standalone operation

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	AudioDelay::AudioDelay(std::shared_ptr<MidiInputData> _midiInputData, 
		std::shared_ptr<AudioDelayParameters> _parameters, 
		uint32_t blockSize) :
		SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- standalone ONLY: parameters
		if (!parameters)
			parameters.reset(new AudioDelayParameters);
		
		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(DELAY_AUDIO_INPUTS, DELAY_AUDIO_OUTPUTS, blockSize));
	}

	/**
	\brief Resets object to initialized state
	- call once during initialization
	- call any time sample rate changes (after init)

	\param _sampleRate the current sample rate in Hz

	\returns true if successful, false otherwise
	*/
	bool AudioDelay::reset(double _sampleRate)
	{
		// --- if sample rate did not change
		if (sampleRate == _sampleRate)
		{
			// --- just flush buffer and return
			delayBuffer_L.flushBuffer();
			delayBuffer_R.flushBuffer();
			return true;
		}

		sampleRate = _sampleRate;

		// --- create new buffer, will store sample rate and length(mSec)
		createDelayBuffers(_sampleRate, 2000.0);

		return true;
	}

	/**
	\brief Updates the selected core
		- Recalculates delay in fractional samples for current block

	\returns true if successful, false otherwise
	*/
	bool AudioDelay::update()
	{
		// --- check mix in dB for calc
		dryMix = pow(10.0, parameters->dryLevel_dB / 20.0);
		wetMix = pow(10.0, parameters->wetLevel_dB / 20.0);

		// --- set left and right delay times in fractional
		double newDelayInSamples_L = parameters->leftDelay_mSec*(samplesPerMSec);
		double newDelayInSamples_R = parameters->rightDelay_mSec*(samplesPerMSec);

		// --- new delay time with fraction
		delayInSamples_L = newDelayInSamples_L;
		delayInSamples_R = newDelayInSamples_R;

		return true;
	}

	/**
	\brief Processes audio through the stereo delay
	- Calls the update function first - NOTE: owning object does not need to call update()
	- samples to process should normally be the block size, but may be a partial block in some cases
	due to OS/CPU activity.

	\returns true if successful, false otherwise
	*/
	bool AudioDelay::render(uint32_t samplesToProcess)
	{
		update();

		// --- stereo I/O
		float* leftInBuffer = getAudioBuffers()->getInputBuffer(LEFT_CHANNEL);
		float* leftOutBuffer = getAudioBuffers()->getOutputBuffer(LEFT_CHANNEL);

		float* rightInBuffer = getAudioBuffers()->getInputBuffer(RIGHT_CHANNEL);
		float* rightOutBuffer = getAudioBuffers()->getOutputBuffer(RIGHT_CHANNEL);

		for (uint32_t i = 0; i <samplesToProcess; i++)
		{
			// --- inputs
			double xnL = leftInBuffer[i];
			double xnR = rightInBuffer[i];

			// --- read delays
			double ynL = delayBuffer_L.readBuffer(delayInSamples_L);
			double ynR = delayBuffer_R.readBuffer(delayInSamples_R);

			// --- create input for delay buffer with LEFT channel info
			double dnL = xnL + (parameters->feedback_Pct / 100.0) * ynL;

			// --- create input for delay buffer with RIGHT channel info
			double dnR = xnR + (parameters->feedback_Pct / 100.0) * ynR;

			// --- this sets up a ping-pong delay
			// --- write to LEFT delay buffer with RIGHT channel info
			delayBuffer_L.writeBuffer(dnR);

			// --- write to RIGHT delay buffer with LEFT channel info
			delayBuffer_R.writeBuffer(dnL);

			// --- form mixture out = dry*xn + wet*yn
			double outputL = dryMix*xnL + wetMix*ynL;

			// --- form mixture out = dry*xn + wet*yn
			double outputR = dryMix*xnR + wetMix*ynR;

			// --- set left channel
			leftOutBuffer[i] = outputL;

			// --- set right channel
			rightOutBuffer[i] = outputR;
		}

		return true;
	}

	/**
	\brief Perform note-on operations for the component
		- nothing to do here

	\return true if handled, false if not handled
	*/
	bool AudioDelay::doNoteOn(MIDINoteEvent& noteEvent)
	{
		return true;
	}

	/**
	\brief Perform note-off operations for the component
		- nothing to do here

	\return true if handled, false if not handled
	*/
	bool AudioDelay::doNoteOff(MIDINoteEvent& noteEvent)
	{
		return true;
	}

	/**
	\brief Create new circular buffers on init, or anytime the sample rate changes
	- the circular buffer objects will delete existing buffers automatically

	\return true if handled, false if not handled
	*/
	void  AudioDelay::createDelayBuffers(double _sampleRate, double _bufferLength_mSec)
	{
		// --- store for math
		bufferLength_mSec = _bufferLength_mSec;
		sampleRate = _sampleRate;
		samplesPerMSec = sampleRate / 1000.0;

		// --- total buffer length including fractional part
		bufferLength = (unsigned int)(bufferLength_mSec*(samplesPerMSec)) + 1; // +1 for fractional part

		 // --- create new buffer
		delayBuffer_L.createCircularBuffer(bufferLength);
		delayBuffer_R.createCircularBuffer(bufferLength);
	}

}

