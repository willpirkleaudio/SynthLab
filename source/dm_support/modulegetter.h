#pragma once

#include "../synthbase.h"

// --------------------------------------
//	--- OPTIONAL SynthLab SDK File --- // 
//  -------------------------------------
/**
\file   modulegetter.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------

/**
\class ModuleGetter
\ingroup SynthLabDMObjects
\brief
Object for loading and unloading SyntLab-DM Dynamic Modules which are API-agnostic DLLs (Windows)
or dylibs (MacOS). 

- These use longstanding and very basic functions for DLL management; these functions are 
extremely well documented. 
- The load/unload functions are static and so may be invoked without an underlying object
declaration
- It is important to retain the module handle that is returned from the creation functions;
this handle is required to unload the module at destruction time. 


\author Will Pirkle http://www.willpirkle.com
\remark This object is included and described in further detail in
Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2021 / 04 / 26
*/
class ModuleGetter
{
public:
	/** empty constructor/destructor*/
	ModuleGetter() {}
	virtual ~ModuleGetter() {}

	/** two functions only: load and unload */
	static SynthLab::ModuleCore* loadSynthDll(std::string moduleName);
	static bool unLoadSynthDll(void* moduleHandle);
};

