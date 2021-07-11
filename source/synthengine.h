#ifndef __synthCore_h__
#define __synthCore_h__

// --- we need these
#include "synthbase.h"
#include "synthfunctions.h"
#include "synthvoice.h"

namespace SynthLab
{
	/**
	\struct SynthEngineParameters
	\ingroup SynthEngine

	\brief Contains parameters for the Synth Engine component.
	- A "parameter" is any variable that *may* be connected to a GUI control, however
	parameters are not required to be connected to anything and their default values
	are set in the structure.

	- holds a shared parameter pointer for the voice object, used for sharing data across all voices
	- holds a shared pointer to the audio delay object's parameter but currently not shared with any other object
	- see the Synth Boook for much more detail on how this structure is used to safely and efficiently share data

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct SynthEngineParameters
	{
		SynthEngineParameters() {}

		// --- enable/disable keyboard (MIDI note event) input; when disabled, synth goes into manual mode (Will's VCS3)
		bool enableMIDINoteEvents = true;

		// --- global synth mode
		uint32_t synthModeIndex = enumToInt(SynthMode::kMono);

		// --- global  volume control, controls each output DCA's master volume
		double globalVolume_dB = 0.0;

		// --- master pitch bend, in semitones and cents
		unsigned int globalPitchBendSensCoarse = 7; // --- this number is always positive (parse as +/- value) 7 semitones = perfect 5th
		unsigned int globalPitchBendSensFine = 0;	// --- this number is always positive (parse as +/- value) see MIDI RPN 00 00 (sensitivity) and 00 01 (fine tuning, cents -100/+100) and 00 02 (coarse tuning, semintones -63/+64)

		// --- these are actually really important, especially for non-western music styles and
		//     eccentric electronic music composers too...
		int globalTuningCoarse = 0;		// --- (+/-) semitones, see MIDI spec
		int globalTuningFine = 0;		// --- (+/-) cents see MIDI spec

		// --- unison Detune - this is the max detuning value NOTE a standard (or RPN or NRPN) parameter :/
		double globalUnisonDetune_Cents = 0.0;

		// --- VOICE layer parameters
		std::shared_ptr<SynthVoiceParameters> voiceParameters = std::make_shared<SynthVoiceParameters>();

		// --- FX is unique to engine, not part of voice
		// std::shared_ptr<AudioDelayParameters> audioDelayParameters = std::make_shared<AudioDelayParameters>();
		// bool enableDelayFX = false;
	};


	/**
	\class SynthEngine
	\ingroup SynthEngine
	\brief Encapsulates an entire synth engine, producing one type of synthesizer set of voices
	(e.g. Virtual Analog, Sample Based, FM, etc...)
	- contains an array of SynthVoice objects to render audio and also processes MIDI events
	- contains an audio delay used as a master-buss effect
	- contains functions to interface with framework to deliver dynamic string lists (advanced GUI)
	- creates the global MIDI data object and passes shared pointers to all voices
	- creates the wavetable database object and passes shared pointers to all voices
	- creates the PCM sample database object and passes shared pointers to all voices

	\author Will Pirkle
	\version Revision : 1.0
	\date Date : 2017 / 09 / 24
	*/
	class SynthEngine
	{
	public:
		SynthEngine(uint32_t blockSize = 64);
		virtual ~SynthEngine();

		/** main functions, declared as virtual so you can use as as base class if needed*/
		virtual bool reset(double _sampleRate);
		virtual bool render(SynthProcessInfo& synthProcessInfo);
		virtual bool processMIDIEvent(midiEvent& event);
		virtual bool initialize(const char* dllPath = nullptr);

		/** Functions to help with rendering the final synth output audio stream */
		void accumulateVoice(SynthProcessInfo& synthProcessInfo, double scaling = 0.707);
		void applyGlobalVolume(SynthProcessInfo& synthProcessInfo);

		// --- get parameters
		void getParameters(std::shared_ptr<SynthEngineParameters>& _parameters) { _parameters = parameters; }

		// --- set parameters
		void setParameters(std::shared_ptr<SynthEngineParameters>& _parameters);

		/** Voice stealing helper functions */
		int getFreeVoiceIndex();
		int getVoiceIndexToSteal();
		int getActiveVoiceIndexInNoteOn(unsigned int midiNoteNumber);
		int getStealingVoiceIndexInNoteOn(unsigned int midiNoteNumber);

	protected:
		// --- only need one for iteration
		SynthProcessInfo voiceProcessInfo;

		// --- our modifiers (parameters)
		// --- SynthEngineParameters parameters;
		std::shared_ptr<SynthEngineParameters> parameters = std::make_shared<SynthEngineParameters>();

		// --- shared MIDI tables, via IMIDIData
		std::shared_ptr<MidiInputData> midiInputData = std::make_shared<MidiInputData>();
		std::shared_ptr<MidiOutputData> midiOutputData = std::make_shared<MidiOutputData>();

		// --- array of voice object, via pointers
		std::unique_ptr<SynthVoice> synthVoices[MAX_VOICES] = { 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0 };		///< array of voice objects for the engine

		// --- shared tables, in case they are huge or need a long creation time
		std::shared_ptr<WavetableDatabase> wavetableDatabase = nullptr;

		// --- shared tables, in case they are huge or need a long creation time
		std::shared_ptr<PCMSampleDatabase> sampleDatabase = nullptr;

	};

}

#endif /* defined(__synthCore_h__) */
