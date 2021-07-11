#pragma once

#include "synthbase.h"
#include "basiclookuptables.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   lfocore.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class LFOCore
	\ingroup ModuleCores
	\brief
	LFO that renders all classical and many noisy wavforms
	- demonstrates use of BasicLookupTables
	- demonstrates use of NoiseGenerator
	- demonstrates use of RampModulator for fade-in
	- demonstrates use of Timers for sample and hold and simple time delay

	Base Class: ModuleCore
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.
	- NOTE: These functions have identical names as the SynthModules that own them,
	however the arguments are different. ModuleCores use the CoreProcData structure
	for passing arguments into the cores because they are thunk-barrier compliant.
	- This means that the owning SynthModule must prepare this structure and populate it prior to
	function calls. The large majority of this preparation is done in the SynthModule constructor
	and is one-time in nature.

	GUI Parameters: LFOParameters
	- GUI parameters are delivered into the core via the thunk-barrier compliant CoreProcData
	argument that is passed into each function identically
	- processInfo.moduleParameters contains a void* version of the GUI parameter structure pointer
	- the Core function casts the GUI parameter pointer prior to usage

	Access to Modulators is done via the thunk-barrier compliant CoreProcData argument
	- processInfo.modulationInputs
	- processInfo.modulationOutputs

	Access to audio buffers (I/O/FM) is done via the thunk-barrier compliant CoreProcData argument
	- processInfo.inputBuffers
	- processInfo.outputBuffers
	- processInfo.fmBuffers

	Construction: Cores follow the same construction pattern
	- set the Module type and name parameters
	- expose the 16 module strings
	- expose the 4 mod knob label strings
	- intialize any internal variables

	Standalone Mode:
	- These objects are designed to be internal members of the outer SynthModule that owns them.
	They may be used in standalone mode without modification, and you will use the CoreProcData
	structure to pass information into the functions.

	Module Strings, zero-indexed for your GUI Control:
	- triangle, sine ,ramp_up, ramp_dn, exp_up, exp_dn, exp_tri, square, rand_SH1, pluck

	ModKnob Strings, for fixed GUI controls by index constant
	- MOD_KNOB_A = "Shape"
	- MOD_KNOB_B = "Delay"
	- MOD_KNOB_C = "FadeIn"
	- MOD_KNOB_D = "BPM Sync"

	Render:
	- renders into the modulation output array that is passed into the function via the
	CoreProcData structure and populates the arrays with index values of:
	- kLFONormalOutput normal LFO output value
	- kLFOInvertedOutput 180 degrees out of phase with normal output
	- kUnipolarFromMax unipolar version of the waveform that sits at +1 when the output amplitude is at 0,
	and descends from +1 downward as the output amplitude is increased; used for tremolo
	- kUnipolarFromMin unipolar version of the waveform that sits at 0 when the output amplitude is at 0,
	and ascends from 0 upward as the output amplitude is increased; the ordinary unipolar version
	- designed to operate in a block-fasion, knowing that only the first output sample
	will be used

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class LFOCore : public ModuleCore
	{
	public:
		/** simple default constructor */
		LFOCore();				/* C-TOR */
		
		/** Destructor is empty: all resources are smart pointers */
		virtual ~LFOCore() {}	/* D-TOR */

		/** ModuleCore Overrides */
		virtual bool reset(CoreProcData& processInfo) override;
		virtual bool update(CoreProcData& processInfo) override;
		virtual bool render(CoreProcData& processInfo) override;
		virtual bool doNoteOn(CoreProcData& processInfo) override;
		virtual bool doNoteOff(CoreProcData& processInfo) override;

	protected:
		std::unique_ptr<BasicLookupTables> lookupTables = nullptr; ///< LUTs for some waveforms

		// --- sample rate
		double sampleRate = 0.0;			///< sample rate
		double outputValue = 0.0;			///< current output,
		double rshOutputValue = 0.0;		///< current output,

		// --- timebase
		SynthClock lfoClock;				///< timbase

		// --- for one shot renders
		bool renderComplete = false;		///< flag for one-shot

		// --- for noisy LFOs
		NoiseGenerator noiseGen;			///< for noise based LFOs

		// --- timer for RSH
		Timer sampleHoldTimer;				///< for sample and hold waveforms

		// --- timer for delay
		Timer delayTimer;					///< LFO turn on delay

		// --- ramp mod for fade-in
		RampModulator fadeInModulator;		///< LFO fade-in modulator
	};

} // namespace

