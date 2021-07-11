#pragma once

#include "synthbase.h"
#include "synthfunctions.h"
#include "sinetablesource.h"
#include "dx_eg.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   fmocore.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class FMOCore
	\ingroup ModuleCores
	\brief
	Sinusoidal-only oscillator and attached EG
	- you may add more waveforms to this object to completely change the DX synth!
	- includes a standalone DXEG object for FMOperator 
	- demonstrates use of SineTableSource object

	Base Class: ModuleCore
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.
	- NOTE: These functions have identical names as the SynthModules that own them,
	however the arguments are different. ModuleCores use the CoreProcData structure
	for passing arguments into the cores because they are thunk-barrier compliant.
	- This means that the owning SynthModule must prepare this structure and populate it prior to
	function calls. The large majority of this preparation is done in the SynthModule constructor
	and is one-time in nature.

	GUI Parameters: FMOperatorParameters
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
	- sinewave

	ModKnob Strings, for fixed GUI controls by index constant
	- MOD_KNOB_A = "A"
	- MOD_KNOB_B = "B"
	- MOD_KNOB_C = "C"
	- MOD_KNOB_D = "F.Back"

	Render:
	- renders into the output buffer using pointers in the CoreProcData argument to the render function
	- renders one block of audio per render cycle
	- renders in mono that is copied to the right channel as dual-mono stereo

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class FMOCore : public ModuleCore
	{
	public:
		/** simple default constructor */
		FMOCore();				/* C-TOR */
		
		/** Destructor is empty: all resources are smart pointers */
		virtual ~FMOCore() {}	/* D-TOR */
								
		/** ModuleCore Overrides */
		virtual bool reset(CoreProcData& processInfo) override;
		virtual bool update(CoreProcData& processInfo) override;
		virtual bool render(CoreProcData& processInfo) override;
		virtual bool doNoteOn(CoreProcData& processInfo) override;
		virtual bool doNoteOff(CoreProcData& processInfo) override;

	protected:
		// --- ;ocal variables
		double sampleRate = 0.0;		///< sample rate
		double midiPitch = 0.0;			///< the midi pitch
		double outputAmplitude = 1.0;	///< amplitude in dB
		double panLeftGain = 0.707;		///< left channel gain
		double panRightGain = 0.707;	///< right channel gain
		double outputValue = 0.0;		///< last output value needed for self Feedback
	
		// --- timebase
		SynthClock oscClock;	///< timebase

		// --- table source
		SineTableSource sineTableSource; ///< a single sinewave table

		// --- EG object and parameters
		// 
		std::unique_ptr<DXEG> dxEG = nullptr; ///< an EG for FM modulation
		std::shared_ptr<EGParameters> dxEGParameters = nullptr;///< EG parameters for controlling the EG
	};

} // namespace

