#ifndef __wavesample__
#define __wavesample__

#include "stdint.h"
#include <string>
#include <iostream>
#include <map>

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   pcmsample.h
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
	Simple structure to hold riff-chunk data from a WAV file
	*/
	struct  RIFF_CHUNK
	{
		char  IdentifierString[4] = "";
		uint32_t dwLength = 0;
	};

	/**
	\ingroup SynthStructures
	Simple structure to hold wave file header information about the data.
	*/
	struct WAVE_FILE_HEADER
	{
		uint16_t  wFormatTag = 0;         // Format category
		uint16_t  wChannels = 0;          // Number of channels
		uint32_t  dwSamplesPerSec = 0;    // Sampling rate
		uint32_t  dwAvgBytesPerSec = 0;   // For buffer estimation
		uint16_t  wBlockAlign = 0;        // Data block size
		uint16_t  wBitsPerSample = 0;
	};

	/**
	\ingroup SynthStructures
	Simple structure to mimic the windows-only WAVEFORMATEX on any OS
	*/
	struct WAVEFORMATEX_WP
	{
		uint16_t wFormatTag = 0;         /* format type */
		uint16_t nChannels = 0;          /* number of channels (i.e. mono, stereo...) */
		uint32_t nSamplesPerSec = 0;     /* sample rate */
		uint32_t nAvgBytesPerSec = 0;    /* for buffer estimation */
		uint16_t nBlockAlign = 0;        /* block size of data */
		uint16_t wBitsPerSample = 0;     /* number of bits per sample of mono data */
		uint16_t cbSize = 0;             /* the count in bytes of the size of */
	} ;

	/**
	\ingroup SynthStructures
	Simple structure to hold data about a sample while parsing
	*/
	struct WAVE_SAMPLE
	{
		WAVEFORMATEX_WP   WaveFormatEx;
		char              *pSampleData = nullptr;
		uint32_t          Index = 0;
		uint32_t          Size = 0;
		uint32_t          dwId = 0;
		uint32_t          bPlaying = 0;
	};

	/**
	\ingroup Constants-Enums
	A union for converting datatypes without mangling bits (really old trick)
	*/
	union UWaveData
	{
		float f = 0.0; // only need one initializer
		double d;
		int32_t n;
		uint32_t u;
		uint64_t u64;
	};

	/**
	\class PCMSample
	\ingroup SynthObjects
	\brief
	Opens a WAV file and extracts contents into a floating point buffer, regardless of original datatypes in file.
	After parsing the file, use const float* getSampleBuffer() to acces the read-only buffer of data.
	The following types are supported:
	- 16-BIT Signed Integer PCM
	- 24-BIT Signed Integer PCM 3-ByteAlign
	- 24-BIT Signed Integer PCM 4-ByteAlign
	- 32-BIT Signed Integer PCM
	- 32-BIT Floating Point
	- 64-BIT Floating Point


	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class PCMSample
	{
	public:
		PCMSample() { }
		~PCMSample();// { if (pcmSampleBuffer) delete[] pcmSampleBuffer; }

		/** PCM sample load and buffer access*/
		bool loadPCMSample(const char* filePath);
		const float* getSampleBuffer() { return pcmSampleBuffer; }

		/** pitchless samples (read inc = 1 always) */
		void setPitchless(bool _pitchlessSample) { pitchlessSample = _pitchlessSample; }
		bool isPitchless() { return pitchlessSample; }

		/** only set true if WAV parse was sucessful*/
		bool isSampleLoaded() { return sampleLoaded; }

		//@{
		/**
		\brief
		Immutable variables can only be set during file parse
		*/
		uint32_t getNumChannels() { return numChannels; }
		uint32_t getSampleRate() { return sampleRate; }
		uint32_t getSampleCount() { return sampleCount; }
		uint32_t getLoopType() { return loopType; }
		uint32_t getSmpteFormat() { return smpteFormat; }
		uint32_t getSmpteOffset() { return smpteOffset; }
		//@}

		//@{
		/**
		\brief
		Mutable variables that may need to be re-calculated after parsing
		*/
		uint32_t getLoopCount() { return loopCount; }
		void setLoopCount(uint32_t u) { loopCount = u; }

		uint32_t getLoopStartIndex() { return loopStartIndex; }
		void setLoopStartIndex(uint32_t u) { loopStartIndex = u; }

		uint32_t getLoopEndIndex() { return loopEndIndex; }
		void setLoopEndIndex(uint32_t u) { loopEndIndex = u; }

		uint32_t getUnityMIDINote() { return unityMIDINote; }
		void setUnityMIDINote(uint32_t u) { unityMIDINote = u; }

		uint32_t getUnityMIDIPitchFraction() { return unityMIDIPitchFraction; }
		void setUnityMIDIPitchFraction(uint32_t u) { unityMIDIPitchFraction = u; }
		//@}

	protected:
		uint32_t numChannels = 0;
		uint32_t sampleRate = 0;
		uint32_t sampleCount = 0;
		uint32_t loopCount = 0;
		uint32_t loopStartIndex = 0;
		uint32_t loopEndIndex = 0;
		uint32_t loopType = 0;
		uint32_t unityMIDINote = 0;
		uint32_t unityMIDIPitchFraction = 0;
		uint32_t smpteFormat = 0;
		uint32_t smpteOffset = 0;
		bool sampleLoaded = false;

		// --- the WAV file converted to floats on range of -1.0 --> +1.0
		float* pcmSampleBuffer = nullptr;
		bool pitchlessSample = false;
	};

	/**
	\ingroup SynthStructures
	simple case converter for unsigned chars; used for filename case setting
	*/
	struct convertUpper {
		void operator()(char& c) { c = toupper((unsigned char)c); }
	};

	/**
	\class WaveFolder
	\ingroup SynthObjects
	\brief
	Opens a folder full of WAV files and gleans information about the files to prep them for parsing
	and having their sample guts extracted. This object can figure out MIDI unity note numbers from
	the WAV file names (e.g. Dagga_A#4.wav). This object is used to parse folders of samples to build
	patches for sample based synths.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class WaveFolder
	{
	public:	// Functions
			//
			// One Time Initialization
			// waveFolder is the FULLY qualified file name + additional path info
			// VALID Examples: audio.wav
			//				   //samples//audio.wav
		WaveFolder(const char* _waveFolderPath,
                   const char* _waveFolderName) {
            waveFolderPath = _waveFolderPath;
			waveFolderName = _waveFolderName;
			buildNoteTables();
		}
		~WaveFolder() {}

		// --- get next folder
		uint32_t parseFolder(PCMSample** sampleSet, bool pitchlessLoops, bool aubioSlices = false);

        // --- add file and information about it to a map, used for parsing files in second step
        void addNextFileToMap(std::string fileFolderPath, std::string fileName, bool aubioSlices, std::map<int, std::string>* wavFilePaths, int& fileCount);

        // --- helper
        void eraseSubStr(std::string & mainStr, const std::string & toErase)
        {
            // Search for the substring in string
            size_t pos = mainStr.find(toErase);
            if (pos != std::string::npos)
            {
                // If found then erase it from string
                mainStr.erase(pos, toErase.length());
            }
        }

	protected:
		const char* waveFolderPath;
		const char* waveFolderName;
		void buildNoteTables();
		std::string noteTableSharps[120];	///< table with characters used to decode filenames with sharps
		std::string noteTableFlats[120];	///< table with characters used to decode filenames with flats
		int32_t findNoteNumberInName(const char* filename, bool shiftUpOctave = true); ///< figure out MIDI note number from string
	};

} // namespace
#endif
