
#if defined _WINDOWS || defined _WINDLL
    #include <windows.h>
#else
// --- dylib
    #include <dlfcn.h>
    #include <corefoundation/CFBundle.h>
#endif

#include "modulegetter.h"

// --------------------------------------
//	--- OPTIONAL SynthLab SDK File --- // 
//  -------------------------------------
/**
\file   modulegetter.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------

#if defined _WIN32 || defined _WIN64

/**
\brief
Loads a Windows DLL.
- opens the DLL
- queries for creation function
- uses returned function pointer to create module

\param moduleName the name of the file, e.g. biquadfilters.dll

\returns a pointer to the object if sucessful, nullptr otherwise
*/
SynthLab::ModuleCore* ModuleGetter::loadSynthDll(std::string moduleName)
{
	HINSTANCE userModuleHandle;
	typedef SynthLab::ModuleCore* (*pSigProcModule)();
	pSigProcModule pDLL;

	userModuleHandle = LoadLibrary(LPCTSTR(moduleName.c_str()));
	if (userModuleHandle != nullptr)
	{
		pDLL = (pSigProcModule)GetProcAddress(userModuleHandle, "createModuleCore");
		if (pDLL)
		{
			SynthLab::ModuleCore* core = (pDLL)();
			if(core) core->setModuleHandle(userModuleHandle);
			return core;
		}
	}
	return nullptr;
}

/**
\brief
Unload a Windows DLL.

\param moduleHandle the cached handle that was returned when the object
was created.

\returns true if successful
*/
bool ModuleGetter::unLoadSynthDll(void* moduleHandle)
{
	return FreeLibrary((HMODULE)moduleHandle);
}

#else

/**
\brief
Loads a MacOS dylib.
- opens the dylib
- queries for creation function
- uses returned function pointer to create module

\param moduleName the name of the file, e.g. biquadfilters.dylib

\returns a pointer to the object if sucessful, nullptr otherwise
*/
SynthLab::ModuleCore* ModuleGetter::loadSynthDll(std::string moduleName)
{
     SynthLab::ModuleCore* core = nullptr;
     void* dllHandle = dlopen(moduleName.c_str(), RTLD_GLOBAL);
     if(dllHandle)
     {
         typedef SynthLab::ModuleCore* (*pSigProcModule)();
         pSigProcModule pDLL;
                                            
         pDLL = (pSigProcModule)dlsym (dllHandle, "createModuleCore");
         if(!pDLL) return nullptr;
    
         core = (pDLL)();
         if(core) core->setModuleHandle(dllHandle);
     }
    
    return core;
}

/**
\brief
Unload a MacOS dylib.

\param moduleHandle the cached handle that was returned when the object
was created.

\returns true if successful
*/
bool ModuleGetter::unLoadSynthDll(void* moduleHandle)
{
	int success = dlclose(moduleHandle);
	if (success == 0)
		return true;
	return false;
}

#endif

