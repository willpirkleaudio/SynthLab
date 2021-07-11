#include "synthlabdm.h"

#include <fstream>
#include <iomanip>

#if defined _WINDOWS || defined _WINDLL
    #include <windows.h>
    #include <mmsystem.h>
#else
    #import <CoreFoundation/CoreFoundation.h>
#endif

// --------------------------------------
//	--- OPTIONAL SynthLab SDK File --- // 
//  -------------------------------------
/**
\file   synthlabdm.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------


/**
\brief
Dynamic deleter module for the shared pointers to use. 

\param core pointer to a core; OK if null which can happen if the DLL loading fails.

*/
inline void DeleteDLLModule(SynthLab::ModuleCore* core)
{
    if(!core) return;
	void* handle = core->getModuleHandle();
	delete core;
	if(handle) ModuleGetter::unLoadSynthDll(handle);
}

/**
\brief
The destructor clears out the shared pointers, that delete themselves just afterwards.
- in a proper implementation, this will be the last object that owns a shared pointer
just prior to self-destruction

*/
DynamicModuleManager::~DynamicModuleManager()
{
	modules.clear();
	loadableModules.clear();
}

/**
\brief
Sets the DM configuration booleans from the config.txt file; 
if the file does not exist, the default config is used
*/
void DynamicModuleManager::setConfigData(std::string fileline, SynthLab::DMConfig& config)
{
	std::size_t found = fileline.find("kDMBuild");
	if (found != std::string::npos)
	{
		std::size_t foundarg = fileline.find("true");
		if (foundarg != std::string::npos)
			config.dm_build = true;
		else
			config.dm_build = false;
		return;
	}
	
	found = fileline.find("kDualMonoFilters");
	if (found != std::string::npos)
	{
		std::size_t foundarg = fileline.find("true");
		if (foundarg != std::string::npos)
			config.dual_mono_filters = true;
		else
			config.dual_mono_filters = false;
		return;
	}

	found = fileline.find("kHalfSampleSet");
	if (found != std::string::npos)
	{
		std::size_t foundarg = fileline.find("true");
		if (foundarg != std::string::npos)
			config.half_sample_set = true;
		else
			config.half_sample_set = false;
		return;
	}

	found = fileline.find("kReduceUnisonVoices");
	if (found != std::string::npos)
	{
		std::size_t foundarg = fileline.find("true");
		if (foundarg != std::string::npos)
			config.reduced_unison_count = true;
		else
			config.reduced_unison_count = false;
		return;
	}

	found = fileline.find("kAnalogFGNFilters");
	if (found != std::string::npos)
	{
		std::size_t foundarg = fileline.find("true");
		if (foundarg != std::string::npos)
			config.analog_fgn_filters = true;
		else
			config.analog_fgn_filters = false;
		return;
	}

	found = fileline.find("kParamSmoothing");
	if (found != std::string::npos)
	{
		std::size_t foundarg = fileline.find("true");
		if (foundarg != std::string::npos)
			config.parameterSmoothing = true;
		else
			config.parameterSmoothing = false;
		return;
	}
}

/**
\brief
Loads the DM configuration from the config.txt file; if the file does not exist, the default config is used
*/
bool DynamicModuleManager::loadDMConfig(std::string folderPath, SynthLab::DMConfig& config)
{
	folderPath.append("/config.txt");
	
	std::ifstream inFile;
	inFile.open(folderPath.c_str(), std::ifstream::in);

	// --- OK?
	bool bIsOpen = inFile.is_open();
	if (!bIsOpen)
	{
		config.dm_build = false;
		config.dual_mono_filters = false;
		config.half_sample_set = false;
		config.reduced_unison_count = false;
		return false;
	}

	std::string str;
	while (std::getline(inFile, str))
	{
		setConfigData(str, config);
	}
	
	inFile.close();

	return true;
}


/**
\brief
Opens a folder and loads all of the modules it finds in succession.
- uses the loadableModules list as a mechanism for deciding if an object
should be loaded. This is mainly for the different oscillator objects.
- there are two parts to the function for Windows and MacOS
- ultimately calls the loadDynamicModule( ) function that does the DLL load
- this function can be called more than once to append the list of modules
but that is not done in SynthLab

\ param folderPath the fully qualified path to the folder

\return the number of dynamic modules that were loaded
*/
uint32_t DynamicModuleManager::loadAllDynamicModulesInFolder(std::string folderPath)
{
	uint32_t count = 0;
	modules.clear();

#if defined _WINDOWS || defined _WINDLL
	folderPath.append("/windows");
#else
	folderPath.append("/macos");
#endif

	std::string subFolderPath = folderPath;
	subFolderPath.append("/oscillators");
	count += loadAllDynamicModulesInSubFolder(subFolderPath);

	subFolderPath = folderPath;
	subFolderPath.append("/filters");
	count += loadAllDynamicModulesInSubFolder(subFolderPath);

	subFolderPath = folderPath;
	subFolderPath.append("/lfos");
	count += loadAllDynamicModulesInSubFolder(subFolderPath);

	subFolderPath = folderPath;
	subFolderPath.append("/egs");
	count += loadAllDynamicModulesInSubFolder(subFolderPath);

	return count;
}

