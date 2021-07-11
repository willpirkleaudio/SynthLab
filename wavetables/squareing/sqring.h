#ifndef _squareringmorph
#define _squareringmorph

#include "sqring_0.h"
#include "sqring_1.h"
#include "sqring_2.h"
#include "sqring_3.h"
#include "sqring_4.h"
#include "sqring_5.h"
#include "sqring_6.h"
#include "sqring_7.h"
#include "sqring_8.h"
#include "sqring_9.h"
#include "sqring_10.h"
#include "sqring_11.h"
#include "sqring_12.h"
#include "sqring_13.h"
#include "sqring_14.h"
#include "sqring_15.h"

const unsigned int SquareRingMorph_TablePtrsCount = 16;

static SynthLab::SynthLabTableSet* SquareRingMorph_TablePtrs[SquareRingMorph_TablePtrsCount] = { &sqring_0_TableSet, &sqring_1_TableSet, &sqring_2_TableSet, &sqring_3_TableSet, &sqring_4_TableSet, &sqring_5_TableSet, &sqring_6_TableSet, &sqring_7_TableSet, &sqring_8_TableSet, &sqring_9_TableSet, &sqring_10_TableSet, &sqring_11_TableSet, &sqring_12_TableSet, &sqring_13_TableSet, &sqring_14_TableSet, &sqring_15_TableSet };

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string SquareRingMorph_TableNames[SquareRingMorph_TablePtrsCount] = {
	"sqring 0",
	"sqring 1",
	"sqring 2",
	"sqring 3",
	"sqring 4",
	"sqring 5",
	"sqring 6",
	"sqring 7",
	"sqring 8",
	"sqring 9",
	"sqring 10",
	"sqring 11",
	"sqring 12",
	"sqring 13",
	"sqring 14",
	"sqring 15" };

// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource squarering_StaticTables[SquareRingMorph_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it.
static SynthLab::SynthLabBankSet squarering_BankDescriptor(SquareRingMorph_TablePtrsCount, &SquareRingMorph_TablePtrs[0], &SquareRingMorph_TableNames[0], &squarering_StaticTables[0]);

#endif // definer
