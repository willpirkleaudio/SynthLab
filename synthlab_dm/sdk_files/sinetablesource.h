#ifndef _SineSource_h
#define _SineSource_h

#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   sinetablesource.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{

	/**
	\class SineTableSource
	\ingroup SynthObjects
	\brief
	Storage for one static sinusoidal table source; stores a single sine table that 
	is used for all notes.
	- exposes the IWavetavleSource interface
	- simplest wavetable source example
	- the owning object (a wavetable core) selects the table based on pitch during the update() phase
	which is ignored since the source only has one table. Then, the owning object makes calls to read 
	the table during the render() phase
	- see also StaticWavetable

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class SineTableSource : public IWavetableSource
	{
	public:
		/** 
		\brief 
		Stores the information about the static sinusoidal wavetable (see synthconstants.h)
		- sets up the length and mask 
		*/
		SineTableSource() 
		{
			sineWavetable.dTable = &sinetable[0];
			sineWavetable.tableLength = sineTableLength;
			sineWavetable.wrapMask = sineTableLength - 1;
			sineWavetable.waveformName = "sinewave";
		}

		~SineTableSource() { } ///< emptu destructor

		/**
		\return name of selected string as const char*
		*/
		virtual const char* getWaveformName() override
		{
			return sineWavetable.waveformName;
		}

		/** 
		\brief
		Nothing to do for this object since there is only one table
		*/
		inline virtual void selectTable(uint32_t midiNoteNumber) override { }

		/**
		\brief
		Read and interpolate the table; uses linear interpolation but could be changed to
		4th order LaGrange interpolation instead

		\param normalizedPhaseInc the phase increment value; usually this is the mcounter
		member of a SynthClock object and is fo/fs where fo is the desired oscillator frequency
		*/
		inline virtual double readWaveTable(double normalizedPhaseInc) override
		{
			// --- two samples from table
			double wtData[2] = { 0.0, 0.0 };

			// --- location = N(fo/fs)
			double wtReadLocation = sineWavetable.tableLength * normalizedPhaseInc;

			// --- split the fractional index into int.frac parts
			double dIntPart = 0.0;
			double fracPart = modf(wtReadLocation, &dIntPart);
			uint32_t readIndex = (uint32_t)dIntPart;
			uint32_t nextReadIndex = (readIndex + 1) & sineWavetable.wrapMask;

			// --- two table reads
			wtData[0] = sineWavetable.dTable[readIndex];
			wtData[1] = sineWavetable.dTable[nextReadIndex];

			// --- interpolate the output
			double output = doLinearInterpolation(wtData[0], wtData[1], fracPart);

			// --- scale as needed
			return sineWavetable.outputComp * output;
		}

		/**
		\return the length of the selected wavetable
		*/
		virtual uint32_t getWaveTableLength()  override { return sineWavetable.tableLength; }

	protected:
		// --- prefab table valid for all MIDI notes
		StaticWavetable sineWavetable;///<// --- prefab table valid for all MIDI notes
	};

}
#endif // definer

