#ifndef __exciter_h__
#define __exciter_h__

#include "synthbase.h"
#include "synthfunctions.h"
#include "vafilters.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   exciter.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class ExciterEG
	\ingroup SynthObjects
	\brief
	Special purpose EG just for the Karplus Strong exciter. 
	- does not use a base class or interface
	- custom designed for use with KS algorithm

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class ExciterEG
	{
	public:
		ExciterEG() {}				///< empty constructor
		virtual ~ExciterEG() {}		///< empty destructor

	public:
		///< reset/render (simialar to SynthModule)
		bool reset(double _sampleRate);
		double render();

		///< start the FSM
		bool startEG();

		///< param setter
		void setParameters(double _attackTime_mSec, double _holdTime_mSec, double _releaseTime_mSec);

	protected:
		double attackTime_mSec = -1.0;	///< att: is a time duration
		double releaseTime_mSec = -1.0;	///< rel: is a time to decay from max output to 0.0
		double holdTime_mSec = -1.0;	///< rel: is a time to decay from max output to 0.0

		// --- calculate time params
		void calcAttackCoeff(double attackTime, double attackTimeScalar = 1.0);
		void calcReleaseCoeff(double releaseTime, double releaseTimeScalar = 1.0);

		//--- Coefficient, offset and TCO values
		//    for each state
		double attackCoeff = 0.0;		///< see AnalogEG
		double attackOffset = 0.0;		///< see AnalogEG
		double attackTCO = 0.0;			///< see AnalogEG

		double releaseCoeff = 0.0;		///< see AnalogEG
		double releaseOffset = 0.0;		///< see AnalogEG
		double releaseTCO = 0.0;		///< see AnalogEG

		double sampleRate = 0.0;		///< sample rate
		// --- the current output of the EG
		double envelopeOutput = 0.0;	///< the current envelope output sample

		// --- stage variable
		EGState state = EGState::kOff;		///< EG state variable

		// --- timer for hold portion
		Timer holdTimer;				///< holding timer
	
		// --- RUN/STOP flag
		bool noteOn = false;			///< run/stop flag
	};

	/**
	\class Exciter
	\ingroup SynthObjects
	\brief
	Special purpose object for use as Karplus Strong exciter
	- does not use a base class or interface
	- custom designed for use with KS algorithm

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class Exciter
	{
	public:
		Exciter() {}			///< empty constructor
		virtual ~Exciter() {}	///< empty constructor

	public:
		/** Similar functions as SynthModule, only simpler */
		bool reset(double _sampleRate);
		double render(double coupledInput = 0.0);
		void startExciter();
		void setParameters(double attackTime_mSec, double holdTime_mSec, double releaseTime_mSec);
	
	protected:
		NoiseGenerator noiseGen;	///< noise maker
		ExciterEG noiseEG;			///< EG to shape the noise
		DCRemovalFilter dcFilter;	///< DC removal for short term random bias
	};


} // namespace
#endif /* defined(__exciter_h__) */
