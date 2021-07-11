#include "sfxwtcore.h"
#include "../wavetables/static_tables/sfxtables.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   sfxwtcore.cpp
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
	- note use of DrumWTSource structures to hold table information here;
	this is not dynamic data and is small in size and nature

	\returns the newly constructed object
	*/
	SFXWTCore::SFXWTCore()
	{
		moduleType = WTO_MODULE;
		moduleName = "SFX WT";
		preferredIndex = 3; // ordering for user

		/*
			Module Strings, zero-indexed for your GUI Control:
			- Creme, JawBreak, Laser, Minipop, Noisey, Ploppy, Pop, RobotPunch, Seeit, Speeder, Splok,
			  Sploop, SprinGun, Swish, Swoosh, WackaBot
		*/
		sfxTables[0].addWavetable(&creme[0], cremeLength, "Creme");
		sfxTables[1].addWavetable(&jawbreak[0], jawbreakLength, "JawBreak");
		sfxTables[2].addWavetable(&laser[0], laserLength, "Laser");
		sfxTables[3].addWavetable(&minipop[0], minipopLength, "Minipop");
		sfxTables[4].addWavetable(&noisey[0], noiseyLength, "Noisey");
		sfxTables[5].addWavetable(&ploppy[0], ploppyLength, "Ploppy");
		sfxTables[6].addWavetable(&pop[0], popLength, "Pop");
		sfxTables[7].addWavetable(&robotpunch[0], robotpunchLength, "RobotPunch");
		sfxTables[8].addWavetable(&seeit[0], seeitLength, "Seeit");
		sfxTables[9].addWavetable(&speeder[0], speederLength, "Speeder");
		sfxTables[10].addWavetable(&splok[0], splokLength, "Splok");
		sfxTables[11].addWavetable(&sploop[0], sploopLength, "Sploop");
		sfxTables[12].addWavetable(&springun[0], springunLength, "SprinGun");
		sfxTables[13].addWavetable(&swish[0], swishLength, "Swish");
		sfxTables[14].addWavetable(&swoosh[0], swooshLength, "Swoosh");
		sfxTables[15].addWavetable(&wackabot[0], wackabotLength, "WackaBot");

		// --- our WTO waveforms, built in
		for (uint32_t i = 0; i<MODULE_STRINGS; i++)
			coreData.moduleStrings[i] = sfxTables[i].getWaveformName();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A]	 = "Shape";
		coreData.modKnobStrings[MOD_KNOB_B]	 = "HSync";
		coreData.modKnobStrings[MOD_KNOB_C]	 = "Phase";
		coreData.modKnobStrings[MOD_KNOB_D]	 = "D";
	}


	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- initialize timbase and hard synchronizer
	- check and add wavetables to the database

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SFXWTCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- store for compare
		sampleRate = processInfo.sampleRate;

		// --- reset to new start phase
		oscClock.reset();
		uint32_t uniqueIndex = 0;

		for (uint32_t i = 0; i < MODULE_STRINGS; i++)
		{
			if (!processInfo.wavetableDatabase->getTableSource(sfxTables[i].getWaveformName()))
			{
				processInfo.wavetableDatabase->addTableSource(sfxTables[i].getWaveformName(), &sfxTables[i], uniqueIndex);
				coreData.uniqueIndexes[i] = uniqueIndex;
			}
			else
				coreData.uniqueIndexes[i] = processInfo.wavetableDatabase->getWaveformIndex(sfxTables[i].getWaveformName());
		}
		return true;
	}


	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- NO pitch modulation for these SFX samples
	- selects a wavetable based on waveform name
	- calculates final gain and pan values

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SFXWTCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

		// --- calculate the moduated pitch value
		double oscillatorFrequency = sampleRate / sfxTables[parameters->waveIndex].getWaveTableLength();

		// --- select the wavetable source if changed
		if (currentWaveIndex != parameters->waveIndex)
		{
			// --- try to get the table with a unique index if possible
			selectedTableSource = processInfo.wavetableDatabase->getTableSource(coreData.uniqueIndexes[parameters->waveIndex]);

			// --- use the name (slow)
			if (!selectedTableSource)
				selectedTableSource = processInfo.wavetableDatabase->getTableSource(sfxTables[parameters->waveIndex].getWaveformName());

			currentWaveIndex = parameters->waveIndex;
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
	- applies phase distortion for shape modulation

	\param clock the current timebase
	\param forceLoop true to force this sample to loop (repeated over duration)

	\returns true if successful, false otherwise
	*/
	double SFXWTCore::renderSample(SynthClock& clock, bool forceLoop)
	{
		double oscOutput =  selectedTableSource ? selectedTableSource->readWaveTable(clock.mcounter) : 0.0;
		if(forceLoop)
			clock.advanceWrapClock();
		else
			oneShotDone = clock.advanceWrapClock();

		return oscOutput;
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
	bool SFXWTCore::render(CoreProcData& processInfo)
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
	- reset clock
	- reset one-shot flag

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SFXWTCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		WTOscParameters* parameters = static_cast<WTOscParameters*>(processInfo.moduleParameters);

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
	bool SFXWTCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}


} // namespace


