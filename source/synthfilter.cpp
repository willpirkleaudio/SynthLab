#include "synthfilter.h"
#include "vafiltercore.h"
#include "bqfiltercore.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthfilter.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs SynthFilter module.
	- See class declaration for information on standalone operation

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	SynthFilter::SynthFilter(std::shared_ptr<MidiInputData> _midiInputData, 
		std::shared_ptr<FilterParameters> _parameters, 
		uint32_t blockSize) :
		SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- create if missing
		if (!parameters)
			parameters.reset(new FilterParameters);

		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(FILTER_AUDIO_INPUTS, FILTER_AUDIO_OUTPUTS, blockSize));

		// --- setup the core processing structure for dynamic cores
		coreProcessData.inputBuffers = getAudioBuffers()->getInputBuffers();
		coreProcessData.outputBuffers = getAudioBuffers()->getOutputBuffers();
		coreProcessData.modulationInputs = modulationInput->getModulatorPtr();
		coreProcessData.modulationOutputs = modulationOutput->getModulatorPtr();
		coreProcessData.moduleParameters = parameters.get();
		coreProcessData.midiInputData = midiInputData->getIMIDIInputData();

		// --- setup the cores
		if (midiInputData->getAuxDAWDataUINT(kDMBuild) == 0)
		{
			std::shared_ptr<VAFilterCore> vaCore = std::make_shared<VAFilterCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(vaCore));

			std::shared_ptr<BQFilterCore> bqCore = std::make_shared<BQFilterCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(bqCore));
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
	bool SynthFilter::reset(double _sampleRate)
	{
		// --- core[0]
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
	\brief Updates the selected core

	\returns true if successful, false otherwise
	*/
	bool SynthFilter::update()
	{
        if(!selectedCore) return false;
        return selectedCore->update(coreProcessData);
	}

	/**
	\brief Renders processes audio from the selected core
	- Calls the update function first - NOTE: owning object does not need to call update()
	- samples to process should normally be the block size, but may be a partial block in some cases
	due to OS/CPU activity.

	\returns true if successful, false otherwise
	*/
	bool SynthFilter::render(uint32_t samplesToProcess)
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
	bool SynthFilter::doNoteOn(MIDINoteEvent& noteEvent)
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
	bool SynthFilter::doNoteOff(MIDINoteEvent& noteEvent)
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

