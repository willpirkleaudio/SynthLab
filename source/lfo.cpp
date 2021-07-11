#include "lfo.h"
#include "lfocore.h"
#include "fmlfocore.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   lfo.cpp
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
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	SynthLFO::SynthLFO(std::shared_ptr<MidiInputData> _midiInputData, 
		std::shared_ptr<LFOParameters> _parameters, 
		uint32_t blockSize)
		: SynthModule(_midiInputData)
		, parameters(_parameters)
	{	
		// --- for standalone operation
		if (!parameters)
			parameters.reset(new LFOParameters);

		// --- audio buffers
		audioBuffers.reset(new SynthProcessInfo(2, 2, blockSize));

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
			std::shared_ptr<LFOCore> classicCore = std::make_shared<LFOCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(classicCore));

			std::shared_ptr<FMLFOCore> fmCore = std::make_shared<FMLFOCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(fmCore));
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
	bool SynthLFO::reset(double _sampleRate)
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
	\brief Updates the selected core; sets GLOBAL engine variable unisonDetuneCents
	that may have changed since last operation

	\returns true if successful, false otherwise
	*/
	bool SynthLFO::update()
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
	bool SynthLFO::render(uint32_t samplesToProcess)
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
	bool SynthLFO::doNoteOn(MIDINoteEvent& noteEvent)
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
	bool SynthLFO::doNoteOff(MIDINoteEvent& noteEvent)
	{
		coreProcessData.noteEvent = noteEvent;
		for (uint32_t i = 0; i < NUM_MODULE_CORES; i++)
		{
			if (moduleCores[i])
				moduleCores[i]->doNoteOff(coreProcessData);
		}
		return true;
	}

} // namespace


