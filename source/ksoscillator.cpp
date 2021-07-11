#include "ksoscillator.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   ksoscillator.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	KSOscillator::KSOscillator(std::shared_ptr<MidiInputData> _midiInputData,
		std::shared_ptr<KSOscParameters> _parameters,
		uint32_t blockSize)
		: SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- standalone ONLY: parameters
		if (!parameters)
			parameters.reset(new KSOscParameters);

		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(KS_OSC_INPUTS, KS_OSC_OUTPUTS, blockSize));

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
			uint32_t moduleIndex = 0;
			std::shared_ptr<KSOCore> defaultCore = std::make_shared<KSOCore>();
			addModuleCore(std::static_pointer_cast<ModuleCore>(defaultCore));
		}

	}	/* C-TOR */

	bool KSOscillator::reset(double _sampleRate)
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

	bool KSOscillator::update()
	{
        if(!selectedCore) return false;
        return selectedCore->update(coreProcessData);
	}

	bool KSOscillator::render(uint32_t samplesToProcess)
	{
		// --- update parameters for this block
		update();

		coreProcessData.samplesToProcess = samplesToProcess;
        if(!selectedCore) return false;
        return selectedCore->render(coreProcessData);
	}

	bool KSOscillator::doNoteOn(MIDINoteEvent& noteEvent)
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

	bool KSOscillator::doNoteOff(MIDINoteEvent& noteEvent)
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

