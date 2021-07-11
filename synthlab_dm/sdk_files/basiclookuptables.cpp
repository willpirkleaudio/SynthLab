#include "basiclookuptables.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   basiclookuptables.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief
	Construction:
	- creates new Hann window table
	- you can add more here

     */
	BasicLookupTables::BasicLookupTables()
	{
		hannTable.reset(new LookUpTable(DEFAULT_LUT_LENGTH));
		for (uint32_t n = 0; n < DEFAULT_LUT_LENGTH; n++)
		{
			hannTable->table[n] = (0.5 * (1.0 - cos((n*2.0*kPi) / (double)(DEFAULT_LUT_LENGTH - 1))));
		}
	}

	/**
	\brief
	Reads and interpolates a table using a pointer to the table

	\param table table pointer
	\param index read location, expected to be fractional so interpolation is used

	\returns the newly constructed object
	*/
	double BasicLookupTables::readTableByTablePointer(double* table, double index)
	{
		// --- get the read location
		double dIntPart = 0.0;
		double fracPart = modf(index, &dIntPart);
		uint32_t readIndex = (uint32_t)dIntPart;
		uint32_t nextReadIndex = readIndex + 1;
		nextReadIndex &= DEFAULT_LUT_WRAP_MASK;
		return  doLinearInterpolation(table[readIndex], table[nextReadIndex], fracPart);
	}

	/**
	\brief
	Reads and interpolates a table using a table index (codified enum)

	\param tableIndex zero-indexed enumeration
	\param index read location, expected to be fractional so interpolation is used

	\returns the newly constructed object
	*/
	double BasicLookupTables::readTableByTableIndex(uint32_t tableIndex, double index)
	{
		if (index >= DEFAULT_LUT_LENGTH) return 0.0;
		double* table = nullptr;
		switch (tableIndex)
		{
		case HANN_LUT:
		{
			table = hannTable->table;
			break;
		}
		default:
			return 0.0;
		}

		// --- do the interpolated read
		return readTableByTablePointer(table, index);
	}


} // namespace



