#include "synthlabcore.h"

// -----------------------------
//	    SynthLab SDK File     //
//			   for			  //
//	      DynamicModule       //
//	       Development        //
//  ----------------------------

/**
\file   synthlabcore.cpp extracted from fmocore.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{

	/**
	\brief Constructs FM oscillator Core including standalone DXEG object

	Construction: Cores follow the same construction pattern
	- set the Module type and name parameters
	- expose the 16 module strings
	- expose the 4 mod knob label strings
	- intialize any internal variables
	*/
	SynthLabCore::SynthLabCore()
	{
		moduleType = FMO_MODULE;
		moduleName = "SynLabFMO";
		preferredIndex = 0;

		// --- our solitary waveform
		/*
			Module Strings, zero-indexed for your GUI Control:
			- sinewave
		*/
		coreData.moduleStrings[0] = "sinewave";			coreData.moduleStrings[8] =  empty_string.c_str();
		coreData.moduleStrings[1] = empty_string.c_str();		coreData.moduleStrings[9] =  empty_string.c_str();
		coreData.moduleStrings[2] = empty_string.c_str();		coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = empty_string.c_str();		coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = empty_string.c_str();		coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();		coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();		coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();		coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A]	= "A";
		coreData.modKnobStrings[MOD_KNOB_B]	= "B";
		coreData.modKnobStrings[MOD_KNOB_C]	= "C";
		coreData.modKnobStrings[MOD_KNOB_D]	= "F.Back";

		// --- create a DX EG in standalone mode (nullptr for midi & parameters)
		dxEG.reset(new DXEG(nullptr, nullptr, 64));

		// ---- store the parameters for easy control of object
		dxEGParameters = dxEG->getParameters();

		// --- core 1 = DX core
		dxEG->selectModuleCore(1);
	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- add the single sinusoidal wavetable to the database
	- reset the EG

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::reset(CoreProcData& processInfo)
	{
        // --- parameters - uncomment if needed
		// FMOperatorParameters* parameters = static_cast<FMOperatorParameters*>(processInfo.moduleParameters);

		// --- store
		sampleRate = processInfo.sampleRate;

		// --- reset
		oscClock.reset();

		uint32_t uniqueIndex = 0;

		// --- initialize wavetables
		uint32_t moduleStringIndex = 0;

		// --- NOTE: we only have one sinewave carrier/modulator at index 0
		;
		if (!processInfo.wavetableDatabase->getTableSource(sineTableSource.getWaveformName()))
		{
			if (!processInfo.wavetableDatabase->addTableSource(sineTableSource.getWaveformName(), &sineTableSource, uniqueIndex))
				; // error
			else
				coreData.uniqueIndexes[moduleStringIndex] = uniqueIndex;
		}
		else
			coreData.uniqueIndexes[moduleStringIndex] = processInfo.wavetableDatabase->getWaveformIndex(sineTableSource.getWaveformName());

		// --- reset for operation
		dxEG->reset(sampleRate);

		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- forwards EG parameters to the member DXEG object
	- calcuates and applies pitch mod
	- calculates final gain and pan values

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		FMOperatorParameters* parameters = static_cast<FMOperatorParameters*>(processInfo.moduleParameters);

		// --- eg
		dxEGParameters->startLevel = parameters->dxEGParameters.startLevel;
		dxEGParameters->attackTime_mSec = parameters->dxEGParameters.attackTime_mSec;
		dxEGParameters->decayTime_mSec = parameters->dxEGParameters.decayTime_mSec;
		dxEGParameters->decayLevel = parameters->dxEGParameters.decayLevel;
		dxEGParameters->slopeTime_mSec = parameters->dxEGParameters.slopeTime_mSec;
		dxEGParameters->curvature = parameters->dxEGParameters.curvature;
		dxEGParameters->sustainLevel = parameters->dxEGParameters.sustainLevel;
		dxEGParameters->releaseTime_mSec = parameters->dxEGParameters.releaseTime_mSec;

		// --- mod knob D is self modulation
		dxEGParameters->modKnobValue[MOD_KNOB_D] = parameters->modKnobValue[MOD_KNOB_D];

		bool sustainOverride = processInfo.midiInputData->getCCMIDIData(SUSTAIN_PEDAL) > 63;
		dxEG->setSustainOverride(sustainOverride);

		// --- update the attached EG
		dxEG->update();

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
		double oscillatorFrequency = midiPitch*pitchShift*parameters->ratio;

		// --- BOUND the value to our range - in theory, we would bound this to any NYQUIST
		boundValue(oscillatorFrequency, FM_OSC_MIN, FM_OSC_MAX);

		// --- phase inc = fo/fs
		oscClock.setFrequency(oscillatorFrequency, sampleRate);

		// --- scale from dB
		outputAmplitude = dB2Raw(parameters->outputAmplitude_dB);

		// --- panmod is unique for FM operators
		double panModulator = processInfo.modulationInputs->getModValue(kUniqueMod);
		double panTotal = parameters->panValue + 0.5*panModulator;
		boundValueBipolar(panTotal);

		// --- equal power calculation in synthfunction.h
		calculatePanValues(panTotal, panLeftGain, panRightGain);

		return true;
	}

	/**
	\brief Renders one block of audio data
	Core Specific:
	- gets EG output value for the entire block
	- applies instantaneous phase modulation to synth clock timebase by addition
	- runs the oscillator for one sample period
	- applies EG contour to output of oscillator
	- includes self-modulation state variable if connected
	- renders into the output buffer using pointers in the CoreProcData argument to the render function
	- renders one block of audio per render cycle
	- renders in mono that is copied to the right channel as dual-mono stereo

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		FMOperatorParameters* parameters = static_cast<FMOperatorParameters*>(processInfo.moduleParameters);

		float* leftOutBuffer = processInfo.outputBuffers[LEFT_CHANNEL];
		float* rightOutBuffer = processInfo.outputBuffers[RIGHT_CHANNEL];
		float* pmBufferL = processInfo.fmBuffers == nullptr ? nullptr : processInfo.fmBuffers[LEFT_CHANNEL];
		float* pmBufferR = processInfo.fmBuffers == nullptr ? nullptr : processInfo.fmBuffers[RIGHT_CHANNEL];

		// --- self modulation
		bool selfModulate = parameters->modKnobValue[MOD_KNOB_D] > 0.0;

		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			// --- get the EG output; note that this is NOT granulized (block processed) but
			//     rather processes each output for the operator
			//     this minimizes steps in the harmonics which can result with high indexes
			//     of modulation and/or series operators
			dxEG->render();
			double egOutput = dxEG->getModulationOutput()->getModValue(kEGNormalOutput);

			// --- PHASE MODULATION
			if (pmBufferL && pmBufferR)
			{
				// --- convert PM buffer to mono
				double modValue = parameters->phaseModIndex * (0.5*pmBufferL[i] + 0.5*pmBufferR[i]);
				oscClock.addPhaseOffset(modValue);
			}
			else if(selfModulate)
			{
				// --- note the mapping here max value is 0.20 -- it gets very squealy above
				//     this value, but feel free to experiment
				double feedback = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_D], 0.0, 0.20);
				double modValue = parameters->phaseModIndex * feedback * outputValue;
				oscClock.addPhaseOffset(modValue);
			}

			// --- render
			outputValue = egOutput * sineTableSource.readWaveTable(oscClock.mcounter);

			// --- scale by gain control
			outputValue *= outputAmplitude;

			// --- write to output buffers
			leftOutBuffer[i] = outputValue * panLeftGain;
			rightOutBuffer[i] = outputValue * panRightGain;

			if ((pmBufferL && pmBufferR) || selfModulate)
			{
				oscClock.removePhaseOffset();
				oscClock.wrapClock();
			}

			// --- setup for next cycle
			oscClock.advanceWrapClock();
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
	- resets timebase
	- rests DXEG for another run

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/

	bool SynthLabCore::doNoteOn(CoreProcData& processInfo)
	{
        // --- parameters - uncomment if needed
		// FMOperatorParameters* parameters = static_cast<FMOperatorParameters*>(processInfo.moduleParameters);

		// --- parameters
		midiPitch = processInfo.noteEvent.midiPitch;

		// --- reset to new start phase
		oscClock.reset();

		// --- start FSM
		dxEG->doNoteOn(processInfo.noteEvent);

		return true;
	}


	/**
	\brief Note-off handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- sends attached EG into release phase

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::doNoteOff(CoreProcData& processInfo)
	{
        // --- parameters - uncomment if needed
		// FMOperatorParameters* parameters = static_cast<FMOperatorParameters*>(processInfo.moduleParameters);

		// --- go to release state
		dxEG->doNoteOff(processInfo.noteEvent);

		return true;
	}

} // namespace