/**
\brief
Opens a folder and loads all of the modules it finds in succession. 
- uses the loadableModules list as a mechanism for deciding if an object
should be loaded. This is mainly for the different oscillator objects.
- there are two parts to the function for Windows and MacOS
- ultimately calls the loadDynamicModule( ) function that does the DLL load
- this function can be called more than once to append the list of modules
but that is not done in SynthLab

\ param folderPath the fully qualified path to the folder

\return the number of dynamic modules that were loaded
*/
uint32_t DynamicModuleManager::loadAllDynamicModulesInSubFolder(std::string folderPath)
{
	uint32_t count = 0;

#if defined _WINDOWS || defined _WINDLL
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	HANDLE hFirstFind;

	std::string folderWildcard = folderPath;
	folderWildcard.append("\\*.dll");

	// find first file
	hFirstFind = FindFirstFileEx(folderWildcard.c_str(), FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);

	if (hFirstFind == INVALID_HANDLE_VALUE)
		return false;

	// save
	hFind = hFirstFind;

	// add first file
	std::string fileFolderPath = folderPath;
	std::string fileName = FindFileData.cFileName;

	fileFolderPath.append("/");
	int fileCount = 0;
	bool bWorking = true;
	while (bWorking)
	{
		// --- make the path
		std::string filePath = fileFolderPath;
		filePath.append(FindFileData.cFileName);
		const char* fff = FindFileData.cFileName;
		const char* ccc = filePath.c_str();

		uint32_t modCount = loadDynamicModule(filePath.c_str());
		count += modCount;
	
		// find the next one and do it again until no more .wav files
		bWorking = FindNextFile(hFind, &FindFileData);
	}

	// close the finder
	FindClose(hFirstFind);

#else
    // --- iterate through the module cores
    CFStringRef path = CFStringCreateWithCString(NULL, folderPath.c_str(), kCFStringEncodingASCII);
    CFURLRef pathURL = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, true);
    CFURLEnumeratorRef enumerator = CFURLEnumeratorCreateForDirectoryURL(NULL, pathURL, kCFURLEnumeratorSkipInvisibles, NULL);
    CFURLRef childURL;
    CFURLEnumeratorResult enumeratorResult;

    do // iterate
    {
        enumeratorResult = CFURLEnumeratorGetNextURL(enumerator, &childURL, NULL);
        if (enumeratorResult == kCFURLEnumeratorSuccess)
        {
            // --- convert to char*
            CFStringRef pathString = CFURLCopyPath(childURL);
            CFStringRef fileString = CFURLCopyLastPathComponent(childURL);
            CFStringEncoding encodingMethod = CFStringGetSystemEncoding(); 
            char buffer[1024];
            const char* fileName = CFStringGetCStringPtr(fileString, encodingMethod);
            
            // --- CFStringGetCStringPtr can and will arbitrarily return NULL so...
            if (fileName == NULL)
            {
                if (CFStringGetCString(fileString, buffer, 1024, encodingMethod))
                    fileName = buffer;
            }
            if(fileName)
            {
                char buffer[1024];
                const char* filePath = CFStringGetCStringPtr(pathString, encodingMethod);
                // --- CFStringGetCStringPtr can and will arbitrarily return NULL so...
                if (filePath == NULL)
                {
                    if (CFStringGetCString(pathString, buffer, 1024, encodingMethod))
                        filePath = buffer;
                }

                if (loadDynamicModule(filePath))
                    count++;
            }
        }
        else if (enumeratorResult == kCFURLEnumeratorError)
        {
            return false;
        }
    } while (enumeratorResult != kCFURLEnumeratorEnd);
#endif
    
	return count;
}

/**
\brief
Tries to load a DLL or dylib
- loads the module
- them, checks the loadableModules list to make sure this is a legal module for the synth
- if loadable it stores the pointer
- if not loadable, it simply returns; the smart pointer will delete itself when
the function returns

\ param modulePath the fully qualified path to the DLL or dylib file

\return number of modules loaded, or 0 if not loaded
*/
uint32_t DynamicModuleManager::loadDynamicModule(std::string modulePath)
{
	uint32_t count = 0;

    std::shared_ptr<SynthLab::ModuleCore> module(ModuleGetter::loadSynthDll(modulePath), DeleteDLLModule);
	if (module)
	{
		uint32_t moduleType = module->getModuleType();
		if (std::find(loadableModules.begin(), loadableModules.end(), moduleType) != loadableModules.end())
		{
			// --- push first module, then get duplicates
			modules.push_back(module);
			count++;

			// --- get duplicates depending on count for voice
			uint32_t modCount = getModuleCountForType(moduleType) - 1; // -1 because we already got first one
			for (uint32_t i = 0; i < modCount; i++)
			{
				std::shared_ptr<SynthLab::ModuleCore> module(ModuleGetter::loadSynthDll(modulePath), DeleteDLLModule);
				modules.push_back(module);
				count++;
			}
		}
	}
	return count;
}

uint32_t DynamicModuleManager::getModuleCountForType(uint32_t type)
{
	if (type == SynthLab::LFO_MODULE)
		return NUM_LFO_MODULE;
	else if (type == SynthLab::EG_MODULE)
		return NUM_EG_MODULE;
	else if (type == SynthLab::DCA_MODULE)
		return NUM_DCA_MODULE;
	else if (type == SynthLab::FILTER_MODULE)
		return NUM_FILTER_MODULE;
	else if (type == SynthLab::WTO_MODULE)
	{
		if (doubleOscillatorSet)
			return 2 * NUM_WTO_MODULE;
		else
			return NUM_WTO_MODULE;
	}
	else if (type == SynthLab::VAO_MODULE)
		return NUM_VAO_MODULE;
	else if (type == SynthLab::FMO_MODULE)
		return NUM_FMO_MODULE;
	else if (type == SynthLab::PCMO_MODULE)
		return NUM_PCMO_MODULE;
	else if (type == SynthLab::KSO_MODULE)
		return NUM_KSO_MODULE;
	else if (type == SynthLab::OSC_MODULE)
		return NUM_OSC_MODULE;

	return 0; // not found
}
