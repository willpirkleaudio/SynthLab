#ifndef _DynSource_h
#define _DynSource_h

#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   dynamictablesource.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class DynamicTableSource
	\ingroup SynthObjects
	\brief
	Storage for one dynamic table source; a wavetable that is created dynamically at load time,
	rather than being loaded from a static resource or table array. 
	- exposes the IWavetableSource interface
	- stores 128 DynamicWavetable objects, one for each MIDI note
	- stores a selected table
	- the owning object (a wavetable core) selects the table based on pitch during the update() phase, 
	then makes calls to read the table during the render() phase 
	- see also DynamicWavetable

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class DynamicTableSource : public IWavetableSource
	{
	public:
		DynamicTableSource() { }///< empty constructor
		~DynamicTableSource() { }///< empty destructor

		/** 
		\return name of selected string as const char*
		*/
		virtual const char* getWaveformName() override
		{
			return selectedTable.waveformName;
		}

		/**
		\brief
		Select a table based on MIDI note number
		*/
		inline virtual void selectTable(uint32_t midiNoteNumber) override
		{
			if (midiNoteNumber >= NUM_MIDI_NOTES) return;
			selectedTable = wavetableSet[midiNoteNumber];
		}

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
			double wtReadLocation = selectedTable.tableLength * normalizedPhaseInc;

			// --- split the fractional index into int.frac parts
			double dIntPart = 0.0;
			double fracPart = modf(wtReadLocation, &dIntPart);
			uint32_t readIndex = (uint32_t)dIntPart;
			uint32_t nextReadIndex = (readIndex + 1) & selectedTable.wrapMask;

			// --- two table reads
			wtData[0] = selectedTable.table.get()[readIndex];
			wtData[1] = selectedTable.table.get()[nextReadIndex];

			// --- interpolate the output
			double output = doLinearInterpolation(0.0, 1.0, wtData[0], wtData[1], fracPart);

			// --- scale as needed
			return selectedTable.outputComp * output;
		}

		/**
		\return the length of the selected wavetable
		*/
		virtual uint32_t getWaveTableLength() override { return selectedTable.tableLength; }


		/**
		\brief
		Adds a new wavetable or tables to the array of 128 tables, one for each MIDI note
		- one table may be used to cover multiple MIDI notes
		- if a single table is used for a single MIDI note, set the start and end numbers identically

		\param startNoteNumber the MIDI note number of the first note to add the table
		\param endNoteNumber the MIDI note number of the last note to add the table
		\param _table the shared pointer to the table (array)
		\param length the size of the table (array)
		\param name the unique name of this table
		*/
		inline void addWavetable(uint32_t startNoteNumber, uint32_t endNoteNumber, 
			std::shared_ptr<double> _table, uint32_t length, const char* name)
		{
			if (endNoteNumber >= NUM_MIDI_NOTES) return;

			DynamicWavetable wt(_table, length, name);

			for (uint32_t i = startNoteNumber; i <= endNoteNumber; i++)
			{
				wavetableSet[i] = wt;
			}

			selectedTable = wavetableSet[endNoteNumber];
		}

		/**
		\brief
		Clear out the wavetables to initialize or re-initialize
		*/
		inline void clearAllWavetables()
		{
			for (uint32_t i = 0; i < NUM_MIDI_NOTES; i++)
			{
				if (wavetableSet->table)
					wavetableSet->table = nullptr;
			}
		}

	protected:
		// --- prefab table valid for all MIDI notes
		DynamicWavetable wavetableSet[NUM_MIDI_NOTES];///<--- prefab table valid for all MIDI notes
		DynamicWavetable selectedTable;///<--- selected table, stored here
	};

}
#endif // definer

