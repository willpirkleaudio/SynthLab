#ifndef _squaremorph0
#define _squaremorph0

#include "sqrduty_0.h"
#include "sqrduty_1.h"
#include "sqrduty_2.h"
#include "sqrduty_3.h"
#include "sqrduty_4.h"
#include "sqrduty_5.h"
#include "sqrduty_6.h"
#include "sqrduty_7.h"

const unsigned int SquareMorph0_TablePtrsCount = 8;

static SynthLab::SynthLabTableSet* SquareMorph0_TablePtrs[SquareMorph0_TablePtrsCount] = { &sqrduty_0_TableSet, &sqrduty_1_TableSet, &sqrduty_2_TableSet, &sqrduty_3_TableSet, &sqrduty_4_TableSet, &sqrduty_5_TableSet, &sqrduty_6_TableSet, &sqrduty_7_TableSet };

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string SquareMorph0_TableNames[SquareMorph0_TablePtrsCount] = {
	"sqrduty 0",
	"sqrduty 1",
	"sqrduty 2",
	"sqrduty 3",
	"sqrduty 4",
	"sqrduty 5",
	"sqrduty 6",
	"sqrduty 7" };

// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource sqrduty_StaticTables[SquareMorph0_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it.
static SynthLab::SynthLabBankSet sqrduty_BankDescriptor(SquareMorph0_TablePtrsCount, &SquareMorph0_TablePtrs[0], &SquareMorph0_TableNames[0], &sqrduty_StaticTables[0]);

#endif // definer
