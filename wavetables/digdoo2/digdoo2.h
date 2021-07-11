#ifndef _digdoo2
#define _digdoo2

#include "digdoo2_0.h"
#include "digdoo2_1.h"
#include "digdoo2_2.h"
#include "digdoo2_3.h"
#include "digdoo2_4.h"
#include "digdoo2_5.h"
#include "digdoo2_6.h"
#include "digdoo2_7.h"
#include "digdoo2_8.h"
#include "digdoo2_9.h"
#include "digdoo2_10.h"
#include "digdoo2_11.h"
#include "digdoo2_12.h"
#include "digdoo2_13.h"
#include "digdoo2_14.h"
#include "digdoo2_15.h"

const unsigned int DigDoo2_TablePtrsCount = 16;

static SynthLab::SynthLabTableSet* DigDoo2_TablePtrs[DigDoo2_TablePtrsCount] = { &digdoo2_0_TableSet, &digdoo2_1_TableSet, &digdoo2_2_TableSet, &digdoo2_3_TableSet, &digdoo2_4_TableSet, &digdoo2_5_TableSet, &digdoo2_6_TableSet, &digdoo2_7_TableSet, &digdoo2_8_TableSet, &digdoo2_9_TableSet, &digdoo2_10_TableSet, &digdoo2_11_TableSet, &digdoo2_12_TableSet, &digdoo2_13_TableSet, &digdoo2_14_TableSet, &digdoo2_15_TableSet }; 

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string DigDoo2_TableNames[DigDoo2_TablePtrsCount] = { 
	"digdoo2 0",
	"digdoo2 1",
	"digdoo2 2",
	"digdoo2 3",
	"digdoo2 4",
	"digdoo2 5",
	"digdoo2 6",
	"digdoo2 7",
	"digdoo2 8",
	"digdoo2 9",
	"digdoo2 10",
	"digdoo2 11",
	"digdoo2 12",
	"digdoo2 13",
	"digdoo2 14",
	"digdoo2 15" }; 

// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource DigDoo2_StaticTables[DigDoo2_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it. 
static SynthLab::SynthLabBankSet DigDoo2_BankDescriptor(DigDoo2_TablePtrsCount, &DigDoo2_TablePtrs[0], &DigDoo2_TableNames[0], &DigDoo2_StaticTables[0]);

#endif // definer
