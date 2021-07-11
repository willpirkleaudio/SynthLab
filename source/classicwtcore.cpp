#include "classicwtcore.h"
#include "../wavetables/static_tables/anasaw.h"
#include "../wavetables/static_tables/anasquare.h"
#include "../wavetables/static_tables/vs_wt.h"
#include "../wavetables/static_tables/fm_wt.h"
#include "../wavetables/static_tables/akwf_8.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   classicwtcore.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief
	Construction: Cores follow the same construction pattern
	- set the Module type and name parameters
	- expose the 16 module strings
	- expose the 4 mod knob label strings
	- intialize any internal variables

	Core Specific:
	- uses SynthLab::SynthLabTableSet structure to describe the wavetables
	in the #included directories
	- you are free to create your own custom table descriptions

	\returns the newly constructed object
	*/
	ClassicWTCore::ClassicWTCore()
	{
		moduleType = WTO_MODULE;
		moduleName = "Classic WT";
		preferredIndex = 0; // ordering for user, DM only

		/*
			Module Strings, zero-indexed for your GUI Control:
			- ana_saw, ana_sqr, fm_bass1, fm_bass2, fm_organ, vs_saw, vs_octsqr, vs_buzz,
			  akwf_br7, akwf_br12, akwf_br17, akwf_br24, akwf_dst15, akwf_dst22, akwf_dst27,
			  akwf_tri11
		*/

		// --- our WTO waveforms, built in anasaw_TableSet
		coreData.moduleStrings[0] = anasaw_TableSet.waveformName;		coreData.moduleStrings[8] = akwf_br7_TableSet.waveformName;
		coreData.moduleStrings[1] = anasquare_TableSet.waveformName;	coreData.moduleStrings[9] = akwf_br12_TableSet.waveformName;
		coreData.moduleStrings[2] = fm_bass1_TableSet.waveformName;		coreData.moduleStrings[10] = akwf_br17_TableSet.waveformName;
		coreData.moduleStrings[3] = fm_bass2_TableSet.waveformName;		coreData.moduleStrings[11] = akwf_br24_TableSet.waveformName;
		coreData.moduleStrings[4] = fm_organ_TableSet.waveformName;		coreData.moduleStrings[12] = akwf_dst15_TableSet.waveformName;
		coreData.moduleStrings[5] = vs_saw_TableSet.waveformName;		coreData.moduleStrings[13] = akwf_dst22_TableSet.waveformName;
		coreData.moduleStrings[6] = vs_octsqr_TableSet.waveformName;	coreData.moduleStrings[14] = akwf_dst27_TableSet.waveformName;
		coreData.moduleStrings[7] = vs_buzz_TableSet.waveformName;		coreData.moduleStrings[15] = akwf_tri11_TableSet.waveformName;

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A]		= "Shape";
		coreData.modKnobStrings[MOD_KNOB_B]		= "HSync";
		coreData.modKnobStrings[MOD_KNOB_C]		= "Phase";
		coreData.modKnobStrings[MOD_KNOB_D]		= "D";
	}


	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- initialize timbase and hard synchronizer
	- check and add wavetables to the database

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool ClassicWTCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- store
		sampleRate = processInfo.sampleRate;

		// -- reset the synhcronizer
		hardSyncronizer.reset(sampleRate, 0.0);

		// --- reset to new start phase
		oscClock.reset(parameters->modKnobValue[MOD_KNOB_A]);

		// --- initialize wavetables   anasaw_TableSet
		uint32_t index = 0;
		checkAddWavetable(anasaw_TableSet, processInfo, index++);
		checkAddWavetable(anasquare_TableSet, processInfo, index++);
		checkAddWavetable(fm_bass1_TableSet, processInfo, index++);
		checkAddWavetable(fm_bass2_TableSet, processInfo, index++);
		checkAddWavetable(fm_organ_TableSet, processInfo, index++);
		checkAddWavetable(vs_saw_TableSet, processInfo, index++);
		checkAddWavetable(vs_octsqr_TableSet, processInfo, index++);
		checkAddWavetable(vs_buzz_TableSet, processInfo, index++);

		checkAddWavetable(akwf_br7_TableSet, processInfo, index++);
		checkAddWavetable(akwf_br12_TableSet, processInfo, index++);
		checkAddWavetable(akwf_br17_TableSet, processInfo, index++);
		checkAddWavetable(akwf_br24_TableSet, processInfo, index++);
		checkAddWavetable(akwf_dst15_TableSet, processInfo, index++);
		checkAddWavetable(akwf_dst22_TableSet, processInfo, index++);
		checkAddWavetable(akwf_dst27_TableSet, processInfo, index++);
		checkAddWavetable(akwf_tri11_TableSet, processInfo, index++);

		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- calculates the pitch modulation value from GUI controls, input modulator kBipolarMod,
	and MIDI pitch bend
	- selects a wavetable based on modulated frequency to avoid aliasing
	- sets up hard-sync (this oscillator's unique modulation)
	- calculates final gain and pan values

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool ClassicWTCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- get the pitch bend value in semitones
		double midiPitchBend = calculatePitchBend(processInfo.midiInputData);

		// --- get the master tuning multiplier in semitones
		double masterTuning = calculateMasterTuning(processInfo.midiInputData);

		// --- calculate combined tuning offsets by simply adding values in semitones
		double freqMod = processInfo.modulationInputs->getModValue(kBipolarMod) * kOscBipolarModRangeSemitones;

		// --- apply final intensity control
		// freqMod *= parameters->modKnobValue[WTO_PITCH_MOD];

		// --- do the portamento
		double glideMod = glideModulator->getNextModulationValue();

		// --- calculate combined tuning offsets by simply adding values in semitones
		double currentPitchModSemitones = glideMod +
			freqMod +
			midiPitchBend +
			masterTuning +
			(parameters->oscSpecificDetune) +	/* semitones */
			(parameters->octaveDetune * 12) +	/* octaves =  semitones*12 */
			(parameters->coarseDetune) +		/* semitones */
			(parameters->fineDetune / 100.0) +	/* cents/100 = semitones */
			(processInfo.unisonDetuneCents / 100.0);	/* cents/100 = semitones */

		// --- lookup the pitch shift modifier (fraction)
		//double pitchShift = pitchShiftTableLookup(currentPitchModSemitones);
		// --- or ...
		// --- direct calculation version 2^(n/12) - note that this is equal temperatment
		double pitchShift = pow(2.0, currentPitchModSemitones / 12.0);

		// --- calculate the moduated pitch value
		double oscillatorFrequency = midiPitch*pitchShift;

		// --- BOUND the value to our range - in theory, we would bound this to any NYQUIST
		boundValue(oscillatorFrequency, WT_OSC_MIN, WT_OSC_MAX);

		// --- phase inc = fo/fs
		oscClock.setFrequency(oscillatorFrequency, sampleRate);

		// --- set the hard sync helper
		/*
			ALL Cores have a special Modulation that is routed as kUniqueMod
			For the ClassicWTCore this is Hard Sync Modulation
		*/
		parameters->hardSyncRatio = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_B], 1.0, 4.0);

		// --- unique mod  input
		double hsMod = processInfo.modulationInputs->getModValue(kUniqueMod);

		// --- additive modulation with full wave rectified control signal (cut LFO rate in half)
		double mod = fabs(hsMod);
		mapDoubleValue(mod, 0.0, 0.0, HSYNC_MOD_SLOPE);
		parameters->hardSyncRatio += mod;
		boundValue(parameters->hardSyncRatio, 1.0, 4.0);

		// --- update
		hardSyncronizer.setHardSyncFrequency(oscillatorFrequency*(parameters->hardSyncRatio));

		// --- select the wavetable source; only if changed
		if (parameters->waveIndex != currentWaveIndex)
		{
			// --- try to get by index first (faster at runtime)
			selectedTableSource = processInfo.wavetableDatabase->getTableSource(coreData.uniqueIndexes[parameters->waveIndex]);

			// --- see if there is a map version (slower at runtime)
			if(!selectedTableSource)
				selectedTableSource = processInfo.wavetableDatabase->getTableSource(coreData.moduleStrings[parameters->waveIndex]);

			currentWaveIndex = parameters->waveIndex;
		}

		// --- select table
		uint32_t midiNote = midiNoteNumberFromOscFrequency(oscillatorFrequency);
		selectedTableSource->selectTable(midiNote);

		// --- scale from GUI dB
		outputAmplitude = dB2Raw(parameters->outputAmplitude_dB);

		// --- shape
		parameters->oscillatorShape = parameters->modKnobValue[MOD_KNOB_A];
		boundValueBipolar(parameters->oscillatorShape);

		// --- pan
		double panTotal = parameters->panValue;
		boundValueBipolar(panTotal);

		// --- equal power calculation in synthfunction.h
		calculatePanValues(panTotal, panLeftGain, panRightGain);

		return true;
	}

	/**
	\brief Renders one sample out of the wavetable
	Core Specific:
	- applies phase distortion for shape modulation

	\param clock the current timebase
	\param shape the shape amount [-1, +1] from GUI and/or modulation

	\returns true if successful, false otherwise
	*/
	double ClassicWTCore::renderSample(SynthClock& clock, double shape)
	{
		double mCounter = clock.mcounter;

		// --- apply shape via PD
		mCounter = applyPhaseDistortion(mCounter, shape);

		// --- use source to read table
		double oscOutput = selectedTableSource->readWaveTable(mCounter);

		// --- setup for next cycle
		clock.advanceWrapClock();
		return oscOutput;
	}

	/**
	\brief Renders one hard-synced sample from the wavetable
	Core Specific:
	- calls renderSample() for the main and reset oscillators as needed
	- detects new reset signal and restarts synchronizer

	\param clock the current timebase
	\param shape the shape amount [-1, +1] from GUI and/or modulation

	\returns true if successful, false otherwise
	*/
	double  ClassicWTCore::renderHardSyncSample(SynthClock& clock, double shape)
	{
		double output = 0.0;

		if (hardSyncronizer.isProcessing())
		{
			// --- read table
			double outputA = 0.0;
			double outputB = 0.0;

			// --- two samples to blend
			outputA = renderSample(hardSyncronizer.getCrossFadeClock(), shape);
			outputB = renderSample(hardSyncronizer.getHardSyncClock(), shape);

			// --- do the crossfade
			output = hardSyncronizer.doHardSyncXFade(outputA, outputB);

			// --- advance the resetting-clock
			clock.advanceWrapClock();
		}
		else
		{
			// --- render
			output = renderSample(hardSyncronizer.getHardSyncClock(), shape);

			// --- check to see if we wrapped -> retrigger oscillator
			if (clock.advanceWrapClock())
				hardSyncronizer.startHardSync(clock);
		}

		return output;
	}

	/**
	\brief Renders the output of the module
	- renders to output buffer using pointers in the CoreProcData argument
	- supports FM via the pmBuffer if avaialble; if pmBuffer is NULL then there is no FM
	Core Specific:
	- call rendering sub-function
	- apply and remove phase offsets for phase modulation (FM)
	- renders one block of audio per render cycle
	- renders in mono that is copied to the right channel as dual-mono stereo

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool ClassicWTCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		float* leftOutBuffer = processInfo.outputBuffers[LEFT_CHANNEL];
		float* rightOutBuffer = processInfo.outputBuffers[RIGHT_CHANNEL];
		float* pmBuffer = processInfo.fmBuffers == nullptr ? nullptr : processInfo.fmBuffers[MONO_CHANNEL];

		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			// --- PHASE MODULATION
			if (pmBuffer)
			{
				double modValue = parameters->phaseModIndex * pmBuffer[i];
				oscClock.addPhaseOffset(modValue);
				hardSyncronizer.addPhaseOffset(modValue);
			}

			// --- render the saw
			double oscOutput = 0.0;
			if (parameters->hardSyncRatio > 1.0)
				oscOutput = renderHardSyncSample(oscClock, parameters->oscillatorShape); // shape is passed in, for the case of sync finished
			else
				oscOutput = renderSample(oscClock, parameters->oscillatorShape);

			// --- scale by gain control
			oscOutput *= outputAmplitude;

			// --- write to output buffers
			leftOutBuffer[i] = oscOutput * panLeftGain;
			rightOutBuffer[i] = oscOutput * panRightGain;

			if (pmBuffer)
			{
				oscClock.removePhaseOffset();
				oscClock.wrapClock();
				hardSyncronizer.removePhaseOffset();
			}
		}

		// --- advance the glide modulator
		glideModulator->advanceClock(processInfo.samplesToProcess);

		// --- rendered
		return true;
	}

	/**
	\brief Note-on handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- saves MIDI pitch for modulation calculation in update() function
	- sets wavetable start-read location (aka "Phase")

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool ClassicWTCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- parameters
		midiPitch = processInfo.noteEvent.midiPitch;

		// --- reset to new start phase
		if(processInfo.unisonStartPhase > 0.0)
			oscClock.reset(processInfo.unisonStartPhase / 360.0);
		else
			oscClock.reset(parameters->modKnobValue[MOD_KNOB_C]); // MOD_KNOB_C = start phase

		currentWaveIndex = -1;
		return true;
	}

	/**
	\brief Note-off handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- nothing to do

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool ClassicWTCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}

	/**
	\brief helper function to check database for wavetable and add it if the table does not exist
	- wavetables are identified with unique waveform name string
	- uses static table sources since these wavetables are fixed and never re-calculated
	- sets the unique index of the table in the database for faster parsing; you may still use the
	  unique name string as a means of lookup, but it will slow the update() function a bit

	Core Specific:
	- nothing to do

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	void ClassicWTCore::checkAddWavetable(SynthLabTableSet& slTableSet, CoreProcData& processInfo, uint32_t waveIndex)
	{
		if (!processInfo.wavetableDatabase->getTableSource(slTableSet.waveformName))
		{
			// --- this needs to cross the thunk-layer and we need naked (not std::shared) pointers
			//
			// --- note that there are a couple of different approaches for this, see other WT cores
			wavetables[waveIndex].addSynthLabTableSet(&slTableSet);
			uint32_t uniqueIndex = 0;
			if (!processInfo.wavetableDatabase->addTableSource(slTableSet.waveformName, &wavetables[waveIndex], uniqueIndex))
				; // delete wt;
			else
				coreData.uniqueIndexes[waveIndex] = uniqueIndex;
		}
		else
			coreData.uniqueIndexes[waveIndex] = processInfo.wavetableDatabase->getWaveformIndex(slTableSet.waveformName);
	}

} // namespace


