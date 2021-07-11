#ifndef __synthVoice_h__
#define __synthVoice_h__

// --- SynthLab only
#include "synthbase.h"
#include "synthfunctions.h"

namespace SynthLab
{
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

		// --- unison Detune - each voice will be detuned differently
		double unisonDetuneCents = 0.0;
		double unisonStartPhase = 0.0;
		double unisonPan = 0.0;

		// --- add your module GUI parametrs here
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
		unsigned int getMIDINoteNumber() { return voiceMIDIEvent.midiData1; } ///< note is data byte 1, velocity is byte 2

		// --- voice steal
		unsigned int getStealMIDINoteNumber() { return voiceStealMIDIEvent.midiData1; } ///< note is data byte 1, velocity is byte 2
		bool voiceIsStealing() { return stealPending; } ///< trur if voice will be stolen

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
		void writeToMixBuffer(std::shared_ptr<AudioBuffer> oscBuffers, uint32_t samplesInBlock, double scaling); ///< write to final mix buffer

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

		// ---- place your individual synth components here



	};

}
#endif /* defined(__synthVoice_h__) */
