#pragma once

#include "synthbase.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   dxegcore.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class DXEGCore
	\ingroup ModuleCores
	\brief
	Yamaha DX EG-based

	Base Class: ModuleCore
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.
	- NOTE: These functions have identical names as the SynthModules that own them,
	however the arguments are different. ModuleCores use the CoreProcData structure
	for passing arguments into the cores because they are thunk-barrier compliant.
	- This means that the owning SynthModule must prepare this structure and populate it prior to
	function calls. The large majority of this preparation is done in the SynthModule constructor
	and is one-time in nature.

	GUI Parameters: EGParameters
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
	- ADSlSR, ADSlR

	ModKnob Strings, for fixed GUI controls by index constant
	- MOD_KNOB_A = "Start Lvl"
	- MOD_KNOB_B = "Decay Lvl"
	- MOD_KNOB_C = "Slope Time"
	- MOD_KNOB_D = "Curvature"

	Render:
	- renders into the modulation output array that is passed into the function via the
	CoreProcData structure and populates the arrays with index values of:
	- kEGNormalOutput normal EG output value
	- kEGBiasedOutput biased EG (sustain level subtracted out)
	- designed to operate in a block-fasion, knowing that only the first output sample
	will be used

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class DXEGCore : public ModuleCore
	{
	public:
		/** simple default constructor */
		DXEGCore();				/* C-TOR */

		/** Destructor is empty: all resources are smart pointers */
		virtual ~DXEGCore() {}		/* D-TOR */

		/** ModuleCore Overrides */
		virtual bool reset(CoreProcData& processInfo) override;
		virtual bool update(CoreProcData& processInfo) override;
		virtual bool render(CoreProcData& processInfo) override;
		virtual bool doNoteOn(CoreProcData& processInfo) override;
		virtual bool doNoteOff(CoreProcData& processInfo) override;

		/** ModuleCore Overrides for EG Cores only */
		virtual int32_t getState() override { return enumToInt(state); }
		virtual bool shutdown() override;
		virtual void setSustainOverride(bool b)override;

	protected:
		/**  calculate new step size */
		inline double setStepInc(double timeMsec, double scale = 1.0);

		/**  local variables */
		double egStepInc = 0.0;			///< linear time stepping
		double attackTimeScalar = 0.0;	///< for MIDI modulations to attack time
		double decayTimeScalar = 0.0;	///< for MIDI modulations to decay time
		EGState state = EGState::kOff;	///< state variable for FSM
		double envelopeOutput = 0.0;	///< current outupt
		double linearEnvOutput = 0.0;	///< current outupt
		double curveEnvOutput = 0.0;	///< current outupt
		double dxOutput = 0.0;			///< final output value
		double releaseLevel = 0.0;		///< release end point
		double sampleRate = 0.0;		///< fs

		bool sustainOverride = false;		///< if true, places the EG into sustain mode
		bool releasePending = false;		///< a flag set when a note off event occurs while the sustain pedal is held, telling the EG to go to the release state once the pedal is released
		bool resetToZero = false;			///< notes the EG is in reset-to-zero mode
		bool noteOff = false;				///< not currently used
		bool retriggered = false;			///< for retrigger modulation
		double lastTriggerMod = 0.0;		///< for detecting rising edge of modulator

		// --- inc value for shutdown
		double incShutdown = 0.0;			///< shutdown linear incrementer
	};
} // namespace

