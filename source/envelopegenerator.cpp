#include "envelopegenerator.h"
#include "analogegcore.h"
#include "dxegcore.h"
#include "linearegcore.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   envelopegenerator.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs Envelope Generator module.
	- See class declaration for information on standalone operation

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	*/
	EnvelopeGenerator::EnvelopeGenerator(std::shared_ptr<MidiInputData> _midiInputData,
		std::shared_ptr<EGParameters> _parameters, 
		uint32_t blockSize) :
		SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- for standalone operation
		if (!parameters)
			parameters.reset(new EGParameters);

		// --- setup the core processing structure for dynamic cores
		coreProcessData.modulationInputs = modulationInput->getModulatorPtr();
		coreProcessData.modulationOutputs = modulationOutput->getModulatorPtr();
		coreProcessData.moduleParameters = parameters.get();
		coreProcessData.midiInputData = midiInputData->getIMIDIInputData();

		// --- setup the cores
		if (midiInputData->getAuxDAWDataUINT(kDMBuild) == 0)
		{
			// --- setup the cores
			std::shared_ptr<AnalogEGCore> analogCore = std::make_shared<AnalogEGCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(analogCore));

			std::shared_ptr<DXEGCore> dxCore = std::make_shared<DXEGCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(dxCore));

			std::shared_ptr<LinearEGCore> linearCore = std::make_shared<LinearEGCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(linearCore));
		}
	}

	/**
	\brief Resets object to initialized state
	- call once during initialization
	- call any time sample rate changes (after init)
	- resets all member cores

	\param _sampleRate the current sample rate in Hz

	\returns true if successful, false otherwise
	*/
	bool EnvelopeGenerator::reset(double _sampleRate)
	{
		// --- selects first core
		selectDefaultModuleCore();
		
		coreProcessData.sampleRate = _sampleRate;
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			if (moduleCores[i])
				moduleCores[i]->reset(coreProcessData);

		}
		return true;
	}

	/**
	\brief Updates the selected core; sets GLOBAL engine variable unisonDetuneCents
	that may have changed since last operation

	\returns true if successful, false otherwise
	*/
	bool EnvelopeGenerator::update()
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
	bool EnvelopeGenerator::render(uint32_t samplesToProcess)
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
	bool EnvelopeGenerator::doNoteOn(MIDINoteEvent& noteEvent)
	{
		coreProcessData.noteEvent = noteEvent;
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
	bool EnvelopeGenerator::doNoteOff(MIDINoteEvent& noteEvent)
	{
		coreProcessData.noteEvent = noteEvent;
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			if (moduleCores[i])
				moduleCores[i]->doNoteOff(coreProcessData);
		}
		return true;
	}

	/**
	\brief Get staste of selected core; used as part of note life-cycle

	\returns state encoded as uint32
	*/
	int32_t EnvelopeGenerator::getState()
	{
        if(!selectedCore) return false;
        return selectedCore->getState();
	}

	/**
	\brief Quickly turn off the EG for voice steal and RTZ operation

	\returns true if successful
	*/
	bool EnvelopeGenerator::shutdown()
	{
        if(!selectedCore) return false;
        return selectedCore->shutdown();
	}
}
