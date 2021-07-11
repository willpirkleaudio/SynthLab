// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthlabdll.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021

- http://www.willpirkle.com
*/

#define NOMINMAX

// *** NOTE: relative paths to SDK files ********************************************
//           these are setup so that ModuleCore DM projects are compiled directly
//           from the SDK itself with minimal effort
//
//           You may want to adjust these to suit your particular development folder
//           system if you only want to use parts of the SDK or plan on modifying
//           SDK files, or need to alter the folder hierarchy for your setup.
// **********************************************************************************
#include "../../../source/synthbase.h"

// --- Exported ModuleCore Creation Function
#if defined _WIN32 || defined _WIN64
#include <windows.h>

#define DllExport extern "C" __declspec(dllexport)
DllExport SynthLab::ModuleCore* createModuleCore();

#else

#define EXPORT __attribute__((visibility("default")))
EXPORT
extern "C" SynthLab::ModuleCore* createModuleCore();

#endif
