#ifndef _SynthLabSMPLSource_h
#define _SynthLabSMPLSource_h

#include "synthfunctions.h"
#include "pcmsample.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthlabpcmsource.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	// --- constants
	//const uint32_t MAX_SAMPLES = 128;

	/**
	\class SynthLabPCMSource
	\ingroup SynthObjects
	\brief
	Storage for a set of PCM samples that constitute a patch or instrument
	- exposes the IPCMSampleSource interface
	- initialized with folders full of wave files that are multi-samples
	- the owning object (a wavetable core) selects the PCM sample based on pitch during the update() phase
	which is ignored since the source only has one table. Then, the owning object makes calls to read
	the sample during the render() phase
	- see also PCMSample

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class SynthLabPCMSource : public IPCMSampleSource
	{
	public:
		SynthLabPCMSource() {} // default

		/**
		\brief
		Parses a folder full of wave files and extracts the PCM samples
		
		\param _sampleFolderPath the fully qualified path to the sample folder
		\param _sampleFolderName the name of the sample folder; this will be the name of the bank of samples
		\param _sampleRate fs
		\param pitchlessLoops set true to make pitchless where phaseInc always equals 1.0
		\param aubioSlices set true if the folder holds wave files made with aubio

		*/
		SynthLabPCMSource(const char* _sampleFolderPath, const char* _sampleFolderName, double _sampleRate, 
			bool pitchlessLoops = false, bool aubioSlices = false)
		{
			// --- use initializer
			init(_sampleFolderPath, _sampleFolderName, _sampleRate, pitchlessLoops, aubioSlices);
		}

		/**
		\brief
		Separated initializer, parses a folder full of wave files and extracts the PCM samples

		\param _sampleFolderPath the fully qualified path to the sample folder
		\param _sampleFolderName the name of the sample folder; this will be the name of the bank of samples
		\param _sampleRate fs
		\param pitchlessLoops set true to make pitchless where phaseInc always equals 1.0
		\param aubioSlices set true if the folder holds wave files made with aubio
		*/
		void init(const char* _sampleFolderPath, const char* _sampleFolderName,
			double _sampleRate, bool pitchlessLoops = false, bool aubioSlices = false)
		{
			// --- sample directory is also name of sample set
			sampleFolderPath = _sampleFolderPath;
			sampleFolderName = _sampleFolderName;
			sampleRate = _sampleRate;

			for (uint32_t i = 0; i < NUM_MIDI_NOTES; i++)
			{
				sampleSet[i] = nullptr;
			}

			selectedSample = nullptr;

			// --- parse sample FILES
			WaveFolder waveFolder(sampleFolderPath.c_str(), sampleFolderName.c_str());
			waveFolder.parseFolder(&sampleSet[0], pitchlessLoops, aubioSlices);
		}

		/**
		\brief
		Delete all samples from memory.
		*/
		inline virtual void deleteSamples()
		{
			for (uint32_t i = 0; i < NUM_MIDI_NOTES; i++)
			{
				PCMSample* pSample = nullptr;
				PCMSample* pDeletedSample = nullptr;
				for (int i = 0; i<NUM_MIDI_NOTES; i++)
				{
					if (sampleSet[i])
					{
						pSample = sampleSet[i];
						if (pSample != pDeletedSample)
						{
							pDeletedSample = pSample;
							delete pSample;
						}
						sampleSet[i] = nullptr;
					}
				}
			}
		}

		/**
		\brief
		query for valid sample count (not used in SynthLab but avialable)
		*/
		inline virtual uint32_t getValidSampleCount()
		{
			uint32_t count = 0;
			for (int i = 0; i<NUM_MIDI_NOTES; i++)
			{
				if (sampleSet[i]) count++;
			}
			return count;
		}

		/**
		\brief
		Find all non-null sample pointers
		*/
		inline virtual bool haveValidSamples()
		{
			if (getValidSampleCount() > 0)
				return true;
			return false;
		}

		/**
		\brief
		Destructor nothing to do, samples were deleted prior to destruction here
		*/
		~SynthLabPCMSource(){ }

		/**
		\brief
		Selects a PCM sample based on the target oscillator frequency

		\param oscFrequency target frequency
		*/
		inline virtual double selectSample(double oscFrequency)
		{
			uint32_t midiNote = midiNoteNumberFromOscFrequency(oscFrequency);
			selectedSample = sampleSet[midiNote];

			if (!selectedSample) return 0.0;
			double inc = 0.0;

			if (selectedSample->isPitchless())
			{
				inc = 1.0;
			}
			else
			{
				// --- get unity note frequency
				double dUnityFrequency = midiNoteNumberToOscFrequency(selectedSample->getUnityMIDINote());
				
				// --- calculate increment
				inc = oscFrequency / dUnityFrequency;
			}

			return inc;
		}

		/**
		\brief
		Read and interpolate the table; uses linear interpolation but could be changed to
		4th order LaGrange interpolation instead
		- checks loop points and looping mode
		- for one shot will set the flag once event is done

		\param readIndex the floating point index (may be fractional); this value will be 
		changed during the function call

		\param inc the phase increment for this read operation; the readIndex will be automatically 
		bumped by this value 
		*/
		inline virtual PCMSampleOutput readSample(double& readIndex, double inc)
		{
			PCMSampleOutput output;

			if (!selectedSample)
				return output; // auto 0.0s

			if (selectedSample->getNumChannels() == 1)
				output.numActiveChannels = 1;
			else 
				output.numActiveChannels = 2;

			if (readIndex < 0)
				return output; // auto 0.0s

			double numChannels = (double)selectedSample->getNumChannels();

			if (selectedSample->getLoopCount() > 0)
			{
				// --- use loop points for looping
				if (loopMode == SampleLoopMode::sustain)
				{
					if (readIndex > (double)(selectedSample->getLoopEndIndex()) / numChannels)
						readIndex = readIndex - (double)(selectedSample->getLoopEndIndex()) / numChannels + (double)(selectedSample->getLoopStartIndex()) / numChannels;
				}
				// --- use loop points for looping
				else if (loopMode == SampleLoopMode::loop)
				{
					if (readIndex > (double)(selectedSample->getSampleCount() - numChannels - 1) / numChannels)
						readIndex = 0;
				}
			}
			else // there are no loops
			{
				if (loopMode == SampleLoopMode::sustain) // use end->start samples
				{
					if (readIndex > (double)(selectedSample->getSampleCount() - numChannels - 1) / numChannels)
					{
						readIndex = -1;
						return output;
					}
				}
				else if (loopMode == SampleLoopMode::oneShot) // use end->start samples
				{
					if (readIndex > (double)(selectedSample->getSampleCount() - numChannels - 1) / numChannels)
					{
						readIndex = -1;
						return output;
					}
				}
				else if (loopMode == SampleLoopMode::loop)
				{
					if (readIndex > (double)(selectedSample->getSampleCount() - numChannels - 1) / numChannels)
						readIndex = 0;
				}

			}

			// --- split the fractional index into int.frac parts
			double dIntPart = 0.0;
			double fracPart = modf(readIndex, &dIntPart);
			uint32_t nReadIndex = (uint32_t)dIntPart;

			// --- mono or stereo file? CURRENTLY ONLY SUPPORTING THESE 2
			if (selectedSample->getNumChannels() == 1)
			{
				int nReadIndexNext = nReadIndex + 1 > selectedSample->getSampleCount() - 1 ? 0 : nReadIndex + 1;

				// interpolate between the two
				output.audioOutput[LEFT_CHANNEL] = doLinearInterpolation(0, 1, selectedSample->getSampleBuffer()[nReadIndex], selectedSample->getSampleBuffer()[nReadIndexNext], fracPart);
				output.audioOutput[RIGHT_CHANNEL] = output.audioOutput[LEFT_CHANNEL];

				readIndex += inc;
			}
			else if (selectedSample->getNumChannels() == 2)
			{
				// --- interpolate across interleaved buffer!
				int nReadIndexLeft = (int)readIndex * 2;

				// --- setup second index for interpolation; wrap the buffer if needed, we know last sample is Right channel
				//     so reset to top (the 0 after ?)
				int nReadIndexNextLeft = nReadIndexLeft + 2 > selectedSample->getSampleCount() - 1 ? 0 : nReadIndexLeft + 2;

				// --- interpolate between the two
				output.audioOutput[LEFT_CHANNEL] = doLinearInterpolation(0, 1, selectedSample->getSampleBuffer()[nReadIndexLeft], selectedSample->getSampleBuffer()[nReadIndexNextLeft], fracPart);

				// --- do the right channel
				int nReadIndexRight = nReadIndexLeft + 1;

				// --- find the next one, skipping over, note wrap goes to index 1 ---> 1
				int nReadIndexNextRight = nReadIndexRight + 2 > selectedSample->getSampleCount() - 1 ? 1 : nReadIndexRight + 2;

				// --- interpolate between the two
				output.audioOutput[RIGHT_CHANNEL] = doLinearInterpolation(0, 1, selectedSample->getSampleBuffer()[nReadIndexRight], selectedSample->getSampleBuffer()[nReadIndexNextRight], fracPart);
			
				readIndex += inc;
			}
			return output;
		}

		/** set loop mode
		*/
		virtual void setSampleLoopMode(SampleLoopMode _loopMode) { loopMode = _loopMode; }

	protected:
		// --- 128 samples
		PCMSample* sampleSet[NUM_MIDI_NOTES];	///< one PCM sample pointer per note
		PCMSample* selectedSample = nullptr;				///< currently selected sample
		std::string sampleFolderPath;			///< folder of wave files
		std::string sampleFolderName;			///< name of patch
		double sampleRate = 0.0;				///< fs
		SampleLoopMode loopMode = SampleLoopMode::sustain; ///< different looping modes
	};

}
#endif // definer

