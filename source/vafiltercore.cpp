
#include "vafiltercore.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   vafiltercore.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs
	Construction: Cores follow the same construction pattern
	- set the Module type and name parameters
	- expose the 16 module strings
	- expose the 4 mod knob label strings
	- intialize any internal variables

	Core Specific:
	- note the way this core stores families of filters
	- each filter family produces all filter outputs at once

	\returns the newly constructed object
	*/
	VAFilterCore::VAFilterCore()
	{
		moduleType = FILTER_MODULE;
		moduleName = "VAFilters";
		preferredIndex = 0; // ordering for user

		/*
			Module Strings, zero-indexed for your GUI Control:
			- No_Filter, LPF1, HPF1, APF1, SVF_LP, SVF_HP, SVF_BP, SVF_BS, Korg35_LP, Korg35_HP,
			  Moog_LP1, Moog_LP2, Moog_LP3, Moog_LP4, Diode_LP4
		*/

		// --- our filters
		coreData.moduleStrings[0] = "No Filter";	coreData.moduleStrings[8] = "Korg35 LPF";
		coreData.moduleStrings[1] = "LPF1";			coreData.moduleStrings[9] =  "Korg35 HPF";
		coreData.moduleStrings[2] = "HPF1";			coreData.moduleStrings[10] =  "Moog LPF1";
		coreData.moduleStrings[3] = "APF1";			coreData.moduleStrings[11] = "Moog LPF2";
		coreData.moduleStrings[4] = "SVF LPF";		coreData.moduleStrings[12] = "Moog LPF3";
		coreData.moduleStrings[5] = "SVF HPF";		coreData.moduleStrings[13] = "Moog LPF4";
		coreData.moduleStrings[6] = "SVF BPF";		coreData.moduleStrings[14] = "Diode LPF4";
		coreData.moduleStrings[7] = "SVF BSF";		coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A] = "Key Track";
		coreData.modKnobStrings[MOD_KNOB_B] = "Drive";
		coreData.modKnobStrings[MOD_KNOB_C] = "EG Int";
		coreData.modKnobStrings[MOD_KNOB_D] = "BP Int";
	}


	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- reset all filters
	- reset limiters

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool VAFilterCore::reset(CoreProcData& processInfo)
	{
		for (uint32_t i = 0; i < STEREO_CHANNELS; i++)
		{
			va1[i].reset(processInfo.sampleRate);
			svf[i].reset(processInfo.sampleRate);
			korg35[i].reset(processInfo.sampleRate);
			moog[i].reset(processInfo.sampleRate);
			diode[i].reset(processInfo.sampleRate);

			// --- output limiter
			limiter[i].reset(processInfo.sampleRate);
			limiter[i].setThreshold_dB(-1.0);
		}

		// --- OPTIONAL flag for dual mono operation (conserves CPU)
		forceDualMonoFilters = processInfo.midiInputData->getAuxDAWDataUINT(kDualMonoFilters) == 1;

		return true;
	}


	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- calculates the fc modulation value from GUI controls, input modulators kBipolarMod, and kEGMod
	- monitors key-tracking to adjust fc again if needed
	- calcualtes new filter coeffients once, then copies into reolicated filters (left/right)
	- calculates final gain and drive values

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool VAFilterCore::update(CoreProcData& processInfo)
	{
		// --- parameters
		FilterParameters* parameters = static_cast<FilterParameters*>(processInfo.moduleParameters);

		// --- filter drive
		parameters->filterDrive = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_B], 1.0, 10.0);

		// --- MIDI modulation via CC 75 (filter fc)
		//
		// --- homework

		// --- bipolar freqmod (0.5 is to split the total range)
		double bpInt = (getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_D], 0.0, +1.0));
		double bpFmodSemitones = bpInt*0.5*freqModSemitoneRange * processInfo.modulationInputs->getModValue(kBipolarMod);

		// --- EG input here
		double egInt = (getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_C], 0.0, +1.0));
		double egFmodSemitones = egInt*freqModSemitoneRange * processInfo.modulationInputs->getModValue(kEGMod);

		// --- setup fc mod
		double fc = parameters->fc;
		double ktFmodSemotones = 0.0;

		// --- key tracking
		if (parameters->enableKeyTrack)
		{
			ktFmodSemotones = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_A], -48.0, +48.0);
			fc = midiPitch;
		}

		// --- sum modulations
		double fcModSSemis = bpFmodSemitones + egFmodSemitones + ktFmodSemotones;

		// --- multiply by pitch shift factor
		fc *= pow(2.0, fcModSSemis / 12.0);
		boundValue(fc, freqModLow, freqModHigh);

		// --- output amplitude
		outputAmp = pow(20.0, parameters->filterOutputGain_dB / 20.0);

		// --- decision tree for type and index
		if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kLPF1))
		{
			if (parameters->analogFGN)
				outputIndex = ANM_LPF1;
			else
				outputIndex = LPF1;

			selectedModel = FilterModel::kFirstOrder;
			va1[LEFT].setFilterParams(fc, parameters->Q);
			va1[LEFT].copyCoeffs(va1[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kHPF1))
		{
			outputIndex = HPF1;
			selectedModel = FilterModel::kFirstOrder;
			va1[LEFT].setFilterParams(fc, parameters->Q);
			va1[LEFT].copyCoeffs(va1[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kAPF1))
		{
			outputIndex = APF1;
			selectedModel = FilterModel::kFirstOrder;
			va1[LEFT].setFilterParams(fc, parameters->Q);
			va1[LEFT].copyCoeffs(va1[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kSVF_LP))
		{
			if (parameters->analogFGN)
				outputIndex = ANM_LPF2;
			else
				outputIndex = LPF2;

			selectedModel = FilterModel::kSVF;
			svf[LEFT].setFilterParams(fc, parameters->Q);
			svf[LEFT].copyCoeffs(svf[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kSVF_HP))
		{
			outputIndex = HPF2;
			selectedModel = FilterModel::kSVF;
			svf[LEFT].setFilterParams(fc, parameters->Q);
			svf[LEFT].copyCoeffs(svf[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kSVF_BP))
		{
			outputIndex = BPF2;
			selectedModel = FilterModel::kSVF;
			svf[LEFT].setFilterParams(fc, parameters->Q);
			svf[LEFT].copyCoeffs(svf[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kSVF_BS))
		{
			outputIndex = BSF2;
			selectedModel = FilterModel::kSVF;
			svf[LEFT].setFilterParams(fc, parameters->Q);
			svf[LEFT].copyCoeffs(svf[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kKorg35_LP))
		{
			if (parameters->analogFGN)
				outputIndex = ANM_LPF2;
			else
				outputIndex = LPF2;

			selectedModel = FilterModel::kKorg35;
			korg35[LEFT].setFilterParams(fc, parameters->Q);
			korg35[LEFT].copyCoeffs(korg35[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kKorg35_HP))
		{
			outputIndex = HPF2;
			selectedModel = FilterModel::kKorg35;
			korg35[LEFT].setFilterParams(fc, parameters->Q);
			korg35[LEFT].copyCoeffs(korg35[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kMoog_LP1))
		{
			if (parameters->analogFGN)
				outputIndex = ANM_LPF1;
			else
				outputIndex = LPF1;

			selectedModel = FilterModel::kMoog;
			moog[LEFT].setFilterParams(fc, parameters->Q);
			moog[LEFT].copyCoeffs(moog[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kMoog_LP2))
		{
			if (parameters->analogFGN)
				outputIndex = ANM_LPF2;
			else
				outputIndex = LPF2;

			selectedModel = FilterModel::kMoog;
			moog[LEFT].setFilterParams(fc, parameters->Q);
			moog[LEFT].copyCoeffs(moog[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kMoog_LP3))
		{
			if (parameters->analogFGN)
				outputIndex = ANM_LPF3;
			else
				outputIndex = LPF3;

			selectedModel = FilterModel::kMoog;
			moog[LEFT].setFilterParams(fc, parameters->Q);
			moog[LEFT].copyCoeffs(moog[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kMoog_LP4))
		{
			if (parameters->analogFGN)
				outputIndex = ANM_LPF4;
			else
				outputIndex = LPF4;

			selectedModel = FilterModel::kMoog;
			moog[LEFT].setFilterParams(fc, parameters->Q);
			moog[LEFT].copyCoeffs(moog[RIGHT]);
		}
		else if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kDiode_LP4))
		{
			if (parameters->analogFGN)
				outputIndex = ANM_LPF4;
			else
				outputIndex = LPF4;

			selectedModel = FilterModel::kDiode;
			diode[LEFT].setFilterParams(fc, parameters->Q);
			diode[LEFT].copyCoeffs(diode[RIGHT]);
		}

		return true;
	}

	/**
	\brief Renders the output of the module
	- processes input to output buffer using pointers in the CoreProcData argument
	Core Specific:
	- process left and right channels through independent filters
	- apply peak limiter
	- renders one block of audio per render cycle
	- renders in mono that is copied to the right channel as dual-mono stereo

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool VAFilterCore::render(CoreProcData& processInfo)
	{
		// --- parameters
		FilterParameters* parameters = static_cast<FilterParameters*>(processInfo.moduleParameters);

		// --- stereo I/O
		float* leftInBuffer = processInfo.inputBuffers[LEFT_CHANNEL];
		float* leftOutBuffer = processInfo.outputBuffers[LEFT_CHANNEL];

		float* rightInBuffer = processInfo.inputBuffers[RIGHT_CHANNEL];
		float* rightOutBuffer = processInfo.outputBuffers[RIGHT_CHANNEL];

		for (uint32_t i = 0; i < processInfo.samplesToProcess; i++)
		{
			double xnL = leftInBuffer[i];
			double xnR = rightInBuffer[i];

			if (parameters->filterIndex == enumToInt(VAFilterAlgorithm::kBypassFilter))
			{
				leftOutBuffer[i] = xnL;
				rightOutBuffer[i] = xnR;
				continue;
			}

			// --- waveshaper drive
			if (parameters->filterDrive > 1.05)
			{
				xnL = tanhWaveShaper(xnL, parameters->filterDrive);
				xnR = tanhWaveShaper(xnR, parameters->filterDrive);
			}

			FilterOutput* output[STEREO];

			if (selectedModel == FilterModel::kFirstOrder)
			{
				output[LEFT] = va1[LEFT].process(xnL);
				output[RIGHT] = forceDualMonoFilters ? output[LEFT] : va1[RIGHT].process(xnR);
			}
			else if (selectedModel == FilterModel::kSVF)
			{
				output[LEFT] = svf[LEFT].process(xnL);
				output[RIGHT] = forceDualMonoFilters ? output[LEFT] : svf[RIGHT].process(xnR);
			}
			else if (selectedModel == FilterModel::kKorg35)
			{
				output[LEFT] = korg35[LEFT].process(xnL);
				output[RIGHT] = forceDualMonoFilters ? output[LEFT] : korg35[RIGHT].process(xnR);
			}
			else if (selectedModel == FilterModel::kMoog)
			{
				output[LEFT] = moog[LEFT].process(xnL);
				output[RIGHT] = forceDualMonoFilters ? output[LEFT] : moog[RIGHT].process(xnR);
			}
			else if (selectedModel == FilterModel::kDiode)
			{
				output[LEFT] = diode[LEFT].process(xnL);
				output[RIGHT] = forceDualMonoFilters ? output[LEFT] : diode[RIGHT].process(xnR);
			}

			// --- select output (use code below to bypass the limiter if you like)
			leftOutBuffer[i] = outputAmp * limiter[LEFT].process(output[LEFT]->filter[outputIndex]);
			rightOutBuffer[i] = forceDualMonoFilters ? leftOutBuffer[i] : outputAmp * limiter[RIGHT].process(output[RIGHT]->filter[outputIndex]);
		}

		return true;
	}

	/**
	\brief Note-on handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- saves MIDI pitch for key tracking

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool VAFilterCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- save note pitch for key tracking
		midiPitch = processInfo.noteEvent.midiPitch;

		return true;
	}

	/**
	\brief Note-off handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- nothing to do

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool VAFilterCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}

} // namespace


