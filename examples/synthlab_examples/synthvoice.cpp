// --- Synth Core v1.0
//
#include "synthvoice.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthvoice.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date  cc 20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief
	Construction:
	- constructs each of its SynthModule objects and uses one of the members of the voice
	parameter structure as the shared parameter pointers
	- supplies wavetable database to oscillators that need it
	- supplies PCM sample database to oscillators that need it
	- sets up the modulation matrix source and destinations using the SynthModule components
	- sets up hard-wired modulation matrix routings
	- supports standalone operation using nullptrs (see documentation)

	\param _midiInputData shared pointer to MIDI input data; created on SynthEngine
	\param _midiOutputData shared pointer to MIDI input data; not used
	\param _parameters shared pointer to voice parameter structure; shared with all other voices
	\param _wavetableDatabase shared pointer to wavetable database; created on SynthEngine
	\param _sampleDatabase shared pointer to PCM sample database; created on SynthEngine
	\param blockSize the block size to be used for the lifetime of operation; OK if arriving blocks are smaller than this value, NOT OK if larger

	\returns the newly constructed object
	*/
	SynthVoice::SynthVoice(std::shared_ptr<MidiInputData> _midiInputData,
		std::shared_ptr<MidiOutputData> _midiOutputData,
		std::shared_ptr<SynthVoiceParameters> _parameters,
		std::shared_ptr<WavetableDatabase> _wavetableDatabase,
		std::shared_ptr<PCMSampleDatabase> _sampleDatabase,
		uint32_t _blockSize)
		: midiInputData(_midiInputData)		//<- set our midi dat interface value
		, midiOutputData(_midiOutputData)
		, parameters(_parameters)	//<- set our parameters
		, wavetableDatabase(_wavetableDatabase)
		, sampleDatabase(_sampleDatabase)
		, blockSize(_blockSize)
	{
		// for standalone
		if (!midiInputData)
			midiInputData.reset(new (MidiInputData));
		if (!midiOutputData)
			midiOutputData.reset(new (MidiOutputData));

		// --- this happens in stand-alone mode; does not happen otherwise;
		//     the first initialized SynthLab component creates its own parameters
		if (!parameters)
			parameters = std::make_shared<SynthVoiceParameters>();

		// --- NOTE: in standalone mode, the modules will create the wavetable and PCM databases
		//           locally, so they do not need to be checked here.

		// --- LFOs
		lfo[0].reset(new SynthLFO(midiInputData, parameters->lfo1Parameters, blockSize));
		lfo[1].reset(new SynthLFO(midiInputData, parameters->lfo2Parameters, blockSize));

		// --- initialize cores; may be overwritten if you use dynamic strings
		for (uint32_t i = 0; i < NUM_LFO; i++) {
			lfo[i]->selectModuleCore(enumToInt(lfoCores[i]));
		}

		// --- EGs
		ampEG.reset(new EnvelopeGenerator(midiInputData, parameters->ampEGParameters, blockSize));
		filterEG.reset(new EnvelopeGenerator(midiInputData, parameters->filterEGParameters, blockSize));
		auxEG.reset(new EnvelopeGenerator(midiInputData, parameters->auxEGParameters, blockSize));

		// --- initialize cores; may be overwritten if you use dynamic strings
		ampEG->selectModuleCore(enumToInt(ampEGCore));
		filterEG->selectModuleCore(enumToInt(filterEGCore));
		auxEG->selectModuleCore(enumToInt(auxEGCore));

		// --- filters
		filter[0].reset(new SynthFilter(midiInputData, parameters->filter1Parameters, blockSize));
		filter[1].reset(new SynthFilter(midiInputData, parameters->filter2Parameters, blockSize));

		// --- setup the Analog FGN if desired
		parameters->filter1Parameters->analogFGN = midiInputData->getAuxDAWDataUINT(kAnalogFGNFilters) == 1;
		parameters->filter2Parameters->analogFGN = midiInputData->getAuxDAWDataUINT(kAnalogFGNFilters) == 1;

		// --- initialize cores; may be overwritten if you use dynamic strings
		for (uint32_t i = 0; i < NUM_FILTER; i++) {
			filter[i]->selectModuleCore(enumToInt(filterCores[i]));
		}

		// --- SYNTHLAB-WT: 4 wavetable oscillators
#ifdef SYNTHLAB_WT
		oscillator[0].reset(new WTOscillator(midiInputData, parameters->osc1Parameters, wavetableDatabase, blockSize));
		oscillator[1].reset(new WTOscillator(midiInputData, parameters->osc2Parameters, wavetableDatabase, blockSize));
		oscillator[2].reset(new WTOscillator(midiInputData, parameters->osc3Parameters, wavetableDatabase, blockSize));
		oscillator[3].reset(new WTOscillator(midiInputData, parameters->osc4Parameters, wavetableDatabase, blockSize));

		// --- initialize cores; may be overwritten if you use dynamic strings
		for (uint32_t i = 0; i < NUM_OSC; i++) {
			oscillator[i]->selectModuleCore(enumToInt(wtCores[i]));
		}
#elif SYNTHLAB_VA
		oscillator[0].reset(new VAOscillator(midiInputData, parameters->osc1Parameters, blockSize));
		oscillator[1].reset(new VAOscillator(midiInputData, parameters->osc2Parameters, blockSize));
		oscillator[2].reset(new VAOscillator(midiInputData, parameters->osc3Parameters, blockSize));
		oscillator[3].reset(new VAOscillator(midiInputData, parameters->osc4Parameters, blockSize));

		// --- initialize cores; for VA there is only one so this is not really needed
		//     keeping the code in case there are more cores in the future
		for (uint32_t i = 0; i < NUM_OSC; i++) {
			oscillator[i]->selectModuleCore(0);
		}
#elif SYNTHLAB_PCM
		oscillator[0].reset(new PCMOscillator(midiInputData, parameters->osc1Parameters, sampleDatabase, blockSize));
		oscillator[1].reset(new PCMOscillator(midiInputData, parameters->osc2Parameters, sampleDatabase, blockSize));
		oscillator[2].reset(new PCMOscillator(midiInputData, parameters->osc3Parameters, sampleDatabase, blockSize));
		oscillator[3].reset(new PCMOscillator(midiInputData, parameters->osc4Parameters, sampleDatabase, blockSize));

		// --- initialize cores; 
		for (uint32_t i = 0; i < NUM_OSC; i++) {
			oscillator[i]->selectModuleCore(enumToInt(pcmCores[i]));// enumToInt(pcmCores[i]));
		}
#elif SYNTHLAB_KS
		oscillator[0].reset(new KSOscillator(midiInputData, parameters->osc1Parameters, blockSize));
		oscillator[1].reset(new KSOscillator(midiInputData, parameters->osc2Parameters, blockSize));
		oscillator[2].reset(new KSOscillator(midiInputData, parameters->osc3Parameters, blockSize));
		oscillator[3].reset(new KSOscillator(midiInputData, parameters->osc4Parameters, blockSize));

		// --- initialize cores; 
		for (uint32_t i = 0; i < NUM_OSC; i++) {
			oscillator[i]->selectModuleCore(0);
		}
#elif SYNTHLAB_DX
		oscillator[0].reset(new FMOperator(midiInputData, parameters->osc1Parameters, wavetableDatabase, blockSize));
		oscillator[1].reset(new FMOperator(midiInputData, parameters->osc2Parameters, wavetableDatabase, blockSize));
		oscillator[2].reset(new FMOperator(midiInputData, parameters->osc3Parameters, wavetableDatabase, blockSize));
		oscillator[3].reset(new FMOperator(midiInputData, parameters->osc4Parameters, wavetableDatabase, blockSize));

		// --- initialize cores; 
		for (uint32_t i = 0; i < NUM_OSC; i++) {
			oscillator[i]->selectModuleCore(0);
		}
#elif SYNTHLAB_WS
		// --- two WS oscillators: one main oscillator
		wsOscillator[MAIN_OSC].reset(new WSOscillator(midiInputData, parameters->wsOsc1Parameters, wavetableDatabase, blockSize));
		
		// --- and one for detuning coarse/fine (harmonize)
		wsOscillator[DETUNED_OSC].reset(new WSOscillator(midiInputData, parameters->wsOsc2Parameters, wavetableDatabase, blockSize));

		// --- the wavesequencer
		waveSequencer.reset(new WaveSequencer(midiInputData, parameters->waveSequencerParameters, blockSize));
#endif

		// --- DCA
		dca.reset(new DCA(midiInputData, parameters->dcaParameters, blockSize));

		// --- mod matrix
		modMatrix.reset(new ModMatrix(parameters->modMatrixParameters));

		// --- create our audio buffers
		mixBuffers.reset(new SynthProcessInfo(NO_CHANNELS, STEREO_CHANNELS, blockSize));

		// --- mod matrix can be reconfigured on the fly
		//
		// --- (1) clear the arrays
		modMatrix->clearModMatrixArrays();

		// --- (2) setup possible sources and destinations; can also be done on the fly
		//
		// --- 8 sources
		modMatrix->addModSource(kSourceLFO1_Norm, lfo[0]->getModulationOutput()->getModArrayPtr(kLFONormalOutput));
		modMatrix->addModSource(kSourceLFO2_Norm, lfo[1]->getModulationOutput()->getModArrayPtr(kLFONormalOutput));

		// --- tremolo mod
		modMatrix->addModSource(kSourceAmpEG_Norm, ampEG->getModulationOutput()->getModArrayPtr(kEGNormalOutput));
		modMatrix->addModSource(kSourceFilterEG_Norm, filterEG->getModulationOutput()->getModArrayPtr(kEGNormalOutput));
		modMatrix->addModSource(kSourceAuxEG_Norm, auxEG->getModulationOutput()->getModArrayPtr(kEGNormalOutput));

		modMatrix->addModSource(kSourceAmpEG_Bias, ampEG->getModulationOutput()->getModArrayPtr(kEGBiasedOutput));
		modMatrix->addModSource(kSourceFilterEG_Bias, filterEG->getModulationOutput()->getModArrayPtr(kEGBiasedOutput));
		modMatrix->addModSource(kSourceAuxEG_Bias, auxEG->getModulationOutput()->getModArrayPtr(kEGBiasedOutput));

		// --- 26 destinations
		// 
#ifndef SYNTHLAB_WS
		// --- oscillators
		modMatrix->addModDestination(kDestOsc1_fo, oscillator[0]->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc2_fo, oscillator[1]->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc3_fo, oscillator[2]->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc4_fo, oscillator[3]->getModulationInput()->getModArrayPtr(kBipolarMod));

		// --- kUniqueMod is specific to each oscillator; 
		modMatrix->addModDestination(kDestOsc1_Mod, oscillator[0]->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc2_Mod, oscillator[1]->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc3_Mod, oscillator[2]->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc4_Mod, oscillator[3]->getModulationInput()->getModArrayPtr(kUniqueMod));

		// --- connect to morphing wavetable oscillators
		modMatrix->addModDestination(kDestOsc1_Morph, oscillator[0]->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		modMatrix->addModDestination(kDestOsc2_Morph, oscillator[1]->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		modMatrix->addModDestination(kDestOsc3_Morph, oscillator[2]->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		modMatrix->addModDestination(kDestOsc4_Morph, oscillator[3]->getModulationInput()->getModArrayPtr(kWaveMorphMod));

		// --- shape mod is no on the mod matrix GUI (homework)
		modMatrix->addModDestination(kDestOsc1_Shape, oscillator[0]->getModulationInput()->getModArrayPtr(kShapeMod));
		modMatrix->addModDestination(kDestOsc2_Shape, oscillator[1]->getModulationInput()->getModArrayPtr(kShapeMod));
		modMatrix->addModDestination(kDestOsc3_Shape, oscillator[2]->getModulationInput()->getModArrayPtr(kShapeMod));
		modMatrix->addModDestination(kDestOsc4_Shape, oscillator[3]->getModulationInput()->getModArrayPtr(kShapeMod));

#elif defined SYNTHLAB_WS
		// --- oscillators
		modMatrix->addModDestination(kDestOsc1_fo, wsOscillator[MAIN_OSC]->getWTOscillator(0)->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc2_fo, wsOscillator[MAIN_OSC]->getWTOscillator(1)->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc3_fo, wsOscillator[MAIN_OSC]->getWTOscillator(2)->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc4_fo, wsOscillator[MAIN_OSC]->getWTOscillator(3)->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc5_fo, wsOscillator[DETUNED_OSC]->getWTOscillator(0)->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc6_fo, wsOscillator[DETUNED_OSC]->getWTOscillator(1)->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc7_fo, wsOscillator[DETUNED_OSC]->getWTOscillator(2)->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestOsc8_fo, wsOscillator[DETUNED_OSC]->getWTOscillator(3)->getModulationInput()->getModArrayPtr(kBipolarMod));

		modMatrix->addModDestination(kDestOsc1_Mod, wsOscillator[MAIN_OSC]->getWTOscillator(0)->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc2_Mod, wsOscillator[MAIN_OSC]->getWTOscillator(1)->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc3_Mod, wsOscillator[MAIN_OSC]->getWTOscillator(2)->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc4_Mod, wsOscillator[MAIN_OSC]->getWTOscillator(3)->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc5_Mod, wsOscillator[DETUNED_OSC]->getWTOscillator(0)->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc6_Mod, wsOscillator[DETUNED_OSC]->getWTOscillator(1)->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc7_Mod, wsOscillator[DETUNED_OSC]->getWTOscillator(2)->getModulationInput()->getModArrayPtr(kUniqueMod));
		modMatrix->addModDestination(kDestOsc8_Mod, wsOscillator[DETUNED_OSC]->getWTOscillator(3)->getModulationInput()->getModArrayPtr(kUniqueMod));

		// --- wavesequencer wiring (to be hardwired)
		modMatrix->addModSource(kSourceWSWaveMix_A, waveSequencer->getModulationOutput()->getModArrayPtr(kWSWaveMix_A));
		modMatrix->addModSource(kSourceWSWaveMix_B, waveSequencer->getModulationOutput()->getModArrayPtr(kWSWaveMix_B));
		modMatrix->addModSource(kSourceWSWaveIndex_A, waveSequencer->getModulationOutput()->getModArrayPtr(kWSWaveIndex_A));
		modMatrix->addModSource(kSourceWSWaveIndex_B, waveSequencer->getModulationOutput()->getModArrayPtr(kWSWaveIndex_B));
		modMatrix->addModSource(kSourceWSWaveAmpMod_A, waveSequencer->getModulationOutput()->getModArrayPtr(kWSWaveAmpMod_A));
		modMatrix->addModSource(kSourceWSWaveAmpMod_B, waveSequencer->getModulationOutput()->getModArrayPtr(kWSWaveAmpMod_B));
		modMatrix->addModSource(kSourceWSPitchMod_A, waveSequencer->getModulationOutput()->getModArrayPtr(kWSPitchMod_A));
		modMatrix->addModSource(kSourceWSPitchMod_B, waveSequencer->getModulationOutput()->getModArrayPtr(kWSPitchMod_B));
		modMatrix->addModSource(kSourceWStepSeqMod, waveSequencer->getModulationOutput()->getModArrayPtr(kWStepSeqMod));
		modMatrix->addModSource(kSourceWSXfadeDone, waveSequencer->getModulationOutput()->getModArrayPtr(kWSXFadeDone));
		
		modMatrix->addModDestination(kDestOsc1_WSWaveMix_A, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqWave_AGainMod));
		modMatrix->addModDestination(kDestOsc1_WSWaveMix_B, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqWave_BGainMod));
		modMatrix->addModDestination(kDestOsc1_WSWaveIndex_A, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqWaveIndex_AMod));
		modMatrix->addModDestination(kDestOsc1_WSWaveIndex_B, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqWaveIndex_BMod));
		modMatrix->addModDestination(kDestOsc1_WSWaveAmp_A, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqAmp_AMod));
		modMatrix->addModDestination(kDestOsc1_WSWaveAmp_B, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqAmp_BMod));
		modMatrix->addModDestination(kDestOsc1_WSWavePitch_A, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqPitch_AMod));
		modMatrix->addModDestination(kDestOsc1_WSWavePitch_B, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqPitch_BMod));
		modMatrix->addModDestination(kDestOsc1WSXFadeDone, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqXFadeDoneMod));
		
		modMatrix->addModDestination(kDestOsc2_WSWaveMix_A, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqWave_AGainMod));
		modMatrix->addModDestination(kDestOsc2_WSWaveMix_B, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqWave_BGainMod));
		modMatrix->addModDestination(kDestOsc2_WSWaveIndex_A, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqWaveIndex_AMod));
		modMatrix->addModDestination(kDestOsc2_WSWaveIndex_B, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqWaveIndex_BMod));
		modMatrix->addModDestination(kDestOsc2_WSWaveAmp_A, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqAmp_AMod));
		modMatrix->addModDestination(kDestOsc2_WSWaveAmp_B, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqAmp_BMod));
		modMatrix->addModDestination(kDestOsc2_WSWavePitch_A, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqPitch_AMod));
		modMatrix->addModDestination(kDestOsc2_WSWavePitch_B, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqPitch_BMod));
		modMatrix->addModDestination(kDestOsc2WSXFadeDone, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveSeqXFadeDoneMod));

		// --- Hardwire the WaveStepNumber, the wave step that is playing
		modMatrix->addModSource(kSourceWSStepNumber_A, waveSequencer->getModulationOutput()->getModArrayPtr(kWSWaveStepNumber_A));
		modMatrix->addModDestination(kDestWSWaveStepNumber_A1, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveStepNumber_A));
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSStepNumber_A, kDestWSWaveStepNumber_A1);

		modMatrix->addModDestination(kDestWSWaveStepNumber_A2, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveStepNumber_A));
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSStepNumber_A, kDestWSWaveStepNumber_A2);

		modMatrix->addModSource(kSourceWSStepNumber_B, waveSequencer->getModulationOutput()->getModArrayPtr(kWSWaveStepNumber_B));
		modMatrix->addModDestination(kDestWSWaveStepNumber_B1, wsOscillator[MAIN_OSC]->getModulationInput()->getModArrayPtr(kWaveStepNumber_B));
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSStepNumber_B, kDestWSWaveStepNumber_B1);

		modMatrix->addModDestination(kDestWSWaveStepNumber_B2, wsOscillator[DETUNED_OSC]->getModulationInput()->getModArrayPtr(kWaveStepNumber_B));
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSStepNumber_B, kDestWSWaveStepNumber_B2);

		// --- connect to morphing wavetable oscillators
		modMatrix->addModDestination(kDestOsc1_Morph, wsOscillator[MAIN_OSC]->getWTOscillator(0)->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		modMatrix->addModDestination(kDestOsc2_Morph, wsOscillator[MAIN_OSC]->getWTOscillator(1)->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		modMatrix->addModDestination(kDestOsc3_Morph, wsOscillator[MAIN_OSC]->getWTOscillator(2)->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		modMatrix->addModDestination(kDestOsc4_Morph, wsOscillator[MAIN_OSC]->getWTOscillator(3)->getModulationInput()->getModArrayPtr(kWaveMorphMod));

		modMatrix->addModDestination(kDestOsc5_Morph, wsOscillator[DETUNED_OSC]->getWTOscillator(0)->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		modMatrix->addModDestination(kDestOsc6_Morph, wsOscillator[DETUNED_OSC]->getWTOscillator(1)->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		modMatrix->addModDestination(kDestOsc7_Morph, wsOscillator[DETUNED_OSC]->getWTOscillator(2)->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		modMatrix->addModDestination(kDestOsc8_Morph, wsOscillator[DETUNED_OSC]->getWTOscillator(3)->getModulationInput()->getModArrayPtr(kWaveMorphMod));

		// --- unique to WS
		modMatrix->addModDestination(kDestFilterEGRetrigger, filterEG->getModulationInput()->getModArrayPtr(kTriggerMod), kMMTransformUnipolar);
		modMatrix->addModDestination(kDestAuxEGRetrigger, auxEG->getModulationInput()->getModArrayPtr(kTriggerMod), kMMTransformUnipolar);

		// --- hardwire connections unique to WS
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveMix_A, kDestOsc1_WSWaveMix_A);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveMix_B, kDestOsc1_WSWaveMix_B);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveIndex_A, kDestOsc1_WSWaveIndex_A);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveIndex_B, kDestOsc1_WSWaveIndex_B);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveAmpMod_A, kDestOsc1_WSWaveAmp_A);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveAmpMod_B, kDestOsc1_WSWaveAmp_B);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSPitchMod_A, kDestOsc1_WSWavePitch_A);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSPitchMod_B, kDestOsc1_WSWavePitch_B);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSXfadeDone, kDestOsc1WSXFadeDone);

		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveMix_A, kDestOsc2_WSWaveMix_A);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveMix_B, kDestOsc2_WSWaveMix_B);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveIndex_A, kDestOsc2_WSWaveIndex_A);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveIndex_B, kDestOsc2_WSWaveIndex_B);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveAmpMod_A, kDestOsc2_WSWaveAmp_A);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSWaveAmpMod_B, kDestOsc2_WSWaveAmp_B);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSPitchMod_A, kDestOsc2_WSWavePitch_A);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSPitchMod_B, kDestOsc2_WSWavePitch_B);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceWSXfadeDone, kDestOsc2WSXFadeDone);

		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc1_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc2_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc3_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc4_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc5_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc6_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc7_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc8_Morph);

