// --- includes
#include "resonator.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   resonator.cpp
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
	Reset resonator for another run

	\param _sampleRate new sample rate

	\returns true if successful
	*/
	bool Resonator::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;

		// --- reset with lowest f0 = 8.176 Hz (MIDI 0)
		delayLine.reset(_sampleRate, MIDI_NOTE_0_FREQ);
		loopFilter.reset();
		fracDelayAPF.reset();

		return true;
	}

	/**
	\brief
	flush delay lines, and clear filter state registers.
	*/
	void Resonator::flushDelays()
	{
		delayLine.clear();
		loopFilter.reset();
		fracDelayAPF.reset();
	}

	/**
	\brief
	set the resonator frequency and decay time

	\param frequency current frequency of the resonator
	\param _decay current decay value (from GUI most likely)

	\return the delay time in samples (fractional)
	*/
	double Resonator::setParameters(double frequency, double _decay)
	{
		// --- store for render
		decay = _decay;

		// --- calculate delay time for this pitch
		double delayTime = sampleRate / frequency;

		// --- the tube + LPF = L + 1/2 samples, so back calculate to get the delay length
		double delayLength = delayTime - 0.5;

		// --- now take integer portion
		uint32_t intDelayLen = (uint32_t)delayLength;

		// --- this guarantees that apfDelta will be betwen [0.0, 1.0] or fractional delay
		double apfDelta = delayTime - ((double)intDelayLen + 0.5);

		// --- calculate normalized frequency in Hz
		double omega_0 = kTwoPi*frequency / sampleRate;
		double omega_0_half = omega_0 / 2.0;

		// --- calcuate APF coefficients using desired fractional delay, apfDelta
		double alpha = sin((1.0 - apfDelta)*omega_0_half) / sin((1.0 + apfDelta)*omega_0_half);

		// --- delay is -1 because of the way the CircularBuffer works, expecting read/write
		delayLine.setDelayInSamples(intDelayLen - 1);
		//delayLine.setDelayInSamples(intDelayLen);

		// --- set APF for fractional delay
		fracDelayAPF.setAlpha(alpha);

		return delayTime;
	}

	/**
	\brief
	Process an exciter signal (or 0.0) through the resonator

	\param xn input sample

	\return output sample for resonator
	*/
	double Resonator::process(double xn)
	{
		// --- read delay
		double delayOut = delayLine.readDelay();

		// --- filter input + delay output
		double filterOut = loopFilter.processAudioSample(xn + delayOut);

		// --- create fractional delay with APF
		double yn = fracDelayAPF.processAudioSample(filterOut);

		// --- write the value into the delay and scale
		delayLine.writeDelay(yn*decay);

		// --- done
		return yn;
	}

} // namespace
