#ifndef __synthSruxt_h__
#define __synthSruxt_h__

#include <cmath>
#include <random>
#include <string>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <memory>

#include "synthconstants.h"

#define _MATH_DEFINES_DEFINED

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthstructures.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{ 
	/**
	\struct XFadeData
	\ingroup SynthStructures
	\brief
	Data about a crossfade operation.
	- includes gain coefficients for linear, constant power and square law
	- note the crossfade done flag, that lets caller know when crossfade is completed

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct XFadeData
	{
		// --- stereo input/output crossfader
		double linearGain[2] = { 0.0, 0.0 };		///< linear coefficients
		double constPwrGain[2] = { 0.0, 0.0 };		///< constant power coefficients
		double squareLawGain[2] = { 0.0, 0.0 };		///< square law coefficients
		bool crossfadeFinished = false;				///< crossfade is done
	};


	/**
	\struct MidiOutputData
	\ingroup SynthStructures
	\brief
	MIDI output message and data information.
	- not used in SynthLab

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct MidiOutputData
	{
		MidiOutputData() {}

		// --- shared MIDI tables, via IMIDIData
		uint32_t globalMIDIData[kNumMIDIGlobals] = { 0 };	///< the global MIDI INPUT table that is shared across the voices via the IMIDIData interface
		uint32_t ccMIDIData[kNumMIDICCs] = { 0 };			///< the global MIDI CC INPUT table that is shared across the voices via the IMIDIData interface
	};

	/**
	\struct WaveStringData
	\ingroup SynthStructures
	\brief
	Information about the selected core and the selected wavform within that core.
	- used in the wave sequencer oscillator to keep track of which oscillator is playing which waveform

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct WaveStringData
	{
		WaveStringData(uint32_t _coreIndex, uint32_t _coreWaveIndex)
			: coreIndex(_coreIndex)
			, coreWaveIndex(_coreWaveIndex) {}

		uint32_t coreIndex = 0;			///< selected core
		uint32_t coreWaveIndex = 0;		///< selected waveform within core
	};

	/**
	\struct LookUpTable
	\ingroup SynthStructures
	\brief
	Structure to hold a dynamic LUT and its length.
	- very low level array that is not shared
	- destructor must be called during lifecycle

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	struct LookUpTable
	{
		LookUpTable(uint32_t _tableLength)
		{
			tableLength = _tableLength;
			table = new double[tableLength];
		}
		~LookUpTable()
		{
			if (table) delete[] table;
		}
		uint32_t tableLength = 1024;
		double* table = nullptr;
	};


	/**
	\struct PluginInfo
	\ingroup SynthStructures
	\brief
	Structure that is used during the base class initilize( ) funciton call, after object instantiation is complete.
	This structure contains the path to the DLL itself which can be used to open/save files including pre-installed
	WAV files for sample based synths.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct PluginInfo
	{
		PluginInfo() {}

		PluginInfo& operator=(const PluginInfo& data)	// need this override for collections to work
		{
			if (this == &data)
				return *this;

			pathToDLL = data.pathToDLL; ///< note copies pointer - somewhat useless

			return *this;
		}

		const char* pathToDLL;	///< complete path to the DLL (component) without trailing backslash
	};

	/**
	\struct midiEvent
	\ingroup SynthStructures
	\brief
	Information about a MIDI event.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct midiEvent
	{
		midiEvent() {}

		midiEvent(uint32_t _midiMessage,
			uint32_t _midiChannel,
			uint32_t _midiData1,
			uint32_t _midiData2,
			uint32_t _midiSampleOffset = 0)
			: midiMessage(_midiMessage)
			, midiChannel(_midiChannel)
			, midiData1(_midiData1)
			, midiData2(_midiData2)
			, midiSampleOffset(_midiSampleOffset) {}

		midiEvent& operator=(const midiEvent& data)	// need this override for collections to work
		{
			if (this == &data)
				return *this;

			midiMessage = data.midiMessage;
			midiChannel = data.midiChannel;
			midiData1 = data.midiData1;
			midiData2 = data.midiData2;
			midiSampleOffset = data.midiSampleOffset;
			return *this;
		}

		uint32_t    midiMessage = 0;			///< BYTE message as UINT
		uint32_t    midiChannel = 0;			///< BYTE channel as UINT
		uint32_t    midiData1 = 0;				///< BYTE data 1 as UINT
		uint32_t    midiData2 = 0;				///< BYTE data 2 as UINT
		uint32_t    midiSampleOffset = 0;		///< sample offset of midi event within audio buffer
	};

	/**
	\struct MIDINoteEvent
	\ingroup SynthStructures
	\brief
	Information about a MIDI note event (note on or note off).

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct MIDINoteEvent
	{
		// --- call with no params for default
		MIDINoteEvent(double _midiPitch = 0.0, uint32_t _midiNoteNumber = 0, uint32_t _midiNoteVelocity = 0)
			: midiPitch(_midiPitch)
			, midiNoteNumber(_midiNoteNumber)
			, midiNoteVelocity(_midiNoteVelocity) {}

		double midiPitch = 0.0;			///< pitch in Hz of the MIDI note that was played
		uint32_t midiNoteNumber = 0;	///< note number (saved for portamento and voice steal)
		uint32_t midiNoteVelocity = 0;	///< note velocity (saved for portamento and voice steal)
	};
	/**
	\struct midiEvent
	\ingroup SynthStructures
	\brief
	Information about a portamento event.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct GlideInfo
	{
		GlideInfo(uint32_t _startMIDINote, uint32_t _endMIDINote, double _glideTime_mSec, double _sampleRate)
			: startMIDINote(_startMIDINote)
			, endMIDINote(_endMIDINote)
			, glideTime_mSec(_glideTime_mSec)
			, sampleRate(_sampleRate) {}

		uint32_t startMIDINote = 0;		///< starting MIDI note for the glide
		uint32_t endMIDINote = 0;		///< ending MIDI note for the glide
		double glideTime_mSec = 0.0;	///< glide time to cover the range of notes
		double sampleRate = 0.0;		///< fs
	};




	/**
	\struct ModuleCoreData
	\ingroup SynthStructures
	\brief
	Contains the two sets of strings unique to each core: the module strings (waveforms for oscillators)
	and the mod knob label strings. 

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct ModuleCoreData
	{
		// --- one droplist control, depending on type of module can contain max 8 slots
		const char* moduleStrings[MODULE_STRINGS] = { "","","","","","","","","","","","","","","","" };	///< up to 16
		int32_t uniqueIndexes[MODULE_STRINGS] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };		///< up to 16
		const char* modKnobStrings[MOD_KNOBS] = { "","","","" };		///< up to 4
	};

	///**
	//\struct MorphBankData
	//\ingroup SynthStructures
	//\brief
	//Information about a bank of wavetables that are used in the morphing wavetable core. 

	//\author Will Pirkle http://www.willpirkle.com
	//\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	//\version Revision : 1.0
	//\date Date : 2021 / 05 / 02
	//*/
	//struct MorphBankData
	//{
	//	std::string bankName = empty_string.c_str();	///< one name for bank
	//	uint32_t numTables = 1;							///< number of wavetables in bank (up to 16)
	//	std::string tableNames[MODULE_STRINGS];			///< names of wavetables
	//	int32_t tableIndexes[MODULE_STRINGS] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };			///< unique indexes for faster lookup
	//	StaticTableSource wavetables[MODULE_STRINGS];
	//};

} // namespace

#endif /* defined(__synthDefs_h__) */