#endif
		// --- LFOs
		modMatrix->addModDestination(kDestLFO1_fo, lfo[0]->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestLFO2_fo, lfo[1]->getModulationInput()->getModArrayPtr(kBipolarMod));

		// --- EGs
		modMatrix->addModDestination(kDestDCA_EGMod, dca->getModulationInput()->getModArrayPtr(kEGMod));
		modMatrix->addModDestination(kDestDCA_AmpMod, dca->getModulationInput()->getModArrayPtr(kMaxDownAmpMod));
		modMatrix->addModDestination(kDestDCA_PanMod, dca->getModulationInput()->getModArrayPtr(kPanMod));

		// --- FILTERS
		modMatrix->addModDestination(kDestFilter1_fc_EG, filter[0]->getModulationInput()->getModArrayPtr(kEGMod));
		modMatrix->addModDestination(kDestFilter1_fc_Bipolar, filter[0]->getModulationInput()->getModArrayPtr(kBipolarMod));
		modMatrix->addModDestination(kDestFilter2_fc_EG, filter[1]->getModulationInput()->getModArrayPtr(kEGMod));
		modMatrix->addModDestination(kDestFilter2_fc_Bipolar, filter[1]->getModulationInput()->getModArrayPtr(kBipolarMod));

		// --- EG Re-triggers
		modMatrix->addModDestination(kDestAmpEGRetrigger, ampEG->getModulationInput()->getModArrayPtr(kTriggerMod), kMMTransformUnipolar);
	
		// --- hardwired routing here
		// --- kEG1_Normal -> kDCA_EGMod
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAmpEG_Norm, kDestDCA_EGMod);
		parameters->modMatrixParameters->setMM_DestDefaultValue(kDestDCA_AmpMod, 1.0);

		// --- connect AuxEG to morph mod
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc1_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc2_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc3_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc4_Morph);

	}

	/**
	\brief
	Initialize the voice sub-components; this really only applies to PCM oscillators that need DLL path
	BUT, DLL path is avaialble for all modules and may be used in clever ways

	\param dllPath path to folder containing this DLL

	\returns true if sucessful
	*/
	bool SynthVoice::initialize(const char* dllPath)
	{
		// --- initialize all sub components that need the DLL path
#ifdef SYNTHLAB_WS
		wsOscillator[MAIN_OSC]->initialize(dllPath);
		wsOscillator[DETUNED_OSC]->initialize(dllPath);
#else
		for (uint32_t i = 0; i<NUM_OSC; i++)
			oscillator[i]->initialize(dllPath);
#endif
		
		for (uint32_t i = 0; i<NUM_LFO; i++)
			lfo[i]->initialize(dllPath);
		
		return true;
	}

	/**
	\brief
	Reset all SynthModules on init or when sample rate changes

	\param _sampleRate sample rate

	\returns true if sucessful
	*/
	bool SynthVoice::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		currentMIDINote = -1;

		// --- optional; you may disable
		dcFilter[LEFT_CHANNEL].reset(_sampleRate);
		dcFilter[RIGHT_CHANNEL].reset(_sampleRate);

		// --- reset osc
