#include "synthlabcore.h"

// -----------------------------
//	    SynthLab SDK File     //
//			   for			  //
//	      DynamicModule       //
//	       Development        //
//  ----------------------------

/**
\file   synthlabcore.cpp extracted from drumwtcore.cpp
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
	- in addition, this core adds its tables directly to setup the database

	Core Specific:
	- tables are defined in wavetables/drumtables.h file
	- uses DrumWTSource to store pointers to the tables
	- gets table pointers from database
	- uses static
	*/
	SynthLabCore::SynthLabCore()
	{
		moduleType = WTO_MODULE;
		moduleName = "Drum WT";
		preferredIndex = 3; // ordering for user, DM only

		/*
			Module Strings, zero-indexed for your GUI Control:
			- Kick_1, Kick_2, Kick_3, Snare_1, Snare_2, Snare_3, Closed_HH, Open_HH, Tom_1,
			  Tom_2, Tom_3, Tom_4, Tom_5, Crash_1, Crash_2, Crash_3
		*/

		// --- this is an array of static tables;
		//     there is little overhead in storing information about it
		drumTables[0].addWavetable(&kick1[0], kick1Length, "Kick 1");
		drumTables[1].addWavetable(&kick2[0], kick2Length, "Kick 2");
		drumTables[2].addWavetable(&kick3[0], kick3Length, "Kick 3");
		drumTables[3].addWavetable(&snare1[0], snare1Length, "Snare 1");
		drumTables[4].addWavetable(&snare2[0], snare2Length, "Snare 2");
		drumTables[5].addWavetable(&snare3[0], snare3Length, "Snare 3");
		drumTables[6].addWavetable(&closedhat[0], closedhatLength, "Closed HH");
		drumTables[7].addWavetable(&openhat[0], openhatLength, "Open HH");

		drumTables[8].addWavetable(&tom1[0], tom1Length, "Tom 1");
		drumTables[9].addWavetable(&tom2[0], tom2Length, "Tom 2");
		drumTables[10].addWavetable(&tom3[0], tom3Length, "Tom 3");
		drumTables[11].addWavetable(&tom4[0], tom4Length, "Tom 4");
		drumTables[12].addWavetable(&tom5[0], tom5Length, "Tom 5");
		drumTables[13].addWavetable(&crash1[0], crash1Length, "Crash 1");
		drumTables[14].addWavetable(&crash2[0], crash2Length, "Crash 2");
		drumTables[15].addWavetable(&crash3[0], crash3Length, "Crash 3");

		// --- our WTO waveforms, built in
		for (uint32_t i = 0; i<MODULE_STRINGS; i++)
			coreData.moduleStrings[i] = drumTables[i].getWaveformName();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A] = "A";
		coreData.modKnobStrings[MOD_KNOB_B] = "B";
		coreData.modKnobStrings[MOD_KNOB_C]	= "C";
		coreData.modKnobStrings[MOD_KNOB_D] = "D";
	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- wavetable database is accessed via processInfo.wavetableDatabase
	- initialize timbase
	- check table sources and add if needed

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::reset(CoreProcData& processInfo)
	{
        // --- parameters - uncomment if needed
		//WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- store for compare
		sampleRate = processInfo.sampleRate;

		// --- reset to new start phase
		oscClock.reset();

		// --- check database; this is another way to do it
		for (uint32_t i = 0; i < MODULE_STRINGS; i++)
		{
			uint32_t uniqueIndex = 0;
			if (!processInfo.wavetableDatabase->getTableSource(drumTables[i].getWaveformName()))
			{
				if (!processInfo.wavetableDatabase->addTableSource(drumTables[i].getWaveformName(), &drumTables[i], uniqueIndex))
					;
				else
					coreData.uniqueIndexes[i] = uniqueIndex;
			}
			else
				coreData.uniqueIndexes[i] = processInfo.wavetableDatabase->getWaveformIndex(drumTables[i].getWaveformName());
		}


		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- there is NO pitch modulation as these are pitchless drum samples
	- to add pitch modulation, see the ClassicWTCore object
	- calculates final gain and pan values

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- this is a pitchless table, so frequency is extracted from table length
		double oscillatorFrequency = sampleRate / drumTables[parameters->waveIndex].getWaveTableLength();

		// --- select the wavetable source, only if changed
		if (parameters->waveIndex != currentWaveIndex)
		{
			// --- try to get by index first (faster at runtime)
			selectedTableSource = processInfo.wavetableDatabase->getTableSource(coreData.uniqueIndexes[parameters->waveIndex]);

			// --- see if there is a map version (slower at runtime)
			if (!selectedTableSource)
				selectedTableSource = processInfo.wavetableDatabase->getTableSource(drumTables[parameters->waveIndex].getWaveformName());
		}

		// --- phase inc = fo/fs
		oscClock.setFrequency(oscillatorFrequency, sampleRate);

		// --- scale from dB
		outputAmplitude = dB2Raw(parameters->outputAmplitude_dB);

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
	- reads drum table in one-shot fashion

	\param clock the current timebase

	\returns true if successful, false otherwise
	*/
	double SynthLabCore::renderSample(SynthClock& clock, bool forceLoop)
	{
		// --- read source
		double oscOutput =  selectedTableSource ? selectedTableSource->readWaveTable(clock.mcounter) : 0.0;

		// --- advance and wrap clock; save wrap notice for one-shot
		if (forceLoop)
			clock.advanceWrapClock();
		else
			oneShotDone = clock.advanceWrapClock();

		//oneShotDone = clock.advanceWrapClock();
		return oscOutput;
	}

	/**
	\brief Renders the output of the module
	- renders to output buffer using pointers in the CoreProcData argument
	- calls rendering sub-function
	- the simplest render function of any oscillator

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		float* leftOutBuffer = processInfo.outputBuffers[LEFT_CHANNEL];
		float* rightOutBuffer = processInfo.outputBuffers[RIGHT_CHANNEL];

		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			// --- render the drum hit
			double oscOutput = oneShotDone ? 0.0 : renderSample(oscClock, parameters->forceLoop);

			// --- scale by gain control
			oscOutput *= outputAmplitude;

			// --- write to output buffers
			leftOutBuffer[i] = oscOutput * panLeftGain;
			rightOutBuffer[i] = oscOutput * panRightGain;
		}

		// --- rendered
		return true;
	}

	/**
	\brief Note-on handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- resets the clock
	- sets one-shot flag

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters - uncomment if needed
		// WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- reset the bool
		oneShotDone = false;
		oscClock.reset();
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


} // namespace


