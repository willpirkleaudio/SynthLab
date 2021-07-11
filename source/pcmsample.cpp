#include "pcmsample.h"
#include "synthfunctions.h"

#include <fstream>
#include <iomanip>

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   pcmsample.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	PCMSample::~PCMSample()
	{
		if (pcmSampleBuffer) delete[] pcmSampleBuffer;
	}

#if defined _WIN32 || defined _WIN64
    #include <windows.h>
#else
    #import <CoreFoundation/CoreFoundation.h>
#endif

	/** @loadPCMSample
	\brief
	Opens a WAV file and extracts the audio guts into a buffer of floats. Anytime later, you can
	use isSampleLoaded( ) to see if the sample data is valid.

	\param pFilePath fully qualified WAV file path

	\return true if sucessful
	*/
	bool PCMSample::loadPCMSample(const char* pFilePath)
	{
		sampleLoaded = false;
		bool bFailed = false;
		RIFF_CHUNK RiffChunk = { {0} };
		WAVE_FILE_HEADER WaveFileHeader;
		uint32_t dwIncrementBytes;
		WAVE_SAMPLE WaveSample;
		numChannels = 0;
		sampleRate = 0;
		sampleCount = 0;

		std::ifstream inFile;
		inFile.open(pFilePath, std::ifstream::binary | std::ifstream::in);

        // --- OK?
		bool bIsOpen = inFile.is_open();

		if (!bIsOpen)
			return false;

		// --- this is used to identify each chunk
		char szIdentifier[5] = { 0 };

		// --- advance 12 chars
		inFile.seekg(12);

		// --- read first RiffChunk and the WaveFileHeader
		inFile.read((char*)(&RiffChunk), sizeof(RiffChunk));
		inFile.read((char*)(&WaveFileHeader), sizeof(WaveFileHeader));

		// --- set the waveformatex
		WaveSample.WaveFormatEx.wFormatTag = WaveFileHeader.wFormatTag;
		WaveSample.WaveFormatEx.nChannels = WaveFileHeader.wChannels;
		WaveSample.WaveFormatEx.nSamplesPerSec = WaveFileHeader.dwSamplesPerSec;
		WaveSample.WaveFormatEx.nAvgBytesPerSec = WaveFileHeader.dwAvgBytesPerSec;
		WaveSample.WaveFormatEx.nBlockAlign = WaveFileHeader.wBlockAlign;
		WaveSample.WaveFormatEx.wBitsPerSample = WaveFileHeader.wBitsPerSample;
		WaveSample.WaveFormatEx.cbSize = 0;

		// --- I don't support these types (compressed, uLaw/aLaw, etc..)
		if (WaveSample.WaveFormatEx.wFormatTag != 1 && WaveSample.WaveFormatEx.wFormatTag != 3)
		{
			inFile.close();
			return false;
		}

		// --- for backing up the first seek
		dwIncrementBytes = sizeof(WaveFileHeader);

		do {
			// RiffChunk.dwLength - dwIncrementBytes sets the file pointer to position 0 first time through
			// advance by RiffChunk.dwLength - dwIncrementBytes
			int64_t position = inFile.tellg();
			inFile.seekg(position + (RiffChunk.dwLength - dwIncrementBytes));

			// --- advanced past end of file?
			bFailed = inFile.fail();

			if (!bFailed)
			{
				// --- read the RiffChunk
				inFile.read((char*)(&RiffChunk), sizeof(RiffChunk));

				// --- this now makes the seekg() advance in RiffChunk.dwLength chunks
				//     which vary with the type of chunk
				dwIncrementBytes = 0;

				// --- extract the chunk identifier
				memcpy(szIdentifier, RiffChunk.IdentifierString, 4);
			}

		} while (strcmp(szIdentifier, "data") && !bFailed);

		// --- AUDIO DATA CHUNK data
		// --- 16 bit
		if (!bFailed && WaveSample.WaveFormatEx.wBitsPerSample == 16)
		{
			//WaveSample.pSampleData = (char *)LocalAlloc(LMEM_ZEROINIT, RiffChunk.dwLength);
			//WaveSample.Size = RiffChunk.dwLength;

			//inFile.read(WaveSample.pSampleData, RiffChunk.dwLength);

			//UINT nSampleCount = (float)RiffChunk.dwLength / (float)(WaveSample.WaveFormatEx.wBitsPerSample / 8.0);
			//sampleCount = nSampleCount;

			//numChannels = WaveSample.WaveFormatEx.nChannels;
			//sampleRate = WaveSample.WaveFormatEx.nSamplesPerSec;

			//if (pcmSampleBuffer)
			//	delete[] pcmSampleBuffer;

			//pcmSampleBuffer = new float[nSampleCount];
			//short* pShorts = new short[nSampleCount];
			//memset(pShorts, 0, nSampleCount * sizeof(short));
			//int m = 0;
			//for (UINT i = 0; i < nSampleCount; i++)
			//{
			//	// MSB
			//	pShorts[i] = (unsigned char)WaveSample.pSampleData[m + 1];
			//	pShorts[i] <<= 8;

			//	// LSB
			//	short lsb = (unsigned char)WaveSample.pSampleData[m];
			//	// in case top of lsb is bad
			//	lsb &= 0x00FF;
			//	pShorts[i] |= lsb;
			//	m += 2;
			//}

			//// convet to float -1.0 -> +1.0
			//for (UINT i = 0; i < nSampleCount; i++)
			//{
			//	pcmSampleBuffer[i] = ((float)pShorts[i]) / 32768.f;
			//}

			//delete[] pShorts;
			//LocalFree(WaveSample.pSampleData);

			//sampleLoaded = true;
            WaveSample.pSampleData = (char*)malloc(RiffChunk.dwLength);
            WaveSample.Size = RiffChunk.dwLength;

			inFile.read(WaveSample.pSampleData, RiffChunk.dwLength);

			uint32_t nSampleCount = (float)RiffChunk.dwLength / (float)(WaveSample.WaveFormatEx.wBitsPerSample / 8.0);
			sampleCount = nSampleCount;

			numChannels = WaveSample.WaveFormatEx.nChannels;
			sampleRate = WaveSample.WaveFormatEx.nSamplesPerSec;

			if (pcmSampleBuffer)
				delete[] pcmSampleBuffer;

			pcmSampleBuffer = new float[nSampleCount];
			int16_t* pShorts = new int16_t[nSampleCount];
			memset(pShorts, 0, nSampleCount * sizeof(int16_t));
			int m = 0;
			for (uint32_t i = 0; i < nSampleCount; i++)
			{
				// MSB
				pShorts[i] = (unsigned char)WaveSample.pSampleData[m + 1];
				pShorts[i] <<= 8;

				// LSB
				short lsb = (unsigned char)WaveSample.pSampleData[m];
				// in case top of lsb is bad
				lsb &= 0x00FF;
				pShorts[i] |= lsb;
				m += 2;
			}

			// convet to float -1.0 -> +1.0
			for (uint32_t i = 0; i < nSampleCount; i++)
			{
				pcmSampleBuffer[i] = ((float)pShorts[i]) / 32768.f;
			}

			delete[] pShorts;
            sampleLoaded = true;
            free(WaveSample.pSampleData);
		}

		// --- 24 bits
		else if (!bFailed && WaveSample.WaveFormatEx.wBitsPerSample == 24)
		{
            WaveSample.pSampleData = (char*)malloc(RiffChunk.dwLength);
			WaveSample.Size = RiffChunk.dwLength;

			inFile.read(WaveSample.pSampleData, RiffChunk.dwLength);

            uint32_t nSampleCount = (float)RiffChunk.dwLength / (float)(WaveSample.WaveFormatEx.wBitsPerSample / 8.0);
			sampleCount = nSampleCount;

			numChannels = WaveSample.WaveFormatEx.nChannels;
			sampleRate = WaveSample.WaveFormatEx.nSamplesPerSec;

			if (pcmSampleBuffer)
				delete[] pcmSampleBuffer;

			// our buffer gets created
			pcmSampleBuffer = new float[nSampleCount];

			int32_t* pSignedInts = new int32_t[nSampleCount];
			memset(pSignedInts, 0, nSampleCount * sizeof(int32_t));

            int m = 0;
            int32_t mask = 0x000000FF;

			// 24-bits in 3-byte packs
			if (WaveSample.WaveFormatEx.nBlockAlign / WaveSample.WaveFormatEx.nChannels == 3)
			{
				for (uint32_t i = 0; i < nSampleCount; i++)
				{
					// MSB
					pSignedInts[i] = (unsigned char)WaveSample.pSampleData[m + 2];
					pSignedInts[i] <<= 24;

					// NSB
					int nsb = (int)WaveSample.pSampleData[m + 1];
					// in case top of nsb is bad
					nsb &= mask;
					nsb <<= 16;
					pSignedInts[i] |= nsb;

					// LSB
					int lsb = (int)WaveSample.pSampleData[m];
					// in case top of lsb is bad
					lsb &= mask;
					lsb <<= 8;
					pSignedInts[i] |= lsb;

					m += 3;
				}

				// --- 24-bits in 4-byte packs
				if (WaveSample.WaveFormatEx.nBlockAlign / WaveSample.WaveFormatEx.nChannels == 4)
				{
					for (uint32_t i = 0; i < nSampleCount; i++)
					{
						// MSB
						pSignedInts[i] = (unsigned char)WaveSample.pSampleData[m + 3];
						pSignedInts[i] <<= 24;

						// NSB
						int nsb = (int)WaveSample.pSampleData[m + 2];
						// in case top of nsb is bad
						nsb &= mask;
						nsb <<= 16;
						pSignedInts[i] |= nsb;

						// NSB2
						int nsb2 = (int)WaveSample.pSampleData[m + 1];
						// in case top of nsb is bad
						nsb2 &= mask;
						nsb2 <<= 8;
						pSignedInts[i] |= nsb2;

						// LSB
						int lsb = (int)WaveSample.pSampleData[m];
						// in case top of lsb is bad
						lsb &= mask;
						pSignedInts[i] |= lsb;

						m += 4;
					}
				}

				// convet to float -1.0 -> +1.0
				for (uint32_t i = 0; i < nSampleCount; i++)
				{
					pcmSampleBuffer[i] = ((float)pSignedInts[i]) / 2147483648.0; // 2147483648.0 = 1/2 of 2^32
				}

				delete[] pSignedInts;
                sampleLoaded = true;
                free(WaveSample.pSampleData);
			}
		}
		// --- 32 bits
		else if (!bFailed && WaveSample.WaveFormatEx.wBitsPerSample == 32)
		{
            WaveSample.pSampleData = (char*)malloc(RiffChunk.dwLength);
			WaveSample.Size = RiffChunk.dwLength;

			inFile.read(WaveSample.pSampleData, RiffChunk.dwLength);

            uint32_t nSampleCount = (float)RiffChunk.dwLength / (float)(WaveSample.WaveFormatEx.wBitsPerSample / 8.0);
			sampleCount = nSampleCount;

			numChannels = WaveSample.WaveFormatEx.nChannels;
			sampleRate = WaveSample.WaveFormatEx.nSamplesPerSec;

			if (pcmSampleBuffer)
				delete[] pcmSampleBuffer;

			// our buffer gets created
			pcmSampleBuffer = new float[nSampleCount];

			if (WaveSample.WaveFormatEx.wFormatTag == 1)
			{
				int32_t* pSignedInts = new int32_t[nSampleCount];
				memset(pSignedInts, 0, nSampleCount * sizeof(int32_t));

                int m = 0;
                int32_t mask = 0x000000FF;

				for (uint32_t i = 0; i < nSampleCount; i++)
				{
					// MSB
					pSignedInts[i] = (unsigned char)WaveSample.pSampleData[m + 3];
					pSignedInts[i] <<= 24;

					// NSB
					int nsb = (int)WaveSample.pSampleData[m + 2];
					// in case top of nsb is bad
					nsb &= mask;
					nsb <<= 16;
					pSignedInts[i] |= nsb;

					// NSB2
					int nsb2 = (int)WaveSample.pSampleData[m + 1];
					// in case top of nsb is bad
					nsb2 &= mask;
					nsb2 <<= 8;
					pSignedInts[i] |= nsb2;

					// LSB
					int lsb = (int)WaveSample.pSampleData[m];
					// in case top of lsb is bad
					lsb &= mask;
					pSignedInts[i] |= lsb;

					m += 4;
				}

				// convet to float -1.0 -> +1.0
				for (uint32_t i = 0; i < nSampleCount; i++)
				{
					pcmSampleBuffer[i] = ((float)pSignedInts[i]) / 2147483648.0; // 2147483648.0 = 1/2 of 2^32
				}

				delete[] pSignedInts;
				sampleLoaded = true;
			}
			else if (WaveSample.WaveFormatEx.wFormatTag == 3) // float
			{
				uint32_t* pUSignedInts = new uint32_t[nSampleCount];
				memset(pUSignedInts, 0, nSampleCount * sizeof(uint32_t));

                uint32_t m = 0;
                uint32_t mask = 0x000000FF;

				for (uint32_t i = 0; i < nSampleCount; i++)
				{
					// MSB
					pUSignedInts[i] = (unsigned char)WaveSample.pSampleData[m + 3];
					pUSignedInts[i] <<= 24;

					// NSB
					int nsb = (unsigned int)WaveSample.pSampleData[m + 2];
					// in case top of nsb is bad
					nsb &= mask;
					nsb <<= 16;
					pUSignedInts[i] |= nsb;

					// NSB2
					int nsb2 = (unsigned int)WaveSample.pSampleData[m + 1];
					// in case top of nsb is bad
					nsb2 &= mask;
					nsb2 <<= 8;
					pUSignedInts[i] |= nsb2;

					// LSB
					int lsb = (unsigned int)WaveSample.pSampleData[m];
					// in case top of lsb is bad
					lsb &= mask;
					pUSignedInts[i] |= lsb;

					m += 4;
				}

				// Use the Union trick to re-use same memory location as two different data types
				//
				// see: http://www.cplusplus.com/doc/tutorial/other_data_types/#union
				UWaveData wd;
				for (uint32_t i = 0; i < nSampleCount; i++)
				{
					// save uint version
					wd.u = pUSignedInts[i];
					pcmSampleBuffer[i] = wd.f;
				}

				delete [] pUSignedInts;
                sampleLoaded = true;
                free(WaveSample.pSampleData);
			}
		}
		// --- 64 bits
		else if (!bFailed && WaveSample.WaveFormatEx.wBitsPerSample == 64)
		{
            WaveSample.pSampleData = (char*)malloc(RiffChunk.dwLength);
			WaveSample.Size = RiffChunk.dwLength;

			inFile.read(WaveSample.pSampleData, RiffChunk.dwLength);

            uint32_t nSampleCount = (float)RiffChunk.dwLength / (float)(WaveSample.WaveFormatEx.wBitsPerSample / 8.0);
			sampleCount = nSampleCount;

			numChannels = WaveSample.WaveFormatEx.nChannels;
			sampleRate = WaveSample.WaveFormatEx.nSamplesPerSec;

			if (pcmSampleBuffer)
				delete[] pcmSampleBuffer;

			// our buffer gets created
			pcmSampleBuffer = new float[nSampleCount];

			// floating point only
			if (WaveSample.WaveFormatEx.wFormatTag == 3) // float
			{
				unsigned long long* pUSignedLongLongs = new unsigned long long[nSampleCount];
				memset(pUSignedLongLongs, 0, nSampleCount * sizeof(unsigned long long));

				int m = 0;
				unsigned long long mask = 0x00000000000000FF;

				for (uint32_t i = 0; i < nSampleCount; i++)
				{
					// MSB
					pUSignedLongLongs[i] = (unsigned char)WaveSample.pSampleData[m + 7];
					pUSignedLongLongs[i] <<= 56;

					// NSB
					unsigned long long nsb = (unsigned long long)WaveSample.pSampleData[m + 6];
					// in case top of nsb is bad
					nsb &= mask;
					nsb <<= 48;
					pUSignedLongLongs[i] |= nsb;

					// NSB2
					unsigned long long nsb2 = (unsigned long long)WaveSample.pSampleData[m + 5];
					// in case top of nsb is bad
					nsb2 &= mask;
					nsb2 <<= 40;
					pUSignedLongLongs[i] |= nsb2;

					// NSB3
					unsigned long long nsb3 = (unsigned long long)WaveSample.pSampleData[m + 4];
					// in case top of nsb is bad
					nsb3 &= mask;
					nsb3 <<= 32;
					pUSignedLongLongs[i] |= nsb3;

					// NSB4
					unsigned long long nsb4 = (unsigned long long)WaveSample.pSampleData[m + 3];
					// in case top of nsb is bad
					nsb4 &= mask;
					nsb4 <<= 24;
					pUSignedLongLongs[i] |= nsb4;

					// NSB5
					unsigned long long nsb5 = (unsigned long long)WaveSample.pSampleData[m + 2];
					// in case top of nsb is bad
					nsb5 &= mask;
					nsb5 <<= 16;
					pUSignedLongLongs[i] |= nsb5;

					// NSB6
					unsigned long long nsb6 = (unsigned long long)WaveSample.pSampleData[m + 1];
					// in case top of nsb is bad
					nsb6 &= mask;
					nsb6 <<= 8;
					pUSignedLongLongs[i] |= nsb6;

					// LSB
					unsigned long long lsb = (unsigned long long)WaveSample.pSampleData[m];
					// in case top of lsb is bad
					lsb &= mask;
					pUSignedLongLongs[i] |= lsb;

					m += 8;
				}

				// Use the Union trick to re-use same memory location as two different data types
				//
				// see: http://www.cplusplus.com/doc/tutorial/other_data_types/#union
				UWaveData wd;
				for (uint32_t i = 0; i < nSampleCount; i++)
				{
					wd.u64 = pUSignedLongLongs[i];
					pcmSampleBuffer[i] = (float)wd.d; // cast the union's double as a float to chop off bottom
				}

				delete[] pUSignedLongLongs;
                sampleLoaded = true;
                free(WaveSample.pSampleData);
            }
		}

		// --- now find the loops, MIDI note info, etc... (smpl)
		int dInc = 0;
		do {
			int64_t position = inFile.tellg();
			if (position < 0)
				bFailed = true;
			else
			{
				inFile.seekg(position + dInc);
				bFailed = inFile.fail();
			}

			if (!bFailed)
			{
				// --- read chunk, looking for 'smpl'
				inFile.read((char*)(&RiffChunk), sizeof(RiffChunk));
				dInc = RiffChunk.dwLength;
				memcpy(szIdentifier, RiffChunk.IdentifierString, 4);
			}
		} while (strcmp(szIdentifier, "smpl") && !bFailed);

		// Found a smpl chunk
		/* smpl CHUNK format
		 Offset	Size	Description			Value
		 0x00	4	Chunk ID			"smpl" (0x736D706C)
		 0x04	4	Chunk Data Size		36 + (Num Sample Loops * 24) + Sampler Data
		 0x08	4	Manufacturer		0 - 0xFFFFFFFF	<------ // SKIPPING THIS //
		 0x0C	4	Product				0 - 0xFFFFFFFF	<------ // SKIPPING THIS //
		 0x10	4	Sample Period		0 - 0xFFFFFFFF	<------ // SKIPPING THIS (already know it)//
		 0x14	4	MIDI Unity Note		0 - 127
		 0x18	4	MIDI Pitch Fraction	0 - 0xFFFFFFFF
		 0x1C	4	SMPTE Format		0, 24, 25, 29, 30
		 0x20	4	SMPTE Offset		0 - 0xFFFFFFFF
		 0x24	4	Num Sample Loops	0 - 0xFFFFFFFF
		 0x28	4	Sampler Data		0 - 0xFFFFFFFF
		 0x2C
		 List of Sample Loops ------------------ //

		 MIDI Unity Note
		 The MIDI unity note value has the same meaning as the instrument chunk's MIDI Unshifted Note field which
		 specifies the musical note at which the sample will be played at it's original sample rate
		 (the sample rate specified in the format chunk).

		 MIDI Pitch Fraction
		 The MIDI pitch fraction specifies the fraction of a semitone up from the specified MIDI unity note field.
		 A value of 0x80000000 means 1/2 semitone (50 cents) and a value of 0x00000000 means no fine tuning
		 between semitones.

		 SMPTE Format
		 The SMPTE format specifies the Society of Motion Pictures and Television E time format used in the
		 following SMPTE Offset field. If a value of 0 is set, SMPTE Offset should also be set to 0.
		 Value	SMPTE Format
		 0	no SMPTE offset
		 24	24 frames per second
		 25	25 frames per second
		 29	30 frames per second with frame dropping (30 drop)
		 30	30 frames per second

		 SMPTE Offset
		 The SMPTE Offset value specifies the time offset to be used for the synchronization / calibration
		 to the first sample in the waveform. This value uses a format of 0xhhmmssff where hh is a signed value
		 that specifies the number of hours (-23 to 23), mm is an unsigned value that specifies the
		 number of minutes (0 to 59), ss is an unsigned value that specifies the number of seconds
		 (0 to 59) and ff is an unsigned value that specifies the number of frames (0 to -1).

		 // Sample Loop Data Struct
		 Offset	Size	Description		Value
		 0x00	4	Cue Point ID	0 - 0xFFFFFFFF
		 0x04	4	Type			0 - 0xFFFFFFFF
		 0x08	4	Start			0 - 0xFFFFFFFF
		 0x0C	4	End				0 - 0xFFFFFFFF
		 0x10	4	Fraction		0 - 0xFFFFFFFF
		 0x14	4	Play Count		0 - 0xFFFFFFFF

		 Loop type:
		 Value	Loop Type
		 0	Loop forward (normal)
		 1	Alternating loop (forward/backward, also known as Ping Pong)
		 2	Loop backward (reverse)
		 3 - 31	Reserved for future standard types
		 32 - 0xFFFFFFFF	Sampler specific types (defined by manufacturer)*/

		 // skipping some stuff; could add later
		if (!bFailed)
		{
			dInc = 12; // 3 don't cares 4 bytes each
			loopType = 0;
			loopCount = 0;
			loopStartIndex = 0;
			loopEndIndex = 0;
			unityMIDINote = 0;
			unityMIDIPitchFraction = 0;
			smpteFormat = 0;
			smpteOffset = 0;

			// found a loop set; currently only taking the FIRST loop set
			// only need one loop set for potentially sustaining waves
            int64_t position = inFile.tellg();
			inFile.seekg(position + dInc);

			// --- MIDI Note Number
			inFile.read((char*)(&unityMIDINote), 4);

			// --- unityMIDIPitchFraction
			inFile.read((char*)(&unityMIDIPitchFraction), 4);

			// --- smpteFormat
			inFile.read((char*)(&smpteFormat), 4);

			// --- smpteOffset
			inFile.read((char*)(&smpteOffset), 4);

			// --- MIDI Note Number
			inFile.read((char*)(&loopCount), 4);

			// --- skip cuepoint & sampledata
			position = inFile.tellg();
			inFile.seekg(position + 8);

			// --- loopType
			inFile.read((char*)(&loopType), 4);

			// --- loop start sample
			inFile.read((char*)(&loopStartIndex), 4);
			loopStartIndex *= numChannels;

			// --- loop end sample
			inFile.read((char*)(&loopEndIndex), 4);
			loopEndIndex *= numChannels;
		}
		else  // no loops
		{
			loopType = 0;
			loopCount = 0;
			loopStartIndex = 0;
			loopEndIndex = 0;
			unityMIDINote = 0;
			unityMIDIPitchFraction = 0;
			smpteFormat = 0;
			smpteOffset = 0;
		}

		// --- close the file
		inFile.close();

		// --- hope we got one loaded!
		return sampleLoaded;
	}


	/** @buildNoteTables
	\brief
	Sets up the tables for trying to suss out the MIDI note number from a filename that attempts
	to include the note name/number in its filename.
	*/
	void WaveFolder::buildNoteTables()
	{
		static const char* notesSharps[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		static const char* notesFlats[] = { "C", "DB", "D", "EB", "E", "F", "GB", "G", "AB", "A", "BB", "B" };
		static const char* nums[] = { "-1", "0", "1", "2", "3", "4", "5", "6", "7", "8" };

		uint32_t noteNum = 0;
		uint32_t numNum = 0;

		for (uint32_t i = 0; i < 10; i++)
		{
			for (uint32_t j = 0; j < 12; j++)
			{
				std::string nnS = concatStrings(notesSharps[j], nums[numNum]);
				noteTableSharps[noteNum] = nnS;

				std::string nnF = concatStrings(notesFlats[j], nums[numNum]);
				noteTableFlats[noteNum] = nnF;

				noteNum++;
			}
			numNum++;
		}
	}

	/** @findNoteNumberInName
	\brief
	Try to figure out the MIDI note number from the WAV filename.

	\param filename the WAV filename
	\param shiftUpOctave set true to offset by one octave; this is due to the two
	different MIDI note numbering systems still in use that are off by one octave

	\return the MIDI note number, if found or -1 if not found
	*/
	int32_t WaveFolder::findNoteNumberInName(const char* filename, bool shiftUpOctave)
	{
		int32_t notFound = -1; // -1 = not found

		std::string filenamestr(filename);
		for_each(filenamestr.begin(), filenamestr.end(), convertUpper());

		for (uint32_t i = 0; i < 120; i++)
		{
			std::size_t found = filenamestr.find(noteTableSharps[i]);
			if (found != std::string::npos)
			{
				if(shiftUpOctave && int32_t(i) + 12 < 128)
					return int32_t(i) + 12;

				return int32_t(i);
			}

			found = filenamestr.find(noteTableFlats[i]);
			if (found != std::string::npos)
			{
				if (shiftUpOctave && int32_t(i) + 12 < 128)
					return int32_t(i) + 12;

				return int32_t(i);
			}
		}

		return notFound;
	}

	/** @addNextFileToMap
	\brief
	Adds information about a WAV file in the folder to a map that is later used to parse
	the files in succession.

	\param fileFolderPath the WAV *folder* path
	\param fileName the WAV filename
	\param aubioSlices set to TRUE if these are aubio slice files
	\param wavFilePaths map to add information to
	\param fileCount return value of the number of files found
	*/
    void WaveFolder::addNextFileToMap(std::string fileFolderPath, std::string fileName,
        bool aubioSlices, std::map<int, std::string>* wavFilePaths, int& fileCount)
    {
        // make the path
        std::string filePath = fileFolderPath;
        filePath.append(fileName);

        if (aubioSlices)
        {
            //fileName
            std::string filenamestr(fileName);
            for_each(filenamestr.begin(), filenamestr.end(), convertUpper());

            std::string toErase(".WAV");
            size_t pos = filenamestr.find(toErase);
            if (pos != std::string::npos)
            {
                // If found then erase it from string
                filenamestr.erase(pos, toErase.length());
            }

            std::string folder(waveFolderName);
            for_each(folder.begin(), folder.end(), convertUpper());
            // const char* fff = folder.c_str();

            folder.append("_");

            pos = filenamestr.find(folder);
            if (pos != std::string::npos)
            {
                // If found then erase it from string
                filenamestr.erase(pos, folder.length());
            }

            toErase.assign(".");
            pos = filenamestr.find(toErase);
            if (pos != std::string::npos)
            {
                // If found then erase it from string
                filenamestr.erase(pos, toErase.length());
            }

            // --- filename is just a number now
            // const char* ppp = filenamestr.c_str();

            std::stringstream convert(filenamestr);
            int strAsInt = 0;
            convert >> strAsInt;

            wavFilePaths->insert(std::make_pair(strAsInt, filePath));
            fileCount++;
        }
        else
        {
            wavFilePaths->insert(std::make_pair(fileCount++, filePath));
        }
    }

	/** @parseFolder
	\brief
	The main function that opens a folder, creates the WAV information map, and then parses
	the files in succession after that. This uses some platform independent code for
	iterating through folders and files.

	\param sampleSet the return variable, an array of pointers to loaded PCM samples
	\param pitchlessLoops set TRUE if you know the folder has pitchless loops like drum loops
	\param aubioSlices set to TRUE if these are aubio slice files

	\return the number of WAV files successfully opened and parsed
	*/
	uint32_t WaveFolder::parseFolder(PCMSample** sampleSet, bool pitchlessLoops, bool aubioSlices)
	{
		uint32_t count = 0;
		std::map<int, std::string> wavFilePaths;
		int fileCount = 0;
		std::string fileFolderPath = waveFolderPath;

#if defined _WINDOWS || defined _WINDLL
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		HANDLE hFirstFind;

		std::string folderWildcard = waveFolderPath;
		folderWildcard.append("\\*.wav");

		// find first file
		hFirstFind = FindFirstFileEx(folderWildcard.c_str(), FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);

		if (hFirstFind == INVALID_HANDLE_VALUE)
			return false;

		// save
		hFind = hFirstFind;

		// add first file
		std::string fileName = FindFileData.cFileName;

		fileFolderPath.append("/");

		bool bWorking = true;
		while (bWorking)
		{
			addNextFileToMap(fileFolderPath, FindFileData.cFileName, aubioSlices, &wavFilePaths, fileCount);

			// find the next one and do it again until no more .wav files
			bWorking = FindNextFile(hFind, &FindFileData);
		}

		// close the finder
		FindClose(hFirstFind);
#else
		// --- iterate through the wave files in the folder
		CFStringRef path = CFStringCreateWithCString(NULL, waveFolderPath, kCFStringEncodingASCII);
		CFURLRef pathURL = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, true);
		CFURLEnumeratorRef enumerator = CFURLEnumeratorCreateForDirectoryURL(NULL, pathURL, kCFURLEnumeratorSkipInvisibles, NULL);
		CFURLRef childURL;
		CFURLEnumeratorResult enumeratorResult;

		do // iterate
		{
			enumeratorResult = CFURLEnumeratorGetNextURL(enumerator, &childURL, NULL);
			if (enumeratorResult == kCFURLEnumeratorSuccess)
			{
				// --- convert to char*
				CFStringRef fileString = CFURLCopyLastPathComponent(childURL);
				CFStringEncoding encodingMethod = CFStringGetSystemEncoding(); // CFStringGetFastestEncoding()
                char buffer[512];
                const char* fileName = CFStringGetCStringPtr(fileString, encodingMethod);
                if (fileName == NULL)
                {
                    if (CFStringGetCString(fileString, buffer, 512, encodingMethod))
                        fileName = buffer;
                }

                if(fileName)
                {
                    // call sub-function to add the files
                    std::string fileFolderPath = waveFolderPath;
                    fileFolderPath.append("/");
                    addNextFileToMap(fileFolderPath, fileName, aubioSlices, &wavFilePaths, fileCount);
                }
			}
			else if (enumeratorResult == kCFURLEnumeratorError)
			{
				return false;
			}
		} while (enumeratorResult != kCFURLEnumeratorEnd);

#endif
		// --- setup aubio mapping and decode filnames
		//
		// --- THIS is where you can change how the aubio slices are mapped
		//
		// --- here, I am mapping them to the C-major scale white keys, starting at middle C
		uint32_t aubioNote = 60;
		static uint32_t majorscale[] = { 2, 2, 1, 2, 2, 2, 1 };
		uint32_t stepCount = 0;

		// --- iterate through filenames and extract WAV data guts
		const std::string folder = fileFolderPath;
		std::map<int, std::string>::iterator it = wavFilePaths.begin();
		while (it != wavFilePaths.end())
		{
			std::string path = it->second;
			std::string file = it->second;

			// --- extract filename, in case MIDI note is encoded as a string
			eraseSubStr(file, folder);
			const char* fileString = file.c_str();

			// --- Here is where the sample is ultimately created
			PCMSample* sample = new PCMSample();
			if (!sample) return 0;

			sample->loadPCMSample(path.c_str());

			// --- add to sample set
			if (sample->isSampleLoaded())
			{
				if (aubioSlices)
				{
					sample->setLoopStartIndex(0);

					if (sample->getNumChannels() == 1)
						sample->setLoopEndIndex(sample->getSampleCount());
					else
						sample->setLoopEndIndex(sample->getSampleCount() - 1);

					sample->setLoopCount(1);
					sample->setPitchless(true);

					if (aubioNote < 128)
						sampleSet[aubioNote] = sample;

					aubioNote += majorscale[stepCount];
					stepCount++;
					if (stepCount == 7)
						stepCount = 0;

					count++;
				}
				else if (pitchlessLoops)
				{
					sample->setLoopStartIndex(0);

					if (sample->getNumChannels() == 1)
						sample->setLoopEndIndex(sample->getSampleCount());
					else
						sample->setLoopEndIndex(sample->getSampleCount() - 1);

					sample->setLoopCount(1);
					sample->setPitchless(true);

					int32_t noteNum = findNoteNumberInName(fileString);
					if (noteNum >= 0)
					{
						sampleSet[noteNum] = sample;
						count++;
					}
					else if (count < 128)
					{
						sampleSet[count] = sample;
						count++;
					}
				}
				// NOTE: if the file has NO MIDI UNITY note, this value will be 0,
				//       even though that is a legal MIDI note number;
				//       it is an unsigned int right out of the file
				else if (sample->getUnityMIDINote() > 0)
				{
					sampleSet[sample->getUnityMIDINote()] = sample;
					count++;
				}
				else
				{
					int32_t noteNum = findNoteNumberInName(fileString);
					if (noteNum >= 0)
					{
						sample->setUnityMIDINote(noteNum);
						sampleSet[noteNum] = sample;
						count++;
					}
					else if (count < 128)
					{
						sampleSet[count] = sample;
						count++;
					}
				}
			}
			else // file could not be parsed (could be compressed, or other non-supported format)
				delete sample;

			it++;
		}

		int nLastIndex = -1;
		PCMSample* lastSample = nullptr;

		if (!aubioSlices)
		{
			// now comb the array and replicate pointers
			for (int i = 0; i < 127; i++)
			{
				if (sampleSet[i])
				{
					nLastIndex = i;
					lastSample = sampleSet[i];
				}
			}

			if (!lastSample)
				return false;// no samples : (

			// upper part first
			for (int i = 127; i >= nLastIndex; i--)
				sampleSet[i] = lastSample;

			int index = nLastIndex - 1; // first index already has value in it
			while (index >= 0)
			{
				if (!sampleSet[index])
					sampleSet[index] = lastSample;
				else
					lastSample = sampleSet[index];

				index--;
			}
		}

		return count;
	}

}//  namespace
