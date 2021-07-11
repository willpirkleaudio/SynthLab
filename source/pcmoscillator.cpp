#include "pcmoscillator.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   pcmoscillator.cpp
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
	\param _sampleDatabase shared sample database resource; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	PCMOscillator::PCMOscillator(std::shared_ptr<MidiInputData> _midiInputData,
		std::shared_ptr<PCMOscParameters> _parameters,
		std::shared_ptr<PCMSampleDatabase> _sampleDatabase,
		uint32_t blockSize)
		: SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(SMPL_OSC_INPUTS, SMPL_OSC_OUTPUTS, blockSize));
		
		// --- standalone ONLY: parameters
		if (!parameters)
			parameters.reset(new PCMOscParameters);

		// --- only for standalone operation
		if (!_sampleDatabase)
			sampleDatabase.reset(new PCMSampleDatabase);
		else
			sampleDatabase = _sampleDatabase;

		// --- setup the core processing structure for dynamic cores
		coreProcessData.inputBuffers = getAudioBuffers()->getInputBuffers();
		coreProcessData.outputBuffers = getAudioBuffers()->getOutputBuffers();
		coreProcessData.modulationInputs = modulationInput->getModulatorPtr();
		coreProcessData.modulationOutputs = modulationOutput->getModulatorPtr();
		coreProcessData.moduleParameters = parameters.get();
		coreProcessData.midiInputData = midiInputData->getIMIDIInputData();
		coreProcessData.sampleDatabase = sampleDatabase->getIPCMSampleDatabase();

		// --- setup the cores
		if (midiInputData->getAuxDAWDataUINT(kDMBuild) == 0)
		{
			std::shared_ptr<LegacyPCMCore> defaultCore = std::make_shared<LegacyPCMCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(defaultCore));

			std::shared_ptr<MellotronCore> mellotron = std::make_shared<MellotronCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(mellotron));

			std::shared_ptr<WaveSliceCore> slicedLoops = std::make_shared<WaveSliceCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(slicedLoops));
		}
	
	}	/* C-TOR */

	/**
	\brief Resets object to initialized state
	- call once during initialization
	- call any time sample rate changes (after init)
	- resets all member cores
	- note that it stores the DLL path so that cores may locate their WAV files properly

	\param _sampleRate the current sample rate in Hz

	\returns true if successful, false otherwise
	*/
	bool PCMOscillator::reset(double _sampleRate)
	{
		// --- core[0]
		selectDefaultModuleCore();

		coreProcessData.dllPath = dllDirectory.c_str();
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
	bool PCMOscillator::update()
	{
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
	bool PCMOscillator::render(uint32_t samplesToProcess)
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
	bool PCMOscillator::doNoteOn(MIDINoteEvent& noteEvent)
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
	bool PCMOscillator::doNoteOff(MIDINoteEvent& noteEvent)
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

