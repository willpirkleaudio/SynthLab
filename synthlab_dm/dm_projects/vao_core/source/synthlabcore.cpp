#include "synthlabcore.h"

// -----------------------------
//	    SynthLab SDK File     //
//			   for			  //
//	      DynamicModule       //
//	       Development        //
//  ----------------------------

/**
\file   synthlabcore.cpp extracted from vaocore.cpp
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
	- the one and only VA core object
	- uses BLEP correction
	*/
	SynthLabCore::SynthLabCore()
	{
		moduleType = VAO_MODULE;
		moduleName = "SynLabVAO";
		preferredIndex = 0; // ordering for user

		/*
			Module Strings, zero-indexed for your GUI Control:
			- saw/sqr, sawtooth, square
		*/
		coreData.moduleStrings[0] = "saw/sqr";				coreData.moduleStrings[8] =  empty_string.c_str();
		coreData.moduleStrings[1] = "sawtooth";				coreData.moduleStrings[9] =  empty_string.c_str();
		coreData.moduleStrings[2] = "square";				coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = empty_string.c_str();	coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = empty_string.c_str();	coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();	coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();	coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();	coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A]	= "WaveMix";
		coreData.modKnobStrings[MOD_KNOB_B] = "PulseWidth";
		coreData.modKnobStrings[MOD_KNOB_C]	= "C";
		coreData.modKnobStrings[MOD_KNOB_D] = "D";
	}


	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- initialize timbase

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		VAOscParameters* parameters = static_cast<VAOscParameters*>(processInfo.moduleParameters);

		sampleRate = processInfo.sampleRate;

		if (parameters->waveIndex == enumToInt(VAWaveform::kSawtooth))
		{
			oscClock.reset(0.5); // 0.5 for saw
		}
		else // all others
		{
			oscClock.reset(0.0);
		}

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
	- calculates pulse width information (unique modulator for this oscillator)
	- calculates final gain and pan values

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		VAOscParameters* parameters = static_cast<VAOscParameters*>(processInfo.moduleParameters);

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
		boundValue(oscillatorFrequency, VA_OSC_MIN, VA_OSC_MAX);

		// --- phase inc = fo/fs
		oscClock.setFrequency(oscillatorFrequency, sampleRate);

		// --- scale from dB
		outputAmplitude = dB2Raw(parameters->outputAmplitude_dB);

		// --- pan
		double panTotal = parameters->panValue;// +processInfo.modulationInputs[kAuxBPMod_1];
		boundValueBipolar(panTotal);

		// --- equal power calculation in synthfunction.h
		calculatePanValues(panTotal, panLeftGain, panRightGain);

		// --- pulse width from ModKnob
		//     note the way this works 0.0 -> 50% PW  1.0 -> 95% PW
		pulseWidth = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_B], VA_MIN_PW, VA_MAX_PW);

		// --- this value is bipolar in nature
		double pwModulator = processInfo.modulationInputs->getModValue(kUniqueMod);
		pwModulator = fabs(pwModulator) * PW_MOD_RANGE; // effectively double the LFO rate because of the way PW works
		pulseWidth += pwModulator;

		// --- bound it
		boundValue(pulseWidth, VA_MIN_PW, VA_MAX_PW);

		return true;
	}


	/**
	\brief Renders a BLEP sawtooth sample
	Core Specific:
	- note the check to see how many points may be safely calculated per side of discontinuity
	- use trivial oscillator output and correct with BLEP

	\param clock the current timebase
	\param advanceClock true to advance clock after render

	\returns true if successful, false otherwise
	*/
	double SynthLabCore::renderSawtoothSample(SynthClock& clock, bool advanceClock)
	{
		// --- NOTE:
		uint32_t pointsPerSide = 0;
		if (clock.frequency_Hz <= sampleRate / 8.0) // Fs/8 = Nyquist/4
			pointsPerSide = 4;
		else if (clock.frequency_Hz <= sampleRate / 4.0) // Fs/4 = Nyquist/2
			pointsPerSide = 2;
		else // Nyquist
			pointsPerSide = 1;

		// --- setup output
		// --- first create the trivial saw from modulo counter
		double sawOut = bipolar(clock.mcounter);

		// --- setup for BLEP correction
		double blepCorrection = 0.0;
		uint32_t tableLen = 4096;
		double edgeHeight = 1.0;

		blepCorrection = doBLEP_N(tableLen,					/* BLEP table length */
									clock.mcounter,			/* current phase value */
									fabs(clock.phaseInc),	/* abs(phaseInc) is for FM synthesis with negative frequencies */
									edgeHeight,				/* sawtooth edge height = 1.0 */
									false,					/* falling edge */
									pointsPerSide,			/* N points per side */
									false);					/* no interpolation */

		// --- add the correction factor
		sawOut += blepCorrection;

		// --- setup for next sample
		if(advanceClock)
			clock.advanceWrapClock();

		return sawOut;
	}

	/**
	\brief Renders a BLEP squarewave sample with sum-of-saws methog
	Core Specific:
	- see synth book for details on sum-of-saws

	\param clock the current timebase

	\returns true if successful, false otherwise
	*/
	double SynthLabCore::renderSquareSample(SynthClock& clock, double& sawtoothSample)
	{
		// --- sum-of-saws method
		// --- set first sawtooth output: false = do not advance the clock
		sawtoothSample = renderSawtoothSample(clock, false);

		// --- phase shift on second oscillator
		clock.addPhaseOffset(pulseWidth);

		// --- generate 2nd saw: false = do not advance the clock
		double saw2 = renderSawtoothSample(clock, false);

		// --- subtract = 180 out of phase
		double squareOut = 0.5*sawtoothSample - 0.5*saw2;

		// --- apply DC correction
		double dcCorrection = 1.0 / pulseWidth;

		// --- modfiy for less than 50%
		if (pulseWidth < 0.5)
			dcCorrection = 1.0 / (1.0 - pulseWidth);

		// --- apply correction
		squareOut *= dcCorrection;

		// --- restore original clock state
		clock.removePhaseOffset();

		// --- *now* advance the clock
		clock.advanceWrapClock();

		return squareOut;
	}

	/**
	\brief Renders the output of the module
	- renders to output buffer using pointers in the CoreProcData argument
	- supports FM via the pmBuffer if avaialble; if pmBuffer is NULL then there is no FM
	Core Specific:
	- call rendering sub-function
	- allows mix of square and saw (a la Oberhiem SEM)

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		VAOscParameters* parameters = static_cast<VAOscParameters*>(processInfo.moduleParameters);

		float* leftOutBuffer = processInfo.outputBuffers[LEFT_CHANNEL];
		float* rightOutBuffer = processInfo.outputBuffers[RIGHT_CHANNEL];

		// --- render square and saw at same time
		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			double oscOutput = 0.0;

			// --- render both saw and square always, choose output with param
			double sawOutput = 0.0;
			double sqrOutput = renderSquareSample(oscClock, sawOutput);

			if (parameters->waveIndex == enumToInt(VAWaveform::kSawAndSquare))
			{
				// --- blend with mod knob
				double mix = parameters->modKnobValue[MOD_KNOB_A];

				// --- blend
				oscOutput = sawOutput*(1.0 - mix) + sqrOutput*mix;
			}
			else if (parameters->waveIndex == enumToInt(VAWaveform::kSawtooth))
				oscOutput = sawOutput;
			else if (parameters->waveIndex == enumToInt(VAWaveform::kSquare))
				oscOutput = sqrOutput;

			// --- scale
			oscOutput *= outputAmplitude;

			// --- write to output buffers
			leftOutBuffer[i] = oscOutput * panLeftGain;
			rightOutBuffer[i] = oscOutput * panRightGain;
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
	- resets clock, sets phase depending on waveform

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/bool SynthLabCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		VAOscParameters* parameters = static_cast<VAOscParameters*>(processInfo.moduleParameters);

		// --- parameters
		midiPitch = processInfo.noteEvent.midiPitch;

		// --- reset to new start phase
		if (parameters->waveIndex == enumToInt(VAWaveform::kSawtooth))
		{
			if (processInfo.unisonStartPhase > 0.0)
				oscClock.reset(0.5 + processInfo.unisonStartPhase / 720.0);
			else
				oscClock.reset(0.5); // 0.5 for saw
		}
		else // all others
		{
			if (processInfo.unisonStartPhase > 0.0)
				oscClock.reset(processInfo.unisonStartPhase / 360.0);
			else
				oscClock.reset(0.0);
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


