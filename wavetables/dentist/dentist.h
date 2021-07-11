#ifndef _dentist
#define _dentist

#include "dentist_0.h"
#include "dentist_1.h"
#include "dentist_2.h"
#include "dentist_3.h"
#include "dentist_4.h"
#include "dentist_5.h"
#include "dentist_6.h"
#include "dentist_7.h"
#include "dentist_8.h"
#include "dentist_9.h"
#include "dentist_10.h"
#include "dentist_11.h"
#include "dentist_12.h"
#include "dentist_13.h"
#include "dentist_14.h"
#include "dentist_15.h"

const unsigned int Dentist_TablePtrsCount = 16;

static SynthLab::SynthLabTableSet* Dentist_TablePtrs[Dentist_TablePtrsCount] = { &dentist_0_TableSet, &dentist_1_TableSet, &dentist_2_TableSet, &dentist_3_TableSet, &dentist_4_TableSet, &dentist_5_TableSet, &dentist_6_TableSet, &dentist_7_TableSet, &dentist_8_TableSet, &dentist_9_TableSet, &dentist_10_TableSet, &dentist_11_TableSet, &dentist_12_TableSet, &dentist_13_TableSet, &dentist_14_TableSet, &dentist_15_TableSet }; 

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string Dentist_TableNames[Dentist_TablePtrsCount] = { 
	"dentist 0",
	"dentist 1",
	"dentist 2",
	"dentist 3",
	"dentist 4",
	"dentist 5",
	"dentist 6",
	"dentist 7",
	"dentist 8",
	"dentist 9",
	"dentist 10",
	"dentist 11",
	"dentist 12",
	"dentist 13",
	"dentist 14",
	"dentist 15" }; 


// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource Dentist_StaticTables[Dentist_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it. 
static SynthLab::SynthLabBankSet Dentist_BankDescriptor(Dentist_TablePtrsCount, &Dentist_TablePtrs[0], &Dentist_TableNames[0], &Dentist_StaticTables[0]);

#endif // definer

