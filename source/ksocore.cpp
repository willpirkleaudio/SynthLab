#include "ksocore.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   ksocore.cpp
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

	\returns the newly constructed object
	*/
	KSOCore::KSOCore()
	{
		moduleType = KSO_MODULE;
		moduleName = "SynLabKSO";
		preferredIndex = 0; // DM ordering


		// --- our module strings (algorithms
		/*
			Module Strings, zero-indexed for your GUI Control:
			- nylon, dist gtr, bass
		*/
		coreData.moduleStrings[kNylonGtr] = "nylon";		coreData.moduleStrings[8] =  empty_string.c_str();
		coreData.moduleStrings[kDistGtr] = "dist gtr";		coreData.moduleStrings[9] =  empty_string.c_str();
		coreData.moduleStrings[kBass] = "bass";				coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[kSilent] = empty_string.c_str();		coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = empty_string.c_str();			coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();			coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();			coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();			coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A] = "Detune";
		coreData.modKnobStrings[MOD_KNOB_B] = "Boost";
		coreData.modKnobStrings[MOD_KNOB_C]	= "Bite";
		coreData.modKnobStrings[MOD_KNOB_D]	= "Pluck Pos";
	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- reset member objects
	- setup bass filter with fc = 150 Hz, Q = 0.707
	- setup distortion filter with fc = 2000 Hz, Q = 1.0

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool KSOCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		KSOscParameters* parameters = static_cast<KSOscParameters*>(processInfo.moduleParameters);

		sampleRate = processInfo.sampleRate;

		sampleRate = processInfo.sampleRate;
		resonator.reset(sampleRate);
		exciter.reset(sampleRate);
		pluckPosFilter.reset(sampleRate);
		highShelfFilter.reset(sampleRate);
		bodyFilter.reset(sampleRate);
		bassFilter.reset(sampleRate);
		bassFilter.setParameters(150.0, 0.707);
		distortionFilter.reset(sampleRate);
		distortionFilter.setParameters(2000.0, 2.5);

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
	- calculates pluck position filtering parameters
	Changes body filter parameters based on selected model:
	- nylong string: fc = 400Hz, Q = 1.0
	- distorted electric guitar: fc = 300Hz, Q = 2.0
	- bass guitar: fc = 250Hz, Q = 1.0

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool KSOCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		KSOscParameters* parameters = static_cast<KSOscParameters*>(processInfo.moduleParameters);

		// --- this tube may or may not respond to pitch bend and other frequency
		//     modifications, depending on its usage
		// --- get the pitch bend value in semitones
		double midiPitchBend = calculatePitchBend(processInfo.midiInputData);

		// --- get the master tuning multiplier in semitones
		double masterTuning = calculateMasterTuning(processInfo.midiInputData);

		// --- calculate combined tuning offsets by simply adding values in semitones
		double freqMod = processInfo.modulationInputs->getModValue(kBipolarMod) * kOscBipolarModRangeSemitones;

		// --- do the portamento
		double glideMod = glideModulator->getNextModulationValue();

		// --- coarse tuning from mod knob A
		parameters->coarseDetune = (int32_t)getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_A], -12.0, 12.0);

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
		double oscillatorFrequency = midiPitch * pitchShift;

		//oscillatorFrequency = 1000.0;

		// --- BOUND the value to our range
		boundValue(oscillatorFrequency, OSC_FMIN, OSC_FMAX);

		// --- Resonator:
		double delayLen = resonator.setParameters(oscillatorFrequency, parameters->decay);

		// --- Pluck Position (homework - make this the unique modulation destination)
		pluckPosition = (uint32_t)getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_D], 10.0, 2.0);
		pluckPosFilter.setDelayInSamples((delayLen / pluckPosition));

		// --- exciter
		exciter.setParameters(parameters->attackTime_mSec, parameters->holdTime_mSec, parameters->releaseTime_mSec);

		// --- filters
		double bite_dB = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_C], 0.0, 20.0);
		highShelfFilter.setParameters(2000.0, bite_dB);

		// --- boost
		double body_dB = +3.0;

		// --- change body filter resonance based on model
		if (parameters->algorithmIndex == kNylonGtr)
			bodyFilter.setParameters(400.0, 1.0, body_dB);
		else if (parameters->algorithmIndex == kDistGtr)
			bodyFilter.setParameters(300.0, 2.0, body_dB);
		else if (parameters->algorithmIndex == kBass)
			bodyFilter.setParameters(250.0, 1.0, body_dB);

		// --- panmod is unique mod for this core
		double panModulator = processInfo.modulationInputs->getModValue(kUniqueMod);
		double panTotal = parameters->panValue + 0.5*panModulator;
		boundValueBipolar(panTotal);

		// --- equal power calculation in synthfunction.h
		calculatePanValues(panTotal, panLeftGain, panRightGain);

		double output_dB = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_B], 0.0, 20.0);
		outputAmplitude = pow(10.0, output_dB / 20.0);


		return true;
	}


	/**
	\brief Renders the output of the module
	- renders to output buffer using pointers in the CoreProcData argument
	- supports FM via the pmBuffer if avaialble; if pmBuffer is NULL then there is no FM
	Core Specific:
	- calls the subfiltering in this order:
	- (1) render exciter output sample
	- (2) apply high-shelf filter -> pluck position filter
	- (3) render the resonator using the filtered input
	- (4) apply distortion for electric guitar
	- (5) process through body filter

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool KSOCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		KSOscParameters* parameters = static_cast<KSOscParameters*>(processInfo.moduleParameters);

		float* leftOutBuffer = processInfo.outputBuffers[LEFT_CHANNEL];
		float* rightOutBuffer = processInfo.outputBuffers[RIGHT_CHANNEL];

		// --- render square and saw at same time
		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			double input = exciter.render();
			input = highShelfFilter.processAudioSample(input);

			if (parameters->algorithmIndex == kNylonGtr)
				input = pluckPosFilter.processAudioSample(input, PluckFilterType::kPluckAndBridge);
			else if (parameters->algorithmIndex == kDistGtr)
				input = pluckPosFilter.processAudioSample(input, PluckFilterType::kPluckAndPickup);
			else if (parameters->algorithmIndex == kBass)
				input = pluckPosFilter.processAudioSample(input, PluckFilterType::kPluckPickupBridge);
			else if (parameters->algorithmIndex == kSilent)
				input = 0.0;

			// --- resonate the excitation
			double oscOutput = resonator.process(input);

			// --- VERY simple guitar amp sim
			//
			// --- this is just a simple overdriven tanh() waveshapewr
			//     you DEFINITELY want to change this to something that
			//     you like better (see my tube addendum here:
			//     http://www.willpirkle.com/fx-book-bonus-material/chapter-19-addendum/
			if (parameters->algorithmIndex == kDistGtr)
			{
				// --- the x10 will add sustain, the 5000.0 will add distortion
				oscOutput = tanhWaveShaper(oscOutput*10.0, 5000.0); // adjust distortion with 2nd argument

				//// --- really simple clipping version, no tanh() call
				//oscOutput *= 100.0;
				//boundValue(oscOutput, -0.25, +0.25);
				//oscOutput *= 4.0;

				// --- drop -6dB to make up for energy
				//
				//     note that this will pull out the very high frequency components;
				//     you may want to adjust this filter (see reset() function)
				oscOutput = 0.5*distortionFilter.processAudioSample(oscOutput);
			}

			// --- add resonance if desired
			oscOutput = bodyFilter.processAudioSample(oscOutput);
			oscOutput *= outputAmplitude;

			// --- write to output buffers
			leftOutBuffer[i] = panLeftGain * oscOutput;
			rightOutBuffer[i] = panRightGain * oscOutput;
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
	- resets all sub-filters and objects

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool KSOCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		KSOscParameters* parameters = static_cast<KSOscParameters*>(processInfo.moduleParameters);

		// --- parameters
		midiPitch = processInfo.noteEvent.midiPitch;

		// --- reset
		resonator.flushDelays();
		pluckPosFilter.clear();
		highShelfFilter.reset(sampleRate);
		bassFilter.reset(sampleRate);
		bodyFilter.reset(sampleRate);

		// --- start excitation
		exciter.startExciter();

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
	bool KSOCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}

} // namespace


