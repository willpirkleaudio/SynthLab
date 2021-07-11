#ifndef __synthVoice_h__
#define __synthVoice_h__

// --- we need these
#include "../../source/synthbase.h"

// --- components common to all SynthLab synths
#include "../../source/modmatrix.h"
#include "../../source/dca.h"
#include "../../source/lfo.h"
#include "../../source/envelopegenerator.h"
#include "../../source/synthfilter.h"

//#define SYNTHLAB_WT 1
//#define SYNTHLAB_VA 1
//#define SYNTHLAB_PCM 1
//#define SYNTHLAB_KS  1
//#define SYNTHLAB_DX  1
//#define SYNTHLAB_WS  1

// --- SL_WT
#ifdef SYNTHLAB_WT
#include "../../source/wtoscillator.h"
#endif

#ifdef SYNTHLAB_VA
#include "../../source/vaoscillator.h"
#endif

#ifdef SYNTHLAB_PCM
#include "../../source/pcmoscillator.h"
#endif

#ifdef SYNTHLAB_KS
#include "../../source/ksoscillator.h"
#endif

#ifdef SYNTHLAB_DX
#include "../../source/fmoperator.h"
#endif

#ifdef SYNTHLAB_WS
#include "../../source/wsoscillator.h"
#include "../../source/sequencer.h"
#endif

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   synthvoice.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	// --- SYNTHLAB STEP 1: Set Default Cores
	//
	//     The "default cores" will setup the initial state of the synth's modules.
	//     If you are using dynamic string loading (below) then these will be overwritten.

	// --- there are 2 built-in LFO cores
	enum class lfoCoreType { standardLFO, fmLFO };

	// --- there are 2 built-in EG cores
	enum class egCoreType { analogEG, dxEG };

	// --- there are 2 built-in filter cores
	enum class filterCoreType { virtualAnalog, biQuad };

	// --- SETUP DEFAULT CORES HERE ------------------------------------ //
	// --- LFOs
	const lfoCoreType lfoCores[NUM_LFO] =
	{
		lfoCoreType::standardLFO,	/* CORE 0 */
		lfoCoreType::fmLFO			/* CORE 1 */
	};

	// --- EGs (individually named)
	const egCoreType ampEGCore = egCoreType::analogEG;
	const egCoreType filterEGCore = egCoreType::analogEG;
	const egCoreType auxEGCore = egCoreType::dxEG;

	// --- FILTERS
	const filterCoreType filterCores[NUM_FILTER] =
	{
		filterCoreType::virtualAnalog,	/* CORE 0 */
		filterCoreType::biQuad			/* CORE 1 */
	};

	// --- VIRTUAL ANALOG FGN (finite gain at Nyquist)
	//     This option (see book) requires more CPU, VA filters only.
	//     Set this to FALSE for normal bilinear transform opertion
	//     and less CPU usage
	const bool useAnalogFGN = false;

	// --- there are 4 wavetable oscillators to choose from
	enum class wtCoreType { classicWT, morphingWT, soundFXWT, drumWT };

	// --- wavetable cores (SynthLab-WT only)
	const wtCoreType wtCores[NUM_OSC] =
	{
		wtCoreType::classicWT,	/* CORE 0 */
		wtCoreType::classicWT,	/* CORE 1 */
		wtCoreType::morphingWT,	/* CORE 2 */
		wtCoreType::soundFXWT	/* CORE 3 */
	};

	// --- there are 4 wavetable oscillators to choose from
	enum class pcmCoreType { legacyPCM, mellotronPCM, waveslicePCM };

	// --- wavetable cores (SynthLab-PCM only)
	const pcmCoreType pcmCores[NUM_OSC] =
	{
		pcmCoreType::legacyPCM,		/* CORE 0 */
		pcmCoreType::legacyPCM,		/* CORE 1 */
		pcmCoreType::mellotronPCM,	/* CORE 2 */
		pcmCoreType::waveslicePCM	/* CORE 3 */
	};

	// --- SYNTHLAB STEP 2: Choose your GUI paradigm
	/*
	// -------------------------- //
	//	*** Fixed GUI Strings *** //
	// -------------------------- //
	//
	If you are using the simplest implementation with fixed GUI string lists,
	then these will be hard-coded and you will NOT provide a GUI interface
	for selecting a core for any of the modules.

	- you hardware the GUI control strings when you create the synth GUI:
		LFO Waveforms
		Oscillator Waveforms
		Filter Types
		EG Contours
		Mod Knob Labels (A, B, C, D)

		--------------------------
		STRING LISTS FOR YOUR GUI
		--------------------------


	// --------------------------- //
	// *** Dynamic GUI Strings *** //
	// --------------------------- //
	//
	This is the advanced mode of operation

	(1) You provide a GUI control for the module cores for each module. This control
		has a maximum string count of 4 and you set it up with 4 dummy strings at the start.

		When the GUI is opening, you dynamically populate these controls with the
		core name strings.

		Use the synthVoice->getModuleCoreNames( ) function to retrieve a vector full
		of these strings and use that to populate the control, overwriting the dummy
		variables you setup. Note that this happens at run-time when the GUI is opened.

		std::vector<std::string> SynthVoice::getModuleCoreNames(uint32_t moduleType)

		The moduleType is an unsigned int, and you can find the list in the synthconstants.h file.

		To get a list of LFO core names, you would write:

		// --- vector to fill
		std::vector<std::string> coreStrings;

		// --- get LFO core names
		coreStrings = synthVoice->getModuleCoreNames(LFO_MODULE);


	(2) You also provide the core string list control that will contain the
		strings specific to that module. For oscillators, this control will hold
		waveform name strings. These GUI controls have a maximum string count of 16
		and you setup the GUI Control with 16 dummy strings as placeholders.

		When the user selects a core from the core-list, you dynamically populate
		these controls with these core-specific strings. For an oscillator module,
		selecting a new core will change the strings in the GUI control.

		It is important that you manually (in code) select the first core and then
		populate its liston startup so that the initial state is established.

		Use the synthVoice->getModuleCoreStrings( ) function to retrieve a vector full
		of these strings and use that to populate the control, overwriting the dummy
		variables you setup. Note that this happens at run-time when the GUI is opened
		and then again whenever the user selects a different core.

		The function uses an unsigned int mask to parse the information, e.g. LFO1_WAVEFORMS,
		FILTER1_TYPES, EG2_CONTOUR, etc... as defined in synthconstants.h

		To retrieve the oscillator 1 waveform strings as a result of the user selecting
		a new core for that oscillator, write:

		// --- vector to fill
		std::vector<std::string> waveforms;

		// --- get OSC1 waveforms
		waveforms = synthVoice->getModuleStrings(OSC1_WAVEFORMS, false); // <- false = not-mod knob labels


		Mod Knob Labels:
		In addition to the module string list being changed, selecting a new core may
		also change the A, B, C, D labels above each of the 4 mod knobs. You need to
		have a GUI library that allows you to change these strings during run-time.

		Use the synthVoice->getModuleCoreStrings( ) function to retrieve the mod knob
		names. Set the argument modKnobs = true to retrieve these strings. Use the constants
		defined in synthconstants.h to select the strings for a particular module.

		To get the mod knob label strings in a vector (it will return a list of 4 strings)
		for LFO1, write:

		// --- vector to fill
		std::vector<std::string> modKnobLables;

		// --- get LFO1 mod knobs
		modKnobLables = synthVoice->getModuleStrings(LFO1_MOD_KNOBS, true); // <- false = not-mod knob labels
	*/

	// ----------------------------------------------------------------------------------------------------------------------------
	/**
	\struct SynthVoiceParameters
	\ingroup SynthVoice

	\brief Contains parameters for the Synth Voice component.
	- A "parameter" is any variable that *may* be connected to a GUI control, however
	parameters are not required to be connected to anything and their default values
	are set in the structure.

	- holds shared pointers to custom parameter structures for all of the voice's
	SynthModules
	-includes a few global-voice parameters like poramento time and unison detuning

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct SynthVoiceParameters
	{
		SynthVoiceParameters() {}

		// --- synth mode; engine has same variable
		uint32_t synthModeIndex = enumToInt(SynthMode::kMono);

		// --- synth mode; engine has same variable
		uint32_t filterModeIndex = enumToInt(FilterMode::kSeries);

		// --- portamento (glide)
		bool enablePortamento = false;// false;

		// --- glide time
		double glideTime_mSec = 0.0;

		// --- legato mode
		bool legatoMode = false;

		// --- freerun
		bool freeRunOscMode = false;

		// --- unison Detune - each voice will be detuned differently
		double unisonDetuneCents = 0.0;
		double unisonStartPhase = 0.0;
		double unisonPan = 0.0;

		// --- components SL_WT
#ifdef SYNTHLAB_WT
		std::shared_ptr<WTOscParameters> osc1Parameters = std::make_shared<WTOscParameters>();
		std::shared_ptr<WTOscParameters> osc2Parameters = std::make_shared<WTOscParameters>();
		std::shared_ptr<WTOscParameters> osc3Parameters = std::make_shared<WTOscParameters>();
		std::shared_ptr<WTOscParameters> osc4Parameters = std::make_shared<WTOscParameters>();
#elif defined SYNTHLAB_VA
		std::shared_ptr<VAOscParameters> osc1Parameters = std::make_shared<VAOscParameters>();
		std::shared_ptr<VAOscParameters> osc2Parameters = std::make_shared<VAOscParameters>();
		std::shared_ptr<VAOscParameters> osc3Parameters = std::make_shared<VAOscParameters>();
		std::shared_ptr<VAOscParameters> osc4Parameters = std::make_shared<VAOscParameters>();
#elif defined SYNTHLAB_PCM
		std::shared_ptr<PCMOscParameters> osc1Parameters = std::make_shared<PCMOscParameters>();
		std::shared_ptr<PCMOscParameters> osc2Parameters = std::make_shared<PCMOscParameters>();
		std::shared_ptr<PCMOscParameters> osc3Parameters = std::make_shared<PCMOscParameters>();
		std::shared_ptr<PCMOscParameters> osc4Parameters = std::make_shared<PCMOscParameters>();
#elif defined SYNTHLAB_KS
		std::shared_ptr<KSOscParameters> osc1Parameters = std::make_shared<KSOscParameters>();
		std::shared_ptr<KSOscParameters> osc2Parameters = std::make_shared<KSOscParameters>();
		std::shared_ptr<KSOscParameters> osc3Parameters = std::make_shared<KSOscParameters>();
		std::shared_ptr<KSOscParameters> osc4Parameters = std::make_shared<KSOscParameters>();
#elif defined SYNTHLAB_DX
		// --- fm Algo
		uint32_t fmAlgorithmIndex = enumToInt(DX100Algo::kFM1);
		std::shared_ptr<FMOperatorParameters> osc1Parameters = std::make_shared<FMOperatorParameters>();
		std::shared_ptr<FMOperatorParameters> osc2Parameters = std::make_shared<FMOperatorParameters>();
		std::shared_ptr<FMOperatorParameters> osc3Parameters = std::make_shared<FMOperatorParameters>();
		std::shared_ptr<FMOperatorParameters> osc4Parameters = std::make_shared<FMOperatorParameters>();
#elif defined SYNTHLAB_WS
		std::shared_ptr<WaveSequencerParameters> waveSequencerParameters = std::make_shared<WaveSequencerParameters>();

		std::shared_ptr<WSOscParameters> wsOsc1Parameters = std::make_shared<WSOscParameters>();
		std::shared_ptr<WSOscParameters> wsOsc2Parameters = std::make_shared<WSOscParameters>();
#endif

		// --- LFOs
		std::shared_ptr<LFOParameters> lfo1Parameters = std::make_shared<LFOParameters>();
		std::shared_ptr<LFOParameters> lfo2Parameters = std::make_shared<LFOParameters>();

		// --- EGs
		std::shared_ptr<EGParameters> ampEGParameters = std::make_shared<EGParameters>();
		std::shared_ptr<EGParameters> filterEGParameters = std::make_shared<EGParameters>();
		std::shared_ptr<EGParameters> auxEGParameters = std::make_shared<EGParameters>();

		// --- filters
		std::shared_ptr<FilterParameters> filter1Parameters = std::make_shared<FilterParameters>();
		std::shared_ptr<FilterParameters> filter2Parameters = std::make_shared<FilterParameters>();

		// --- DCA
		std::shared_ptr<DCAParameters> dcaParameters = std::make_shared<DCAParameters>();

		// --- ModMatrix
		std::shared_ptr<ModMatrixParameters> modMatrixParameters = std::make_shared<ModMatrixParameters>();

		// --- Dynamic String suport: you can use these to keep track of the lists and knobs
		uint32_t updateCodeDroplists = 0;
		uint32_t updateCodeKnobs = 0;
	};

	// --- voice mode: note on or note off states
	enum class voiceState { kNoteOnState, kNoteOffState };

	/**
	\class SynthVoice
	\ingroup SynthVoice
	\brief
	This is the voice object for a software synth.

	The voice performs three tasks during the synth’s operation:
	- initialization
	- responding to MIDI note-on and note-off messages
	- controlling the audio signal flow through a set of member objects called modules.

	The voice object’s central responsibility is maintaining this set of SynthModule objects
	that make up the synthesizer components such as oscillators and filters.
	I designed the SynthVoice object to expose simple functions that service these
	three areas of operation. The voice object also processes incoming MIDI data for note-on
	and note-off events, which it uses to control its set of modules.

	1.	Initialization: the voice calls the module’s reset function
	2.	Note-on and Note-off: the voice calls the doNoteOn and doNoteOff methods on its set of modules
	3.	Controlling Audio Signal Flow: the voice calls the module’s update and render functions during each block processing cycle, and delivers the rendered audio back to the engine

	Base Class: None
	- This object has no base class. It may be used as a base class for your own implementaitons,
	so several functions are declared as virtual.

	GUI Parameters: SynthVoiceParameters
	- getParameters() function allows direct access to std::shared_ptr<SynthVoiceParameters>

	std::shared_ptr<SynthVoiceParameters> getParameters()

	- call the getParameters() function
	- set the parameters in the SynthVoiceParameters structure with new values, typically from a GUI
	- the parameters will be applied to the underlying module during the render cycle.

	Construction:

	(1) For use within a synth project, the constructor
	is specialized to use shared recources for:
	- MidiInputData
	- MidiOutputData (currently not supported or used, but may be added for MIDI output-only plugins)
	- WavetableDatabase
	- PCMSampleDatabase

	The owning object (SynthEngine for the SynthLab projects) must pass these valid pointers
	to the object at construction time. Typically the engine will be the primary synthesizers
	of these resources. See the 2nd Edition Synth Book for more information.

	(2) Standalone:

	To use in standalone mode, call the constructor with the shared resoure pointers as null:

	SynthVoice(nullptr, nullptr, nullptr, nullptr, nullptr, 64);

	In standalone mode, the object creates and maintains these resources:
	- SynthVoiceParameters: in standalone mode only, these are synthesized locally on the object,
	and then the owning object may obtain a shared pointer to them to read/write the parameters directly.

	Render:
	- renders into its own AudioBuffers object; see SynthModule::getAudioBuffers()
	- renders stereo by default.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class SynthVoice
	{
	public:
		SynthVoice(std::shared_ptr<MidiInputData> _midiInputData,
			std::shared_ptr<MidiOutputData> _midiOutputData,
			std::shared_ptr<SynthVoiceParameters> _parameters,
			std::shared_ptr<WavetableDatabase> _wavetableDatabase,
			std::shared_ptr<PCMSampleDatabase> _sampleDatabase,
			uint32_t _blockSize = 64);

		virtual ~SynthVoice() {} ///< empty destructor

		/** main functions, declared as virtual so you can use as as base class if needed*/
		virtual bool reset(double _sampleRate);
		virtual bool update();
		virtual bool render(SynthProcessInfo& synthProcessInfo);
		virtual bool processMIDIEvent(midiEvent& event);
		virtual bool initialize(const char* dllPath = nullptr);
		virtual bool doNoteOn(midiEvent& event);
		virtual bool doNoteOff(midiEvent& event);

		/** returns voice activity; it is active if playing a note event */
		bool isVoiceActive() { return voiceIsActive; }

		/** returns voice state; it is either note-on or note-off */
		voiceState getVoiceState() { return voiceNoteState; }

		// --- timestamps for determining note age;
		//     public because containing object needs to manipulate them
		uint32_t getTimestamp() { return timestamp; }		///< get current timestamp, the higher the value, the older the voice has been running
		void incrementTimestamp() { timestamp++; }			///< increment timestamp when a new note is triggered
		void clearTimestamp() { timestamp = 0; }			///< reset timestamp after voice is turned off

		// --- MIDI note stuff
		uint32_t getMIDINoteNumber() { return voiceMIDIEvent.midiData1; } ///< note is data byte 1, velocity is byte 2

		// --- voice steal
		uint32_t getStealMIDINoteNumber() { return voiceStealMIDIEvent.midiData1; } ///< note is data byte 1, velocity is byte 2
		bool voiceIsStealing() { return stealPending; } ///< trur if voice will be stolen

		// --- Dynamic Strings
		// --- these are only for dynamic module loading and can be removed for non ASPiK versions
		// --- optional for frameworks that can load dynamic GUI stuff
		std::vector<std::string> getModuleCoreNames(uint32_t moduleType);			///< only for dynamic string loading
		std::vector<std::string> getModuleStrings(uint32_t mask, bool modKnobs);	///< only for dynamic string loading

		// --- Dynamic String suport
		void setAllCustomUpdateCodes(); ///< one of many ways to keep track of what needs updating; this will likely be very dependent on your GUI system and plugin framework

		// --- functions to load individual cores, if using this option (see top of file)
		void loadLFOCore(uint32_t lfoIndex, uint32_t index); ///< load a new LFO core
		void loadFilterCore(uint32_t filterIndex, uint32_t index);///< load a new filter core
		void loadOscCore(uint32_t oscIndex, uint32_t index);///< load a new oscillator core
		void loadEGCore(uint32_t egIndex, uint32_t index);///< load a new EG core

		// --- DM STUFF ---
		void setDynamicModules(std::vector<std::shared_ptr<SynthLab::ModuleCore>> modules); ///< add dynamically loaded DLL modules to existing cores

	protected:
		/** standalone operation only */
		std::shared_ptr<SynthVoiceParameters> parameters = nullptr;

		// --- local storage
		double sampleRate = 0.0;
		uint32_t blockSize = 64;

		// --- interface pointer
		std::shared_ptr<MidiInputData> midiInputData = nullptr;			///< shared MIDI input data
		std::shared_ptr<MidiOutputData> midiOutputData = nullptr;	///< shared MIDI output data (not used in SynthLab)
		std::shared_ptr<WavetableDatabase> wavetableDatabase = nullptr; ///< shared wavetable database
		std::shared_ptr<PCMSampleDatabase> sampleDatabase = nullptr;	///< shared PCM database

		std::shared_ptr<AudioBuffer> mixBuffers = nullptr; ///< buffers for mixing audio and procesisng the voice digital audio engine
		void accumulateToMixBuffer(std::shared_ptr<AudioBuffer> oscBuffers, uint32_t samplesInBlock, double scaling); ///< accumulating voice audio data
		void writeToMixBuffer(std::shared_ptr<AudioBuffer> oscBuffers, uint32_t samplesInBlock, double scaling = 1.0); ///< write to final mix buffer

		// --- voice timestamp, for knowing the age of a voice
		uint32_t timestamp = 0;						///<voice timestamp, for knowing the age of a voice
		int32_t currentMIDINote = -1;				///<voice timestamp, for knowing the age of a voice

		// --- note message state
		voiceState voiceNoteState = voiceState::kNoteOffState; ///< state variable

		// --- per-voice stuff
		bool voiceIsActive = false;	///< activity flag
		midiEvent voiceMIDIEvent;	///< MIDI note event for current voice

		// --- for voice stealing
		bool stealPending = false;		///< stealing is inevitible
		midiEvent voiceStealMIDIEvent; ///< MIDI note event for the new (stolen) voice

		// --- each voice has a modulation matrix
		//     but rows/columns are shared via matrix parameters
		std::unique_ptr<ModMatrix> modMatrix; ///< mod matrix for this voice

