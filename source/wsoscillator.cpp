#include "wsoscillator.h"
#include "classicwtcore.h"
#include "fourierwtcore.h"
#include "morphwtcore.h"


// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   wsoscillator.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs Wave Sequencing Oscillator module.
	- See class declaration for information on standalone operation

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param _waveTableDatabase shared wavetable resource; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	WSOscillator::WSOscillator(std::shared_ptr<MidiInputData> _midiInputData,
		std::shared_ptr<WSOscParameters> _parameters,
		std::shared_ptr<WavetableDatabase> _waveTableDatabase,
		uint32_t blockSize)
		: SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(0, WT_OSC_OUTPUTS, blockSize));

		for (uint32_t i = 0; i < NUM_WS_OSCILLATORS; i++)
		{
			waveSeqParams[i].reset(new(WTOscParameters));
			waveSeqParams[i]->modKnobValue[MOD_KNOB_A] = 0.5;
			waveSeqParams[i]->modKnobValue[MOD_KNOB_B] = 1.0; // HSync enabled
			waveSeqParams[i]->modKnobValue[MOD_KNOB_C] = 0.0;

			// this is the morph intensity for morphing WT, and not assigned for WT
			waveSeqParams[i]->modKnobValue[MOD_KNOB_D] = 1.0;
		}
	
		// --- four wavetable oscillators
		waveSeqOsc[0].reset(new WTOscillator(_midiInputData, waveSeqParams[0], _waveTableDatabase, blockSize));
		waveSeqOsc[1].reset(new WTOscillator(_midiInputData, waveSeqParams[1], _waveTableDatabase, blockSize));
		waveSeqOsc[2].reset(new WTOscillator(_midiInputData, waveSeqParams[2], _waveTableDatabase, blockSize));
		waveSeqOsc[3].reset(new WTOscillator(_midiInputData, waveSeqParams[3], _waveTableDatabase, blockSize));
	
	}	/* C-TOR */


	/**
	\brief Resets object to initialized state
	- call once during initialization
	- call any time sample rate changes (after init)
	- create string map for waveform names

	\param _sampleRate the current sample rate in Hz

	\returns true if successful, false otherwise
	*/
	bool WSOscillator::reset(double _sampleRate)
	{
		// --- create string map
		makeWaveStringMap();

		for (uint32_t i = 0; i < NUM_WS_OSCILLATORS; i++)
		{
			oscMixCoeff[i] = 0.0;
			waveSeqOsc[i]->reset(_sampleRate);
		}

		// --- initial pair of oscillators
		activeOsc[0] = 0;
		activeOsc[1] = 1;

		// --- start round robin operation
		initRoundRobin = true;
		return true;
	}

	/**
	\brief Updates the two active oscillators during the update() phase

	\returns true if successful, false otherwise
	*/
	void WSOscillator::updateActiveOscillators()
	{
		uint32_t wave_AStepNumber = getModulationInput()->getModValue(kWaveStepNumber_A);
		uint32_t wave_BStepNumber = getModulationInput()->getModValue(kWaveStepNumber_B);

		// --- need to know the WS index of this oscillator
		std::shared_ptr<WTOscParameters> params0 = waveSeqOsc[activeOsc[0]]->getParameters();
		params0->panValue = parameters->panValue[wave_AStepNumber];
		params0->modKnobValue[MOD_KNOB_B] = parameters->hardSyncRatio[wave_AStepNumber];
		params0->modKnobValue[MOD_KNOB_D] = parameters->morphIntensity[wave_AStepNumber];
		params0->coarseDetune = parameters->detuneSemis[wave_AStepNumber];
		params0->fineDetune = parameters->detuneCents[wave_AStepNumber];
		params0->forceLoop = true; // force drum and SFX to repeat each time triggered

		std::shared_ptr<WTOscParameters> params1 = waveSeqOsc[activeOsc[1]]->getParameters();
		params1->panValue = parameters->panValue[wave_BStepNumber];
		params1->modKnobValue[MOD_KNOB_B] = parameters->hardSyncRatio[wave_BStepNumber];
		params1->modKnobValue[MOD_KNOB_D] = parameters->morphIntensity[wave_BStepNumber];
		params1->coarseDetune = parameters->detuneSemis[wave_BStepNumber];
		params1->fineDetune = parameters->detuneCents[wave_BStepNumber];
		params1->forceLoop = true; // force drum and SFX to repeat each time triggered

		//waveSeqOsc[activeOsc[0]]->update();
		//waveSeqOsc[activeOsc[1]]->update();
	}

	/**
	\brief Updates the the two active oscillators
	- watches the output of the wave sequencer to know when to switch to the next pair of oscillators
	- forwards some information to the oscillators as part of its own update phase

	\returns true if successful, false otherwise
	*/
	bool WSOscillator::update()
	{
        
		uint32_t wave_AIndex = getModulationInput()->getModValue(kWaveSeqWaveIndex_AMod);
        uint32_t wave_BIndex = getModulationInput()->getModValue(kWaveSeqWaveIndex_BMod);

		double oscAMixCoeff = getModulationInput()->getModValue(kWaveSeqWave_AGainMod);
		double oscBMixCoeff = getModulationInput()->getModValue(kWaveSeqWave_BGainMod);

		// --- this needs to be done during update() because the mod values are not ready
		//     until the sequencer is run the first time, which is after note-on
		if (initRoundRobin)
		{
			setNewOscWaveA(activeOsc[0], wave_AIndex, oscAMixCoeff);
			setNewOscWaveB(activeOsc[1], wave_BIndex, oscBMixCoeff);
			initRoundRobin = false;
			updateActiveOscillators();
			return true;
		}

		// --- soloing?
		if (parameters->soloWaveWSIndex >= 0)
		{
			wave_AIndex = parameters->soloWaveWSIndex;
			wave_BIndex = parameters->soloWaveWSIndex;

			if (currSoloWave != wave_AIndex) // new solo
			{
				setNewOscWaveA(activeOsc[0], wave_AIndex, oscAMixCoeff);
				setNewOscWaveB(activeOsc[1], wave_BIndex, oscBMixCoeff);
				updateActiveOscillators();
			}
			currSoloWave = wave_AIndex;
		}
		else
			currSoloWave = -1;

		// --- check xfade boundary to switch oscillators
		bool xfadeDone = getModulationInput()->getModValue(kWaveSeqXFadeDoneMod) == 0 ? false : true;
		if (xfadeDone)
		{
			// --- rotate to next pair of oscillators
			if (++activeOsc[0] >= NUM_WS_OSCILLATORS) activeOsc[0] = 0;
			if (++activeOsc[1] >= NUM_WS_OSCILLATORS) activeOsc[1] = 0;

			// --- set wave index and mix coeff
			setNewOscWaveA(activeOsc[0], wave_AIndex, oscAMixCoeff);
			setNewOscWaveB(activeOsc[1], wave_BIndex, oscBMixCoeff); 
			updateActiveOscillators();
			return true;
		}

		// --- forward the settings - NOTE: WS Specific only
		waveSeqParams[activeOsc[0]]->oscSpecificDetune = getModulationInput()->getModValue(kWaveSeqPitch_AMod);
		waveSeqParams[activeOsc[0]]->outputAmplitude_dB = getModulationInput()->getModValue(kWaveSeqAmp_AMod);
		oscMixCoeff[activeOsc[0]] = oscAMixCoeff;

		waveSeqParams[activeOsc[1]]->oscSpecificDetune = getModulationInput()->getModValue(kWaveSeqPitch_BMod);
		waveSeqParams[activeOsc[1]]->outputAmplitude_dB = getModulationInput()->getModValue(kWaveSeqAmp_BMod);
		oscMixCoeff[activeOsc[1]] = oscBMixCoeff;
		
		updateActiveOscillators();
		return true;
	}

	/**
	\brief Renders audio from the selected core.
	- Calls the update function first - NOTE: owning object does not need to call update()
	- samples to process should normally be the block size, but may be a partial block in some cases
	due to OS/CPU activity.

	\param samplesToProcess the number of samples in this audio block

	\returns true if successful, false otherwise
	*/
	bool WSOscillator::render(uint32_t samplesToProcess)
	{
		// --- update parameters for this block
		update();

		waveSeqOsc[activeOsc[0]]->render(samplesToProcess);
		waveSeqOsc[activeOsc[1]]->render(samplesToProcess);

		// --- mix output buffers into our buffer
		getAudioBuffers()->flushBuffers();

		mixOscBuffers(waveSeqOsc[activeOsc[0]]->getAudioBuffers(), samplesToProcess, oscMixCoeff[activeOsc[0]]);
		mixOscBuffers(waveSeqOsc[activeOsc[1]]->getAudioBuffers(), samplesToProcess, oscMixCoeff[activeOsc[1]]);

		return true;
	}

	/**
	\brief Calls the note-on handler for all cores on all four internal oscillators
	so that they will be running during the round-robin render phase

	\returns true if successful, false otherwise
	*/
	bool WSOscillator::doNoteOn(MIDINoteEvent& noteEvent)
	{
		// --- select each core and call noteOn so all cores
		//     of all oscillators will have the messages;
		//     NOTE: the oscillators do NOT need to all be running
		//     only the two "active oscillators" will be running
		for (uint32_t core = 0; core < NUM_MODULE_CORES; core++)
		{
			for (uint32_t i = 0; i < NUM_WS_OSCILLATORS; i++)
			{
				waveSeqOsc[i]->selectModuleCore(core);
				waveSeqOsc[i]->doNoteOn(noteEvent);
			}
		}

		initRoundRobin = true;
		return true;
	}

	/**
	\brief Calls the note-off handler on four internal oscillators

	\returns true if successful, false otherwise
	*/
	bool WSOscillator::doNoteOff(MIDINoteEvent& noteEvent)
	{
		for (uint32_t i = 0; i < NUM_WS_OSCILLATORS; i++)
		{
			waveSeqOsc[i]->doNoteOff(noteEvent);
		}
		return true;
	}

	/**
	\brief Calls the startGlideModulation handler for the first two oscillators; these
	will always be the first two in the sequence.

	\returns true if successful, false otherwise
	*/
	bool WSOscillator::startGlideModulation(GlideInfo& glideInfo)
	{
		waveSeqOsc[0]->startGlideModulation(glideInfo);
		waveSeqOsc[1]->startGlideModulation(glideInfo);
		return true;
	}

	/**
	\brief Creates a mapping that links the waveform names to their underlying cores and oscillators.
	- when the user selects a waveform, this decodes the oscillator and core that will be needed for rendering

	\returns true if successful, false otherwise
	*/
	void WSOscillator::makeWaveStringMap()
	{
		// --- store our own deocder version
		waveStringFinder.clear();

		// --- core 0 strings
		std::vector<std::string> coreWaveStrings;
		for (uint32_t core = 0; core < NUM_MODULE_CORES; core++)
		{
			// --- all oscillators have identical cores
			waveSeqOsc[0]->getModuleStrings(core, coreWaveStrings, empty_string);

			// --- stores the core and wave index with the GUI total wave index
			for (uint32_t i = 0; i < coreWaveStrings.size(); i++)
			{
				WaveStringData wsd(core, i);
				waveStringFinder.push_back(wsd);
			}
			coreWaveStrings.clear();
		}
	}


	/**
	\brief Helper function to mix oscillator buffers together during render() phase into the audio buffers
	on this SynthModule

	\param oscBuffers audio buffers to add to the audio output buffers
	\param samplesInBlock samples to add
	\param scaling a multiplier/divider used for scaling during mixing

	*/
	void WSOscillator::mixOscBuffers(std::shared_ptr<AudioBuffer> oscBuffers, uint32_t samplesInBlock, double scaling)
	{
		float* leftOutBuffer = getAudioBuffers()->getOutputBuffer(LEFT_CHANNEL);
		float* rightOutBuffer = getAudioBuffers()->getOutputBuffer(RIGHT_CHANNEL);
		float* leftOscBuffer = oscBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOscBuffer = oscBuffers->getOutputBuffer(RIGHT_CHANNEL);

		for (uint32_t i = 0; i < samplesInBlock; i++)
		{
			// --- stereo
			leftOutBuffer[i] += leftOscBuffer[i] * scaling;
			rightOutBuffer[i] += rightOscBuffer[i] * scaling;
		}
	}

	/**
	\brief Finds out if oscillator has the same core and waveform index as the selection

	\param oscIndex index in oscillator array
	\param waveAIndex index of waveform A for the wave-string-finder
	\param waveBIndex index of waveform B for the wave-string-finder

	\return true if oscillator matches the criteria, false otherwise
	*/
	bool WSOscillator::oscIsFree(uint32_t oscIndex, uint32_t waveAIndex, uint32_t waveBIndex)
	{
		if (waveSeqOsc[oscIndex]->getSelectedCoreIndex() == waveStringFinder[waveAIndex].coreIndex &&
			waveSeqParams[oscIndex]->waveIndex == waveStringFinder[waveAIndex].coreWaveIndex)
			return false;

		if (waveSeqOsc[oscIndex]->getSelectedCoreIndex() == waveStringFinder[waveBIndex].coreIndex &&
			waveSeqParams[oscIndex]->waveIndex == waveStringFinder[waveBIndex].coreWaveIndex)
			return false;

		return true;
	}

	/**
	\brief Finds out if oscillator has the same waveform index as the selection

	\param oscIndex index in oscillator array
	\param waveIndex index of waveform for the wave-string-finder

	\return true if oscillator matches the criteria, false otherwise
	*/
	bool WSOscillator::oscHasWaveIndex(uint32_t oscIndex, uint32_t waveIndex)
	{
		if (waveSeqOsc[oscIndex]->getSelectedCoreIndex() == waveStringFinder[waveIndex].coreIndex &&
			waveSeqParams[oscIndex]->waveIndex == waveStringFinder[waveIndex].coreWaveIndex)
			return true;

		return false;
	}

	/**
	\brief Change oscillator's waveform to Wave A.

	\param oscIndex index in oscillator array
	\param waveAIndex index of waveform A for the wave-string-finder
	\param oscAMixCoeff mixing coefficient for the waveform to try to keep levels 
	consistent across oscillators and waveforms

	*/
	void WSOscillator::setNewOscWaveA(uint32_t oscIndex, uint32_t waveAIndex, double oscAMixCoeff)
	{
        uint32_t coreIndex = waveStringFinder[waveAIndex].coreIndex;
        boundUIntValue(coreIndex, 0, NUM_MODULE_CORES-1);
        uint32_t waveIndex = waveStringFinder[waveAIndex].coreWaveIndex;
        boundUIntValue(waveIndex, 0, MODULE_STRINGS-1);

		waveSeqOsc[oscIndex]->selectModuleCore(coreIndex);
		waveSeqParams[oscIndex]->waveIndex = waveIndex;
		waveSeqParams[oscIndex]->oscSpecificDetune = getModulationInput()->getModValue(kWaveSeqPitch_AMod);
		waveSeqParams[oscIndex]->outputAmplitude_dB = getModulationInput()->getModValue(kWaveSeqAmp_AMod);
		oscMixCoeff[oscIndex] = oscAMixCoeff;
	}
	
	/**
	\brief Change oscillator's waveform to Wave B.

	\param oscIndex index in oscillator array
	\param waveBIndex index of waveform B for the wave-string-finder
	\param oscAMixCoeff mixing coefficient for the waveform to try to keep levels
	consistent across oscillators and waveforms

	*/
	void WSOscillator::setNewOscWaveB(uint32_t oscIndex, uint32_t waveBIndex, double oscBMixCoeff)
	{
        uint32_t coreIndex = waveStringFinder[waveBIndex].coreIndex;
        boundUIntValue(coreIndex, 0, NUM_MODULE_CORES-1);
        uint32_t waveIndex = waveStringFinder[waveBIndex].coreWaveIndex;
        boundUIntValue(waveIndex, 0, MODULE_STRINGS-1);

        waveSeqOsc[oscIndex]->selectModuleCore(coreIndex);
        waveSeqParams[oscIndex]->waveIndex = waveIndex;
		waveSeqParams[oscIndex]->oscSpecificDetune = getModulationInput()->getModValue(kWaveSeqPitch_BMod);
		waveSeqParams[oscIndex]->outputAmplitude_dB = getModulationInput()->getModValue(kWaveSeqAmp_BMod);
		oscMixCoeff[oscIndex] = oscBMixCoeff;
	}


	/**
	\brief Gets the modulation input array pointer from one of the internal oscillators

	\param oscIndex index in oscillator array
	*/
	std::shared_ptr<Modulators> WSOscillator::getWSOscModulationInput(uint32_t oscIndex)
	{
		if (oscIndex > NUM_WS_OSCILLATORS - 1) return nullptr;
		return waveSeqOsc[oscIndex]->getModulationInput();
	}

	/**
	\brief Provides direct access to internal oscillator objects.

	\param oscIndex index in oscillator array
	*/
	std::shared_ptr<WTOscillator> WSOscillator::getWTOscillator(uint32_t oscIndex)
	{
		if (oscIndex >= NUM_WS_OSCILLATORS)
			oscIndex = NUM_WS_OSCILLATORS - 1;
		return waveSeqOsc[oscIndex];
	}


}

