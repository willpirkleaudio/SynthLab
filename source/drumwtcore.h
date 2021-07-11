#pragma once

#include "synthbase.h"
#include "synthfunctions.h"
#include "synthlabwtsource.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   drumwtcore.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class DrumWTCore
	\ingroup ModuleCores
	\brief
	Wavetables that implement electronic drum samples; these samples were extracted from WAV files
	and converted into wavetables using RackAFX-TableMaker

	This is one of the simplest wavetable oscillator cores because the samples are pitchless
	and there is no pitch modulation applied. Pitch modulation addition is an excellent
	homework chore. 

	Base Class: ModuleCore
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.
	- NOTE: These functions have identical names as the SynthModules that own them,
	however the arguments are different. ModuleCores use the CoreProcData structure
	for passing arguments into the cores because they are thunk-barrier compliant.
	- This means that the owning SynthModule must prepare this structure and populate it prior to
	function calls. The large majority of this preparation is done in the SynthModule constructor
	and is one-time in nature.

	GUI Parameters: WTOscParameters
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
	- Kick_1, Kick_2, Kick_3, Snare_1, Snare_2, Snare_3, Closed_HH, Open_HH, Tom_1,
	  Tom_2, Tom_3, Tom_4, Tom_5, Crash_1, Crash_2, Crash_3

	ModKnob Strings, for fixed GUI controls by index constant
	- MOD_KNOB_A = "A"
	- MOD_KNOB_B = "B"
	- MOD_KNOB_C = "C"
	- MOD_KNOB_D = "D"

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
	class DrumWTCore : public ModuleCore
	{
	public:
		/** simple default constructor */
		DrumWTCore();				/* C-TOR */
		
		/** Destructor is empty: all resources are smart pointers */
		virtual ~DrumWTCore() {}		/* D-TOR */

		/** ModuleCore Overrides */
		virtual bool reset(CoreProcData& processInfo) override;
		virtual bool update(CoreProcData& processInfo) override;
		virtual bool render(CoreProcData& processInfo) override;
		virtual bool doNoteOn(CoreProcData& processInfo) override;
		virtual bool doNoteOff(CoreProcData& processInfo) override;

	protected:
		/** helper to render each sample from wavetable*/
		double renderSample(SynthClock& clock, bool forceLoop);

		// --- basic variables
		double sampleRate = 0.0;		///< sample rate
		double outputAmplitude = 1.0;	///< amplitude in dB
		double panLeftGain = 0.707;		///< left channel gain
		double panRightGain = 0.707;	///< right channel gain
		bool oneShotDone = false;		///< one-shot flag
		int32_t currentWaveIndex = -1;  ///< to minimize dictionary (map) lookup iterating

		// --- timebase
		SynthClock oscClock;	///< timebase

		// --- table source
		IWavetableSource* selectedTableSource = nullptr;	///< selected based on oscillator pitch

		// --- wavetable sources; these are used to register the tables with the database
		//     if the tables already exist, these won't be used. Notice that the update() function
		//     ONLY uses wavetable sources from the database! 
		//
		//     NOTE: this is an old-fashioned mapping where each drum is it's own patch
		//           See the comments in pcmsample.cpp for mapping each drum to a different key
		//           search for: // --- here, I am mapping them to the C-major scale white keys, starting at middle C
		DrumWTSource drumTables[MODULE_STRINGS];
	};

} // namespace

