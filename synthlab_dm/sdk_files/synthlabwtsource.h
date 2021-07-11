#ifndef _SynthLabWTSource_h
#define _SynthLabWTSource_h

#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthlabwtsource.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class StaticTableSource
	\ingroup SynthObjects
	\brief
	Storage for one static table source; a static table is pre-compiled into the synth, or (optionally)
	read from a file. The "source" stores a set of these tables to maximize frequency content while
	prohibiting aliasing. 
	- exposes the IWavetavleSource interface
	- stores 128 StaticWavetable objects, one for each MIDI note
	- stores a selected table
	- the owning object (a wavetable core) selects the table based on pitch during the update() phase,
	then makes calls to read the table during the render() phase
	- see also StaticWavetable

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class StaticTableSource : public IWavetableSource
	{
	public:
		StaticTableSource() { }///< empty constructor
		~StaticTableSource() { }///< empty destructor

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
			//double dIntPart = 0.0;
			//double fracPart = modf(wtReadLocation, &dIntPart);
			uint32_t readIndex = (uint32_t)wtReadLocation;
			uint32_t nextReadIndex = (readIndex + 1) & selectedTable.wrapMask;

			// --- two table reads
			uint64_t u0 = selectedTable.uTable[readIndex];
			uint64_t u1 = selectedTable.uTable[nextReadIndex];
			wtData[0] = *(reinterpret_cast<double*>(&u0));
			wtData[1] = *(reinterpret_cast<double*>(&u1));

			// --- interp
			double fracPart = wtReadLocation - readIndex;
			double output = doLinearInterpolation(wtData[0], wtData[1], fracPart);

			// --- scale as needed
			return selectedTable.outputComp * output;
		}

		/**
		\return the length of the selected wavetable
		*/
		virtual uint32_t getWaveTableLength() override { return selectedTable.tableLength; }

		/**
		\brief
		Adds a new SET of wavetables to the array of 128 tables, one for each MIDI note
		- uses the SynthLabTableSet to encode the set of pointers to tables
		- tables are stored in .h files and compiled into product
		- you can change this to add tables in whatever format you like

		\param synthLabTableSet a set of wavetables that have been setup to maximize frequency content
		with zero aliasing
		*/
		inline void addSynthLabTableSet(SynthLabTableSet* synthLabTableSet)
		{
			for (uint32_t i = 0; i < NUM_MIDI_NOTES; i++)
			{
				StaticWavetable wt(synthLabTableSet->ppHexTableSet[i],
									 synthLabTableSet->tableLengths[i],
									 synthLabTableSet->waveformName,
									 synthLabTableSet->outputComp, 
									 synthLabTableSet->tableFs);

				wavetableSet[i] = wt;
			}
			// --- set a default table
			selectedTable = wavetableSet[MIDI_NOTE_A4]; 
		}

	protected:
		// --- 128 wavetables
		StaticWavetable wavetableSet[NUM_MIDI_NOTES];///<--- prefab table valid for all MIDI notes
		StaticWavetable selectedTable;///<--- selected table, stored here
	};

	/**
	\class DrumWTSource
	\ingroup SynthObjects
	\brief
	Storage for one static table source, specifically for drums which are pitchless and one-shot
	- exposes the IWavetableSource interface
	- the owning object (a wavetable core) selects the table based on MIDI note number during the update() phase,
	then makes calls to read the table during the render() phase
	- see also StaticWavetable

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class DrumWTSource : public IWavetableSource
	{
	public:
		DrumWTSource() { }///< empty constructor
		~DrumWTSource() { }///< empty destructor

		/**
		\return name of selected string as const char*
		*/
		virtual const char* getWaveformName() override
		{
			return drumTable.waveformName;
		}

		/**
		\brief
		Select a table based on MIDI note number; nothing to do here
		*/
		inline virtual void selectTable(uint32_t midiNoteNumber)  override { }
	
		/**
		\brief
		Read and interpolate the table; uses linear interpolation but could be changed to
		4th order LaGrange interpolation instead

		\param normalizedPhaseInc the phase increment value; usually this is the mcounter
		member of a SynthClock object and is fo/fs where fo is the desired oscillator frequency
		*/
		inline virtual double readWaveTable(double normalizedPhaseInc) override
		{
			// --- core must calculate correct oscClocking (its easy)
			double wtReadLocation = drumTable.tableLength * normalizedPhaseInc;
			double output = 0.0;

			if (drumTable.dTable)
				output = drumTable.dTable[(uint32_t)wtReadLocation];
			else
			{
				// --- two table reads
				uint64_t u0 = drumTable.uTable[(uint32_t)wtReadLocation];
				output = *(reinterpret_cast<double*>(&u0));
			}

			// --- scale as needed
			return drumTable.outputComp * output;
		}

		/**
		\return the length of the selected wavetable
		*/
		virtual uint32_t getWaveTableLength() override { return drumTable.tableLength; }

		/**
		\brief
		Adds a new wavetable to the array of 128 tables, one for each MIDI note
		- this function uses pointers to double arrays

		\param _table the shared pointer to the table (array)
		\param length the size of the table (array)
		\param name the unique name of this table
		\param outputComp a scaling factor used on playback to amplify or attenuate the output
		*/
		inline void addWavetable(const double* _table, uint32_t length, const char* name, double outputComp = 1.0)
		{
			// --- create the static table
			StaticWavetable wt(_table, length, name, outputComp);
			drumTable = wt;
		}

		/**
		\brief
		Adds a new wavetable to the array of 128 tables, one for each MIDI note
		- this function uses pointers to unsinged 64-bit int arrays

		\param _table the shared pointer to the table (array)
		\param length the size of the table (array)
		\param name the unique name of this table
		\param outputComp a scaling factor used on playback to amplify or attenuate the output
		*/
		inline void addWavetable(const uint64_t* _table, uint32_t length, const char* name, double outputComp = 1.0)
		{
			// --- create the static table
			StaticWavetable wt(_table, length, name, outputComp);
			drumTable = wt;
		}

	protected:
		// --- one table per source
		StaticWavetable drumTable; ///< one table per drum
	};


	/**
	\struct MorphBankData
	\ingroup SynthStructures
	\brief
	Information about a bank of wavetables that are used in the morphing wavetable core.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 02
	*/
	struct MorphBankData
	{
		std::string bankName = empty_string.c_str();	///< one name for bank
		uint32_t numTables = 1;							///< number of wavetables in bank (up to 16)
		std::string tableNames[MODULE_STRINGS];			///< names of wavetables
		int32_t tableIndexes[MODULE_STRINGS] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };			///< unique indexes for faster lookup
		StaticTableSource staticSources[MODULE_STRINGS];
	};
}
#endif // definer

