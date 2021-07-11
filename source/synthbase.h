#ifndef __synthBase_h__
#define __synthBase_h__

#include <cmath>
#include <random>
#include <string>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <memory>
#include <algorithm>
#include <map>

#include "synthstructures.h"
#include "synthlabparams.h"

#define _MATH_DEFINES_DEFINED

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthbase.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\ingroup SynthStructures
	Constants for reducing CPU and/or memory usage; also for resetting the DM synth to default mode
	if you write a toxic module and need to revert
	*/
	struct DMConfig
	{
		bool dm_build = false;
		bool dual_mono_filters = false;
		bool half_sample_set = false;
		bool reduced_unison_count = false;
		bool analog_fgn_filters = false;
		bool parameterSmoothing = true;
	};
	
	// ---------------------- SYNTH OBJECTS WITHOUT BASE CLASES --------------------------------------------- //
	//
	/* 
		These are simple, stand-alone objects or primary synth objects that perform intrinsically
		basic/fundamental synth operations. 
	   
		Many of these are used as base classes for extended objects.

		These are NOT interfaces, which are defined in a separate section of this file.
	*/
	//
	// ------------------------------------------------------------------------------------------------------- //

	/**
	\class AudioBuffer
	\ingroup SynthObjects
	\brief
	Encapsulates the audio buffering requirements of any module that uses audio samples for input and/or output.
	- includes sepearate audio input and output buffers (arrays)
	- audio buffers are channel-independent so there are separate Left and Right channel buffers for each direction
	- automatically supports multi-channel operation beyond stereo
	- follows identical design pattern as that used in all plugin APIs and DAWs
	- uses float buffers for maximum compatibility across target APIs and DAWs
	- audio buffers are used to move frequency modulation values (FM) or phase modualtion values (PM) in 
	addition to the normal input/output
	- FM and PM buffers are also categorized as INPUT or OUTPU depending on origin and destination

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class AudioBuffer
	{
	public:
		AudioBuffer() {}
		AudioBuffer(uint32_t _numInputChannels, uint32_t _numOutputChannels, uint32_t _blockSize);
		~AudioBuffer();

		/** setup the buffers for a set number of channels - this cannot change after init */
		void init(uint32_t _numInputChannels, uint32_t _numOutputChannels, uint32_t _blockSize);
		void flushBuffers(); ///< clear out the audio; used often

		/** get a pointer to an input or output buffer for a certain channel */
		float* getInputBuffer(uint32_t channel);
		float* getOutputBuffer(uint32_t channel);

		/** arrays of pointers to ALL input and output buffers */
		float** getInputBuffers() { return inputBuffer; }
		float** getOutputBuffers() { return outputBuffer; }

		/** get number of input or output channels; these valued do not change once initialized */
		uint32_t getInputChannelCount() { return numInputChannels; }
		uint32_t getOutputChannelCount() { return numOutputChannels; }

		/** current block size in frames - one sample per channel */
		uint32_t getBlockSize() { return blockSize; }
		uint32_t getSamplesInBlock() { return samplesInBlock; }
		void setSamplesInBlock(uint32_t _samplesInBlock);

	protected:
		void destroyInputBuffers();
		void destroyOutputBuffers();
		float** inputBuffer = nullptr;	///< array of input buffer pointers
		float** outputBuffer = nullptr;	///< array of output buffer pointers
		uint32_t numInputChannels = 1;
		uint32_t numOutputChannels = 1;
		uint32_t blockSize = 64;		///< the maximum block size
		uint32_t samplesInBlock = 64;	///< the number of samples to process in the block (in case of partial blocks)
	};


	/**
	\class SynthClock
	\ingroup SynthObjects
	\brief
	Compact modulo counter with wrapping used as the timebase for all oscillators. 
	- the mcounter member variable is the modulo counter value on the range [0.0, 1.0]
	- the clock can run forwards or backwards (for negative frequencies)
	- you may temporarily shift the current phase of the clock forwards or backwards, with modulo wrap
	to implement phase modulation for the DX synths
	- functions allow saving the current phase so that the state can be restored after the phase modulation

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class SynthClock
	{
	public:
		SynthClock() {}
		~SynthClock() {}
		SynthClock& operator=(const SynthClock& params);

		/** Initialization and reset */
		void reset(double startValue = 0.0);
		void initWithClock(SynthClock& clock);

		/** advancing and wrapping the clock mcounter */
		void advanceClock(uint32_t renderInterval = 1);
		bool advanceWrapClock(uint32_t renderInterval = 1);
		bool wrapClock();

		/** get/set the clock frequency; which affects the phaseInc */
		void setFrequency(double _frequency_Hz, double _sampleRate);
		double getFrequency() { return frequency_Hz; }

		/** Phase Modulation support */
		void addPhaseOffset(double _phaseOffset, bool wrap = true);
		void removePhaseOffset();

		/** Frequency Modulation support */
		void addFrequencyOffset(double _freqOffset);
		void removeFrequencyOffset();

		/** For both PM and FM support */
		void saveState();
		void restoreState();

	public: // for fastest access
		double mcounter = 0.0;			///< modulo counter [0.0, +1.0], this is the value you use
		double phaseInc = 0.0;			///< phase inc = fo/fs
		double phaseOffset = 0.0;		///< PM
		double freqOffset = 0.0;		///< FM
		double frequency_Hz = 0.0;		///< clock frequency
		double sampleRate = 0.0;
		enum { MOD_COUNTER, PHASE_INC, PHASE_OFFSET, FREQUENCY_HZ, NUM_VARS };///< for state save
		double state[NUM_VARS] = { 0.0, 0.0, 0.0, 0.0 };///< for state save
	};
	
	/**
	\class Timer
	\ingroup SynthObjects
	\brief
	Ultra compact timer object that is used for many different functionalities
	- Set the timer to expire after some number of milliseconds OR some number of sample intervals
	- functions to advance the timer and the timer tick count
	- use timerExpired( ) to query the timer's expiration state: true = expired

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class Timer
	{
	public:
		Timer() {}
		~Timer() {}

		/** Initialize */
		void resetTimer() { counter = 0; }		///< reset the counter
		
		/** Set the expiration time for the counter */
		void setExpireSamples(uint32_t _targetValueInSamples) { targetValueInSamples = _targetValueInSamples; } ///< set target value
		void setExpireMilliSec(double timeMSec, double sampleRate) { setExpireSamples( ((uint32_t)(sampleRate*(timeMSec / 1000.0))) ); } ///< set target value
		
		/** get the expiration time in samples */
		uint32_t getExpireSamples() { return targetValueInSamples; } 

		/** query if timer has expired (elapsed) */
		bool timerExpired() { return (counter >= targetValueInSamples); } ///< check if we hit target

		/** Tick count functions*/
		void advanceTimer(uint32_t ticks = 1) { counter += ticks; }		///< advance by 1
		uint32_t getTick() { return counter; }		///< tick count

	protected:
		uint32_t counter = 0;///< the timer counter
		uint32_t targetValueInSamples = 0;///< curent target galue
	};

	/**
	\class XFader
	\ingroup SynthObjects
	\brief
	Crossfades two values (from A to B) 
	- specified with crossfade time in samples
	- crossfade() function accepts two values, A and B and then returns a single value that is the combination of the two
	as a result of the crossfade
	- after the crossfade is complete, the crossfade() function will always return 100% B 

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class XFader
	{
	public:
		XFader() {} ///< simple construction
		XFader(uint32_t _xfadeTime_Samples);

		/** initialization*/
		void reset();

		/** fader functions */
		void setXFadeTime(uint32_t _xfadeTime_Samples);
		void startCrossfade() { running = true; }
		void stopCrossfade() { running = false; }
		bool isCrossfading() { return running; }

		// --- crossfade FROM A to B
		//     returns TRUE if the crossfade is still going (needs more samples)
		//			   FALSE if the crossfade is finished (done)
		bool crossfade(XFadeType xfadeType, double inputA, double inputB, double& output);

	protected:
		uint32_t xfadeTime_Samples = 4410;	///< the target crossfade time
		uint32_t xfadeTime_Counter = 0;		///< counter for timer
		bool running = false;				///< state variable
	};


	/**
	\class XHoldFader
	\ingroup SynthObjects
	\brief
	Crossfades two values (from A to B) and then holds B for some amount of time
	- specified with two times in samples: crossfade time and hold time
	- unlike the XFader object, this does not mathematically manipulate two input values A and B to produce a final
	output value, but instead returns *information about* the current crossfade and hold states in a XFadeData structure
	- this allows for simpler use in the WaveSequencer object
	- note that this object returns ALL crossfade values for linear, square law, and constant power

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class XHoldFader
	{
	public:
		XHoldFader() {}
		~XHoldFader() {}

		// --- fader functions
		void reset();

		/** get/set crossfade time*/
		inline void setXFadeTimeSamples(uint32_t _xfadeTimeSamples){ xfadeTime_Samples = _xfadeTimeSamples; }
		inline uint32_t getXFadeTimeSamples(){ return xfadeTime_Samples; }

		/** get/set hold time*/
		void setHoldTimeSamples(double _holdTimeSamples);
		inline double getHoldTimeSamples() { return holdTime_Samples; }

		/** perform crossfade from FROM A to B*/
		XFadeData getCrossfadeData();

	protected:
		uint32_t xfadeTime_Samples = 4410;	///< target crossfade time
		uint32_t xfadeTime_Counter = 0;		///< crossfade timer counter
		uint32_t holdTime_Samples = 4410;	///< target hold time
		uint32_t holdTime_Counter = 0;		///< hold timer counter
		bool holding = false;				///< state variable for holding
	};


	/**
	\class SlewLimiter
	\ingroup SynthObjects
	\brief
	Implements a simple slew limiter using a one pole lowpass filter
	- the slew value is the single feedback coefficient (b1) in the filter

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class SlewLimiter
	{
	public:
		SlewLimiter() {}
		~SlewLimiter() {}

		// --- slewing functions
		void reset() { z1 = 0; }		///< reset the counter

		// --- slew value is:  0 <= slewvalue <= 0.9999
		void setSlewValue(double _g) { g = _g; }///< b1 coefficient
		inline double doSlewLimiter(double input)///< run the one pole filter
		{
			double output = input*(1.0 - g) + g*z1;
			z1 = output;
			return output;
		}

	protected:
		double g = 0;		///< one pole filter feedback coefficient
		double z1 = 0.0;	///< one pole filter state variable
	};


	/**
	\class Synchronizer
	\ingroup SynthObjects
	\brief
	This is a very specialized object that performs the hard-sync operation using two SynthClocks
	- designed for use with an oscillator that is using a SynthClock for the timebase and running at a rate that is HIGHER than the
	MIDI pitch, called the "main oscillator" in the synth book
	- contains two SynthClocks: one is used as the "reset oscillator" in the synth book
	- the other is used as a timer for smearing over the discontinuity

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class Synchronizer
	{
	public:
		Synchronizer() {}
		~Synchronizer() {}

		/** initialization */
		bool reset(double _sampleRate, double startPhase, int32_t xfadeSamples = 16);

		/** hard sync functions */
		bool setHardSyncFrequency(double hardSyncFrequency);
		SynthClock& getHardSyncClock() { return hardSyncClock; }
		SynthClock& getCrossFadeClock() { return crossFadeClock; }
		void startHardSync(SynthClock oscClock);
		double doHardSyncXFade(double inA, double inB);
		bool isProcessing() { return hardSyncFader.isCrossfading(); }

		/** for hard syncing with phase modulation */
		void addPhaseOffset(double offset);
		void removePhaseOffset();

	protected:
		SynthClock hardSyncClock;	///< clock for reset oscillator
		SynthClock crossFadeClock;	///< crossfading timer
		XFader hardSyncFader;		///< crossfader for smearing discontinuity
		double hardSyncFrequency = 440.0;	///< reset oscillator rate
		double sampleRate = 44100.0;		///< fs
	};


	/**
	\class RampModulator
	\ingroup SynthObjects
	\brief
	This is a tiny modulator object that produces an output that ramps up or down linearly between
	two values over a specified time in milliseconds.
	- used for LFO fade-in time
	- can be used to fade-in or fade-out *any* kind of value, from audio samples to modulation values
	to modulation times
	- the timer always counts UP regardless of the direction of the ramp

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class RampModulator
	{
	public:
		RampModulator() {}
		~RampModulator() {}

		/** the main function that sets up and start the modulator */
		bool startModulator(double startValue, double endValue, double modTime_mSec, double sampleRate);
		
		/** the mod time may be changed while running, normally not used but permitted */
		bool setModTime(double modTime_mSec, double sampleRate);

		/** get the next modulation value - this is the main output function; auto advances the clock  */
		double getNextModulationValue(uint32_t advanceClock = 1);

		/** manually advance the clock */
		void advanceClock(uint32_t ticks = 1);
		inline bool isActive() { return timerActive; } ///< checks to see if the modulator is running, or completed

	protected:
		bool timerActive = false;	///< state of modulator, running (true) or expired (false)
		double timerInc = 0.0;		///< timer incrementer value
		double countUpTimer = 0.0;	///< current timer value; this always counts UP regardless of the polarity of the modulation (up or down)
		double modRange = 1.0;		///< range of modulation output
		double modStart = 0.0;		///< ramp mod start value
		double modEnd = 1.0;		///< ramp mod end value
	};

	/**
	\class GlideModulator
	\ingroup SynthObjects
	\brief
	Specialized version of the RampModulator, with nearly identically named functions to peform
	the portamento operation (aka glide modulation)
	- optimized to perform the ramp modulation between two MIDI note numbers, startNote and endNote
	- still performs the linear modulation; caller can convert this modulation value to a pitch modulation
	for oscillator pitch modulation in semitones; this converts the portamento from linear to linear-in-semitones
	- the timer always counts DOWN regardless of the direction of the ramp

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class GlideModulator
	{
	public:
		GlideModulator() {}
		~GlideModulator() {}

		/** the main function that sets up and start the modulator */
		bool startModulator(double startNote, double endNote, double glideTime_mSec, double sampleRate);
		
		/** the glide time may be changed while running, normally not used but permitted */
		bool setGlideTime(double glideTime_mSec, double sampleRate);
		
		/** get the next modulation value - this is the main output function; auto advances the clock  */
		double getNextModulationValue(uint32_t advanceClock = 1);

		/** manually advance the clock */
		void advanceClock(uint32_t ticks = 1);
		inline bool isActive() { return timerActive; }///< checks to see if the modulator is running, or completed

	protected:
		bool timerActive = false;	///< state of modulator, running (true) or expired (false)
		double timerInc = 0.0;		///< timer incrementer value
		double countDownTimer = 1.0;///< current timer value; this always counts DOWN regardless of the polarity of the modulation (up or down)
		double glideRange = 1.0;	///< range (distance between MIDI note numbers, as double)
	};

	/**
	\class NoiseGenerator
	\ingroup SynthObjects
	\brief
	Simple object that generates white, gaussian white or pink noise
	
	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class NoiseGenerator
	{
	public:
		NoiseGenerator() {}
		~NoiseGenerator() {}

		/** random number generator */
		std::default_random_engine defaultGeneratorEngine;

		/** noise generation functions */
		double doGaussianWhiteNoise(double mean = 0.0, double variance = 1.0);
		double doWhiteNoise();
		double doPinkNoise();
		double doPinkingFilter(double white);

	protected:
		/** pinking filter coefficients */
		double bN[3] = { 0.0, 0.0, 0.0 };

		// --- another method of white noise, faster
		float g_fScale = 2.0f / 0xffffffff;
		int g_x1 = 0x67452301;
		int g_x2 = 0xefcdab89;
	};

	// ---------------------- SYNTH INTERFACE OBJECTS-------------------------------------------------------- //
	//
	/*
		These are pure abstrace interfaces to be used as base classes for the synth objects. These allow
		container objects to hold (shared) simple interface pointers, or the actual object pointers; all
		object interfacing is done through the common objec model interfaces here.
	*/
	//
	// ------------------------------------------------------------------------------------------------------- //
	
	/**
	\class IModulator
	\ingroup SynthInterfaces
	\brief
	Interface for modulator objects. 
	- Modulator objects have multiple "channels" implemented as slots in an array of doubles
	- each slot in the array (channel) corresponds to a modulation source value (e.g. normal LFO output)
	or a modulation destination location (e.g. bi-polar modulation value that will connect to an oscillator)
	- there are MAX_MODULATION_CHANNELS per Modulator (you may easily change this constant)
	- this interface is used to allow access to specific modulation values or an entire array of values on
	a given channel
	- this is one of a handful of objects that uses naked pointers; this is necessary here because pointers to the 
	modulation arrays must be able to cross a thunk barrier when used with the module-core paradigm, not
	possible with std::unique or shared pointers.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class IModulator
	{
	public:
		/**
		\brief
			Used for the modulation matrix to have instant access to the array
			- example: 
			- double* pValue = lfo[0]->getModulationOutput()->getModArrayPtr(kLFONormalOutput))

			\param index the index of the channel for the modulator 
		*/ 
		virtual double* getModArrayPtr(uint32_t index) = 0;

		/**
		\brief
		get a modulation value from the array for a certain channel
		- example: 
		- double modulationValue = lfo[0]->getModulationOutput()->getModValue(kLFONormalOutput))

		\param index the index of the channel for the modulator
		*/
		virtual double getModValue(uint32_t index) = 0;
	
		/**
		\brief
		set a modulation value to the array for a certain channel
		- example:
		- lfo[0]->getModulationOutput()->setModValue(kLFONormalOutput, 0.1234))

		\param index the index of the channel for the modulator
		\param value the modulation value to set
		*/
		virtual void setModValue(uint32_t index, double value) = 0;
	};

	// ---------------------- WAVETABLES --------------------------------------------------------- //
	/**
	\class IWavetableSource
	\ingroup SynthInterfaces
	\brief
	Interface for wavetable sources
	- A wavetable database stores a set of wavetable sources
	- each source represents one waveform generator
	- each wavetable source has a uniqe name string, that is usually the name of the waveform
	which itself must be unique otherwise the user would be confused
	- the source may consist of a single table (such as the SineTableSource) while others may 
	implement an array of tables, for example to implement bandlimited wavetables across a set of keys
	- The source interface functions allow:
	1. selection of a table (this should mark the table in some way as being the currently selected item)
	2. read the selected wavetable
	3. get the name and length of the table

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class IWavetableSource
	{
	public:
		/**
		\brief
		Objects that access the database will select a table based on the user's waveform selection
		- the note number is used to decode the selection to permit bandlimited sets of tables
		which is the normal operation in SynthLab

		\param midiNoteNumber the note number corresponding to the currently rendered pitch
		*/
		virtual void selectTable(uint32_t midiNoteNumber) = 0;
	
		/**
		\brief
		Read a table at a normalized index where 0.0 is the start of the table and 1.0 is the end of it.
		- the object that overrides this function must also implement any interpolation needed

		\param normalizedPhaseInc normalized index in the table to access

		\return the output value from the table, usually with interpoltion of some kind
		*/
		virtual double readWaveTable(double normalizedPhaseInc) = 0;
	
		/**
		\return the table length
		*/
		virtual uint32_t getWaveTableLength() = 0;


		/**
		\return the table name (unique)
		*/
		virtual const char* getWaveformName() = 0;
		
		///**
		//\return the table index (unique) for faster iteration (OPTIONAL), or -1 if not found
		//*/
		//virtual int32_t getWaveformIndex() { return -1; }
	};

	/**
	\class IWavetableDatabase
	\ingroup SynthInterfaces
	\brief
	Interface for wavetable databases
	- A wavetable database stores a set of wavetable sources and allows the calling object access
	to these sources; for example a wavetable oscillator would request a wavetable source pointer
	for the "analog saw" table set
	- The database interface functions allow:
	1. get a wavetable source interface pointer
	2. add/remove wavetable sources to/from the database
	3. clear all the table sources (for destruction time)

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class IWavetableDatabase
	{
	public:
		/**
		\brief
		get a table source based on its unique name string

		\param uniqueTableName the name of the table source (see IWavetableSource::getWaveformName( ))

		\return an IWavetableSource pointer to the source
		*/
		virtual IWavetableSource* getTableSource(const char* uniqueTableName) = 0;
		virtual IWavetableSource* getTableSource(uint32_t uniqueTableIndex) { return nullptr; }

		/**
		\brief
		adds a table to the database

		\param uniqueTableName the name of the table source (see IWavetableSource::getWaveformName( ))
		\param tableSource the IWavetableSource interface pointer to the object
	
		\return true if sucessful, false otherwise
		*/
		virtual bool addTableSource(const char* uniqueTableName, IWavetableSource* tableSource, uint32_t& uniqueIndex) = 0;

		/**
		\brief
		remove a table from the database

		\param uniqueTableName the name of the table source (see IWavetableSource::getWaveformName( ))
		\return true if sucessful, false otherwise
		*/
		virtual bool removeTableSource(const char* uniqueTableName) = 0;

		/**
		\brief
		clear all source pointers

		\return true if sucessful, false otherwise
		*/
		virtual bool clearTableSources() = 0;

		/**
		\return the table index (unique) for faster iteration (OPTIONAL), or -1 if not found
		*/
		virtual int32_t getWaveformIndex(const char* uniqueTableName) { return -1; }
	};


	// ---------------------- PCM SAMPLES --------------------------------------------------------- //
	/**
	\struct PCMSampleOutput
	\ingroup SynthStructures
	\brief
	Holds the audio output samples from reading a PCM sample file.
	- contains an audio output array of 2 slots for mono or stereo PCM samples
	- you can easily increase this by altering the static declaration here
	- the numActiveChannels is used for multi-channel operation

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct PCMSampleOutput
	{
		double audioOutput[STEREO_CHANNELS] = { 0.0, 0.0 }; ///< array of audio output samples
		uint32_t numActiveChannels = 0; ///< number of active channels; not used in SynthLab but available
	};

	/** \ingroup Constants-Enums
	SampleLoopMode fpr PCM sample read operation */
	enum class SampleLoopMode { loop, sustain, oneShot };

	/**
	\class IPCMSampleSource
	\ingroup SynthInterfaces
	\brief
	Interface for PCM sample sources
	- A PCM sample database stores a set of PCM sample sources
	- each source represents one waveform generator, or one set of PCM samples that make a patch
	- each PCM sample source has a uniqe name string, that is usually the name of the waveform or the sample
	which itself must be unique otherwise the user would be confused
	- the source may consist of a PCM sample but most implement an array of PCM samples, for example to 
	implement bandlimited multi-samples across a set of keys
	- The source interface functions allow:
	1. selection of a table (this should mark the table in some way as being the currently selected item)
	2. read the selected wavetable
	3. set the sample looping mode
	4. delete the samples (for low level destruction)

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class IPCMSampleSource
	{
	public:
		/**
		\brief
		Selects a PCM sample based on the current oscillation frequency (after all modulations have been applied)

		\param oscFrequency the frequency in Hz of the currently rendered pitch
		*/
		virtual double selectSample(double oscFrequency) = 0;
	
		/**
		\brief
		Read a PCM sample at a fractional read index location (not normalized)
		- modifies the readIndex that is passed-by-reference depending on the number of samples read

		\param readIndex unnormalized read location within the sample
		\param inc sample incrememter value (called phaseInc for the oscillator objects)

		\return a PCMSampleOutput structure that contians the audio sample(s)
		*/
		virtual PCMSampleOutput readSample(double& readIndex, double inc) = 0;

		/**
		\brief
		Set the looping mode:
		- SampleLoopMode::loop: loop from start to end of sample
		- SampleLoopMode::sustain: loop over a pre-set range of sustain samples (steady state section of sample)
		- SampleLoopMode::oneShot: play the sample once and never again

		\param _loopMode the SampleLoopMode
		*/
		virtual void setSampleLoopMode(SampleLoopMode _loopMode) = 0;

		/**
		\brief
		Delete the samples, part of destruction process
		*/
		virtual void deleteSamples() = 0;
				
		/**
		\brief
		query for valid sample count (not used in SynthLab but avialable)
		*/
		virtual uint32_t getValidSampleCount() = 0;

		/**
		\brief
		query for valid samples; needed if WAV parsing fails and we need to delete the entry
		*/
		virtual bool haveValidSamples() = 0;
	};

	/**
	\class IPCMSampleDatabase
	\ingroup SynthInterfaces
	\brief
	Interface for PCM sample databases
	- A PCM sample database stores a set of PCM sample sources and allows the calling object access
	to these sources; for example a PCM oscillator would request a PCM sample source pointer
	for the "303Snare" PCM sample set
	- The database interface functions allow:
	1. get a PCM sample source interface pointer
	2. add/remove PCM sample sources to/from the database
	3. clear all the table sources (for destruction time)

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class IPCMSampleDatabase
	{
	public:
		/**
		\brief
		get a PCM sample source based on its unique name string

		\param uniqueSampleSetName the name of the PCM samples 
		(usually the name of a folder that contains the samples as WAV files)

		\return an IPCMSampleSource pointer to the source
		*/
		virtual IPCMSampleSource* getSampleSource(const char* uniqueSampleSetName) = 0;
	
		/**
		\brief
		adds a PCM sample to the database

		\param uniqueSampleSetName the name of the PCM sample set
		\param sampleSource the IPCMSampleSource interface pointer to the object

		\return true if sucessful, false otherwise
		*/
		virtual bool addSampleSource(const char* uniqueSampleSetName, IPCMSampleSource* sampleSource) = 0;
		

		/**
		\brief
		remove a PCM sample set from the database

		\param uniqueSampleSetName the name of the sample set

		\return true if sucessful, false otherwise
		*/
		virtual bool removeSampleSource(const char* uniqueSampleSetName) = 0;
		
		/**
		\brief
		clear all the sample sources

		\return true if sucessful, false otherwise
		*/
		virtual bool clearSampleSources() = 0;
	};

	/**
	\class IMidiInputData
	\ingroup SynthInterfaces
	\brief
	Interface for a MIDI input data object
	- The MIDI input data object stores MIDI data from the MIDI input stream; the interface
	provides read-access to the MIDI input object's MIDI Data 
	- There are three types of MIDI data
	1. Global Data: 
	- global (aka "Master") MIDI values such as volume and tuning
	- the most recent two MIDI note on and note off messages (needed for portamento)
	2. CC Data: 
	- 128 CC values that should be organzied according to CC number
	3. Aux DAW data: not necessarily MIDI data, e.g. current DAW BPM 

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class IMidiInputData
	{
	public:
		/**
		\brief
		get a global MIDI data value
		- Example: get the MIDI pitch bend controller's current LSB value
				 - uint32_t lsb = processInfo.midiInputData->getGlobalMIDIData(kMIDIPitchBendDataLSB);

		\param index index in the array of MIDI global data values; see enum globalMIDI
		in the synthconstants.h file

		\return the MIDI value coded as an 32 bit unsigned integer 
		*/
		virtual uint32_t getGlobalMIDIData(uint32_t index) = 0;

		/**
		\brief
		get a MIDI CC value
		- Example: get the MIDI volume CC #7
		- uint32_t midiVolumeGain = midiInputData->getCCMIDIData(VOLUME_CC07);

		\param index index in the array of MIDI CC data values; a very few of these are already
		encoded as 	// --- CONTINUOUS CONTROLLERS in synthconstants.h

		\return the MIDI value coded as an 32 bit unsigned integer
		*/
		virtual uint32_t getCCMIDIData(uint32_t index) = 0;

		/**
		\brief
		get aux data as a uint32_t datatype, may or may not be MIDI value

		\param index index in the array values; see enum auxMIDI in synthconstants.h

		\return the MIDI value coded as a 32 bit unsigned integer
		*/
		virtual uint32_t getAuxDAWDataUINT(uint32_t index) = 0;
		
		/**
		\brief
		get aux data as a float datatype, may or may not be MIDI value

		\param index index in the array values; see enum auxMIDI in synthconstants.h

		\return the MIDI value coded as floating point value
		*/
		virtual float getAuxDAWDataFloat(uint32_t index) = 0;
	};

	/**
	\ingroup Constants-Enums
	enumeration for filter output slots in the filter output struct */
	enum { LPF1, LPF2, LPF3, LPF4, HPF1, HPF2, HPF3, HPF4, BPF2, BPF4, BSF2, BSF4, 
		APF1, APF2, ANM_LPF1, ANM_LPF2, ANM_LPF3, ANM_LPF4, NUM_FILTER_OUTPUTS };

	//@{
	/**
	\ingroup Constants-Enums
	constants for GUI control mapping 
	slope = (output_end - output_start) / (input_end - input_start)
	*/
	const double GUI_Q_MIN = 1.0;
	const double GUI_Q_MAX = 10.0;
	const double SVF_Q_SLOPE = (25.0 - 0.707) / (GUI_Q_MAX - GUI_Q_MIN);
	const double KORG35_Q_SLOPE = (2.0 - 0.707) / (GUI_Q_MAX - GUI_Q_MIN);
	const double MOOG_Q_SLOPE = (4.0 - 0.0) / (GUI_Q_MAX - GUI_Q_MIN);
	const double DIODE_Q_SLOPE = (17.0 - 0.0) / (GUI_Q_MAX - GUI_Q_MIN);
	const double HSYNC_MOD_SLOPE = 3.0;
	//@}

	/**
	\struct FilterOutput
	\ingroup SynthStructures
	\brief
	Holds an array of filter output values; SynthLab filters can produce multiple outputs at once. 
	For example, the 1st order VA filter produces 1st order LPF, HPF and APF output samples in the 
	same render() function. 

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct FilterOutput
	{
		FilterOutput() { clearData(); }
		double filter[NUM_FILTER_OUTPUTS];
		void clearData()
		{
			memset(&filter[0], 0, sizeof(double) * NUM_FILTER_OUTPUTS);
		}
	};


	/**
	\class IFilterBase
	\ingroup SynthInterfaces
	\brief
	Simple interface for SynthLab filters
	- set of four interface functions for processing audio through the filter
	- these functions correspond to similar functions for synth modules and cores including
	1. a reset function
	2. an update function to calculate new coefficients
	3. a processing fuction (corresponds to render())
	4. a function to set parameters

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class IFilterBase
	{
		/**
		\brief
		reset the filter to intialized state; called once at init time and
		again any time the sample rate changes

		\return true if sucessful, false otherwise
		*/
		virtual bool reset(double _sampleRate) = 0;

		/**
		\brief
		update the filter due to changes in the GUI controls and/or modulations

		\return true if sucessful, false otherwise
		*/
		virtual bool update() = 0;

		/**
		\brief
		Process audio through the filter. Different filters produce different outputs in the FilterOutput's
		array.

		\return FilterOutput structure containing outputs in pre-defined slots
		*/
		virtual FilterOutput* process(double xn) = 0;

		/**
		\brief
		Sets the two parameters of all synth filters. You can add more here if you need to.

		\param _fc the center or cutoff frequency of the filter
		\param _Q the quality factor (damping) of the filter
		*/
		virtual void setFilterParams(double _fc, double _Q) = 0;
	};


	/**
	\struct CoreProcData
	\ingroup SynthStructures
	\brief
	This structure holds all of the information needed to call functions on a ModuleCore object. This 
	structure must contain datatypes that are basic C++ types or structures of them, and can not 
	contain anything from the std:: library. 
	- this structure must survive being passed across a thunk-barrier for use with dynamic modules
	
	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct CoreProcData
	{
		/** Interfaces to modulation arrays */
		IModulator* modulationInputs = nullptr; ///< input modulation values
		IModulator* modulationOutputs = nullptr;///< output modulation values

		/** old fashioned (naked) pointers can survive the thunk-barrier */
		float** inputBuffers = nullptr;		///< set of input bufers, one per channel
		float** outputBuffers = nullptr;	///< set of output bufers, one per channel
		float** fmBuffers = nullptr;		///< used for DX synths (phase modulator synths)

		/** Pointers to shared resources */
		IWavetableDatabase* wavetableDatabase = nullptr;	///< wavetable database, usually owned by engine
		IPCMSampleDatabase* sampleDatabase = nullptr;		///< PCM sample database, usually owned by engine
		IMidiInputData* midiInputData = nullptr;			///< MIDI input daa, usually owned by engine
		void* moduleParameters = nullptr;					///< module parameters, cloaked as void* -- varies according to module
		const char* dllPath = nullptr;						///< path to the plugin, used for finding PCM sample WAV files

		double sampleRate = 0.0;			///< fs
		uint32_t samplesToProcess = 64;		///< number of samples in this block
		double unisonDetuneCents = 0.0;		///< detuning value for this core
		double unisonStartPhase = 0.0;		///< unison start phase value for this core

		double BPM = 120.0;			///< current BPM, needed for LFO sync to BPM
		MIDINoteEvent noteEvent;	///< the MIDI note event for the current audio block
	};

	// ----------------------------------- SYNTH OBJECTS ----------------------------------------------------- //
	//
	/*
		Fundamental objects that are used throughout SynthLab
	*/
	//
	// ------------------------------------------------------------------------------------------------------- //

	/**
	\struct SynthLabTableSet
	\ingroup SynthStructures
	\brief
	This structure defines a set of wavetables that are usually found in .h files and compiled into 
	the synth. This structure is used specifically with wavetables that are made with RackAFX-TableMaker 
	utility functions, that can encode tables in hex format for storage without needing fractional decimal
	representation.
	- the wavetable oscillator cores all use this format for storing their tables on the database
	- this is NOT REQUIRED as the database only deals with the source interface pointers, so 
	your implementation details are not needed 
	- this is just for registering the tables at initialization time, and is not used afterward
	- this is for example only - feel free to encode and store tables however you wish

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct SynthLabTableSet
	{
		// --- multi initializer construction
		SynthLabTableSet(const char* _waveformName, double _tableFs,
			uint32_t* _tableLengths, uint64_t** _ppHexTableSet, double _outputComp)
			: waveformName(_waveformName)
			, tableFs(_tableFs)
			, tableLengths(_tableLengths)
			, ppHexTableSet(_ppHexTableSet)
			, outputComp(_outputComp)
		{ }

		// --- ptr to array of table lengths
		uint32_t* tableLengths = nullptr;	///< pointers to lengths of each of the hex encoded tables
		uint64_t** ppHexTableSet = nullptr; ///< pointers to sets of hex encoded tables

		// --- fs for this table set
		double tableFs = 44100.0;

		// --- output scaling factor (NOT volume or attenuation, waveform specific)
		double outputComp = 1.0; ///< output scaling factor

		// --- GUI string for this waveform
		const char* waveformName; ///< string for the GUI
	};

	// --- fwd decl
	class StaticTableSource;
	/**
	\struct SynthLabBankSet
	\ingroup SynthStructures
	\brief
	This super-structure holds a set of SynthLabTableSet called a "bank" and used in the morphing wavetable
	core to register and install these tables with the database. 
	- this is NOT REQUIRED as the database only deals with the source interface pointers, so
	your implementation details are not needed
	- this is for example only - feel free to encode and store tables however you wish; ultimately
	this just aggregates a set of wavetables into a conceptual "bank" but all oscillators access and
	read wavetables the same way so this is just for registering the tables at initialization time, and 
	is not used afterward

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct SynthLabBankSet
	{
		SynthLabBankSet(unsigned int _tablePtrsCount, SynthLabTableSet** _tablePtrs, std::string* _tableNames, StaticTableSource* _staticSources = nullptr)
			: tablePtrsCount(_tablePtrsCount)
			, tablePtrs(_tablePtrs)
			, tableNames(_tableNames)
		    , staticSources(_staticSources) {}

		unsigned int tablePtrsCount = 32; ///< number of pointers
		SynthLabTableSet** tablePtrs = nullptr; ///< set of table-sets
		std::string* tableNames = nullptr; ///< names of tables
		StaticTableSource* staticSources = nullptr;
	};
	
	/**
	\ingroup Constants-Enums
	the default wavetable length */
	const uint32_t kDefaultWaveTableLength = 256;

	/**
	\struct StaticWavetable
	\ingroup SynthStructures
	\brief
	Structure for holding information about a static wavetable, that is read from a static location, 
	either compiled into the synth as a resource, or from a binary data file at startup time. 
	- the only real difference between static and dynamic wavetables as far as this structure is concerned
	is that static tables may be encoded as 64-bit HEX data OR as doubles whereas dynamic tables are encoded
	with double values that are calculated at load time

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct StaticWavetable
	{
		StaticWavetable() {}

		StaticWavetable(const uint64_t* _table, uint32_t _tableLength, const char* _waveformName,
			double _outputComp = 1.0, double _tableFs = 44100)
			: uTable(_table)
			, tableLength(_tableLength)
			, waveformName(_waveformName)
			, outputComp(_outputComp)
			, tableFs(_tableFs)
		{
			wrapMask = tableLength - 1;
			dTable = nullptr;
		}

		StaticWavetable(const double* _table, uint32_t _tableLength, const char* _waveformName,
			double _outputComp = 1.0, double _tableFs = 44100)
			: dTable(_table)
			, tableLength(_tableLength)
			, waveformName(_waveformName)
			, outputComp(_outputComp)
			, tableFs(_tableFs)
		{
			wrapMask = tableLength - 1;
			uTable = nullptr;
		}

		// --- either/or static tables
		const uint64_t* uTable;	///< table of 64-bit HEX values
		const double*	dTable;	///< table of 64-bit doubles
		uint32_t tableLength = kDefaultWaveTableLength; ///< length
		uint32_t wrapMask = kDefaultWaveTableLength - 1; ///< wrapping mask = length - 1
		double outputComp = 1.0;	///< output scaling factor (NOT volume or attenuation, waveform specific)
		double tableFs = 44100.0;
		const char* waveformName;	///< waveform name string
	};

	/**
	\struct DynamicWavetable
	\ingroup SynthStructures
	\brief
	Structure for holding information about a static wavetable, that is read from a static location,
	either compiled into the synth as a resource, or from a binary data file at startup time.
	- the only main difference between static and dynamic wavetables as far as this structure is concerned
	is that:
	- static tables may be encoded as 64-bit HEX data OR as doubles and referenced with naked pointers
	- dynamic tables are encoded with double values that are calculated at load time and referenced
	with shared pointers so that the dynamically gererated tables are disposed of properly

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct DynamicWavetable
	{
		DynamicWavetable() {}

		DynamicWavetable(std::shared_ptr<double> _table, uint32_t _tableLength, const char* _waveformName,
			double _outputComp = 1.0, double _tableFs = 44100)
			: table(_table)
			, tableLength(_tableLength)
			, waveformName(_waveformName)
			, outputComp(_outputComp)
			, tableFs(_tableFs)
		{
			wrapMask = tableLength - 1;
		}

		std::shared_ptr<double> table = nullptr; ///< the table (shared)
		uint32_t tableLength = kDefaultWaveTableLength; ///< length
		uint32_t wrapMask = kDefaultWaveTableLength - 1; ///< mask = length - 1
		double outputComp = 1.0;	///< output scaling factor (NOT volume or attenuation, waveform specific)
		double tableFs = 44100.0;
		const char* waveformName;   ///< waveform name
	};


	/**
	\struct WavetableDatabase
	\ingroup SynthObjects
	\brief
	Object that acts as the wavetable database, as shared synth-wide resource. You should study this
	especially if you want to implement your own database with your own wavetable formats, etc...
	- exposes the IWavetableDatabase; your own object only needs to expose this interface
	- this is an example object to study if you want to roll your own version
	- the wavetable sources in the database are uniquely identified with their name strings
	- a std::map is used to make the dictionary of table sources

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class WavetableDatabase : public IWavetableDatabase
	{
	public:
		WavetableDatabase() {}
		~WavetableDatabase();

		/** IWavetableDatabase overrides */
		virtual IWavetableSource* getTableSource(const char* uniqueTableName) override;
		virtual IWavetableSource* getTableSource(uint32_t uniqueTableIndex) override;
		virtual bool addTableSource(const char* uniqueTableName, IWavetableSource* tableSource, uint32_t& uniqueIndex) override;
		virtual bool removeTableSource(const char* uniqueTableName) override;
		virtual bool clearTableSources() override;
        virtual int32_t getWaveformIndex(const char* uniqueTableName) override;

		/** convenience function to return this as interface pointer */
		IWavetableDatabase* getIWavetableDatabase() { return this; }

	protected:
		typedef std::map < std::string, IWavetableSource* > wavetableSourceMap; ///< map that connects wavetable names to source objects
		wavetableSourceMap wavetableDatabase;
		std::vector<IWavetableSource*> wavetableVector;
	};


	/**
	\struct PCMSampleDatabase
	\ingroup SynthObjects
	\brief
	Object that acts as the PCM sample database, as shared synth-wide resource. You should study this
	especially if you want to implement your own database with your own PCM sample formats, etc...
	- exposes the IPCMSampleDatabase; your own object only needs to expose this interface
	- this is an example object to study if you want to roll your own version
	- the PCM sources in the database are uniquely identified with their name strings
	- a std::map is used to make the dictionary of table sources

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class PCMSampleDatabase : public IPCMSampleDatabase
	{
	public:
		PCMSampleDatabase() {}
		~PCMSampleDatabase();

		/** IPCMSampleDatabase overrides */
		virtual IPCMSampleSource* getSampleSource(const char* uniqueSampleSetName) override;
		virtual bool addSampleSource(const char* uniqueSampleSetName, IPCMSampleSource* sampleSource) override;
		virtual bool removeSampleSource(const char* uniqueSampleSetName) override;
		virtual bool clearSampleSources() override;

		/** convenience function to return this as interface pointer */
		IPCMSampleDatabase* getIPCMSampleDatabase() { return this; }

	protected:
		typedef std::map < std::string, IPCMSampleSource* >sampleSourceMap;
		sampleSourceMap sampleDatabase;///< map that connects PCM sample set names to source objects
		std::vector<IPCMSampleSource*> sources;
	};

	/**
	\class SynthProcessInfo
	\ingroup SynthObjects
	\brief
	This structure holds all of the information needed to for the plugin framework to send MIDI information into
	the engine, and receive rendered audio samples that result. 
	- derived from AudioBuffer and inherits its input and outout audio buffers directly from the base class
	- many function calls to this object will be to the AudioBuffer below
	- adds functions and storage for MIDI events
	- includes special aux data from the DAW such as BPM

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class SynthProcessInfo : public AudioBuffer
	{
	public:
		SynthProcessInfo(){}
		SynthProcessInfo(uint32_t _numInputChannels, uint32_t _numOutputChannels, uint32_t _blockSize);
		~SynthProcessInfo() {}

		/** MIDI events and functions*/
		void pushMidiEvent(midiEvent event);
		void clearMidiEvents();
		uint64_t getMidiEventCount();
		midiEvent* getMidiEvent(uint32_t index);

		/** Aux information from the DAW */
		double absoluteBufferTime_Sec = 0.0;			///< the time in seconds of the sample index at top of buffer
		double BPM = 0.0;								///< beats per minute, aka "tempo"
		double timeSigNumerator = 0.0;					///< time signature numerator
		uint32_t timeSigDenomintor = 0;					///< time signature denominator

	protected:
		/** set of MIDI events for this audio processing block */
		std::vector<midiEvent> midiEventQueue;          ///< queue 
	};

	/**
	\class MidiInputData
	\ingroup SynthObjects
	\brief
	Holds all MIDI input data values.
	- exposes the IMidiInputData interface
	- allows access to the three types of MIDI input data
	- has function that returns (this) cast as the IMidiInputData pointer

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class MidiInputData : public IMidiInputData
	{
	public:
		MidiInputData();
		~MidiInputData() {}

		/** IMidiInputData access functions */
		virtual uint32_t getGlobalMIDIData(uint32_t index);
		virtual uint32_t getCCMIDIData(uint32_t index);
		virtual uint32_t getAuxDAWDataUINT(uint32_t index);
		virtual float getAuxDAWDataFloat(uint32_t index);
		
		/** functions to set the incoming MIDI data */
		void setGlobalMIDIData(uint32_t index, uint32_t value);
		void setCCMIDIData(uint32_t index, uint32_t value);
		void setAuxDAWDataUINT(uint32_t index, uint32_t value);
		void setAuxDAWDataFloat(uint32_t index, float value);

		/** get this cast as interface pointer */
		IMidiInputData* getIMIDIInputData() { return this; }

	protected:
		// --- shared MIDI tables, via IMIDIData
		uint32_t globalMIDIData[kNumMIDIGlobals];	///< the global MIDI INPUT table that is shared across the voices via the IMIDIData interface
		uint32_t ccMIDIData[kNumMIDICCs];			///< the global MIDI CC INPUT table that is shared across the voices via the IMIDIData interface
		double auxData[kNumMIDIAuxes];				///< the aux values, specific to SynthLab
	};

	/**
	\class Modulators
	\ingroup SynthObjects
	\brief
	Implements a modulator object.
	- exposes the IModulator interface
	- each SynthModule owns two Modulators objects, one for the modulation input values (that come from 
	modulation sources) and one for the modulation outputs, if the module writes to them (i.e. if the module
	is a modulator object)

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class Modulators : public IModulator
	{
	public:
		Modulators();

		/** for modulation matrix to have instant access to the input array */
		virtual double* getModArrayPtr(uint32_t index);
		virtual double getModValue(uint32_t index);
		virtual void setModValue(uint32_t index, double value);

		// --- fast clearing of array
		void clear();

		/** set defaults to avoid accidental note-off state, e.g. MIDI volume is accidentally initialized to 0.0 */
		void initInputValues();

		/** helper function to return pointer to interface */
		IModulator* getModulatorPtr() { return this; }

	protected:
		// --- array of modulation inputs or outputs
		double modArray[MAX_MODULATION_CHANNELS]; 
	};

	/**
	\class ModuleCore
	\ingroup SynthObjects
	\brief
	Abstract base class that encapsulates functionality of a module core; used with the Module-Core paradigm. 
	- each Core is a special variation on the Module that owns it
	- for example, the wavetable oscillator object owns four cores that implement
	four different kinds of wavetables from classic waveforms, to electronic drums
	- this is the most important object in SynthLab for getting started
	- you can compile SynthLab DM cores that impelement only one of these objects
	- although this is an abstract base class, it is not an interface because it contains
	non-abstract functions and implements some very low level functionality itself to make
	it easier to use the derived classes in a stand-alone manner

	Abstract Functions:
	- note that the main five abstract functions all take the same argument, CoreProcData
	- this argument is capable of surviving calls acrss the thunk-layer
	- the owning SynthModule will prepare the CoreProcData argument for the core to use

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class ModuleCore
	{
	public:
		/** 
		\brief 
		Constructs a ModuleCore
		- just resets the glide modulator
		*/
		ModuleCore() { glideModulator.reset(new(GlideModulator)); }
        virtual ~ModuleCore() { }

		/** abstract base class functions: see synth book for details*/
		virtual bool reset(CoreProcData& processInfo) = 0;
		virtual bool update(CoreProcData& processInfo) = 0;
		virtual bool render(CoreProcData& processInfo) = 0;
		virtual bool doNoteOn(CoreProcData& processInfo) = 0;
		virtual bool doNoteOff(CoreProcData& processInfo) = 0;

		/** ModuleCore virtual functions for EGs only -- they need a few additional methods */
		virtual int32_t getState() { return -1; }
		virtual bool shutdown() { return false; }
		virtual void setSustainOverride(bool sustain) { return; }
		virtual void setStandAloneMode(bool b) { standAloneMode = b; }

		/** to start glide modulation directly, without needing a derived class pointer */
		bool startGlideModulation(GlideInfo& glideInfo) {
			return glideModulator->startModulator(glideInfo.startMIDINote, glideInfo.endMIDINote, glideInfo.glideTime_mSec, glideInfo.sampleRate);
		}

		/**  needed for dynamic loading/unloading */
		uint32_t getModuleType() { return moduleType; }
		const char* getModuleName() { return moduleName; }
		void* getModuleHandle() { return moduleHandle; }
		void  setModuleHandle(void* handle) { moduleHandle = handle; }
		uint32_t getModuleIndex() { return moduleIndex; }
		void  setModuleIndex(uint32_t index) { moduleIndex = index; }
		int32_t getPreferredModuleIndex() { return preferredIndex; }
		void  setPreferredModuleIndex(uint32_t index) { preferredIndex = index; }

		/** 
		\brief
		provides access to the core data: 
		- this includes the 16 module strings (these are waveform names 
		for oscillators, filter types for filters, etc...)
		- and the four mod-knob labels; this is only used for systems that can load
		dynamically filled GUI controls

		*/
		ModuleCoreData& getModuleData() { return coreData; }

	protected:
		// --- module
		uint32_t moduleType = UNDEFINED_MODULE; ///< type of module, LFO_MODULE, EG_MODULE, etc...
		const char* moduleName = nullptr;		///< module name must be set in derived constructor
		void* moduleHandle = nullptr;			///< used for dynamically loading cores from DLLs
		uint32_t moduleIndex = 0;				///< index of this core
		int32_t preferredIndex = -1;			///< preferred index of this DYNAMIC core
		ModuleCoreData coreData;				///< core strings (16) and mod knob labels (4)
		bool standAloneMode = false;			///< flag for stand-alone mode of operation outside of SynthLab
		std::unique_ptr<GlideModulator> glideModulator;	///< built-in glide modulator for oscillators
	};

	/**
	\class SynthModule
	\ingroup SynthObjects
	\brief
	Abstract base class that encapsulates functionality of a module; used with the Module-Core paradigm.
	- almost all Modules are generalized, and own up to four (4) module core objects
	that actually implement the specialized behavior
	- objects derived from SynthModule ARE NOT OBLIGED to follow the module-core paradigm, nor
	are they required to hold, initialize, and maintain the four cores; instead they may
	implemement the module functionality directly
	- the WSOScillator is an example of a SynthModule that does not use any ModuleCore objects
	- the five main abstract functions will all map to the same named functions on the 
	ModuleCore objects, if the SynthModule chooses to use the Module-Core paradigm
	- for most objects, this is a very simple implementation that is mainly responsible for
	initializing and maintainig its set of four ModuleCores and these are very thin objects

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class SynthModule
	{
	public:
		SynthModule(std::shared_ptr<MidiInputData> _midiInputData);
        virtual ~SynthModule();

		/** abstract base class functions: see synth book for details*/
		virtual bool reset(double _sampleRate) = 0;
		virtual bool update() = 0;
		virtual bool render(uint32_t samplesToProcess = 1) = 0;
		virtual bool doNoteOn(MIDINoteEvent& noteEvent) = 0;
		virtual bool doNoteOff(MIDINoteEvent& noteEvent) = 0;

		/** for knowing the path to the DLL itself for loading PCM samples or other resources */
		virtual bool initialize(const char* _dllDirectory) { dllDirectory = _dllDirectory;  return true; }

		/** EG only */
		virtual int32_t getState() { return -1; }
		virtual bool shutdown() { return false; }

		/** for starting glide modulation on underlying objects */
		virtual bool startGlideModulation(GlideInfo& glideInfo);

		/**  access to modulators */
		std::shared_ptr<Modulators> getModulationInput() { return modulationInput; }
		std::shared_ptr<Modulators> getModulationOutput() { return modulationOutput; }

		/**  access to audio buffers (I/O) */
		std::shared_ptr<AudioBuffer> getAudioBuffers() { return audioBuffers; }

		/**  --- for unison mode only */
		void setUnisonMode(double _unisonDetuneCents, double _unisonStarPhase) {
			unisonDetuneCents = _unisonDetuneCents; unisonStartPhase = _unisonStarPhase;
		}

		/** for DX synths only */
		void setFMBuffer(std::shared_ptr<AudioBuffer> pmBuffer) { fmBuffer = pmBuffer; }
		void clearFMBuffer() { fmBuffer = nullptr; }

		/** functions for dealing with the module cores; SynthLab only; these may be ignored in 
		many stand-alone implementations */
		virtual bool getModuleStrings(std::vector<std::string>& moduleStrings, std::string ignoreStr = "");
		virtual bool getModuleStrings(uint32_t coreIndex, std::vector<std::string>& moduleStrings, std::string ignoreStr);
		virtual bool getAllModuleStrings(std::vector<std::string>& moduleStrings, std::string ignoreStr);
		virtual bool getModKnobStrings(std::vector<std::string>& modKnobStrings);
		virtual bool getModKnobStrings(uint32_t coreIndex, std::vector<std::string>& modKnobStrings);
		virtual bool getModuleCoreStrings(std::vector<std::string>& moduleCoreStrings);
		virtual bool addModuleCore(std::shared_ptr<ModuleCore> core);
		virtual uint32_t getSelectedCoreIndex();
		virtual bool selectModuleCore(uint32_t index);
		virtual bool selectDefaultModuleCore();
		virtual void packCores();
		virtual bool clearModuleCores();
		virtual void setStandAloneMode(bool b);

	protected:
		/** modulation input bus */
		std::shared_ptr<Modulators> modulationInput = std::make_shared<Modulators>();

		/**  modulation outputs bus */
		std::shared_ptr<Modulators> modulationOutput = std::make_shared<Modulators>();

		/**  MIDI Data Interface */
		std::shared_ptr<MidiInputData> midiInputData = nullptr;

		/**  audio buffers for pitched oscillator and filters and DCA */
		std::shared_ptr<AudioBuffer> audioBuffers = nullptr;

		/**  glide modulator */
		std::unique_ptr<GlideModulator> glideModulator;

		/**  audio buffers for pitched oscillator and filters and DCA */
		std::shared_ptr<AudioBuffer> fmBuffer = nullptr;

		/**  for core-specific stuff */
		std::shared_ptr<ModuleCore> moduleCores[NUM_MODULE_CORES];
		std::shared_ptr<ModuleCore> selectedCore = nullptr;

		/**  for modules without cores */
		ModuleCoreData moduleData;	///< modulestrings (16) and mod knob labels (4)

		double unisonDetuneCents = 0.0;
		double unisonStartPhase = 0.0;
		bool standAloneMode = false;

		/** helpers for dealing with Cores */
		CoreProcData coreProcessData;
		std::string dllDirectory;
	};


	// ----------------------------------- FX-FILTERING OBJECTS ---------------------------------------------- //
	//
	/*
		Objects taken primarily from Will Pirkle's FX book (2nd edition)
		Most objects are heavily detailed in the book Designing Audio Effects Plugins in C++ 2nd Ed and
		at http://aspikplugins.com/sdkdocs/html/index.html
	*/
	//
	// ------------------------------------------------------------------------------------------------------- //
	
	/**
	@doLinearInterp
	\ingroup SynthFunctions

	@brief performs linear interpolation of fractional x distance between two adjacent (x,y) points;
	returns interpolated value

	\param y1 - the y coordinate of the first point
	\param y2 - the 2 coordinate of the second point
	\param fractional_X - the interpolation location as a fractional distance between x1 and x2 (which are not needed)
	\return the interpolated value or y2 if the interpolation is outside the x interval
	*/
	inline double doLinearInterp(double y1, double y2, double fractional_X)
	{
		// --- check invalid condition
		if (fractional_X >= 1.0) return y2;

		// --- use weighted sum method of interpolating
		return fractional_X*y2 + (1.0 - fractional_X)*y1;
	}

	/**
	\class CircularBuffer
	\ingroup FX-FilteringObjects
	\brief
	The CircularBuffer object implements a simple circular buffer. It uses a wrap mask to wrap the read 
	or write index quickly. 
	- Heavily detailed in the book Designing Audio Effects Plugins in C++ 2nd Ed and 
	at http://aspikplugins.com/sdkdocs/html/index.html

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	template <typename T>
	class CircularBuffer
	{
	public:
		CircularBuffer() {}		/* C-TOR */
		~CircularBuffer() {}	/* D-TOR */

		/** flush buffer by resetting all values to 0.0 */
		void flushBuffer() { memset(&buffer[0], 0, bufferLength * sizeof(T)); }

		/** Create a buffer based on a target maximum in SAMPLES
		//	   do NOT call from realtime audio thread; do this prior to any processing */
		void createCircularBuffer(uint32_t _bufferLength)
		{
			// --- find nearest power of 2 for buffer, and create
			createCircularBufferPowerOfTwo((uint32_t)(pow(2, ceil(log(_bufferLength) / log(2)))));
		}

		/** Create a buffer based on a target maximum in SAMPLESwhere the size is
		pre-calculated as a power of two */
		void createCircularBufferPowerOfTwo(uint32_t _bufferLengthPowerOfTwo)
		{
			// --- reset to top
			writeIndex = 0;

			// --- find nearest power of 2 for buffer, save it as bufferLength
			bufferLength = _bufferLengthPowerOfTwo;

			// --- save (bufferLength - 1) for use as wrapping mask
			wrapMask = bufferLength - 1;

			// --- create new buffer
			buffer.reset(new T[bufferLength]);

			// --- flush buffer
			flushBuffer();
		}

		/** write a value into the buffer; this overwrites the previous oldest value in the buffer */
		void writeBuffer(T input)
		{
			// --- write and increment index counter
			buffer[writeIndex++] = input;

			// --- wrap if index > bufferlength - 1
			writeIndex &= wrapMask;
		}

		/** read an arbitrary location that is delayInSamples old */
		T readBuffer(int32_t delayInSamples)//, bool readBeforeWrite = true)
		{
			// --- subtract to make read index
			//     note: -1 here is because we read-before-write,
			//           so the *last* write location is what we use for the calculation
			int32_t readIndex = (writeIndex - 1) - delayInSamples;

			// --- autowrap index
			readIndex &= wrapMask;

			// --- read it
			return buffer[readIndex];
		}

		/** read an arbitrary location that includes a fractional sample */
		T readBuffer(double delayInFractionalSamples)
		{
			// --- truncate delayInFractionalSamples and read the int part
			int32_t intPart = (int32_t)delayInFractionalSamples;

			T y1 = readBuffer(intPart);

			// --- if no interpolation, just return value
			if (!interpolate) return y1;

			// --- else do interpolation
			//
			int readIndexNext = intPart + 1;

			// --- autowrap index
			readIndexNext &= wrapMask;

			// --- read the sample at n+1 (one sample OLDER)
			T y2 = readBuffer(readIndexNext);

			// --- get fractional part
			double fraction = delayInFractionalSamples - intPart;

			// --- do the interpolation (you could try different types here)
			return doLinearInterp(y1, y2, fraction);
		}

		/** enable or disable interpolation; usually used for diagnostics or in algorithms that require strict integer samples times */
		void setInterpolate(bool b) { interpolate = b; }

	private:
		std::unique_ptr<T[]> buffer = nullptr;	///< smart pointer will auto-delete
		uint32_t writeIndex = 0;		///> write index
		uint32_t bufferLength = 1024;	///< must be nearest power of 2
		uint32_t wrapMask = 1023;		///< must be (bufferLength - 1)
		bool interpolate = true;			///< interpolation (default is ON)
	};

	/**
	\class CircularBuffer
	\ingroup FX-FilteringObjects
	\brief
	Simple delay line object implements audio delay as a circular buffer.
	- used as the delay line for the Resonator object with Karplus-Strong string simluation
	- intialized with lowest oscillator value (in Hz) that the resonator can synthesize
	rather than the usual delay time
	- taken directly from the AudioDelay object 
	- Heavily detailed in the book Designing Audio Effects Plugins in C++ 2nd Ed and
	at http://aspikplugins.com/sdkdocs/html/index.html

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class DelayLine
	{
	public:
		DelayLine(void) {}	/* C-TOR */
		~DelayLine(void) {}	/* D-TOR */

	public:
		/** flush the delay with 0s */
		void clear(){ delayBuffer.flushBuffer();}

		/** 
		\brief
		reset the delay, calculate a new length based on sample rate and minimum pitch
		
		\param _sampleRate fs
		\param minimumPitch pitch in Hz of the lowest note that could fit into this delay line
		*/
		void reset(double _sampleRate, double minimumPitch = MIDI_NOTE_0_FREQ) // 8.176 = MIDI note 0
		{
			// ---
			delayBuffer.createCircularBuffer( ((uint32_t)(_sampleRate / minimumPitch)) );
			clear();
		}

		/**
		\brief
		set delay time in samples; used to access the delay when reading

		\param _delaySamples delay in samples, may be fractional 
		*/	
		void setDelayInSamples(double _delaySamples)
		{
			delaySamples = _delaySamples;
		}

		/**
		\brief
		write a value into the top of the delay

		\param xn value to write
		*/
		void writeDelay(double xn)
		{
			// --- write to delay buffer
			delayBuffer.writeBuffer(xn);
		}

		/**
		\brief
		read a delayed value at the location specified in the call to setDelayInSamples()

		\return the freshly read delay value
		*/
		double readDelay()
		{
			double yn = delayBuffer.readBuffer(delaySamples);
			return yn;
		}

	private:
		// --- our only variable
		double delaySamples = 0; ///< delay time in samples

		// --- delay buffer of doubles
		CircularBuffer<double> delayBuffer; ///< circular buffer to implement delay
	};


	/**
	\struct BQCoeffs
	\ingroup FX-FilteringObjects
	\brief
	Structure to hold the seven coeffieicents used in the AudioFilter object from
	Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	struct BQCoeffs
	{
		// --- filter coefficients
		double coeff[7] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	};

	/**
	\class BQAudioFilter
	\ingroup FX-FilteringObjects
	\brief
	Simple version of the AudioFilter object from Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	that impelments a biquad audio filter. 
	- see the book and website for more details

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class BQAudioFilter
	{
	public:
		BQAudioFilter(void) {}	/* C-TOR */
		~BQAudioFilter(void) {}	/* D-TOR */

	public:
		/** reset the object */
		void reset(){ flushDelays(); }

		/** flush state variables */
		void flushDelays()
		{
			for (uint32_t i = 0; i<numStates; i++)
				state[i] = 0.0;
		}

		/** set biquad coeffieicnets directly */
		void setCoeffs(BQCoeffs& _coeffs) {
			bq = _coeffs;
		}

		/** copy biquad coeffieicnets to a destination */
		void copyCoeffs(BQAudioFilter& destination) {
			destination.setCoeffs(bq);
		}

		/** run the filter

		\param xn the input sample

		\return the filtered output
		*/
		double processAudioSample(double xn)
		{
			// --- transposed canonical
			double yn = bq.coeff[a0] * xn + state[xz1];

			// --- shuffle/update
			state[xz1] = bq.coeff[a1] * xn - bq.coeff[b1] * yn + state[xz2];
			state[xz2] = bq.coeff[a2] * xn - bq.coeff[b2] * yn;
			return xn*bq.coeff[d0] + yn*bq.coeff[c0];
		}

	protected:
		enum { a0, a1, a2, b1, b2, c0, d0 };
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 }; ///< state registers
		BQCoeffs bq;	///< coefficients
	};


	/**
	\class FracDelayAPF
	\ingroup FX-FilteringObjects
	\brief
	Implements a first order APF that is used to generate a fractional delay for the physcial model of a string.
	- you set the single coefficient alpha directly
	- this really just runs a simple 1st order feedbadk/feedforward structure and does no calculation

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class FracDelayAPF
	{
	public:
		FracDelayAPF(void) {}	/* C-TOR */
		~FracDelayAPF(void) {}	/* D-TOR */

	public:
		/** clear state registers */
		void reset()
		{
			state[0] = 0.0;
			state[1] = 0.0;
		}

		/** set the single coefficient */
		void setAlpha(double _alpha)
		{
			alpha = _alpha;
		}

		/** 
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn)
		{
			double yn = xn*alpha + state[0] - alpha*state[1];
			state[0] = xn;
			state[1] = yn;
			return yn;
		}

	private:
		// --- our only coefficient
		double alpha = 0.0; ///< single coefficient
		double state[2] = { 0.0, 0.0 }; ///< state registers
	};


	/**
	\class ResLoopFilter
	\ingroup FX-FilteringObjects
	\brief
	Implements a first order feedforward LPF with coefficients a0 = a1 = 0.5
	- generates LPF with zero at Nyquist
	- generates exactly 1/2 sample of delay
	- used for tuning the resonator in a plucked string model for an exact pitch

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class ResLoopFilter
	{
	public:
		ResLoopFilter(void) {}	/* C-TOR */
		~ResLoopFilter(void) {}	/* D-TOR */

	public:
		/** flush state variables */
		void reset()
		{
			state[0] = 0.0;
			state[1] = 0.0;
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn)
		{
			double yn = 0.5*xn + 0.5*state[0];
			state[0] = xn;
			return yn;
		}

	private:
		double state[2] = { 0.0, 0.0 }; ///< state variables
	};


	/**
	\class DCRemovalFilter
	\ingroup FX-FilteringObjects
	\brief
	Implements a first order HPF with fc = 2.0Hz
	- taken from the AudioFilter object in the FX book below
	- generates HPF with zero at 0.0Hz
	- can not change fc after construction

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class DCRemovalFilter
	{
	public:
		DCRemovalFilter(void) {}	/* C-TOR */
		~DCRemovalFilter(void) {}	/* D-TOR */

	public:
		/** flush state variables and set the fc value, that is usually just hardcoded*/
		void reset(double _sampleRate)
		{
			for (uint32_t i = 0; i<numStates; i++)
				state[i] = 0.0;

			sampleRate = _sampleRate;

			// --- see book for formulae
			double theta_c = kTwoPi*fc / sampleRate;
			double gamma = cos(theta_c) / (1.0 + sin(theta_c));

			// --- update coeffs
			coeffs[a0] = (1.0 + gamma) / 2.0;
			coeffs[a1] = -(1.0 + gamma) / 2.0;
			coeffs[a2] = 0.0;
			coeffs[b1] = -gamma;
			coeffs[b2] = 0.0;
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn)
		{
			// --- transposed canonical
			double yn = coeffs[a0] * xn + state[xz1];

			// --- shuffle/update
			state[xz1] = coeffs[a1] * xn - coeffs[b1] * yn + state[xz2];
			state[xz2] = coeffs[a2] * xn - coeffs[b2] * yn;
			return yn;
		}

	protected:
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 };		///< state variables
		enum { a0, a1, a2, b1, b2 };
		double coeffs[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };	///< biquad coefficients
		double fc = 1.0;								///< hardcoded fc value
		double sampleRate = 1.0;
	};


	/**
	\class TinyBPF
	\ingroup FX-FilteringObjects
	\brief
	Implements a simple 2nd order BPF
	- taken from the AudioFilter object in the FX book below
	- can set the fo and Q values
	- uses biquad filter

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class TinyBPF
	{
	public:
		TinyBPF(void) {}	/* C-TOR */
		~TinyBPF(void) {}	/* D-TOR */

	public:
		/** flush state variables */
		void reset(double _sampleRate)
		{
			for (uint32_t i = 0; i<numStates; i++)
				state[i] = 0.0;

			sampleRate = _sampleRate;
		}

		/** set the fo and Q values */
		void setParameters(double _fc, double _Q)
		{
			if (fc == _fc && Q == _Q)
			return;

			fc = _fc;
			Q = _Q;

			// --- see book for formulae
			double K = tan(kPi*fc / sampleRate);
			double delta = K*K*Q + K + Q;

			// --- update coeffs
			coeffs[a0] = K / delta;;
			coeffs[a1] = 0.0;
			coeffs[a2] = -K / delta;
			coeffs[b1] = 2.0*Q*(K*K - 1) / delta;
			coeffs[b2] = (K*K*Q - K + Q) / delta;
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn)
		{
			// --- transposed canonical
			double yn = coeffs[a0] * xn + state[xz1];

			// --- shuffle/update
			state[xz1] = coeffs[a1] * xn - coeffs[b1] * yn + state[xz2];
			state[xz2] = coeffs[a2] * xn - coeffs[b2] * yn;
			return yn;
		}

	protected:
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 };
		enum { a0, a1, a2, b1, b2 };
		double coeffs[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
		double fc = 5.0;
		double Q = 0.5;
		double sampleRate = 1.0;
	};

	/**
	\class LP2Filter
	\ingroup FX-FilteringObjects
	\brief
	Implements a simple 2nd order LPF
	- taken from the AudioFilter object in the FX book below
	- can set the fc and Q values
	- uses biquad filter

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class LP2Filter
	{
	public:
		LP2Filter(void) {}	/* C-TOR */
		~LP2Filter(void) {}	/* D-TOR */

	public:
		/** reset object to init state */
		void reset(double _sampleRate)
		{
			sampleRate = _sampleRate;
			clear();
		}

		/** flush state variables */
		void clear()
		{
			for (uint32_t i = 0; i<numStates; i++)
				state[i] = 0.0;
		}

		/** set the filter fc and Q */
		void setParameters(double _fc, double _Q)
		{
			if (fc == _fc && Q == _Q)
				return;

			fc = _fc;
			Q = _Q;

			// --- see book for formulae
			double theta_c = 2.0*kPi*fc / sampleRate;
			double d = 1.0 / Q;
			double betaNumerator = 1.0 - ((d / 2.0)*(sin(theta_c)));
			double betaDenominator = 1.0 + ((d / 2.0)*(sin(theta_c)));

			double beta = 0.5*(betaNumerator / betaDenominator);
			double gamma = (0.5 + beta)*(cos(theta_c));
			double alpha = (0.5 + beta - gamma) / 2.0;

			// --- update coeffs
			coeffs[a0] = alpha;
			coeffs[a1] = 2.0*alpha;
			coeffs[a2] = alpha;
			coeffs[b1] = -2.0*gamma;
			coeffs[b2] = 2.0*beta;
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn)
		{
			// --- transposed canonical
			double yn = coeffs[a0] * xn + state[xz1];

			// --- shuffle/update
			state[xz1] = coeffs[a1] * xn - coeffs[b1] * yn + state[xz2];
			state[xz2] = coeffs[a2] * xn - coeffs[b2] * yn;
			return yn;
		}

	protected:
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 };
		enum { a0, a1, a2, b1, b2 };
		double coeffs[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
		double fc = 5.0;
		double Q = 0.5;
		double sampleRate = 1.0;
	};


	/**
	\class HP2Filter
	\ingroup FX-FilteringObjects
	\brief
	Implements a simple 2nd order HPF
	- taken from the AudioFilter object in the FX book below
	- can set the fc and Q values
	- uses biquad filter

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class HP2Filter
	{
	public:
		HP2Filter(void) {}	/* C-TOR */
		~HP2Filter(void) {}	/* D-TOR */

	public:
		/** reset to init state */
		void reset(double _sampleRate)
		{
			for (uint32_t i = 0; i<numStates; i++)
				state[i] = 0.0;

			sampleRate = _sampleRate;
		}

		/** set filter fc and Q */
		void setParameters(double _fc, double _Q)
		{
			if (fc == _fc && Q == _Q)
				return;

			fc = _fc;
			Q = _Q;

			double theta_c = kTwoPi*fc / sampleRate;
			double d = 1.0 / Q;

			// --- see book for formulae
			double betaNumerator = 1.0 - ((d / 2.0)*(sin(theta_c)));
			double betaDenominator = 1.0 + ((d / 2.0)*(sin(theta_c)));

			double beta = 0.5*(betaNumerator / betaDenominator);
			double gamma = (0.5 + beta)*(cos(theta_c));
			double alpha = (0.5 + beta + gamma) / 2.0;

			// --- update coeffs
			coeffs[a0] = alpha;
			coeffs[a1] = -2.0*alpha;
			coeffs[a2] = alpha;
			coeffs[b1] = -2.0*gamma;
			coeffs[b2] = 2.0*beta;
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn)
		{
			// --- transposed canonical
			double yn = coeffs[a0] * xn + state[xz1];

			// --- shuffle/update
			state[xz1] = coeffs[a1] * xn - coeffs[b1] * yn + state[xz2];
			state[xz2] = coeffs[a2] * xn - coeffs[b2] * yn;
			return yn;
		}

	protected:
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 };
		enum { a0, a1, a2, b1, b2 };
		double coeffs[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
		double fc = 5.0;
		double Q = 0.5;
		double sampleRate = 1.0;
	};

	/**
	\class TinyReson
	\ingroup FX-FilteringObjects
	\brief
	Minute implementation of a 2nd order resonator filter
	- taken from the AudioFilter object in the FX book below
	- can set the fc and Q values
	- uses biquad filter

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class TinyReson
	{
	public:
		TinyReson(void) {}	/* C-TOR */
		~TinyReson(void) {}	/* D-TOR */

	public:
		/** reset to init state */
		void reset(double _sampleRate)
		{
			for(uint32_t i=0; i<numStates; i++)
				state[i] = 0.0;

			sampleRate = _sampleRate;
		}

		/** set filter fc and Q */
		void setParameters(double _fc, double _Q)
		{
			if (fc == _fc && Q == _Q)
				return;

			fc = _fc;
			Q = _Q;

			double theta_c = kTwoPi*fc / sampleRate;
			double BW = fc / Q;
			coeffs[b2] = exp(-2.0*kPi*(BW / sampleRate));
			coeffs[b1] = ((-4.0*coeffs[b2]) / (1.0 + coeffs[b2]))*cos(theta_c);
			coeffs[a0] = 1.0 - pow(coeffs[b2], 0.5);
			coeffs[a2] = -coeffs[a0];
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn, double loss = 1.0)
		{
			// --- direct
			double yn = coeffs[a0]*xn + coeffs[a2]*state[xz2] - coeffs[b1]*state[yz1] -coeffs[b2]*state[yz2];
			state[xz2] = state[xz1];
			state[xz1] = xn;
			state[yz2] = state[yz1];
			state[yz1] = yn * loss;
			return yn;
		}

	protected:
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 };
		enum { a0, a2, b1, b2 };
		double coeffs[4] = { 0.0, 0.0, 0.0, 0.0 };
		double fc = 440.0;
		double Q = 0.5;
		double sampleRate = 1.0;
	};

	/**
	\class LowShelfFilter
	\ingroup FX-FilteringObjects
	\brief
	Implementation of a low shelving filter
	- taken from the AudioFilter object in the FX book below
	- can set the fc and Q values
	- uses biquad filter

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class LowShelfFilter
	{
	public:
		LowShelfFilter(void) {}	/* C-TOR */
		~LowShelfFilter(void) {}	/* D-TOR */

	public:
		/** reset to init state */
		void reset(double _sampleRate)
		{
			for (uint32_t i = 0; i<numStates; i++)
				state[i] = 0.0;

			sampleRate = _sampleRate;
		}

		/** set shelving frequency and boost/cut in dB */
		void setParameters(double shelfFreq, double boostCut_dB)
		{
			double theta_c = kTwoPi*shelfFreq / sampleRate;
			double mu = pow(10.0, boostCut_dB / 20.0);

			double beta = 4.0 / (1.0 + mu);
			double delta = beta*tan(theta_c / 2.0);
			double gamma = (1.0 - delta) / (1.0 + delta);

			// --- update coeffs
			coeffs[a0] = (1.0 - gamma) / 2.0;
			coeffs[a1] = (1.0 - gamma) / 2.0;
			coeffs[a2] = 0.0;
			coeffs[b1] = -gamma;
			coeffs[b2] = 0.0;

			coeffs[c0] = mu - 1.0;
			coeffs[d0] = 1.0;
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn)
		{
			// --- transposed canonical
			double yn = coeffs[a0] * xn + state[xz1];

			// --- shuffle/update
			state[xz1] = coeffs[a1] * xn - coeffs[b1] * yn + state[xz2];
			state[xz2] = coeffs[a2] * xn - coeffs[b2] * yn;
			return xn*coeffs[d0] + yn*coeffs[c0];
		}

	protected:
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 };
		enum { a0, a1, a2, b1, b2, c0, d0 };
		double coeffs[7] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		double fc = 440.0;
		double boostCut_dB = 0.0;
		double sampleRate = 1.0;
	};


	/**
	\class HighShelfFilter
	\ingroup FX-FilteringObjects
	\brief
	Implementation of a high shelving filter
	- taken from the AudioFilter object in the FX book below
	- can set the fc and Q values
	- uses biquad filter

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class HighShelfFilter
	{
	public:
		HighShelfFilter(void) {}	/* C-TOR */
		~HighShelfFilter(void) {}	/* D-TOR */

	public:
		/** reset to init state */
		void reset(double _sampleRate)
		{
			for (uint32_t i = 0; i<numStates; i++)
				state[i] = 0.0;

			sampleRate = _sampleRate;
		}

		/** set shelving frequency and boost/cut in dB */
		void setParameters(double shelfFreq, double boostCut_dB)
		{
			double theta_c = kTwoPi*shelfFreq / sampleRate;
			double mu = pow(10.0, boostCut_dB / 20.0);

			double beta = (1.0 + mu) / 4.0;
			double delta = beta*tan(theta_c / 2.0);
			double gamma = (1.0 - delta) / (1.0 + delta);

			coeffs[a0] = (1.0 + gamma) / 2.0;
			coeffs[a1] = -coeffs[a0];
			coeffs[a2] = 0.0;
			coeffs[b1] = -gamma;
			coeffs[b2] = 0.0;

			coeffs[c0] = mu - 1.0;
			coeffs[d0] = 1.0;
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn)
		{
			// --- transposed canonical
			double yn = coeffs[a0] * xn + state[xz1];

			// --- shuffle/update
			state[xz1] = coeffs[a1] * xn - coeffs[b1] * yn + state[xz2];
			state[xz2] = coeffs[a2] * xn - coeffs[b2] * yn;
			return xn*coeffs[d0] + yn*coeffs[c0];
		}

	protected:
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 };
		enum { a0, a1, a2, b1, b2, c0, d0 };
		double coeffs[7] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		double fc = 440.0;
		double boostCut_dB = 0.0;
		double sampleRate = 1.0;
	};

	/**
	\class ParametricFilter
	\ingroup FX-FilteringObjects
	\brief
	Implementation of a constant-Q parametric EQ filter
	- taken from the AudioFilter object in the FX book below
	- can set the fc and Q values
	- uses biquad filter

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class ParametricFilter
	{
	public:
		ParametricFilter(void) {}	/* C-TOR */
		~ParametricFilter(void) {}	/* D-TOR */

	public:
		/** reset to init state */
		void reset(double _sampleRate)
		{
			for (uint32_t i = 0; i<numStates; i++)
				state[i] = 0.0;

			sampleRate = _sampleRate;
		}

		/** set filter fc, Q and boost/cut values */
		void setParameters(double _fc, double _Q, double _boostCut_dB)
		{
			if (fc == _fc && Q == _Q && boostCut_dB == _boostCut_dB)
				return;
			fc = _fc;
			Q = _Q;
			boostCut_dB = _boostCut_dB;

			// --- see book for formulae
			double theta_c = kTwoPi*fc / sampleRate;
			double mu = pow(10.0, boostCut_dB / 20.0);

			// --- clamp to 0.95 pi/2 (you can experiment with this)
			double tanArg = theta_c / (2.0 * Q);
			if (tanArg >= 0.95*kPi / 2.0) tanArg = 0.95*kPi / 2.0;

			// --- intermediate variables (you can condense this if you wish)
			double zeta = 4.0 / (1.0 + mu);
			double betaNumerator = 1.0 - zeta*tan(tanArg);
			double betaDenominator = 1.0 + zeta*tan(tanArg);

			double beta = 0.5*(betaNumerator / betaDenominator);
			double gamma = (0.5 + beta)*(cos(theta_c));
			double alpha = (0.5 - beta);

			// --- update coeffs
			coeffs[a0] = alpha;
			coeffs[a1] = 0.0;
			coeffs[a2] = -alpha;
			coeffs[b1] = -2.0*gamma;
			coeffs[b2] = 2.0*beta;

			coeffs[c0] = mu - 1.0;
			coeffs[d0] = 1.0;
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output */
		double processAudioSample(double xn)
		{
			// --- transposed canonical
			double yn = coeffs[a0] * xn + state[xz1];

			// --- shuffle/update
			state[xz1] = coeffs[a1] * xn - coeffs[b1] * yn + state[xz2];
			state[xz2] = coeffs[a2] * xn - coeffs[b2] * yn;
			return xn*coeffs[d0] + yn*coeffs[c0];
		}

	protected:
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 };
		enum { a0, a1, a2, b1, b2, c0, d0 };
		double coeffs[7] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		double fc = 0.0;
		double Q = 0.0;
		double boostCut_dB = 0.0;
		double sampleRate = 1.0;
	};


	/**
	\class LP1PFilter
	\ingroup FX-FilteringObjects
	\brief
	Implementation of a one-pole LPF
	- taken from the AudioFilter object in the FX book below
	- can set the fc and Q values
	- uses biquad filter

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class LP1PFilter
	{
	public:
		LP1PFilter(void) {}	/* C-TOR */
		~LP1PFilter(void) {}	/* D-TOR */

	public:
		/** reset to init state */
		void reset(double _sampleRate)
		{
			clear();
			sampleRate = _sampleRate;
		}

		/** clear state variables */
		void clear()
		{
			for (uint32_t i = 0; i<numStates; i++)
				state[i] = 0.0;
		}

		/** set the filter fc */
		void setParameters(double _fc)
		{
			if (fc == _fc)
				return;

			fc = _fc;

			// --- see book for formulae
			double theta_c = 2.0*kPi*fc / sampleRate;
			double gamma = 2.0 - cos(theta_c);

			double filter_b1 = pow((gamma*gamma - 1.0), 0.5) - gamma;
			double filter_a0 = 1.0 + filter_b1;

			// --- update coeffs
			coeffs[a0] = filter_a0;
			coeffs[a1] = 0.0;
			coeffs[a2] = 0.0;
			coeffs[b1] = filter_b1;
			coeffs[b2] = 0.0;
		}

		/**
		\brief
		run the filter

		\param xn the input sample
		\return the filtered output
		*/
		double processAudioSample(double xn)
		{
			// --- transposed canonical
			double yn = coeffs[a0] * xn + state[xz1];

			// --- shuffle/update
			state[xz1] = coeffs[a1] * xn - coeffs[b1] * yn + state[xz2];
			state[xz2] = coeffs[a2] * xn - coeffs[b2] * yn;
			return yn;
		}

	protected:
		enum { xz1, xz2, yz1, yz2, numStates };
		double state[4] = { 0.0, 0.0, 0.0, 0.0 };
		enum { a0, a1, a2, b1, b2 };
		double coeffs[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
		double fc = 5.0;
		double Q = 0.5;
		double sampleRate = 1.0;
	};

	/** \ingroup Constants-Enums
	enumeration for pluck filter; which is reall three filters in one, with multiple output points
	*/
	enum class PluckFilterType {kPluck, kPluckAndBridge, kPickup, kPluckAndPickup, kBridge, kPluckPickupBridge};


	/**
	\class PluckPosFilter
	\ingroup FX-FilteringObjects
	\brief
	Comnination of three filters in one; note that the figure in the book does not show the 
	variety of connection combinations and filter bypassing possible, nor the multiple
	output points
	- filters all come from the FX book below
	- Pluck Position: a comb filter
	- Bridge Filter: a lossy integrator with very low fc
	- Pickup Filter: a 2nd order LPF whose parameters are adjusted differently for guitar vs. bass guitar pickups

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	class PluckPosFilter
	{
	public:
		PluckPosFilter(void) {}	/* C-TOR */
		~PluckPosFilter(void) {}	/* D-TOR */

	public:
		/** flush filter */
		void clear()
		{
			combDelay.clear();
			bridgeIntegrator.clear();
		}

		/**
		\brief
		reset the delay, calculate a new length based on sample rate and minimum pitch
		- note how pickup and bridge filters are hard-coded for init

		\param _sampleRate fs
		\param minimumPitch pitch in Hz of the lowest note that could fit into this delay line
		*/
		void reset(double _sampleRate, double minimumPitch = MIDI_NOTE_0_FREQ) // 8.176 = MIDI note 0
		{
			// ---
			combDelay.reset(_sampleRate, minimumPitch);
			combDelay.clear();

			pickupFilter.reset(_sampleRate);
			pickupFilter.setParameters(2500.0, 1.5); // guitar pickup settings

			bridgeIntegrator.reset(_sampleRate);
			bridgeIntegrator.setParameters(20.0); // lower than the lowest note to synthesize which is note 0
		}

		/**
		\brief
		set comb delay time - this will be based on virtual plucking position on string

		\param _delaySamples delay time in samples
		*/
		void setDelayInSamples(double _delaySamples)
		{
			// --- the (-1) is needed here because of the way the circular buffer counts samples for read/write
			combDelay.setDelayInSamples(_delaySamples - 1);
		}

		/**
		\brief
		run the string of filters
		- the type argument specifies the ordering and output location

		\param xn the input value
		\param type the type of combination of filters to run

		*/
		double processAudioSample(double xn, PluckFilterType type)
		{
			if (type == PluckFilterType::kBridge)
				return 12.0*bridgeIntegrator.processAudioSample(xn);

			if (type == PluckFilterType::kPickup)
				return pickupFilter.processAudioSample(xn);

			// --- pluck position
			double yn = combDelay.readDelay();
			combDelay.writeDelay(xn);

			// --- output pluck
			double pluck = 0.5*(xn - yn);
			if (type == PluckFilterType::kPluck)
				return pluck;

			// --- pluck and pickup
			if (type == PluckFilterType::kPluckAndPickup)
				return pickupFilter.processAudioSample(pluck);

			// --- pluck and bridge
			if (type == PluckFilterType::kPluckAndBridge)
				return 12.0*bridgeIntegrator.processAudioSample(pluck);

			if (type == PluckFilterType::kPluckPickupBridge)
			{
				double pu = 2.0*pickupFilter.processAudioSample(pluck);
				return 12.0*bridgeIntegrator.processAudioSample(pu);
			}


			return xn; // should never get here
		}

	protected:
		DelayLine combDelay; ///< for pluck position
		LP1PFilter bridgeIntegrator; ///< for bridge LPF
		LP2Filter pickupFilter; /// for simulating an electric guitar pickup
	};

}// namespace

  /** \defgroup SynthModules
  \brief
  These are the modular building blocks in the SynthLab project. All of these except the DCA
  include four member cores (it is easy to increase the maximum core count).

  Included Modules:
  - FMOperator
  - WTOscillator
  - VAOscillator
  - PCMOscillator
  - KSOscillator
  - NoiseOscillator
  - SynthFilter
  - LFO
  - DCA
  - EnvelopeGenerator
  - AudioDelay
  - WaveSequencer


  **/

  /** \defgroup ModuleCores
  \brief
  These are the worker objects that do all of the processing and synthesis computations. Each
  core provides a variation on the SynthModule's functionality.

  Dynamic Architecture:
  - Preferred manner of operation; requires GUI that can dynamically change the contents of
  list controls and text labels
  - The cores expose a set of 16 ModuleString values and 4 ModKnobLabel values that the GUI
  may optionally use to allow the user to select cores and have the waveform and custom
  knob control text lablels change on the fly as the cores change. This is the most dynamic
  version of the software with the most flexibility.
  - Any core may be used in any module.
  - Each core's ModuleStrings constitute a "bank" and so your oscillators may expose banks of waveforms,
  the filters expose banks of filter types, and so on.
  - SynthLab-WS shows how to combine banks together into larger sets of lists to expose for the user

  Fixed Architecture:
  - Simplest way to create a complete syth.
  - The cores, waveforms and mod knob label values are pre-programmed in a fixed setup.
  - There is only one pre-se;lected core per module; In a four-oscillator synth, each oscillator
  could expose one different core, or all cold expose the same cores depending on your synth design.

  The synth core modules and architecture are written so that the resulting synth objects do not know
  whether they are part of a fixed or dynamic architecture. There is no difference in the code on the
  synth object side. The only coding differences are in the plugin framework that is creating the
  plugin objects.

  Included CoreModules:
  - AnalogEGCore
  - BQFilterCore
  - ClassicWTCore
  - DrumWTCore
  - DXEGCore
  - FMLFOCore
  - FourierWTCore
  - KSOCore
  - LinearEGCore
  - LFOCore
  - MellotronCore
  - MorphWTCore
  - LegacyPCMCore
  - SFXWTCore
  - VAFilterCore
  - WaveSliceCore


  \brief

  **/


  /** \defgroup SynthObjects
  \brief
  Very specilized and very small objects that handle the most fundamental implementations in the synth.
  This includes timer, ramp modulator, cross-fade-holder objects and many more.
  - highly specialized, difficult to shoehorn into a common interface
  - expose a small number of simple functions


  \brief

  **/

  /** \defgroup DynamicModuleObjects
  \brief
  These objects are used for loading and unloading dynamic modules (DM) which are API-agnostic DLLs (Windows)
  or dylibs (MacOS). These use longstanding and very basic functions for DLL management; these
  functions are extremely well documented.
  - not technically part of SynthLab and not needed to use the SynthLab objects
  - used in the ASPiK PluginCore (the audio processor object in ASPiK) to load
  modules at run-time
  - involve DLL and dylib functions


  **/

  /** \defgroup DynamicStringObjects
  \brief
  Dynamic string management for custom GUI controls that allow the string lists to be changed
  or the text labels to be altered; this is really just a big database of GUI controls
  and save interfaces to have them update in a thread-safe manner
  - SynthLab's Module/Core paradigm allows the loading of different ModuleCores, which expose
  customized strings. For example, the oscillator cores expose a set of strings that are the
  waveform or sample names. And each core also exposes a set of mod knob labels for its
  specialized functions. This is detailed in the synth book and website.
  - the number of strings is fixed and won't change, however some string slots may be empty
  signified with "-----"
  - this will NOT interfere with automation because of the constant size of the parameter lists
  - core names: there are always four (4) core names available, though they may not all be populated
  which is OK
  - the four cores names are loaded into a GUI control - a list box or a combo box; each time the user
  selects a new core, ANOTHER GUI control is updated with the module strings (16 of them) and
  the four (4) Mod Knob labels are changed to reflect the new core
  - you will need to know how to deal with dynamic string loading in your plugin framework
  - these objects are designed to work with ASPiK plugins and are dependent on one interface, ICustomView
  which is used to re-populate the GUI controls with new strings
  - core names: there are 4 cores per module
  - moduleStrings[16]: these are the 16 strings exposed in the ModuleCore constructors
  - modKnobLabels[4]: these are the labels that replace the A, B, C and D ModKnob placeholders


  **/

  /** \defgroup SynthStructures
  \brief


  \brief

  **/

  /** \defgroup DynamicStringStructures
  \brief


  \brief

  **/

  /** \defgroup SynthInterfaces
  \brief
  These are pure abstrace interfaces to be used as base classes for the synth objects. These allow
  container objects to hold (shared) simple interface pointers, or the actual object pointers; all
  object interfacing is done through the common objec model interfaces here.

  - Most of these are pure abstract with functions only, while a few are more complex.
  - Some of them require special data structures or enumerations; when this happens these items will be
  defined above the objects, rather than in the gereralized synthstructures.h and synthconstants.h files.
  - This is just to make it easier to find them.

  **/





  /** \defgroup SynthEngine
  \brief
  - The synth engine implements the entire synth architecture and SynthEngine is the
  single C++ object you need to add to your plugin framework’s processing object.

  - In the example plugins designed with ASPiK, this is the PluginCore C++ object,
  for JUCE it is the AudioProcessor object.

  - Each of the synth projects is packaged in a single SynthEngine object, which is
  implemented in synthengine.h and sythengine.cpp and located in a directory with
  the other supporting object files.

  - These objects are framework agnostic meaning that they are pure C++,
  have no bindings to any plugin framework and do not require additional libraries
  beyond the built-in standard template libraries included in your compiler.


  - The engine performs three tasks during the synth’s operation:
  1. initialization,
  2. applying GUI control changes, and
  3. rendering the synthesized audio.

  - I designed the SynthEngine object to expose simple functions that service these
  operational functions, with an interface that is not dependent on any plugin framework.


  - Your plugin framework’s processing object will create and use the engine object –
  it is the sole C++ object and interface that you need to wire-into your plugin.

  - After instantiation, the plugin will call five methods on the engine for the
  three engine functions as seen from the plugin/DAW side:
  1. Initialization: the plugin calls the engine’s reset and initialize functions

  2. Set GUI Parameters: the plugin calls getParameters for a shared pointer to the
  engine’s GUI connected parameters and setParameters to instruct the engine to update
  its states, causing a trickle-down transfer of parameters using shared pointers and without data copying

  3. Rendering Audio: the plugin calls the engine’s render function, passing it audio
  buffers to fill one a block by block basis

  **/


  /** \defgroup SynthVoice
  \brief
  The SynthLab SynthVoice object is responsible for rendering note events and there is one
  voice object per note of polyphony in the synths. As with the engine, the SynthVoice object has no base class,
  but is setup to be a base class with the virtual functions and destructor, so feel free to
  subclass your own when you are ready.

  - The term “voice” has several meanings in synth lingo, but here it also includes the synth type.
  - The voice architecture is covered in Section 1.5 of the Synth book.

  The voice performs three tasks during the synth’s operation: initialization, responding to MIDI note-on
  and note-off messages, and controlling the audio signal flow through a set of member objects called modules.

  - The voice object’s central responsibility is maintaining this set of SynthModule objects that make up the
  synthesizer components such as oscillators and filters. I designed the SynthVoice object to expose
  simple functions that service these three areas of operation.
  1.	Initialization: the voice calls the module’s reset function
  2.	Note-on and Note-off: the voice calls the doNoteOn and doNoteOff methods on its set of modules
  3.	Controlling Audio Signal Flow: the voice calls the module’s update and render functions during each block processing cycle, and delivers the rendered audio back to the engine

  - The voice object also processes incoming MIDI data for note-on and note-off events, which it uses to
  control its set of modules.


  **/

  /** \defgroup Constants-Enums


  **/



  /** \defgroup FX-FilteringObjects
  \brief
  These are all very small objects that were culled from Will Pirkle's book
  Designing Audio Effects Plugins in C++ 2nd Ed. They are taken either directly verbatim,
  or with slight name changes from the fxobjects.h and fxobjects.cpp files that you can
  download that accompany the book.
  - these are heavily detailed in the book above and at http://aspikplugins.com/sdkdocs/html/index.html
  and so there is only minimal documentation here
  - please see the link above for more information on these objects

  **/

  /** \defgroup SynthFunctions

  \brief

  **/

  /** \defgroup MIDIFunctions

  \brief

  **/

  /** \defgroup ModulationFunctions

  \brief

  **/

#endif /* defined(__synthDefs_h__) */
