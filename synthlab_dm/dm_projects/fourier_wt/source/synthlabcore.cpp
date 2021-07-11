#include "synthlabcore.h"

// -----------------------------
//	    SynthLab SDK File     //
//			   for			  //
//	      DynamicModule       //
//	       Development        //
//  ----------------------------

/**
\file   synthlabcore.cpp extracted from fourierwtcore.cpp
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
	- demonstrates Fourier Synthesis; see createTables()
	- implements two Fourier tables, add more here
	*/
	SynthLabCore::SynthLabCore()
	{
		moduleType = WTO_MODULE;
		moduleName = "Fourier WT";
		preferredIndex = 2;

		// --- our solitary waveform
		/*
			Module Strings, zero-indexed for your GUI Control:
			- sinewave, parabola
		*/
		coreData.moduleStrings[0] = "sinewave";					coreData.moduleStrings[8] =  empty_string.c_str();
		coreData.moduleStrings[1] = "parabola";					coreData.moduleStrings[9] =  empty_string.c_str();
		coreData.moduleStrings[2] = empty_string.c_str();		coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = empty_string.c_str();		coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = empty_string.c_str();		coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();		coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();		coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();		coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A] = "Shape";
		coreData.modKnobStrings[MOD_KNOB_B] = "HSync";
		coreData.modKnobStrings[MOD_KNOB_C]	= "Phase";
		coreData.modKnobStrings[MOD_KNOB_D] = "D";
	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- initialize timbase and hard synchronizer
	- create new tables if fs changed, or on first reset
	 -query and add new tables to database

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		bool sampleRateChanged = (sampleRate != processInfo.sampleRate);

		// --- store for compare
		sampleRate = processInfo.sampleRate;

		// -- reset the synhcronizer
		hardSyncronizer.reset(sampleRate, 0.0);

		// --- reset to new start phase
		oscClock.reset(parameters->modKnobValue[MOD_KNOB_C]);

		// --- create new tables if fs changed, or on first reset
		if (sampleRateChanged)
		{
			processInfo.wavetableDatabase->removeTableSource("sinewave");
			processInfo.wavetableDatabase->removeTableSource("parabola");
			createTables(sampleRate);
		}

		uint32_t uniqueIndex = 0;
		if (!processInfo.wavetableDatabase->getTableSource(coreData.moduleStrings[0]))
		{
			processInfo.wavetableDatabase->addTableSource(coreData.moduleStrings[0], &sineTableSource, uniqueIndex);
			coreData.uniqueIndexes[0] = uniqueIndex;
		}

		if (!processInfo.wavetableDatabase->getTableSource(coreData.moduleStrings[1]))
		{
			processInfo.wavetableDatabase->addTableSource(coreData.moduleStrings[1], &dynamicTableSource, uniqueIndex);
			coreData.uniqueIndexes[1] = uniqueIndex;
		}

		// --- init; note how I try to get the table with the faster method first
		selectedTableSource = processInfo.wavetableDatabase->getTableSource(coreData.uniqueIndexes[0]);
		if(!selectedTableSource)
			selectedTableSource = processInfo.wavetableDatabase->getTableSource(coreData.moduleStrings[0]);

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
	- this is identical to the ClassicWTCore code; the only difference is dynamic tables

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

		// --- direct calculation version 2^(n/12) - note that this is equal temperatment
		double pitchShift = pow(2.0, currentPitchModSemitones / 12.0);

		// --- calculate the moduated pitch value
		double oscillatorFrequency = midiPitch*pitchShift;

		// --- BOUND the value to our range - in theory, we would bound this to any NYQUIST
		boundValue(oscillatorFrequency, WT_OSC_MIN, WT_OSC_MAX);

		// --- set the hard sync helper MOD_KNOB_B = HSYNC
		hardSyncRatio = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_B], 1.0, 4.0);

		// --- unique mod  input
		double hsMod = processInfo.modulationInputs->getModValue(kUniqueMod);

		// --- additive modulation with +/-half-way max (2.0)
		double mod = doBipolarModulation(hsMod, -2.0, 2.0);
		parameters->hardSyncRatio += mod;
		boundValue(parameters->hardSyncRatio, 1.0, 4.0);

		// --- set it
		hardSyncronizer.setHardSyncFrequency(oscillatorFrequency*hardSyncRatio);

		// --- select the wavetable source if changed
		if (currentWaveIndex != parameters->waveIndex)
		{
			// --- try to get the table with a unique index if possible
			selectedTableSource = processInfo.wavetableDatabase->getTableSource(coreData.uniqueIndexes[parameters->waveIndex]);

			// --- use the name (slow)
			if (!selectedTableSource)
				selectedTableSource = processInfo.wavetableDatabase->getTableSource(coreData.moduleStrings[parameters->waveIndex]);

			currentWaveIndex = parameters->waveIndex;
		}

		// --- select the wavetable based on note number of new osc frequency
		if(selectedTableSource)
			selectedTableSource->selectTable(midiNoteNumberFromOscFrequency(oscillatorFrequency));

		// --- phase inc = fo/fs
		oscClock.setFrequency(oscillatorFrequency, sampleRate);

		// --- scale from dB
		outputAmplitude = dB2Raw(parameters->outputAmplitude_dB);

		// --- shape MOD_KNOB_A = shape
		parameters->oscillatorShape = bipolar(parameters->modKnobValue[MOD_KNOB_A]) + processInfo.modulationInputs->getModValue(kShapeMod);
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
	double SynthLabCore::renderSample(SynthClock& clock, double shape)
	{
		double mCounter = clock.mcounter;
		mCounter = applyPhaseDistortion(mCounter, unipolar(shape));

		double oscOutput =  selectedTableSource ? selectedTableSource->readWaveTable(mCounter) : 0.0;
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
	double  SynthLabCore::renderHardSyncSample(SynthClock& clock, double shape)
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
			if (hardSyncRatio > 1.0)
				oscOutput = renderHardSyncSample(oscClock, parameters->oscillatorShape);
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
	bool SynthLabCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- parameters
		midiPitch = processInfo.noteEvent.midiPitch;

		// --- reset to new start phase
		if (processInfo.unisonStartPhase > 0.0)
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
	bool SynthLabCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}

	/**
	\brief Table Creation funciont
	- creates a parabola waveform
	- bandlimited to each MIDI note's capabilities; calculates harmonic number limit

	\param sampleRate required for bandlimiting operation

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::createTables(double sampleRate)
	{
		if (sampleRate == currentTableRate)
			return false; // not newly created

		currentTableRate = sampleRate;

		dynamicTableSource.clearAllWavetables();

		// --- create the tables
		unsigned int numTables = 9;// 9 octave tables

		// Note A0, bottom of piano = Note 21
		uint32_t startMIDINote = 0;
		uint32_t seedMIDINote = MIDI_NOTE_A0;
		uint32_t endMIDINote = seedMIDINote;

		double seedFreq = midiNoteNumberToOscFrequency(seedMIDINote, 440.0);

		for (int j = 0; j < numTables; j++)
		{
			std::shared_ptr<double> tableAccumulator(new double[kDefaultWaveTableLength], std::default_delete<double[]>());

			memset(tableAccumulator.get(), 0, kDefaultWaveTableLength * sizeof(double));

			int numHarmonics = (int)((sampleRate / 2.0 / seedFreq) - 1.0);
			double maxTableValue = 0;

			for (int i = 0; i < kDefaultWaveTableLength; i++)
			{
				// --- if no harmonics, stuff a SIN table in there
				if (numHarmonics < 1)
				{
					// sin(wnT) = sin(2pi*i/kWaveTableLength)
					tableAccumulator.get()[i] = sin(((double)i / kDefaultWaveTableLength)*(2.0 * kPi));
				}
				else
				{
					for (int g = 1; g <= numHarmonics; g++)
					{
						// --- Parabola Waveform
						double n = double(g);
						tableAccumulator.get()[i] += (1.0 / (n*n))*cos(2.0*kPi*i*n / kDefaultWaveTableLength);
					}
				}

				// --- store the max values
				if (i == 0)
				{
					maxTableValue = tableAccumulator.get()[i];
				}
				else
				{
					// --- test and store
					if (tableAccumulator.get()[i] > maxTableValue)
						maxTableValue = tableAccumulator.get()[i];
				}
			}

			// --- normalize
			for (int i = 0; i < kDefaultWaveTableLength; i++)
			{
				// normalize it
				tableAccumulator.get()[i] /= maxTableValue;
			}

			// --- store
			if (j == numTables - 1)
				endMIDINote = 127;

			// --- add octave table across a range of MIDI notes from start to end
			dynamicTableSource.addWavetable(startMIDINote, endMIDINote, tableAccumulator,  kDefaultWaveTableLength, "parabola");

			// --- next table is one octave up
			startMIDINote = seedMIDINote + 1;
			seedFreq *= 2.0;
			seedMIDINote += 12;
			endMIDINote = seedMIDINote;
		}
		return true;
	}
} // namespace




