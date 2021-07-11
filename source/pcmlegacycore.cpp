#include "pcmlegacycore.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   pcmlegacycore.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs
	Construction: Cores follow the same construction pattern
	- set the Module type and name parameters
	- expose the 16 module strings
	- expose the 4 mod knob label strings
	- intialize any internal variables

	Core Specific:
	- Waveform names are the names of folders in the \SynthLabSamples\Legacy container folder

	\returns the newly constructed object
	*/
	LegacyPCMCore::LegacyPCMCore()
	{
		moduleType = PCMO_MODULE;
		moduleName = "Legacy";
		preferredIndex = 0; // DM ordering

		// --- our PCM samples; names here match Folder names in the SynthLabSamples
		/*
			Module Strings, zero-indexed for your GUI Control:
			- Dagga, DigPuff, Divider, Doom, Dumma, Fazzy, FuzzVibe, Heaver, KickWob, Knocka, MultiWave,
			  OldFlatty, Polsar, Quorum, RazzBass, Wobbular
		*/
		coreData.moduleStrings[0] = "Dagga";		coreData.moduleStrings[8] = "KickWob";
		coreData.moduleStrings[1] = "DigPuff";		coreData.moduleStrings[9] = "Knocka";
		coreData.moduleStrings[2] = "Divider";		coreData.moduleStrings[10] = "MultiWave";
		coreData.moduleStrings[3] = "Doom";			coreData.moduleStrings[11] = "OldFlatty";
		coreData.moduleStrings[4] = "Dumma";		coreData.moduleStrings[12] = "Polsar";
		coreData.moduleStrings[5] = "Fazzy";		coreData.moduleStrings[13] = "Quorum";
		coreData.moduleStrings[6] = "FuzzVibe";		coreData.moduleStrings[14] = "RazzBass";
		coreData.moduleStrings[7] = "Heaver";		coreData.moduleStrings[15] = "Wobbular";

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A]	= "A";
		coreData.modKnobStrings[MOD_KNOB_B]	= "B";
		coreData.modKnobStrings[MOD_KNOB_C]	= "C";
		coreData.modKnobStrings[MOD_KNOB_D]	= "D";
	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- initialize timbase and hard synchronizer
	- parses WAV folders and adds PCM samples to database as needed

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool LegacyPCMCore::reset(CoreProcData& processInfo)
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
			for (uint32_t i = HALF_LEGACY_STRINGS; i < MODULE_STRINGS; i++)
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
        //
		pluginFolderPath += "/SynthLabSamples/Legacy/";

		// --- iterate through each folder to creaate the multisample sets
		uint32_t setLimit = processInfo.midiInputData->getAuxDAWDataUINT(kHalfSampleSet) == 1 ? HALF_LEGACY_STRINGS : MODULE_STRINGS;
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
	- selects a PCM sample based on the oscillator's modulated frequency
	- calculates final gain and pan values

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool LegacyPCMCore::update(CoreProcData& processInfo)
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
	- supports FM via the pmBuffer if avaialble; if pmBuffer is NULL then there is no FM
	Core Specific:
	- simple reading of PCM wave buffer
	- no FM
	- applies gain/volume
	- renders one block of audio per render cycle
	- renders in mono that is copied to the right channel as dual-mono stereo

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool LegacyPCMCore::render(CoreProcData& processInfo)
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

			if(selectedSampleSource)
			{
				// --- read sample
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
	- resets read index values to -1 indicating a new note event

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool LegacyPCMCore::doNoteOn(CoreProcData& processInfo)
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
	bool LegacyPCMCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}

	/**
	\brief Query the PCM database and add sample sources as needed

	\param sampleDirectory dirctory full of PCM WAV files
	]param sampleName name of this sample set to check
	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	void LegacyPCMCore::checkAddSampleSet(std::string sampleDirectory,
										  std::string sampleName,
										  CoreProcData& processInfo,
										  uint32_t index)
	{
		// --- there is one and only one PCM source per waveform "patch" or "sample"
		if (index >= MODULE_STRINGS) return;

		// --- try to find the source, if not existing add it
		if (!processInfo.sampleDatabase->getSampleSource(sampleName.c_str()))
		{
			// --- initializes PCM samples (opens folders of WAV files and extracts audio samples)
			pcmSources[index].init(sampleDirectory.c_str(), sampleName.c_str(), processInfo.sampleRate);
			pcmSources[index].setSampleLoopMode(SampleLoopMode::sustain);
			processInfo.sampleDatabase->addSampleSource(sampleName.c_str(), &pcmSources[index]);
		}
	}


} // namespace


