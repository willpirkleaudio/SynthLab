#include "fmoperator.h"


// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   fmoperator.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs FM Operator Oscillator module.
	- See class declaration for information on standalone operation

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param _waveTableDatabase shared wavetable resource; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	FMOperator::FMOperator(std::shared_ptr<MidiInputData> _midiInputData,
		std::shared_ptr<FMOperatorParameters> _parameters,
		std::shared_ptr<WavetableDatabase> _waveTableDatabase,
		uint32_t blockSize)
		: SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- standalone ONLY: parameters
		if (!parameters)
			parameters.reset(new FMOperatorParameters);

		// --- standalone ONLY: database (not CPU efficient if using more than one of these objects)
		if (!_waveTableDatabase)
			waveTableDatabase.reset(new WavetableDatabase);
		else
			waveTableDatabase = _waveTableDatabase;

		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(FM_OSC_INPUTS, FM_OSC_OUTPUTS, blockSize));

		// --- setup the core processing structure for dynamic cores
		coreProcessData.inputBuffers = getAudioBuffers()->getInputBuffers();
		coreProcessData.outputBuffers = getAudioBuffers()->getOutputBuffers();
		coreProcessData.modulationInputs = modulationInput->getModulatorPtr();// [0];
		coreProcessData.modulationOutputs = modulationOutput->getModulatorPtr();
		coreProcessData.moduleParameters = parameters.get();
		coreProcessData.midiInputData = midiInputData->getIMIDIInputData();
		coreProcessData.wavetableDatabase = waveTableDatabase->getIWavetableDatabase();

		// --- setup the cores
		if (midiInputData->getAuxDAWDataUINT(kDMBuild) == 0)
		{
			std::shared_ptr<FMOCore> defaultCore = std::make_shared<FMOCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(defaultCore));
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
	bool FMOperator::reset(double _sampleRate)
	{
		// --- core[0]
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
	bool FMOperator::update()
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
	bool FMOperator::render(uint32_t samplesToProcess)
	{
		// --- update parameters for this block
		update();

		coreProcessData.samplesToProcess = samplesToProcess;
		coreProcessData.fmBuffers = fmBuffer ? fmBuffer->getOutputBuffers() : nullptr;
			
        if(!selectedCore) return false;
        return selectedCore->render(coreProcessData);
	}


	/**
	\brief Calls the note-on handler for all cores.

	\returns true if successful, false otherwise
	*/
	bool FMOperator::doNoteOn(MIDINoteEvent& noteEvent)
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
	bool FMOperator::doNoteOff(MIDINoteEvent& noteEvent)
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

