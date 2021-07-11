#include "synthlabdll.h"

// --- this is the file containing your ModuleCore object for DM exporting
//     this DLL support file must be in the same folder as the synthlabcore.h
//     file; this is mainly to avoid having to rename the module-core files
//     each time you want to start a new project
#include "synthlabcore.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthlabdll.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021

- http://www.willpirkle.com

*/
#if defined _WIN32 || defined _WIN64

void* moduleHandle;   // DLL module handle

extern "C"
{
	/**
	\brief 
	Windows entry point function for opening/loading DLL
	- stores handle for deletion later
	- not used for MacOS

	\param hInst the instance HANDLE for the DLL
	\param dwReason reason for function call, attach or detach
	\param lpvReserved is reserved and not used
	\returns TRUE if DLL is loaded
	*/
	BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID /*lpvReserved*/)
	{
		if (dwReason == DLL_PROCESS_ATTACH)
		{
			moduleHandle = hInst;
		}
		else if (dwReason == DLL_PROCESS_DETACH)
		{
			moduleHandle = nullptr;
		}

		return TRUE;
	}
}

/**
\brief
Creation function for DM server

\returns a new instance of the ModuleCore object
*/
DllExport SynthLab::ModuleCore* createModuleCore()
{
	SynthLab::ModuleCore* module = new SynthLab::SynthLabCore();
	return module;
}
#else
// --- MacOS version
SynthLab::ModuleCore* createModuleCore()
{
	SynthLab::ModuleCore* module = new SynthLab::SynthLabCore();
	return module;
}
#endif









