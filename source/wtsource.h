#ifndef _WTSource_h
#define _WTSource_h

#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   wsoscillator.cpp
\author Will Pirkle
\brief See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	// --- constants
	const uint32_t MAX_WAVE_TABLES = 128;

	// -------------------------------------------------
	class WavetableSource : public IWavetableSource
	{
	public:
		WavetableSource() {
			// --- clear the array of 128 table-pointers
			memset(wavetableSet, 0, NUM_MIDI_NOTES*(sizeof(double*)));
		}

		// --- clean up
		~WavetableSource() { }

		virtual const char* getWaveformName()
		{
			return selectedTable.waveformName;
		}

		// --- set the current table for note event
		inline virtual void selectTable(uint32_t midiNoteNumber)
		{
			selectedTable = wavetableSet[midiNoteNumber];
		}

		// --- read and interpolate: could add lagrange here
		inline virtual double readWaveTable(double oscClockIndex)
		{
			// --- two samples from table
			double wtData[2] = { 0.0, 0.0 };

			// --- location = N(fo/fs)
			double wtReadLocation = selectedTable.tableLength * oscClockIndex;

			// --- split the fractional index into int.frac parts
			double dIntPart = 0.0;
			double fracPart = modf(wtReadLocation, &dIntPart);
			uint32_t readIndex = (uint32_t)dIntPart;
			uint32_t nextReadIndex = (readIndex + 1) & selectedTable.wrapMask;

			// --- two table reads
			wtData[0] = uint64ToDouble(selectedTable.table[readIndex]);
			wtData[1] = uint64ToDouble(selectedTable.table[nextReadIndex]);

			// --- interpolate the output
			double output = doLinearInterpolation(0.0, 1.0, wtData[0], wtData[1], fracPart);

			// --- scale as needed
			return selectedTable.outputComp * output;
		}

		// --- get len
		virtual uint32_t getWaveTableLength() { return selectedTable.tableLength; }

		// --- for init with HiResWTSet in a .h file
		inline void addWavetable(Wavetable* table, uint32_t midiNoteNumber)
		{
			//wavetableSet[midiNoteNumber] = table;
		}

	protected:
		// --- 128 wavetables
		Wavetable wavetableSet[NUM_MIDI_NOTES];
		Wavetable selectedTable;
	};

}
#endif // definer

