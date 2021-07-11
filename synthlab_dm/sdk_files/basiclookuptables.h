#pragma once
#ifndef __basiclookuptables_h__
#define  __basiclookuptables_h__

#include "synthbase.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   basiclookuptables.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class BasicLookupTables
	\ingroup SynthObjects
	\brief
	Very basic lookup table object
	- holds a smart pointer to a single LookupTable structure, Hann Window
	- you can add more tables and access functions as you like
	- provides multiple functions to access the lookup table with different
	lookup index types

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class BasicLookupTables
	{
	public:
		BasicLookupTables(); 
		~BasicLookupTables() {}
		double readTableByTablePointer(double* table, double index); 
		double readTableByTableIndex(uint32_t tableIndex, double index);
		inline double readTableByTableIndexNormalized(uint32_t table, double normalizedIndex) { return readTableByTableIndex(table, normalizedIndex*DEFAULT_LUT_LENGTH); } ///< read a table with enumerated table index
		inline double readHannTableWithNormIndex(double normalizedIndex) { return readTableByTablePointer(hannTable->table, normalizedIndex*DEFAULT_LUT_LENGTH); }///< read Hann table
		inline double readSineTableWithNormIndex(double normalizedIndex) { return readTableByTablePointer(&sin_1024[0], normalizedIndex*DEFAULT_LUT_LENGTH); }///<read sine table

	protected:
		// --- tables go here
		std::unique_ptr<LookUpTable> hannTable = nullptr;///< a single lookup table - you can add more tables here
	};


} // namespace

#endif