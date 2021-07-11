#include "filtermodule.h"

// -----------------------------
//	--- SynthLab SDK File --- //
//  ----------------------------
/**
\file   filtermodule.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\brief Constructs Digitally Controlled Amplifier module.
	- See class declaration for information on standalone operation

	\param _midiInputData shared MIDI input resource; may be nullptr
	\param _parameters shared GUI and operational parameters; may be nullptr
	\param blockSize the synth block process size in frames (stereo)

	\returns the newly constructed object
	*/
	FilterModule::FilterModule(std::shared_ptr<MidiInputData> _midiInputData,
		std::shared_ptr<FilterParameters> _parameters,
		uint32_t blockSize) :
		SynthModule(_midiInputData)
		, parameters(_parameters)
	{
		// --- standalone ONLY: parameters
		if (!parameters)
			parameters.reset(new FilterParameters);

		// --- create our audio buffers
		audioBuffers.reset(new SynthProcessInfo(STEREO_INPUTS, STEREO_OUTPUTS, blockSize));

		// --- module strings
		moduleData.moduleStrings[vicLPF2] = "VicLPF2";                moduleData.moduleStrings[8] = empty_string.c_str();
		moduleData.moduleStrings[vicBPF2] = "VicBPF2";                moduleData.moduleStrings[9] = empty_string.c_str();
		moduleData.moduleStrings[2] = empty_string.c_str();	    moduleData.moduleStrings[10] = empty_string.c_str();
		moduleData.moduleStrings[3] = empty_string.c_str();	    moduleData.moduleStrings[11] = empty_string.c_str();
		moduleData.moduleStrings[4] = empty_string.c_str();	    moduleData.moduleStrings[12] = empty_string.c_str();
		moduleData.moduleStrings[5] = empty_string.c_str();	    moduleData.moduleStrings[13] = empty_string.c_str();
		moduleData.moduleStrings[6] = empty_string.c_str();	    moduleData.moduleStrings[14] = empty_string.c_str();
		moduleData.moduleStrings[7] = empty_string.c_str();	    moduleData.moduleStrings[15] = empty_string.c_str();

		// --- mod knobs
		moduleData.modKnobStrings[MOD_KNOB_A] = "Key Track";
		moduleData.modKnobStrings[MOD_KNOB_B] = "B";
		moduleData.modKnobStrings[MOD_KNOB_C] = "C";
		moduleData.modKnobStrings[MOD_KNOB_D] = "D";

	}

	/**
	\brief Resets object to initialized state
	- call once during initialization
	- call any time sample rate changes (after init)

	\param _sampleRate the current sample rate in Hz

	\returns true if successful, false otherwise
	*/
	bool FilterModule::reset(double _sampleRate)
	{
		// --- store
		sampleRate = _sampleRate;

		// --- flush buffers in filters
		for (uint32_t i = 0; i < STEREO_CHANNELS; i++)
		{
			// --- reset; sample rate not needed
			filters[i].reset();
		}

		return true;
	}

	/**
	\brief Updates object by applying GUI parameter and input modulations to the internal variables
	- calculates a set of gain values, then applies them to the left and right channels
	- basically a big gain calculator

	\returns true if successful, false otherwise
	*/
	bool FilterModule::update()
	{
		// --- to be modulated
		double filterFc = parameters->fc;

		// --- bipolar freqmod (0.5 is to split the total range)
		double bpFmodSemitones = 0.5*freqModSemitoneRange * getModulationInput()->getModValue(kBipolarMod);

		// --- EG input here
		double egFmodSemitones = freqModSemitoneRange *  getModulationInput()->getModValue(kEGMod);

		// --- setup keytrack fc mod
		double ktFmodSemotones = 0.0;

		// --- key tracking
		if (parameters->enableKeyTrack)
		{
			// --- key track amount
			ktFmodSemotones = getModKnobValueLinear(parameters->modKnobValue[MOD_KNOB_A], -48.0, +48.0);
			
			// --- overwrite fc
			filterFc = midiPitch;
		}

		// --- sum modulations
		double fcModSSemis = bpFmodSemitones + egFmodSemitones + ktFmodSemotones;

		// --- multiply by pitch shift factor
		filterFc *= pow(2.0, fcModSSemis / 12.0);
		boundValue(filterFc, freqModLow, freqModHigh);

		// --- setup biquad structures and load with coefficients depending on filter type
		if (parameters->filterIndex == vicLPF2)
		{
			// http://vicanek.de/articles/BiquadFits.pdf
			double theta_c = 2.0*kPi*filterFc / sampleRate;
			double q = 1.0 / (2.0*parameters->Q);

			// --- impulse invariant
			double b_1 = 0.0;
			double b_2 = exp(-2.0*q*theta_c);
			if (q <= 1.0)
			{
				b_1 = -2.0*exp(-q*theta_c)*cos(pow((1.0 - q*q), 0.5)*theta_c);
			}
			else
			{
				b_1 = -2.0*exp(-q*theta_c)*cosh(pow((q*q - 1.0), 0.5)*theta_c);
			}

			// --- LOOSE FIT --- //
			double f0 = theta_c / kPi; // note f0 = fraction of pi, so that f0 = 1.0 = pi = Nyquist

			double r0 = 1.0 + b_1 + b_2;
			double denom = (1.0 - f0*f0)*(1.0 - f0*f0) + (f0*f0) / (parameters->Q*parameters->Q);
			denom = pow(denom, 0.5);
			double r1 = ((1.0 - b_1 + b_2)*f0*f0) / (denom);

			double a_0 = (r0 + r1) / 2.0;
			double a_1 = r0 - a_0;
			double a_2 = 0.0;

			BQCoeffs bq;
			bq.coeff[c0] = 1.0;
			bq.coeff[d0] = 0.0;
			bq.coeff[a0] = a_0;
			bq.coeff[a1] = a_1;
			bq.coeff[a2] = a_2;
			bq.coeff[b1] = b_1;
			bq.coeff[b2] = b_2;

			// --- update on filters
			filters[LEFT_CHANNEL].setCoeffs(bq);
			filters[RIGHT_CHANNEL].setCoeffs(bq);
		}
		else if (parameters->filterIndex == vicBPF2)
		{
			// http://vicanek.de/articles/BiquadFits.pdf
			double theta_c = 2.0*kPi*filterFc / sampleRate;
			double q = 1.0 / (2.0*parameters->Q);

			// --- impulse invariant
			double b_1 = 0.0;
			double b_2 = exp(-2.0*q*theta_c);
			if (q <= 1.0)
			{
				b_1 = -2.0*exp(-q*theta_c)*cos(pow((1.0 - q*q), 0.5)*theta_c);
			}
			else
			{
				b_1 = -2.0*exp(-q*theta_c)*cosh(pow((q*q - 1.0), 0.5)*theta_c);
			}

			// --- LOOSE FIT --- //
			double f0 = theta_c / kPi; // note f0 = fraction of pi, so that f0 = 1.0 = pi = Nyquist

			double r0 = (1.0 + b_1 + b_2) / (kPi*f0*parameters->Q);
			double denom = (1.0 - f0*f0)*(1.0 - f0*f0) + (f0*f0) / (parameters->Q*parameters->Q);
			denom = pow(denom, 0.5);

			double r1 = ((1.0 - b_1 + b_2)*(f0 / parameters->Q)) / (denom);

			double a_1 = -r1 / 2.0;
			double a_0 = (r0 - a_1) / 2.0;
			double a_2 = -a_0 - a_1;

			BQCoeffs bq;
			bq.coeff[c0] = 1.0;
			bq.coeff[d0] = 0.0;
			bq.coeff[a0] = a_0;
			bq.coeff[a1] = a_1;
			bq.coeff[a2] = a_2;
			bq.coeff[b1] = b_1;
			bq.coeff[b2] = b_2;

			// --- update on filters
			filters[LEFT_CHANNEL].setCoeffs(bq);
			filters[RIGHT_CHANNEL].setCoeffs(bq);
		}

		return true; // handled
	}

	/**
	\brief Processes audio from the input buffers to the output buffers
	- Calls the update function first - NOTE: owning object does not need to call update()
	- samples to process should normally be the block size, but may be a partial block in some cases
	due to OS/CPU activity.
	- the input and output buffers are accessed with
		audioBuffers->getInputBuffer() and
		audioBuffers->getOutputBuffer()

	\returns true if successful, false otherwise
	*/
	bool FilterModule::render(uint32_t samplesToProcess)
	{
		// --- update parameters for this block
		update();

		// --- FilterModule processes every sample into output buffers
		float* leftInBuffer = audioBuffers->getInputBuffer(LEFT_CHANNEL);
		float* rightInBuffer = audioBuffers->getInputBuffer(RIGHT_CHANNEL);
		float* leftOutBuffer = audioBuffers->getOutputBuffer(LEFT_CHANNEL);
		float* rightOutBuffer = audioBuffers->getOutputBuffer(RIGHT_CHANNEL);

		// --- process block
		for (uint32_t i = 0; i < samplesToProcess; i++)
		{
			// --- stereo
			leftOutBuffer[i] = filters[LEFT_CHANNEL].processAudioSample(leftInBuffer[i]);
			rightOutBuffer[i] = filters[RIGHT_CHANNEL].processAudioSample(rightInBuffer[i]);
		}

		return true;
	}

	/**
	\brief Perform note-on operations for the component

	\return true if handled, false if not handled
	*/
	bool FilterModule::doNoteOn(MIDINoteEvent& noteEvent)
	{
		// --- just save for keytrack
		midiPitch = noteEvent.midiPitch;

		return true;
	}

	/**
	\brief Perform note-off operations for the component;
		   Here there is nothing to do.

	\return true if handled, false if not handled
	*/
	bool FilterModule::doNoteOff(MIDINoteEvent& noteEvent)
	{
		return true;
	}
}