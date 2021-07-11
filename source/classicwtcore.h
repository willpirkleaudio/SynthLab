#pragma once

#include "synthbase.h"
#include "synthfunctions.h"
#include "synthlabwtsource.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   classicwtcore.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class ClassicWTCore
	\ingroup ModuleCores
	\brief
	Wavetable oscillator with classic waveforms.
	- demonstrates use of IWavetableSource interface object

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
	- ana_saw, ana_sqr, fm_bass1, fm_bass2, fm_organ, vs_saw, vs_octsqr, vs_buzz,
	  akwf_br7, akwf_br12, akwf_br17, akwf_br24, akwf_dst15, akwf_dst22, akwf_dst27,
	  akwf_tri11

	ModKnob Strings, for fixed GUI controls by index constant
	- MOD_KNOB_A = "Shape"
	- MOD_KNOB_B = "HSync"
	- MOD_KNOB_C = "Phase"
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
	class ClassicWTCore : public ModuleCore
	{
	public:
		/** simple default constructor */
		ClassicWTCore();				/* C-TOR */
		
		/** Destructor is empty: all resources are smart pointers */
		virtual ~ClassicWTCore() {}		/* D-TOR */

		/** ModuleCore Overrides */
		virtual bool reset(CoreProcData& processInfo) override;
		virtual bool update(CoreProcData& processInfo) override;
		virtual bool render(CoreProcData& processInfo) override; 
		virtual bool doNoteOn(CoreProcData& processInfo) override;
		virtual bool doNoteOff(CoreProcData& processInfo) override;

		/** Helper functions for rendering  */
		double renderSample(SynthClock& clock, double shape = 0.5);
		double renderHardSyncSample(SynthClock& clock, double shape = 0.5);

	protected:
		// --- basic variables
		double sampleRate = 0.0;		///< sample rate
		double midiPitch = 0.0;			///< the midi pitch
		double outputAmplitude = 1.0;	///< amplitude in dB
		double panLeftGain = 0.707;		///< left channel gain
		double panRightGain = 0.707;	///< right channel gain
		int32_t currentWaveIndex = -1;  ///< to minimize dictionary (map) lookup iterating

		// --- timebase
		SynthClock oscClock; ///< the oscillator timebase

		// --- table source
		IWavetableSource* selectedTableSource = nullptr; ///< selected table

		// -- hard sync helper
		Synchronizer hardSyncronizer;	///< hard sync helper

		// --- function to add wavetables
		void checkAddWavetable(SynthLabTableSet& slTableSet, CoreProcData& processInfo, uint32_t waveIndex);

		// --- wavetable sources; these are used to register the tables with the database
		//     if the tables already exist, these won't be used. Notice that the update() function
		//     ONLY uses wavetable sources from the database! 
		StaticTableSource wavetables[MODULE_STRINGS];
	};

} // namespace

