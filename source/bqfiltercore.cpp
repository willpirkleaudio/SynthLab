
#include "bqfiltercore.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   bqfiltercore.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief
	Construction: Cores follow the same construction pattern
	- set the Module type and name parameters
	- expose the 16 module strings
	- expose the 4 mod knob label strings
	- intialize any internal variables

	Core Specific:
	- implements a few of the filters from Will Pirkle's FX Plugin book
	- includes one pole HPF, LPF

	\returns the newly constructed object
	*/
	BQFilterCore::BQFilterCore()
	{
		moduleType = FILTER_MODULE;
		moduleName = "BQFilters";
		preferredIndex = 1; // ordering for user, DM only

		// --- our filters
		/*
			Module Strings, zero-indexed for your GUI Control:
			- No_Filter, _1PLPF, _1PHPF, BQ_LPF2, BQ_HPF2
		*/
		coreData.moduleStrings[0] = "No Filter";				coreData.moduleStrings[8] = empty_string.c_str();
		coreData.moduleStrings[1] = "One Pole LPF";				coreData.moduleStrings[9] = empty_string.c_str();
		coreData.moduleStrings[2] = "One Pole HPF";				coreData.moduleStrings[10] = empty_string.c_str();
		coreData.moduleStrings[3] = "BQ LPF2";					coreData.moduleStrings[11] = empty_string.c_str();
		coreData.moduleStrings[4] = "BQ HPF2";					coreData.moduleStrings[12] = empty_string.c_str();
		coreData.moduleStrings[5] = empty_string.c_str();		coreData.moduleStrings[13] = empty_string.c_str();
		coreData.moduleStrings[6] = empty_string.c_str();		coreData.moduleStrings[14] = empty_string.c_str();
		coreData.moduleStrings[7] = empty_string.c_str();		coreData.moduleStrings[15] = empty_string.c_str();

		// --- modulation control knobs
		coreData.modKnobStrings[MOD_KNOB_A] = "Key Track";
		coreData.modKnobStrings[MOD_KNOB_B] = "Drive";
		coreData.modKnobStrings[MOD_KNOB_C] = "EG Int";
		coreData.modKnobStrings[MOD_KNOB_D] = "BP Int";
	}


	/**
	\brief Resets object to initialized state
	- parameters are accessed via the processInfo.moduleParameters pointer
	- initializes sample rate dependent stuff via processInfo.sampleRate
	- calculates intitial coefficients
	- sets initial state variables

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool BQFilterCore::reset(CoreProcData& processInfo)
	{
		sampleRate = processInfo.sampleRate;

		// --- just reset sub-filters
		for (uint32_t i = 0; i < STEREO_CHANNELS; i++)
		{
			filter[i].reset();
		}

		return true;
	}

	/**
	\brief Updates the object for the next block of audio processing
	- parameters are accessed via the processInfo.moduleParameters pointer
	- modulator inputs are accessied via processInfo.modulationInputs
	- mod knob values are accessed via parameters->modKnobValue[]
	Core Specific:
	- calculates fc modulation value from input modulators, kBipolarMod and kEGMod
	- calculates key-tracking modulation value (if enabled)
	- calculated the coefficients for various filters based on user selection

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool BQFilterCore::update(CoreProcData& processInfo)
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

		BQCoeffs bq;
		bq.coeff[c0] = 1.0;
		bq.coeff[d0] = 0.0;

		// --- use mapping function for Q -> K
		double mappedQ = parameters->Q;
		mapDoubleValue(mappedQ, 1.0, 10.0, 0.707, 20.0);

		if (parameters->filterIndex == enumToInt(BQFilterAlgorithm::k1PLPF))
		{
			// --- see 2nd Ed FX for formulae
			double theta_c = kTwoPi*fc / sampleRate;
			double gamma = 2.0 - cos(theta_c);
			double filter_b1 = pow((gamma*gamma - 1.0), 0.5) - gamma;
			double filter_a0 = 1.0 + filter_b1;

			// --- update coeffs
			bq.coeff[a0] = filter_a0;
			bq.coeff[a1] = 0.0;
			bq.coeff[a2] = 0.0;
			bq.coeff[b1] = filter_b1;
			bq.coeff[b2] = 0.0;
		}
		else if (parameters->filterIndex == enumToInt(BQFilterAlgorithm::k1PHPF))
		{
			// --- see 2nd Ed FX for formulae
			double theta_c = kTwoPi*fc / sampleRate;
			double gamma = 2.0 + cos(theta_c);
			double filter_b1 = gamma - pow((gamma*gamma - 1.0), 0.5);
			double filter_a0 = 1.0 - filter_b1;

			// --- update coeffs
			bq.coeff[a0] = filter_a0;
			bq.coeff[a1] = 0.0;
			bq.coeff[a2] = 0.0;
			bq.coeff[b1] = filter_b1;
			bq.coeff[b2] = 0.0;
		}
		else if (parameters->filterIndex == enumToInt(BQFilterAlgorithm::kLPF2))
		{
			// --- see 2nd Ed FX for formulae
			double theta_c = kTwoPi*fc / sampleRate;
			double d = 1.0 / mappedQ;
			double betaNumerator = 1.0 - ((d / 2.0)*(sin(theta_c)));
			double betaDenominator = 1.0 + ((d / 2.0)*(sin(theta_c)));
			double beta = 0.5*(betaNumerator / betaDenominator);
			double gamma = (0.5 + beta)*(cos(theta_c));
			double alpha = (0.5 + beta - gamma) / 2.0;

			// --- update coeffs
			bq.coeff[a0] = alpha;
			bq.coeff[a1] = 2.0*alpha;
			bq.coeff[a2] = alpha;
			bq.coeff[b1] = -2.0*gamma;
			bq.coeff[b2] = 2.0*beta;
		}
		else if (parameters->filterIndex == enumToInt(BQFilterAlgorithm::kHPF2))
		{
			// --- see 2nd Ed FX for formulae
			double theta_c = kTwoPi*fc / sampleRate;
			double d = 1.0 / mappedQ;
			double betaNumerator = 1.0 - ((d / 2.0)*(sin(theta_c)));
			double betaDenominator = 1.0 + ((d / 2.0)*(sin(theta_c)));

			double beta = 0.5*(betaNumerator / betaDenominator);
			double gamma = (0.5 + beta)*(cos(theta_c));
			double alpha = (0.5 + beta + gamma) / 2.0;

			// --- update coeffs
			bq.coeff[a0] = alpha;
			bq.coeff[a1] = -2.0*alpha;
			bq.coeff[a2] = alpha;
			bq.coeff[b1] = -2.0*gamma;
			bq.coeff[b2] = 2.0*beta;
		}
		// --- set coefficients on filters
		filter[LEFT_CHANNEL].setCoeffs(bq);
		filter[RIGHT_CHANNEL].setCoeffs(bq);

		return true;
	}

	/**
	\brief Renders the output of the module
	- processes from input buffer to output buffer using pointers in the CoreProcData argument
	Core Specific:
	- filter sub-objects perform processing
	- implements drive control with arc-tangent waveshaper

	\param processInfo the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool BQFilterCore::render(CoreProcData& processInfo)
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

			if (parameters->filterIndex == enumToInt(BQFilterAlgorithm::kBypassFilter))
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

			// --- select output
			leftOutBuffer[i] = outputAmp * filter[LEFT_CHANNEL].processAudioSample(xnL);
			rightOutBuffer[i] = outputAmp * filter[RIGHT_CHANNEL].processAudioSample(xnR);
		}

		return true;
	}

	/**
	\brief Note-on handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- saves MIDI pitch in case of key-tracking modulation
	- flushes biquad state variable delay lines

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool BQFilterCore::doNoteOn(CoreProcData& processInfo)
	{
		// --- save note pitch for key tracking
		midiPitch = processInfo.noteEvent.midiPitch;

		// --- clear out delays
		flushDelays();

		return true;
	}

	/**
	\brief Note-off handler for the ModuleCore
	- parameters are accessed via the processInfo.moduleParameters pointer
	- MIDI note information is accessed via processInfo.noteEvent

	Core Specific:
	- nothing to do here

	\param processInfo is the thunk-barrier compliant data structure for passing all needed parameters

	\returns true if successful, false otherwise
	*/
	bool BQFilterCore::doNoteOff(CoreProcData& processInfo)
	{
		return true;
	}

} // namespace


