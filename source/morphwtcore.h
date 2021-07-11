#pragma once

#include "synthbase.h"
#include "synthfunctions.h"
#include "synthlabwtsource.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   morphwtcore.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class MorphWTCore
	\ingroup ModuleCores
	\brief
	Morphing Wavetable oscillator 
	- demonstrates morphing between two IWavetableSources
	- demonstrates use of MorphBankData to store a bank of wavetable oscillator waveforms
	- morphs across a range of up to 16 waveforms, each of which is a set of high-resolution wavetables
	- allows hard-sync of wavetables during morph (!!)

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
	- PrimalWaves, DigiDoo1, DigiDoo2, DigiDoo3, SawDemon, SquareDuty,S quareComb, SquareRing, SineMorph, Dentist

	ModKnob Strings, for fixed GUI controls by index constant
	- MOD_KNOB_A = "A"
	- MOD_KNOB_B = "HSync"
	- MOD_KNOB_C = "MorphStart"
	- MOD_KNOB_D = "MorphMod"

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
	class MorphWTCore : public ModuleCore
	{
	public:
		/** simple default constructor */
		MorphWTCore();				/* C-TOR */
		
		/** Destructor is empty: all resources are smart pointers */
		virtual ~MorphWTCore() {}		/* D-TOR */

		/** ModuleCore Overrides */
		virtual bool reset(CoreProcData& processInfo) override;
		virtual bool update(CoreProcData& processInfo) override;
		virtual bool render(CoreProcData& processInfo) override;
		virtual bool doNoteOn(CoreProcData& processInfo) override;
		virtual bool doNoteOff(CoreProcData& processInfo) override;

		/** Render helper functions */
		double renderSample(SynthClock& clock); ///< render a sample
		double renderHardSyncSample(SynthClock& clock); ///< render a hard-sunk sample

	protected:
		// --- local variables
		double sampleRate = 0.0;		///< sample rate
		double currentTableRate = 0.0;	///< sample rate
		double midiPitch = 0.0;			///< the midi pitch
		double outputAmplitude = 1.0;	///< amplitude in dB
		double panLeftGain = 0.707;		///< left channel gain
		double panRightGain = 0.707;	///< right channel gain
		double hardSyncRatio = 1.0;		///< hard sync ratio with modulators applied
		int32_t currentWaveIndex = -1;  ///< to minimize dictionary (map) lookup iterating
		int32_t currentTable0 = -1;		///< to minimize dictionary (map) lookup iterating
		int32_t currentTable1 = -1;		///< to minimize dictionary (map) lookup iterating
		
		// --- const power summing:
		double mixValue0 = 0.0;
		double mixValue1 = 0.0;

		// --- timebase
		SynthClock oscClock;	///< timebase

		// --- table source
		IWavetableSource* selectedTableSource[2] = { nullptr, nullptr }; ///< two tables to morph across

		// -- hard sync helper
		Synchronizer hardSyncronizer; ///< hard synchronizer

		// --- set of banks, one for each module string
		MorphBankData morphBankData[MODULE_STRINGS];	///< morphing bank of wavetables
		void addMorphBankData(std::string name, SynthLabBankSet& slTableSet, uint32_t index); ///< add a bank top database

		int32_t table0last = -1;	///< running index of last table_0
		int32_t table1last = -1;	///< running index of last table_1
		double morphLocation = 0.0; ///< fractional location to morph between table_0 and table_1
		double lastMorphMod = -10.0;///< last morph modulator

		// --- helper function
		void checkAddWaveBank(SynthLabBankSet& slTableSet, CoreProcData& processInfo, uint32_t waveIndex);
		int32_t checkAddWavetable(SynthLabTableSet& slTableSet, StaticTableSource* tableSource, CoreProcData& processInfo);
	};

} // namespace

