#ifndef _squarecombmorph
#define _squarecombmorph

#include "sqrcomb_0.h"
#include "sqrcomb_1.h"
#include "sqrcomb_2.h"
#include "sqrcomb_3.h"
#include "sqrcomb_4.h"
#include "sqrcomb_5.h"
#include "sqrcomb_6.h"
#include "sqrcomb_7.h"
#include "sqrcomb_8.h"
#include "sqrcomb_9.h"
#include "sqrcomb_10.h"
#include "sqrcomb_11.h"
#include "sqrcomb_12.h"
#include "sqrcomb_13.h"
#include "sqrcomb_14.h"
#include "sqrcomb_15.h"

const unsigned int SquareCombMorph_TablePtrsCount = 16;

static SynthLab::SynthLabTableSet* SquareCombMorph_TablePtrs[SquareCombMorph_TablePtrsCount] = { &sqrcomb_0_TableSet, &sqrcomb_1_TableSet, &sqrcomb_2_TableSet, &sqrcomb_3_TableSet, &sqrcomb_4_TableSet, &sqrcomb_5_TableSet, &sqrcomb_6_TableSet, &sqrcomb_7_TableSet, &sqrcomb_8_TableSet, &sqrcomb_9_TableSet, &sqrcomb_10_TableSet, &sqrcomb_11_TableSet, &sqrcomb_12_TableSet, &sqrcomb_13_TableSet, &sqrcomb_14_TableSet, &sqrcomb_15_TableSet };

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string SquareCombMorph_TableNames[SquareCombMorph_TablePtrsCount] = {
	"sqrcomb 0",
	"sqrcomb 1",
	"sqrcomb 2",
	"sqrcomb 3",
	"sqrcomb 4",
	"sqrcomb 5",
	"sqrcomb 6",
	"sqrcomb 7",
	"sqrcomb 8",
	"sqrcomb 9",
	"sqrcomb 10",
	"sqrcomb 11",
	"sqrcomb 12",
	"sqrcomb 13",
	"sqrcomb 14",
	"sqrcomb 15" };

// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource squarecomb_StaticTables[SquareCombMorph_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it.
static SynthLab::SynthLabBankSet squarecomb_BankDescriptor(SquareCombMorph_TablePtrsCount, &SquareCombMorph_TablePtrs[0], &SquareCombMorph_TableNames[0], &squarecomb_StaticTables[0]);

#endif // definer
