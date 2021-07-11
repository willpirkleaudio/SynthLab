#pragma once

#include "synthbase.h"
#include "synthconstants.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   limiter.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class SimpleLPF
	\ingroup SynthObjects
	\brief
	Encapsulates a tiny first order, all-pole LPF object

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class SimpleLPF
	{
	public:
		SimpleLPF(void) {}	/* C-TOR */
		~SimpleLPF(void) {}	/* D-TOR */

	public:
		/** reset members to initialized state */
		virtual bool reset(double _sampleRate)
		{
			state = 0.0;
			return true;
		}

		/** set one and only g-coefficient */
		/**
		\param _lpf_g the b1 coefficient
		*/
		void setLPF_g(double _lpf_g) { lpf_g = _lpf_g; }

		/** process simple one pole FB back filter */
		/**
		\param xn input
		\return the processed sample
		*/
		virtual double processAudioSample(double xn)
		{
			double yn = (1.0 - lpf_g)*xn + lpf_g*state;
			state = yn;
			return yn;
		}

	private:
		double lpf_g = 0.8;	///< g coefficient
		double state = 0.0;	///< single state (z^-1) register
	};

	/**
	\class LogPeakDetector
	\ingroup SynthObjects
	\brief
	Encapsulates McNally's peak detector with added LPF smoothing (optional)
	- output is in dB
	- can detect values greater than 0dBFS

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class LogPeakDetector
	{
	public:
		LogPeakDetector(void) {}	/* C-TOR */
		~LogPeakDetector(void) {}	/* D-TOR */

	public:
		/** \brief
		reset members to initialized state */
		virtual bool reset(double _sampleRate)
		{
			// --- store the sample rate
			sampleRate = _sampleRate;
			samplePeriod_T = 1.0 / sampleRate;

			// --- storage to hold peak value
			peakStore = 0.0;

			// --- setup simple one-pole LPF to smooth detector output
			lpf.reset(sampleRate);

			return true;
		}

		/**
		\brief process MONO input 
		\param xn input
		\return the processed sample
		*/
		virtual double processAudioSample(double xn)
		{
			// --- all modes do Full Wave Rectification
			double input = fabs(xn);

			// --- output variable is output of peak storage
			double rn = peakStore;

			// --- we are recursive so need to check underflow
			checkFloatUnderflow(rn);

			// --- can not be (-)
			rn = fmax(rn, 0.0);

			// --- setup inputs to the peak store register
			double attackValue = 0.0;
			double releaseValue = 0.0;

			// --- form difference
			double diff = input - peakStore;
			if (diff > 0.0)
				attackValue = attackTime;

			// --- attack branch accumultes with peak store
			attackValue += peakStore;

			// --- release branch multiplies with output
			releaseValue = rn * releaseTime;

			// --- sum of branches
			peakStore = attackValue - releaseValue;

			// --- filtering to smooth response
			double yn = lpf.processAudioSample(rn);

			// --- setup for log( )
			if (yn <= 0)
				return -96.0; // should only get called for string of 0's

			// --- true log output in dB, can go above 0dBFS!
			return 20.0 * log10(yn);
		}

		/** 
		\brief
		set attack/release times
		\param attack_in_ms attack in milliseconds
		\param release_in_ms release in milliseconds
		*/
		inline void setAttackReleaseTimes(double attack_in_ms, double release_in_ms)
		{
			// --- cook parameters here
			setAttackTime(attack_in_ms);
			setReleaseTime(release_in_ms);
		}

		const double kSmallestPositiveFloatValue = 1.175494351e-38;         /* min positive value */
		const double kSmallestNegativeFloatValue = -1.175494351e-38;         /* min negative value */

		/**
		\brief 
		set check for underflow
		*/
		inline bool checkFloatUnderflow(double& value)
		{
			bool retValue = false;
			if (value > 0.0 && value < kSmallestPositiveFloatValue)
			{
				value = 0;
				retValue = true;
			}
			else if (value < 0.0 && value > kSmallestNegativeFloatValue)
			{
				value = 0;
				retValue = true;
			}
			return retValue;
		}
	private:
		SimpleLPF lpf;				///< smoohter lpg
		double attackTime = 0.0;	///< attack time coefficient
		double releaseTime = 0.0;	///< release time coefficient
		double sampleRate = 44100;	///< stored sample rate
		double samplePeriod_T = 1.0;	///< stored period
		double lastEnvelope = 0.0;	///< last output
		double peakStore = 0.0;		///< peak store (see McNally reference)

		/** \brief
		set our internal atack time coefficients based on times and sample rate */
		void setAttackTime(double attack_in_ms)
		{
			// --- calculate 10% - 90% risetime
			attackTime = 1.0 - exp((-2.2*samplePeriod_T) / (attack_in_ms * 0.001));
		}

		/**  \brief
		set our internal release time coefficients based on times and sample rate */
		void setReleaseTime(double release_in_ms)
		{
			// --- calculate 10% - 90% falltime
			releaseTime = 1.0 - exp((-2.2*samplePeriod_T) / (release_in_ms * 0.001));
		}
	};


	/**
	\class LinPeakDetector
	\ingroup SynthObjects
	\brief
	Encapsulates McNally's peak detector with added LPF smoothing (optional)
	- output is in raw (linear, non dB) form
	- can detect values greater than 1.0

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class LinPeakDetector
	{
	public:
		LinPeakDetector(void) {}	/* C-TOR */
		~LinPeakDetector(void) {}	/* D-TOR */

	public:
		/** \brief
		reset members to initialized state */
		virtual bool reset(double _sampleRate)
		{
			// --- store the sample rate
			sampleRate = _sampleRate;
			samplePeriod_T = 1.0 / sampleRate;

			// --- storage to hold peak value
			peakStore = 0.0;

			// --- setup simple one-pole LPF to smooth detector output
			lpf.reset(sampleRate);

			return true;
		}

		/**
		\brief process MONO input
		\param xn input
		\return the processed sample
		*/
		virtual double processAudioSample(double xn)
		{
			// --- all modes do Full Wave Rectification
			double input = fabs(xn);

			// --- output variable is output of peak storage
			double rn = peakStore;

			// --- we are recursive so need to check underflow
			checkFloatUnderflow(rn);

			// --- can not be (-)
			rn = fmax(rn, 0.0);

			// --- setup inputs to the peak store register
			double attackValue = 0.0;
			double releaseValue = 0.0;

			// --- form difference
			double diff = input - peakStore;
			if (diff > 0.0)
				attackValue = attackTime;

			// --- attack branch accumultes with peak store
			attackValue += peakStore;

			// --- release branch multiplies with output
			releaseValue = rn * releaseTime;

			// --- sum of branches
			peakStore = attackValue - releaseValue;

			// --- filtering to smooth response
			double yn = lpf.processAudioSample(rn);

			return yn;
		}

		/**
		\brief
		set attack/release times
		\param attack_in_ms attack in milliseconds
		\param release_in_ms release in milliseconds
		*/
		inline void setAttackReleaseTimes(double attack_in_ms, double release_in_ms)
		{
			// --- cook parameters here
			setAttackTime(attack_in_ms);
			setReleaseTime(release_in_ms);
		}

		const double kSmallestPositiveFloatValue = 1.175494351e-38;         /* min positive value */
		const double kSmallestNegativeFloatValue = -1.175494351e-38;         /* min negative value */

																			 /**
																			 \brief
																			 set check for underflow
																			 */
		inline bool checkFloatUnderflow(double& value)
		{
			bool retValue = false;
			if (value > 0.0 && value < kSmallestPositiveFloatValue)
			{
				value = 0;
				retValue = true;
			}
			else if (value < 0.0 && value > kSmallestNegativeFloatValue)
			{
				value = 0;
				retValue = true;
			}
			return retValue;
		}
	private:
		SimpleLPF lpf;				///< smoohter lpg
		double attackTime = 0.0;	///< attack time coefficient
		double releaseTime = 0.0;	///< release time coefficient
		double sampleRate = 44100;	///< stored sample rate
		double samplePeriod_T = 1.0;	///< stored period
		double lastEnvelope = 0.0;	///< last output
		double peakStore = 0.0;		///< peak store (see McNally reference)

									/** \brief
									set our internal atack time coefficients based on times and sample rate */
		void setAttackTime(double attack_in_ms)
		{
			// --- calculate 10% - 90% risetime
			attackTime = 1.0 - exp((-2.2*samplePeriod_T) / (attack_in_ms * 0.001));
		}

		/**  \brief
		set our internal release time coefficients based on times and sample rate */
		void setReleaseTime(double release_in_ms)
		{
			// --- calculate 10% - 90% falltime
			releaseTime = 1.0 - exp((-2.2*samplePeriod_T) / (release_in_ms * 0.001));
		}
	};

	// --- 

	/**
	\class Limiter
	\ingroup SynthObjects
	\brief
	Implements a custom peak limiter designed especially for self oscillating filters 
	whose outputs are > 0dBFS

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class Limiter
	{
	public:
		Limiter() {}
		~Limiter() {}

		/**  \brief
		reset internal detector */
		void reset(double _sampleRate)
		{
			// --- reset detector (always first)
			linDetector.reset(_sampleRate);
			logDetector.reset(_sampleRate);

			// --- init;
			linDetector.setAttackReleaseTimes(
				10.0,	/* attack time mSec*/
				200.0	/* release time mSec*/);
			logDetector.setAttackReleaseTimes(
				10.0,	/* attack time mSec*/
				200.0	/* release time mSec*/);
		}

		/**  \brief
		set threshold in dB */
		void setThreshold_dB(double _threshold_dB) { threshold_dB = _threshold_dB; }

		/**  \brief
		set threshold in dB */
		void setThreshold(double _threshold) { threshold = _threshold; }

		/**  \brief
		calculate limiter gain using dB values for threshold and detection

		\param fDetectorValue detected value in dB
		\param fThreshold threshold of limiting in dB
		*/
		double calcLimiterGaindB(float fDetectorValue, float fThreshold)
		{
			// --- slope variable // limiting is infinite ratio thus CS->1.0
			//                       you can play with this for compression CS < 1.0
			double CS = 1.0; // 1.0;

			// --- compute gain; threshold and detection values are in dB
			double yG = CS*(fThreshold - fDetectorValue);  // [Eq. 13.1]

			// --- clamp; this allows ratios of 1:1 to still operate
			yG = fmin(0.0, yG);

			// --- convert back to linear
			return pow(10.0, yG / 20.0);
		}

		/**  \brief
		calculate limiter gain using dB values for threshold and detection

		\param fDetectorValue detected value in dB
		\param fThreshold threshold of limiting in dB
		*/
		double calcLimiterGain(float fDetectorValue, float fThreshold)
		{
			// --- slope variable // limiting is infinite ratio thus CS->1.0
			//                       you can play with this for compression CS < 1.0
			double CS = 1.0; // 1.0;

			// --- compute gain; threshold and detection values are in raw form
			double yG = (fThreshold - fDetectorValue);  // [Eq. 13.1]

			// --- clamp; this allows ratios of 1:1 to still operate
			yG = yG > 0.0 ? 1.0 : CS*(fThreshold / fDetectorValue);

			// --- already linear
			return yG;
		}

		/**  \brief
		process one sample through limiter

		\return limited output value
		*/
		double process(double input)
		{
			//double detectedValue_dB = logDetector.processAudioSample(input);
			//double gain = calcLimiterGaindB(detectedValue_dB, threshold_dB);
			double detectedValue = linDetector.processAudioSample(input);
			double gain = calcLimiterGain(detectedValue, threshold);
			return input*gain;
		}

	protected:
		LogPeakDetector logDetector;	///< the peak detector
		double threshold_dB = -1.5; ///< hardcoded threshold in dB
		
		LinPeakDetector linDetector;	///< the peak detector
		double threshold = 0.8413;  ///< hardcoded threshold for -1.5dB
	};

} // namespace