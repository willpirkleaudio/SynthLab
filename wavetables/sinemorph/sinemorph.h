#ifndef _sinemorph
#define _sinemorph

#include "sinemorph_0.h"
#include "sinemorph_1.h"
#include "sinemorph_2.h"
#include "sinemorph_3.h"
#include "sinemorph_4.h"
#include "sinemorph_5.h"
#include "sinemorph_6.h"
#include "sinemorph_7.h"
#include "sinemorph_8.h"
#include "sinemorph_9.h"
#include "sinemorph_10.h"
#include "sinemorph_11.h"
#include "sinemorph_12.h"
#include "sinemorph_13.h"
#include "sinemorph_14.h"
#include "sinemorph_15.h"

const unsigned int SineMorph_TablePtrsCount = 16;

static SynthLab::SynthLabTableSet* SineMorph_TablePtrs[SineMorph_TablePtrsCount] = { &SineMorph_0_TableSet, &SineMorph_1_TableSet, &SineMorph_2_TableSet, &SineMorph_3_TableSet, &SineMorph_4_TableSet, &SineMorph_5_TableSet, &SineMorph_6_TableSet, &SineMorph_7_TableSet, &SineMorph_8_TableSet, &SineMorph_9_TableSet, &SineMorph_10_TableSet, &SineMorph_11_TableSet, &SineMorph_12_TableSet, &SineMorph_13_TableSet, &SineMorph_14_TableSet, &SineMorph_15_TableSet };

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string SineMorph_TableNames[SineMorph_TablePtrsCount] = {
	"SineMorph 0",
	"SineMorph 1",
	"SineMorph 2",
	"SineMorph 3",
	"SineMorph 4",
	"SineMorph 5",
	"SineMorph 6",
	"SineMorph 7",
	"SineMorph 8",
	"SineMorph 9",
	"SineMorph 10",
	"SineMorph 11",
	"SineMorph 12",
	"SineMorph 13",
	"SineMorph 14",
	"SineMorph 15" };

// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource SineMorph_StaticTables[SineMorph_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it.
static SynthLab::SynthLabBankSet SineMorph_BankDescriptor(SineMorph_TablePtrsCount, &SineMorph_TablePtrs[0], &SineMorph_TableNames[0], &SineMorph_StaticTables[0]);

#endif // definer
