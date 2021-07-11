#ifndef _primalwaves
#define _primalwaves

#include "primalwaves_0.h"
#include "primalwaves_1.h"
#include "primalwaves_2.h"
#include "primalwaves_3.h"

//#include "../../source/synthbase.h"

const unsigned int PrimalWaves_TablePtrsCount = 4;

static SynthLab::SynthLabTableSet* PrimalWaves_TablePtrs[PrimalWaves_TablePtrsCount] = { &PrimalWaves_1_TableSet, &PrimalWaves_3_TableSet, &PrimalWaves_2_TableSet, &PrimalWaves_0_TableSet };

// --- Define Wavform Names: Here is where you can override the table names all at once;
//     Remember to keep the names short and simple, refrain from the underscore, and keep the character count below 32 for compatibiltiy with brick files.
static std::string PrimalWaves_TableNames[PrimalWaves_TablePtrsCount] = {
	"DigiSaw",
	"DigiSine",
	"DigiSqr",
	"DigiTri" };

// --- set of static wavetable sources for these "burned in" tables; these are blank and will be filled in during load
static SynthLab::StaticTableSource PrimalWaves_StaticTables[PrimalWaves_TablePtrsCount];

// --- This is the bank descriptor; you can initialize the bank with it
static SynthLab::SynthLabBankSet PrimalWaves_BankDescriptor(PrimalWaves_TablePtrsCount, &PrimalWaves_TablePtrs[0], &PrimalWaves_TableNames[0], &PrimalWaves_StaticTables[0]);

#endif // definer

