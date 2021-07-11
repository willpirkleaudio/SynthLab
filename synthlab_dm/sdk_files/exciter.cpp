// --- includes
#include "exciter.h"


// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   exciter.cpp
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
	Reset Exciter for another run

	\param _sampleRate new sample rate

	\returns true if successful
	*/
	bool ExciterEG::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		holdTimer.resetTimer();

		// --- analog time constants
		attackTCO = exp(-1.5);
		releaseTCO = exp(-4.95);

		// --- recalc these, SR dependent
		calcAttackCoeff(attackTime_mSec);
		calcReleaseCoeff(releaseTime_mSec);

		// --- reset the state
		envelopeOutput = 0.0;
		state = EGState::kOff;

		return true;
	}

	/**
	\brief
	Set attack, hold and relesase times
	- calculates coefficients only if parameters have changed

	\param _attackTime_mSec attack time in mSec
	\param _holdTime_mSec hold time in mSec
	\param _releaseTime_mSec release time in mSec
	*/
	void  ExciterEG::setParameters(double _attackTime_mSec, double _holdTime_mSec, double _releaseTime_mSec)
	{
		if (attackTime_mSec != _attackTime_mSec)
			calcAttackCoeff(_attackTime_mSec);

		if (releaseTime_mSec != _releaseTime_mSec)
			calcReleaseCoeff(_releaseTime_mSec);

		if (holdTime_mSec != _holdTime_mSec)
		{
			holdTimer.setExpireMilliSec(_holdTime_mSec, sampleRate);
			holdTime_mSec = _holdTime_mSec;
		}
	}

	/**
	\brief
	Calculate analg EG time constants

	\param attackTime attack time in mSec
	\param attackTimeScalar optional scalar to speed up or slow down attack
	*/
	void ExciterEG::calcAttackCoeff(double attackTime, double attackTimeScalar)
	{
		// --- store for comparing so don't need to waste cycles on updates
		attackTime_mSec = attackTime;

		// --- samples for the exponential rate
		double samples = sampleRate*((attackTime_mSec*attackTimeScalar) / 1000.0);

		// --- coeff and base for iterative exponential calculation
		attackCoeff = exp(-log((1.0 + attackTCO) / attackTCO) / samples);
		attackOffset = (1.0 + attackTCO)*(1.0 - attackCoeff);
	}
		
	/**
	\brief
	Calculate analg EG time constants

	\param releaseTime release time in mSec
	\param releaseTimeScalar optional scalar to speed up or slow down release
	*/
	void ExciterEG::calcReleaseCoeff(double releaseTime, double releaseTimeScalar)
	{
		// --- store for comparing so don't need to waste cycles on updates
		releaseTime_mSec = releaseTime;

		// --- samples for the exponential rate
		double samples = sampleRate*((releaseTime_mSec*releaseTimeScalar) / 1000.0);

		// --- coeff and base for iterative exponential calculation
		releaseCoeff = exp(-log((1.0 + releaseTCO) / releaseTCO) / samples);
		releaseOffset = -releaseTCO*(1.0 - releaseCoeff);
	}

	/**
	\brief
	Start the FSM

	\return true if successfule
	*/
	bool ExciterEG::startEG()
	{
		// --- reset the state
		envelopeOutput = 0.0;
		state = EGState::kAttack;
		holdTimer.resetTimer();

		return true;
	}

	/**
	\brief
	Run the FSM and calculate the new envelope out

	\return new envelope value
	*/
	double ExciterEG::render()
	{
		// --- decode the state
		switch (state)
		{
			case EGState::kOff:
			{
				// --- output is OFF
				envelopeOutput = 0.0;
				break;
			}

			case EGState::kAttack:
			{
				// --- render value
				envelopeOutput = attackOffset + envelopeOutput*attackCoeff;

				// --- check go to next state
				if (envelopeOutput >= 1.0 || attackTime_mSec <= 0.0)
				{
					envelopeOutput = 1.0;
					if (holdTime_mSec > 0.0)
						state = EGState::kHold;	// go to HOLD
					else
						state = EGState::kRelease;	// go to relase

					break;
				}
				break;
			}

			case EGState::kHold:
			{
				envelopeOutput = 1.0;	// --- hold value

				holdTimer.advanceTimer();
				if (holdTimer.timerExpired())
					state = EGState::kRelease;	// go to relase

				break;
			}

			case EGState::kRelease:
			{
				// --- render value
				envelopeOutput = releaseOffset + envelopeOutput*releaseCoeff;

				// --- check go to next state
				if (envelopeOutput <= 0.0 || releaseTime_mSec <= 0.0)
				{
					envelopeOutput = 0.0;
					state = EGState::kOff;			// go to OFF state
					break;
				}
				break;
			}
            case EGState::kDelay:
            case EGState::kDecay:
            case EGState::kSlope:
            case EGState::kSustain:
            case EGState::kShutdown:
                break;

		}
		return envelopeOutput;
	}

	/**
	\brief
	Reset the exciter iternal components. 

	\return true if successfule
	*/
	bool Exciter::reset(double _sampleRate)
	{
		noiseEG.reset(_sampleRate);
		dcFilter.reset(_sampleRate);
		return true;
	}

	/**
	\brief
	Render the exciter:
	- generate noise
	- shape this noise with the EG
	- remove DC component

	\return shaped noise blast
	*/
	double Exciter::render(double coupledInput)
	{
		double noise = noiseGen.doWhiteNoise();
		double eg = noiseEG.render();
		double ahr = (noise * eg) + coupledInput;
		return dcFilter.processAudioSample(ahr);
	}

	/**
	\brief
	Start the exciter:
	- init/start the noise maker

	*/
	void Exciter::startExciter()
	{
		noiseEG.startEG();
	}

	/**
	\brief
	Set the attack/hold/release time on the noiseEG object

	\param attackTime_mSec attack in mSec
	\param holdTime_mSec hold in mSec
	\param releaseTime_mSec release in mSec
	*/
	void Exciter::setParameters(double attackTime_mSec, double holdTime_mSec, double releaseTime_mSec)
	{
		noiseEG.setParameters(attackTime_mSec, holdTime_mSec, releaseTime_mSec);
	}


} // namespace