#ifdef SYNTHLAB_WS
		wsOscillator[MAIN_OSC]->reset(_sampleRate);
		wsOscillator[DETUNED_OSC]->reset(_sampleRate);
		waveSequencer->reset(_sampleRate);
#else
		for (uint32_t i = 0; i<NUM_OSC; i++)
			oscillator[i]->reset(_sampleRate);
#endif	
		for (uint32_t i = 0; i<NUM_LFO; i++)
			lfo[i]->reset(_sampleRate);

		for (uint32_t i = 0; i<NUM_FILTER; i++)
			filter[i]->reset(_sampleRate);

		ampEG->reset(_sampleRate);
		filterEG->reset(_sampleRate);
		auxEG->reset(_sampleRate);

		dca->reset(_sampleRate);
		return true;
	}

	/**
	\brief
	Update voice specific stuff.
	- the main thing this does is to load new cores as user selects them

	\returns true if sucessful
	*/
	bool SynthVoice::update()
	{
		// --- do updates to sub-components 
		//     NOTE: this is NOT for GUI control updates for normal synth operation
		//
		// ---- sets unison mode detuning and phase from GUI controls (optional)
#ifdef SYNTHLAB_WS
		wsOscillator[MAIN_OSC]->setUnisonMode(parameters->unisonDetuneCents, parameters->unisonStartPhase);
		wsOscillator[DETUNED_OSC]->setUnisonMode(parameters->unisonDetuneCents, parameters->unisonStartPhase);
#else
		for (uint32_t i = 0; i<NUM_OSC; i++)
			oscillator[i]->setUnisonMode(parameters->unisonDetuneCents, parameters->unisonStartPhase);
#endif
		// --- mode affects the EGs
		if (parameters->synthModeIndex == enumToInt(SynthMode::kLegato) ||
			parameters->synthModeIndex == enumToInt(SynthMode::kUnisonLegato))
			parameters->ampEGParameters->legatoMode = true;
		else
			parameters->ampEGParameters->legatoMode = false;

		// --- dynamic string loading
		//parameters->updateCodeDroplists = 0;
		//parameters->updateCodeKnobs = 0;

		// --- check for new modules here
		if (parameters->lfo1Parameters->moduleIndex != lfo[0]->getSelectedCoreIndex())
		{
			// --- we have a new LFO1
			loadLFOCore(1, parameters->lfo1Parameters->moduleIndex);
		}
		if (parameters->lfo2Parameters->moduleIndex != lfo[1]->getSelectedCoreIndex())
		{
			// --- we have a new LFO2
			loadLFOCore(2, parameters->lfo2Parameters->moduleIndex);
		}

		if (parameters->ampEGParameters->moduleIndex != ampEG->getSelectedCoreIndex())
		{
			// --- we have a new AmpEG
			loadEGCore(1, parameters->ampEGParameters->moduleIndex);
		}
		if (parameters->filterEGParameters->moduleIndex != filterEG->getSelectedCoreIndex())
		{
			// --- we have a new filterEG
			loadEGCore(2, parameters->filterEGParameters->moduleIndex);
		}
		if (parameters->auxEGParameters->moduleIndex != auxEG->getSelectedCoreIndex())
		{
			// --- we have a new auxEG
			loadEGCore(3, parameters->auxEGParameters->moduleIndex);
		}

		if (parameters->filter1Parameters->moduleIndex != filter[0]->getSelectedCoreIndex())
		{
			// --- we have a new fFilter 1
			loadFilterCore(1, parameters->filter1Parameters->moduleIndex);
		}
		if (parameters->filter2Parameters->moduleIndex != filter[1]->getSelectedCoreIndex())
		{
			// --- we have a new filter 2
			loadFilterCore(2, parameters->filter2Parameters->moduleIndex);
		}

#ifndef SYNTHLAB_WS
		if (parameters->osc1Parameters->moduleIndex != oscillator[0]->getSelectedCoreIndex())
		{
			// --- we have a new OSC1
			loadOscCore(1, parameters->osc1Parameters->moduleIndex);
		}
		if (parameters->osc2Parameters->moduleIndex != oscillator[1]->getSelectedCoreIndex())
		{
			// --- we have a new OSC2
			loadOscCore(2, parameters->osc2Parameters->moduleIndex);
		}
		if (parameters->osc3Parameters->moduleIndex != oscillator[2]->getSelectedCoreIndex())
		{
			// --- we have a new OSC3
			loadOscCore(3, parameters->osc3Parameters->moduleIndex);
		}
		if (parameters->osc4Parameters->moduleIndex != oscillator[3]->getSelectedCoreIndex())
		{
			// --- we have a new OSC4
			loadOscCore(4, parameters->osc4Parameters->moduleIndex);
		}
#endif
		return true;
	}

	/**
	\brief
	Accumulate buffers into voice's mix buffers

	\param oscBuffers buffers to add to the mix buffer
	\param samplesInBlock samples in this block to accumulate
	\param scaling scaling mix coefficient if needed

	*/
	void SynthVoice::accumulateToMixBuffer(std::shared_ptr<AudioBuffer> oscBuffers, uint32_t samplesInBlock, double scaling)
	{
		float* leftOutBuffer = mixBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOutBuffer = mixBuffers->getOutputBuffer(RIGHT_CHANNEL);
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
	\brief
	Write buffer into voice's mix buffers; unlike accumulate, this overwrites the audio data

	\param oscBuffers buffers to overwrite into the mix buffer
	\param samplesInBlock samples in this block to accumulate
	\param scaling scaling mix coefficient if needed

	*/
	void SynthVoice::writeToMixBuffer(std::shared_ptr<AudioBuffer> oscBuffers, uint32_t samplesInBlock, double scaling)
	{
		float* leftOutBuffer = mixBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOutBuffer = mixBuffers->getOutputBuffer(RIGHT_CHANNEL);
		float* leftOscBuffer = oscBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOscBuffer = oscBuffers->getOutputBuffer(RIGHT_CHANNEL);

		for (uint32_t i = 0; i < samplesInBlock; i++)
		{
			// --- stereo
			leftOutBuffer[i] = leftOscBuffer[i] * scaling;
			rightOutBuffer[i] = rightOscBuffer[i] * scaling;
		}
	}

	/**
	\brief
	Render a block of audio data for an active note event
	- call the render function on modulators
	- run the modulation matrix
	- call the render function on mod targets
	- move audio output from oscillators to filters
	- move audio output from filters to DCA
	- move audio output from DCA to main mix buffers
	- check the status of the Amp EG object to see if note has expired
	- set steal-pending flag if voice is to be taken for next event

	\param synthProcessInfo structure of data needed for rendering this block.

	*/
	bool SynthVoice::render(SynthProcessInfo& synthProcessInfo)
	{
		uint32_t samplesToProcess = synthProcessInfo.getSamplesInBlock();

		// --- clear for accumulation
		mixBuffers->flushBuffers();

		// --- render modulators first
		for (uint32_t i = 0; i<NUM_LFO; i++)
			lfo[i]->render(samplesToProcess);

		// --- EGs
		ampEG->render(samplesToProcess);
		filterEG->render(samplesToProcess);
		auxEG->render(samplesToProcess);

#ifdef SYNTHLAB_WS
		// --- sequencer generates modulation values
		waveSequencer->render(samplesToProcess);
#endif

		// --- run modulation matrix; this does all block modultion routing 
		//     sources -> destinations
		modMatrix->runModMatrix();

#ifdef SYNTHLAB_WS
		// --- render the 4 oscillatorsin one object
		// --- wave sequencer
		wsOscillator[MAIN_OSC]->render(samplesToProcess);
		wsOscillator[DETUNED_OSC]->render(samplesToProcess);

		// --- send to mix buffer
		accumulateToMixBuffer(wsOscillator[MAIN_OSC]->getAudioBuffers(), samplesToProcess, 0.5);
		accumulateToMixBuffer(wsOscillator[DETUNED_OSC]->getAudioBuffers(), samplesToProcess, 0.5);

#elif defined SYNTHLAB_DX
		// --- render the 4 oscillators
		// --- FM1 = *4->3->2->1---->out
		if (parameters->fmAlgorithmIndex == enumToInt(DX100Algo::kFM1))
		{
			oscillator[3]->clearFMBuffer();
			oscillator[3]->render(samplesToProcess);

			oscillator[2]->setFMBuffer(oscillator[3]->getAudioBuffers());
			oscillator[2]->render(samplesToProcess);

			oscillator[1]->setFMBuffer(oscillator[2]->getAudioBuffers());
			oscillator[1]->render(samplesToProcess);

			oscillator[0]->setFMBuffer(oscillator[1]->getAudioBuffers());
			oscillator[0]->render(samplesToProcess);

			writeToMixBuffer(oscillator[0]->getAudioBuffers(), samplesToProcess);
		}
		// --- FM2 = *4+3->2->1---->out
		else if (parameters->fmAlgorithmIndex == enumToInt(DX100Algo::kFM2))
		{
			// --- 4+3
			oscillator[3]->clearFMBuffer();
			oscillator[3]->render(samplesToProcess);
			accumulateToMixBuffer(oscillator[3]->getAudioBuffers(), samplesToProcess, 0.5);

			oscillator[2]->clearFMBuffer();
			oscillator[2]->render(samplesToProcess);
			accumulateToMixBuffer(oscillator[3]->getAudioBuffers(), samplesToProcess, 0.5);

			oscillator[1]->setFMBuffer(mixBuffers);
			oscillator[1]->render(samplesToProcess);

			oscillator[0]->setFMBuffer(oscillator[1]->getAudioBuffers());
			oscillator[0]->render(samplesToProcess);

			writeToMixBuffer(oscillator[0]->getAudioBuffers(), samplesToProcess);
		}
		// --- FM3 = 3->2(+4)->1---->out
		else if (parameters->fmAlgorithmIndex == enumToInt(DX100Algo::kFM3))
		{
			// --- 3->2
			oscillator[2]->clearFMBuffer();
			oscillator[2]->render(samplesToProcess);

			oscillator[1]->setFMBuffer(oscillator[2]->getAudioBuffers());
			oscillator[1]->render(samplesToProcess);
			accumulateToMixBuffer(oscillator[1]->getAudioBuffers(), samplesToProcess, 0.5);

			oscillator[3]->clearFMBuffer();
			oscillator[3]->render(samplesToProcess);
			accumulateToMixBuffer(oscillator[3]->getAudioBuffers(), samplesToProcess, 0.5);

			oscillator[0]->setFMBuffer(mixBuffers);
			oscillator[0]->render(samplesToProcess);

			writeToMixBuffer(oscillator[0]->getAudioBuffers(), samplesToProcess);
		}
		// --- FM4 = 4->3(+2)->1---->out
		else if (parameters->fmAlgorithmIndex == enumToInt(DX100Algo::kFM4))
		{
			// --- render 4
			oscillator[3]->clearFMBuffer();
			oscillator[3]->render(samplesToProcess);

			// --- 4 modulates 3
			oscillator[2]->setFMBuffer(oscillator[3]->getAudioBuffers());
			oscillator[2]->render(samplesToProcess);

			// --- render 2
			oscillator[1]->clearFMBuffer();
			oscillator[1]->render(samplesToProcess);

			// --- sum the two modulator outputs
			accumulateToMixBuffer(oscillator[2]->getAudioBuffers(), samplesToProcess, 0.5);
			accumulateToMixBuffer(oscillator[1]->getAudioBuffers(), samplesToProcess, 0.5);

			// --- apply to final operator
			oscillator[0]->setFMBuffer(mixBuffers);
			oscillator[0]->render(samplesToProcess);

			// --- write output
			writeToMixBuffer(oscillator[0]->getAudioBuffers(), samplesToProcess);
		}
		// --- FM5 = (3->1) + (4->2) ---- out
		else if (parameters->fmAlgorithmIndex == enumToInt(DX100Algo::kFM5))
		{
			// --- 3->1
			oscillator[2]->clearFMBuffer();
			oscillator[2]->render(samplesToProcess);

			oscillator[0]->setFMBuffer(oscillator[2]->getAudioBuffers());
			oscillator[0]->render(samplesToProcess);

			// --- 4->2
			oscillator[3]->clearFMBuffer();
			oscillator[3]->render(samplesToProcess);

			oscillator[1]->setFMBuffer(oscillator[3]->getAudioBuffers());
			oscillator[1]->render(samplesToProcess);

			// --- sum
			accumulateToMixBuffer(oscillator[0]->getAudioBuffers(), samplesToProcess, 0.5);
			accumulateToMixBuffer(oscillator[1]->getAudioBuffers(), samplesToProcess, 0.5);
		}
		// --- FM6 = 4 -> (1 + 2 + 3)
		else if (parameters->fmAlgorithmIndex == enumToInt(DX100Algo::kFM6))
		{
			// --- 4
			oscillator[3]->clearFMBuffer();
			oscillator[3]->render(samplesToProcess);

			// --- 4 -> 3,2,1
			oscillator[2]->setFMBuffer(oscillator[3]->getAudioBuffers());
			oscillator[1]->setFMBuffer(oscillator[3]->getAudioBuffers());
			oscillator[0]->setFMBuffer(oscillator[3]->getAudioBuffers());

			oscillator[2]->render(samplesToProcess);
			oscillator[1]->render(samplesToProcess);
			oscillator[0]->render(samplesToProcess);

			// --- sum
			accumulateToMixBuffer(oscillator[2]->getAudioBuffers(), samplesToProcess, 0.333);
			accumulateToMixBuffer(oscillator[1]->getAudioBuffers(), samplesToProcess, 0.333);
			accumulateToMixBuffer(oscillator[0]->getAudioBuffers(), samplesToProcess, 0.333);
		}
		// --- FM7 = 4 -> (3) + 2 + 1)
		else if (parameters->fmAlgorithmIndex == enumToInt(DX100Algo::kFM7))
		{
			// --- 4 -> 3
			oscillator[3]->clearFMBuffer();
			oscillator[3]->render(samplesToProcess);

			oscillator[2]->setFMBuffer(oscillator[3]->getAudioBuffers());
			oscillator[2]->render(samplesToProcess);

			// --- 2 + 1
			oscillator[1]->clearFMBuffer();
			oscillator[1]->render(samplesToProcess);

			oscillator[0]->clearFMBuffer();
			oscillator[0]->render(samplesToProcess);

			// --- sum
			accumulateToMixBuffer(oscillator[2]->getAudioBuffers(), samplesToProcess, 0.333);
			accumulateToMixBuffer(oscillator[1]->getAudioBuffers(), samplesToProcess, 0.333);
			accumulateToMixBuffer(oscillator[0]->getAudioBuffers(), samplesToProcess, 0.333);
		}
		// --- FM8 = (1 + 2 + 3 + 4)---- out
		else if (parameters->fmAlgorithmIndex == enumToInt(DX100Algo::kFM8))
		{
			// --- all 
			oscillator[3]->clearFMBuffer();
			oscillator[2]->clearFMBuffer();
			oscillator[1]->clearFMBuffer();
			oscillator[0]->clearFMBuffer();

			oscillator[3]->render(samplesToProcess);
			oscillator[2]->render(samplesToProcess);
			oscillator[1]->render(samplesToProcess);
			oscillator[0]->render(samplesToProcess);

			// --- sum
			accumulateToMixBuffer(oscillator[3]->getAudioBuffers(), samplesToProcess, 0.25);
			accumulateToMixBuffer(oscillator[2]->getAudioBuffers(), samplesToProcess, 0.25);
			accumulateToMixBuffer(oscillator[1]->getAudioBuffers(), samplesToProcess, 0.25);
			accumulateToMixBuffer(oscillator[0]->getAudioBuffers(), samplesToProcess, 0.25);
		}
#else // all others
		// --- render the 4 oscillators
		for (uint32_t i = 0; i < NUM_OSC; i++)
		{
			oscillator[i]->render(samplesToProcess);
			accumulateToMixBuffer(oscillator[i]->getAudioBuffers(), samplesToProcess, 0.25);
		}

		// --- you may comment this out if needed
		//     many cobbled wavetables have small DC offsets that add up over time
		removeMixBufferDC(samplesToProcess);
#endif

		// --- setup filtering
		if (parameters->filterModeIndex == enumToInt(FilterMode::kSeries))
		{
			// --- to Filter1
			copyBufferToInput(mixBuffers, filter[0]->getAudioBuffers(), STEREO_TO_STEREO, samplesToProcess);

			// --- update and render
			filter[0]->render(samplesToProcess);

			// --- to Filter2
			copyOutputToInput(filter[0]->getAudioBuffers(), filter[1]->getAudioBuffers(), STEREO_TO_STEREO, samplesToProcess);

			// --- update and render
			filter[1]->render(samplesToProcess);

			// --- to DCA
			copyOutputToInput(filter[1]->getAudioBuffers(), dca->getAudioBuffers(), STEREO_TO_STEREO, samplesToProcess);
		}
		else
		{
			// --- to Filter1
			copyBufferToInput(mixBuffers, filter[0]->getAudioBuffers(), STEREO_TO_STEREO, samplesToProcess);

			// --- to Filter2
			copyBufferToInput(mixBuffers, filter[1]->getAudioBuffers(), STEREO_TO_STEREO, samplesToProcess);

			// --- clear mix buffers so we can mix the filter outputs into it
			mixBuffers->flushBuffers();

			// --- update and render
			filter[0]->render(samplesToProcess);
			accumulateToMixBuffer(filter[0]->getAudioBuffers(), samplesToProcess, 0.5);

			// --- update and render
			filter[1]->render(samplesToProcess);
			accumulateToMixBuffer(filter[1]->getAudioBuffers(), samplesToProcess, 0.5);

			// --- to DCA
			copyBufferToInput(mixBuffers, dca->getAudioBuffers(), STEREO_TO_STEREO, samplesToProcess);
		}

		// --- update and render
		dca->render(samplesToProcess);

		// --- to mains
		copyOutputToOutput(dca->getAudioBuffers(), synthProcessInfo, STEREO_TO_STEREO, samplesToProcess);

		// --- check for note off condition
		if (voiceIsActive)
		{
			if (ampEG->getState() == enumToInt(EGState::kOff))
			{				
#ifdef SYNTHLAB_WS
				// --- turn off LEDs
				waveSequencer->clearStatusArray();
#endif
				// --- check for steal pending
				if (stealPending)
				{
					// --- turn off old note event
					doNoteOff(voiceMIDIEvent);

					// --- load new note info
					voiceMIDIEvent = voiceStealMIDIEvent;

					// --- turn on the new note
					doNoteOn(voiceMIDIEvent);

					// --- stealing accomplished!
					stealPending = false;
				}
				else
					voiceIsActive = false;
			}
		}
		return true;
	}

	/**
	\brief
	Note-on handler for voice
	- For oscillators: start glide modulators then call note-on handlers
	- For all others: call note-on handlers
	- set and save voice state information

	\param event MIDI note event

	*/
	bool SynthVoice::doNoteOn(midiEvent& event)
	{
		// --- calculate MIDI -> pitch value
		double midiPitch = midiNoteNumberToOscFrequency(event.midiData1);
		int32_t lastMIDINote = currentMIDINote;
		currentMIDINote = (int32_t)event.midiData1;

		MIDINoteEvent noteEvent(midiPitch, event.midiData1, event.midiData2);
		
		// --- create glide info structure out of notes and times
		GlideInfo glideInfo(lastMIDINote, currentMIDINote, parameters->glideTime_mSec, sampleRate);

		// --- set glide mod
#ifdef SYNTHLAB_WS
		wsOscillator[MAIN_OSC]->startGlideModulation(glideInfo);
		wsOscillator[DETUNED_OSC]->startGlideModulation(glideInfo);

		wsOscillator[MAIN_OSC]->doNoteOn(noteEvent);
		wsOscillator[DETUNED_OSC]->doNoteOn(noteEvent);

		waveSequencer->doNoteOn(noteEvent);
#else
		for (uint32_t i = 0; i < NUM_OSC; i++)
		{
			oscillator[i]->startGlideModulation(glideInfo);
			oscillator[i]->doNoteOn(noteEvent);
		}
#endif
		// --- needed forLFO  modes
		for (uint32_t i = 0; i < NUM_LFO; i++)
			lfo[i]->doNoteOn(noteEvent);

		// --- DCA
		dca->doNoteOn(noteEvent);

		for (uint32_t i = 0; i<NUM_FILTER; i++)
			filter[i]->doNoteOn(noteEvent);

		// --- EGs
		ampEG->doNoteOn(noteEvent);
		filterEG->doNoteOn(noteEvent);
		auxEG->doNoteOn(noteEvent);

		// --- set the flag
		voiceIsActive = true; // we are ON
		voiceNoteState = voiceState::kNoteOnState;

		// --- this saves the midi note number and velocity so that we can identify our own note
		voiceMIDIEvent = event;

		return true;
	}

	/**
	\brief
	Note-off handler for voice
	- just forwards note-off handling to sub-objects

	\param event MIDI note event
	*/
	bool SynthVoice::doNoteOff(midiEvent& event)
	{
		// --- lookup MIDI -> pitch value
		double midiPitch = midiNoteNumberToOscFrequency(event.midiData1);

		MIDINoteEvent noteEvent(midiPitch, event.midiData1, event.midiData2);

#ifdef SYNTHLAB_WS
		wsOscillator[MAIN_OSC]->doNoteOff(noteEvent);
		wsOscillator[DETUNED_OSC]->doNoteOff(noteEvent);

		// --- not really needed
		waveSequencer->doNoteOff(noteEvent);
#else		// --- subcomponents
		for (uint32_t i = 0; i<NUM_OSC; i++)
			oscillator[i]->doNoteOff(noteEvent);
#endif
		for (uint32_t i = 0; i<NUM_FILTER; i++)
			filter[i]->doNoteOff(noteEvent);

		ampEG->doNoteOff(noteEvent);
		filterEG->doNoteOff(noteEvent);
		auxEG->doNoteOff(noteEvent);

		// --- set our current state; the ampEG will determine the final state
		voiceNoteState = voiceState::kNoteOffState;
		
		return true;
	}

	/**
	\brief
	MIDI Event handler
	- decodes MIDI message and processes note-on and note-off events only
	- all other MIDI messages are decoded and data stored in Synth Engine prior to calling this function

	\param event MIDI note event
	*/
	bool SynthVoice::processMIDIEvent(midiEvent& event)
	{
		// --- the voice only needs to process note on and off
		//     Other MIDI info such as CC can be found in global midi table via our midiData interface
		if (event.midiMessage == NOTE_ON)
		{
			// --- clear timestamp
			clearTimestamp();

			// --- call the subfunction
			doNoteOn(event);
		}
		else if (event.midiMessage == NOTE_OFF)
		{
			// --- call the subfunction
			doNoteOff(event);
		}

		return true;
	}

	/**
	\brief
	Update custom GUI codes; this is optional and only used as an example for keeping track of custom GUI
	events and needs. Use at your own risk:
	- will be very dependent on your coding style
	- will be very dependent on your framework's GUI

	\param event MIDI note event
	*/
	void SynthVoice::setAllCustomUpdateCodes()
	{
		// --- initialize to true
		parameters->updateCodeDroplists |= LFO1_WAVEFORMS;
		parameters->updateCodeDroplists |= LFO2_WAVEFORMS;
		parameters->updateCodeDroplists |= OSC1_WAVEFORMS;
		parameters->updateCodeDroplists |= OSC2_WAVEFORMS;
		parameters->updateCodeDroplists |= OSC3_WAVEFORMS;
		parameters->updateCodeDroplists |= OSC4_WAVEFORMS;
		parameters->updateCodeDroplists |= EG1_CONTOUR;
		parameters->updateCodeDroplists |= EG2_CONTOUR;
		parameters->updateCodeDroplists |= EG3_CONTOUR;
		parameters->updateCodeDroplists |= FILTER1_TYPES;
		parameters->updateCodeDroplists |= FILTER2_TYPES;
		parameters->updateCodeDroplists |= WAVE_SEQ_WAVES_1;
		parameters->updateCodeDroplists |= WAVE_SEQ_WAVES_2;
		parameters->updateCodeDroplists |= WAVE_SEQ_WAVES_3;
		parameters->updateCodeDroplists |= WAVE_SEQ_WAVES_4;
		parameters->updateCodeDroplists |= WAVE_SEQ_WAVES_5;
		parameters->updateCodeDroplists |= WAVE_SEQ_WAVES_6;
		parameters->updateCodeDroplists |= WAVE_SEQ_WAVES_7;
		parameters->updateCodeDroplists |= WAVE_SEQ_WAVES_8;

		parameters->updateCodeKnobs |= LFO1_MOD_KNOBS;
		parameters->updateCodeKnobs |= LFO2_MOD_KNOBS;
		parameters->updateCodeKnobs |= OSC1_MOD_KNOBS;
		parameters->updateCodeKnobs |= OSC2_MOD_KNOBS;
		parameters->updateCodeKnobs |= OSC3_MOD_KNOBS;
		parameters->updateCodeKnobs |= OSC4_MOD_KNOBS;

		parameters->updateCodeKnobs |= EG1_MOD_KNOBS;
		parameters->updateCodeKnobs |= EG2_MOD_KNOBS;
		parameters->updateCodeKnobs |= EG3_MOD_KNOBS;
		parameters->updateCodeKnobs |= FILTER1_MOD_KNOBS;
		parameters->updateCodeKnobs |= FILTER2_MOD_KNOBS;


	}

	/**
	\brief
	Get function that returns module core NAMES for a given type of module
	- optional: only needed if you allow dynamic loading of strings
	- will be very dependent on your framework's GUI
	- only need to query the first item in any set since they have the same attributes

	\param moduleType enumerated unsigned int that encodes the type of module for the query
	*/
	std::vector<std::string> SynthVoice::getModuleCoreNames(uint32_t moduleType)
	{
		std::vector<std::string> moduleCoreNames;

		// --- ADD: EG
		if (moduleType == LFO_MODULE)
		{
			lfo[0]->getModuleCoreStrings(moduleCoreNames);
		}
		if (moduleType == FILTER_MODULE)
		{
			filter[0]->getModuleCoreStrings(moduleCoreNames);
		}

		if (moduleType == EG_MODULE)
		{
			ampEG->getModuleCoreStrings(moduleCoreNames);
		}

#ifdef SYNTHLAB_WT
		if (moduleType == WTO_MODULE)
		{
			oscillator[0]->getModuleCoreStrings(moduleCoreNames);
		}
#endif

#ifdef SYNTHLAB_WS
		if (moduleType == WTO_MODULE)
		{
			wsOscillator[MAIN_OSC]->getWTOscillator(0)->getModuleCoreStrings(moduleCoreNames);
		}
#endif

#ifdef SYNTHLAB_VA
		if (moduleType == VAO_MODULE)
		{
			oscillator[0]->getModuleCoreStrings(moduleCoreNames);
		}
#endif

#ifdef SYNTHLAB_PCM
		if (moduleType == PCMO_MODULE)
		{
			oscillator[0]->getModuleCoreStrings(moduleCoreNames);
		}
#endif

#ifdef SYNTHLAB_KS
		if (moduleType == KSO_MODULE)
		{
			oscillator[0]->getModuleCoreStrings(moduleCoreNames);
		}
#endif

#ifdef SYNTHLAB_DX
		if (moduleType == FMO_MODULE)
		{
			oscillator[0]->getModuleCoreStrings(moduleCoreNames);
		}
#endif

			return moduleCoreNames;
	}

	/**
	\brief
	Get function that returns module core Strings for a given type of module
	- optional: only needed if you allow dynamic loading of strings
	- will be very dependent on your framework's GUI
	- only need to query the first item in any set since they have the same attributes

	\param mask mask variable for decoding the type of query
	\param modKnobs true if querying for mod knob label strings
	*/
	std::vector<std::string> SynthVoice::getModuleStrings(uint32_t mask, bool modKnobs)
	{
		std::vector<std::string> strings;
		if (modKnobs)
		{
			if (mask == LFO1_MOD_KNOBS)
				lfo[0]->getModKnobStrings(strings);
			else if (mask == LFO2_MOD_KNOBS)
				lfo[1]->getModKnobStrings(strings);

			else if (mask == EG1_MOD_KNOBS)
				ampEG->getModKnobStrings(strings);
			else if (mask == EG2_MOD_KNOBS)
				filterEG->getModKnobStrings(strings);
			else if (mask == EG3_MOD_KNOBS)
				auxEG->getModKnobStrings(strings);

			else if (mask == FILTER1_MOD_KNOBS)
				filter[0]->getModKnobStrings(strings);
			else if (mask == FILTER2_MOD_KNOBS)
				filter[1]->getModKnobStrings(strings);
#ifdef SYNTHLAB_WS
			else if (mask == OSC1_MOD_KNOBS)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getModKnobStrings(strings);
			else if (mask == OSC2_MOD_KNOBS)
				wsOscillator[MAIN_OSC]->getWTOscillator(1)->getModKnobStrings(strings);
			else if (mask == OSC3_MOD_KNOBS)
				wsOscillator[MAIN_OSC]->getWTOscillator(2)->getModKnobStrings(strings);
			else if (mask == OSC4_MOD_KNOBS)
				wsOscillator[MAIN_OSC]->getWTOscillator(3)->getModKnobStrings(strings);
#else		
			// --- subcomponents
			else if (mask == OSC1_MOD_KNOBS)
				oscillator[0]->getModKnobStrings(strings);
			else if (mask == OSC2_MOD_KNOBS)
				oscillator[1]->getModKnobStrings(strings);
			else if (mask == OSC3_MOD_KNOBS)
				oscillator[2]->getModKnobStrings(strings);
			else if (mask == OSC4_MOD_KNOBS)
				oscillator[3]->getModKnobStrings(strings);
#endif
		}
		else
		{
			if (mask == LFO1_WAVEFORMS)
				lfo[0]->getModuleStrings(strings);
			else if (mask == LFO2_WAVEFORMS)
				lfo[1]->getModuleStrings(strings);

			else if (mask == EG1_CONTOUR)
				ampEG->getModuleStrings(strings);
			else if (mask == EG2_CONTOUR)
				filterEG->getModuleStrings(strings);
			else if (mask == EG3_CONTOUR)
				auxEG->getModuleStrings(strings);

			else if (mask == FILTER1_TYPES)
				filter[0]->getModuleStrings(strings);
			else if (mask == FILTER2_TYPES)
				filter[1]->getModuleStrings(strings);

#ifdef SYNTHLAB_WS
			else if (mask == OSC1_MOD_KNOBS)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getModKnobStrings(strings);
			else if (mask == OSC2_MOD_KNOBS)
				wsOscillator[MAIN_OSC]->getWTOscillator(1)->getModKnobStrings(strings);
			else if (mask == OSC3_MOD_KNOBS)
				wsOscillator[MAIN_OSC]->getWTOscillator(2)->getModKnobStrings(strings);
			else if (mask == OSC4_MOD_KNOBS)
				wsOscillator[MAIN_OSC]->getWTOscillator(3)->getModKnobStrings(strings);

			else if (mask == WAVE_SEQ_WAVES_1)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getAllModuleStrings(strings, empty_string); // gets all currrent core strings //
			else if (mask == WAVE_SEQ_WAVES_2)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getAllModuleStrings(strings, empty_string); // gets all currrent core strings //
			else if (mask == WAVE_SEQ_WAVES_3)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getAllModuleStrings(strings, empty_string); // gets all currrent core strings //
			else if (mask == WAVE_SEQ_WAVES_4)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getAllModuleStrings(strings, empty_string); // gets all currrent core strings //
			else if (mask == WAVE_SEQ_WAVES_5)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getAllModuleStrings(strings, empty_string); // gets all currrent core strings //
			else if (mask == WAVE_SEQ_WAVES_6)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getAllModuleStrings(strings, empty_string); // gets all currrent core strings //
			else if (mask == WAVE_SEQ_WAVES_7)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getAllModuleStrings(strings, empty_string); // gets all currrent core strings //
			else if (mask == WAVE_SEQ_WAVES_8)
				wsOscillator[MAIN_OSC]->getWTOscillator(0)->getAllModuleStrings(strings, empty_string); // gets all currrent core strings //

#else		
			else if (mask == OSC1_WAVEFORMS)
				oscillator[0]->getModuleStrings(strings);
			else if (mask == OSC2_WAVEFORMS)
				oscillator[1]->getModuleStrings(strings);
			else if (mask == OSC3_WAVEFORMS)
				oscillator[2]->getModuleStrings(strings);
			else if (mask == OSC4_WAVEFORMS)
				oscillator[3]->getModuleStrings(strings);
#endif
		}

		return strings;
	}


	/**
	\brief
	Function to add dynamically loaded cores (DLLs) at load-time. These are added to the SynthModule's core
	array. 

	\param modules a vector of freshly created ModuleCores that were parsed at load-time
	*/
	void SynthVoice::setDynamicModules(std::vector<std::shared_ptr<SynthLab::ModuleCore>> modules)//std::vector<ModuleCore*> modules)
	{
		uint32_t lfo_L = NUM_LFO;
		uint32_t eg_L = NUM_EG;
		uint32_t filter_L = NUM_FILTER;
		uint32_t osc_L = NUM_OSCILLATORS;
		uint32_t wsosc_L = 2;

		uint32_t lfo_C = 0;
		uint32_t eg_C = 0;
		uint32_t filter_C = 0;
		uint32_t osc_C = 0;
		uint32_t wsosc_C = 0;
		uint32_t osc_C2 = 0;

		uint32_t numModules = modules.size();
		for (uint32_t i = 0; i < numModules; i++)
		{
			std::shared_ptr<SynthLab::ModuleCore> unit = modules[i];
			uint32_t type = unit->getModuleType();

			if (type == LFO_MODULE && lfo_C < (NUM_LFO * NUM_MODULE_CORES))
			{
				uint32_t index = lfo_C - lfo_L*((uint32_t)((float)lfo_C / (float)lfo_L));
				lfo[index]->addModuleCore(unit);
				lfo_C++;
			}

			if (type == FILTER_MODULE && filter_C < (NUM_FILTER * NUM_MODULE_CORES))
			{
				uint32_t index = filter_C - filter_L*((uint32_t)((float)filter_C / (float)filter_L));
				filter[index]->addModuleCore(unit);
				filter_C++;
			}

			if (type == EG_MODULE && eg_C < (NUM_EG * NUM_MODULE_CORES))
			{
				uint32_t index = eg_C - eg_L*((uint32_t)((float)eg_C / (float)eg_L));
				if (index == 0) ampEG->addModuleCore(unit);
				else if (index == 1) filterEG->addModuleCore(unit);
				else if (index == 2) auxEG->addModuleCore(unit);
				eg_C++;
			}

#ifdef SYNTHLAB_WT
			if (type == WTO_MODULE && osc_C < (NUM_OSC * NUM_MODULE_CORES))
			{
				uint32_t index = osc_C - osc_L*((uint32_t)((float)osc_C / (float)osc_L));
				oscillator[index]->addModuleCore(unit);
				osc_C++;
			}
#endif

#ifdef SYNTHLAB_PCM
			if (type == PCMO_MODULE && osc_C < (NUM_OSC * NUM_MODULE_CORES))
			{
				uint32_t index = osc_C - osc_L*((uint32_t)((float)osc_C / (float)osc_L));
				oscillator[index]->addModuleCore(unit);
				osc_C++;
			}
#endif

#ifdef SYNTHLAB_VA
			if (type == VAO_MODULE && osc_C < (NUM_OSC * NUM_MODULE_CORES))
			{
				uint32_t index = osc_C - osc_L*((uint32_t)((float)osc_C / (float)osc_L));
				oscillator[index]->addModuleCore(unit);
				osc_C++;
		}
#endif

#ifdef SYNTHLAB_WS
			// --- each has four oscillators inside. but doubled for WS only
			if (type == WTO_MODULE && osc_C2 < (NUM_WS_OSC * NUM_OSC* NUM_MODULE_CORES))
			{
				uint32_t index = ((uint32_t)((float)osc_C / (float)osc_L));
				uint32_t index2 = wsosc_C - osc_L*((uint32_t)((float)wsosc_C / (float)osc_L));				
				wsOscillator[index]->getWTOscillator(index2)->addModuleCore(unit);
				
				if (index2 == (NUM_OSCILLATORS-1) && index == (NUM_WS_OSC-1)) osc_C = 0;
				else osc_C++;

				osc_C2++;
				wsosc_C++;
			}
#endif

#ifdef SYNTHLAB_KS
			if (type == KSO_MODULE && osc_C < (NUM_OSC * NUM_MODULE_CORES))
			{
				uint32_t index = osc_C - osc_L*((uint32_t)((float)osc_C / (float)osc_L));
				oscillator[index]->addModuleCore(unit);
				osc_C++;
			}
#endif
#ifdef SYNTHLAB_DX
			if (type == FMO_MODULE && osc_C < (NUM_OSC * NUM_MODULE_CORES))
			{
				uint32_t index = osc_C - osc_L*((uint32_t)((float)osc_C / (float)osc_L));
				oscillator[index]->addModuleCore(unit);
				osc_C++;
			}
#endif
		}

		// --- need to guarantee that the 
		for (uint32_t i = 0; i < NUM_LFO; i++) {
			lfo[i]->packCores();
		}
		for (uint32_t i = 0; i < NUM_FILTER; i++) {
			filter[i]->packCores();
		}
#ifdef SYNTHLAB_WS
		for (uint32_t j = 0; j < NUM_WS_OSC; j++) 
		{
			for (uint32_t i = 0; i < NUM_OSC; i++)
			{
				wsOscillator[j]->getWTOscillator(i)->packCores();
			}
		}

#else
		for (uint32_t i = 0; i < NUM_OSC; i++) {
			oscillator[i]->packCores();
		}
#endif
		ampEG->packCores();
		filterEG->packCores();
		auxEG->packCores();
	}

	/**
	\brief
	Function to load a new LFO Core
	- optional, only for systems that allow dynamic GUIs

	\param lfoIndex index of LFO (0 or 1 as there are 2 LFOs)
	\param index Core index parameter (0, 1, 2 or 3 as there are 4 cores)
	*/
	void SynthVoice::loadLFOCore(uint32_t lfoIndex, uint32_t index)
	{
		if (lfoIndex == 1)
		{
			lfo[0]->selectModuleCore(index);
			parameters->updateCodeDroplists |= LFO1_WAVEFORMS;
			parameters->updateCodeKnobs |= LFO1_MOD_KNOBS;

			// --- THIS IS NOT NEEDED - remove; the modulation input and output array pointers
			//     DO NOT CHANGE when a new core is loaded.
			modMatrix->clearModSource(kSourceLFO1_Norm);
			modMatrix->addModSource(kSourceLFO1_Norm, lfo[0]->getModulationOutput()->getModArrayPtr(kLFONormalOutput));

			modMatrix->clearModDestination(kDestLFO1_fo);
			modMatrix->addModDestination(kDestLFO1_fo, lfo[0]->getModulationInput()->getModArrayPtr(kBipolarMod));
		}
		else if (lfoIndex == 2)
		{
			lfo[1]->selectModuleCore(index);
			parameters->updateCodeDroplists |= LFO2_WAVEFORMS;
			parameters->updateCodeKnobs |= LFO2_MOD_KNOBS;

			modMatrix->clearModSource(kSourceLFO2_Norm);
			modMatrix->addModSource(kSourceLFO2_Norm, lfo[1]->getModulationOutput()->getModArrayPtr(kLFONormalOutput));

			modMatrix->clearModDestination(kDestLFO2_fo);
			modMatrix->addModDestination(kDestLFO2_fo, lfo[1]->getModulationInput()->getModArrayPtr(kBipolarMod));
		}
	}

	/**
	\brief
	Function to load a new filter Core
	- optional, only for systems that allow dynamic GUIs

	\param filterIndex index of filter (0 or 1 as there are 2 filters)
	\param index Core index parameter (0, 1, 2 or 3 as there are 4 cores)
	*/
	void SynthVoice::loadFilterCore(uint32_t filterIndex, uint32_t index)
	{
		if (filterIndex == 1)
		{
			filter[0]->selectModuleCore(index);
			parameters->updateCodeDroplists |= FILTER1_TYPES;
			parameters->updateCodeKnobs |= FILTER1_MOD_KNOBS;

			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceFilterEG_Norm, kDestFilter1_fc_EG);

			modMatrix->clearModDestination(kDestFilter1_fc_EG);
			modMatrix->clearModDestination(kDestFilter1_fc_Bipolar);

			modMatrix->addModDestination(kDestFilter1_fc_EG, filter[0]->getModulationInput()->getModArrayPtr(kEGMod));
			modMatrix->addModDestination(kDestFilter1_fc_Bipolar, filter[0]->getModulationInput()->getModArrayPtr(kBipolarMod));
		}
		else if (filterIndex == 2)
		{
			filter[1]->selectModuleCore(index);
			parameters->updateCodeDroplists |= FILTER2_TYPES;
			parameters->updateCodeKnobs |= FILTER2_MOD_KNOBS;

			modMatrix->clearModDestination(kDestFilter2_fc_EG);
			modMatrix->clearModDestination(kDestFilter2_fc_Bipolar);

			modMatrix->addModDestination(kDestFilter2_fc_EG, filter[1]->getModulationInput()->getModArrayPtr(kEGMod));
			modMatrix->addModDestination(kDestFilter2_fc_Bipolar, filter[1]->getModulationInput()->getModArrayPtr(kBipolarMod));
		}
	}

	/**
	\brief
	Function to load a new oscillator Core
	- optional, only for systems that allow dynamic GUIs

	\param oscIndex index of oscillator (0, 1, 2 or 3 as there are 4 oscillators)
	\param index Core index parameter (0, 1, 2 or 3 as there are 4 cores)
	*/
	void SynthVoice::loadOscCore(uint32_t oscIndex, uint32_t index)
	{
#ifdef SYNTHLAB_WS
		if (oscIndex == 1)
		{
			// --- OPTIONAL: Used for dynamic menus in SynthLab-DM
			//parameters->updateCodeDroplists |= OSC1_WAVEFORMS;
			parameters->updateCodeKnobs |= OSC1_MOD_KNOBS;
		}
#else
		if (oscIndex == 1)
		{
			oscillator[0]->selectModuleCore(index);

			// --- OPTIONAL: Used for dynamic menus in SynthLab-DM
			parameters->updateCodeDroplists |= OSC1_WAVEFORMS;
			parameters->updateCodeKnobs |= OSC1_MOD_KNOBS;

			// --- reset mod matrix pointers to new core modulation arrays
			modMatrix->clearModDestination(kDestOsc1_fo);
			modMatrix->clearModDestination(kDestOsc1_Mod);
			modMatrix->clearModDestination(kDestOsc1_Morph);
			modMatrix->clearModDestination(kDestOsc2_Shape);

			modMatrix->addModDestination(kDestOsc1_fo, oscillator[0]->getModulationInput()->getModArrayPtr(kBipolarMod));
			modMatrix->addModDestination(kDestOsc1_Mod, oscillator[0]->getModulationInput()->getModArrayPtr(kUniqueMod));
			modMatrix->addModDestination(kDestOsc1_Morph, oscillator[0]->getModulationInput()->getModArrayPtr(kWaveMorphMod));
			modMatrix->addModDestination(kDestOsc1_Shape, oscillator[0]->getModulationInput()->getModArrayPtr(kShapeMod));
		}
		else if (oscIndex == 2)
		{
			oscillator[1]->selectModuleCore(index);
			parameters->updateCodeDroplists |= OSC2_WAVEFORMS;
			parameters->updateCodeKnobs |= OSC2_MOD_KNOBS;

			// --- reset mod matrix pointers to new core modulation arrays
			modMatrix->clearModDestination(kDestOsc2_fo);
			modMatrix->clearModDestination(kDestOsc2_Mod);
			modMatrix->clearModDestination(kDestOsc2_Morph);
			modMatrix->clearModDestination(kDestOsc2_Shape);

			modMatrix->addModDestination(kDestOsc2_fo, oscillator[1]->getModulationInput()->getModArrayPtr(kBipolarMod));
			modMatrix->addModDestination(kDestOsc2_Mod, oscillator[1]->getModulationInput()->getModArrayPtr(kUniqueMod));
			modMatrix->addModDestination(kDestOsc2_Morph, oscillator[1]->getModulationInput()->getModArrayPtr(kWaveMorphMod));
			modMatrix->addModDestination(kDestOsc2_Shape, oscillator[1]->getModulationInput()->getModArrayPtr(kShapeMod));
		}
		if (oscIndex == 3)
		{
			oscillator[2]->selectModuleCore(index);
			parameters->updateCodeDroplists |= OSC3_WAVEFORMS;
			parameters->updateCodeKnobs |= OSC3_MOD_KNOBS;

			// --- reset mod matrix pointers to new core modulation arrays
			modMatrix->clearModDestination(kDestOsc3_fo);
			modMatrix->clearModDestination(kDestOsc3_Mod);
			modMatrix->clearModDestination(kDestOsc3_Shape);
			modMatrix->clearModDestination(kDestOsc3_Morph);

			modMatrix->addModDestination(kDestOsc3_fo, oscillator[2]->getModulationInput()->getModArrayPtr(kBipolarMod));
			modMatrix->addModDestination(kDestOsc3_Mod, oscillator[2]->getModulationInput()->getModArrayPtr(kUniqueMod));
			modMatrix->addModDestination(kDestOsc3_Shape, oscillator[2]->getModulationInput()->getModArrayPtr(kShapeMod));
			modMatrix->addModDestination(kDestOsc3_Morph, oscillator[2]->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		}
		if (oscIndex == 4)
		{
			oscillator[3]->selectModuleCore(index);
			parameters->updateCodeDroplists |= OSC4_WAVEFORMS;
			parameters->updateCodeKnobs |= OSC4_MOD_KNOBS;

			// --- reset mod matrix pointers to new core modulation arrays
			modMatrix->clearModDestination(kDestOsc4_fo);
			modMatrix->clearModDestination(kDestOsc4_Mod);
			modMatrix->clearModDestination(kDestOsc4_Shape);
			modMatrix->clearModDestination(kDestOsc4_Morph);

			modMatrix->addModDestination(kDestOsc4_fo, oscillator[3]->getModulationInput()->getModArrayPtr(kBipolarMod));
			modMatrix->addModDestination(kDestOsc4_Mod, oscillator[3]->getModulationInput()->getModArrayPtr(kUniqueMod));
			modMatrix->addModDestination(kDestOsc4_Shape, oscillator[3]->getModulationInput()->getModArrayPtr(kShapeMod));
			modMatrix->addModDestination(kDestOsc4_Morph, oscillator[3]->getModulationInput()->getModArrayPtr(kWaveMorphMod));
		}

		// --- morph routing
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc1_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc2_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc3_Morph);
		parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc4_Morph);
