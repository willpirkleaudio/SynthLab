#include "mellotroncore.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   mellotron.cpp
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
	- Waveform names are the names of folders in the \SynthLabSamples\Mellotron\ container folder

	\returns the newly constructed object
	*/
	MellotronCore::MellotronCore()
	{
		moduleType = PCMO_MODULE;
		moduleName = "Mellotron";
		preferredIndex = 1; // ordering for user

		// --- our waveforms
		/*
			Module Strings, zero-indexed for your GUI Control:
			- Cello, Choir, M300_Brass, M300A, M300B, MK2_Brass, MK2_Flute, MK2_Violins, String_Section, Woodwinds
		*/
		coreData.moduleStrings[0] = "Cello";			coreData.moduleStrings[8] =  "M300A";
		coreData.moduleStrings[1] = "Choir";			coreData.moduleStrings[9] =  "M300B";
		coreData.moduleStrings[2] = "M300 Brass";		coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = "String Section";	coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = "Woodwinds";		coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = "MK2 Brass";		coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = "MK2 Flute";		coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = "MK2 Violins";		coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A]	 = "A";
		coreData.modKnobStrings[MOD_KNOB_B]	 = "B";
		coreData.modKnobStrings[MOD_KNOB_C]	 = "C";
		coreData.modKnobStrings[MOD_KNOB_D]	 = "D";
	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- initialize timbase and hard synchronizer
	- finds WAV files in a common folder (name of instrument)
	- sample sets are loaded into database if not existing already

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool MellotronCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		PCMOscParameters* parameters = static_cast<PCMOscParameters*>(processInfo.moduleParameters);

		// --- store
		sampleRate = processInfo.sampleRate;

		// --- reset to new start phase
		readIndex = 0.0;

		// --- for reduced memory requirements
		if (processInfo.midiInputData->getAuxDAWDataUINT(kHalfSampleSet) == 1)
		{
			for (uint32_t i = HALF_MELLOTRON_STRINGS; i < MODULE_STRINGS; i++)
				coreData.moduleStrings[i] = empty_string.c_str();
		}

        // --- initialize samples
        //
        // --- processInfo.dllPath is the folder that contains the plugin binary (Windows)
        //     or outer bundle folder (MacOS)
        //
        // --- NOTE: this is where you can change the location of the samples folder, for example
        //           to a fixed location where an installer dropped the files... or hardcoded for development
        //
        std::string pluginFolderPath = processInfo.dllPath;

		// --- append the location of the samples
		pluginFolderPath += "/SynthLabSamples/Mellotron/";

		// --- iterate through each folder to creaate the multisample sets
		uint32_t setLimit = processInfo.midiInputData->getAuxDAWDataUINT(kHalfSampleSet) == 1 ? HALF_MELLOTRON_STRINGS : MODULE_STRINGS;
		for (uint32_t i = 0; i < setLimit; i++)
		{
			std::string sampleFile = concatStrings(pluginFolderPath, coreData.moduleStrings[i]);
			checkAddSampleSet(sampleFile.c_str(), coreData.moduleStrings[i], processInfo, i);
		}

		// --- select first one
		currentIndex = 0;
		selectedSampleSource = processInfo.sampleDatabase->getSampleSource(coreData.moduleStrings[currentIndex]);

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
	- selects a PCM sample based on modulated frequency
	- calculates final gain and pan values

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool MellotronCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		PCMOscParameters* parameters = static_cast<PCMOscParameters*>(processInfo.moduleParameters);

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
		boundValue(oscillatorFrequency, PCM_OSC_MIN, PCM_OSC_MAX);

		// --- select the sample source by unique name
		//
		// --- this is a map lookup, so the current index is saved as this value won't change often
		if (currentIndex != parameters->waveIndex)
		{
			const char* wave = coreData.moduleStrings[parameters->waveIndex];
			selectedSampleSource = processInfo.sampleDatabase->getSampleSource(wave);
			currentIndex = parameters->waveIndex;
		}

		// --- select sample and calcualte phase inc
		phaseInc = selectedSampleSource ? selectedSampleSource->selectSample(oscillatorFrequency) : 0.0;

		// --- pan
		double panModulator = processInfo.modulationInputs->getModValue(kUniqueMod);
		double panTotal = parameters->panValue + 0.5*panModulator;
		boundValueBipolar(panTotal);

		// --- equal power calculation in synthfunction.h
		calculatePanValues(panTotal, panLeftGain, panRightGain);

		// --- scale from dB
		outputAmplitude = dB2Raw(parameters->outputAmplitude_dB);

		return true;
	}

	/**
	\brief Renders the output of the module
	- renders to output buffer using pointers in the CoreProcData argument
	- does not support FM
	Core Specific:
	- read PCM sample from source (may be mono or stereo)
	- write to output buffers

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool MellotronCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		PCMOscParameters* parameters = static_cast<PCMOscParameters*>(processInfo.moduleParameters);

		// --- buffers
		float* leftOutBuffer = processInfo.outputBuffers[LEFT_CHANNEL];
		float* rightOutBuffer = processInfo.outputBuffers[RIGHT_CHANNEL];

		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			// --- read and output samples
			leftOutBuffer[i] = 0.0;
			rightOutBuffer[i] = 0.0;

			if (selectedSampleSource)
			{
				PCMSampleOutput output = selectedSampleSource->readSample(readIndex, phaseInc);
				// --- gain and pan
				leftOutBuffer[i] = outputAmplitude*panLeftGain*output.audioOutput[LEFT_CHANNEL];
				rightOutBuffer[i] = outputAmplitude*panRightGain*output.audioOutput[RIGHT_CHANNEL];
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
	- resets PCM sample lookup index

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool MellotronCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		PCMOscParameters* parameters = static_cast<PCMOscParameters*>(processInfo.moduleParameters);

		// --- parameters
		midiPitch = processInfo.noteEvent.midiPitch;

		// --- reset to new start phase
		readIndex = 0.0;

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
	bool MellotronCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}

	/**
	\brief Query the database and add a set of PCM samples if not existing already

	\param sampleDirectory folder full of folders of samples
	\param sampleName name of sub-folder with set of PCM samples
	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	void MellotronCore::checkAddSampleSet(std::string sampleDirectory,
										  std::string sampleName,
										  CoreProcData& processInfo,
										  uint32_t index)
	{
		// --- there is one and only one PCM source per waveform "patch" or "sample"
		if (index >= MODULE_STRINGS) return;

		// --- try to find the source, if not existing add it
		if (!processInfo.sampleDatabase->getSampleSource(sampleName.c_str()))
		{
			pcmSources[index].init(sampleDirectory.c_str(), sampleName.c_str(), processInfo.sampleRate);
			pcmSources[index].setSampleLoopMode(SampleLoopMode::loop);// note: try one-shot as well
			processInfo.sampleDatabase->addSampleSource(sampleName.c_str(), &pcmSources[index]);
		}
	}

} // namespace


