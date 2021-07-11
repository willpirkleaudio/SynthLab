#pragma once

#include <array>

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthlabparams.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	#define enumToInt(ENUM) static_cast<int>(ENUM)

	inline double semitonesBetweenFreqs(double startFrequency, double endFrequency)
	{
		return log2(endFrequency / startFrequency)*12.0;
	}

	//@{
	/**
	\ingroup Constants-Enums
	General purpose audio channel number constants
	*/
	const uint32_t MONO_INPUT = 1;
	const uint32_t MONO_OUTPUT = 1;
	const uint32_t STEREO_INPUTS = 2;
	const uint32_t STEREO_OUTPUTS = 2;
	//@}

	/**
	\struct OscParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for general purpose oscillators or those that don't fit into one of the other
	categories
	- waveIndex is the index of the waveform string in the GUI selection control
	- modKnobValue[4] are the four mod knob values, all on the range [0, 1]

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct OscParameters
	{
		OscParameters() {}

		// --- bank index
		uint32_t waveIndex = 0;				///< index of waveform, usually linked to GUI control

		// --- tuning
		double octaveDetune = 0.0;			///<  +/- 4 octaves
		double coarseDetune = 0.0;			///<  +/-12 semitones
		double fineDetune = 0.0;			///<  +/-50 cents
		double unisonDetuneCents = 0.0;		///<  fine tune for unison
		double oscSpecificDetune = 0.0;		///<  +/-12 semitones

		// --- mod and output
		double outputAmplitude_dB = 0.0;	///<  dB
		double oscillatorShape = 0.0;		///<  [-1, +1]
		double hardSyncRatio = 1.00;		///<  [1, +???]
		double panValue = 0.00;				///<  [-1, +1]
		double phaseModIndex = 1.0;			///<  [1, 4]
		double shape = 0.0;					///<  [-1, +1]
		double modKnobValue[4] = { 0.5, 0.0, 0.0, 0.0 }; ///< mod knobs
		uint32_t moduleIndex = 0;			///< module indentifier
		bool forceLoop = false;				///< force the wavetable/sample to loop at extremes
	};


	//@{
	/**
	\ingroup Constants-Enums
	Oscillator constant; modify these to change limits and other oscillator behavior.
	*/
	// --- mod knobs, other knob controls
	const double OSC_FMIN = MIDI_NOTE_0_FREQ;
	const double OSC_FMAX = 20480.0;
	const uint32_t OSC_INPUTS = 1; // FM audio inputs
	const uint32_t OSC_OUTPUTS = 2;// dual-mono output

	// --- modulation min/max
	const double WT_OSC_MIN = MIDI_NOTE_0_FREQ;
	const double WT_OSC_MAX = 20480.0;
	const uint32_t WT_OSC_INPUTS = 1; // FM audio inputs
	const uint32_t WT_OSC_OUTPUTS = 2;// dual-mono output
	//@}

	/**
	\struct WTOscParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for wavetable oscillators (note that several SynthLab oscillators are actually
	wavetable at the core). Notable members:
	- waveIndex is the index of the waveform string in the GUI selection control
	- modKnobValue[4] are the four mod knob values, all on the range [0, 1]
	
	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct WTOscParameters
	{
		WTOscParameters() {}

		// --- bank index
		uint32_t waveIndex = 0;				///< index of waveform, usually linked to GUI control

		// --- tuning
		double octaveDetune = 0.0;			///<  +/- 4 octaves
		double coarseDetune = 0.0;			///<  +/-12 semitones
		double fineDetune = 0.0;			///<  +/-50 cents
		double unisonDetuneCents = 0.0;		///<  fine tune for unison
		double oscSpecificDetune = 0.0;		///<  +/-12 semitones

		// --- mod and output
		double outputAmplitude_dB = 0.0;	///<  dB
		double oscillatorShape = 0.0;		///<  [-1, +1]
		double hardSyncRatio = 1.00;		///<  [1, +???]
		double panValue = 0.00;				///<  [-1, +1]
		double phaseModIndex = 1.0;			///<  [1, 4]
		double modKnobValue[4] = { 0.5, 0.0, 0.0, 0.0 }; ///< mod knobs
		uint32_t moduleIndex = 0;			///< module indentifier
		bool forceLoop = false;				///< force the wavetable to loop at extremes
	};

	
	/**
	\struct WSOscParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the wave sequencer module. Notable members:
	- doubleOscillator true if the synth is using two WSOscillators for detuning and 
	intervals
	- all others are in sets of 8, one for each step in the wave sequencer

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct WSOscParameters
	{
		WSOscParameters() {}

		// --- tuning
		double detuneSemis[8] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };					// --- fine tune for unison
		double detuneCents[8] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };					// --- fine tune for unison
		double oscillatorShape[8] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };				// [-1, +1]
		double hardSyncRatio[8] = { 1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00 };			// [1, +4]
		double morphIntensity[8] = { 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00 };	// [0, +1]
		double panValue[8] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };						// [-1, +1]

		bool doubleOscillator = true;// false;
		int32_t soloWaveWSIndex = -1;
	};


	//@{
	/**
	\ingroup Constants-Enums
	Virtual analog oscillator constant; modify these to change limits and other oscillator behavior.
	*/
	// --- mod knobs, other knob controls
	const double VA_OSC_MIN = MIDI_NOTE_0_FREQ;
	const double VA_OSC_MAX = 20480.0;
	const uint32_t VA_OSC_INPUTS = 0; // no audio inputs
	const uint32_t VA_OSC_OUTPUTS = 2;// dual-mono output
	const double VA_MIN_PW = 0.5;
	const double VA_MAX_PW = 0.95;
	const double PW_MOD_RANGE = (VA_MAX_PW - VA_MIN_PW);
	const double HALF_PW_MOD_RANGE = PW_MOD_RANGE / 2.0;

	// --- VA Oscillator Waveforms
	enum class VAWaveform { kSawAndSquare, kSawtooth, kSquare };
	//@}

	/**
	\struct VA1Coeffs
	\ingroup SynthStructures
	\brief
	Tiny structure to hold 1st order VA filter coefficients, to make it easier to share
	them across sync-tuned filters.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct VA1Coeffs
	{
		// --- filter coefficients
		double alpha = 0.0;			///< alpha is (wcT/2)
		double beta = 1.0;			///< beta value, not used
	};

	/**
	\struct VAOscParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the VAOscillator and its ModuleCores. Notable members:
	- doubleOscillator true if the synth is using two WSOscillators for detuning and
	intervals
	- all others are in sets of 8, one for each step in the wave sequencer

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct VAOscParameters
	{
		VAOscParameters() {}

		// --- two waveforms
		uint32_t waveIndex = enumToInt(VAWaveform::kSawAndSquare);///< waveform index in the GUI

		// ---  detuning values
		double octaveDetune = 0.0;			///< 1 = up one octave, -1 = down one octave
		double coarseDetune = 0.0;			///< 1 = up one octave, -1 = down one octave
		double fineDetune = 0.0;			///< 1 = up one half-step, -1 = down one half-step

											///< --- this is a root-detuner for unison mode (could combine with detuneCents but would become tangled)
		double unisonDetune = 0.0;			///< 1 = up one cent, -1 = down one cent

		double pulseWidth_Pct = 50.0;		///< sqr wave only
		double outputAmplitude_dB = 0.0;	///< dB

		double oscillatorShape = 0.0;		///< [-1, +1]
		double hardSyncRatio = 1.00;		///< [1, +4]
		double panValue = 0.00;				///< [-1, +1]
		double phaseModIndex = 1.0;			///< [1, 4]
		double waveformMix = 0.5;			///< [1, +???]

		double modKnobValue[4] = { 0.5, 0.0, 0.0, 0.0 };///< mod knobs
		uint32_t moduleIndex = 0;			///< module identifier
	};


	//@{
	/**
	\ingroup Constants-Enums
	Constants for the synth PCM sample oscillators and their cores
	*/
	const double PCM_OSC_MIN = MIDI_NOTE_0_FREQ;
	const double PCM_OSC_MAX = 20480.0;
	const uint32_t SMPL_OSC_INPUTS = 0;
	const uint32_t SMPL_OSC_OUTPUTS = 2;// dual-mono or stereo output
	//@}

	/**
	\struct PCMOscParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the PCM sample oscillator and its cores. Notable members:
	- waveIndex is the index from the GUI selection control for the waveform string; usually the same
	as the folder of WAV files used to generate it
	- modKnobValue[4] are the four mod knob values

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct PCMOscParameters
	{
	public:
		// ---  index within bank
		uint32_t waveIndex = 0;				///< the waveform string is usually the patch or WAV folder name

											// ---  detuning values
		double octaveDetune = 0.0;			///< 1 = up one octave, -1 = down one octave
		double coarseDetune = 0.0;			///< 1 = up one octave, -1 = down one octave
		double fineDetune = 0.0;			///< 1 = up one half-step, -1 = down one half-step

											// --- this is a root-detuner for unison mode (could combine with detuneCents but would become tangled)
		double unisonDetune = 0.0;			///< 1 = up one cent, -1 = down one cent
		double outputAmplitude_dB = 0.0;	///< in dB

		double oscillatorShape = 0.0;		///< [-1, +1]
		double hardSyncRatio = 1.00;		///< [1, 4]
		double phaseModIndex = 1.0;			///< [1, 4]
		double freqModIndex = 1.0;			///< [1, 4]
		double panValue = 0.0;				///< [-1, +1]
		double modKnobValue[4] = { 0.5, 0.0, 0.0, 0.0 };	///< mod knobs
		uint32_t moduleIndex = 0;	///< module identifier
	};

	//@{
	/**
	\ingroup Constants-Enums
	Constants for the synth KS oscillators and their cores
	*/
	const double KS_OSC_MIN = MIDI_NOTE_0_FREQ;
	const double KS_OSC_MAX = 20480.0;
	const uint32_t KS_OSC_INPUTS = 0;
	const uint32_t KS_OSC_OUTPUTS = 2;// dual-mono or stereo output
	const double MAX_KSO_ATTACK_MSEC = 500.0;
	const double MAX_KSO_HOLD_MSEC = 2000.0;
	const double MAX_KSO_RELEASE_MSEC = 5000.0;
	//@}

	/**
	\struct KSOscParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the Karplus-Strong oscillator and its cores. Notable members:
	- algorithmIndex is the index from the GUI selection control for the plucked string algorithm
	- pluckPosition is the fraction of a string for the pluck; at the highest setting the pluck
	is in the center of the string; as this value is reduced the pluck position moves closer and closer
	to the bridge. At the minimum position, you are 1/12 the string length from the bridge.
	- includes EG contros for the exciter shaper
	- modKnobValue[4] are the four mod knob values

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct KSOscParameters
	{
		KSOscParameters() {}

		// ---  index within bank
		uint32_t algorithmIndex = 0;		///< GUI index for KS algorithm

		double attackTime_mSec = 0.0;		///< exciter EG
		double holdTime_mSec = 0.0;			///< exciter EG
		double releaseTime_mSec = 0.0;		///< exciter EG

											// ---  detuning values
		double octaveDetune = 0.0;			///< 1 = up one octave, -1 = down one octave
		double coarseDetune = 0.0;			///< 1 = up one octave, -1 = down one octave
		double fineDetune = 0.0;			///< 1 = up one half-step, -1 = down one half-step
		double unisonDetune = 0.0;			///< 1 = up one cent, -1 = down one cent
		double outputAmplitude_dB = 0.0;	///< dB

		double oscillatorShape = 0.0;		///</ [-1, +1]
		double hardSyncRatio = 1.00;		///< [1, +4]
		double phaseModIndex = 1.0;			///</ [1, +4]
		double freqModIndex = 1.0;			///< [1, +4]
		double panValue = 0.0;				///< [-1, +1]
		double decay = 0.9;					///< [-1, +1]
		uint32_t  pluckPosition = 1;		///< [+1, +12]
		double modKnobValue[4] = { 0.5, 0.0, 0.0, 0.0 }; ///< mod knob values
		uint32_t moduleIndex = 0;	/// module ID
	};

	/**
	\struct ModSource
	\ingroup SynthStructures
	\brief
	Structure that encapsulates the controls for a modulation source as part of the modulation matrix. 
	- this structure only defines an intensity level setting
	- see the ModDestination for the other controls

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct ModSource
	{
		ModSource() {}
		ModSource& operator=(const ModSource& params)
		{
			if (this == &params)
				return *this;

			intensity = params.intensity;

			return *this;
		}

		// --- fast clearing of array
		void clear() {
			intensity = 1.0;
		}

		// --- intensity only 
		double intensity = 1.0; ///< one final intensity knob
	};

	/**
	\struct ModDestination
	\ingroup SynthStructures
	\brief
	Structure that encapsulates the controls for a modulation destination as part of the modulation matrix.
	- enable the destination, which enables the routing
	- set the channel intensity control
	- optionally, can hardwire a routing
	- with a hardwired intensity (not shown to user)

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct ModDestination
	{
		ModDestination() { clear(); }
		ModDestination& operator=(const ModDestination& params)
		{
			if (this == &params)
				return *this;

			memcpy(channelEnable, params.channelEnable, MAX_MODULATION_CHANNELS * sizeof(uint32_t));
			memcpy(channelIntensity, params.channelIntensity, MAX_MODULATION_CHANNELS * sizeof(double));

			memcpy(channelHardwire, params.channelHardwire, MAX_MODULATION_CHANNELS * sizeof(bool));
			memcpy(hardwireIntensity, params.hardwireIntensity, MAX_MODULATION_CHANNELS * sizeof(double));

			intensity = params.intensity;
			defautValue = params.defautValue;

			enableChannelIntensity = params.enableChannelIntensity;
			priorityModulation = params.priorityModulation;

			return *this;
		}

		// --- fast clearing of array
		void clear() {
			memset(channelEnable, 0, MAX_MODULATION_CHANNELS * sizeof(uint32_t));
			memset(channelIntensity, 0, MAX_MODULATION_CHANNELS * sizeof(double));
			memset(channelHardwire, 0, MAX_MODULATION_CHANNELS * sizeof(bool));
			memset(hardwireIntensity, 0, MAX_MODULATION_CHANNELS * sizeof(double));
			intensity = 1.0;
		}

		// --- channel enable and intensity controls
		uint32_t channelEnable[MAX_MODULATION_CHANNELS];	///< channel enable on/off switches
		double channelIntensity[MAX_MODULATION_CHANNELS];	///< channel intensity controls

		bool channelHardwire[MAX_MODULATION_CHANNELS];		///< channel hardwire on/off switches
		double hardwireIntensity[MAX_MODULATION_CHANNELS];	///< hardwire intensity controls	

		// --- use separate intensities for each channel
		bool enableChannelIntensity = false;				///< to enable this mode, in addition to or instead of the source and destination intensity controls

		double intensity = 1.0;				///< one final intensity knob
		double defautValue = 0.0;			///< to allow max down, etc...
		bool priorityModulation = false;	///< for high-priority modulator; not used in SynthLab
	};


	/**
	\struct ModMatrixParameters
	\ingroup SynthStructures
	\brief
	Custom GUI control structure for the modulation matrix. 
	- consists of a set of modulation ROWS (sources) and COLUMNS (destinations)
	- the parameter structure also does some of the work when the mod matrix
	is being prgrammed; this was simpler than having the matrix object perform the chores
	since much of it happens in the GUI update handler in your framework. 

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct ModMatrixParameters
	{
		ModMatrixParameters() {}
		ModMatrixParameters& operator=(const ModMatrixParameters& params)	// need this override for collections to work
		{
			if (this == &params)
				return *this;

			modSourceRows = params.modSourceRows;
			modDestinationColumns = params.modDestinationColumns;

			return *this;
		}

		/** array of modulation sources (rows) */
		std::shared_ptr<std::array<ModSource, kNumberModSources>> modSourceRows = std::make_shared<std::array<ModSource, kNumberModSources>>();
		
		/** array of modulation destinations (columns) */
		std::shared_ptr<std::array<ModDestination, kNumberModDestinations>> modDestinationColumns = std::make_shared<std::array<ModDestination, kNumberModDestinations>>();

		/** 
		\brief
		set source intensity value 
		
		\param destination the index of the source row
		\param intensity the intensity value from [0, 1]
		*/
		void setMM_SourceIntensity(uint32_t destination, double intensity)
		{
			modSourceRows->at(destination).intensity = intensity;
		}

		/**
		\brief
		enable/disable a routing channel

		\param source the channel for the source
		\param destination the index of the destination column
		\param enable true to enable the routing, false to disonnect it
		*/
		void setMM_ChannelEnable(uint32_t source, uint32_t destination, bool enable)
		{
			modDestinationColumns->at(destination).channelEnable[source] = enable;
		}

		/**
		\brief
		set/clear a routing as hardwired

		\param source the channel for the source
		\param destination the index of the destination column
		\param enable true to enable the routing, false to disonnect it
		*/
		void setMM_HardwireEnable(uint32_t source, uint32_t destination, bool enable)
		{
			modDestinationColumns->at(destination).channelHardwire[source] = enable;
		}

		/**
		\brief
		set the channel intensity control that connects a source/destination pair in a routing
		- not used in SythLab project, but gives flexibility if you want to use it

		\param source the channel for the source
		\param destination the index of the destination column
		\param intensity the strength of the routing [0, 1]
		*/
		void setMM_ChannelIntensity(uint32_t source, uint32_t destination, double intensity)
		{
			modDestinationColumns->at(destination).channelIntensity[source] = intensity;
		}

		/**
		\brief
		set the hardwired intensity control that connects a source/destination pair in a routing
		and bypasses the normal enable/intensity switches

		\param source the channel for the source
		\param destination the index of the destination column
		\param intensity the strength of the routing [0, 1]
		*/
		void setMM_DestHardwireIntensity(uint32_t source, uint32_t destination, double intensity)
		{
			modDestinationColumns->at(destination).hardwireIntensity[source] = intensity;
		}

		/**
		\brief
		set the destination (column) intensity control

		\param destination the index of the destination column
		\param intensity the strength of the routing [0, 1]
		*/
		void setMM_DestIntensity(uint32_t destination, double intensity)
		{
			modDestinationColumns->at(destination).intensity = intensity;
		}

		/**
		\brief
		set a defalt value to prevent accidental no-note events

		\param destination the index of the destination column
		\param defaultValue initial value
		*/
		void setMM_DestDefaultValue(uint32_t destination, double defaultValue)
		{
			modDestinationColumns->at(destination).defautValue = defaultValue;
		}

		/**
		\brief
		mark a destination has high-priority
		- high-priority modulations are done on every sample intervale and are fine rather than coarse
		like their block processing countarparts
		- not used in SynthLab

		\param destination the index of the destination column
		\param _priorityModulation true for priority
		*/
		void setMM_DestHighPriority(uint32_t destination, bool _priorityModulation)
		{
			modDestinationColumns->at(destination).priorityModulation = _priorityModulation;
		}

		/**
		\brief
		Helper function to set a source/destination/intensity trio that defines a hard-wired
		routing.

		\param source the index of the channel
		\param destination the index of the destination column
		\param intensity on range [-1.0, +1.0]
		*/
		void setMM_HardwiredRouting(uint32_t source, uint32_t destination, double intensity = 1.0)
		{
			setMM_ChannelEnable(source, destination, true);
			setMM_HardwireEnable(source, destination, true);
			setMM_DestIntensity(destination, intensity);
			setMM_DestHardwireIntensity(source, destination, intensity);
		}
	};

	//@{
	/**
	\ingroup Constants-Enums
	Constants for the noise generator
	*/
	const uint32_t NOISE_OSC_INPUTS = 0; // no audio inputs
	const uint32_t NOISE_OSC_OUTPUTS = 2;// dual-mono output

	// --- Noise Oscillator Waveforms
	enum class NoiseWaveform { kWhiteNoise, kPinkNoise, kGaussWhiteNoise };
	//@}

	/**
	\struct NoiseOscillatorParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the NoiseOscillator object.
	- normally used for GUI controls, but SynthLab uses it internally for built-in 
	noise oscillators. 

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct NoiseOscillatorParameters
	{
		NoiseOscillatorParameters(){}
		NoiseOscillatorParameters& operator=(const NoiseOscillatorParameters& params)
		{
			if (this == &params)
				return *this;

			waveform = params.waveform;
			outputAmplitude_dB = params.outputAmplitude_dB;

			return *this;
		}

		// --- two waveforms
		NoiseWaveform waveform = NoiseWaveform::kWhiteNoise; ///< noise wavform
		double outputAmplitude_dB = 0.0;					 ///< output in dB
	};


	//@{
	/**
	\ingroup Constants-Enums
	Constants for the synth filter objects and cores
	*/
	const double freqModLow = 20.0;
	const double freqModHigh = 18000.0; // -- this is reduced from 20480.0 only for self oscillation at upper frequency range
	const double freqModSemitoneRange = semitonesBetweenFreqs(freqModLow, freqModHigh);
	const uint32_t FILTER_AUDIO_INPUTS = 2;
	const uint32_t FILTER_AUDIO_OUTPUTS = 2;
	enum class FilterModel { kFirstOrder, kSVF, kKorg35, kMoog, kDiode };
	enum { FLT1, FLT2, FLT3, FLT4 };
	const int MOOG_SUBFILTERS = 4;
	const int DIODE_SUBFILTERS = 4;
	const int KORG_SUBFILTERS = 3;
	enum class VAFilterAlgorithm {
		kBypassFilter, kLPF1, kHPF1, kAPF1, kSVF_LP, kSVF_HP, kSVF_BP, kSVF_BS, kKorg35_LP, kKorg35_HP, kMoog_LP1, kMoog_LP2, kMoog_LP3, kMoog_LP4, kDiode_LP4
	};
	enum class BQFilterAlgorithm {
		kBypassFilter, k1PLPF, k1PHPF, kLPF2, kHPF2
	};
	//@}


	/**
	\struct FilterParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the any of the synth filters. This structure is designed to take care
	of both VA and biquad filter parameter requirements. Notable members:
	- filterIndex the selected index from a GUI control that the user toggles
	- modKnobValue[4] the four mod knob values on the range [0, 1]

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct FilterParameters
	{
		// --- use with strongly typed enums
		int32_t filterIndex = 0;	///< filter index in GUI control

		double fc = 1000.0;					///< parameter fc
		double Q = 1.0;						///< parameter Q
		double filterOutputGain_dB = 0.0;	///< parameter output gain in dB
		double filterDrive = 1.0;			///< parameter drive (distortion)
		double bassGainComp = 0.0;			///< 0.0 = no bass compensation, 1.0 = restore all bass 
		bool analogFGN = true;				///< use analog FGN filters; adds to CPU load

		// --- key tracking
		bool enableKeyTrack = false;		///< key track flag
		double keyTrackSemis = 0.0;			///< key tracking ratio in semitones

		// --- Mod Knobs and core support
		double modKnobValue[4] = { 0.5, 0.0, 0.0, 0.0 }; ///< mod knobs
		uint32_t moduleIndex = 0;	///< module identifier
	};


	//@{
	/**
	\ingroup Constants-Enums
	Constants for the synth EG objects and cores
	*/
	enum { kEGNormalOutput, kEGBiasedOutput, kNumEGOutputs }; 
	enum class EGState { kOff, kDelay, kAttack, kHold, kDecay, kSlope, kSustain, kRelease, kShutdown };
	enum class AnalogEGContour { kADSR, kAR };  //  --- indexes match modulecore strings
	enum class DXEGContour { kADSlSR, kADSlR }; //  --- indexes match modulecore strings
	const double MAX_EG_VALUE = 1.0;
	//@}

	/**
	\struct EGParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the any of the synth EGs. This structure is designed to take care
	of both analog EG and the DX EG requirements. Notable members:
	- egContourIndex the selected contour from a GUI control that the user toggles
	- modKnobValue[4] the four mod knob values on the range [0, 1]

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct EGParameters
	{
		EGParameters() {}
		int32_t egContourIndex = 0;		///< iundex from GUI control

		// --- modes
		bool resetToZero = false;				///< reset to zero (see book)
		bool legatoMode = false;				///< legato
		bool velocityToAttackScaling = false;	///< one of two EG modulations
		bool noteNumberToDecayScaling = false;	///< one of two EG modulations

		//--- ADSR times from user
		double attackTime_mSec = 250.0;		///<from GUI control
		double decayTime_mSec = 1000.0;		///<from GUI control
		double slopeTime_mSec = 0.0;		///<from GUI control
		double releaseTime_mSec = 3000.0;	///<from GUI control
		
		// --- for DXEG 
		double startLevel = 0.0;	///<from GUI control
		double endLevel = 0.0;		///<from GUI control
		double decayLevel = 0.707;	///<from GUI control
		double sustainLevel = 0.707;///<from GUI control
		double curvature = 0.0;		///<from GUI control

		// --- mod knobs
		double modKnobValue[4] = { 0.5, 0.0, 0.0, 0.0 }; ///< mod knobs
		uint32_t moduleIndex = 0;   // core
	};


	/**
	\struct ExciterParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the Exciter object for the KS Oscillator and cores
	- the exciter is more or less the oscillator for a plucked string model
	- tbe GUI controls are for the AHR EG that is embedded in the KS Cores

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct ExciterParameters
	{
		ExciterParameters& operator=(const ExciterParameters& data)
		{
			if (this == &data)
				return *this;

			attackTime_mSec = data.attackTime_mSec;
			holdTime_mSec = data.holdTime_mSec;
			releaseTime_mSec = data.releaseTime_mSec;
			return *this;
		}

		double attackTime_mSec = 1000.0;	///< from a GUI control
		double holdTime_mSec = 5000.0;		///< from a GUI control
		double releaseTime_mSec = 2500.0;	///< from a GUI control
	};

	//@{
	/**
	\ingroup Constants-Enums
	Constants for the synth FM Operator object
	*/
	const double FM_OSC_MIN = MIDI_NOTE_0_FREQ;
	const double FM_OSC_MAX = 20480.0;
	const uint32_t FM_OSC_INPUTS = 0; // FM audio inputs
	const uint32_t FM_OSC_OUTPUTS = 2;// dual-mono output
	//@}

	/**
	\struct FMOperatorParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for moving GUI control information to the FMOperator 
	oscillator object and its cores. Notable members:
	- waveIndex is the FM algorithm (8 of them in SynthLab-DX)
	- note that in addition to the standard oscillator GUI controls, there are also a set of controls for the DX EG
	that is built-into each FM Operator

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct FMOperatorParameters
	{
		FMOperatorParameters() {}

		// --- bank index
		uint32_t waveIndex = 0;				///< DX algorithm

		// --- tuning
		double octaveDetune = 0.0;			///< +/- 4 octaves
		double coarseDetune = 0.0;			///< +/-12 semitones
		double fineDetune = 0.0;			///< +/-50 cents
		double unisonDetune = 0.0;			///< fine tune for unison

		// --- mod and output
		double outputAmplitude_dB = 0.0;	///< dB
		double oscillatorShape = 0.0;		///< [-1, +1]
		double panValue = 0.00;				///< [-1, +1]
		double phaseModIndex = 0.0;			///< [0, 4]
		double modKnobValue[4] = { 0.5, 0.0, 0.0, 0.0 };///< mod knobs
		uint32_t moduleIndex = 0;			///< module identifier

		// --- FM Operator only
		EGParameters dxEGParameters;		///< parameters for embdedded EG
		double ratio = 1.0;					///< FM ratio +/- 4 octaves
	};

	//@{
	/**
	\ingroup Constants-Enums
	Constants for the synth DCA object
	*/
	const uint32_t DCA_AUDIO_INPUTS = 2; // stereo/dual mono
	const uint32_t DCA_AUDIO_OUTPUTS = 2;// stereo
	//@}


	/**
	\struct DCAParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the DCA. This is a very simple object
	- the DCA does not implement cores because it is already so simple
	- GUI controls are mostly gain and pan
	- one of the few GUI structures with the modulation intensity values included
	because the DCA is hardwired to the output EG and behaves slightly differently 
	than the other synth objects.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct DCAParameters
	{
		DCAParameters() {}

		// --- individual parameters
		double gainValue_dB = 0.0;		///< for per-voice gain control (not same as master MIDI volume)
		double panValue = 0.0;			///<  [-1, +1] --> [left -> right]
		double ampEGIntensity = 1.0;	///<  [-1, +1]
		double ampModIntensity = 1.0;	///<  [0, +1]
		double panModIntensity = 1.0;	///<  [0, +1] for external GUI control only, defaults to 1 to make mm work
		uint32_t moduleIndex = 0;		///< module identifier
	};

	//@{
	/**
	\ingroup Constants-Enums
	Constants for the synth LFO object
	*/					
	const double LFO_FCMOD_MIN = 0.02;
	const double LFO_FCMOD_MAX = 200.0;
	const double LFO_RANGE = ((LFO_FCMOD_MAX - LFO_FCMOD_MIN));
	const double LFO_HALF_RANGE = LFO_RANGE / 2.0;
	const double MAX_LFO_DELAY_MSEC = 5000.0;
	const double MAX_LFO_FADEIN_MSEC = 5000.0;

	// --- 8 basic waveforms, plus exensions
	enum class LFOWaveform {
		kTriangle, kSin, kRampUp, kRampDown,
		kExpRampUp, kExpRampDn, kExpTriangle, kSquare,
		kRSH, kPluck
	};
	// --- 3 fms
	const uint32_t NUM_FMLFO_OPS = 3;
	enum class FMLFOWaveform { kFM2, kFM3A, kFM3B };
	enum class LFOMode { kSync, kOneShot, kFreeRun };
	// --- array locations in mod output
	enum {
		kLFONormalOutput,
		kLFOInvertedOutput,
		kUnipolarFromMin,
		kUnipolarFromMax,
		kNumLFOOutputs
	};
	//@}

	/**
	\struct LFOParameters
	\ingroup SynthStructures
	\brief
	Custom parameter structure for the LFO object. Notable members:
	- waveformIndex is the index from the GUI selection control
	- modKnobValue[4] are the four mod knob values

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct LFOParameters
	{
		int32_t waveformIndex = 0;		///< selection index from GUI
		int32_t modeIndex = 0;			///< one shot, free run, sync
		double frequency_Hz = 0.5;		///< parameter fc
		double outputAmplitude = 1.0;	///< parameter output gain in dB
		uint32_t quantize = 0;			///< for stepped LFOs
		double modKnobValue[4] = { 0.5, 0.0, 0.0, 0.0 }; ///< mod knobs
		uint32_t moduleIndex = 0;		///< module ID
	};


	//@{
	/**
	\ingroup Constants-Enums
	Constants for the audio delay used as the master FX
	*/
	const uint32_t DELAY_AUDIO_INPUTS = 2;
	const uint32_t DELAY_AUDIO_OUTPUTS = 2;
	//@}

	/**
	\struct AudioDelayParameters
	\ingroup SynthParameters
	\brief
	Custom parameter structure for the AudioDelay object.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	struct AudioDelayParameters
	{
		AudioDelayParameters() {}
		/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
		AudioDelayParameters& operator=(const AudioDelayParameters& params)	// need this override for collections to work
		{
			if (this == &params)
				return *this;

			wetLevel_dB = params.wetLevel_dB;
			dryLevel_dB = params.dryLevel_dB;
			feedback_Pct = params.feedback_Pct;
			leftDelay_mSec = params.leftDelay_mSec;
			rightDelay_mSec = params.rightDelay_mSec;

			return *this;
		}

		// --- individual parameters
		double wetLevel_dB = -3.0;			///< wet output level in dB
		double dryLevel_dB = -3.0;			///< dry output level in dB
		double feedback_Pct = 0.0;			///< feedback as a % value
		double leftDelay_mSec = 2000.0;		///< left delay time
		double rightDelay_mSec = 2000.0;	///< right delay time
	};
} // namespace

