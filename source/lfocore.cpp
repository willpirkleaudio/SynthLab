#include "lfocore.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   lfocore.cpp
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
	- creates new lookup table, which are compact in nature
	(no reason to share the resource)

	\returns the newly constructed object
	*/
	LFOCore::LFOCore()
	{
		moduleType = LFO_MODULE;
		moduleName = "ClassicLFO";
		preferredIndex = 0; // ordering for user

		lookupTables.reset(new(BasicLookupTables));

		// --- our LFO waveforms //"clip sine";//
		/*
			Module Strings, zero-indexed for your GUI Control:
			- triangle, sine ,ramp_up, ramp_dn, exp_up, exp_dn, exp_tri, square, rand_SH1, pluck
		*/
		coreData.moduleStrings[0] = "triangle";	coreData.moduleStrings[8] =  "rand S&H1";
		coreData.moduleStrings[1] = "sine";		coreData.moduleStrings[9] =  "pluck";
		coreData.moduleStrings[2] = "ramp up";	coreData.moduleStrings[10] = "clip sine";
		coreData.moduleStrings[3] = "ramp dn";	coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = "exp up";	coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = "exp dn";	coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = "exp tri";	coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = "square";	coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A]	= "Shape";
		coreData.modKnobStrings[MOD_KNOB_B]	= "Delay";
		coreData.modKnobStrings[MOD_KNOB_C]	= "FadeIn";
		coreData.modKnobStrings[MOD_KNOB_D] = "BPM Sync";

	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- resets and initializes clocks and timers
	- sets initial state variables

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool LFOCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		LFOParameters* parameters = static_cast<LFOParameters*>(processInfo.moduleParameters);

		// --- store
		sampleRate = processInfo.sampleRate;

		// --- clear
		outputValue = 0.0;
		rshOutputValue = noiseGen.doWhiteNoise();
		renderComplete = false;

		// --- reset clocks and timers
		lfoClock.reset();
		sampleHoldTimer.resetTimer();
		delayTimer.resetTimer();

		// --- initialize with current value
		lfoClock.setFrequency(parameters->frequency_Hz, sampleRate);

		// --- to setup correct start phases, avoid clicks
		if (parameters->waveformIndex == enumToInt(LFOWaveform::kTriangle))
			lfoClock.addPhaseOffset(0.25);

		if (parameters->waveformIndex == enumToInt(LFOWaveform::kRampUp) || parameters->waveformIndex == enumToInt(LFOWaveform::kRampDown))
			lfoClock.addPhaseOffset(0.5);

		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- sets BPM Sync clocking if wanted
	- applies FM if needed (LFO1 can modulate LFO2's fo)
	- updates clock with current frquency
	- generates modulation values from RampModulator
	- generates timing values for sample/hold and delay

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool LFOCore::update(CoreProcData& processInfo)
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
		lfoClock.setFrequency(newFrequency_Hz, sampleRate);

		// --- update the sampleHoldTimer; this will NOT reset the timer
		if (parameters->waveformIndex == enumToInt(LFOWaveform::kRSH))
			sampleHoldTimer.setExpireSamples(uint32_t(sampleRate / newFrequency_Hz));

		// --- update the delay timer; this will NOT reset the timer
		if (!delayTimer.timerExpired())
		{
			double delay = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_B], 0.0, MAX_LFO_DELAY_MSEC);
			delayTimer.setExpireSamples(msecToSamples(sampleRate, delay));
		}

		if (fadeInModulator.isActive() && delayTimer.timerExpired())
		{
			double fadeIn_mSec = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_C], 0.0, MAX_LFO_FADEIN_MSEC);
			fadeInModulator.setModTime(fadeIn_mSec, processInfo.sampleRate);
		}
		return true;
	}

	/**
	\brief Renders the output of the module
	- write modulator output with: processInfo.modulationOutputs->setModValue( )
	Core Specific:
	- runs delay timer first
	- runs LFO next,
	- applies fade-in modulation if needed
	- then modifies outputs:
	- kLFONormalOutput normal LFO output value
	- kLFOInvertedOutput 180 degrees out of phase with normal output
	- kUnipolarFromMax unipolar version of the waveform that sits at +1 when the output amplitude is at 0,
	and descends from +1 downward as the output amplitude is increases; used for tremolo
	- kUnipolarFromMin unipolar version of the waveform that sits at 0 when the output amplitude is at 0,
	and ascends from 0 upward as the output amplitude is increases; the ordinary unipolar version

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool LFOCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		LFOParameters* parameters = static_cast<LFOParameters*>(processInfo.moduleParameters);

		// --- for debugging, you can pipe this out to your audio output and measure on scope
		// float* outputBuffer = processInfo.outputBuffers[LEFT_CHANNEL];

		// --- one shot flag
		if (renderComplete) return true;

		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			// --- delay timer
			if (!delayTimer.timerExpired())
			{
				// --- advance timer
				delayTimer.advanceTimer();

				// --- wait...
				outputValue = 0.0;
			}
			else
			{
				// --- check for completed 1-shot on this sample period
				bool bWrapped = lfoClock.wrapClock();
				if (bWrapped && parameters->modeIndex == enumToInt(LFOMode::kOneShot))
				{
					renderComplete = true;
					outputValue = 0.0;
					return renderComplete;
				}

				// --- calculate the oscillator value
				if (parameters->waveformIndex == 10)
				{
					// --- calculate normal angle
					double angle = lfoClock.mcounter*kTwoPi;
					double sine = 1.25 * sin(angle);
					boundValue(sine, -1.0, +1.0); // could also use boundBipolarValue
					outputValue = sine;
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kSin))
				{
					// --- calculate normal angle
					double angle = lfoClock.mcounter*kTwoPi - kPi;
					double sine = parabolicSine(-angle);
					outputValue = sine;
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kTriangle))
				{
					outputValue = 1.0 - 2.0*fabs(bipolar(lfoClock.mcounter));
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kExpTriangle))
				{
					outputValue = bipolar(concaveXForm(fabs(bipolar(lfoClock.mcounter)), true));
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kRampUp))
				{
					outputValue = bipolar(lfoClock.mcounter);
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kExpRampUp))
				{
					outputValue = bipolar(concaveXForm(lfoClock.mcounter, true));
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kRampDown))
				{
					outputValue = -bipolar(lfoClock.mcounter);
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kExpRampDn))
				{
					outputValue = bipolar(concaveXForm(1.0 - lfoClock.mcounter, true));
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kSquare))
				{
					outputValue = lfoClock.mcounter <= 0.5 ? +1.0 : -1.0;
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kPluck))
				{
					// --- use hann table; can experiment
					outputValue = lookupTables->readHannTableWithNormIndex(lfoClock.mcounter);
				}
				else if (parameters->waveformIndex == enumToInt(LFOWaveform::kRSH))
				{
					// --- has hold time been exceeded?
					if (sampleHoldTimer.timerExpired())
					{
						// --- if so, generate next output sample
						rshOutputValue = noiseGen.doWhiteNoise();
						sampleHoldTimer.resetTimer();
					}
					else
						sampleHoldTimer.advanceTimer();

					// --- outputValue is a member variable and persists so works for holds
					outputValue = rshOutputValue;
				}
				else // NOTE: ALL oscillators need a default!
				{
					// ---  default is triangle
					outputValue = 1.0 - 2.0*fabs(bipolar(lfoClock.mcounter));
				}

				// --- quantizer (stepper)
				if (parameters->quantize > 0)
					outputValue = quantizeBipolarValue(outputValue, parameters->quantize);

				// --- scale by amplitude
				outputValue *= parameters->outputAmplitude;

				// --- SHAPE --- //
				double shape = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_A], 0.0, 1.0);
				double shapeOut = 0.0;
				if (shape >= 0.5)
					shapeOut = bipolarConvexXForm(outputValue, true);
				else
					shapeOut = bipolarConcaveXForm(outputValue, true);

				// --- split bipolar for multiplier
				shape = splitBipolar(shape);
				outputValue = shape*shapeOut + (1.0 - shape)*outputValue;

				// --- NOTE: inside the loop, advance by 1
				lfoClock.advanceClock();
			}

			// --- set outputs: either write first output or last in block
			//
			// --- first output sample only:
			if (i == 0)
			{
				// --- FADE IN --- //
				double fadeInMod = 1.0;
				if (fadeInModulator.isActive())
				{
					fadeInMod = fadeInModulator.getNextModulationValue();
					fadeInModulator.advanceClock(processInfo.samplesToProcess);
				}
				outputValue *= fadeInMod;

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
			// --- For Debugging only, to see and test LFO
			// outputBuffer[i] = outputValue;

			//// --- NOTE: inside the loop, advance by 1
			//lfoClock.advanceClock();
		}

		return true;
	}

	/**
	\brief Note-on handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- reset delay timer and fade-in ramp modulator
	- reset sample and hold timer
	- reset timebase for next run

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool LFOCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		LFOParameters* parameters = static_cast<LFOParameters*>(processInfo.moduleParameters);

		renderComplete = false;

		if (parameters->modeIndex != enumToInt(LFOMode::kFreeRun))
		{
			lfoClock.reset();
			sampleHoldTimer.resetTimer();

			outputValue = 0.0;
			rshOutputValue = noiseGen.doWhiteNoise();

			delayTimer.resetTimer();
			double delay_mSec = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_B], 0.0, MAX_LFO_DELAY_MSEC);
			delayTimer.setExpireSamples(msecToSamples(sampleRate, delay_mSec));

			double fadeIn_mSec = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_C], 0.0, MAX_LFO_FADEIN_MSEC);
			fadeInModulator.startModulator(0.0, 1.0, fadeIn_mSec, processInfo.sampleRate);
		}

		if (parameters->waveformIndex == enumToInt(LFOWaveform::kTriangle))
			lfoClock.addPhaseOffset(0.25);

		if (parameters->waveformIndex == enumToInt(LFOWaveform::kRampUp) || parameters->waveformIndex == enumToInt(LFOWaveform::kRampDown))
			lfoClock.addPhaseOffset(0.5);

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
	bool LFOCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}

} // namespace


