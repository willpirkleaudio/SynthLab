#pragma once

#include "synthbase.h"
#include "synthfunctions.h"


// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   sequencer.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class JumpTable
	\ingroup SynthObjects
	\brief
	A customized circular buffer for the wave sequencer object. This is identical
	to the CircularBuffer object in the FX plugin book. 

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	template <typename T>
	class JumpTable
	{
	public:
		JumpTable() {}		///< empty constructor
		~JumpTable() {}		///< empty constructor

		/** 
		\brief
		flush jump table by resetting all values to 0.0 
		*/
		void flushBuffer() { memset(&buffer[0], 0, bufferLength * sizeof(T)); }

		/**
		\brief
		reset the read-index to head of buffer
		*/
		void reset() { readIndex = 0; }

		/** 
		\brief
		Create a buffer based on a target maximum in SAMPLES do NOT call from realtime audio thread;
		do this prior to any processing */
		void createCircularBuffer(unsigned int _bufferLength)
		{
			// --- find nearest power of 2 for buffer, and create
			createCircularBufferPowerOfTwo((unsigned int)(pow(2, ceil(log(_bufferLength) / log(2)))));
		}

		/** \brief
		Create a buffer based on a target maximum in SAMPLESwhere the size is
		pre-calculated as a power of two */
		void createCircularBufferPowerOfTwo(unsigned int _bufferLengthPowerOfTwo)
		{
			// --- reset to top
			readIndex = 0;

			// --- find nearest power of 2 for buffer, save it as bufferLength
			bufferLength = _bufferLengthPowerOfTwo;

			// --- save (bufferLength - 1) for use as wrapping mask
			wrapMask = bufferLength - 1;

			// --- create new buffer
			buffer.reset(new T[bufferLength]);

			// --- flush buffer
			flushBuffer();
		}

		/** \brief
		write a value into the buffer; this overwrites the previous oldest value in the buffer */
		void writeBuffer(uint32_t index, T input)
		{
			// --- write and increment index counter
			buffer[index] = input;
		}

		/**\brief
		read output of delay line */
		T readBufferCircular()
		{
			T output = buffer[readIndex++];

			// --- autowrap index
			readIndex &= wrapMask;

			// --- read it
			return output;
		}

		/**\brief
		read an arbitrary location that is index samples old */
		T readBuffer(int index)
		{
			// --- read it
			return buffer[index];
		}


	private:
		std::unique_ptr<T[]> buffer = nullptr;	///< smart pointer will auto-delete
		unsigned int readIndex = 0;			///> read index
		unsigned int bufferLength = 1024;	///< must be nearest power of 2
		unsigned int wrapMask = 1023;		///< must be (bufferLength - 1)
	};
	
	/**
	\struct LaneStep
	\ingroup SynthStructures
	\brief
	Holds all the information needed for one step of one lane in the wave sequencer. 

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct LaneStep
	{
		LaneStep() {}
		LaneStep& operator=(const LaneStep& aLane)	// need this override for collections to work
		{
			if (this == &aLane)
				return *this;

			stepValue = aLane.stepValue;
			stepDurationSamples = aLane.stepDurationSamples;
			xfadeDurationSamples = aLane.xfadeDurationSamples;
			stepDurationSamplesRunning = aLane.stepDurationSamplesRunning;
			xfadeDurationSamplesRunning = aLane.xfadeDurationSamplesRunning;

			stepDurationNote = aLane.stepDurationNote;
			xfadeDurationNote = aLane.xfadeDurationNote;

			probability_Pct = aLane.probability_Pct;
			stepMode = aLane.stepMode;
			isNULLStep = aLane.isNULLStep;
			nextStepIndex = aLane.nextStepIndex;
			previousStepIndex = aLane.previousStepIndex;

			return *this;
		}

		/** 
		\brief
			initialize the step and crossfade durations
		*/
		void initStepTiming()
		{
			stepDurationSamplesRunning = stepDurationSamples;
			xfadeDurationSamplesRunning = xfadeDurationSamples;
		}

		/**
		\brief
			change the step duration time in samples
		*/
		void updateStepDurationSamples(uint32_t _stepDurationSamples)
		{
			stepDurationSamples = _stepDurationSamples;
		}

		/**
		\brief
			change the step cross-fade time in samples
		*/
		void updateStepXFadeSamples(uint32_t _xfadeDurationSamples)
		{
			xfadeDurationSamples = _xfadeDurationSamples;
		}

		// --- see korg wavestate docs -----------------------
		//     these are all equivalent values and need to be 
		//     updated together
		uint32_t stepDurationSamplesRunning = 0;	///< running count of step samples
		double stepDurationMilliSec = 0.0;			///< current duration in mSec
		NoteDuration stepDurationNote = NoteDuration::kQuarter;///< current step duration as note rhythm
		// --------------------------------------------------- 

		// --- XFADE Time
		//     cannot exceed 2X duration of shortest step
		uint32_t xfadeDurationSamplesRunning = 0;	///< running count of crossfade samples
		double xfadeDurationMilliSec = 0.0;			///< current crossfade in mSec
		NoteDuration xfadeDurationNote = NoteDuration::kQuarter;///< current crossfade duration as note rhythm
		// --------------------------------------------------- 

		// --- probability %
		double probability_Pct = 100.0;	///< probability of this step playing

		int32_t getNextStepIndex() { return nextStepIndex; }								///< get the next index
		void setNextStepIndex(int32_t _nextStepIndex) { nextStepIndex = _nextStepIndex; }	///< set the next index

		int32_t getPreviousStepIndex() { return previousStepIndex; }										///< get the pprevious index
		void setPreviousStepIndex(int32_t _previousStepIndex) { previousStepIndex = _previousStepIndex; }	///< set the pprevious index

		double getStepValue() { return stepValue; }		///< getter current value
		void setStepValue(double _stepValue) { stepValue = _stepValue; }///< setter current value

		/** handle changes in probability*/
		void updateStepProbability()
		{
			double wn = noiseGen.doWhiteNoise();
			wn = unipolar(wn);
			double probPct = wn * 100.0;

			if(probability_Pct < 100.0)
				int t = 0;

			if (probPct <= probability_Pct)
				setNULLStep(false);
			else
				setNULLStep(true);
		}

		bool getIsNULLStep() { return isNULLStep; }
		void setNULLStep(bool _isNULLStep) { isNULLStep = _isNULLStep; /*stepValue  = 0.0;*/ }

	protected:
		uint32_t stepDurationSamples = 0;	///< this is what we use for the crossfader operaiton
		uint32_t xfadeDurationSamples = 0;	///< this is what we use for the crossfader operaiton
		int32_t nextStepIndex = -1;			///< current next step
		int32_t previousStepIndex = -1;		///< current previous step
		double stepValue = 0.0;				///< value of step
		uint32_t stepMode = 0;				///< for individual modes of operation; e.g. timing has note and rest modes
		bool isNULLStep = false;			///< allow for NULL (non-playing) steps
		NoiseGenerator noiseGen;			///> moise gen for probability
	};
	
	/**
	\class LaneStep
	\ingroup SynthObjects
	\brief
	A Lane manages a set of Lane Steps (8 for SynthLab). 
	- maintains the Jump table for looping and random re-shuffle of step sequence
	- handles loop timing
	- maintains knowledge of next step in sequence (for forward direction)
	- maintains knowledge of previous step in sequence (for reverse direction)
	- conceptually a kind of linked list where lane steps are the list items

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class Lane
	{
	public:
		Lane(){
			resetJumpTable();   ///< construction resets table
		}
		~Lane() {}				///< empty destructor 

		void resetJumpTable()	///< reset sequence to 0-1-2-3-4-5-6-7
		{
			for (uint32_t i = 0; i < MAX_SEQ_STEPS; i++)
				jumpTable[i] = i;
		}

		// --- https://stackoverflow.com/questions/21877904/getting-random-unique-integers-in-c
		//
		
		/** \brief
			for randomizing the sequence in the table
		*/
		inline void shuffleJumpTable() 
		{			
			std::vector<int> v = { jumpTable[0], jumpTable[1], jumpTable[2], jumpTable[3], jumpTable[4], jumpTable[5], jumpTable[6], jumpTable[7] };
			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(v.begin(), v.end(), g);
			for (uint32_t i = 0; i < MAX_SEQ_STEPS; i++)
			{
				jumpTable[i] = v[i];
			}
		}

		/** \brief
			for update to next set of points (forward or backwards)
		*/
		inline void updateLaneLoopPoints()
		{
			for (uint32_t i = 0; i < MAX_SEQ_STEPS; i++)
			{
				if (i == startPoint)
					laneStep[getJumpTableValue(i)].setPreviousStepIndex(getJumpTableValue(endPoint));
				else
					laneStep[getJumpTableValue(i)].setPreviousStepIndex(-1);

				if (i == endPoint)
					laneStep[getJumpTableValue(i)].setNextStepIndex(getJumpTableValue(startPoint));
				else
					laneStep[getJumpTableValue(i)].setNextStepIndex(-1);
			}
		}

		/** \brief
			Initialize timing of a lane step
		*/
		void initStepTiming(uint32_t stepIndex)
		{
			laneStep[stepIndex].initStepTiming();
		}

		/** \brief
			Initialize timing of the FIRST lane step; needed only at startup
		*/
		void initCurrentStep(uint32_t jumpIndex)
		{
			jumpTableIndex = jumpIndex;
			currentStepIndex = jumpTable[jumpTableIndex];
			initStepTiming(currentStepIndex);
			currentStep = laneStep[currentStepIndex];
			currentStepValue = currentStep.getStepValue();
		}

		/** \brief
			Initialize timing of the NEXT step; needed only at startup
		*/
		void initNextStep(uint32_t jumpIndex, uint32_t loopDirectionIndex, bool applyProbability = false)
		{
			bool forwardBackward = loopDirectionIndex == enumToInt(LoopDirection::kForwardBackward);
			bool looped = false;
			jumpTableIndex = jumpIndex;
			nextStepIndex = getNextStepIndex(jumpTableIndex, forwardBackward, looped);
           // boundIntValue(nextStepIndex, 0, 7);
            
            initStepTiming(nextStepIndex);
            nextStep = laneStep[nextStepIndex];

            if(applyProbability)
				nextStep.updateStepProbability();

			if (nextStep.getIsNULLStep())
				nextStepValue = currentStep.getStepValue();
			else
				nextStepValue = nextStep.getStepValue();
		}

		/** \brief
			Next step becomes the current step
		*/
		void setCurrentStepFromNextStep()
		{
			currentStep = laneStep[nextStepIndex];
			currentStepIndex = nextStepIndex;
			
			if (!currentStep.getIsNULLStep())
				currentStepValue = currentStep.getStepValue();
		}

		/** \brief
			Load next step;
			- update probabilities for next step
			- detect loop endpoints and re-shuffle if needed
		*/
		bool loadNextStep(uint32_t loopDirectionIndex, bool applyProbability = false)
		{
			bool forwardBackward = loopDirectionIndex == enumToInt(LoopDirection::kForwardBackward);
			bool looped = false;
			uint32_t next = getNextStepIndex(jumpTableIndex, forwardBackward, looped);

			if (looped && randomizeSteps)
			{
				shuffleJumpTable();
				updateLaneLoopPoints();
			}

			nextStepIndex = next;
			initStepTiming(nextStepIndex);
			nextStep = laneStep[nextStepIndex];

			if(applyProbability)
				nextStep.updateStepProbability();

			if (nextStep.getIsNULLStep())
				nextStepValue = currentStep.getStepValue();
			else
				nextStepValue = nextStep.getStepValue();

			return looped;
		}
	
		/** \brief
			Find index of next step
			- always need to know the next steap
			- checks forward/reverse operation
			- supports bi-directionality
			- returns true in hitLoopPoint if the next step is an endpoint (start or end)
		
		\param jumpTableIndex the current index, the next index is calculated
		\param forwardBackward true if forward/backward mode enabled
		\param hitLoopPoint true if advancing to next step hits a loop point
		*/
		uint32_t getNextStepIndex(uint32_t& jumpTableIndex, bool forwardBackward, bool& hitLoopPoint)
		{
			uint32_t nextStepIndex = 0;
			uint32_t nextIndex = calcNextIndex(jumpTableIndex);
			uint32_t previousIndex = calcPreviousIndex(jumpTableIndex);

			// --- if forward
			if (forwardDirection)
			{
				if (currentStep.getNextStepIndex() < 0)
				{
					nextStepIndex = jumpTable[nextIndex];
					jumpTableIndex = nextIndex;
				}
				else // hit a loop point
				{
					hitLoopPoint = true;
					if (forwardBackward)
					{
						jumpTableIndex = previousIndex;
						nextStepIndex = jumpTable[jumpTableIndex];
						forwardDirection = false;
					}
					else
					{
						nextStepIndex = currentStep.getNextStepIndex();
						jumpTableIndex = nextStepIndex;
					}
				}
			}
			else // going backwards
			{
				if (currentStep.getPreviousStepIndex() < 0)
				{
					nextStepIndex = jumpTable[previousIndex];
					jumpTableIndex = previousIndex;
				}
				else // it a loop point
				{
					hitLoopPoint = true;
					if (forwardBackward)
					{
						jumpTableIndex = nextIndex;
						nextStepIndex = jumpTable[nextIndex];
						forwardDirection = true;
					}
					else
					{
						nextStepIndex = currentStep.getPreviousStepIndex();
						jumpTableIndex = nextStepIndex;
					}
				}
			}
			return nextStepIndex;
		}


		/** \brief
			Update the LED meter index with current stap index
			- update the LED on-duration with current step duration
		*/
		void updateLEDMeterWithCurrentStep()
		{
			currentLEDStep = currentStepIndex;
			currentLEDStepDuration = currentStep.stepDurationSamplesRunning;
		}

		/** \brief
			Update the LED meter index with next stap index
			- update the LED on-duration with next step duration
		*/
		void updateLEDMeterWithNextStep()
		{
			currentLEDStep = nextStepIndex;
			currentLEDStepDuration = nextStep.stepDurationSamplesRunning;
		}


		/** \brief
			Update values of current step and next step as long as they are non-null
		*/
		void updateStepValues()
		{
			if(!currentStep.getIsNULLStep())
				currentStepValue = currentStep.getStepValue();
			
			if (!nextStep.getIsNULLStep())
				nextStepValue = nextStep.getStepValue();
		}

		/** \brief
			Enable re-shuffling of jump table
		*/
		void setRandomizeSteps(bool _randomizeSteps)
		{
			if (randomizeSteps != _randomizeSteps)
			{
				randomizeSteps = _randomizeSteps;
				if (!randomizeSteps)
					resetJumpTable();
			}
		}

	public: 
		// --- LOOP points
		uint32_t startPoint = 0;			///< start loop point
		uint32_t endPoint = MAX_SEQ_STEPS;	///< end loop point
		bool forwardDirection = true;		///< forward flag

		// --- steps for the lane
		LaneStep laneStep[MAX_SEQ_STEPS];	///< array of lane steps for this lane
		LaneStep currentStep;				///< current 
		LaneStep nextStep;					///> next in sequence
		uint32_t currentLEDStep = 0;		///< index of current LED 
		uint32_t currentLEDStepDuration = 0; ///< on-time of current LED 
		int32_t getCurrentStepIndex() { return currentStepIndex; }
		int32_t getNextStepIndex() { return nextStepIndex; }
		double getCurrentStepValue() { return currentStepValue; }
		double getNextStepValue() { return nextStepValue; }
		
		int32_t getJumpTableValue(uint32_t index) ///< get a jump table value (0 to 7)
		{
			if (index > MAX_SEQ_STEPS - 1)
				return 0;

			return jumpTable[index];
		}

	protected:
		inline uint32_t calcNextIndex(uint32_t baseIndex)
		{
			///< find next step, wrap if needed (should not need to, but this is backup)
			uint32_t next = baseIndex + 1;
			next &= WRAP_MASK;
			return next;
		}

		inline uint32_t calcPreviousIndex(uint32_t baseIndex)
		{
			///< find next step, wrap if needed (should not need to, but this is backup)
			uint32_t next = baseIndex - 1;
			next &= WRAP_MASK;
			return next;
		}

		// --- table of steps
		int32_t jumpTable[MAX_SEQ_STEPS]; ///< table of lane step index values

		// --- realtime operation
		int32_t currentStepIndex = 0;		///< always know current step index
		int32_t nextStepIndex = 0;			///< always know next step index
		uint32_t jumpTableIndex = 0;		///< current index in jump table

		double currentStepValue = 0.0;	///< always know current step value
		double nextStepValue = 0.0;		///< always know next step value
		bool randomizeSteps = false;	///< flag to randomize steps
	};

	struct WaveSequencerStatusMeters
	{
		WaveSequencerStatusMeters& operator=(const WaveSequencerStatusMeters& wssm)	// need this override for collections to work
		{
			if (this == &wssm)
				return *this;

			memcpy(timingLaneMeter, wssm.timingLaneMeter, MAX_SEQ_STEPS * sizeof(uint32_t));
			memcpy(waveLaneMeter, wssm.waveLaneMeter, MAX_SEQ_STEPS * sizeof(uint32_t));
			memcpy(pitchLaneMeter, wssm.pitchLaneMeter, MAX_SEQ_STEPS * sizeof(uint32_t));
			memcpy(stepSeqLaneMeter, wssm.stepSeqLaneMeter, MAX_SEQ_STEPS * sizeof(uint32_t));
			return *this;
		}

		uint32_t timingLaneMeter[MAX_SEQ_STEPS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		uint32_t waveLaneMeter[MAX_SEQ_STEPS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		uint32_t pitchLaneMeter[MAX_SEQ_STEPS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		uint32_t stepSeqLaneMeter[MAX_SEQ_STEPS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	};

	enum class soloType { SOLO_OFF, SOLO_0, SOLO_1, SOLO_2, SOLO_3, SOLO_4, SOLO_5, SOLO_6, SOLO_7 };

	/**
	\struct WaveSequencerParameters
	\ingroup SynthStructures
	\brief
	GUI Parameters for wave sequencer object.
	- storage/maintenance for timing lane start/stop/random attributes
	- storage/maintenance for lane step crossfade start/stop/direction
	- storage/maintenance for lane step wave start/stop/direction
	- storage/maintenance for lane step pitch start/stop/direction

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct WaveSequencerParameters
	{
		WaveSequencerParameters() {}
		WaveSequencerStatusMeters statusMeters; ///< set of meters for outbound monitoring of lane status
		bool haltSequencer = false;

		/** --- TIMING --------------------------- */
		double BPM = 120;
		double timeStretch = 1.0;
		bool interpolateStepSeqMod = false;
		bool randomizeStepOrder = false;
		bool randomizePitchOrder = false;
		bool randomizeWaveOrder = false;
		bool randomizeSSModOrder = false;

		/**  --- LOOP points */
		uint32_t timingLoopStart = 1; // NOTE: 1 is the minimum value here, 1-indexed to match GUI for user
		uint32_t timingLoopEnd = MAX_SEQ_STEPS;// -1;
		uint32_t timingLoopDirIndex = enumToInt(LoopDirection::kForward);

		/**  --- step durations */
		double stepDurationMilliSec[MAX_SEQ_STEPS] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };
		uint32_t stepDurationNoteIndex[MAX_SEQ_STEPS] = { enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter) };
		uint32_t stepType[MAX_SEQ_STEPS] = { 0,0,0,0,0,0,0,0 }; // 0 = NOTE, 1 = REST
		// uint32_t timingLaneMeter[MAX_SEQ_STEPS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		// --------------------------------------------------- 

		/** ---  XFADE Times */
		double xfadeDurationMilliSec[MAX_SEQ_STEPS] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };
		uint32_t xfadeDurationNoteIndex[MAX_SEQ_STEPS] = { enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter),enumToInt(NoteDuration::kQuarter) };
		// --------------------------------------------------- 

		/**  MOD LANES: Pitch, Wave, Sequencer */
		uint32_t modLoopStart[NUM_MOD_LANES] = { 1, 1, 1 };  // NOTE: 1 is the minimum value here, 1-indexed to match GUI for user
		uint32_t modLoopEnd[NUM_MOD_LANES] = { MAX_SEQ_STEPS, MAX_SEQ_STEPS, MAX_SEQ_STEPS }; 
		uint32_t modLoopDirIndex[NUM_MOD_LANES] = { enumToInt(LoopDirection::kForward), enumToInt(LoopDirection::kForward), enumToInt(LoopDirection::kForward) };

		/**  --- WAVE_LANE --------------------------- */
		double waveLaneAmp_dB[MAX_SEQ_STEPS] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };
		double waveLaneValue[MAX_SEQ_STEPS] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };
		double waveLaneProbability_pct[MAX_SEQ_STEPS] = { 100.0,100.0,100.0,100.0,100.0,100.0,100.0,100.0 }; // WRAP_MASK = MAX-1

		/**  --- PITCH_LANE --------------------------- */
		double pitchLaneValue[MAX_SEQ_STEPS] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };
		double pitchLaneProbability_pct[MAX_SEQ_STEPS] = { 100.0,100.0,100.0,100.0,100.0,100.0,100.0,100.0 }; // WRAP_MASK = MAX-1

		/**  --- STEP_SEQ_LANE --------------------------- */
		double stepSeqValue[MAX_SEQ_STEPS] = { -1.0,1.0,-0.3,-0.1, 1.0, 0.6, 0.3, 0.0 };
		double stepSeqProbability_pct[MAX_SEQ_STEPS] = { 100.0,100.0,100.0,100.0,100.0,100.0,100.0,100.0 }; // WRAP_MASK = MAX-1
	};

	/**
	\class WaveSequencer
	\ingroup SynthModules
	\brief
	WaveSequencer module.
	- is really a big modulation generation system
	- timing lane locks all lanes to its current step
	- each non-timing lane generates a modulation value of some kind
	- writes information int the module's Modulation Output array, just like all other modulators

	Base Class: SynthModule
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.

	Databases: None

	GUI Parameters: WaveSequencerParameters
	- getParameters() function allows direct access to std::shared_ptr<WaveSequencerParameters>

	std::shared_ptr<WaveSequencerParameters> getParameters()

	- call the getParameters() function
	- set the parameters in the WaveSequencerParameters structure with new values, typically from a GUI
	To apply these new parameters either:
	- (a) call the module's update() function OR
	- (b) call the render() function which in turn calls the update() method.

	Ordinarily, this operation happens just prior to calling the render() function so that is the preferred
	method of operation to avoid multiple calls to the update() function, which is usually the most CPU intensive
	function of the SynthModule.

	Access to Modulators
	- std::shared_ptr<Modulators> getModulationInput()
	- std::shared_ptr<Modulators> getModulationOutput()

	Access to audio buffers (I/O)
	- std::shared_ptr<AudioBuffer> getAudioBuffers()

	Reads:
	- Modulation Input Values (modulators)

	Writes:
	- Modulation Output Values (modulators)

	Construction:

	(1) For use within a synth project, the constructor
	is specialized to use shared recources for:
	- MidiInputData
	- WaveSequencerParameters

	The owning object (SynthVoice for the SynthLab projects) must pass these valid pointers
	to the object at construction time. Typically the engine or voice will be the primary synthesizers
	of these resources. See the 2nd Edition Synth Book for more information.

	(2) Standalone:

	To use in standalone mode, call the constructor with the shared resoure pointers as null:

	WaveSequencer(nullptr, nullptr, 64);

	In standalone mode, the object creates and maintains these resources:
	- MidiInputData: this has no read access so you cannot access its data. Ordinarily, you
	pass this shared pointer into the object so that it has access to the current MIDI input data.
	The object does not write data into this resource, so it is functionally non-opeational.

	- WaveSequencerParameters: in standalone mode only, these are synthesized locally on the object,
	and then the owning object may obtain a shared pointer to them to read/write the parameters directly.

	Render: renders modulation values into the Modulators output array:
	- kWSWaveMix_A
	- kWSWaveMix_B
	- kWSWaveIndex_A
	- kWSWaveIndex_B
	- kWSWaveAmpMod_A
	- kWSWaveAmpMod_B
	- kWSPitchMod_A
	- kWSPitchMod_B
	- kWStepSeqMod
	- kWSWaveStepNumber_A
	- kWSWaveStepNumber_B

	- NOTE: this structure returns information for LED meters or flashing lights on GUI

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class WaveSequencer : public SynthModule
	{
	public:
		/** One and only specialized constructor; pointers may be null for stanalone */
		WaveSequencer(std::shared_ptr<MidiInputData> _midiInputData,
			std::shared_ptr<WaveSequencerParameters> _parameters,
			uint32_t blockSize = 64);	/* C-TOR */

		/** Destructor is empty: all resources are smart pointers */
		virtual ~WaveSequencer() {}		/* D-TOR */

		/** SynthModule Overrides */
		virtual bool reset(double _sampleRate) override;
		virtual bool update() override;
		virtual bool render(uint32_t samplesToProcess = 1) override;
		virtual bool doNoteOn(MIDINoteEvent& noteEvent) override;
		virtual bool doNoteOff(MIDINoteEvent& noteEvent) override;

		/** Specialized functions */
		void clearStatusArray();
		uint32_t setCurrentTimingXFadeSamples();
		void setXFadeHoldParams(uint32_t xfadeInTimeSamples);
		void updateLaneLoopPoints();

	protected:
		/** for standalone operation */
		std::shared_ptr<WaveSequencerParameters> parameters = nullptr;

		// --- for LEDs 
		uint32_t sampleCounter = 0; ///< for LED timing

		// --- lanes
		Lane	timingLane;			///< timing lane
		Lane	waveLane;			///< waveform lane
		Lane	pitchLane;			///< pitch lane
		Lane	stepSeqLane;		///< step sequemcer lane

		// --- crossfader
		XHoldFader xHoldFader;		///< for crossfading waveforms and values
		bool initialStep = false;

		// --- for probabilities
		NoiseGenerator noiseGen;	///< for probability

		// --- basic variables
		double sampleRate = 0.0;		///< sample rate
		double samplesPerMSec = 0.0;	///< for step counting
	};

} // namespace

