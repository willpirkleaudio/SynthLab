#ifndef __rezonator_h__
#define __rezonator_h__

#include "synthbase.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   resonator.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class Resonator
	\ingroup SynthObjects
	\brief
	Special purpose object for use as Karplus Strong resonator
	- implements single delay-line version of KS algorithm
	- initial state of delay is EMPTY and the exciter feeds the 
	delay free path directly

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class Resonator
	{
	public:
		Resonator() {}			///< empty constructor
		virtual ~Resonator() {}	///< empty constructor

	public:
		/** Similar functions as SynthModule */
		bool reset(double _sampleRate);
		double process(double xn);
		double setParameters(double frequency, double _decay);
		
		// --- flush out delay
		void flushDelays();

	protected:
		// --- sample rate
		double sampleRate = 0.0;			///< sample rate	
		double decay = 0.0;					///< feedback coefficient controls rate	
		DelayLine delayLine;				///< delay line for KD
		FracDelayAPF fracDelayAPF;			///< APF for fractional daley
		ResLoopFilter loopFilter;			///< 1st order 1/2 sample delay LPF
	};


} // namespace
#endif /* defined(__rezonator_h__) */
