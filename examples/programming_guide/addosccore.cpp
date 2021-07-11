
#include "addosccore.h"

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
	- depends on your core type

	\returns the newly constructed object
	*/
	AddOscCore::AddOscCore()
	{
		moduleType = OSC_MODULE;
		moduleName = "Add Osc";

		// --- setup your module string here; use empty_string.c_str() for blank (empty) strings
		coreData.moduleStrings[0] = "sine";                 coreData.moduleStrings[8] = empty_string.c_str();
		coreData.moduleStrings[1] = "additive";             coreData.moduleStrings[9] = empty_string.c_str();
		coreData.moduleStrings[2] = empty_string.c_str();   coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = empty_string.c_str();   coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = empty_string.c_str();   coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();   coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();   coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();   coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A] = "Harm_2"; 
		coreData.modKnobStrings[MOD_KNOB_B] = "Harm_3"; 
		coreData.modKnobStrings[MOD_KNOB_C] = "Harm_4"; 
		coreData.modKnobStrings[MOD_KNOB_D] = "Harm_5"; 

	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- resets and initializes clocks and timers
	- sets initial state variables

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool AddOscCore::reset(CoreProcData& processInfo)
	{
		// --- save sample rate
		sampleRate = processInfo.sampleRate;

		// --- Get Parameters
		//
		OscParameters* parameters = static_cast<OscParameters*>(processInfo.moduleParameters);

		// --- RESET OPERATIONS HERE
		oscClock.reset();

		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool AddOscCore::update(CoreProcData& processInfo)
	{
		// --- Get Parameters (will use in next module)
		//
		OscParameters* parameters = static_cast<OscParameters*>(processInfo.moduleParameters);

		// --- UPDATE OPERATIONS HERE
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
		boundValue(oscillatorFrequency, OSC_FMIN, OSC_FMAX);

		// --- phase inc = fo/fs this sets it
		oscClock.setFrequency(oscillatorFrequency, sampleRate);

		return true;
	}

/**
	\brief Renders the output of the module
	- write modulator output with: processInfo.modulationOutputs->setModValue( )

	Core Specific:
	- see class declaration (comments and Doxygen docs) about where to write output values
	- see template below

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool AddOscCore::render(CoreProcData& processInfo)
	{
		// --- Get Parameters
		//
		OscParameters* parameters = static_cast<OscParameters*>(processInfo.moduleParameters);

		// --- get output buffers
		float* leftOutBuffer = processInfo.outputBuffers[LEFT_CHANNEL];
		float* rightOutBuffer = processInfo.outputBuffers[RIGHT_CHANNEL];

		// --- render additive signal
		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			double oscOutput = 0.0;

			// --- generate the fundamental
			double fund = sin(oscClock.mcounter * kTwoPi);

			if (parameters->waveIndex == 0) // fundamental only
				oscOutput = fund;
			else if (parameters->waveIndex == 1)// additive
			{
				// --- get the harmonic amplitudes from mod-knobs (or other GUI controls)
				//     note that the value is already on the range we desire, from 0.0 to 1.0 so there
				//     is no need to use the helper functions
				double h2Amp = parameters->modKnobValue[MOD_KNOB_A];
				double h3Amp = parameters->modKnobValue[MOD_KNOB_B];
				double h4Amp = parameters->modKnobValue[MOD_KNOB_C];
				double h5Amp = parameters->modKnobValue[MOD_KNOB_D];

				// --- additive signal
				oscOutput = fund + h2Amp * (doChebyWaveshaper(fund, ChebyT2))
					+ h3Amp * (doChebyWaveshaper(fund, ChebyT3))
					+ h4Amp * (doChebyWaveshaper(fund, ChebyT4))
					+ h5Amp * (doChebyWaveshaper(fund, ChebyT5));

				// --- scale by -12dB to prevent clipping
				oscOutput *= 0.25; 
			}

			// --- write out
			leftOutBuffer[i] = oscOutput;
			rightOutBuffer[i] = oscOutput;

			// --- advance clock and wrap if needed
			oscClock.advanceWrapClock();
		}

		return true;
	}

		/**
	\brief Note-on handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool AddOscCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		midiPitch = processInfo.noteEvent.midiPitch;

		// --- reset timebase
		oscClock.reset();

		return true;
	}

		/**
	\brief Note-off handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool AddOscCore::doNoteOff(CoreProcData& processInfo)
	{
		// do note off specific stuff

		return true;
	}

} // namespace


