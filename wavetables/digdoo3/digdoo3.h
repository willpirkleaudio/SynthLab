#ifndef _digdoo3
#define _digdoo3

#include "digdoo3_0.h"
#include "digdoo3_1.h"
#include "digdoo3_2.h"
#include "digdoo3_3.h"
#include "digdoo3_4.h"
#include "digdoo3_5.h"
#include "digdoo3_6.h"
#include "digdoo3_7.h"
#include "digdoo3_8.h"
#include "digdoo3_9.h"
#include "digdoo3_10.h"
#include "digdoo3_11.h"
#include "digdoo3_12.h"
#include "digdoo3_13.h"
#include "digdoo3_14.h"
#include "digdoo3_15.h"

const unsigned int DigDoo3_TablePtrsCount = 16;

static SynthLab::SynthLabTableSet* DigDoo3_TablePtrs[DigDoo3_TablePtrsCount] = { &digdoo3_0_TableSet, &digdoo3_1_TableSet, &digdoo3_2_TableSet, &digdoo3_3_TableSet, &digdoo3_4_TableSet, &digdoo3_5_TableSet, &digdoo3_6_TableSet, &digdoo3_7_TableSet, &digdoo3_8_TableSet, &digdoo3_9_TableSet, &digdoo3_10_TableSet, &digdoo3_11_TableSet, &digdoo3_12_TableSet, &digdoo3_13_TableSet, &digdoo3_14_TableSet, &digdoo3_15_TableSet }; 

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string DigDoo3_TableNames[DigDoo3_TablePtrsCount] = { 
	"digdoo3 0",
	"digdoo3 1",
	"digdoo3 2",
	"digdoo3 3",
	"digdoo3 4",
	"digdoo3 5",
	"digdoo3 6",
	"digdoo3 7",
	"digdoo3 8",
	"digdoo3 9",
	"digdoo3 10",
	"digdoo3 11",
	"digdoo3 12",
	"digdoo3 13",
	"digdoo3 14",
	"digdoo3 15" }; 

// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource DigDoo3_StaticTables[DigDoo3_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it. 
static SynthLab::SynthLabBankSet DigDoo3_BankDescriptor(DigDoo3_TablePtrsCount, &DigDoo3_TablePtrs[0], &DigDoo3_TableNames[0], &DigDoo3_StaticTables[0]);

#endif // definer
