#pragma once

#include "synthbase.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   bqfiltercore.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class BQFilterCore
	\ingroup ModuleCores
	\brief
	Implements Filters via BiQuad structures; includes one pole HPF and LPF

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
	- No_Filter, _1PLPF, _1PHPF, BQ_LPF2, BQ_HPF2

	ModKnob Strings, for fixed GUI controls by index constant
	- MOD_KNOB_A = "Key Track"
	- MOD_KNOB_B = "Drive"
	- MOD_KNOB_C = "EG Int"
	- MOD_KNOB_D = "BP Int"

	Render:
	- processes all audio samples in block
	- processes from input buffer to output buffer using pointers in the CoreProcData argument

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class BQFilterCore : public ModuleCore
	{
	public:
		/** simple default constructor */
		BQFilterCore();				/* C-TOR */
		
		/** Destructor is empty: all resources are smart pointers */
		virtual ~BQFilterCore() {}		/* D-TOR */

		/** ModuleCore Overrides */
		virtual bool reset(CoreProcData& processInfo) override;
		virtual bool update(CoreProcData& processInfo) override;
		virtual bool render(CoreProcData& processInfo) override;
		virtual bool doNoteOn(CoreProcData& processInfo) override;
		virtual bool doNoteOff(CoreProcData& processInfo) override;
		
		/** flush biquad delays on reset and new note events */
		void flushDelays()
		{
			for (uint32_t i = 0; i < STEREO_CHANNELS; i++)
				filter[i].flushDelays();
		}

	protected:
		BQAudioFilter filter[STEREO_CHANNELS];	///< biquad audio filter objects
		enum { a0, a1, a2, b1, b2, c0, d0 };	///< coefficients
		double sampleRate = 1.0;				///< fs
		double outputAmp = 1.0;					///< output scaling
		double midiPitch = 440.0;				///< midi note pitch
	};



} // namespace

