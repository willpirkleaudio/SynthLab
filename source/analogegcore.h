#pragma once

#include "synthbase.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   analogcore.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class AnalogEGCore
	\ingroup ModuleCores
	\brief
	Analog EG emulator
	
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
	- ADSR, AR

	ModKnob Strings, for fixed GUI controls by index constant 
	- MOD_KNOB_A = "Start Lvl"
	- MOD_KNOB_B = "B"
	- MOD_KNOB_C = "C"
	- MOD_KNOB_D = "D"

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
	class AnalogEGCore : public ModuleCore
	{
	public:
		/** simple default constructor */
		AnalogEGCore();					/* C-TOR */
		
		/** Destructor is empty: all resources are smart pointers */
		virtual ~AnalogEGCore() {}		/* D-TOR */

		/** ModuleCore Overrides */
		virtual bool reset(CoreProcData& processInfo) override;
		virtual bool update(CoreProcData& processInfo) override;
		virtual bool render(CoreProcData& processInfo) override;
		virtual bool doNoteOn(CoreProcData& processInfo) override;
		virtual bool doNoteOff(CoreProcData& processInfo) override;

		/** ModuleCore Overrides for EG Cores only */
		virtual int32_t getState() override { return enumToInt(state); }
		virtual bool shutdown() override;
		virtual void setSustainOverride(bool b) override;

	protected:
		bool noteOff = false;		///< for retriggering EG
		bool retriggered = false;	///< for retriggering EG
		double lastTriggerMod = 0.0;///< for retriggering EG trigger detection

		// --- index of contour
		double sustainLevel = 1.0;		///< level, not time
		double attackTime_mSec = 1.0;	///< att: is a time duration
		double decayTime_mSec = 1.0;	///< dcy: is a time to decay from max output to 0.0
		double releaseTime_mSec = 1.0;	///< rel: is a time to decay from max output to 0.0

		/** Calculate Time Coefficients */
		inline void calcAttackCoeff(double attackTime, double attackTimeScalar = 1.0)
		{
			// --- store for comparing so don't need to waste cycles on updates
			attackTime_mSec = attackTime;

			// --- samples for the exponential rate
			double samples = sampleRate*((attackTime_mSec*attackTimeScalar) / 1000.0);

			// --- coeff and base for iterative exponential calculation
			attackCoeff = exp(-log((1.0 + attackTCO) / attackTCO) / samples);
			attackOffset = (1.0 + attackTCO)*(1.0 - attackCoeff);
		}

		/** Calculate Time Coefficients */
		inline void calcDecayCoeff(double decayTime, double decayTimeScalar = 1.0)
		{
			// --- store for comparing so don't need to waste cycles on updates
			decayTime_mSec = decayTime;

			// --- samples for the exponential rate
			double samples = sampleRate*((decayTime_mSec*decayTimeScalar) / 1000.0);

			// --- coeff and base for iterative exponential calculation
			decayCoeff = exp(-log((1.0 + decayTCO) / decayTCO) / samples);
			decayOffset = (sustainLevel - decayTCO)*(1.0 - decayCoeff);
		}

		/** Calculate Time Coefficients */
		void calcReleaseCoeff(double releaseTime, double releaseTimeScalar = 1.0)
		{
			// --- store for comparing so don't need to waste cycles on updates
			releaseTime_mSec = releaseTime;

			// --- samples for the exponential rate
			double samples = sampleRate*((releaseTime_mSec*releaseTimeScalar) / 1000.0);

			// --- coeff and base for iterative exponential calculation
			releaseCoeff = exp(-log((1.0 + releaseTCO) / releaseTCO) / samples);
			releaseOffset = -releaseTCO*(1.0 - releaseCoeff);
		}

		// --- sample rate for time calculations
		double sampleRate = 0.0;			///< sample rate

		// --- the current output of the EG
		double envelopeOutput = 0.0;		///< the current envelope output sample


		//--- Coefficient, offset and TCO values
		//    for each state
		double attackCoeff = 0.0;	///< exponential feedback coefficient
		double attackOffset = 0.0;	///< TCO offset to allow proper attack/decay on [1, 0]
		double attackTCO = 0.0;		///< TCO value for calculating offset

		double decayCoeff = 0.0;	///< exponential feedback coefficient
		double decayOffset = 0.0;	///< TCO offset to allow proper attack/decay on [1, 0]
		double decayTCO = 0.0;		///< TCO value for calculating offset

		double releaseCoeff = 0.0;	///< exponential feedback coefficient
		double releaseOffset = 0.0;	///< TCO offset to allow proper attack/decay on [1, 0]
		double releaseTCO = 0.0;	///< TCO value for calculating offset

		// --- for sustain pedal input; these two bools keep track of the override, 
		//     and eventual release when the sus pedal is released after user released key
		bool sustainOverride = false;		///< if true, places the EG into sustain mode
		bool releasePending = false;		///< a flag set when a note off event occurs while the sustain pedal is held, telling the EG to go to the release state once the pedal is released

		// --- inc value for shutdown
		double incShutdown = 0.0;			///< shutdown linear incrementer

		// --- stage variable
		EGState state = EGState::kOff;		///< EG state variable
	};



} // namespace