#ifdef SYNTHLAB_WS
		// --- wave sequencer (only used in SynthLab-WS)
		std::unique_ptr<WaveSequencer> waveSequencer;

		// ---- components: 1 WS oscillator
		const enum { MAIN_OSC, DETUNED_OSC, NUM_WS_OSC };
		std::unique_ptr<WSOscillator> wsOscillator[NUM_WS_OSC] = { nullptr, nullptr };	///< oscillator (WS only_
#else
		// ---- components: 4 oscillators
		std::unique_ptr<SynthModule> oscillator[NUM_OSC];	///< oscillators
#endif

		// --- LFOs
		std::unique_ptr<SynthLFO> lfo[NUM_LFO];				///< LFOs

		// --- Filters
		std::unique_ptr<SynthFilter> filter[NUM_FILTER];	///< filters

		// --- EGs
		std::unique_ptr<EnvelopeGenerator> ampEG;		///< amp EG
		std::unique_ptr<EnvelopeGenerator> filterEG;	///< filter EG
		std::unique_ptr<EnvelopeGenerator> auxEG;		///< auxEG

		// --- DCA(s)
		std::unique_ptr<DCA> dca;						///< one and only DCA

		/** remove DC offsets - many wavetables contain DC offsets when cobbled together*/
		DCRemovalFilter dcFilter[STEREO_CHANNELS];	///< DC removal for short term random bias
		inline void removeMixBufferDC(uint32_t blockSize)
		{
			float* leftOutBuffer = mixBuffers->getOutputBuffer(LEFT_CHANNEL);
			float* rightOutBuffer = mixBuffers->getOutputBuffer(RIGHT_CHANNEL);
			for (uint32_t i = 0; i < blockSize; i++)
			{
				// --- stereo
				leftOutBuffer[i] = dcFilter[LEFT_CHANNEL].processAudioSample(leftOutBuffer[i]);
				rightOutBuffer[i] = dcFilter[RIGHT_CHANNEL].processAudioSample(rightOutBuffer[i]);
			}
		}
	};

}
#endif /* defined(__synthVoice_h__) */
