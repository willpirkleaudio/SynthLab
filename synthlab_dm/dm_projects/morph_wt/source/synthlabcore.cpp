#include "synthlabcore.h"

// -----------------------------
//	    SynthLab SDK File     //
//			   for			  //
//	      DynamicModule       //
//	       Development        //
//  ----------------------------

/**
\file   synthlabcore.cpp extracted from morphwtcore.cpp
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
	- uses MorphBankData structures to hold information about these STATIC structures
	*/
	SynthLabCore::SynthLabCore()
	{
		moduleType = WTO_MODULE;
		moduleName = "Morph WT";
		preferredIndex = 1; // ordering for user

		// --- this must be done before setting module strings
		uint32_t count = 0;

		// --- our morphing banks
		/*
			Module Strings, zero-indexed for your GUI Control:
			- PrimalWaves, DigiDoo1, DigiDoo2, DigiDoo3, SawDemon, SquareDuty, SquareComb, SquareRing, SineMorph, Dentist
		*/
		addMorphBankData("PrimalWaves", PrimalWaves_BankDescriptor, count++);
		addMorphBankData("DigiDoo1", DigDoo1_BankDescriptor, count++);
		addMorphBankData("DigiDoo2", DigDoo1_BankDescriptor, count++);
		addMorphBankData("DigiDoo3", DigDoo3_BankDescriptor, count++);
		addMorphBankData("SawDemon", sawdemon_BankDescriptor, count++);
		addMorphBankData("SquareDuty", sqrduty_BankDescriptor, count++);
		addMorphBankData("SquareComb", squarecomb_BankDescriptor, count++);
		addMorphBankData("SquareRing", squarering_BankDescriptor, count++);
		addMorphBankData("SineMorph", SineMorph_BankDescriptor, count++);
		addMorphBankData("Dentist", Dentist_BankDescriptor, count++);

		// --- loads the waveform strings for dynamic menus
		for(uint32_t i=0; i<MODULE_STRINGS; i++)
			coreData.moduleStrings[i] = morphBankData[i].bankName.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A]	= "A";
		coreData.modKnobStrings[MOD_KNOB_B]	= "HSync";
		coreData.modKnobStrings[MOD_KNOB_C]	= "MorphStart";
		coreData.modKnobStrings[MOD_KNOB_D]	= "MorphMod";
	}


	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- initialize timbase and hard synchronizer
	- check and add wavetable banks to the database
	- a bank is a set of wavetable sources

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- store for compare
		sampleRate = processInfo.sampleRate;

		// -- reset the synhcronizer
		hardSyncronizer.reset(sampleRate, 0.0);

		// --- reset to new start phase
		oscClock.reset(parameters->modKnobValue[MOD_KNOB_A]);

		// --- add banks here (EXACT same ordering as strings in constructor)
		uint32_t count = 0;
		checkAddWaveBank(PrimalWaves_BankDescriptor, processInfo, count++);
		checkAddWaveBank(DigDoo1_BankDescriptor, processInfo, count++);
		checkAddWaveBank(DigDoo2_BankDescriptor, processInfo, count++);
		checkAddWaveBank(DigDoo3_BankDescriptor, processInfo, count++);
		checkAddWaveBank(sawdemon_BankDescriptor, processInfo, count++);
		checkAddWaveBank(sqrduty_BankDescriptor, processInfo, count++);
		checkAddWaveBank(squarecomb_BankDescriptor, processInfo, count++);
		checkAddWaveBank(squarering_BankDescriptor, processInfo, count++);
		checkAddWaveBank(SineMorph_BankDescriptor, processInfo, count++);
		checkAddWaveBank(Dentist_BankDescriptor, processInfo, count++);

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
	- selects a pair of wavetables based on modulated frequency AND morphing location across bank
	- sets up hard-sync (this oscillator's unique modulation)
	- calculates final gain and pan values

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- get the pitch bend value in semitones
		double midiPitchBend = calculatePitchBend(processInfo.midiInputData);

		// --- get the master tuning multiplier in semitones
		double masterTuning = calculateMasterTuning(processInfo.midiInputData);

		// --- calculate combined tuning offsets by simply adding values in semitones
		double freqMod = processInfo.modulationInputs->getModValue(kBipolarMod) * kOscBipolarModRangeSemitones;

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

		// --- direct calculation version 2^(n/12) - note that this is equal temperatment
		double pitchShift = pow(2.0, currentPitchModSemitones / 12.0);

		// --- calculate the moduated pitch value
		double oscillatorFrequency = midiPitch*pitchShift;

		// --- BOUND the value to our range - in theory, we would bound this to any NYQUIST
		boundValue(oscillatorFrequency, WT_OSC_MIN, WT_OSC_MAX);

		// --- set the hard sync helper MOD_KNOB_B = HSYNC
		parameters->hardSyncRatio = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_B], 1.0, 4.0);

		// --- update
		hardSyncronizer.setHardSyncFrequency(oscillatorFrequency*(parameters->hardSyncRatio));

		// --- select the wavetable sources
		// ---
		double morphMod = processInfo.modulationInputs->getModValue(kWaveMorphMod);

		/*
			ALL Cores have a special Modulation that is routed as kUniqueMod
			For the SynthLabCore this is the morphing location
		*/
		// --- add to morph mod value; note scaling
		double specialMod = 0.5*processInfo.modulationInputs->getModValue(kUniqueMod);
		morphMod += specialMod;

		// --- apply final intensity
		morphMod *= parameters->modKnobValue[MOD_KNOB_D];

		// --- NOTE -1
		uint32_t morphTables = morphBankData[parameters->waveIndex].numTables - 1;

		// --- calculate morph start location
		double morphStart = (double)morphTables * parameters->modKnobValue[MOD_KNOB_C];
		morphLocation = (morphMod * (morphTables - morphStart)) + morphStart;
		boundValue(morphLocation, 0.0, (double)morphTables);// -1);

		// --- get table indexes for morph
		uint32_t table0 = (uint32_t)morphLocation;
		uint32_t table1 = (uint32_t)morphLocation + 1;
		if (table1 > morphTables)
			table1 = table0;

		// --- split the fractional index into int.frac parts
		double morphFraction = morphLocation - (uint32_t)morphLocation;

		// --- calculate mix values
		calculateConstPwrMixValues(bipolar(morphFraction), mixValue0, mixValue1);

		// --- select the pair of tables for morph
		if (currentWaveIndex != parameters->waveIndex)
		{
			// --- try to get with index values (fastest)
			selectedTableSource[0] = processInfo.wavetableDatabase->getTableSource(morphBankData[parameters->waveIndex].tableIndexes[table0]);
			selectedTableSource[1] = processInfo.wavetableDatabase->getTableSource(morphBankData[parameters->waveIndex].tableIndexes[table1]);
			currentWaveIndex = parameters->waveIndex;
		}
		else
		{
			if (table0 != currentTable0)
			{
				selectedTableSource[0] = processInfo.wavetableDatabase->getTableSource(morphBankData[parameters->waveIndex].tableIndexes[table0]);
				currentTable0 = table0;
			}
			if (table1 != currentTable1)
			{
				selectedTableSource[1] = processInfo.wavetableDatabase->getTableSource(morphBankData[parameters->waveIndex].tableIndexes[table1]);
				currentTable1 = table1;
			}
		}

		if (morphMod != lastMorphMod)
			lastMorphMod = morphMod;

		// --- do we need to switch to next pair of tables?
		if (table0last != table0)
		{
			table0last = table0;
			table1last = table1;
		}

		// --- select the wavetable based on note number of new osc frequency
		if (selectedTableSource[0])
			selectedTableSource[0]->selectTable(midiNoteNumberFromOscFrequency(oscillatorFrequency));
		if (selectedTableSource[1])
			selectedTableSource[1]->selectTable(midiNoteNumberFromOscFrequency(oscillatorFrequency));

		// --- phase inc = fo/fs
		oscClock.setFrequency(oscillatorFrequency, sampleRate);

		// --- scale from dB
		outputAmplitude = dB2Raw(parameters->outputAmplitude_dB);

		// --- pan
		double panTotal = parameters->panValue;// +processInfo.modulationInputs[kAuxBPMod_1];
		boundValueBipolar(panTotal);

		// --- equal power calculation in synthfunction.h
		calculatePanValues(panTotal, panLeftGain, panRightGain);

		return true;
	}


	/**
	\brief Renders one sample out of the wavetable
	Core Specific:
	- note the check to see if we are at the end of the tables (morph = 1)
	- uses constant power morphing; you can try the others too: square law and linear

	\param clock the current timebase

	\returns true if successful, false otherwise
	*/
	double SynthLabCore::renderSample(SynthClock& clock)
	{
		double mCounter = clock.mcounter;
		if (!selectedTableSource[0] || !selectedTableSource[1])
			return 0.0;

		// --- integer morph location
		if (selectedTableSource[0] == selectedTableSource[1])
		{
			double oscOutput = selectedTableSource[0]->readWaveTable(mCounter);
			clock.advanceWrapClock();
			return oscOutput;
		}

		// --- two table reads
		double oscOutput0 = selectedTableSource[0]->readWaveTable(mCounter);
		double oscOutput1 = selectedTableSource[1]->readWaveTable(mCounter);

		// --- morph
		double oscOutput = oscOutput0*mixValue0 + oscOutput1*mixValue1;

		// --- increment phase
		clock.advanceWrapClock();
		return oscOutput;
	}

	/**
	\brief Renders one hard-synced sample from the wavetable
	Core Specific:
	- calls renderSample() for the main and reset oscillators as needed
	- detects new reset signal and restarts synchronizer

	\param clock the current timebase

	\returns true if successful, false otherwise
	*/
	double  SynthLabCore::renderHardSyncSample(SynthClock& clock)
	{
		double output = 0.0;

		if (hardSyncronizer.isProcessing())
		{
			// --- read table
			double outputA = 0.0;
			double outputB = 0.0;

			// --- two samples to blend
			outputA = renderSample(hardSyncronizer.getCrossFadeClock());
			outputB = renderSample(hardSyncronizer.getHardSyncClock());

			// --- do the crossfade
			output = hardSyncronizer.doHardSyncXFade(outputA, outputB);

			// --- advance the resetting-clock
			clock.advanceWrapClock();
		}
		else
		{
			// --- render
			output = renderSample(hardSyncronizer.getHardSyncClock());

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
	bool SynthLabCore::render(CoreProcData& processInfo)
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
				oscOutput = renderHardSyncSample(oscClock);
			else
				oscOutput = renderSample(oscClock);

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
	- resets table index values to -1 indicating a new note event

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters un-comment if needed
		// WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- parameters
		midiPitch = processInfo.noteEvent.midiPitch;

		// --- reset to new start phase
		if (processInfo.unisonStartPhase > 0.0)
			oscClock.reset(processInfo.unisonStartPhase / 360.0);

		table0last = -1;
		table1last = -1;
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
	bool SynthLabCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}

	/**
	\brief Adds a set of tables to the morph bank data structures, storing names of the tables

	Core Specific:
	- nothing to do

	\param name name of wavetable
	\param slBankSet set of sets-of-tables to be added
	\param index index in MorphBankData structure
	*/
	void SynthLabCore::addMorphBankData(std::string name, SynthLabBankSet& slBankSet, uint32_t index)
	{
		morphBankData[index].bankName.assign(name);
		morphBankData[index].numTables = slBankSet.tablePtrsCount;
		for (uint32_t i = 0; i < slBankSet.tablePtrsCount; i++)
		{
			morphBankData[index].tableNames[i] = slBankSet.tableNames[i].c_str();

			// --- if these are static (compiled into the plugin) use the bank wavetable source
			if (slBankSet.staticSources)
				morphBankData[index].staticSources[i] = slBankSet.staticSources[i];
			// --- if dynamic, you will need to create the table here, and destroy it when done
		}
	}

	/**
	\brief Calls the querying function to check and add a new wavebank (set of wavetables)

	\param slBankSet set of banks to be added
	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters
    \param waveIndex index of module-string array corresponding to this set
	*/
	void SynthLabCore::checkAddWaveBank(SynthLabBankSet& slBankSet, CoreProcData& processInfo, uint32_t waveIndex)
	{
		for (uint32_t i = 0; i < slBankSet.tablePtrsCount; i++)
		{
			SynthLabTableSet* table = slBankSet.tablePtrs[i];
			std::string waveName = slBankSet.tableNames[i];

			if (table)
			{
				table->waveformName = waveName.c_str();
				morphBankData[waveIndex].tableIndexes[i] = checkAddWavetable(*table, &morphBankData[waveIndex].staticSources[i], processInfo);
			}
		}
	}

	/**
	\brief Function that queries the datbase for the various tables based on unique table names
	- this is a set-wise version of the same function found on the non-morphing cores

	\param slTableSet set of tables tp be added
	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns index of table in database if successful, -1 otherwise
	*/
	int32_t SynthLabCore::checkAddWavetable(SynthLabTableSet& slTableSet, StaticTableSource* tableSource, CoreProcData& processInfo)
	{
		uint32_t uniqueIndex = 0;
		int32_t foundIndex = -1;
		if (!processInfo.wavetableDatabase->getTableSource(slTableSet.waveformName))
		{
			tableSource->addSynthLabTableSet(&slTableSet);

			if (!processInfo.wavetableDatabase->addTableSource(slTableSet.waveformName, tableSource, uniqueIndex))
				return -1; // could not add should never happen

			return uniqueIndex;
		}
		else
			foundIndex = processInfo.wavetableDatabase->getWaveformIndex(slTableSet.waveformName);

		return foundIndex;
	}

} // namespace
