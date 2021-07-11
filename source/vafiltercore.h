#pragma once

#include "synthbase.h"
#include "synthfunctions.h"
#include "limiter.h"
#include "vafilters.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   vafiltercore.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class VAFilterCore
	\ingroup ModuleCores
	\brief
	Implements all of the virtual analog (VA) filters in the Synth Book using sub-filtering objects
	- breaks filters into families (1st order, SVF, Korg, Moog, Diode) 
	and generates ALL outputs for that family at once
	
	Demonstrates use of following VAFilter objects:
	- VA1Filter first order filters
	- VASVFilter state variable filters
	- VAKorg35Filter Korg35 filters
	- VAMoogFilter Moog filters
	- VADiodeFilter Diode ladder filter (VCS3)

	Base Class: ModuleCore
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.
	- NOTE: These functions have identical names as the SynthModules that own them,
	however the arguments are different. ModuleCores use the CoreProcData structure
	for passing arguments into the cores because they are thunk-barrier compliant.
	- This means that the owning SynthModule must prepare this structure and populate it prior to
	function calls. The large majority of this preparation is done in the SynthModule constructor
	and is one-time in nature.

	GUI Parameters: FilterParameters
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
	- No_Filter, LPF1, HPF1, APF1, SVF_LP, SVF_HP, SVF_BP, SVF_BS, Korg35_LP, Korg35_HP,
	  Moog_LP1, Moog_LP2, Moog_LP3, Moog_LP4, Diode_LP4

	ModKnob Strings, for fixed GUI controls by index constant
	- MOD_KNOB_A = "Key Track"
	- MOD_KNOB_B = "Drive"
	- MOD_KNOB_C = "EG Int"
	- MOD_KNOB_D = "BP Int"

	Render:
	- renders into the output buffer using pointers in the CoreProcData argument to the render function
	- processes one block of audio input into one block of audio output per render cycle
	- processes in mono that is copied to the right channel as dual-mono stereo

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class VAFilterCore : public ModuleCore
	{
	public:
		/** simple default constructor */
		VAFilterCore();				/* C-TOR */
	
		/** Destructor is empty: all resources are smart pointers */
		virtual ~VAFilterCore() {}		/* D-TOR */

		/** ModuleCore Overrides */
		virtual bool reset(CoreProcData& processInfo) override;
		virtual bool update(CoreProcData& processInfo) override;
		virtual bool render(CoreProcData& processInfo) override;
		virtual bool doNoteOn(CoreProcData& processInfo) override;
		virtual bool doNoteOff(CoreProcData& processInfo) override;

	protected:
		// --- our member filters
		VA1Filter va1[STEREO];			///< 1st order VA
		VASVFilter svf[STEREO];			///< SVF
		VAKorg35Filter korg35[STEREO];	///< Korg35
		VAMoogFilter moog[STEREO];		///< moog
		VADiodeFilter diode[STEREO];	///< diode

		// --- global filter type
		FilterModel selectedModel = FilterModel::kFirstOrder;
		uint32_t outputIndex = 0;		///< selected output
		double outputAmp = 1.0;			///< filter output amplitude, tweked from GUI in dB
		bool forceDualMonoFilters = false; ///< DM option for slow machines

		// --- output limiter
		Limiter limiter[STEREO];		///< limiters to squelch oscillations

		// --- for key track
		double midiPitch = 440.0;		///< key tracking
	};



} // namespace

