#ifndef _sawdemon
#define _sawdemon

#include "sawdemon_0.h"
#include "sawdemon_1.h"
#include "sawdemon_2.h"
#include "sawdemon_3.h"
#include "sawdemon_4.h"
#include "sawdemon_5.h"
#include "sawdemon_6.h"
#include "sawdemon_7.h"

const unsigned int sawdemon_TablePtrsCount = 8;

static SynthLab::SynthLabTableSet* sawdemon_TablePtrs[sawdemon_TablePtrsCount] = { &sawdemon_0_TableSet, &sawdemon_1_TableSet, &sawdemon_2_TableSet, &sawdemon_3_TableSet, &sawdemon_4_TableSet, &sawdemon_5_TableSet, &sawdemon_6_TableSet, &sawdemon_7_TableSet }; 

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string sawdemon_TableNames[sawdemon_TablePtrsCount] = { 
	"sawdemon 0",
	"sawdemon 1",
	"sawdemon 2",
	"sawdemon 3",
	"sawdemon 4",
	"sawdemon 5",
	"sawdemon 6",
	"sawdemon 7" }; 

// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource sawdemon_StaticTables[sawdemon_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it. 
static SynthLab::SynthLabBankSet sawdemon_BankDescriptor(sawdemon_TablePtrsCount, &sawdemon_TablePtrs[0], &sawdemon_TableNames[0], &sawdemon_StaticTables[0]);

#endif // definer
