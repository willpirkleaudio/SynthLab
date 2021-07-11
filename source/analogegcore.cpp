
#include "analogegcore.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   analogcore.cpp
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
	- implements two EG contours, ADSR and AR
	- you can add more here
	- sets up attack and release time constants
	based on analog capacitor charge/discharge rates

	*/
	AnalogEGCore::AnalogEGCore()
	{
		moduleType = EG_MODULE;
		moduleName = "AnalogEG";
		preferredIndex = 0; // ordering for user, DM only

		// --- our strings
		/*
			Module Strings, zero-indexed for your GUI Control:
			- ADSR, AR
		*/
		coreData.moduleStrings[0] = "ADSR";			coreData.moduleStrings[8] =  empty_string.c_str();
		coreData.moduleStrings[1] = "AR";			coreData.moduleStrings[9] =  empty_string.c_str();
		coreData.moduleStrings[2] = empty_string.c_str();		coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = empty_string.c_str();		coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = empty_string.c_str();		coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();		coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();		coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();	coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A] = "St Lvl";
		coreData.modKnobStrings[MOD_KNOB_B] = "B";
		coreData.modKnobStrings[MOD_KNOB_C] = "C";
		coreData.modKnobStrings[MOD_KNOB_D] = "D";

		// --- analog - use e^-1.5x, e^-4.95x
		attackTCO = exp(-1.5);  // fast attack
		decayTCO = exp(-4.95);
		releaseTCO = decayTCO;
	}

	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- initializes sample rate dependent stuff via processInfo.sampleRate
	- calculates intitial coefficients
	- sets initial state variables

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool AnalogEGCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		// --- wave tables are fs based
		bool bNewSR = processInfo.sampleRate != sampleRate ? true : false;

		// --- retain sample rate for update calculations
		sampleRate = processInfo.sampleRate;

		// --- recrate the tables only if sample rate has changed
		if (bNewSR)
		{
			// --- recalc these, SR dependent
			calcAttackCoeff(parameters->attackTime_mSec);
			calcDecayCoeff(parameters->decayTime_mSec);
			calcReleaseCoeff(parameters->releaseTime_mSec);
		}

		// --- reset the state
		envelopeOutput = parameters->startLevel;
		state = EGState::kOff;
		noteOff = false;
		retriggered = false;
		lastTriggerMod = 0.0;
		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- recalculates attack/decay/release coefficients if needed
	- checks the re-trigger modulation input to alter state machine

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool AnalogEGCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		// --- save for future events
		parameters->startLevel = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_A], -1.0, +1.0);

		// --- sustain level first: other calculations depend on it
		bool sustainUpdate = false;
		if (sustainLevel != parameters->sustainLevel)
			sustainUpdate = true;

		sustainLevel = parameters->sustainLevel;

		if (attackTime_mSec != parameters->attackTime_mSec)
			calcAttackCoeff(parameters->attackTime_mSec);

		if (sustainUpdate || (decayTime_mSec != parameters->decayTime_mSec))
			calcDecayCoeff(parameters->decayTime_mSec);

		if (releaseTime_mSec != parameters->releaseTime_mSec)
			calcReleaseCoeff(parameters->releaseTime_mSec);

		// --- sustain pedal MIDI input
		if(processInfo.midiInputData)
			sustainOverride = processInfo.midiInputData->getCCMIDIData(SUSTAIN_PEDAL) > 63;

		if (releasePending && !sustainOverride)
		{
			releasePending = false;
			doNoteOff(processInfo);
		}
		else if (!noteOff)// process retriggering
		{
			// --- retrigger on low-high transition across 0.5
			//     then, ignore trigger mod when value is > 0.5
			//     NOTE: this is a different strategy from the published book version
			double retrig = processInfo.modulationInputs->getModValue(kTriggerMod);
			if (retrig > 0.5 && lastTriggerMod <= 0.5)
			{
				// --- to to release state (or attack state, to match the book - your choice)
				state = EGState::kRelease;
				retriggered = true; // flag for release state to know to re-trigger to attack state
			}
			// --- need for edge detection
			lastTriggerMod = retrig;
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
	bool AnalogEGCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			// --- decode the state
			switch (state)
			{
                case EGState::kOff:
                {
                    // --- if not legato, reset to start level
                    if (!parameters->legatoMode)
                        envelopeOutput = parameters->startLevel;

                    break;
                }

                case EGState::kAttack:
                {
                    // --- render value
                    envelopeOutput = attackOffset + envelopeOutput*attackCoeff;
                    if (retriggered)retriggered = false;

                    // --- check go to next state
                    if (envelopeOutput >= 1.0 || attackTime_mSec <= 0.0)
                    {
                        envelopeOutput = 1.0;
                        if (parameters->egContourIndex == enumToInt(AnalogEGContour::kADSR))
                            state = EGState::kDecay;	// go to kDecay
                        else if (parameters->egContourIndex == enumToInt(AnalogEGContour::kAR))
                            state = EGState::kRelease;	// go to relase

                        break;
                    }
                    break;
                }

                case EGState::kDecay:
                {
                    // --- render value
                    envelopeOutput = decayOffset + envelopeOutput*decayCoeff;

                    // --- check go to next state
                    if (envelopeOutput <= sustainLevel || decayTime_mSec <= 0.0)
                    {
                        envelopeOutput = sustainLevel;
                        state = EGState::kSustain;		// go to next state
                        break;
                    }
                    break;
                }

                case EGState::kSustain:
                {
                    // --- render value
                    envelopeOutput = sustainLevel;
                    break;
                }

                case EGState::kRelease:
                {
                    // --- if sustain pedal is down, override and return
                    if (sustainOverride)
                    {
                        // --- leave envelopeOutput at whatever value is currently has
                        //     this is in case the user hits the sustain pedal after the note is released
                        break;
                    }
                    else
                        // --- render value
                        envelopeOutput = releaseOffset + envelopeOutput*releaseCoeff;

                    // --- check go to next state
                    if (envelopeOutput <= 0.0 || releaseTime_mSec <= 0.0)
                    {
                        if (retriggered)
                        {
                            envelopeOutput = parameters->startLevel;
                            state = EGState::kAttack;
                        }
                        else
                        {
                            envelopeOutput = 0.0;
                            state = EGState::kOff;			// go to OFF state
                        }
                        break;
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
                        state = EGState::kOff;	// go to next state
                        envelopeOutput = 0.0;		// reset envelope
                        break;
                    }
                    break; // this is needed!!
                }

                case EGState::kDelay:
                case EGState::kHold:
                case EGState::kSlope:
                    break; // not used in this EG

            }

			// --- load up the output on first sample
			if (i == 0)
			{
				processInfo.modulationOutputs->setModValue(kEGNormalOutput, envelopeOutput);
				processInfo.modulationOutputs->setModValue(kEGBiasedOutput, envelopeOutput - sustainLevel);
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
	bool AnalogEGCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		// --- reset the start point
		if (!parameters->legatoMode)
		{
			// --- reset
			envelopeOutput = parameters->startLevel;

			// --- go to the attack state
			state = EGState::kAttack;
		}
		else if (state == EGState::kOff)
			state = EGState::kAttack; // else stay in state


		// --- apply MIDI based modulators - note that this only needs to happen once
		//     and did not belong in old modulation matrix
		if (parameters->velocityToAttackScaling)
		{
			// --- normalize MIDI velocity and invert
			//     MIDI Velocity = 0   --> no change to attack, scalar = 1.0
			//     MIDI Velocity = 127 --> attack time is now 0mSec, scalar = 0.0
			double scalar = 1.0 - (double)processInfo.noteEvent.midiNoteVelocity / 127.0;
			calcAttackCoeff(parameters->attackTime_mSec, scalar);
		}
		if (parameters->noteNumberToDecayScaling)
		{
			// --- normalize MIDI velocity and invert
			//     MIDI Note 0   --> no change to decay, scalar = 1.0
			//     MIDI Note 127 --> decay time is now 0mSec, scalar = 0.0
			double scalar = 1.0 - (double)processInfo.noteEvent.midiNoteNumber / 127.0;
			calcDecayCoeff(parameters->decayTime_mSec, scalar);
		}
		retriggered = false;
		lastTriggerMod = 0.0;
		noteOff = false;
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
	bool AnalogEGCore::doNoteOff(CoreProcData& processInfo)
	{
		if (sustainOverride)
		{
			// --- set releasePending flag
			releasePending = true;
			return true;
		}

		// --- go directly to release state
		if (envelopeOutput > 0)
			state = EGState::kRelease;
		else // sustain was already at zero
			state = EGState::kOff;

		noteOff = true;
		return true; // handled
	}

	/**
	\brief Shutdown handler for EG
	- all EGs should implement this base class function
	- calculates shutdown amp increment based on current output value

	\returns true if successful, false otherwise
	*/
	bool AnalogEGCore::shutdown()
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
	void AnalogEGCore::setSustainOverride(bool b)
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
} // namespace


