#include "wtoscillator.h"
#include "classicwtcore.h"
#include "morphwtcore.h"
#include "sfxwtcore.h"
#include "drumwtcore.h"
#include "fourierwtcore.h"


// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   wtoscillator.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs Wavetable Oscillator module. 
		- See class declaration for information on standalone operation

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param _waveTableDatabase shared wavetable resource; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	WTOscillator::WTOscillator(std::shared_ptr<MidiInputData> _midiInputData, 
		std::shared_ptr<WTOscParameters> _parameters, 
		std::shared_ptr<WavetableDatabase> _waveTableDatabase,
		uint32_t blockSize)
		: SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- standalone ONLY: parameters
		if(!parameters)
			parameters.reset(new WTOscParameters);

		// --- standalone ONLY: database (not CPU efficient if using more than one of these objects)
		if (!_waveTableDatabase)
			waveTableDatabase.reset(new WavetableDatabase);
		else
			waveTableDatabase = _waveTableDatabase;

		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(WT_OSC_INPUTS, WT_OSC_OUTPUTS, blockSize));

		// --- setup the core processing structure for dynamic cores
		coreProcessData.inputBuffers = getAudioBuffers()->getInputBuffers();
		coreProcessData.outputBuffers = getAudioBuffers()->getOutputBuffers();
		coreProcessData.modulationInputs = modulationInput->getModulatorPtr();
		coreProcessData.modulationOutputs = modulationOutput->getModulatorPtr();
		coreProcessData.moduleParameters = parameters.get();
		coreProcessData.midiInputData = midiInputData->getIMIDIInputData();
		coreProcessData.wavetableDatabase = waveTableDatabase->getIWavetableDatabase();

		// --- setup the cores if not DM
		if (midiInputData->getAuxDAWDataUINT(kDMBuild) == 0)
		{
			// Core 0:
			std::shared_ptr<ClassicWTCore> classicCore = std::make_shared<ClassicWTCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(classicCore));

			// Core 1:
			std::shared_ptr<MorphWTCore> morphCore = std::make_shared<MorphWTCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(morphCore));

			// Core 2:
			std::shared_ptr<FourierWTCore> fourierCore = std::make_shared<FourierWTCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(fourierCore));

			// Core 3: Choose One:
			//  for DM this needs to be empty (at least one?)
			std::shared_ptr<SFXWTCore> sfxCore = std::make_shared<SFXWTCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(sfxCore));

			/*	std::shared_ptr<DrumWTCore> drumCore = std::make_shared<DrumWTCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(drumCore));*/
		}


	}	/* C-TOR */

	/**
	\brief Resets object to initialized state
	- call once during initialization
	- call any time sample rate changes (after init)
	- resets all member cores 

	\param _sampleRate the current sample rate in Hz

	\returns true if successful, false otherwise
	*/
	bool WTOscillator::reset(double _sampleRate)
	{
		// --- selects first core
		selectDefaultModuleCore();

		coreProcessData.sampleRate = _sampleRate;
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			if(moduleCores[i])
				moduleCores[i]->reset(coreProcessData);
		}

		return true;
	}

	/**
	\brief Updates the selected core; sets GLOBAL engine variable unisonDetuneCents 
	       that may have changed since last operation

	\returns true if successful, false otherwise
	*/	
	bool WTOscillator::update()
	{
		coreProcessData.unisonDetuneCents = unisonDetuneCents;
        if(!selectedCore) return false;
        return selectedCore->update(coreProcessData);
	}

	/**
	\brief Renders audio from the selected core. 
	- Calls the update function first - NOTE: owning object does not need to call update()
	- samples to process should normally be the block size, but may be a partial block in some cases
	  due to OS/CPU activity. 

	\param samplesToProcess the number of samples in this audio block

	\returns true if successful, false otherwise
	*/	
	bool WTOscillator::render(uint32_t samplesToProcess)
	{
		// --- update parameters for this block
		update();
		coreProcessData.samplesToProcess = samplesToProcess;
        if(!selectedCore) return false;
        return selectedCore->render(coreProcessData);
	}

	/**
	\brief Calls the note-on handler for all cores.

	\returns true if successful, false otherwise
	*/
	bool WTOscillator::doNoteOn(MIDINoteEvent& noteEvent)
	{
		coreProcessData.noteEvent = noteEvent;
		coreProcessData.unisonStartPhase = unisonStartPhase;
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			if (moduleCores[i])
				moduleCores[i]->doNoteOn(coreProcessData);
		}
		
		return true;
	}

	/**
	\brief Calls the note-off handler for all cores.

	\returns true if successful, false otherwise
	*/
	bool WTOscillator::doNoteOff(MIDINoteEvent& noteEvent)
	{
		coreProcessData.noteEvent = noteEvent;
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			if (moduleCores[i])
				moduleCores[i]->doNoteOff(coreProcessData);
		}
		return true;
	}


}

