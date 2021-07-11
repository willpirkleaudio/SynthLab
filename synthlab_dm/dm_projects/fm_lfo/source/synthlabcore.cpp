#include "synthlabcore.h"

// -----------------------------
//	    SynthLab SDK File     //
//			   for			  //
//	      DynamicModule       //
//	       Development        //
//  ----------------------------

/**
\file   synthlabcore.cpp extracted from fmlfocore.cpp
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
	- uses simple FM to create complex LFO patterns
	- three FM algoriths are implemented, add more here

	*/
	SynthLabCore::SynthLabCore()
	{
		moduleType = LFO_MODULE;
		moduleName = "FM-LFO";
		preferredIndex = 1;

		// --- our LFO waveforms
		/*
			Module Strings, zero-indexed for your GUI Control:
			- FM_2, FM_3A, FM_3B
		*/
		coreData.moduleStrings[0] = "FM-2";			coreData.moduleStrings[8] =  empty_string.c_str();
		coreData.moduleStrings[1] = "FM-3A";		coreData.moduleStrings[9] =  empty_string.c_str();
		coreData.moduleStrings[2] = "FM-3B";		coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = empty_string.c_str();	coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = empty_string.c_str();		coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();		coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();	coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();		coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A]	= "Shape";
		coreData.modKnobStrings[MOD_KNOB_B]	= "Ratio";
		coreData.modKnobStrings[MOD_KNOB_C]	= "Index";
		coreData.modKnobStrings[MOD_KNOB_D] = "BPM Sync";
	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- resets and initializes the 3 SynthClock objects that are the operators
	- sets initial state variables

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		LFOParameters* parameters = static_cast<LFOParameters*>(processInfo.moduleParameters);

		// --- store
		sampleRate = processInfo.sampleRate;

		// --- clear
		outputValue = 0.0;

		// --- reset clocks and timers
		for (uint32_t i = 0; i < NUM_FMLFO_OPS; i++)
		{
			fmOpClock[i].reset();
			fmOpClock[i].setFrequency(parameters->frequency_Hz, sampleRate);
		}

		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- supports sync-to-BPM as with the other LFOs
	- Uses three simple stacked FM algorithms:
		kFM2
		- OP2 -> (OP1 * ratio)

		kFM3A
		- OP3 -> (OP2  * ratio) -> (OP1 * 1.414 * ratio)

		kFM3B
		- OP3 -> (OP2  * ratio) -> (OP1 * 0.707 * ratio)

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		LFOParameters* parameters = static_cast<LFOParameters*>(processInfo.moduleParameters);

		// --- check for BPM sync // modKnobValue[BPMSYNC] defaults to center 0.5 position
		double bpmSync = getTimeFromTempo(processInfo.midiInputData->getAuxDAWDataFloat(kBPM), parameters->modKnobValue[MOD_KNOB_D]);
		if(bpmSync > 0.0)
			parameters->frequency_Hz = 1.0 / bpmSync;

		// --- apply linear modulation
		double modValue = processInfo.modulationInputs->getModValue(kFrequencyMod) * LFO_HALF_RANGE;

		// --- apply linear modulation
		double newFrequency_Hz = parameters->frequency_Hz + modValue;
		boundValue(newFrequency_Hz, LFO_FCMOD_MIN, LFO_FCMOD_MAX);

		// --- calculate the phase inc from the frequency
		double ratio = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_B], 0.50, 5.0);
		modStrength = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_C], 0.125, 4.0);

		if (parameters->waveformIndex == enumToInt(FMLFOWaveform::kFM2))
		{
			fmOpClock[0].setFrequency(newFrequency_Hz, sampleRate);
			fmOpClock[1].setFrequency(ratio * newFrequency_Hz, sampleRate);
			// not used fmOpClock[2].setFrequency(newFrequency_Hz, sampleRate);
		}
		else if (parameters->waveformIndex == enumToInt(FMLFOWaveform::kFM3A))
		{
			fmOpClock[0].setFrequency(newFrequency_Hz, sampleRate);
			fmOpClock[1].setFrequency(ratio * newFrequency_Hz, sampleRate);
			fmOpClock[2].setFrequency(1.414 * ratio * newFrequency_Hz, sampleRate);
		}
		else if (parameters->waveformIndex == enumToInt(FMLFOWaveform::kFM3B))
		{
			fmOpClock[0].setFrequency(newFrequency_Hz, sampleRate);
			fmOpClock[1].setFrequency(ratio * newFrequency_Hz, sampleRate);
			fmOpClock[2].setFrequency((1.0/1.414) * ratio * newFrequency_Hz, sampleRate);
		}

		return true;
	}

	/**
	\brief Renders the output of the module
	- write modulator output with: processInfo.modulationOutputs->setModValue( )
	Core Specific:
	- kLFONormalOutput normal LFO output value
	- kLFOInvertedOutput 180 degrees out of phase with normal output
	- kUnipolarFromMax unipolar version of the waveform that sits at +1 when the output amplitude is at 0,
	and descends from +1 downward as the output amplitude is increased; used for tremolo
	- kUnipolarFromMin unipolar version of the waveform that sits at 0 when the output amplitude is at 0,
	and ascends from 0 upward as the output amplitude is increased; the ordinary unipolar version

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		LFOParameters* parameters = static_cast<LFOParameters*>(processInfo.moduleParameters);

		if (renderComplete) return true;

		// --- for DEBUG to write LFO to output buffer for testing
		// --- float* outputBuffer = processInfo.outputBuffers[0];
		//
		// ---- loop over samples
		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			// --- check for completed 1-shot on this sample period
			bool bWrapped = fmOpClock[0].wrapClock();

			if (bWrapped && parameters->modeIndex == enumToInt(LFOMode::kOneShot))
			{
				renderComplete = true;
				outputValue = 0.0;
				return renderComplete;
			}

			// --- calculate the oscillator value
			if (parameters->waveformIndex == enumToInt(FMLFOWaveform::kFM2))
			{
				// --- calculate normal angle
				double angle = fmOpClock[1].mcounter*kTwoPi - kPi;
				double modulator = parabolicSine(-angle);
				double phaseMod = modStrength*modulator;

				fmOpClock[0].addPhaseOffset(phaseMod);
				angle = fmOpClock[0].mcounter*kTwoPi - kPi;
				double carrier = parabolicSine(-angle);
				fmOpClock[0].removePhaseOffset();

				outputValue = carrier;
			}
			else if (parameters->waveformIndex == enumToInt(FMLFOWaveform::kFM3A))
			{
				// --- calculate normal angle
				double angle = fmOpClock[2].mcounter*kTwoPi - kPi;
				double modulator = parabolicSine(-angle);
				double phaseMod = modStrength*modulator;

				fmOpClock[1].addPhaseOffset(phaseMod);
				angle = fmOpClock[1].mcounter*kTwoPi - kPi;
				modulator = parabolicSine(-angle);
				phaseMod = modStrength*modulator;
				fmOpClock[1].removePhaseOffset();

				fmOpClock[0].addPhaseOffset(phaseMod);
				angle = fmOpClock[0].mcounter*kTwoPi - kPi;
				double carrier = parabolicSine(-angle);
				fmOpClock[0].removePhaseOffset();

				outputValue = carrier;
			}
			else if (parameters->waveformIndex == enumToInt(FMLFOWaveform::kFM3B))
			{
				// --- calculate normal angle
				double angle = fmOpClock[2].mcounter*kTwoPi - kPi;
				double modulator = parabolicSine(-angle);
				double phaseMod1 = modStrength*modulator;

				angle = fmOpClock[1].mcounter*kTwoPi - kPi;
				modulator = parabolicSine(-angle);
				double phaseMod2 = modStrength*modulator;

				fmOpClock[0].addPhaseOffset(phaseMod1 + phaseMod2);
				angle = fmOpClock[0].mcounter*kTwoPi - kPi;
				double carrier = parabolicSine(-angle);
				fmOpClock[0].removePhaseOffset();

				outputValue = carrier;
			}

			// --- quantizer (stepper)
			if (parameters->quantize > 0)
				outputValue = quantizeBipolarValue(outputValue, parameters->quantize);

			outputValue = quantizeBipolarValue(outputValue, pow(2.0, 10));

			// --- scale by amplitude
			outputValue *= parameters->outputAmplitude;

			// --- SHAPE --- //
			double shape = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_A], 0.0, 1.0);
			double shapeOut = 0.0;
			if (shape >= 0.5)
				shapeOut = bipolarConvexXForm(outputValue);
			else
				shapeOut = bipolarConcaveXForm(outputValue);

			// --- split bipolar for multiplier
			shape = splitBipolar(shape);
			outputValue = shape*shapeOut + (1.0 - shape)*outputValue;

			// --- set outputs
			// --- first output sample only:
			if (i == 0)
			{
				processInfo.modulationOutputs->setModValue(kLFONormalOutput, outputValue);
				processInfo.modulationOutputs->setModValue(kLFOInvertedOutput, -outputValue);

				// --- special unipolar from max output for tremolo
				//
				// --- first, convert to unipolar
				processInfo.modulationOutputs->setModValue(kUnipolarFromMax, bipolar(outputValue));
				processInfo.modulationOutputs->setModValue(kUnipolarFromMin, bipolar(outputValue));

				// --- then shift upwards by enough to put peaks right at 1.0
				//     NOTE: leaving the 0.5 in the equation - it is the unipolar offset when convering bipolar; but it could be changed...
				processInfo.modulationOutputs->setModValue(kUnipolarFromMax, processInfo.modulationOutputs->getModValue(kUnipolarFromMax) + (1.0 - 0.5 - (parameters->outputAmplitude / 2.0)));

				// --- then shift down enough to put troughs at 0.0
				processInfo.modulationOutputs->setModValue(kUnipolarFromMin, processInfo.modulationOutputs->getModValue(kUnipolarFromMin) - (1.0 - 0.5 - (parameters->outputAmplitude / 2.0)));
			}

			// --- for DEBUG to write LFO to output buffer for testing
			// --- outputBuffer[i] = outputValue;

			// --- setup for next block
			// --- NOTE: inside the loop, advance by 1
			fmOpClock[0].advanceClock();
			fmOpClock[1].advanceWrapClock();
			fmOpClock[2].advanceWrapClock();
		}

		return true;
	}

	/**
	\brief Note-on handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- resets FM operator clocks when not in free-run mode
	- resets rendering flag

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		LFOParameters* parameters = static_cast<LFOParameters*>(processInfo.moduleParameters);

		renderComplete = false;

		if (parameters->modeIndex != enumToInt(LFOMode::kFreeRun))
		{
			// --- get ready for next run
			for (uint32_t i = 0; i < NUM_FMLFO_OPS; i++)
				fmOpClock[i].reset();

			outputValue = 0.0;
		}

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