#endif
	}

	/**
	\brief
	Function to load a new EG Core
	- optional, only for systems that allow dynamic GUIs

	\param egIndex index of EG (0 or 1 as there are 2 EGs)
	\param index Core index parameter (0, 1, 2 or 3 as there are 4 cores)
	*/
	void SynthVoice::loadEGCore(uint32_t egIndex, uint32_t index)
	{
		if (egIndex == 1)
		{
			ampEG->selectModuleCore(index);
			parameters->updateCodeDroplists |= EG1_CONTOUR;
			parameters->updateCodeKnobs |= EG1_MOD_KNOBS;

			modMatrix->clearModSource(kSourceAmpEG_Norm);
			modMatrix->addModSource(kSourceAmpEG_Norm, ampEG->getModulationOutput()->getModArrayPtr(kEGNormalOutput));
			modMatrix->clearModSource(kSourceAmpEG_Bias);
			modMatrix->addModSource(kSourceAmpEG_Bias, ampEG->getModulationOutput()->getModArrayPtr(kEGBiasedOutput));
		
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAmpEG_Norm, kDestDCA_EGMod);
			parameters->modMatrixParameters->setMM_DestDefaultValue(kDestDCA_AmpMod, 1.0);
		}
		else if (egIndex == 2)
		{
			filterEG->selectModuleCore(index);
			parameters->updateCodeDroplists |= EG2_CONTOUR;
			parameters->updateCodeKnobs |= EG2_MOD_KNOBS;

			modMatrix->clearModSource(kSourceFilterEG_Norm);
			modMatrix->addModSource(kSourceFilterEG_Norm, filterEG->getModulationOutput()->getModArrayPtr(kEGNormalOutput));
			modMatrix->clearModSource(kSourceFilterEG_Bias);
			modMatrix->addModSource(kSourceFilterEG_Bias, filterEG->getModulationOutput()->getModArrayPtr(kEGBiasedOutput));
		
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceFilterEG_Norm, kDestFilter1_fc_EG);
		}
		else if (egIndex == 3)
		{
			auxEG->selectModuleCore(index);
			parameters->updateCodeDroplists |= EG3_CONTOUR;
			parameters->updateCodeKnobs |= EG3_MOD_KNOBS;

			modMatrix->clearModSource(kSourceAuxEG_Norm);
			modMatrix->addModSource(kSourceAuxEG_Norm, auxEG->getModulationOutput()->getModArrayPtr(kEGNormalOutput));
			modMatrix->clearModSource(kSourceAuxEG_Bias);
			modMatrix->addModSource(kSourceAuxEG_Bias, auxEG->getModulationOutput()->getModArrayPtr(kEGBiasedOutput));
		
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc1_Morph);
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc2_Morph);
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc3_Morph);
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc4_Morph);

#ifdef SYNTHLAB_WS
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc5_Morph);
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc6_Morph);
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc7_Morph);
			parameters->modMatrixParameters->setMM_HardwiredRouting(kSourceAuxEG_Norm, kDestOsc8_Morph);
#endif
		}
	}
}