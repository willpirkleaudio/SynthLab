#ifndef _digdoo1
#define _digdoo1

#include "digdoo1_0.h"
#include "digdoo1_1.h"
#include "digdoo1_2.h"
#include "digdoo1_3.h"
#include "digdoo1_4.h"
#include "digdoo1_5.h"
#include "digdoo1_6.h"
#include "digdoo1_7.h"
#include "digdoo1_8.h"
#include "digdoo1_9.h"
#include "digdoo1_10.h"
#include "digdoo1_11.h"
#include "digdoo1_12.h"
#include "digdoo1_13.h"
#include "digdoo1_14.h"
#include "digdoo1_15.h"

const unsigned int DigDoo1_TablePtrsCount = 16;

static SynthLab::SynthLabTableSet* DigDoo1_TablePtrs[DigDoo1_TablePtrsCount] = { &digdoo1_0_TableSet, &digdoo1_1_TableSet, &digdoo1_2_TableSet, &digdoo1_3_TableSet, &digdoo1_4_TableSet, &digdoo1_5_TableSet, &digdoo1_6_TableSet, &digdoo1_7_TableSet, &digdoo1_8_TableSet, &digdoo1_9_TableSet, &digdoo1_10_TableSet, &digdoo1_11_TableSet, &digdoo1_12_TableSet, &digdoo1_13_TableSet, &digdoo1_14_TableSet, &digdoo1_15_TableSet }; 

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string DigDoo1_TableNames[DigDoo1_TablePtrsCount] = { 
	"digdoo1 0",
	"digdoo1 1",
	"digdoo1 2",
	"digdoo1 3",
	"digdoo1 4",
	"digdoo1 5",
	"digdoo1 6",
	"digdoo1 7",
	"digdoo1 8",
	"digdoo1 9",
	"digdoo1 10",
	"digdoo1 11",
	"digdoo1 12",
	"digdoo1 13",
	"digdoo1 14",
	"digdoo1 15" }; 

// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource DigDoo1_StaticTables[DigDoo1_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it. 
static SynthLab::SynthLabBankSet DigDoo1_BankDescriptor(DigDoo1_TablePtrsCount, &DigDoo1_TablePtrs[0], &DigDoo1_TableNames[0], &DigDoo1_StaticTables[0]);

#endif // definer
