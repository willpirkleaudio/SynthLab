#include "synthlabcore.h"

// -----------------------------
//	    SynthLab SDK File     //
//			   for			  //
//	      DynamicModule       //
//	       Development        //
//  ----------------------------

/**
\file   synthlabcore.cpp extracted from linearegcore.cpp
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
	- simplest of all EGs
	- only one EG contour implemented

    */
	SynthLabCore::SynthLabCore()
	{
		moduleType = EG_MODULE;
		moduleName = "LinEG";
		preferredIndex = 2; // ordering for user

		// --- our module strings
		/*
			Module Strings, zero-indexed for your GUI Control:
			- ADSR
		*/
		coreData.moduleStrings[0] = "ADSR";						coreData.moduleStrings[8] = empty_string.c_str();
		coreData.moduleStrings[1] = empty_string.c_str();		coreData.moduleStrings[9] = empty_string.c_str();
		coreData.moduleStrings[2] = empty_string.c_str();		coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = empty_string.c_str();		coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = empty_string.c_str();		coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();		coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();		coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();		coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A] = "St Lvl";
		coreData.modKnobStrings[MOD_KNOB_B] = "B";
		coreData.modKnobStrings[MOD_KNOB_C] = "C";
		coreData.modKnobStrings[MOD_KNOB_D] = "D";
	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- sets start level and state varible

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		// --- retain sample rate for update calculations
		sampleRate = processInfo.sampleRate;

		// --- reset the state
		envelopeOutput = parameters->startLevel;
		state = EGState::kOff;

		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]

	Core Specific:
	- does NOT implement the re-trigger modulation to keep it as simple as possible

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::update(CoreProcData& processInfo)
	{
        // --- parameters, uncomment if needed
        // EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		// --- sustain pedal MIDI input
		sustainOverride = processInfo.midiInputData->getCCMIDIData(SUSTAIN_PEDAL) > 63;

		if (releasePending && !sustainOverride)
		{
			releasePending = false;
			doNoteOff(processInfo);
		}

		return true;
	}

	/**
	\brief Renders the output of the module
	- write modulator output with: processInfo.modulationOutputs->setModValue( )

	Core Specific:
	- runs the finite state machine for the EG
	- runs the FSM on each sample interval, but only use the output of the first loop (see book)

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			switch (state)
			{
			case EGState::kOff:
			{
				if (!parameters->legatoMode)
					envelopeOutput = parameters->startLevel;
				break;
			}
			case EGState::kAttack:
			{
				// --- increment the output value
				envelopeOutput += egStepInc;

				// --- check for next state transistion trigger
				if (envelopeOutput >= MAX_EG_VALUE || attackTimeScalar*parameters->attackTime_mSec <= 0.0)
				{
					// --- clamp max value
					envelopeOutput = MAX_EG_VALUE;

					// --- calculate decay step, which decreases (-1.0)
					double scale = -1.0;
					setStepInc(scale * decayTimeScalar * parameters->decayTime_mSec); // going down

					// --- GOTO next state
					state = EGState::kDecay;
				}
				break;
			}
			case EGState::kDecay:
			{
				// --- linear step
				envelopeOutput += egStepInc;

				// --- state transition
				if (envelopeOutput <= parameters->sustainLevel || decayTimeScalar * parameters->decayTime_mSec <= 0.0)
				{
					envelopeOutput = parameters->sustainLevel;
					state = EGState::kSustain;
				}

				break;
			}

			case EGState::kSustain:
			{
				envelopeOutput = parameters->sustainLevel;
				break;
			}

			case EGState::kRelease: // note off sets this
			{
				// --- step is calculated in note-off
				envelopeOutput += egStepInc;

				if (envelopeOutput <= 0.0 || parameters->releaseTime_mSec <= 0.0)
				{
					envelopeOutput = 0.0;
					state = EGState::kOff;
				}
				break;
			}

			case EGState::kShutdown:
			{
				// --- the shutdown state is just a linear taper since it is so short
				envelopeOutput += incShutdown;

				// --- check go to next state
				if (envelopeOutput <= 0)
				{
					state = EGState::kOff;		// go to next state
					envelopeOutput = 0.0;		// reset envelope
					break;
				}
				break; // this is needed!!
			}

			default:
				break;
			}

			if (i == 0)
			{
				processInfo.modulationOutputs->setModValue(kEGNormalOutput, envelopeOutput);
				processInfo.modulationOutputs->setModValue(kEGBiasedOutput, envelopeOutput - parameters->sustainLevel);
			}
		}

		return true;
	}


	/**
	\brief Note-on handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- sets the finite state machine for another run
	- checks attack and decay scaling (only caclulated once per note-event)

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		// --- A is start level, defaults to 0.5!
		parameters->startLevel = parameters->modKnobValue[MOD_KNOB_A];

		// --- reset the state
		if (!parameters->legatoMode)
			envelopeOutput = parameters->startLevel;

		// --- normalize MIDI velocity and invert
		//     MIDI Velocity = 0   --> no change to attack, scalar = 1.0
		//     MIDI Velocity = 127 --> attack time is now 0mSec, scalar = 0.0
		if (parameters->velocityToAttackScaling)
			attackTimeScalar = 1.0 - (double)processInfo.noteEvent.midiNoteVelocity / 127.0;
		else
			attackTimeScalar = 1.0;

		if (parameters->noteNumberToDecayScaling)
			decayTimeScalar = 1.0 - (double)processInfo.noteEvent.midiNoteNumber / 127.0;
		else
			decayTimeScalar = 1.0;

		// --- clamp max and calculate scale for non-zero start levels
		clampMaxValue(parameters->startLevel, MAX_EG_VALUE);
		double scale = MAX_EG_VALUE - parameters->startLevel;

		// --- set the attack step size, back up the EG
		setStepInc(scale * attackTimeScalar * parameters->attackTime_mSec);

		// --- reset the start point
		if (!parameters->legatoMode)
		{
			// --- reset
			envelopeOutput = parameters->startLevel - egStepInc; // back it up

			// --- go to the attack state
			state = EGState::kAttack;
		}
		else if (state == EGState::kOff)
			state = EGState::kAttack; // else stay in state

		return true;
	}


	/**
	\brief Note-off handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- checks the sustain override flag and releases the note if needed
	- goes to release or off mode

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::doNoteOff(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		if (sustainOverride)
		{
			// --- set releasePending flag
			releasePending = true;
			return true;
		}

		double scale = -1.0;
		setStepInc(scale * parameters->releaseTime_mSec);

		// --- go directly to release state
		if (envelopeOutput > 0)
			state = EGState::kRelease;
		else // sustain was already at zero
			state = EGState::kOff;

		return true; // handled
	}

	/**
	\brief Shutdown handler for EG
	- all EGs should implement this base class function
	- calculates shutdown amp increment based on current output value

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::shutdown()
	{
		// --- calculate the linear inc values based on current outputs
		incShutdown = -(1000.0*envelopeOutput) / SHUTDOWN_TIME_MSEC / sampleRate;

		// --- set state and reset counter
		state = EGState::kShutdown;

		// --- for sustain pedal
		sustainOverride = false;
		releasePending = false;
		return true;
	}

	/**
	\brief Sustain pedal handler for EG
	- all EGs should implement this base class function
	- if turned off (pedal released), clears the pending release if exists
	- if turned on, sets the sustain pedal override to keep the EG stuck in the sustain state
	until the pedal is released

    */
	void SynthLabCore::setSustainOverride(bool b)
	{
		sustainOverride = b;

		if (releasePending && !sustainOverride)
		{
			releasePending = false;
			MIDINoteEvent noteEvent;
			CoreProcData cpd;
			doNoteOff(cpd);
		}
	}

	/**
	\brief Calculates linear step increment based on time in millisecs

	\param timeMsec the time in milliseconds
	\scale scalar value for stretching or shrinking the final value

	\returns new step increment value
	*/
	double  SynthLabCore::setStepInc(double timeMsec, double scale)
	{
		// --- this can happen if any segment has 0.0 time (instant anything)
		if (timeMsec == 0.0 || sampleRate == 0.0)
			return 0.0;

		egStepInc = scale * (1000.0 / (timeMsec*sampleRate));
		return  egStepInc;
	}

} // namespace


