#pragma once

#include "../synthbase.h"
#include "../synthconstants.h"
#include "../synthfunctions.h"
#include "modulegetter.h"

// --------------------------------------
//	--- OPTIONAL SynthLab SDK File --- // 
//  -------------------------------------
/**
\file   synthlabdm.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/

// -----------------------------------------------------------------------------
// --- number of modules per voice
const uint32_t NUM_LFO_MODULE = 2;
const uint32_t NUM_EG_MODULE = 3;
const uint32_t NUM_DCA_MODULE = 1; /* not supported, but reserved for future*/
const uint32_t NUM_FILTER_MODULE = 2; 
const uint32_t NUM_WTO_MODULE = 4;
const uint32_t NUM_VAO_MODULE = 4;
const uint32_t NUM_FMO_MODULE = 4;
const uint32_t NUM_PCMO_MODULE = 4;
const uint32_t NUM_KSO_MODULE = 4;
const uint32_t NUM_OSC_MODULE = 4; /* general oscillator, not supported, but reserved for future*/

/**
\class DynamicModuleManager
\ingroup DynamicModuleObjects
\brief
This object maintains a set of SynthLab-DM Dynamic Modules. 
- creates the module from a DLL or dylib file
- maintains a vector of shared pointers to modules
- includes the module deleter function that is needed for the shared pointers
and that is called when the pointer's ref count hits zero; this function is NOT
a member function
- used in conjunction with the ModuleGetter object
- note: not namespaced for generalized use

\author Will Pirkle http://www.willpirkle.com
\remark This object is included and described in further detail in
Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2021 / 04 / 26
*/
class DynamicModuleManager
{
public:
	DynamicModuleManager() {} ///< empty constructor
	virtual ~DynamicModuleManager(); ///< virtual deleter

	/** loading functions*/
	bool loadDMConfig(std::string folderPath, SynthLab::DMConfig& config);
	void setConfigData(std::string fileline, SynthLab::DMConfig& config);
	uint32_t loadAllDynamicModulesInFolder(std::string folderPath);
	uint32_t loadAllDynamicModulesInSubFolder(std::string folderPath);
	uint32_t loadDynamicModule(std::string modulePath);
	void addLoadableModule(uint32_t module) { loadableModules.push_back(module); }

	/** query functions */
	bool haveDynamicModules() { return modules.size() > 0 ? true : false; }
	std::vector<std::shared_ptr<SynthLab::ModuleCore>> getDynamicModules() { return modules; }
	uint32_t getModuleCountForType(uint32_t type);
	void setDoubleOscillators(bool b) { doubleOscillatorSet = b; }

protected:
	std::vector<std::shared_ptr<SynthLab::ModuleCore>> modules; ///< set of pointers fo modules
	std::vector<uint32_t> loadableModules;	///< set of module types that may be loaded in this synth; see synthconstants.h e.g. LFO_MODULE
	bool doubleOscillatorSet = false;
};

