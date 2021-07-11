#include "sequencer.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   sequencer.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs Wave Sequencer Object
	- See class declaration for information on standalone operation
	- initializes the lanes and setsup for first run

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	WaveSequencer::WaveSequencer(std::shared_ptr<MidiInputData> _midiInputData, 
		std::shared_ptr<WaveSequencerParameters> _parameters, 
		uint32_t blockSize)
		: SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		for (uint32_t i = 0; i < MAX_SEQ_STEPS; i++)
		{
			// --- setup first loop points
			if (i == 0)
				timingLane.laneStep[i].setPreviousStepIndex(MAX_SEQ_STEPS - 1);
			else
				timingLane.laneStep[i].setPreviousStepIndex(-1);
			if (i == MAX_SEQ_STEPS - 1)
				timingLane.laneStep[i].setNextStepIndex(0);
			else
				timingLane.laneStep[i].setNextStepIndex(-1);

			// --- setup first loop points
			if (i == 0)
				waveLane.laneStep[i].setPreviousStepIndex(MAX_SEQ_STEPS - 1);
			else
				waveLane.laneStep[i].setPreviousStepIndex(-1);
			if (i == MAX_SEQ_STEPS - 1)
				waveLane.laneStep[i].setNextStepIndex(0);
			else
				waveLane.laneStep[i].setNextStepIndex(-1);

			if (i == 0)
				pitchLane.laneStep[i].setPreviousStepIndex(MAX_SEQ_STEPS - 1);
			else
				pitchLane.laneStep[i].setPreviousStepIndex(-1);
			if (i == MAX_SEQ_STEPS - 1)
				pitchLane.laneStep[i].setNextStepIndex(0);
			else
				pitchLane.laneStep[i].setNextStepIndex(-1);

			if (i == 0)
				stepSeqLane.laneStep[i].setPreviousStepIndex(MAX_SEQ_STEPS - 1);
			else
				stepSeqLane.laneStep[i].setPreviousStepIndex(-1);
			if (i == MAX_SEQ_STEPS - 1)
				stepSeqLane.laneStep[i].setNextStepIndex(0);
			else
				stepSeqLane.laneStep[i].setNextStepIndex(-1);
		}

		// srand(time(NULL));
		sampleCounter = 0;
	}

	/**
	\brief Resets object to initialized state
	- call once during initialization
	- call any time sample rate changes (after init)
	- resets step index values

	\param _sampleRate the current sample rate in Hz

	\returns true if successful, false otherwise
	*/
	bool WaveSequencer::reset(double _sampleRate)
	{
		// --- randomize on each reset and start?
		// srand(time(NULL));
		uint32_t currentStepIndex = 0;
		uint32_t nextStepIndex = 0;
		sampleCounter = 0;

		sampleRate = _sampleRate;
		samplesPerMSec = sampleRate / 1000.0;
		return true;
	}

	
	/**
	\brief Updates the selected core; sets GLOBAL engine variable unisonDetuneCents
	that may have changed since last operation

	\returns true if successful, false otherwise

	//	enum { WAVE_LANE, PITCH_LANE, STEP_SEQ_LANE, NUM_MOD_LANES };
	*/
	bool WaveSequencer::update()
	{
		// --- fd-bckwd does not change anything here; happens on loop boundaries
		if (parameters->timingLoopDirIndex == enumToInt(LoopDirection::kForward))
			timingLane.forwardDirection = true;
		else if (parameters->timingLoopDirIndex == enumToInt(LoopDirection::kBackward))
			timingLane.forwardDirection = false;

		if (parameters->modLoopDirIndex[WAVE_LANE] == enumToInt(LoopDirection::kForward))
			waveLane.forwardDirection = true;
		else if (parameters->modLoopDirIndex[WAVE_LANE] == enumToInt(LoopDirection::kBackward))
			waveLane.forwardDirection = false;

		if (parameters->modLoopDirIndex[PITCH_LANE] == enumToInt(LoopDirection::kForward))
			pitchLane.forwardDirection = true;
		else if (parameters->modLoopDirIndex[PITCH_LANE] == enumToInt(LoopDirection::kBackward))
			pitchLane.forwardDirection = false;

		if (parameters->modLoopDirIndex[STEP_SEQ_LANE] == enumToInt(LoopDirection::kForward))
			stepSeqLane.forwardDirection = true;
		else if (parameters->modLoopDirIndex[STEP_SEQ_LANE] == enumToInt(LoopDirection::kBackward))
			stepSeqLane.forwardDirection = false;

		double timeStretchShrink = 1.0;
		double paramStretch = parameters->timeStretch;
		mapDoubleValue(paramStretch, -5.0, +5.0, 1.0, 9.0);

		if (paramStretch >= 5.0)
			timeStretchShrink = paramStretch - 5.0 + 1.0;
		else
			timeStretchShrink = 1.0 / (5.0 - paramStretch + 1.0);

		for (uint32_t i = 0; i < MAX_SEQ_STEPS; i++)
		{
			// --- durations to msec
			parameters->stepDurationMilliSec[i] = getTimeFromTempo(parameters->BPM, convertIntToEnum(parameters->stepDurationNoteIndex[i], NoteDuration), true);
			parameters->xfadeDurationMilliSec[i] = getTimeFromTempo(parameters->BPM, convertIntToEnum(parameters->xfadeDurationNoteIndex[i], NoteDuration), true);
		
			parameters->stepDurationMilliSec[i] *= timeStretchShrink;
			parameters->xfadeDurationMilliSec[i] *= timeStretchShrink;

			// --- not sure if needed
			timingLane.laneStep[i].stepDurationNote = convertIntToEnum(parameters->stepDurationNoteIndex[i], NoteDuration);
			timingLane.laneStep[i].xfadeDurationNote = convertIntToEnum(parameters->xfadeDurationNoteIndex[i], NoteDuration);

			// --- millisecond to samples
			timingLane.laneStep[i].updateStepDurationSamples(parameters->stepDurationMilliSec[i] * samplesPerMSec);
			timingLane.laneStep[i].updateStepXFadeSamples(parameters->xfadeDurationMilliSec[i] * samplesPerMSec);
			
			// --- all other lanes
			//
			// --- WAVE_LANE
			waveLane.laneStep[i].setStepValue(parameters->waveLaneValue[i]);
			waveLane.laneStep[i].probability_Pct = parameters->waveLaneProbability_pct[i];

			// --- PITCH_LANE
			pitchLane.laneStep[i].setStepValue(parameters->pitchLaneValue[i]);
			pitchLane.laneStep[i].probability_Pct = parameters->pitchLaneProbability_pct[i];

			// --- STEP_SEQ_LANE
			stepSeqLane.laneStep[i].setStepValue(parameters->stepSeqValue[i]);
			stepSeqLane.laneStep[i].probability_Pct = parameters->stepSeqProbability_pct[i];
		}

		// --- timing loop convert to index values here
		timingLane.startPoint = parameters->timingLoopStart - 1;
		timingLane.endPoint = parameters->timingLoopEnd - 1;
		timingLane.setRandomizeSteps(parameters->randomizeStepOrder);
		timingLane.updateLaneLoopPoints();

		// --- timing loop convert to index values here
		waveLane.startPoint = parameters->modLoopStart[WAVE_LANE] - 1;
		waveLane.endPoint = parameters->modLoopEnd[WAVE_LANE] - 1;
        waveLane.setRandomizeSteps(parameters->randomizeWaveOrder);
		waveLane.updateLaneLoopPoints();

		// --- timing loop convert to index values here
		pitchLane.startPoint = parameters->modLoopStart[PITCH_LANE] - 1;
		pitchLane.endPoint = parameters->modLoopEnd[PITCH_LANE] - 1;
		pitchLane.setRandomizeSteps(parameters->randomizePitchOrder);
		pitchLane.updateLaneLoopPoints();

		// --- timing loop convert to index values here
		stepSeqLane.startPoint = parameters->modLoopStart[STEP_SEQ_LANE] - 1;
		stepSeqLane.endPoint = parameters->modLoopEnd[STEP_SEQ_LANE] - 1;
		stepSeqLane.setRandomizeSteps(parameters->randomizeSSModOrder);
		stepSeqLane.updateLaneLoopPoints();

		waveLane.updateStepValues();
		pitchLane.updateStepValues();
		stepSeqLane.updateStepValues();

		return true;
	}

	/**
	\brief
		clear out LED (flashing light) status array
	*/
	void WaveSequencer::clearStatusArray()
	{
		for (uint32_t i = 0; i < MAX_SEQ_STEPS; i++)
		{
			parameters->statusMeters.timingLaneMeter[i] = 0;
			parameters->statusMeters.waveLaneMeter[i] = 0;
			parameters->statusMeters.pitchLaneMeter[i] = 0;
			parameters->statusMeters.stepSeqLaneMeter[i] = 0;
		}
	}

	/**
	\brief
		Set the current cross fade time in samples
	*/
	uint32_t WaveSequencer::setCurrentTimingXFadeSamples()
	{
		// --- max is 2x the shorter step length // Korg
		// --- max is 1x the shorter step length // me, simpler thatn overlapping
		int maxXfade = 1.0 * (fmin(timingLane.currentStep.stepDurationSamplesRunning, timingLane.nextStep.stepDurationSamplesRunning));
		timingLane.currentStep.xfadeDurationSamplesRunning = fmin(maxXfade, timingLane.currentStep.xfadeDurationSamplesRunning);
		return timingLane.currentStep.xfadeDurationSamplesRunning;
	}

	/**
	\brief
		Set the current hold time in samples
	*/
	void WaveSequencer::setXFadeHoldParams(uint32_t xfadeInTimeSamples)
	{
		// --- first hold time is different
		int32_t holdSamples = (timingLane.currentStep.stepDurationSamplesRunning - xfadeInTimeSamples) - (timingLane.currentStep.xfadeDurationSamplesRunning / 2);
		holdSamples = fmax(0, holdSamples);

		// --- setup xfader for fade-in only
		xHoldFader.setHoldTimeSamples(holdSamples);
		xHoldFader.setXFadeTimeSamples(timingLane.currentStep.xfadeDurationSamplesRunning);
	}

	/**
	\brief
		Lane loop points may have changed; this updates the start/end points

	*/
	void WaveSequencer::updateLaneLoopPoints()
	{
		// --- timing loop convert to index values here
		timingLane.startPoint = parameters->timingLoopStart - 1;
		timingLane.endPoint = parameters->timingLoopEnd - 1;

		for (uint32_t i = 0; i < MAX_SEQ_STEPS; i++)
		{
			if (i == timingLane.startPoint)
				timingLane.laneStep[timingLane.getJumpTableValue(i)].setPreviousStepIndex(timingLane.getJumpTableValue(timingLane.endPoint));
			else
				timingLane.laneStep[timingLane.getJumpTableValue(i)].setPreviousStepIndex(-1);

			if (i == timingLane.endPoint)
				timingLane.laneStep[timingLane.getJumpTableValue(i)].setNextStepIndex(timingLane.getJumpTableValue(timingLane.startPoint));
			else
				timingLane.laneStep[timingLane.getJumpTableValue(i)].setNextStepIndex(-1);
		}
	}

	/**
	\brief
	Crank through one iteration of the sequencer over one block of data
	- manages crossfading and holding of step values
	- populates LED status arrays for monitoring the current step in the sequence

	\param samplesToProcess number of samples in block

	\returns true if successful, false otherwise
	*/
	bool WaveSequencer::render(uint32_t samplesToProcess)
	{
        
		if (parameters->haltSequencer)
			return true;

		// --- block update, done once
		update();
		bool xfadeDone = false;

        for (uint32_t i = 0; i < samplesToProcess; i++)
		{
			sampleCounter++;		

			// --- gets params and flags
			XFadeData xfadeParams = xHoldFader.getCrossfadeData();

			// --- go to next step when xfade is done
			if (xfadeParams.crossfadeFinished)
			{
				// --- sticky flag
				xfadeDone = true;

				// --- initial step has a finite fade-in time
				if (initialStep)
				{
					// --- first time, just setup the next step 
					waveLane.setCurrentStepFromNextStep();
					waveLane.loadNextStep(parameters->modLoopDirIndex[WAVE_LANE], true);

					pitchLane.setCurrentStepFromNextStep();
					pitchLane.loadNextStep(parameters->modLoopDirIndex[PITCH_LANE], true);

					stepSeqLane.setCurrentStepFromNextStep();
					stepSeqLane.loadNextStep(parameters->modLoopDirIndex[STEP_SEQ_LANE], true);

					timingLane.setCurrentStepFromNextStep();
					timingLane.loadNextStep(parameters->timingLoopDirIndex);
				
					// --- calculate timing info
					setCurrentTimingXFadeSamples();

					// --- setup crossfader
					uint32_t xfadeInTimeSamples = xHoldFader.getXFadeTimeSamples();
					setXFadeHoldParams(xfadeInTimeSamples);
					initialStep = false;
				}
				else
				{
					// --- Set the current step info with next step info
					waveLane.setCurrentStepFromNextStep();
					pitchLane.setCurrentStepFromNextStep();
					stepSeqLane.setCurrentStepFromNextStep();

					// --- Timing controls the next layer
					timingLane.setCurrentStepFromNextStep();
					timingLane.loadNextStep(parameters->timingLoopDirIndex);

					// --- Now, load next lane steps
					waveLane.loadNextStep(parameters->modLoopDirIndex[WAVE_LANE], true);
					pitchLane.loadNextStep(parameters->modLoopDirIndex[PITCH_LANE], true);
					stepSeqLane.loadNextStep(parameters->modLoopDirIndex[STEP_SEQ_LANE], true);

					// --- calculate timing info
					setCurrentTimingXFadeSamples();

					// --- setup crossfader
					uint32_t xfadeInTimeSamples = xHoldFader.getXFadeTimeSamples() / 2;
					setXFadeHoldParams(xfadeInTimeSamples);
				}
			}

			// --- for LEDs
			if (timingLane.currentLEDStepDuration == sampleCounter)
			{
				timingLane.updateLEDMeterWithNextStep();
				waveLane.updateLEDMeterWithNextStep();
				pitchLane.updateLEDMeterWithNextStep();
				stepSeqLane.updateLEDMeterWithNextStep();

				// --- clear LED values
				clearStatusArray();

				// --- set the LEDs on or off
				parameters->statusMeters.timingLaneMeter[timingLane.currentLEDStep] = 1;

				if (!waveLane.nextStep.getIsNULLStep())
					parameters->statusMeters.waveLaneMeter[waveLane.currentLEDStep] = 1;

				if (!pitchLane.nextStep.getIsNULLStep())
					parameters->statusMeters.pitchLaneMeter[pitchLane.currentLEDStep] = 1;

				if (!stepSeqLane.nextStep.getIsNULLStep())
					parameters->statusMeters.stepSeqLaneMeter[stepSeqLane.currentLEDStep] = 1;

				sampleCounter = 0;
			}

            
			// --- final write to mod outputs
			if (i == samplesToProcess - 1)
			{
				if (xfadeDone)
					modulationOutput->setModValue(kWSXFadeDone, 1.0);
				else
					modulationOutput->setModValue(kWSXFadeDone, 0.0);

				if (parameters->stepType[timingLane.getCurrentStepIndex()] == enumToInt(StepMode::kRest))
					modulationOutput->setModValue(kWSWaveMix_A, 0.0);
				else
					modulationOutput->setModValue(kWSWaveMix_A, xfadeParams.constPwrGain[0]);

				if (parameters->stepType[timingLane.getNextStepIndex()] == enumToInt(StepMode::kRest))
					modulationOutput->setModValue(kWSWaveMix_B, 0.0);
				else
					modulationOutput->setModValue(kWSWaveMix_B, xfadeParams.constPwrGain[1]);

				//double aaa = waveLane.getCurrentStepIndex();
				//double bbb = waveLane.getNextStepIndex();

				modulationOutput->setModValue(kWSWaveStepNumber_A, waveLane.getCurrentStepIndex());
				modulationOutput->setModValue(kWSWaveStepNumber_B, waveLane.getNextStepIndex());

				modulationOutput->setModValue(kWSWaveIndex_A, waveLane.getCurrentStepValue());
                modulationOutput->setModValue(kWSWaveIndex_B, waveLane.getNextStepValue());

				// --- amplitudes are locked to waveforms and do not have their own lane
				modulationOutput->setModValue(kWSWaveAmpMod_A, parameters->waveLaneAmp_dB[pitchLane.getCurrentStepIndex()]);
				modulationOutput->setModValue(kWSWaveAmpMod_B, parameters->waveLaneAmp_dB[pitchLane.getNextStepIndex()]);

				modulationOutput->setModValue(kWSPitchMod_A, pitchLane.getCurrentStepValue());
				modulationOutput->setModValue(kWSPitchMod_B, pitchLane.getNextStepValue());

				if (parameters->interpolateStepSeqMod)
				{
					double interpValue = xfadeParams.linearGain[0] * stepSeqLane.getCurrentStepValue() + xfadeParams.linearGain[1] * stepSeqLane.getNextStepValue();
					modulationOutput->setModValue(kWStepSeqMod, interpValue);
				}
				else
					modulationOutput->setModValue(kWStepSeqMod, stepSeqLane.getCurrentStepValue());
			}
		}
		return true;
	}

	/**
	\brief
	Note on handler that must:
	- reset jump tables for each lane
	- clear modulation output slots
	- setup crossfade/holding times
	- initialize current/next steps on lanes
	- initiliaze probabilities for next steps
	- clear LED meter array

	\returns true if successful, false otherwise
	*/
	bool WaveSequencer::doNoteOn(MIDINoteEvent& noteEvent)
	{
		// --- needed to set the first set of times
		timingLane.resetJumpTable();
		waveLane.resetJumpTable();
		pitchLane.resetJumpTable();
		stepSeqLane.resetJumpTable();

		update();

		// --- clear output array locations
		modulationOutput->setModValue(kWSWaveMix_A, 0.0);
		modulationOutput->setModValue(kWSWaveMix_B, 0.0);
		modulationOutput->setModValue(kWSWaveIndex_A, 0.0);
		modulationOutput->setModValue(kWSWaveIndex_B, 0.0);
		modulationOutput->setModValue(kWSWaveAmpMod_A, 0.0);
		modulationOutput->setModValue(kWSWaveAmpMod_B, 0.0);
		modulationOutput->setModValue(kWSPitchMod_A, 0.0);
		modulationOutput->setModValue(kWSPitchMod_B, 0.0);
		modulationOutput->setModValue(kWStepSeqMod, 0.0);
		modulationOutput->setModValue(kWSWaveStepNumber_A, 0.0);
		modulationOutput->setModValue(kWSWaveStepNumber_B, 0.0);

		// --- randomize on each reset and start?
		xHoldFader.reset();
		initialStep = true; // need this state as a special circumstance
		sampleCounter = 0;

		// --- mod step probabilities
		// enum { WAVE_LANE, PITCH_LANE, STEP_SEQ_LANE, NUM_MOD_LANES };
		for (uint32_t i = 0; i < MAX_SEQ_STEPS; i++)
		{
			waveLane.laneStep[i].updateStepProbability();
			pitchLane.laneStep[i].updateStepProbability();
			stepSeqLane.laneStep[i].updateStepProbability();
		}

		// --- fetch the first pair of segments and setup the first crossfade
		uint32_t startingIndex = timingLane.startPoint;
		uint32_t nextIndex = 0;

		// --- LED
		timingLane.currentLEDStep = startingIndex;

		// --- load current step
		timingLane.initCurrentStep(startingIndex);
		timingLane.initNextStep(startingIndex, parameters->timingLoopDirIndex);

		// --- calculate the XFadeOut start point
		setCurrentTimingXFadeSamples();
        
		// --- SETUP FIRST FADE IN
		//
		// --- fade IN time = symmetrical with fade out point
		uint32_t xfadeOutIndexSamples = (timingLane.currentStep.xfadeDurationSamplesRunning / 2);
		int32_t xfadeInTimeSamples = 0;

		// --- new
        startingIndex = waveLane.startPoint;
		waveLane.initCurrentStep(startingIndex);
		waveLane.initNextStep(startingIndex, parameters->modLoopDirIndex[WAVE_LANE], true);

		startingIndex = pitchLane.startPoint;
		pitchLane.initCurrentStep(startingIndex);
		pitchLane.initNextStep(startingIndex, parameters->modLoopDirIndex[PITCH_LANE], true);

		startingIndex = stepSeqLane.startPoint;
		stepSeqLane.initCurrentStep(startingIndex);
		stepSeqLane.initNextStep(startingIndex, parameters->modLoopDirIndex[STEP_SEQ_LANE], true);

		// --- no fade in time for very first segment
		xfadeInTimeSamples = 0;
		initialStep = true; // there is no fade-in time


		// --- setup xfader for delta hold and fade out to next step
		// --- calculate point of first fade out
		uint32_t delta = timingLane.currentStep.stepDurationSamplesRunning - xfadeOutIndexSamples;
		xHoldFader.setHoldTimeSamples(delta);
		xHoldFader.setXFadeTimeSamples(timingLane.currentStep.xfadeDurationSamplesRunning);

		// --- for output meters or LEDs
		timingLane.updateLEDMeterWithCurrentStep();
		waveLane.updateLEDMeterWithCurrentStep();
		pitchLane.updateLEDMeterWithCurrentStep();
		stepSeqLane.updateLEDMeterWithCurrentStep();

		clearStatusArray();
		parameters->statusMeters.timingLaneMeter[timingLane.currentLEDStep] = 1;
		parameters->statusMeters.waveLaneMeter[waveLane.currentLEDStep] = 1;
		parameters->statusMeters.pitchLaneMeter[pitchLane.currentLEDStep] = 1;
		parameters->statusMeters.stepSeqLaneMeter[stepSeqLane.currentLEDStep] = 1;

		return true;
	}

	/**
	\brief 
	Note off handler; nothing to do here

	\returns true if successful, false otherwise
	*/
	bool WaveSequencer::doNoteOff(MIDINoteEvent& noteEvent)
	{
		return true;
	}
} // namespace


