#include "synthlabcore.h"

// -----------------------------
//	    SynthLab SDK File     //
//			   for			  //
//	      DynamicModule       //
//	       Development        //
//  ----------------------------

/**
\file   synthlabcore.cpp extracted from dxegcore.cpp
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
	- two EG contours are supported

	*/
	SynthLabCore::SynthLabCore()
	{
		moduleType = EG_MODULE;
		moduleName = "DX-EG";
		preferredIndex = 1; // ordering for user, DM only

		/*
			Module Strings, zero-indexed for your GUI Control:
			- ADSlSR, ADSlR
		*/
		coreData.moduleStrings[0] = "ADSlSR";					coreData.moduleStrings[8] =  empty_string.c_str();
		coreData.moduleStrings[1] = "ADSlR";					coreData.moduleStrings[9] =  empty_string.c_str();
		coreData.moduleStrings[2] = empty_string.c_str();		coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = empty_string.c_str();		coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = empty_string.c_str();		coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();		coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();		coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();		coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A] = "St Lvl";
		coreData.modKnobStrings[MOD_KNOB_B] = "Dcy Lvl";
		coreData.modKnobStrings[MOD_KNOB_C] = "Slope";
		coreData.modKnobStrings[MOD_KNOB_D] = "Curve";
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
	bool SynthLabCore::reset(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		// --- retain sample rate for update calculations
		sampleRate = processInfo.sampleRate;

		// --- reset the state
		envelopeOutput = parameters->startLevel;
		state = EGState::kOff;
		noteOff = false; // for retriggering EG
		lastTriggerMod = 0.0;
		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- is used in a standalone manner with the FMOperator object;
	the parameter messaging is direct, bypassing mod knobs
	- checks the re-trigger modulation input to alter state machine

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool SynthLabCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		if (!standAloneMode)
		{
			// --- B is decay level
			parameters->decayLevel = parameters->modKnobValue[MOD_KNOB_B];

			// --- C is slope time, 0 -> 5 seconds
			parameters->slopeTime_mSec = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_C], 0.0, 10000.0);

			// --- D is curvature
			parameters->curvature = parameters->modKnobValue[MOD_KNOB_D];
		}

		// --- this is to prevent a glitch if the slope time is zero
		//     the glitch is NOT a bug, but the user may think so...
		if (parameters->slopeTime_mSec <= 0.1)
			parameters->decayLevel = parameters->sustainLevel;

		if (parameters->decayLevel > 0.9)
			parameters->decayLevel = 0.9;

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
	bool SynthLabCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		EGParameters* parameters = static_cast<EGParameters*>(processInfo.moduleParameters);

		// -- curved output
		double curveEGValue = 0.0;

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

					// --- create the curved version
					curveEGValue = convexXForm(envelopeOutput, true);

					if (retriggered)retriggered = false;

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

					// --- find the decay level on the inverse curve
					double dcyLvl = reverseConcaveXForm(parameters->decayLevel, true);

					// --- setup the mapping
					double map = envelopeOutput;
					mapDoubleValue(map, parameters->decayLevel, 1.0, dcyLvl, 1.0);

					// --- used mapped value on curve section
					curveEGValue = concaveXForm(map);

					// --- state transition
					if (envelopeOutput <= parameters->decayLevel || decayTimeScalar * parameters->decayTime_mSec <= 0.0)
					{
						double scale = parameters->decayLevel < parameters->sustainLevel ? +1.0 : -1.0;
						setStepInc(scale * parameters->slopeTime_mSec);
						envelopeOutput = parameters->decayLevel;
						state = EGState::kSlope;
					}

					break;
				}

				case EGState::kSlope:
				{
					envelopeOutput += egStepInc;
					curveEGValue = envelopeOutput;
					// --- for negative slope
					if (parameters->slopeTime_mSec <= 0.0 || (egStepInc < 0.0 && envelopeOutput <= parameters->sustainLevel))
					{
						envelopeOutput = parameters->sustainLevel;
						if (parameters->egContourIndex == enumToInt(DXEGContour::kADSlR))
						{
							setStepInc(-parameters->releaseTime_mSec);
							state = EGState::kRelease;
						}
						else
							state = EGState::kSustain;
					}
					// --- for positive slope
					else if (parameters->slopeTime_mSec <= 0.0 || (egStepInc > 0.0 && envelopeOutput >= parameters->sustainLevel))
					{
						envelopeOutput = parameters->sustainLevel;
						if (parameters->egContourIndex == enumToInt(DXEGContour::kADSlR))
						{
							setStepInc(-parameters->releaseTime_mSec);
							state = EGState::kRelease;
						}
						else
							state = EGState::kSustain;
					}
					break;
				}

				case EGState::kSustain:
				{
					envelopeOutput = parameters->sustainLevel;
					curveEGValue = parameters->sustainLevel;
					break;
				}

			case EGState::kRelease: // note off sets this
			{
				// --- step is calculated in note-off
				envelopeOutput += egStepInc;

				// --- apply curve
				double find = reverseConcaveXForm(releaseLevel/*parameters->sustainLevel*/, true);
				double map1 = envelopeOutput;
				mapDoubleValue(map1, 0.0, releaseLevel/*parameters->sustainLevel*/, 0.0, find);
				curveEGValue = concaveXForm(map1);

				// --- check go to next state
				if (envelopeOutput <= 0.0 || parameters->releaseTime_mSec <= 0.0)
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

			linearEnvOutput = envelopeOutput;	///< current outupt
			curveEnvOutput = curveEGValue;	///< current outupt

			if (i == 0)
			{
				dxOutput = parameters->curvature*curveEGValue + (1.0 - parameters->curvature)*envelopeOutput;
				processInfo.modulationOutputs->setModValue(kEGNormalOutput, dxOutput);
				processInfo.modulationOutputs->setModValue(kEGBiasedOutput, dxOutput - parameters->sustainLevel);
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

		noteOff = false;
		retriggered = false;
		lastTriggerMod = 0.0;
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
		releaseLevel = dxOutput; // track last output

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

