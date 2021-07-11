#include "synthbase.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthbase.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	// --- AudioBuffer -------------------------------------------------------------------------------------- //

	/**
	\brief
	Specialized constructor that initializes the buffers during construction

	\param _numInputChannels input channel count (1 = mono, 2 = stereo, may be greater than 2 if needed)
	\param _numOutputChannels input channel count (1 = mono, 2 = stereo, may be greater than 2 if needed)
	\param _blockSize the MAXIMUM block size that sets the buffer sizes; this can never be exceeded

	*/
	AudioBuffer::AudioBuffer(uint32_t _numInputChannels, uint32_t _numOutputChannels, uint32_t _blockSize)
	{
		init(_numInputChannels, _numOutputChannels, _blockSize);
	}

	/**
	\brief
	As one of the few objects that uses naked pointers (for maximum compatibility between plugin frameworks
	and/or plugin APIs and the easiest of use) this deletes the audio buffer arrays.
	
	*/
	AudioBuffer::~AudioBuffer()
	{
		destroyInputBuffers();
		destroyOutputBuffers();
	}


	/**
	\brief
	Destroy dynamically allocated input buffer; done at destruct time, or if client want to re-size buffers

	*/
	void AudioBuffer::destroyInputBuffers()
	{
		if (inputBuffer)
		{
			for (uint32_t i = 0; i < numInputChannels; i++)
			{
				delete[] inputBuffer[i];
			}
			delete[] inputBuffer;
			inputBuffer = nullptr;
		}
	}

	/**
	\brief
	Destroy dynamically allocated output buffer; done at destruct time, or if client want to re-size buffers

	*/
	void AudioBuffer::destroyOutputBuffers()
	{
		if (outputBuffer)
		{
			for (uint32_t i = 0; i < numOutputChannels; i++)
			{
				delete[] outputBuffer[i];
			}
			delete[] outputBuffer;
			outputBuffer = nullptr;
		}
	}


	/**
	\brief
	Main initializer that creates the new arrays and sets up the object. 
	
	\param _numInputChannels input channel count (1 = mono, 2 = stereo, may be greater than 2 if needed)
	\param _numOutputChannels input channel count (1 = mono, 2 = stereo, may be greater than 2 if needed)
	\param _blockSize the MAXIMUM block size that sets the buffer sizes; this can never be exceeded

	*/
	void AudioBuffer::init(uint32_t _numInputChannels, uint32_t _numOutputChannels, uint32_t _blockSize)
	{
		// --- delete if existing
		destroyInputBuffers();
		destroyOutputBuffers();

		numInputChannels = _numInputChannels;
		numOutputChannels = _numOutputChannels;
		blockSize = _blockSize;

		if (numInputChannels > 0)
			inputBuffer = new float*[numInputChannels];

		for (uint32_t i = 0; i < numInputChannels; i++)
		{
			inputBuffer[i] = new float[blockSize];
		}

		if (numOutputChannels > 0)
			outputBuffer = new float*[numOutputChannels];

		for (uint32_t i = 0; i < numOutputChannels; i++)
		{
			outputBuffer[i] = new float[blockSize];
		}

		// --- clear out
		flushBuffers();
	}

	/**
	\brief
	Fast clearing of audio buffer data; this is important because buffers are re-used all the time.

	*/
	void AudioBuffer::flushBuffers()
	{
		for (uint32_t i = 0; i < numInputChannels; i++)
		{
			memset(inputBuffer[i], 0, sizeof(float) * blockSize);
		}

		for (uint32_t i = 0; i < numOutputChannels; i++)
		{
			memset(outputBuffer[i], 0, sizeof(float) * blockSize);
		}
	}

	/**
	\brief
	Get a naked pointer to an audio INPUT buffer by channel 
	- for mono/stereo operation: channel 0 = mono or left, channel 1 = right
	- may also be used with > 2 channel operation

	\param channel index of channel to get input buffer
	*/
	float* AudioBuffer::getInputBuffer(uint32_t channel)
	{
		if (channel >= numInputChannels) return nullptr;
		return inputBuffer[channel];
	}

	/**
	\brief
	Get a naked pointer to an audio OUTPUT buffer by channel
	- for mono/stereo operation: channel 0 = mono or left, channel 1 = right
	- may also be used with > 2 channel operation

	\param channel index of channel to get input buffer
	*/
	float* AudioBuffer::getOutputBuffer(uint32_t channel)
	{
		if (channel >= numOutputChannels) return nullptr;
		return outputBuffer[channel];
	}

	/**
	\brief
	Set the number of samples in a block for processing
	- only used because the block size COULD be variable if the host DAW or plugin framework supplies
	an input or output buffer that is a partial block size; very rare, but still possible

	\param _samplesInBlock numner of samples in block
	*/
	void AudioBuffer::setSamplesInBlock(uint32_t _samplesInBlock)
	{
		samplesInBlock = _samplesInBlock;
		samplesInBlock = (uint32_t)fmin(samplesInBlock, blockSize);
	}

	// --- SynthClock -------------------------------------------------------------------------------------- //
	/**
	\brief
	Overloaded = operator for setting two clocks equal to each other by copying member values from a source

	\param params source SynthClock to copy
	*/
	SynthClock& SynthClock::operator=(const SynthClock& params)
	{
		if (this == &params)
			return *this;

		mcounter = params.mcounter;
		phaseInc = params.phaseInc;
		phaseOffset = params.phaseOffset;
		frequency_Hz = params.frequency_Hz;

		for (uint32_t i = 0; i < NUM_VARS; i++)
			state[i] = params.state[i];

		return *this;
	}

	/**
	\brief
	Initialize this clock with another SynthClock

	\param clock source SynthClock to copy
	*/
	void SynthClock::initWithClock(SynthClock& clock)
	{
		mcounter = clock.mcounter;
		phaseInc = clock.phaseInc;
		phaseOffset = clock.phaseOffset;
		frequency_Hz = clock.frequency_Hz;
	}

	/**
	\brief
	Reset to initial state

	\param startValue phase offset at start time; 
	       this value is on the range [0.0, +1.0] corresponding to 0 to 360 degrees of phase shift
	*/
	void SynthClock::reset(double startValue)
	{
		mcounter = startValue;
		phaseOffset = 0.0;
		freqOffset = 0.0;
	}

	/**
	\brief
	Advance the clock some number of tics by adding the phaseInc value

	\param renderInterval number of tics to advance the clock
	*/
	void SynthClock::advanceClock(uint32_t renderInterval)
	{
		mcounter += (renderInterval * phaseInc);
	}

	/**
	\brief
	Advance the clock some number of tics by adding the phaseInc value and then check 
	to see if modulo counter requires wrapping; wrap if needed

	\param renderInterval number of tics to advance the clock
	*/
	bool SynthClock::advanceWrapClock(uint32_t renderInterval)
	{
		mcounter += (renderInterval * phaseInc);
		return wrapClock(); // no wrap
	}

	/**
	\brief
	Wrap the modulo counter; note that this will wrap the modulo counter as many times as needed to get it
	back on the range of [0.0, +1.0] to support phase modulation values greater or less then +/- 1.0
	which happens when the index of modulation is greater than 1.0
	*/
	bool SynthClock::wrapClock()
	{
		if (inRange(0.0, 1.0, mcounter))
			return false;

		bool neg = signbit(mcounter);
		if (!neg && mcounter < 2.0)
			mcounter -= 1.0;
		else if (neg && mcounter > -1.0)
			mcounter += 1.0;
		else
			// --- wrap as many times as needed; this method is slower but takes
			//     the same amount of time no matter how far outside the range the number is
			mcounter = wrapMinMax(mcounter, 0.0, 1.0);
		
		return true;
	}

	/**
	\brief
	Set the clock frequency, which calculates the current phase increment value

	\param _frequency_Hz new clock frequency, OK if this value is negative, clock will run backwards
	\param _sampleRate fs for calculating the phase inc value
	*/
	void SynthClock::setFrequency(double _frequency_Hz, double _sampleRate)
	{
		frequency_Hz = _frequency_Hz;
		sampleRate = _sampleRate;
		phaseInc = frequency_Hz / sampleRate;
	}

	/**
	\brief
	For phase modulation, this adds a phase offset and then optionally checks/wraps the counter as needed

	\param _phaseOffset the amound of phase shift, normally on the range [-1.0, +1.0] but may be larger
	\param wrap set true to automatically check and wrapt the counter as a result of phase offset
	*/
	void SynthClock::addPhaseOffset(double _phaseOffset, bool wrap)
	{
		phaseOffset = _phaseOffset;
		if (phaseInc > 0)
			mcounter += phaseOffset;
		else
			mcounter -= phaseOffset;

		if (wrap)
			wrapClock();
	}

	/**
	\brief
	For phase modulation, this removes a phase offset, notice that the function does not attempt to wrap the counter.
	- many algorithms require checking/wrapping the counter after removal of the phase offset so you must 
	add a call to wrapClock(); after calling this method
	*/
	void SynthClock::removePhaseOffset()
	{
		if (phaseInc > 0)
			mcounter += -phaseOffset;
		else
			mcounter -= -phaseOffset;
	}

	/**
	\brief
	For frequency modulation, this adds a frequency offset, and recalculates the phase increment value

	\param _freqOffset the amound of frequency shift in Hz
	*/
	void SynthClock::addFrequencyOffset(double _freqOffset)
	{
		freqOffset = _freqOffset;
		setFrequency(frequency_Hz + freqOffset, sampleRate);
	}

	/**
	\brief
	For frequency modulation, this removes a frequency offset and recalculates the phase inc

	*/
	void SynthClock::removeFrequencyOffset()
	{
		setFrequency(frequency_Hz - freqOffset, sampleRate);
	}

	/**
	\brief
	Freeze and save the current state of the clock; for PM and FM
	*/
	void SynthClock::saveState()
	{
		state[MOD_COUNTER] = mcounter;
		state[PHASE_INC] = phaseInc;
		state[PHASE_OFFSET] = phaseOffset;
		state[FREQUENCY_HZ] = frequency_Hz;
	}

	/**
	\brief
	Restore the clock to its last saved state; for PM and FM
	*/
	void SynthClock::restoreState()
	{
		mcounter = state[MOD_COUNTER];
		phaseInc = state[PHASE_INC];
		phaseOffset = state[PHASE_OFFSET];
		frequency_Hz = state[FREQUENCY_HZ];
	}

	// --- XFader -------------------------------------------------------------------------------------- //
	/**
	\brief
	Constructs the object with initial crossfade time in samples

	\param _xfadeTime_Samples the time in samples
	*/
	XFader::XFader(uint32_t _xfadeTime_Samples)
	{
		xfadeTime_Samples = _xfadeTime_Samples;
		running = false;
	}

	/**
	\brief
	Resets object to initialized state
	*/
	void XFader::reset()
	{
		xfadeTime_Counter = 0;
		running = false;
	}

	/**
	\brief
	Set the current crossfade time
	- this may be done at any time and may stop the crossfade

	*/
	void XFader::setXFadeTime(uint32_t _xfadeTime_Samples)
	{
		xfadeTime_Samples = _xfadeTime_Samples;
	}

	/**
	\brief
	Perform crossfade FROM A to B on a pair of input values to oroduce a single output value
	- output is retured via pass-by-reference

	\param xfadeType specifies the kind of crossfade XFadeType::kConstantPower, kSquareLaw, kLinear
	\param inputA input value for the A channel that is crossfaded FROM
	\param inputB input value for the B channel that is crossfaded TO
	\param output output value returned from function

	\return TRUE if the crossfade is still going (needs more samples) or 
		    FALSE if the crossfade is finished (done)
	*/
	bool XFader::crossfade(XFadeType xfadeType, double inputA, double inputB, double& output)
	{
		if (!running) return false;

		double x = (double)xfadeTime_Counter / (double)xfadeTime_Samples;

		// --- if we just rolled over:
		if (x >= 1.0)
		{
			// --- output is all B now
			output = inputB;
			xfadeTime_Counter = 0;
			running = false;
			return running;
		}

		// --- calculate gains
		// --- constant power (same as pan calc)
		double gainA = 1.0;
		double gainB = 0.0;
		calculateConstPwrMixValues(bipolar(x), gainA, gainB);

		if (xfadeType == XFadeType::kConstantPower)
			output = inputA*gainA + inputB*gainB;
		else if (xfadeType == XFadeType::kSquareLaw)
			output = inputA*(1.0 - x*x) + inputB*(1.0 - (x - 1.0)*(x - 1.0));	// A gain = 1 - x^2, B gain = 1 - ((x-1)^2)
		else //  linear
			output = inputA*(1.0 - x) + inputB*(x);

		xfadeTime_Counter++;
		return running; // still need xfade
	}

	// --- XHoldFader -------------------------------------------------------------------------------------- //
	/**
	\brief
	Reset to initial state; just resets counters to 0
	*/
	void XHoldFader::reset()
	{
		xfadeTime_Counter = 0;
		holdTime_Counter = 0;
	}

	/**
	\brief
	Setting the hold time is slightly more complicated than the crossfade time becuase it may be done
	at any time; this requires checking and setting/clearing the holding boolean 
	*/
	void XHoldFader::setHoldTimeSamples(double _holdTimeSamples)
	{
		holdTime_Samples = _holdTimeSamples;
		if (holdTime_Samples > 0) holding = true;
		else holding = false;
	}

	/**
	\brief
	Returns the current state of the crossfade or hold
	- this function is a timer-maintenance function
	- returns information in XFadeData structure that includes:
		- linear gain values (for A and B)
		- square law gain values (for A and B)
		- constant power gain values (for A and B)
		- crossfadeFinished flag is set to true once crossfade is done, and holding is occuring
	*/
	XFadeData XHoldFader::getCrossfadeData()
	{
		XFadeData xfadeParams;
		xfadeParams.crossfadeFinished = false;

		if (holding)
		{
			if (holdTime_Counter < holdTime_Samples)
			{
				for (uint32_t i = 0; i < 2; i++)
				{
					xfadeParams.linearGain[0] = xfadeParams.constPwrGain[0] = xfadeParams.squareLawGain[0] = 1.0;
					xfadeParams.linearGain[1] = xfadeParams.constPwrGain[1] = xfadeParams.squareLawGain[1] = 0.0;
				}
				holdTime_Counter++;
				return xfadeParams; // running
			}
			if (holdTime_Counter == holdTime_Samples)
			{
				xfadeTime_Counter = 1;// 1; // this keeps sample accurate 7/19/20
				holding = false;
			}
		}

		// if no crossfade time, goto B instantly
		if (xfadeTime_Samples == 0)
		{
			// --- output is all B
			xfadeParams.linearGain[0] = xfadeParams.constPwrGain[0] = xfadeParams.squareLawGain[0] = 0.0;
			xfadeParams.linearGain[1] = xfadeParams.constPwrGain[1] = xfadeParams.squareLawGain[1] = 1.0;

			xfadeTime_Counter = 0;
			holdTime_Counter = 0;
			xfadeParams.crossfadeFinished = true;
			return xfadeParams; // done
		}

		double x = (double)xfadeTime_Counter / (double)(xfadeTime_Samples - 1);

		// --- if we just rolled over:
		if (x >= 1.0)
		{
			// --- output is all A now
			xfadeParams.linearGain[0] = xfadeParams.constPwrGain[0] = xfadeParams.squareLawGain[0] = 1.0;
			xfadeParams.linearGain[1] = xfadeParams.constPwrGain[1] = xfadeParams.squareLawGain[1] = 0.0;
			xfadeTime_Counter = 0;
			holdTime_Counter = 0;
			xfadeParams.crossfadeFinished = true;
			return xfadeParams; // done
		}

		// --- linear
		xfadeParams.linearGain[0] = 1.0 - x;
		xfadeParams.linearGain[1] = x;

		// --- square law
		xfadeParams.squareLawGain[0] = 1.0 - x*x;
		xfadeParams.squareLawGain[1] = 1.0 - (x - 1.0)*(x - 1.0);

		// --- constant power (same as pan calc)
		calculatePanValues(bipolar(x), xfadeParams.constPwrGain[0], xfadeParams.constPwrGain[1]);
	
		// --- increment
		xfadeTime_Counter++;

		return xfadeParams; // running
	}

	// --- Synchronizer -------------------------------------------------------------------------------------- //
	/**
	\brief
	Specialized reset function that:
	- resets the reset-oscillator
	- resets the crossfader
	- sets the crossfade time in samples - usually just a few (4-10) samples during smear-over
	
	\param _sampleRate fs
	\param startPhase the starting phase of the reset oscillator; used when the main oscillator is phase shifted at
	the note-on time
	\param xfadeSamples crossfade time for discontinuity
	*/
	bool Synchronizer::reset(double _sampleRate, double startPhase, int32_t xfadeSamples)
	{
		sampleRate = _sampleRate;
		hardSyncClock.reset(startPhase);

		hardSyncFader.reset();
		hardSyncFader.setXFadeTime(xfadeSamples);
		return true;
	}

	/**
	\brief
	Sets the new reset oscillator frequency in Hz

	\param hardSyncFrequency frequency in Hz of the reset oscillator
	*/
	bool Synchronizer::setHardSyncFrequency(double hardSyncFrequency)
	{
		boundValue(hardSyncFrequency, 0.0, sampleRate / 2.0);
		hardSyncClock.setFrequency(hardSyncFrequency, sampleRate);
		return true;
	}

	/**
	\brief
	Starts the hard sync operation as a result of being reset, using the main oscillator's SynthClock 
	to get the number of sub-samples to offset the clock with (see synth book). The crossfader object is then started. 

	\param oscClock  main oscillator's SynthClock
	*/
	void Synchronizer::startHardSync(SynthClock oscClock)
	{
		// --- save current location (mod counter) for crossfade
		crossFadeClock.initWithClock(hardSyncClock);

		// --- get subsamples
		double hardSyncSubSample = oscClock.mcounter / oscClock.phaseInc;

		// --- crossFadeClock
		hardSyncClock.mcounter = (hardSyncSubSample)*hardSyncClock.phaseInc;

		// --- start the xfader
		hardSyncFader.startCrossfade();
	}

	/**
	\brief
	Perform the crossfade on the two oscillator signals to smear over the discontinuity

	\param inA input A for crossfade - will be the output of the currently running oscillator
	\param inB input B for crossfade - will be the output of a shadow oscillator that has been reset
	*/
	double Synchronizer::doHardSyncXFade(double inA, double inB)
	{
		// --- do the crossfade
		double output = 0.0;
		hardSyncFader.crossfade(XFadeType::kLinear, inA, inB, output);
		return output;
	}

	/**
	\brief
	Add a phase offset to the reset clock; for supporting phase modulation

	\param offset phase offset value
	*/
	void Synchronizer::addPhaseOffset(double offset)
	{
		hardSyncClock.addPhaseOffset(offset);
	}

	/**
	\brief
	Remove existing a phase offset to the reset clock; for supporting phase modulation
	*/
	void Synchronizer::removePhaseOffset()
	{
		hardSyncClock.removePhaseOffset();
		hardSyncClock.wrapClock();
	}

	// --- RampModulator -------------------------------------------------------------------------------------- //

	/**
	\brief
	Setup the timers and start the ramp modulator running. The modulator produces an output on the range of
	[startValue, endValue] over a specified duration in mSec

	\param startValue starting modulator value
	\param endValue ending modulator value
	\param modTime_mSec time for the ramp-up or ramp-down across the values
	\param sampleRate fs for calculating timer increment
	*/
	bool RampModulator::startModulator(double startValue, double endValue, double modTime_mSec, double sampleRate)
	{
		if (startValue == endValue || modTime_mSec <= 0.0)
		{
			timerActive = false;
			return false;
		}
		modStart = startValue;
		modEnd = endValue;

		modRange = endValue - startValue;
		countUpTimer = 0.0;
		timerInc = (1000.0 / modTime_mSec) / sampleRate;
		timerActive = true;
		return true;
	}

	/**
	\brief
	Change the modulation time; this is optional 

	\param modTime_mSec time for the ramp-up or ramp-down across the values
	\param sampleRate fs for calculating timer increment
	*/
	bool RampModulator::setModTime(double modTime_mSec, double sampleRate)
	{
		timerInc = (1000.0 / modTime_mSec) / sampleRate;
		return true;
	}

	/**
	\brief
	Get the current output of the modulator; this is called repeatedly over the ramp modulation time
	- check the modulator to see when it is done, then stop calling this function

	\param advanceClock number of tics to advance the clock for the next value

	*/
	double RampModulator::getNextModulationValue(uint32_t advanceClock)
	{
		double output = 0.0;
		if (timerActive)
		{
			output = countUpTimer*modRange;
			if (modRange >= 0)
				output += modStart;
			else 
				output = modEnd + output;

			countUpTimer += advanceClock * timerInc;
			if (countUpTimer >= 1.0)
				timerActive = false;
		}

		return output;
	}

	/**
	\brief
	Nudge the clock on the modulator; this is used for block processing where the modulator output outputs 
	one value per audio block. This is called repeatedly to get the modulator clock "caught up" to where it
	needs to be for the *next* block of audio to process.

	\param ticks number of tics to advance the clock for the next value

	*/
	void RampModulator::advanceClock(uint32_t ticks)
	{
		if (timerActive)
		{
			countUpTimer += (ticks * timerInc);
			if (countUpTimer >= 1.0)
				timerActive = false;
		}
	}



	// --- GlideModulator -------------------------------------------------------------------------------------- //

	/**
	\brief
	Setup the timers and start the ramp modulator running. The modulator produces an output on the range of
	[startValue, endValue] over a specified duration in mSec

	\param startValue starting MIDI note number
	\param endValue ending MIDI note number
	\param glideTime_mSec time for the glide-up or glide-down across the values
	\param sampleRate fs for calculating timer increment
	*/
	bool GlideModulator::startModulator(double startValue, double endValue, double glideTime_mSec, double sampleRate)
	{
		if (startValue == endValue || glideTime_mSec <= 0.0)
		{
			timerActive = false;
			return false;
		}

		glideRange = startValue - endValue;
		countDownTimer = 1.0;
		timerInc = (1000.0 / glideTime_mSec) / sampleRate;
		timerActive = true;
		return true;
	}

	/**
	\brief
	Change the glide time; this is optional

	\param glideTime_mSec time for the ramp-up or ramp-down across the values
	\param sampleRate fs for calculating timer increment
	*/
	bool GlideModulator::setGlideTime(double glideTime_mSec, double sampleRate)
	{
		timerInc = (1000.0 / glideTime_mSec) / sampleRate;
		return true;
	}

	/**
	\brief
	Get the current output of the modulator; this is called repeatedly over the ramp modulation time
	- check the modulator to see when it is done, then stop calling this function

	\param advanceClock number of tics to advance the clock for the next value

	*/
	double GlideModulator::getNextModulationValue(uint32_t advanceClock)
	{
		double output = 0.0;
		if (timerActive)
		{
			output = countDownTimer*glideRange;
			countDownTimer -= advanceClock * timerInc;
			if (countDownTimer <= 0.0)
				timerActive = false;
		}
		return output;
	}

	/**
	\brief
	Nudge the clock on the modulator; this is used for block processing where the modulator output outputs
	one value per audio block. This is called repeatedly to get the modulator clock "caught up" to where it
	needs to be for the *next* block of audio to process.

	\param ticks number of tics to advance the clock for the next value

	*/
	void GlideModulator::advanceClock(uint32_t ticks)
	{
		if (timerActive)
		{
			countDownTimer -= (ticks * timerInc);
			if (countDownTimer <= 0.0)
				timerActive = false;
		}
	}



	// NoiseGenerator --------------------------------------------------------------------- //

	/**
	\brief
	Function generate gaussian white noise

	\param mean mean value of gaussian distribution
	\param variance of gaussian distribution

	See also: https://github.com/divisionby-0/A.W.G.N./blob/master/AWGN.h
	*/
	double NoiseGenerator::doGaussianWhiteNoise(double mean, double variance)
	{
		std::normal_distribution<double> normalDistribution(mean, variance);
		return normalDistribution(defaultGeneratorEngine);
	}

	/**
	\brief
	Function generate white noise
	*/
	double NoiseGenerator::doWhiteNoise()
	{
		// --- changed to this faster version; the book version is commented out below
		//     https://www.musicdsp.org/en/latest/Synthesis/216-fast-whitenoise-generator.html
		g_x1 ^= g_x2;
		float output = g_x2 * g_fScale;
		g_x2 += g_x1;
		return (double)output;

		//std::uniform_real_distribution<double> randomDisribution(-1.0, 1.0);
		//return randomDisribution(defaultGeneratorEngine);
	}

	/**
	\brief
	Function generate pink noise by filtering white noise
	*/
	double NoiseGenerator::doPinkNoise()
	{
		double output = doPinkingFilter(doWhiteNoise());
		output *= 0.25; // scalar to reduce output amplitude which swings above 1
		return output;
	}

	/**
	\brief
	run the pinking filter

	\param white the white noise input sample

	\return the pink-noise filtered signal
	*/
	double NoiseGenerator::doPinkingFilter(double white)
	{
		// --- Pink noise filter
		bN[0] = 0.99765 * bN[0] + white * 0.0990460;
		bN[1] = 0.96300 * bN[1] + white * 0.2965164;
		bN[2] = 0.57000 * bN[2] + white * 1.0526913;
		return bN[0] + bN[1] + bN[2] + white * 0.1848;
	}

	// --- SynthProcessInfo -------------------------------------------------------------------------------------- //
	/**
	\brief
	The normal constructor for the SynthProcessInfo object
	- the arguments setup the underlying AudioBuffer
	- nothing else to do

	\param _numInputChannels number of input channels, OK to be 0 (for synths)
	\param _numOutputChannels number of outout channels, must not be 0
	\param _blockSize the maximum block size for rendering the synth audio
	*/
	SynthProcessInfo::SynthProcessInfo(uint32_t _numInputChannels, uint32_t _numOutputChannels, uint32_t _blockSize)
		: AudioBuffer(_numInputChannels, _numOutputChannels, _blockSize)
	{
		// --- base constructor does the work
	}

	/**
	\brief
	Add a MIDI event to the queue
	- call this once per MIDI event per audio buffer, at the top of the block render cycle
	- these will be decoded and transmitted by the voice object prior to rendering data from the components

	\param event MIDI event to push onto the stack
	*/
	void SynthProcessInfo::pushMidiEvent(midiEvent event)
	{
		midiEventQueue.push_back(event);
	}

	/**
	\brief
	Clear the queue
	- called after the messages are decoded and used. 
	*/
	void SynthProcessInfo::clearMidiEvents()
	{
		midiEventQueue.clear();
	}

	/**
	\return the count of events in the queue; used by voice for iterating over messages
	*/
	uint64_t SynthProcessInfo::getMidiEventCount()
	{
		return midiEventQueue.size();
	}

	/**
	\brief
	gets a MIDI event within the event queue

	\param index location within the vector of the event

	\return a pointer to the event, or nullptr if not found
	*/
	midiEvent* SynthProcessInfo::getMidiEvent(uint32_t index)
	{
		if (index >= getMidiEventCount())
			return nullptr;

		return &midiEventQueue[index];
	}

	// WavetableDatabase --------------------------------------------------------------------- //
	/**
	\brief
	clear out the table sources
	*/
	WavetableDatabase::~WavetableDatabase()
	{
		clearTableSources();
	}

	/**
	\brief
	selects a table source based on the unique table name

	\param uniqueTableName name of the table set, usually the same as the waveform string the user sees

	\return a pointer to the IWavetableSource to be used for reading table data
	*/
	IWavetableSource* WavetableDatabase::getTableSource(const char* uniqueTableName)
	{
		if (!uniqueTableName)
			return nullptr;

		if (uniqueTableName == empty_string.c_str() || strlen(uniqueTableName) <= 0)
			return nullptr;

		IWavetableSource* source = nullptr;
		std::string name(uniqueTableName);
		wavetableSourceMap::iterator it = wavetableDatabase.find(name);
		if (it != wavetableDatabase.end())
		{
			source = it->second;
		}

		return source;
	}

	IWavetableSource* WavetableDatabase::getTableSource(uint32_t uniqueTableIndex)
	{
		if (uniqueTableIndex > wavetableVector.size()-1) return nullptr;
		return wavetableVector[uniqueTableIndex];
	}

	/**
	\brief
	add a table source to the database

	\param uniqueTableName name of the table set, usually the same as the waveform string the user sees
	\param tableSource IWavetableSource* to add

	\return true if sucessful
	*/
	bool WavetableDatabase::addTableSource(const char* uniqueTableName, IWavetableSource* tableSource, uint32_t& uniqueIndex)
	{
		if (!uniqueTableName || !tableSource)
			return false;

		if (uniqueTableName == empty_string.c_str() || strlen(uniqueTableName) <= 0)
			return false;

		if (getTableSource(uniqueTableName))
			return false;

		// --- map for controlID-indexing
		std::string name(uniqueTableName);
		wavetableDatabase.insert(std::make_pair(name, tableSource));

		// --- vector for faster lookup
		wavetableVector.push_back(tableSource);
		uniqueIndex = (uint32_t)(wavetableVector.size() - 1);

		return true;
	}

	/**
	\brief
	remove a table source from the database

	\param uniqueTableName name of the table set, usually the same as the waveform string the user sees

	\return true if sucessful
	*/
	bool WavetableDatabase::removeTableSource(const char* uniqueTableName)
	{
		if (!getTableSource(uniqueTableName))
			return false;

		std::string name(uniqueTableName);
		wavetableDatabase.erase(name);
		

		//vec.erase(vec.begin() + index);
		return true;
	}

	/**
	\brief
	clear all entries from the std::map
	- does not delete or destroy anything
	\return true if sucessful
	*/
	bool WavetableDatabase::clearTableSources()
	{
		wavetableDatabase.clear();
		wavetableVector.clear();
		return true;
	}

	/**
	\brief
	get the vector index of a waveform, use at reset() or startup, not runtime
	- does not delete or destroy anything
	\return index of waveform or -1 if not found
	*/
	int32_t WavetableDatabase::getWaveformIndex(const char* uniqueTableName)
	{
		IWavetableSource* source = getTableSource(uniqueTableName);
		if (!source) return -1;

		size_t size = wavetableVector.size();
		for (uint32_t i = 0; i < size; i++) 
		{
			if (wavetableVector[i] == source)
				return i;
		}
		return -1;
	}

	// SampleDatabase --------------------------------------------------------------------- //
	/**
	\brief
	clear out the table sources
	*/
	PCMSampleDatabase::~PCMSampleDatabase()
	{
		clearSampleSources();
	}

	/**
	\brief
	selects a PCM sample source based on the unique table name

	\param uniqueSampleSetName name of the sample set, usually the same as the folder 
	that holds the WAV files for the samples

	\return a pointer to the IPCMSampleSource to be used for reading table data
	*/
	IPCMSampleSource* PCMSampleDatabase::getSampleSource(const char* uniqueSampleSetName)
	{
		if (!uniqueSampleSetName)
			return nullptr;

		if (uniqueSampleSetName == empty_string.c_str() || strlen(uniqueSampleSetName) <= 0)
			return nullptr;

		IPCMSampleSource* source = nullptr;

		std::string name(uniqueSampleSetName);
		sampleSourceMap::iterator it = sampleDatabase.find(name);
		if (it != sampleDatabase.end())
		{
			source = it->second;
		}

		return source;
	}

	/**
	\brief
	add a PCM sample source to the database

	\param uniqueSampleSetName name of the PCM sample set, usually the same as the folder that holds the WAV samples
	\param sampleSource IPCMSampleSource* to add

	\return true if sucessful
	*/
	bool PCMSampleDatabase::addSampleSource(const char* uniqueSampleSetName, IPCMSampleSource* sampleSource)
	{
		if (!uniqueSampleSetName || !sampleSource)
			return false;

		if (getSampleSource(uniqueSampleSetName))
			return false;

		if (sampleSource->getValidSampleCount() <= 0)
			return false;

		// --- map for controlID-indexing
		std::string name(uniqueSampleSetName);
		sampleDatabase.insert(std::make_pair(name, sampleSource));
		sources.push_back(sampleSource);

		return true;
	}

	/**
	\brief
	remove a PCM sample source from the database

	\param uniqueSampleSetName name of the PCM sample set, usually the same as the folder that holds the WAV samples

	\return true if sucessful
	*/
	bool PCMSampleDatabase::removeSampleSource(const char* uniqueSampleSetName)
	{
		if (!getSampleSource(uniqueSampleSetName))
			return false;

		// --- map for controlID-indexing
		sampleSourceMap::iterator it = sampleDatabase.find(uniqueSampleSetName);
		IPCMSampleSource* set = it->second;
		if (!set) return false;

		// --- find and remove
		std::vector<IPCMSampleSource*>::iterator toErease = std::find(sources.begin(), sources.end(), set);
		sampleDatabase.erase(it);              // erasing by iterator
		sources.erase(toErease);

		return true;
	}

	/**
	\brief
	clear all entries from the std::map
	- this DOES call the sample deleter function on the source as part of the destruction of dynamic data

	\return true if sucessful
	*/
	bool PCMSampleDatabase::clearSampleSources()
	{
		for (uint32_t i = 0; i < sources.size(); i++)
		{
			sources[i]->deleteSamples();
		}

		sampleDatabase.clear();
		sources.clear();

		return true;
	}


	// --- MidiInputData -------------------------------------------------------------------------------------- //
	/**
	\brief
	clears out all data arrays
	*/
	MidiInputData::MidiInputData()
	{
		memset(&globalMIDIData[0], 0, sizeof(uint32_t)*kNumMIDIGlobals);
		memset(&ccMIDIData[0], 0, sizeof(uint32_t)*kNumMIDICCs);
		memset(&auxData[0], 0, sizeof(double)*kNumMIDIAuxes);
	}

	/**
	\brief
	get a global MIDI data value
	- example: uint32_t lsb = processInfo.midiInputData->getGlobalMIDIData(kMIDIPitchBendDataLSB);

	\param index in global MIDI input array

	\return the MIDI value as uint32_t
	*/
	uint32_t MidiInputData::getGlobalMIDIData(uint32_t index)
	{
		if (index <kNumMIDIGlobals)
			return globalMIDIData[index];
		return 0;
	}

	/**
	\brief
	get a CC MIDI data value
	- example: midiVolumeGain = midiInputData->getCCMIDIData(VOLUME_CC07);

	\param index in CC MIDI input array

	\return the MIDI value as uint32_t
	*/
	uint32_t MidiInputData::getCCMIDIData(uint32_t index)
	{
		if (index <kNumMIDICCs)
			return ccMIDIData[index];
		return 0;
	}

	/**
	\brief
	get aux data value
	- returns value as uint32_t
	- may or may not be "real" MIDI data
	- may contain DAW parameters such as BPM

	\param index in AUX MIDI input array

	\return the aux value as uint32_t
	*/
	uint32_t MidiInputData::getAuxDAWDataUINT(uint32_t index)
	{
		if (index <kNumMIDIAuxes)
			return auxData[index];
		return 0;
	}

	/**
	\brief
	get aux data value
	- returns value as float
	- may or may not be "real" MIDI data
	- may contain DAW parameters such as BPM

	\param index in AUX MIDI input array

	\return the aux value as float
	*/
	float MidiInputData::getAuxDAWDataFloat(uint32_t index)
	{
		if (index <kNumMIDIAuxes)
			return uint32ToFloat(auxData[index]);
		return 0.0;
	}

	/**
	\brief
	set the global MIDI value

	\param index in Global MIDI input array
	\param value MIDI value to set
	*/
	void MidiInputData::setGlobalMIDIData(uint32_t index, uint32_t value)
	{
		if (index <kNumMIDIGlobals) globalMIDIData[index] = value;
	}

	/**
	\brief
	set the CC MIDI value

	\param index in CC MIDI input array
	\param value MIDI value to set
	*/
	void MidiInputData::setCCMIDIData(uint32_t index, uint32_t value)
	{
		if (index <kNumMIDICCs) ccMIDIData[index] = value;
	}

	/**
	\brief
	set the aux data with a uint32_t

	\param index in CC MIDI input array
	\param value UINT value to set
	*/
	void MidiInputData::setAuxDAWDataUINT(uint32_t index, uint32_t value)
	{
		if (index <kNumMIDIAuxes) auxData[index] = value;
	}

	/**
	\brief
	set the aux data with a float
	- this uses a special function to store the floating point as a uint32_t without casting; does not mangle bits

	\param index in CC MIDI input array
	\param value float value to set
	*/
	void MidiInputData::setAuxDAWDataFloat(uint32_t index, float value)
	{
		uint32_t valueU = floatToUint32(value);
		if (index <kNumMIDIAuxes) auxData[index] = valueU;
	}


	// --- Modulators -------------------------------------------------------------------------------------- //

	/**
	\brief
	constructs the modulator object
	- initializes input value to "proper" settings for a note-on event

	*/
	Modulators::Modulators()
	{
		initInputValues();
	}

	/**
	\brief
	fast clearing of modulator array
	*/
	void Modulators::clear()
	{
		memset(modArray, 0, MAX_MODULATION_CHANNELS * sizeof(double));
	}

	/**
	\brief
	set default values in modulator array
	- initializes input value to "proper" settings for a note-on event
	*/
	void Modulators::initInputValues()
	{
		clear();
		modArray[kEGMod] = 1.0;
		modArray[kAmpMod] = 1.0;
		modArray[kMaxDownAmpMod] = 1.0;
	}

	/**
	\brief
	get a pointer to a slot in the modulation array

	\param index the index of the slot in the array 
	*/
	double* Modulators::getModArrayPtr(uint32_t index)
	{
		if (index < MAX_MODULATION_CHANNELS)
			return &modArray[index];
		return nullptr;
	}

	/**
	\brief
	get a value from a slot in the modulation array

	\param index the index of the slot in the array
	*/
	double Modulators::getModValue(uint32_t index)
	{
		if (index < MAX_MODULATION_CHANNELS)
			return modArray[index];

		return 0.0;
	}

	/**
	\brief
	set a value into a slot in the modulation array

	\param index the index of the slot in the array
	*/
	void Modulators::setModValue(uint32_t index, double value)
	{
		if (index < MAX_MODULATION_CHANNELS)
			modArray[index] = value;
	}
	
	// --- SynthModule -------------------------------------------------------------------------------------- //
	/**
	\brief
	Constructs a SynthModule
	- creats the MIDI input data if not present, for stand-alone operation

	\param _midiInputData MIDI input data interface; if nullptr, then object
	synthesizes its own
	*/
	SynthModule::SynthModule(std::shared_ptr<MidiInputData> _midiInputData)
		: midiInputData(_midiInputData)
	{
		// --- standalone operation
		if (!midiInputData)
			midiInputData.reset(new (MidiInputData));

		// --- initialize to non-zero values for volume and pan
		initMIDIInputData(midiInputData);

		glideModulator.reset(new(GlideModulator));
		clearModuleCores();
	}

	/**
	\brief
	Removes cores, if any
	*/
	SynthModule::~SynthModule()
	{
		clearModuleCores();
	}

	/**
	\brief
	Gets a std::vector of Module Strings
	- Each Module has up to 16 module strings to expose to the user
	- Used in impementations that have dynamically loadable string controls on the GUI

	\param moduleStrings vector pf strings retured via pass-by-reference
	\param ignoreStr a string to ignore when filling the vector (e.g. "empty string")

	\return true if strings were found, false otherwise
	*/
	bool SynthModule::getModuleStrings(std::vector<std::string>& moduleStrings, std::string ignoreStr)
	{
		if (selectedCore)
		{
			ModuleCoreData data = selectedCore->getModuleData();
			moduleStrings = charArrayToStringVector(data.moduleStrings, MODULE_STRINGS, ignoreStr);
			return true;
		}

		// --- use module data instead, for modules with no cores
		moduleStrings = charArrayToStringVector(moduleData.moduleStrings, MODULE_STRINGS, ignoreStr);
		return true;
	}

	/**
	\brief
	Gets a std::vector of Module Strings from a particular core
	- Each Module or Core has up to 16 module strings to expose to the user
	- Used in impementations that have dynamically loadable string controls on the GUI

	\param coreIndex index of core to query for its strings
	\param moduleStrings vector pf strings retured via pass-by-reference
	\param ignoreStr a string to ignore when filling the vector (e.g. "empty string")
	\return true if strings were found, false otherwise
	*/
	bool SynthModule::getModuleStrings(uint32_t coreIndex, std::vector<std::string>& moduleStrings, std::string ignoreStr)
	{
		if (coreIndex > NUM_MODULE_CORES - 1) return false;

		if (moduleCores[coreIndex])
		{
			ModuleCoreData data = moduleCores[coreIndex]->getModuleData();
			moduleStrings = charArrayToStringVector(data.moduleStrings, MODULE_STRINGS, ignoreStr);
			return true;
		}

		// --- use local versions for modules with no cores
		moduleStrings = charArrayToStringVector(moduleData.moduleStrings, MODULE_STRINGS, ignoreStr);
		return true;
	}

	/**
	\brief
	Gets a std::vector of all Module Strings concatenated from all cores in succession
	- used to generate the waveform GUI lists for the WaveSequencer project
	- merges all core module strings together

	\param moduleStrings vector pf strings retured via pass-by-reference
	\param ignoreStr a string to ignore when filling the vector (e.g. "empty string")
	\return true if strings were found, false otherwise
	*/
	bool SynthModule::getAllModuleStrings(std::vector<std::string>& moduleStrings, std::string ignoreStr)
	{
		bool foundStrings = false;
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			if (moduleCores[i])
			{
				ModuleCoreData data = moduleCores[i]->getModuleData();
				appendCharArrayToStringVector(data.moduleStrings, MODULE_STRINGS, moduleStrings, ignoreStr);
				foundStrings = true;
			}
		}

		// --- use local version for modules with no cores
		if(!foundStrings)
			moduleStrings = charArrayToStringVector(moduleData.moduleStrings, MODULE_STRINGS, ignoreStr);

		return true;
	}

	/**
	\brief
	Gets a std::vector of Mod Knob label strings for the selected core
	- Each Module or Core has up to 4 Mod Knob label strings strings to expose to the user
	- Used in impementations that have dynamically loadable string labels for mod knobs

	\param modKnobStrings vector pf strings retured via pass-by-reference

	\return true if strings were found, false otherwise
	*/
	bool SynthModule::getModKnobStrings(std::vector<std::string>& modKnobStrings)
	{
		if (selectedCore)
		{
			ModuleCoreData data = selectedCore->getModuleData();
			modKnobStrings = charArrayToStringVector(data.modKnobStrings, MOD_KNOBS);
			return true;
		}

		// --- get local version for modules with no cores
		modKnobStrings = charArrayToStringVector(moduleData.modKnobStrings, MOD_KNOBS);

		return true;
	}


	/**
	\brief
	Gets a std::vector of Mod Knob label strings for a given core; if the core cannot be found
	returs the mod knob labels of the module itself
	- Each Module or Core has up to 4 Mod Knob label strings strings to expose to the user
	- Used in impementations that have dynamically loadable string labels for mod knobs

	\param modKnobStrings vector pf strings retured via pass-by-reference

	\return true if strings were found, false otherwise
	*/
	bool SynthModule::getModKnobStrings(uint32_t coreIndex, std::vector<std::string>& modKnobStrings)
	{
		if (coreIndex > NUM_MODULE_CORES - 1) return false;

		if (moduleCores[coreIndex])
		{
			ModuleCoreData data = moduleCores[coreIndex]->getModuleData();
			modKnobStrings = charArrayToStringVector(data.modKnobStrings, MOD_KNOBS);
		}

		// --- use local versions for modules with no cores
		modKnobStrings = charArrayToStringVector(moduleData.modKnobStrings, MOD_KNOBS);

		return true;
	}

	/**
	\brief
	Gets a std::vector of the names of the four cores in this module
	- Used in impementations that have dynamically loadable GUI controls for listing the cores

	\param moduleCoreStrings vector pf strings retured via pass-by-reference

	\return true if strings were found, false otherwise
	*/
	bool SynthModule::getModuleCoreStrings(std::vector<std::string>& moduleCoreStrings)
	{
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			std::shared_ptr<ModuleCore> core = moduleCores[i];
			if (core)
				moduleCoreStrings.push_back(core->getModuleName());
		}
		return false;
	}

	/**
	\brief
	starts the built-in glide modulator 

	\return true if sucessful
	*/
	bool SynthModule::startGlideModulation(GlideInfo& glideInfo)
	{
		// --- start up the glide modulator
		if (selectedCore)
			return selectedCore->startGlideModulation(glideInfo);

		return glideModulator->startModulator(glideInfo.startMIDINote, glideInfo.endMIDINote, glideInfo.glideTime_mSec, glideInfo.sampleRate);
	}

	/**
	\brief
	adds a module core to the module's set of four
	- for adding cores to a module, either statically or dynamically

	\return true if sucessful
	*/
	bool SynthModule::addModuleCore(std::shared_ptr<ModuleCore> core)
	{
		// --- try preferred location first, dynamic cores
		//     statically loaded cores follow the order you set in the SynthModule constructor
		int32_t preferredLoadIndex = core->getPreferredModuleIndex();

		if (preferredLoadIndex >= 0 && preferredLoadIndex < NUM_MODULE_CORES)
		{
			// --- if not occupied take this slot
			if (!moduleCores[preferredLoadIndex])
			{
				moduleCores[preferredLoadIndex] = core;
				core->setModuleIndex(preferredLoadIndex);
				return true;
			}
		}

		// --- keep looking
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			if (!moduleCores[i])
			{
				moduleCores[i] = core;
				core->setModuleIndex(i);
				return true;
			}
		}
		return false;
	}


	/**
	\brief
	get the index of the selected core

	\return index of core
	*/
	uint32_t SynthModule::getSelectedCoreIndex()
	{
		if (selectedCore) return selectedCore->getModuleIndex();
		return 0; // default core
	}

	/**
	\brief
	Select a core

	\param index index of core to select
	\return true if found a core to select
	*/
	bool SynthModule::selectModuleCore(uint32_t index)
	{
		if (moduleCores[index])
		{
			selectedCore = moduleCores[index];
			return true;
		}
		return false;
	}

	/**
	\brief
	Select the default core, which is always the first in the list

	\return true if found a core to select
	*/
	bool SynthModule::selectDefaultModuleCore()
	{
		if (moduleCores[DEFAULT_CORE])
		{
			selectedCore = moduleCores[DEFAULT_CORE];
			return true;
		}
		return false;
	}


	/**
	\brief
	packs the cores into non-null ordering
	*/
	void SynthModule::packCores()
	{
		for (uint32_t core = 0; core < NUM_MODULE_CORES; core++)
		{
			if (!moduleCores[core])
			{
				for (uint32_t i = core+1; i < NUM_MODULE_CORES; i++)
				{
					if (moduleCores[i])
					{
						moduleCores[core] = moduleCores[i];
						moduleCores[core]->setModuleIndex(core);
						moduleCores[i] = nullptr;
						break;
					}
				}
			}
		}
	}


	/**
	\brief
	Sets the stand-alone mode flag on all cores

	\param alone true if stand-alone mode, false otherwise
	*/
	void SynthModule::setStandAloneMode(bool alone)
	{
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			if (moduleCores[i])
				moduleCores[i]->setStandAloneMode(alone);
		}
	}


	/**
	\brief
	Clears out the module core pointer list
	- does not delete or destroy the core

	\return true if sucessful
	*/
	bool SynthModule::clearModuleCores()
	{
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			moduleCores[i] = nullptr;
		}
		return true;
	}




} // namespace


