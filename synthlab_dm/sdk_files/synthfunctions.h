// -----------------------------------------------------------------------------
//    RackAFX Synth Objects File:  synthfunctions.h
//
/**
    \file   synthfunctions.h
    \author Will Pirkle
    \date   20-April-2021
    \brief  hard-coded arrays of FIR filter coefficients for the sample rate
    		conversion objects (Interpolator and Decimator)

    		- http://www.aspikplugins.com
			- http://www.willpirkle.com

*/
// -----------------------------------------------------------------------------
#pragma once
#ifndef __synthfunctions_h__
#define  __synthfunctions_h__

// --- turn off non-critical warnings
#pragma warning(disable : 4244)//double to float (default DAW buffers are floats, we process doubles)
#pragma warning(disable : 4018)//int to uint32_t (PCM read index may go backwards)
#pragma warning(disable : 4390)//; empty statements (I use this for NOOP sometimes)
#pragma warning(disable : 4267)

#include "synthbase.h"
#include "bleptables.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   synthfunctions.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	@copyOutputToInput
	\ingroup SynthFunctions

	@brief 
	copies an output audio buffer to an input audio buffer
	- used for moving audio data through the audio engine
	- copies mono->mono. mono->stereo and stereo->stereo

	\param source AudioBuffer whose output is being copied
	\param destination AudioBuffer whose input will receive the copied audio data
	\channel the channels to copy MONO_TO_MONO, MONO_TO_STEREO, STEREO_TO_STEREO
	\param samplesToCopy size of block to copy
	*/
	inline void copyOutputToInput(std::shared_ptr<AudioBuffer> source, std::shared_ptr<AudioBuffer> destination, uint32_t channel, uint32_t samplesToCopy)
	{
		if (channel == MONO_TO_MONO)
		{
			memcpy(destination->getInputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else if (channel == MONO_TO_STEREO)
		{
			memcpy(destination->getInputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination->getInputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else
		{
			memcpy(destination->getInputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination->getInputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(RIGHT_CHANNEL), samplesToCopy * sizeof(float));
		}
	}

	/**
	@copyOutputToOutput
	\ingroup SynthFunctions

	@brief
	copies an output audio buffer to another output audio buffer
	- copies mono->mono. mono->stereo and stereo->stereo

	\param source AudioBuffer whose output is being copied
	\param destination AudioBuffer whose output will receive the copied audio data
	\channel the channels to copy MONO_TO_MONO, MONO_TO_STEREO, STEREO_TO_STEREO
	\param samplesToCopy size of block to copy
	*/
	inline void copyOutputToOutput(std::shared_ptr<AudioBuffer> source, std::shared_ptr<AudioBuffer> destination, uint32_t channel, uint32_t samplesToCopy)
	{
		if (channel == MONO_TO_MONO)
		{
			memcpy(destination->getOutputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else if (channel == MONO_TO_STEREO)
		{
			memcpy(destination->getOutputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination->getOutputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else
		{
			memcpy(destination->getOutputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination->getOutputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(RIGHT_CHANNEL), samplesToCopy * sizeof(float));
		}
	}

	/**
	@copyOutputToOutput
	\ingroup SynthFunctions

	@brief
	copies an output audio buffer to the SynthProcessInfo audio output buffer
	- used for moving the final rendered audio data fom the engine's
	output mix buffers to the plugin framework-supplied buffers
	- copies mono->mono. mono->stereo and stereo->stereo

	\param source AudioBuffer whose output is being copied
	\param destination AudioBuffer whose output will receive the copied audio data
	\channel the channels to copy MONO_TO_MONO, MONO_TO_STEREO, STEREO_TO_STEREO
	\param samplesToCopy size of block to copy
	*/
	inline void copyOutputToOutput(std::shared_ptr<AudioBuffer> source, SynthProcessInfo& destination, uint32_t channel, uint32_t samplesToCopy)
	{
		if (channel == MONO_TO_MONO)
		{
			memcpy(destination.getOutputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else if (channel == MONO_TO_STEREO)
		{
			memcpy(destination.getOutputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination.getOutputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else
		{
			memcpy(destination.getOutputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination.getOutputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(RIGHT_CHANNEL), samplesToCopy * sizeof(float));
		}
	}

	/**
	@copyBufferToInput
	\ingroup SynthFunctions

	@brief
	copies an output audio buffer to an input audio buffer
	- functionally equivalent to copyOutputToInput( ); 
	renamed for easier programming
	- used for voice object to move data from its mix buffers 
	into the rest of the audio chain
	- copies mono->mono. mono->stereo and stereo->stereo

	\param source AudioBuffer whose output is being copied
	\param destination AudioBuffer whose input will receive the copied audio data
	\channel the channels to copy MONO_TO_MONO, MONO_TO_STEREO, STEREO_TO_STEREO
	\param samplesToCopy size of block to copy
	*/
	inline void copyBufferToInput(std::shared_ptr<AudioBuffer> source, std::shared_ptr<AudioBuffer> destination, uint32_t channel, uint32_t samplesToCopy)
	{
		if (channel == MONO_TO_MONO)
		{
			memcpy(destination->getInputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else if (channel == MONO_TO_STEREO)
		{
			memcpy(destination->getInputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination->getInputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else
		{
			memcpy(destination->getInputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination->getInputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(RIGHT_CHANNEL), samplesToCopy * sizeof(float));
		}
	}

	/**
	@copyAudioBufferOutputToSynthOutput
	\ingroup SynthFunctions

	@brief
	copies an output audio buffer to the SynthProcessInfo audio output buffer
	- functionally equivalent to copyOutputToOutput( ); 
	renamed for easier programming
	- used for moving the final rendered audio data fom the engine's
	output mix buffers to the plugin framework-supplied buffers
	- copies mono->mono. mono->stereo and stereo->stereo

	\param source AudioBuffer whose output is being copied
	\param destination AudioBuffer whose output will receive the copied audio data
	\channel the channels to copy MONO_TO_MONO, MONO_TO_STEREO, STEREO_TO_STEREO
	\param samplesToCopy size of block to copy
	*/
	inline void copyAudioBufferOutputToSynthOutput(std::shared_ptr<AudioBuffer> source, SynthProcessInfo& destination, uint32_t channel, uint32_t samplesToCopy)
	{
		if (channel == MONO_TO_MONO)
		{
			memcpy(destination.getOutputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else if (channel == MONO_TO_STEREO)
		{
			memcpy(destination.getOutputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination.getOutputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else
		{
			memcpy(destination.getOutputBuffer(LEFT_CHANNEL), source->getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination.getOutputBuffer(RIGHT_CHANNEL), source->getOutputBuffer(RIGHT_CHANNEL), samplesToCopy * sizeof(float));
		}
	}

	/**
	@copySynthOutputToAudioBufferInput
	\ingroup SynthFunctions

	@brief
	Complementary function that moves audio from the SynthProcessInfo output into 
	an audio buffer
	- used in the plugin framework integration code
	- copies mono->mono. mono->stereo and stereo->stereo

	\param source AudioBuffer whose output is being copied
	\param destination AudioBuffer whose output will receive the copied audio data
	\channel the channels to copy MONO_TO_MONO, MONO_TO_STEREO, STEREO_TO_STEREO
	\param samplesToCopy size of block to copy
	*/
	inline void copySynthOutputToAudioBufferInput(SynthProcessInfo& source, std::shared_ptr<AudioBuffer> destination, uint32_t channel, uint32_t samplesToCopy)
	{
		if (channel == MONO_TO_MONO)
		{
			memcpy(destination->getInputBuffer(LEFT_CHANNEL), source.getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else if (channel == MONO_TO_STEREO)
		{
			memcpy(destination->getInputBuffer(LEFT_CHANNEL), source.getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination->getInputBuffer(RIGHT_CHANNEL), source.getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
		}
		else
		{
			memcpy(destination->getInputBuffer(LEFT_CHANNEL), source.getOutputBuffer(LEFT_CHANNEL), samplesToCopy * sizeof(float));
			memcpy(destination->getInputBuffer(RIGHT_CHANNEL), source.getOutputBuffer(RIGHT_CHANNEL), samplesToCopy * sizeof(float));
		}
	}

	// --- typed enumeration helpers
	/**
	@enumToInt
	\ingroup Constants-Enums
	\def enumToInt
	@brief macro helper to cast a typed enum to an int

	\param ENUM - the typed enum to convert
	\return the enum properly cast as an int (the true underlying datatype for enum class)
	*/
#define enumToInt(ENUM) static_cast<int>(ENUM)

	/**
	@compareEnumToInt
	\ingroup Constants-Enums
	\def compareEnumToInt
	@brief compare a typed enum value to an int

	\param ENUM - the typed enum to compare with
	\param INT - the int to compare with
	\return true if equal false otherwise
	*/
#define compareEnumToInt(ENUM,INT) (static_cast<int>(ENUM) == (INT))

	/**
	@compareIntToEnum
	\ingroup Constants-Enums
	\def compareIntToEnum

	@brief compare a typed enum value to an int

	\param INT - the int to compare with
	\param ENUM - the typed enum to compare with
	\return true if equal false otherwise
	*/
#define compareIntToEnum(INT,ENUM) ((INT) == static_cast<int>(ENUM))

	/**
	@convertIntToEnum
	\ingroup Constants-Enums
	\def convertIntToEnum

	@brief convert an int to an enum, e.g. for passing to functions

	\param INT - the int to compare with
	\param ENUM - the typed enum to compare with
	\return the int value properly cast as the enum type
	*/
#define convertIntToEnum(INT,ENUM) static_cast<ENUM>(INT)

	/**
	@kCTCoefficient
	\ingroup Constants-Enums
	@brief concave and/or convex transform correction factor
	*/
	const double kCTCoefficient = 5.0 / 12.0;

	/**
	@kCTCorrFactorZero
	\ingroup Constants-Enums
	@brief concave/convex transform correction factor at x = 0
	*/
	const double kCTCorrFactorZero = pow(10.0, (-1.0 / kCTCoefficient));

	/**
	@kCTCorrFactorAnitZero
	\ingroup Constants-Enums
	@brief inverse concave/convex transform factor at x = 0
	*/
	const double kCTCorrFactorAnitZero = 1.0 / (1.0 - kCTCorrFactorZero);

	/**
	@kCTCorrFactorUnity
	\ingroup Constants-Enums
	@brief concave/convex transform correction factor at x = 1
	*/
	const double kCTCorrFactorUnity = 1.0 / (1.0 + kCTCoefficient*log10(1.0 + kCTCorrFactorZero));

	/**
	@kCTCorrFactorAntiUnity
	\ingroup Constants-Enums
	@brief inverse concave/convex transform correction factor at x = 1
	*/
	const double kCTCorrFactorAntiUnity = 1.0 / (1.0 + (-pow(10.0, (-1.0 / kCTCoefficient))));

	/**
	@kCTCorrFactorAntiLog
	\ingroup Constants-Enums
	@brief concave/convex transform correction factor
	*/
	const double kCTCorrFactorAntiLog = kCTCoefficient*log10(1.0 + kCTCorrFactorZero);

	/**
	@kCTCorrFactorAntiLogScale
	\ingroup Constants-Enums
	@brief concave/convex transform scaling factor
	*/
	const double kCTCorrFactorAntiLogScale = 1.0 / (-kCTCoefficient*log10(kCTCorrFactorZero) + kCTCorrFactorAntiLog);

	/**
	@normToLogNorm
	\ingroup SynthFunctions

	@brief
	normalized to Log-normalized version 

	\param normalizedValue normalized value
	\return log10 version, normalized
	*/
	inline double normToLogNorm(double normalizedValue)
	{
		return 1.0 + kCTCoefficient*log10(normalizedValue);
	}

	/**
	@logNormToNorm
	\ingroup SynthFunctions

	@brief
	log-normalized to normalized version

	\param logNormalizedValue log-normalized value
	\return normalvalue, normalized
	*/
	inline double logNormToNorm(double logNormalizedValue)
	{
		return pow(10.0, (logNormalizedValue - 1.0) / kCTCoefficient);
	}

	/**
	@normToAntiLogNorm
	\ingroup SynthFunctions

	@brief
	normalized to anti-log normalized version

	\param normalizedValue log-normalized value
	\return anti-log verion, normalized
	*/
	inline double normToAntiLogNorm(double normalizedValue)
	{
		if (normalizedValue == 1.0)
			return 1.0;

		double aln = -kCTCoefficient*log10(1.0 - normalizedValue);
		aln = fmin(1.0, aln);
		return aln;
	}

	/**
	@antiLogNormToNorm
	\ingroup SynthFunctions

	@brief
	anti-log normalized to normalized version 

	\param aLogNormalizedValue log-normalized value
	\return anti-log verion, normalized
	*/
	inline double antiLogNormToNorm(double aLogNormalizedValue)
	{
		return -pow(10.0, (-aLogNormalizedValue / kCTCoefficient)) + 1.0;
	}

	/**
	@getModKnobValueLinear
	\ingroup SynthFunctions

	@brief
	maps a mod-knob value on the range of [0.0, 1.0] to a number on the range [min, max]

	\param normalizedValue normalized value
	\param min the minimum mapped value
	\param max the maximum mapped value
	\return the new value, mapped on the new range
	*/
	inline double getModKnobValueLinear(double normalizedValue, double min, double max)
	{
		return (max - min)*normalizedValue + min;
	}

	/**
	@getModKnobValueLog
	\ingroup SynthFunctions

	@brief
	maps a mod-knob value on the range of [0.0, 1.0] to a number on the range [min, max]
	logarithmically

	\param normalizedValue normalized value
	\param min the minimum mapped value
	\param max the maximum mapped value
	\return the new value, mapped on the new range logarithmically
	*/
	inline double getModKnobValueLog(double normalizedValue, double min, double max)
	{
		normalizedValue = normToLogNorm(normalizedValue);
		return (max - min)*normalizedValue + min;
	}

	/**
	@getModKnobValueAntiLog
	\ingroup SynthFunctions

	@brief
	maps a mod-knob value on the range of [0.0, 1.0] to a number on the range [min, max]
	anti-logarithmically

	\param normalizedValue normalized value
	\param min the minimum mapped value
	\param max the maximum mapped value
	\return the new value, mapped on the new range anti-logarithmically
	*/
	inline double getModKnobValueAntiLog(double normalizedValue, double min, double max)
	{
		normalizedValue = normToAntiLogNorm(normalizedValue);
		return (max - min)*normalizedValue + min;
	}


	/**
	@msecToSamples
	\ingroup SynthFunctions

	@brief
	convert a time in milliseconds to a floating point sample count 

	\param sampleRate fs
	\param timeMSec time in milliseconds to convert
	\return the number of samples (fractional)
	*/
	inline double msecToSamples(double sampleRate, double timeMSec)
	{
		return sampleRate*(timeMSec / 1000.0);
	}

	/**
	@clampMaxValue
	\ingroup SynthFunctions

	@brief  Bound a value to max limits

	\param value - value to bound
	\param maxValue - upper bound limit
	*/
	inline void clampMaxValue(double& value, double maxValue)
	{
		value = fmin(value, maxValue);
	}

	/**
	@clampMinValue
	\ingroup SynthFunctions

	@brief  Bound a value to min limits

	\param value - value to bound
	\param minValue - lower bound limit
	*/
	inline void clampMinValue(double& value, double minValue)
	{
		value = fmax(value, minValue);
	}


	/**
	@boundValue
	\ingroup SynthFunctions

	@brief  Bound a value to min and max limits

	\param value - value to bound
	\param minValue - lower bound limit
	\param maxValue - upper bound limit
	*/
	inline void boundValue(double& value, double minValue, double maxValue)
	{
		const double t = value < minValue ? minValue : value;
		value = t > maxValue ? maxValue : t;
	}

	/**
	@boundUIntValue
	\ingroup SynthFunctions

	@brief  Bound a uint32_t value to min and max limits

	\param value - value to bound
	\param minValue - lower bound limit
	\param maxValue - upper bound limit
	*/
	inline void boundUIntValue(uint32_t& value, uint32_t minValue, uint32_t maxValue)
	{
		const uint32_t t = value < minValue ? minValue : value;
		value = t > maxValue ? maxValue : t;
	}

	/**
	@boundIntValue
	\ingroup SynthFunctions
	\brief Bound an int32_t value to min and max limits

	\param value to be bound, pass by reference
	\param minValue minimum value of bound
	\param maxValue maximum value of bound
	*/
	inline void boundIntValue(int32_t& value, int32_t minValue, int32_t maxValue)
	{
		const int32_t t = value < minValue ? minValue : value;
		value = t > maxValue ? maxValue : t;
	}

	/**
	@boundValueUnipolar
	\ingroup SynthFunctions

	@brief  Bound a value to [0, +1]

	\param value - value to bound
	*/
	inline void boundValueUnipolar(double& value)
	{
		boundValue(value, 0.0, 1.0);
	}

	/**
	@boundValueBipolar
	\ingroup SynthFunctions

	@brief  Bound a value to [-1, +1]

	\param value - value to bound
	*/
	inline void boundValueBipolar(double& value)
	{
		boundValue(value, -1.0, 1.0);
	}



	/**
	@boundMIDIValueByte
	\ingroup SynthFunctions
	\brief 
	Bound a value to 0 and 127 limits

	\param value to be bound, pass by reference
	*/
	inline void boundMIDIValueByte(uint32_t& value)
	{
		boundUIntValue(value, 0, 127);
	}


	/**
	@boundMIDIValueDoubleByte
	\ingroup SynthFunctions
	\brief 
	Bound a value to min and max limits
		
	\param value to be bound, pass by reference
	*/
	inline void boundMIDIValueDoubleByte(uint32_t& value)
	{
		boundUIntValue(value, 0, 16384);
	}

	/**
	@mapDoubleValue
	\ingroup SynthFunctions
	\brief 
	map double on a range of (min, max) to the same double on the range of (minMap, maxMap)

	\param value to be mapped, pass-by-reference, returns in this variable
	\param min minimum value of source range
	\param max maximum value of source range
	\param minMap minimum value of destination (mapped) range
	\param maxMap maximum value of destination (mapped) range
	*/
	inline void mapDoubleValue(double& value, double min, double max, double minMap, double maxMap)
	{
		// --- bound to limits
		boundValue(value, min, max);
		double mapped = ((value - min) / (max - min)) * (maxMap - minMap) + minMap;
		value = mapped;
	}

	/**
	@mapDoubleValue
	\ingroup SynthFunctions
	\brief
	map double on a range of (min, max) to the same double on the range of (minMap, maxMap)
	using a pre-calculated slope variable where:
	
	- slope = (maxMap - minMap) / (max - min)
	- used when the input/output mapping ranges are known apriori which is most of the time

	\param value to be mapped, pass-by-reference, returns in this variable
	\param min minimum value of source range
	\param minMap minimum value of destination (mapped) range
	*/
	inline void mapDoubleValue(double& value, double min, double minMap, double slope)
	{
		// --- bound to limits
		value = minMap + slope * (value - min);
	}

	/**
	@mapIntValue
	\ingroup SynthFunctions
	\brief
	map int on a range of (min, max) to the same int on the range of (minMap, maxMap)

	\param value to be mapped, pass-by-reference, returns in this variable
	\param min minimum value of source range
	\param max maximum value of source range
	\param minMap minimum value of destination (mapped) range
	\param maxMap maximum value of destination (mapped) range
	\param roundValue set true to round the final value (crucial that you get this right for MIDI)
	*/
	inline void mapIntValue(int& value, int min, int max, int minMap, int maxMap, bool roundValue = true)
	{
		// --- bound to limits
		boundIntValue(value, min, max);

		double mapped = (double(value - min) / double(max - min)) * double(maxMap - minMap) + double(minMap);
		if (roundValue)
			value = (int)(floor(mapped + 0.5));
		else
			value = (int)mapped;
	}

	/**
	@mapUintValue
	\ingroup SynthFunctions
	\brief
	map unsigned int on a range of (min, max) to the same unsigned int on the range of (minMap, maxMap)

	\param value to be mapped, pass-by-reference, returns in this variable
	\param min minimum value of source range
	\param max maximum value of source range
	\param minMap minimum value of destination (mapped) range
	\param maxMap maximum value of destination (mapped) range
	\param roundValue set true to round the final value (crucial that you get this right for MIDI)
	*/
	inline void mapUintValue(uint32_t& value, uint32_t min, uint32_t max, uint32_t minMap, uint32_t maxMap, bool roundValue = true)
	{
		// --- bound to limits
		boundUIntValue(value, min, max);

		double mapped = (double(value - min) / double(max - min)) * double(maxMap - minMap) + double(minMap);
		if (roundValue)
			value = (uint32_t)(round(mapped));
		else
			value = (uint32_t)mapped;
	}

	/**
	@mapDoubleToUINT
	\ingroup SynthFunctions
	\brief
	map double on a range of (min, max) to a uint32_t on the range of (minMap, maxMap)

	\param value to be mapped
	\param min minimum value of source range
	\param max maximum value of source range
	\param minMap minimum value of destination (mapped) range
	\param maxMap maximum value of destination (mapped) range
	\param roundValue set true to round the final value 

	\return the mapped value as uint32_t
	*/
	inline uint32_t mapDoubleToUINT(double value, double min, double max, uint32_t minMap, uint32_t maxMap, bool roundValue = false)
	{
		// --- bound 
		boundValue(value, min, max);

		// --- map to range
		uint32_t mappedValue = 0;
		double mapped = (double(value - min) / double(max - min)) * double(maxMap - minMap) + double(minMap);
		if (roundValue)
			mappedValue = (uint32_t)(floor(mapped + 0.5)); //round(mapped));
		else
			mappedValue = (uint32_t)mapped;

		return mappedValue;
	}

	/**
	@mapDoubleToUINT
	\ingroup SynthFunctions
	\brief
	map uint32_t on a range of (min, max) to a double on the range of (minMap, maxMap)

	\param value to be mapped
	\param min minimum value of source range
	\param max maximum value of source range
	\param minMap minimum value of destination (mapped) range
	\param maxMap maximum value of destination (mapped) range

	\return the mapped value as double
	*/
	inline double mapUINTToDouble(uint32_t value, uint32_t min, uint32_t max, double minMap, double maxMap)
	{
		// --- bound to limits
		boundUIntValue(value, min, max);
		//value = (uint32_t)fmin(value, max);
		//value = (uint32_t)fmax(value, min);

		double mapped = (double(value - min) / double(max - min)) * double(maxMap - minMap) + double(minMap);
		return mapped;
	}

	/**
	@midi14_bitToBipolar
	\ingroup MIDIFunctions
	\brief 
	Converts MIDI data bytes 1 and 2 (14-bit) into a bipolar [-1, +1] value

	\param midiDataLSB MIDI LSB (7 bits are valid)
	\param midiDataMSB MIDI MSB (7 bits are valid)

	\return the bipolar version of the 14-bit MIDI value
	*/
	inline double midi14_bitToBipolar(uint32_t midiDataLSB, uint32_t midiDataMSB)
	{
		// --- combine data, LSB or-ed with MSB left-shifted by 7
		int midi14_bitValue = (int)((midiDataLSB & 0x7F) | ((midiDataMSB & 0x7F) << 7));

		// --- convert to bipolar (0x2000 = 8192)
		return (double)(midi14_bitValue - 0x2000) / (double)(0x2000);
	}


	/**
	@midi14_bitToUnipolarInt
	\ingroup MIDIFunctions
	\brief
	Converts MIDI data bytes 1 and 2 (14-bit) into a unipolar value 0 -> 16383

	\param midiDataLSB MIDI LSB (7 bits are valid)
	\param midiDataMSB MIDI MSB (7 bits are valid)

	\return the unipolar version of the 14-bit MIDI value
	*/
	inline uint32_t midi14_bitToUnipolarInt(uint32_t midiDataLSB, uint32_t midiDataMSB)
	{
		// --- combine data, LSB or-ed with MSB left-shifted by 7
		uint32_t midi14_bitValue = (uint32_t)((midiDataLSB & 0x7F) | ((midiDataMSB & 0x7F) << 7));
		return midi14_bitValue;
	}

	/**
	@midi14_bitToUnipolarDouble
	\ingroup MIDIFunctions
	\brief
	Converts MIDI data bytes 1 and 2 (14-bit) into a unipolar double value 0.0 -> 1.0

	\param midiDataLSB MIDI LSB (7 bits are valid)
	\param midiDataMSB MIDI MSB (7 bits are valid)

	\return the unipolar double version of the 14-bit MIDI value
	*/
	inline double midi14_bitToUnipolarDouble(uint32_t midiDataLSB, uint32_t midiDataMSB)
	{
		// --- combine data, LSB or-ed with MSB left-shifted by 7
		uint32_t midi14_bitValue = (uint32_t)((midiDataLSB & 0x7F) | ((midiDataMSB & 0x7F) << 7));

		// --- normalize 0x3FFF = 16383
		return (double)(midi14_bitValue) / (double)(0x3FFF);
	}

	/**
	@midi14_bitToDouble
	\ingroup MIDIFunctions
	\brief
	Converts MIDI data bytes 1 and 2 (14-bit) into a double value on the range [minValue, maxValue]

	\param midiDataLSB MIDI LSB (7 bits are valid)
	\param midiDataMSB MIDI MSB (7 bits are valid)
	\param minValue minimum mapped value
	\param maxValue maximum mapped value

	\return the unipolar double version of the 14-bit MIDI value value on the range [minValue, maxValue]
	*/
	inline double midi14_bitToDouble(uint32_t midiDataLSB, uint32_t midiDataMSB, double minValue, double maxValue)
	{
		// --- combine data, LSB or-ed with MSB left-shifted by 7
		uint32_t midi14_bitValue = (uint32_t)((midiDataLSB & 0x7F) | ((midiDataMSB & 0x7F) << 7));

		// --- normalize 0x3FFF = 16383
		double value = (double)(midi14_bitValue) / (double)(0x3FFF);

		mapDoubleValue(value, 0.0, 1.0, minValue, maxValue);
		return value;
	}

	/**
	@unipolarIntToMIDI14_bit
	\ingroup MIDIFunctions
	\brief
	Converts value from 0 -> 16383 into 14-bit MIDI bytes, pass by reference

	\param unipolarValue value to convert
	\param midiDataLSB returned MIDI LSB (7 bits are valid)
	\param midiDataMSB returned MIDI MSB (7 bits are valid)
	*/
	inline void unipolarIntToMIDI14_bit(uint32_t unipolarValue, uint32_t& midiDataLSB, uint32_t& midiDataMSB)
	{
		// --- convert to 16-bit unsigned short
		unsigned short shValue = (unsigned short)unipolarValue;
		unsigned short shd1 = shValue & 0x007F;

		// --- shift back by 1
		unsigned short shd2 = shValue << 1;

		// --- split into MSB, LSB
		shd2 = shd2 & 0x7F00;

		// --- shift  MSB back to fill LSB position
		shd2 = shd2 >> 8;

		// --- copy into unsigned ints, fill lower portions
		midiDataLSB = shd1;
		midiDataMSB = shd2;
	}


	/**
	@unipolarIntToMIDI14_bit
	\ingroup MIDIFunctions
	\brief
	Converts int value on the range [minValue, maxValue] into 14-bit MIDI bytes, pass by reference

	\param biPolarValue value to convert
	\param minValue minimum value of input range
	\param maxValue maximum value of input range
	\param midiDataLSB returned MIDI LSB (7 bits are valid)
	\param midiDataMSB returned MIDI MSB (7 bits are valid)
	*/
	inline void bipolarIntToMIDI14_bit(int32_t biPolarValue, int32_t minValue, int32_t maxValue, uint32_t& midiDataLSB, uint32_t& midiDataMSB)
	{
		// --- convert bipolar value to 14-bit value
		mapIntValue(biPolarValue, minValue, maxValue, 0, 16383);

		// --- convert to 16-bit unsigned short
		unsigned short shValue = (unsigned short)biPolarValue;
		unsigned short shd1 = shValue & 0x007F;

		// --- shift back by 1
		unsigned short shd2 = shValue << 1;

		// --- split into MSB, LSB
		shd2 = shd2 & 0x7F00;

		// --- shift  MSB back to fill LSB position
		shd2 = shd2 >> 8;

		// --- copy into unsigned ints, fill lower portions
		midiDataLSB = shd1;
		midiDataMSB = shd2;
	}


	/**
	@unipolarIntToMIDI14_bit
	\ingroup MIDIFunctions
	\brief
	Converts double value on the range [0, 1] into 14-bit MIDI bytes, pass by reference

	\param unipolarValue value to convert
	\param midiDataLSB returned MIDI LSB (7 bits are valid)
	\param midiDataMSB returned MIDI MSB (7 bits are valid)
	*/
	inline void unipolarDoubleToMIDI14_bit(double unipolarValue, uint32_t& midiDataLSB, uint32_t& midiDataMSB)
	{
		// --- should never happen, but needed
		boundValue(unipolarValue, 0.0, 1.0);

		// --- convert to 16-bit unsigned short, on range of [0, 16383]
		unsigned short shValue = (unsigned short)(unipolarValue * (double)(0x3FFF));
		unsigned short shd1 = shValue & 0x007F;

		// --- shift back by 1
		unsigned short shd2 = shValue << 1;

		// --- split into MSB, LSB
		shd2 = shd2 & 0x7F00;

		// --- shift  MSB back to fill LSB position
		shd2 = shd2 >> 8;

		// --- copy into unsigned ints, fill lower portions
		midiDataLSB = shd1;
		midiDataMSB = shd2;
	}


	/**
	@midiPitchBendToBipolar
	\ingroup MIDIFunctions
	\brief
	Converts MIDI data bytes 1 and 2 into a bipolar [-1, +1] value

	\param midiData1 LSB
	\param midiData2 MSB

	\return the double, unipolar version of the pitch shift value
	*/
	inline double midiPitchBendToBipolar(uint32_t midiData1, uint32_t midiData2)
	{
		int midiPitchBendValue = (int)((midiData1 & 0x7F) | ((midiData2 & 0x7F) << 7));
		return (double)(midiPitchBendValue - 0x2000) / (double)(0x2000);
	}


	/**
	@doUnipolarModulationFromMin
	\ingroup ModulationFunctions

	@brief Perform unipolar modulation from a min value up to a max value using a unipolar modulator value

	\param unipolarModulatorValue modulation value on range [0.0, +1.0]
	\param minValue lower modulation limit
	\param maxValue upper modulation limit
	\return the modulated value
	*/
	inline double doUnipolarModulationFromMin(double unipolarModulatorValue, double minValue, double maxValue)
	{
		// --- UNIPOLAR bound
		boundValue(unipolarModulatorValue, 0.0, 1.0);

		// --- modulate from minimum value upwards
		return unipolarModulatorValue*(maxValue - minValue) + minValue;
	}

	/**
	@doUnipolarModulationFromMax
	\ingroup ModulationFunctions

	@brief Perform unipolar modulation from a max value down to a min value using a unipolar modulator value

	\param unipolarModulatorValue modulation value on range [0.0, +1.0]
	\param minValue lower modulation limit
	\param maxValue upper modulation limit
	\return the modulated value
	*/
	inline double doUnipolarModulationFromMax(double unipolarModulatorValue, double minValue, double maxValue)
	{
		// --- UNIPOLAR bound
		boundValue(unipolarModulatorValue, 0.0, 1.0);

		// --- modulate from maximum value downwards
		return maxValue - (1.0 - unipolarModulatorValue)*(maxValue - minValue);
	}

	/**
	@doBipolarModulation
	\ingroup ModulationFunctions

	@brief Perform bipolar modulation about a center that his halfway between the min and max values

	\param bipolarModulatorValue modulation value on range [-1.0, +1.0]
	\param minValue lower modulation limit
	\param maxValue upper modulation limit
	\return the modulated value
	*/
	inline double doBipolarModulation(double bipolarModulatorValue, double minValue, double maxValue)
	{
		// --- BIPOLAR bound
		boundValueBipolar(bipolarModulatorValue);

		// --- calculate range and midpoint
		double halfRange = (maxValue - minValue) / 2.0;
		double midpoint = halfRange + minValue;

		return bipolarModulatorValue*(halfRange)+midpoint;
	}

	/**
	@splitBipolar
	\ingroup ModulationFunctions

	@brief maps a unipolar value across two unipolar values, from middle

	INPUT:	   0.0	0.25   0.5  0.75   1.0
				|-----|-----|-----|-----|
	OUTPUT:	   1.0	 0.5   0.0   0.5   1.0

	\param value value to convert
	\return the bipolar value
	*/
	inline double splitBipolar(double value)
	{
		return value >= 0.5 ? 2.0*value - 1.0 : 1.0 - 2.0*value;
	}


	/**
	@bipolar
	\ingroup ModulationFunctions

	@brief calculates the bipolar [-1.0, +1.0] value FROM a unipolar [0.0, +1.0] value

	\param value value to convert
	\return the bipolar value
	*/
	inline double bipolar(double value)
	{
		return 2.0*value - 1.0;
	}

	/**
	@bipolarXForm
	\ingroup ModulationFunctions

	@brief identical to bipolar( ) but returns value with pass-by-reference argument

	\param value value to convert, returns with pass-by-reference
	*/
	inline void bipolarXForm(double& value)
	{
		value = 2.0*value - 1.0;
	}

	/**
	@unipolar
	\ingroup ModulationFunctions

	@brief calculates the unipolar [0.0, +1.0] value FROM a bipolar [-1.0, +1.0] value

	\param value value to convert
	\return the unipolar value
	*/
	inline double unipolar(double value)
	{
		return 0.5*value + 0.5;
	}

	/**
	@unipolarXForm
	\ingroup ModulationFunctions

	@brief identical to unipolar( ) but returns value with pass-by-reference argument

	\param value value to convert, returns with pass-by-reference
	*/
	inline void unipolarXForm(double& value)
	{
		value = 0.5*value + 0.5;
	}

	/**
	@rawTo_dB
	\ingroup SynthFunctions

	@brief calculates dB for given input

	\param raw value to convert to dB
	\return the dB value
	*/
	inline double raw2dB(double raw)
	{
		return 20.0*log10(raw);
	}

	/**
	@dBTo_Raw
	\ingroup SynthFunctions

	@brief converts dB to raw value

	\param dB value to convert to raw
	\return the raw value
	*/
	inline double dB2Raw(double dB)
	{
		return pow(10.0, (dB / 20.0));
	}

	/**
	@peakGainFor_Q
	\ingroup SynthFunctions

	@brief calculates the peak magnitude for a given Q

	\param Q the Q value
	\return the peak gain (not in dB)
	*/
	inline double peakGainFor_Q(double Q)
	{
		// --- no resonance at or below unity
		if (Q <= 0.707) return 1.0;
		return (Q*Q) / (pow((Q*Q - 0.25), 0.5));
	}

	/**
	@dBPeakGainFor_Q
	\ingroup SynthFunctions

	@brief calculates the peak magnitude in dB for a given Q

	\param Q the Q value
	\return the peak gain in dB
	*/
	inline double dBPeakGainFor_Q(double Q)
	{
		return raw2dB(peakGainFor_Q(Q));
	}
	
	// --- limit to [0.1, 0.9] to prevent jagged slopes/aliasing
	const double pdSlope = (0.9 - 0.1) / (1.0 - 0.0);

	// --- for phase distortion // modulatedShape =  parameters->shape + modulationInputs[kAuxBipolarMod_1]
	inline double applyPhaseDistortion(double mcounter, double x_break, double y_break = 0.5)
	{
		mapDoubleValue(x_break, 0.0, 0.1, pdSlope);

		// --- calc 2 slopes
		double m1 = y_break / x_break;
		double m2 = (1.0 - y_break) / (1.0 - x_break);

		// --- only can happen if m1 = m2 = 1.0
		if (m2 == m1)
			return mcounter;

		// --- apply distortion
		if (mcounter <= x_break)
			return m1*mcounter;
		else
			return (mcounter - x_break)*m2 + y_break;

		return mcounter;
	}

	/**
	@sgn
	\ingroup SynthFunctions

	@brief calculates sgn( ) of input
	\param xn the input value
	\return -1 if xn is negative or +1 if xn is 0 or greater
	*/
	inline double sgn(double xn)
	{
		return (xn > 0) - (xn < 0);
	}

	/**
	@calcWSGain
	\ingroup SynthFunctions

	@brief calculates gain of a waveshaper
	\param xn the input value
	\param saturation the saturation control
	\param asymmetry the degree of asymmetry
	\return gain value
	*/
	inline double calcWSGain(double xn, double saturation, double asymmetry)
	{
		double g = ((xn >= 0.0 && asymmetry > 0.0) || (xn < 0.0 && asymmetry < 0.0)) ? saturation * (1.0 + 4.0*fabs(asymmetry)) : saturation;
		return g;
	}

	/**
	@atanWaveShaper
	\ingroup SynthFunctions

	@brief calculates arctangent waveshaper
	\param xn the input value
	\param saturation the saturation control
	\return the waveshaped output value
	*/
	inline double atanWaveShaper(double xn, double saturation)
	{
		if (saturation == 0) return xn;
		return atan(saturation*xn) / atan(saturation);
	}

	/*inline double tanhApprox(double xn)
	{
		return (-0.67436811832e-5 + (0.2468149110712040 + (0.583691066395175e-1 + 0.3357335044280075e-1*xn)*xn)*xn) / (0.2464845986383725 + (0.609347197060491e-1 + (0.1086202599228572 + 0.2874707922475963e-1*xn)*xn)*xn);
	}*/

	/**
	@tanhWaveShaper
	\ingroup SynthFunctions

	@brief calculates hyptan waveshaper
	\param xn the input value
	\param saturation  the saturation control
	\return the waveshaped output value
	*/
	inline double tanhWaveShaper(double xn, double saturation)
	{
		if (saturation == 0) return xn;
		return tanh(saturation*xn) / tanh(saturation);
	}

	/**
	@softClipWaveShaper
	\ingroup SynthFunctions

	@brief calculates hyptan waveshaper
	\param xn the input value
	\param saturation the saturation control
	\return the waveshaped output value
	*/
	inline double softClipWaveShaper(double xn, double saturation)
	{
		if (saturation == 0) return xn;

		// --- un-normalized soft clipper from Reiss book
		return sgn(xn)*(1.0 - exp(-fabs(saturation*xn)));
	}

	/**
	@fuzzExp1WaveShaper
	\ingroup SynthFunctions

	@brief calculates fuzz exp1 waveshaper
	\param xn the input value
	\param saturation the saturation control
	\return the waveshaped output value
	*/
	inline double fuzzExp1WaveShaper(double xn, double saturation, double asymmetry)
	{
		if (saturation == 0) return xn;

		// --- setup gain
		double wsGain = calcWSGain(xn, saturation, asymmetry);
		return sgn(xn)*(1.0 - exp(-fabs(wsGain*xn))) / (1.0 - exp(-wsGain));
	}

	
	/**
	@copyStingList
	\ingroup SynthFunctions

	@brief 
	Copies one vector of strings to another

	\param source the source vector to copy
	\param destination the vector that receives the copy
	*/
	inline void copyStingList(std::vector<std::string>& source, std::vector<std::string>& destination)
	{
		destination.clear();
		uint64_t count = source.size();
		for (uint32_t i = 0; i < count; i++)
		{
			std::string str = source[i];

			destination.push_back(str);
		}
	}

	/**
	@concatStrings
	\ingroup SynthFunctions

	@brief
	Concatenate two strings

	\param s1 the first string
	\param s2 the second string

	\returns the concatenation of the two as s1s2
	*/
	inline std::string concatStrings(std::string s1, std::string s2)
	{
		std::string ret = s1;
		ret.append(s2);
		return ret;
	}

	/**
	@ReplaceSubStrWithStr
	\ingroup SynthFunctions

	@brief
	Helper to relace strings within a std::string object
	- https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string

	\param str the string to manipulate, pass-by-reference
	\param from the string to find
	\param to the string to replace with
	*/
	inline void ReplaceSubStrWithStr(std::string& str, const std::string& from, const std::string& to) 
	{
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
	}

	/**
	@stripLastFolderFromPath
	\ingroup SynthFunctions

	@brief
	Helper to strip the last folder path from a given path
	-  https://stackoverflow.com/questions/10364877/c-how-to-remove-filename-from-path-string/18730891

	\param str the string to manipulate, pass-by-reference
	*/
	inline void stripLastFolderFromPath(std::string& str)
	{
		str = str.substr(0, str.find_last_of("\\/"));
	}

	/**
	@getPluginContainerFolder
	\ingroup SynthFunctions

	@brief
	Helper to get the outer plugin folder with the inner folder path.
	- this assumes bundle packaging which is the same for AU, AAX, VST3, and RAFX

	\param str the string to manipulate, pass-by-reference
	*/
	inline void getPluginContainerFolder(std::string& str)
	{
		for(uint32_t i=0; i<3; i++)
			stripLastFolderFromPath(str);
	}

	/**
	@doLinearInterpolation
	\ingroup SynthFunctions

	@brief performs linear interpolation of x distance between two (x,y) points;
	returns interpolated value

	**NOTE** you must ensure that the x coordinates are not identical, x1 != x2
	         to avoid divide by zero; this code needs to be as efficient as possible
			 So, either make sure programatically or check x1,x2 prior to calling!

	\param x1 the x coordinate of the first point
	\param x2 the x coordinate of the second point
	\param y1 the y coordinate of the first point
	\param y2 the 2 coordinate of the second point
	\param x the interpolation location
	\return the interpolated value or y1 if the x coordinates are unusable
	*/
	inline double doLinearInterpolation(double x1, double x2, double y1, double y2, double x)
	{
		double dydx = (y2 - y1) / (x2 - x1);                                   
		return y1 + dydx * (x - x1);
	}

	/**
	@doLinearInterpolation
	\ingroup SynthFunctions

	@brief performs linear interpolation of fractional x distance between two adjacent (x,y) points;
	returns interpolated value

	\param y1 the y coordinate of the first point
	\param y2 the 2 coordinate of the second point
	\param fractional_X the interpolation location as a fractional distance between x1 and x2 (which are not needed)
	\return the interpolated value or y2 if the interpolation is outside the x interval
	*/
	inline double doLinearInterpolation(double y1, double y2, double fractional_X)
	{
		double dydx = (y2 - y1);
		return y1 + dydx * (fractional_X);

		//// --- check invalid condition
		//if (fractional_X >= 1.0) return y2;

		//// --- use weighted sum method of interpolating
		//return fractional_X*y2 + (1.0 - fractional_X)*y1;
	}

	/**
	@doLagrangeInterpolation
	\ingroup SynthFunctions

	@brief implements n-order Lagrange Interpolation

	\param x Pointer to an array containing the x-coordinates of the input values
	\param y Pointer to an array containing the y-coordinates of the input values
	\param n the order of the interpolator, this is also the length of the x,y input arrays
	\param xbar The x-coorinates whose y-value we want to interpolate
	\return the interpolated value
	*/
	inline double doLagrangeInterpolation(double* x, double* y, int n, double xbar)
	{
		int i, j;
		double fx = 0.0;
		double l = 1.0;
		for (i = 0; i<n; i++)
		{
			l = 1.0;
			for (j = 0; j<n; j++)
			{
				if (j != i)
					l *= (xbar - x[j]) / (x[i] - x[j]);
			}
			fx += l*y[i];
		}
		return (fx);
	}


	/**
	@pitchShiftTableLookup
	\ingroup ModulationFunctions
	\brief 
	use lookup table to find pitch shift multipliers, uses linear interpolation

	\param semitones the number of semitones to pitch shift (fractional)

	\returns the pitch shift multiplier (+12 semitones -> 2.0, -12 semitones -> 0.5)
	*/
	//
	// <----- NOTE: This function was removed because of the insane size of the lookup table (16k)
	// <-----       If you want to use it, #include "pitchshifttable.h and then uncomment this function
	//
	//inline double pitchShiftTableLookup(double semitones)
	//{
	//	double index = kPitchShiftTableCenter + (semitones*kPitchShiftTableCenterSemis);
	//	if (index >= kPitchShiftTableEnd) return pitchShiftTable[kPitchShiftTableEnd];
	//	int intPart = (int)index;
	//	return doLinearInterpolation(0.0, 1.0, pitchShiftTable[intPart], pitchShiftTable[intPart + 1], index - intPart);
	//}


	/**
	@midiNoteNumberFromOscFrequency
	\ingroup MIDIFunctions
	\brief
	converts an oscillator frequency in Hz to the corresponding MIDI note number
	- note that you can apply non-A-440 tuning standards but applying the current value of that A
	for example frequencyA440 -> 432.0Hz etc...
	- https://newt.phys.unsw.edu.au/jw/notes.html

	\param oscillatorFrequency frequency in Hz to convert
	\param frequencyA440 reference frequency of concert-A

	\returns the MIDI note number for the oscillator pitch
	*/
	inline uint32_t midiNoteNumberFromOscFrequency(double oscillatorFrequency, double frequencyA440 = 440.0)
	{
		// --- ceil will round up
		double midiNote = ceil(12.0*log2(oscillatorFrequency / frequencyA440) + 69.0); //--- 69 = MIDI note # for A-440

		// --- grab int part
		uint32_t renderMidiNoteNumber = (uint32_t)(midiNote);

		// --- bound to 0 -> 127
		boundMIDIValueByte(renderMidiNoteNumber);

		return renderMidiNoteNumber;
	}

	/**
	@midiNoteNumberToOscFrequency
	\ingroup MIDIFunctions
	\brief
	converts a MIDI note number into a corresponding oscillator frequency in Hz 
	- note that you can apply non-A-440 tuning standards but applying the current value of that A
	for example frequencyA440 -> 432.0Hz etc...
	- https://newt.phys.unsw.edu.au/jw/notes.html

	\param midiNoteNumber note number to convert
	\param frequencyA440 reference frequency of concert-A

	\returns the frequency in Hz of the pitch
	*/
	inline double midiNoteNumberToOscFrequency(uint32_t midiNoteNumber, double frequencyA440 = 440.0)
	{
		// --- https://newt.phys.unsw.edu.au/jw/notes.html
		double pitch = pow(2.0, (((double)midiNoteNumber - 69.0) / 12.0))*frequencyA440;

		return pitch;
	}

	/**
	@calculateWaveTablePhaseInc
	\ingroup SynthFunctions
	\brief
	calculates the phase-increment for a wavetable for a target oscillator frequency

	\param oscFrequency target oscillator frequency
	\param sampleRate fs
	\param wavetableLength table length

	\returns the phase-increment for a wavetable lookup
	*/
	inline double calculateWaveTablePhaseInc(double oscFrequency, double sampleRate, uint32_t wavetableLength)
	{
		// --- for wavetables, inc = (kWaveTableLength)*(fo/fs)
		return wavetableLength*(oscFrequency / sampleRate);
	}

	/**
	@checkAndWrapWaveTableIndex
	\ingroup SynthFunctions
	\brief
	check and do modulo (fmod) wrap of a wavetable index value

	\param index modulo counter value, retured with pass-by-reference
	\param tableLength wavetable length in samples

	\returns the phase-increment for a wavetable lookup
	*/
	inline bool checkAndWrapWaveTableIndex(double& index, uint32_t tableLength)
	{
		if (index > tableLength)
		{
			index = fmod(index, tableLength);
			return true;
		}
		else if (index < 0.0)
		{
			index = tableLength + fmod(index - tableLength, tableLength);
			return true;
		}

		return false;
	}

	/**
	@countTrailingZero
	\ingroup SynthFunctions
	\brief
	count trailing zeros in a binary number
	- https://www.geeksforgeeks.org/count-trailing-zero-bits-using-lookup-table/

	\param x binary numnber to count trailing zeros

	\returns the number of trailing zeros as int
	*/
	inline int countTrailingZero(int x)
	{
		// Map a bit value mod 37 to its position
		static const int lookup[] = { 32, 0, 1,
			26, 2, 23, 27, 0, 3, 16, 24, 30, 28, 11,
			0, 13, 4, 7, 17, 0, 25, 22, 31, 15, 29,
			10, 12, 6, 0, 21, 14, 9, 5, 20, 8, 19,
			18 };

		// Only difference between (x and -x) is
		// the value of signed magnitude(leftmostbit)
		// negative numbers signed bit is 1
		return lookup[(-x & x) % 37];
	}

	/**
	@countTrailingZeros_x64
	\ingroup SynthFunctions
	\brief
	count trailing zeros in a binary number, return in unsigned int
	- https://www.geeksforgeeks.org/count-trailing-zero-bits-using-lookup-table/

	\param x binary numnber to count trailing zeros

	\returns the number of trailing zeros as uint64_t
	*/
	inline uint64_t countTrailingZeros_x64(uint64_t x)
	{
		uint64_t count = 0;
		while ((x & 1) == 0)
		{
			x = x >> 1;
			count++;
		}
		return count;
	}

	inline double doSimpleSineLUT(double angle)
	{
		uint32_t tableMask = sineTableLength - 1;
		double index = angle*(tableMask / kTwoPi);
		if (index > tableMask) return 0.0;

		// --- two samples from table
		double wtData[2] = { 0.0, 0.0 };

		// --- location = N(fo/fs)
		double wtReadLocation = index;

		// --- split the fractional index into int.frac parts
		uint32_t readIndex = (uint32_t)wtReadLocation;
		uint32_t nextReadIndex = (readIndex + 1) & tableMask;

		// --- two table reads
		wtData[0] = sinetable[readIndex];
		wtData[1] = sinetable[nextReadIndex];

		// --- interpolate the output
		double fracPart = wtReadLocation - readIndex;
		double output = doLinearInterpolation(wtData[0], wtData[1], fracPart);
		return output;
	}

	inline double doSimpleCosineLUT(double angle)
	{
		uint32_t tableMask = sineTableLength - 1;
		double index = angle*(tableMask / kTwoPi);
		if (index > tableMask) return 0.0;

		// --- two samples from table
		double wtData[2] = { 0.0, 0.0 };

		// --- location = N(fo/fs)
		double wtReadLocation = index;

		// --- split the fractional index into int.frac parts
		uint32_t readIndex = (uint32_t)wtReadLocation;
		uint32_t nextReadIndex = (readIndex + 1) & tableMask;

		// --- two table reads
		wtData[0] = costable[readIndex];
		wtData[1] = costable[nextReadIndex];

		// --- interpolate the output
		double fracPart = wtReadLocation - readIndex;
		double output = doLinearInterpolation(wtData[0], wtData[1], fracPart);
		return output;
	}

	/**
	@calculatePanValues
	\ingroup SynthFunctions
	\brief
	calculates the left and right pan values from a bipolar (-1 -> 1) value
	- returns are via pass-by-reference mechanism
	- uses sin/cos quadrants for constant-power curve

	\param bipolarModulator bipolar input value
	\param leftPanValue returned left pan multiplier
	\param rightPanValue returned right pan multiplier
	*/
	inline void calculatePanValues(double bipolarModulator, double& leftPanValue, double& rightPanValue)
	{
		//leftPanValue = cos((kPiOverFour)*(bipolarModulator + 1.0));
		//rightPanValue = sin((kPiOverFour)*(bipolarModulator + 1.0));
		leftPanValue = doSimpleCosineLUT((kPiOverFour)*(bipolarModulator + 1.0));
		rightPanValue = doSimpleSineLUT((kPiOverFour)*(bipolarModulator + 1.0));
		boundValueUnipolar(leftPanValue);
		boundValueUnipolar(rightPanValue);
	}

	/**
	@calculateConstPwrMixValues
	\ingroup SynthFunctions
	\brief
	converts bipolar (-1 -> 1) value into a pair of constant power mixing coefficients
	- returns are via pass-by-reference mechanism
	- identical to constant power pan function

	\param bipolarModulator bipolar input value
	\param mixValueA returned channel A coefficient
	\param mixValueB returned channel B coefficient
	*/
	inline void calculateConstPwrMixValues(double bipolarModulator, double& mixValueA, double& mixValueB)
	{
		calculatePanValues(bipolarModulator, mixValueA, mixValueB);
	}


	/**
	@crossfade
	\ingroup SynthFunctions
	\brief
	crossfade two values And B together by some fractional amount
	- frac = 0.0 is 100%A
	- frac = 1.0 is 100%A
	- frac = 0.5 is a 50%A + 50%B mixture

	\param xfadeType type of crossfade: linear, square law, cosntant power
	\param inputA value A
	\param inputB value B
	\param xFrac fractional distance between them to mix

	\returns the mixture of A and B
	*/
	inline double crossfade(XFadeType xfadeType, double inputA, double inputB, double xFrac)
	{
		double output = 0.0;

		// --- calculate gains
		// --- constant power (same as pan calc)
		double gainA = 1.0;
		double gainB = 0.0;
		calculateConstPwrMixValues(bipolar(xFrac), gainA, gainB);

		if (xfadeType == XFadeType::kConstantPower)
			output = inputA*gainA + inputB*gainB;
		else if (xfadeType == XFadeType::kSquareLaw)
			output = inputA*(1.0 - xFrac*xFrac) + inputB*(1.0 - (xFrac - 1.0)*(xFrac - 1.0)); // A gain = 1 - x^2, B gain = 1 - ((x-1)^2)
		else //  linear
			output = inputA*(1.0 - xFrac + inputB*(xFrac));

		return output;
	}


	/**
	@semitonesBetweenFrequencies
	\ingroup SynthFunctions
	\brief
	calculates the number of semitones between a start and end frequency

	\param startFrequency start frequency in Hz
	\param endFrequency end frequency in Hz

	\returns the number of semitones between
	*/
	inline double semitonesBetweenFrequencies(double startFrequency, double endFrequency)
	{
		return log2(endFrequency / startFrequency)*12.0;
	}

	/**
	@NoteDuration
	\ingroup Constants-Enums
	\brief
	emumearation of musical durations, shortest to longest 
	*/
	enum class NoteDuration {
		k32ndTriplet, k32nd, k16thTriplet, kDot32nd,
		k16th, k8thTriplet, kDot16th,
		k8th, kQuarterTriplet, kDot8th,
		kQuarter, kHalfTriplet, kDotQuarter,
		kHalf, kWholeTriplet, kDotHalf,
		kWhole, kDotWhole,
		kOff,
		kNumNoteDurations
	};

	/**
	@noteDurationTable
	\ingroup Constants-Enums
	\brief
	table of multiplier values that convert a quarter note duration into any of the others in the table

	dotWhole = 3(half) = 6.0
	dotHalf = 3(quarter) = 3.0
	dotQuarter = 3(8th) = 1.5
	dot8th = 3(16th) = 0.75
	dot16th = 3(32nd) = 0.375
	dot32th = 3(64nd) = 3(0.0625) = 0.1875
	tripWhole= doubleWhole / 3 = 2.6667
	tripHalf = whole / 3 = 1.333
	tripQuarter = half / 3 = 0.667
	trip8th = quarter / 3 = 0.333
	trip16th = 8th / 3 = 0.1667
	trip32nd = 16th / 3 = 0.08333
	*/
	static const double noteDurationTable[int(NoteDuration::kNumNoteDurations)] = {
		0.0833, /* k32ndTriplet */

		0.125,	/* k32nd */
		0.1667,	/* k16thTriplet */
		0.1875,	/* kDot32nd */

		0.25,	/* k16th */
		0.333,	/* k8thTriplet */
		0.375,	/* kDot16th */

		0.5,	/* k8th */
		0.667,	/* kQuartTriplet */
		0.75,	/* kDot8th */

		1.0,	/* kQuart */
		1.333,	/* kHalfTriplet */
		1.5,	/* kDotQuart */

		2.0,	/* kHalf */
		2.667,	/* kWholeTriplet */
		3.0,	/* kDotHalf */

		4.0,    /* kWhole */
		6.0,	/* kDotWhole */

		0.0 }; 	/* off */

	/**
	@getTimeFromTempo
	\ingroup SynthFunctions
	\brief
	converts a BPM value and a NoteDuration into a time

	\param BPM timing BPM value
	\param duration NoteDuration constant
	\param returnMilliseconds set true if you want milliseconds

	\returns the time in seconds or milliseconds
	*/
	inline double getTimeFromTempo(double BPM, NoteDuration duration, bool returnMilliseconds = false)
	{
		if (duration == NoteDuration::kOff)
		{
			if (returnMilliseconds)
				return 1.0;
			else
				return 0.001;
		}

		// milliseconds from BPM
		double mSec = 60000.0 / BPM;

		// --- scale by table entry
		double time = 0.0;
		if (returnMilliseconds)
			time = (noteDurationTable[enumToInt(duration)] * mSec);
		else
			time = (noteDurationTable[enumToInt(duration)] * mSec) / 1000.0;

		return time;
	}

	/**
	@getTimeFromTempo
	\ingroup SynthFunctions
	\brief
	converts a BPM value and a normalized note multiplier on the range [0, 1]
	to a note duration multiplier 
	- there are fewer choices with this function ranging from a whole note to a sixteenth note
	- used specifically for the mod-knobs, which transmit normalized values
	- used for the LFO's sync-to-BPM feature

	\param BPM timing BPM value
	\param normalizedNoteMult normalized value

	\returns the time in seconds or milliseconds
	*/
	inline double getTimeFromTempo(double BPM, double normalizedNoteMult)
	{
		if (normalizedNoteMult < 0.01)
			return 0.0;

		// --- micro lookup table of rhythmic multipliers
		static const double lookup[] = { 4.0, 2.0, 1.5, 1.0, 0.75, 0.5, 0.375, 0.25 };
		uint32_t index = mapDoubleToUINT(normalizedNoteMult, 0.0, 1.0, 0, 7);
		double mSec = 60000.0 / BPM;
		return (lookup[index] * mSec) / 1000.0;
	}

	/**
	@quantizeBipolarValue
	\ingroup SynthFunctions
	\brief
	Quantizes a double value into some number of quantization levels
	- used for the step-LFO feature

	\param d the double value to quntize
	\param qLevels number of quantization levels

	\returns the quantized value
	*/
	inline double quantizeBipolarValue(double d, uint32_t qLevels)
	{
		uint32_t u = mapDoubleToUINT(d, -1.0, 1.0, 0, qLevels, true);
		return mapUINTToDouble(u, 0, qLevels, -1.0, 1.0);
	}

	///* union for dataype conversion without bit mangling */
	//union dataUnion
	//{
	//	dataUnion(float _f) { f = _f; }
	//	dataUnion(double _d) { d = _d; }
	//	dataUnion(uint32_t _u) { u = _u; }
	//	dataUnion(uint64_t _u64) { u64 = _u64; }
	//	float f = 0.0; // only need one initializer
	//	double d;
	//	uint32_t u;
	//	uint64_t u64;
	//};


	/**
	@doubleToUint64
	\ingroup SynthFunctions
	\brief
	maps a double value into a uint64 value without casting or mangling bits
	- used to store double values as 64-bit HEX strings for making tables

	\param d the double value to convert

	\returns the input value, bit for bit, encoded as a uint64_t
	*/
	inline uint64_t doubleToUint64(double d)
	{
		return *(reinterpret_cast<uint64_t*>(&d));
	}

	/**
	@uint64ToDouble
	\ingroup SynthFunctions
	\brief
	maps a uint64 value to a double value without casting or mangling bits
	- used to decode double values that were stored as 64-bit HEX strings 

	\param u the uint64_t value to convert

	\returns the input value, bit for bit, encoded as a double
	*/
	inline double uint64ToDouble(uint64_t u)
	{
		return *(reinterpret_cast<double*>(&u));
	}

	/**
	@floatToUint32
	\ingroup SynthFunctions
	\brief
	maps a float value to a uint32_t value without casting or mangling bits
	- used to store double values as 32-bit HEX strings for making tables

	\param f the float value to convert

	\returns the input value, bit for bit, encoded as a uint32_t
	*/
	inline uint32_t floatToUint32(float f)
	{
		return *(reinterpret_cast<uint32_t*>(&f));
	}

	/**
	@uint32ToFloat
	\ingroup SynthFunctions
	\brief
	maps a uint32_t value to a float value without casting or mangling bits
	- used to decode double values that were stored as 32-bit HEX strings

	\param u the uint32_t value to convert

	\returns the input value, bit for bit, encoded as a float
	*/
	inline float uint32ToFloat(uint32_t u)
	{
		return *(reinterpret_cast<float*>(&u));
	}


	/**
	@inRange
	\ingroup SynthFunctions
	\brief
	tests a number to see if it is withing a certain range
	- https://www.geeksforgeeks.org/how-to-check-whether-a-number-is-in-the-rangea-b-using-one-comparison

	\param low the minimum value of the range
	\param high the maximum value of the range
	\param x the value to test

	\returns true if the value is within the range, including endpoints
	*/
	inline bool inRange(double low, double high, double x)
	{
		return ((x - high)*(x - low) <= 0.0);
	}

	/**
	@wrapMax
	\ingroup SynthFunctions
	\brief
	wraps a value around a maximum value enough times that it falls within the maximum boundary
	- wraps x from 0 to max
	- https://stackoverflow.com/questions/4633177/c-how-to-wrap-a-float-to-the-interval-pi-pi

	\param x the value to wrap
	\param max the maximum value to wrap over

	\returns the wrapped value
	*/
	inline double wrapMax(double x, double max)
	{
		/* integer math: `(max + x % max) % max` */
		return fmod(max + fmod(x, max), max);
	}

	/**
	@wrapMinMax
	\ingroup SynthFunctions
	\brief
	wraps a value around a maximum value enough times that it falls within the maximum boundary
	- wraps x from min to max
	- https://stackoverflow.com/questions/4633177/c-how-to-wrap-a-float-to-the-interval-pi-pi

	\param x the value to wrap
	\param min the minimum value to wrap from
	\param max the maximum value to wrap over

	\returns the wrapped value
	*/
	inline double wrapMinMax(double x, double min, double max)
	{
		return min + wrapMax(x - min, max - min);
	}

	/**
	@charArrayToStringVector
	\ingroup SynthFunctions
	\brief
	Converts an old fasioned array of char* strings to a vector of std::strings

	\param charArray the array to convert
	\param size length of array
	\param ignoreString string to ignore and NOT add to the vector

	\returns the std::vector<std::string> version
	*/
	inline std::vector<std::string> charArrayToStringVector(const char** charArray, uint32_t size, std::string ignoreString = "")
	{
		std::vector<std::string> outputVector;
		for (uint32_t i = 0; i < size; i++)
		{
			// const char* cc = charArray[i];
			std::string str(charArray[i]);

			if (str.compare(ignoreString) != 0)
				outputVector.push_back(str);
		}

		return outputVector;
	}

	/**
	@appendCharArrayToStringVector
	\ingroup SynthFunctions
	\brief
	Appends an old fasioned array of char* strings to a vector of std::strings

	\param charArray the array to append
	\param size length of array
	\param outputVector the vector to append, pass by reference
	\param ignoreString string to ignore and NOT append to the vector

	\returns true if successful
	*/
	inline bool appendCharArrayToStringVector(const char** charArray, uint32_t size, std::vector<std::string>& outputVector, std::string ignoreString)
	{
		// std::vector<std::string> outputVector;
		if (size <= 0)
			return false;

		for (uint32_t i = 0; i < size; i++)
		{
			// const char* cc = charArray[i];
			std::string str(charArray[i]);
			if(str.compare(ignoreString) != 0)
				outputVector.push_back(str);
		}

		return true;
	}



	/**
	@mmaMIDItoAtten
	\ingroup MIDIFunctions
	\brief
	calculates the raw attenuation according to MMA DLS spec midiValue = the MIDI (0 -> 127) value to convert
	- uses simple square-law power conversion

	\param midiValue the MIDI velocity value

	\returns the attenuation level
	*/
	inline double mmaMIDItoAtten(uint32_t midiValue)
	{
		if (midiValue == 0)
			return 0.0; // --- floor

		return ((double)midiValue*(double)midiValue) / (127.0*127.0);
	}

	/**
	\struct VectorXFadeData
	\ingroup SynthStructures
	\brief
	Holds mixing coefficient multipliers for the Sequential/Korg vector joystick

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 05 / 2
	*/
	struct VectorXFadeData
	{
		double vectorA = 0.25;
		double vectorB = 0.25;
		double vectorC = 0.25;
		double vectorD = 0.25;

		// --- unipolar
		double vectorAC = 0.5;
		double vectorBD = 0.5;
	};

	
	/**
	@calculateVectorMix
	\ingroup SynthFunctions
	\brief
	converts a joystick (x,y) position into the four mixing coefficients plus the X/Y axis 
	shaodows

	\param joystick_X joytick X coordinate
	\param joystick_Y joytick X coordinate
	\param origin_X origin of X axis
	\param origin_Y origin of Y axis

	\return VectorXFadeData structure of various vector mixing coeffients
	*/
	inline VectorXFadeData calculateVectorMix(double joystick_X, double joystick_Y,
												double origin_X = 0.0, double origin_Y = 0.0)
	{
		VectorXFadeData vectorCoeffs;
		double x = (joystick_X - joystick_Y) / 2.0;
		double y = (joystick_X + joystick_Y) / 2.0;

		// --- these are 0->1 unipolar
		vectorCoeffs.vectorAC = (x + 1.0) / 2.0;
		vectorCoeffs.vectorBD = (y + 1.0) / 2.0;

		// --- offset by origin
		x += origin_X;
		y += origin_Y;

		x *= 127.0;
		y *= 127.0;

		boundValue(x, -128.0, 127.0);/* Limit range to [-128, 127] */
		boundValue(y, -128.0, 127.0);/* Limit range to [-128, 127] */

		// --- offset for center of [0, 255]
		x += 127;
		y += 127;

		// --- the Korg/Sequential Circuits VS Equations
		vectorCoeffs.vectorB = x*y / 645.0;			/* Calculate individual wave % */
		vectorCoeffs.vectorC = x*(255.0 - y) / 645.0;	/* 645=(255^2/100)*127/128 */
		vectorCoeffs.vectorD = (255.0 - x)*(255.0 - y) / 645.0;
		vectorCoeffs.vectorA = 100.0 - vectorCoeffs.vectorB - vectorCoeffs.vectorC - vectorCoeffs.vectorD;

		// --- convert from percent
		vectorCoeffs.vectorA /= 100.0;
		vectorCoeffs.vectorB /= 100.0;
		vectorCoeffs.vectorC /= 100.0;
		vectorCoeffs.vectorD /= 100.0;

		// --- bound to [0, 1] from rounding/truncation
		boundValue(vectorCoeffs.vectorA, 0.0, 1.0);
		boundValue(vectorCoeffs.vectorB, 0.0, 1.0);
		boundValue(vectorCoeffs.vectorC, 0.0, 1.0);
		boundValue(vectorCoeffs.vectorD, 0.0, 1.0);

		return vectorCoeffs;
	}

	/**
	@quadraticSine
	\ingroup SynthFunctions
	\brief
	produces the quadratic sine approximation
	- http://datagenetics.com/blog/july12019/index.html

	\param angle sine angle

	\return quadratic sine approximation at the given angle
	*/
	inline double quadraticSine(double angle)
	{
		double d = fabs(angle);
		double sgn = d == 0 ? 1.0 : angle / d;
		return (4.0 * angle) / (kPiSqared)*(kPi - sgn*angle);
	}

    const double D = 5.0*kPiSqared;
	/**
	@BhaskaraISine
	\ingroup SynthFunctions
	\brief
	produces the Bhaskara's sine approximation
	- more accurate than quadratic sine
     
    \param angle sine angle
    
    \return Bhaskara I approximation

	*/
	inline double BhaskaraISine(double angle)
	{
		double d = fabs(angle);
		double sgn = d == 0 ? 1.0 : angle / d;
		return 16.0*angle*(kPi - sgn*angle) / (D - sgn*4.0*angle*(kPi - sgn*angle));
	}

    const double B = 4.0 / kPi;
    const double C = -4.0 / (kPiSqared);
    const double P = 0.225;
	/**
	@parabolicSine
	\ingroup SynthFunctions
	\brief
	highest accurace sinudoid approximation function 
	-  http://devmaster.net/posts/9648/fast-and-accurate-sine-cosine
	\param angle sine angle -pi to +pi
	*/
	inline double parabolicSine(double angle)
	{
		double y = B * angle + C * angle * fabs(angle);
		return P * (y * fabs(y) - y) + y;
	}


	/**
	@calculatePitchBend
	\ingroup SynthFunctions
	\brief
	Calculate a pitch bend multiplier value based on the global MIDI input values:
	- kMIDIMasterPBSensCoarse (from master controls)
	- kMIDIMasterPBSensFine	(from master controls)
	- kMIDIPitchBendDataLSB (from pitch bend wheel)
	- kMIDIPitchBendDataMSB (from pitch bend wheel)
	- NOTE: the term "Master" is verbatim from the MIDI specifiction as of this writing

	\param midiInputData MIDI input data array as a shared pointer

	\return pitch bend multiplier value as a double
	*/
	inline double calculatePitchBend(std::shared_ptr<MidiInputData> midiInputData)
	{
		// --- calculate MIDI pitch bend range
		double midiPitchBendRange = midiInputData->getGlobalMIDIData(kMIDIMasterPBSensCoarse) +
			(midiInputData->getGlobalMIDIData(kMIDIMasterPBSensFine) / 100.0);

		// --- calculate MIDI pitch bend (USER)
		double midiPitchBend = midiPitchBendRange * midiPitchBendToBipolar(midiInputData->getGlobalMIDIData(kMIDIPitchBendDataLSB),
			midiInputData->getGlobalMIDIData(kMIDIPitchBendDataMSB));

		return midiPitchBend;
	}

	/**
	@calculatePitchBend
	\ingroup MIDIFunctions
	\brief
	Second method to calculate a pitch bend multiplier value based on the global MIDI input values:
	- kMIDIMasterPBSensCoarse (from master controls)
	- kMIDIMasterPBSensFine	(from master controls)
	- kMIDIPitchBendDataLSB (from pitch bend wheel)
	- kMIDIPitchBendDataMSB (from pitch bend wheel)
	- NOTE: the term "Master" is verbatim from the MIDI specifiction as of this writing

	\param midiInputData MIDI input data array as a naked pointer

	\return pitch bend multiplier value as a double
	*/
	inline double calculatePitchBend(IMidiInputData* midiInputData)
	{
		// --- calculate MIDI pitch bend range
		double midiPitchBendRange = midiInputData->getGlobalMIDIData(kMIDIMasterPBSensCoarse) +
			(midiInputData->getGlobalMIDIData(kMIDIMasterPBSensFine) / 100.0);

		// --- calculate MIDI pitch bend (USER)
		double midiPitchBend = midiPitchBendRange * midiPitchBendToBipolar(midiInputData->getGlobalMIDIData(kMIDIPitchBendDataLSB),
			midiInputData->getGlobalMIDIData(kMIDIPitchBendDataMSB));

		return midiPitchBend;
	}

	/**
	@calculateMasterTuning
	\ingroup MIDIFunctions
	\brief
	Method to calculate a tuning (pitch bend) multiplier value based on the global MIDI input values:
	- kMIDIMasterTuneCoarseMSB  (from master controls)
	- kMIDIMasterTuneFineLSB	(from master controls)
	- NOTE: the term "Master" is verbatim from the MIDI specifiction as of this writing

	\param midiInputData MIDI input data array as a naked pointer

	\return pitch bend multiplier value as a double
	*/
	inline double calculateMasterTuning(IMidiInputData* midiInputData)
	{
		// --- coarse (semitones): -64 to +63 maps-> 0, 127 (7-bit)
		int mtCoarse = midiInputData->getGlobalMIDIData(kMIDIMasterTuneCoarseMSB);
		mapIntValue(mtCoarse, 0, 127, -64, +63, false); // false = no rounding

		// --- fine (cents): -100 to +100 as MIDI 14-bit
		double mtFine = midi14_bitToDouble(midiInputData->getGlobalMIDIData(kMIDIMasterTuneFineLSB),
			midiInputData->getGlobalMIDIData(kMIDIMasterTuneFineMSB), -100.0, 100.0);

		// --- this gives proper int.fraction value
		return (double)mtCoarse + ((double)mtFine / 100.0);
	}

	/**
	@calculateNumTables
	\ingroup SynthFunctions
	\brief
	Calculates the number of wavetables needed to cover the MIDI keyboard starting from
	a seed note, and them progressing up the keyboard by some number of semitones between each table

	\param seedMIDINote MIDI the lowest MIDI note with a wavetable
	\param tableIntervalSemitones the interval to skip when counting tables; if this value is 1
	then there will be a table on each MIDI note chromatically (semitone); if this value is 3
	then there will be a table on each minor third boundary

	\return the number of tables needed
	*/
	inline uint32_t calculateNumTables(uint32_t seedMIDINote, uint32_t tableIntervalSemitones)
	{
		uint32_t count = 0;
		uint32_t nextMIDINote = seedMIDINote;
		if (seedMIDINote > 127) return 0;
		if (seedMIDINote == 127) return 1;
		while (nextMIDINote < 128)
		{
			nextMIDINote += tableIntervalSemitones;
			count++;
		}
		return count;
	}

	/**
	\ingroup Constants-Enums
	Built-in Concave/Convex lookup table length */
	static uint32_t xformLUTLen = 512;
	
	/**
	\ingroup Constants-Enums
	Built-in concave lookup table */
	static uint64_t concaveLUT[512] = { 0x3EA15515C0000000, 0x3F3727CC40000000, 0x3F47294020000000, 0x3F51623480000000, 0x3F5732B360000000, 0x3F5D061FA0000000, 0x3F616E3E00000000, 0x3F645AE5E0000000, 0x3F674908E0000000, 0x3F6A38A860000000, 0x3F6D29C620000000, 0x3F700E31C0000000, 0x3F71884100000000, 0x3F730311A0000000, 0x3F747EA460000000, 0x3F75FAFA20000000, 0x3F77781380000000, 0x3F78F5F160000000, 0x3F7A749480000000, 0x3F7BF3FDA0000000, 0x3F7D742DC0000000, 0x3F7EF52580000000, 0x3F803B72E0000000, 0x3F80FCB7C0000000, 0x3F81BE61A0000000, 0x3F828070E0000000, 0x3F8342E620000000, 0x3F8405C1C0000000, 0x3F84C90400000000, 0x3F858CAD80000000, 0x3F8650BE80000000, 0x3F871537A0000000, 0x3F87DA1920000000, 0x3F889F63A0000000, 0x3F89651760000000, 0x3F8A2B34E0000000, 0x3F8AF1BC80000000, 0x3F8BB8AEE0000000, 0x3F8C800C60000000, 0x3F8D47D560000000, 0x3F8E100A60000000, 0x3F8ED8ABE0000000, 0x3F8FA1BA60000000, 0x3F90359B20000000, 0x3F909A8FE0000000, 0x3F90FFBBE0000000, 0x3F91651F40000000, 0x3F91CABA40000000, 0x3F92308D20000000, 0x3F92969840000000, 0x3F92FCDB80000000, 0x3F93635780000000, 0x3F93CA0C60000000, 0x3F9430FA40000000, 0x3F94982180000000, 0x3F94FF8240000000, 0x3F95671D00000000, 0x3F95CEF1C0000000, 0x3F963700E0000000, 0x3F969F4AA0000000, 0x3F9707CF20000000, 0x3F97708EE0000000, 0x3F97D98A00000000, 0x3F9842C0C0000000, 0x3F98AC3380000000, 0x3F9915E260000000, 0x3F997FCDA0000000, 0x3F99E9F5A0000000, 0x3F9A545AC0000000, 0x3F9ABEFD00000000, 0x3F9B29DCE0000000, 0x3F9B94FA80000000, 0x3F9C005660000000, 0x3F9C6BF080000000, 0x3F9CD7C960000000, 0x3F9D43E120000000, 0x3F9DB03840000000, 0x3F9E1CCEC0000000, 0x3F9E89A520000000, 0x3F9EF6BBC0000000, 0x3F9F6412A0000000, 0x3F9FD1AA40000000, 0x3FA01FC180000000, 0x3FA056CE80000000, 0x3FA08DFC40000000, 0x3FA0C54B00000000, 0x3FA0FCBAE0000000, 0x3FA1344C20000000, 0x3FA16BFEA0000000, 0x3FA1A3D2E0000000, 0x3FA1DBC8C0000000, 0x3FA213E0A0000000, 0x3FA24C1A80000000, 0x3FA28476A0000000, 0x3FA2BCF520000000, 0x3FA2F59620000000, 0x3FA32E59E0000000, 0x3FA36740A0000000, 0x3FA3A04A60000000, 0x3FA3D97740000000, 0x3FA412C7A0000000, 0x3FA44C3B80000000, 0x3FA485D320000000, 0x3FA4BF8EC0000000, 0x3FA4F96E60000000, 0x3FA5337240000000, 0x3FA56D9AA0000000, 0x3FA5A7E7A0000000, 0x3FA5E25960000000, 0x3FA61CF040000000, 0x3FA657AC20000000, 0x3FA6928D60000000, 0x3FA6CD9440000000, 0x3FA708C0E0000000, 0x3FA7441340000000, 0x3FA77F8BE0000000, 0x3FA7BB2AC0000000, 0x3FA7F6F020000000, 0x3FA832DC20000000, 0x3FA86EEF20000000, 0x3FA8AB2920000000, 0x3FA8E78A60000000, 0x3FA9241340000000, 0x3FA960C3A0000000, 0x3FA99D9C00000000, 0x3FA9DA9C60000000, 0x3FAA17C520000000, 0x3FAA551660000000, 0x3FAA929040000000, 0x3FAAD03320000000, 0x3FAB0DFF40000000, 0x3FAB4BF480000000, 0x3FAB8A1380000000, 0x3FABC85C40000000, 0x3FAC06CF00000000, 0x3FAC456C00000000, 0x3FAC843360000000, 0x3FACC32580000000, 0x3FAD024280000000, 0x3FAD418AC0000000, 0x3FAD80FE40000000, 0x3FADC09D60000000, 0x3FAE006860000000, 0x3FAE405F60000000, 0x3FAE8082C0000000, 0x3FAEC0D2A0000000, 0x3FAF014F60000000, 0x3FAF41F920000000, 0x3FAF82D020000000, 0x3FAFC3D4C0000000, 0x3FB0028380000000, 0x3FB02333C0000000, 0x3FB043FB20000000, 0x3FB064D9C0000000, 0x3FB085CFC0000000, 0x3FB0A6DD40000000, 0x3FB0C80280000000, 0x3FB0E93F80000000, 0x3FB10A9480000000, 0x3FB12C0180000000, 0x3FB14D86C0000000, 0x3FB16F2460000000, 0x3FB190DA60000000, 0x3FB1B2A900000000, 0x3FB1D49080000000, 0x3FB1F690E0000000, 0x3FB218AA40000000, 0x3FB23ADCC0000000, 0x3FB25D28A0000000, 0x3FB27F8E20000000, 0x3FB2A20D20000000, 0x3FB2C4A5E0000000, 0x3FB2E758A0000000, 0x3FB30A2580000000, 0x3FB32D0C80000000, 0x3FB3500E00000000, 0x3FB3732A00000000, 0x3FB39660A0000000, 0x3FB3B9B240000000, 0x3FB3DD1EE0000000, 0x3FB400A6A0000000, 0x3FB42449C0000000, 0x3FB4480880000000, 0x3FB46BE2E0000000, 0x3FB48FD900000000, 0x3FB4B3EB40000000, 0x3FB4D819A0000000, 0x3FB4FC6480000000, 0x3FB520CBC0000000, 0x3FB5454FE0000000, 0x3FB569F0C0000000, 0x3FB58EAEC0000000, 0x3FB5B38A00000000, 0x3FB5D882C0000000, 0x3FB5FD9900000000, 0x3FB622CD20000000, 0x3FB6481F40000000, 0x3FB66D8F80000000, 0x3FB6931E40000000, 0x3FB6B8CB80000000, 0x3FB6DE9780000000, 0x3FB7048280000000, 0x3FB72A8CA0000000, 0x3FB750B600000000, 0x3FB776FF20000000, 0x3FB79D67E0000000, 0x3FB7C3F0C0000000, 0x3FB7EA99A0000000, 0x3FB8116300000000, 0x3FB8384D00000000, 0x3FB85F57C0000000, 0x3FB88683A0000000, 0x3FB8ADD0A0000000, 0x3FB8D53F40000000, 0x3FB8FCCF80000000, 0x3FB92481A0000000, 0x3FB94C5600000000, 0x3FB9744CC0000000, 0x3FB99C6600000000, 0x3FB9C4A240000000, 0x3FB9ED0180000000, 0x3FBA158420000000, 0x3FBA3E2A60000000, 0x3FBA66F460000000, 0x3FBA8FE260000000, 0x3FBAB8F4C0000000, 0x3FBAE22BC0000000, 0x3FBB0B8780000000, 0x3FBB350860000000, 0x3FBB5EAE80000000, 0x3FBB887A60000000, 0x3FBBB26C00000000, 0x3FBBDC83E0000000, 0x3FBC06C200000000, 0x3FBC312700000000, 0x3FBC5BB2E0000000, 0x3FBC866600000000, 0x3FBCB140A0000000, 0x3FBCDC4320000000, 0x3FBD076DA0000000, 0x3FBD32C0A0000000, 0x3FBD5E3C40000000, 0x3FBD89E0E0000000, 0x3FBDB5AEE0000000, 0x3FBDE1A680000000, 0x3FBE0DC7E0000000, 0x3FBE3A13A0000000, 0x3FBE6689E0000000, 0x3FBE932B00000000, 0x3FBEBFF760000000, 0x3FBEECEF20000000, 0x3FBF1A12C0000000, 0x3FBF4762A0000000, 0x3FBF74DF00000000, 0x3FBFA28840000000, 0x3FBFD05EC0000000, 0x3FBFFE62C0000000, 0x3FC0164A60000000, 0x3FC02D7A80000000, 0x3FC044C1E0000000, 0x3FC05C20E0000000, 0x3FC0739780000000, 0x3FC08B2600000000, 0x3FC0A2CCA0000000, 0x3FC0BA8B60000000, 0x3FC0D262C0000000, 0x3FC0EA52C0000000, 0x3FC1025B80000000, 0x3FC11A7D60000000, 0x3FC132B880000000, 0x3FC14B0D20000000, 0x3FC1637B60000000, 0x3FC17C03A0000000, 0x3FC194A5E0000000, 0x3FC1AD6280000000, 0x3FC1C639C0000000, 0x3FC1DF2BA0000000, 0x3FC1F838A0000000, 0x3FC21160C0000000, 0x3FC22AA460000000, 0x3FC24403C0000000, 0x3FC25D7F00000000, 0x3FC2771660000000, 0x3FC290CA40000000, 0x3FC2AA9AC0000000, 0x3FC2C48840000000, 0x3FC2DE92C0000000, 0x3FC2F8BAE0000000, 0x3FC3130080000000, 0x3FC32D6440000000, 0x3FC347E620000000, 0x3FC3628680000000, 0x3FC37D45A0000000, 0x3FC39823E0000000, 0x3FC3B32160000000, 0x3FC3CE3EA0000000, 0x3FC3E97BA0000000, 0x3FC404D8E0000000, 0x3FC42056C0000000, 0x3FC43BF540000000, 0x3FC457B500000000, 0x3FC4739620000000, 0x3FC48F98E0000000, 0x3FC4ABBDE0000000, 0x3FC4C80520000000, 0x3FC4E46F20000000, 0x3FC500FC40000000, 0x3FC51DACA0000000, 0x3FC53A80E0000000, 0x3FC5577920000000, 0x3FC57495E0000000, 0x3FC591D760000000, 0x3FC5AF3E00000000, 0x3FC5CCCA40000000, 0x3FC5EA7C60000000, 0x3FC60854C0000000, 0x3FC62653C0000000, 0x3FC6447A00000000, 0x3FC662C7A0000000, 0x3FC6813D20000000, 0x3FC69FDAE0000000, 0x3FC6BEA160000000, 0x3FC6DD9100000000, 0x3FC6FCAA40000000, 0x3FC71BED60000000, 0x3FC73B5B20000000, 0x3FC75AF3A0000000, 0x3FC77AB780000000, 0x3FC79AA740000000, 0x3FC7BAC360000000, 0x3FC7DB0C40000000, 0x3FC7FB8260000000, 0x3FC81C2640000000, 0x3FC83CF860000000, 0x3FC85DF960000000, 0x3FC87F29C0000000, 0x3FC8A089E0000000, 0x3FC8C21A60000000, 0x3FC8E3DBE0000000, 0x3FC905CF00000000, 0x3FC927F420000000, 0x3FC94A4BE0000000, 0x3FC96CD6E0000000, 0x3FC98F95C0000000, 0x3FC9B28920000000, 0x3FC9D5B180000000, 0x3FC9F90FC0000000, 0x3FCA1CA440000000, 0x3FCA406FC0000000, 0x3FCA647300000000, 0x3FCA88AEA0000000, 0x3FCAAD2360000000, 0x3FCAD1D1E0000000, 0x3FCAF6BAE0000000, 0x3FCB1BDF00000000, 0x3FCB413F20000000, 0x3FCB66DC20000000, 0x3FCB8CB680000000, 0x3FCBB2CF40000000, 0x3FCBD92720000000, 0x3FCBFFBEE0000000, 0x3FCC269760000000, 0x3FCC4DB180000000, 0x3FCC750E40000000, 0x3FCC9CAE40000000, 0x3FCCC492A0000000, 0x3FCCECBC40000000, 0x3FCD152BE0000000, 0x3FCD3DE2C0000000, 0x3FCD66E1C0000000, 0x3FCD9029E0000000, 0x3FCDB9BC20000000, 0x3FCDE39980000000, 0x3FCE0DC340000000, 0x3FCE383A40000000, 0x3FCE62FFE0000000, 0x3FCE8E1500000000, 0x3FCEB97AE0000000, 0x3FCEE532C0000000, 0x3FCF113DE0000000, 0x3FCF3D9D60000000, 0x3FCF6A52A0000000, 0x3FCF975EE0000000, 0x3FCFC4C380000000, 0x3FCFF281C0000000, 0x3FD0104DA0000000, 0x3FD0278880000000, 0x3FD03EF280000000, 0x3FD0568C40000000, 0x3FD06E5680000000, 0x3FD0865200000000, 0x3FD09E7FA0000000, 0x3FD0B6E040000000, 0x3FD0CF74A0000000, 0x3FD0E83DA0000000, 0x3FD1013C20000000, 0x3FD11A7100000000, 0x3FD133DD60000000, 0x3FD14D8200000000, 0x3FD1676000000000, 0x3FD1817860000000, 0x3FD19BCC00000000, 0x3FD1B65C20000000, 0x3FD1D129C0000000, 0x3FD1EC3600000000, 0x3FD2078200000000, 0x3FD2230F20000000, 0x3FD23EDE60000000, 0x3FD25AF100000000, 0x3FD2774880000000, 0x3FD293E600000000, 0x3FD2B0CB00000000, 0x3FD2CDF8E0000000, 0x3FD2EB7100000000, 0x3FD3093500000000, 0x3FD3274640000000, 0x3FD345A680000000, 0x3FD3645740000000, 0x3FD3835A40000000, 0x3FD3A2B120000000, 0x3FD3C25DE0000000, 0x3FD3E26240000000, 0x3FD402C040000000, 0x3FD42379A0000000, 0x3FD44490C0000000, 0x3FD4660760000000, 0x3FD487E000000000, 0x3FD4AA1CA0000000, 0x3FD4CCBFE0000000, 0x3FD4EFCBE0000000, 0x3FD5134360000000, 0x3FD53728E0000000, 0x3FD55B7F20000000, 0x3FD58048E0000000, 0x3FD5A58900000000, 0x3FD5CB4280000000, 0x3FD5F178A0000000, 0x3FD6182E80000000, 0x3FD63F67A0000000, 0x3FD6672780000000, 0x3FD68F71C0000000, 0x3FD6B84A40000000, 0x3FD6E1B4E0000000, 0x3FD70BB5C0000000, 0x3FD7365160000000, 0x3FD7618C20000000, 0x3FD78D6AC0000000, 0x3FD7B9F240000000, 0x3FD7E727A0000000, 0x3FD8151060000000, 0x3FD843B200000000, 0x3FD87312A0000000, 0x3FD8A33860000000, 0x3FD8D429C0000000, 0x3FD905EDA0000000, 0x3FD9388B20000000, 0x3FD96C09E0000000, 0x3FD9A071C0000000, 0x3FD9D5CB40000000, 0x3FDA0C1F20000000, 0x3FDA4376E0000000, 0x3FDA7BDC60000000, 0x3FDAB55A00000000, 0x3FDAEFFAE0000000, 0x3FDB2BCAC0000000, 0x3FDB68D640000000, 0x3FDBA72A60000000, 0x3FDBE6D580000000, 0x3FDC27E6A0000000, 0x3FDC6A6DC0000000, 0x3FDCAE7C40000000, 0x3FDCF42440000000, 0x3FDD3B79A0000000, 0x3FDD8491A0000000, 0x3FDDCF8300000000, 0x3FDE1C6640000000, 0x3FDE6B5600000000, 0x3FDEBC6F00000000, 0x3FDF0FD040000000, 0x3FDF659BA0000000, 0x3FDFBDF620000000, 0x3FE00C8400000000, 0x3FE03B7EA0000000, 0x3FE06C0320000000, 0x3FE09E2C60000000, 0x3FE0D217A0000000, 0x3FE107E5C0000000, 0x3FE13FBB20000000, 0x3FE179C060000000, 0x3FE1B62340000000, 0x3FE1F51720000000, 0x3FE236D660000000, 0x3FE27BA3A0000000, 0x3FE2C3CAE0000000, 0x3FE30FA420000000, 0x3FE35F9560000000, 0x3FE3B416A0000000, 0x3FE40DB4E0000000, 0x3FE46D1900000000, 0x3FE4D30EA0000000, 0x3FE5408E80000000, 0x3FE5B6CD00000000, 0x3FE6374F20000000, 0x3FE6C408A0000000, 0x3FE75F8B60000000, 0x3FE80D50E0000000, 0x3FE8D23740000000, 0x3FE9B55BE0000000, 0x3FEAC1C0A0000000, 0x3FEC09C720000000, 0x3FEDAFBBC0000000, 0x3FF0000000000000 };

	/**
	\ingroup Constants-Enums
	Built-in reverse-concave lookup table */
	static uint64_t reverseconcaveLUT[512] = { 0x0, 0x3F861DE634288F06, 0x3F95FF72EB93B40D, 0x3FA068E9A6C42F3E, 0x3FA5C333990D3199, 0x3FAB0EC05385D1FD, 0x3FB025DC360B2CDB, 0x3FB2BD220472BF0F, 0x3FB54D4570C60DD3, 0x3FB7D65A2022D0D8, 0x3FBA5873818E9C2B, 0x3FBCD3A4CE8BD413, 0x3FBF48010BAD074C, 0x3FC0DACD84935995, 0x3FC20E42B1AFBB3E, 0x3FC33E6941BF5B9E, 0x3FC46B4A4FFCDFE7, 0x3FC594EEDE8FAB8B, 0x3FC6BB5FD6D0EC70, 0x3FC7DEA6098FE8AC, 0x3FC8FECA2F55906E, 0x3FCA1BD4E8A755B8, 0x3FCB35CEBE494C11, 0x3FCC4CC0217F921A, 0x3FCD60B16C4F071C, 0x3FCE71AAE1BD4E53, 0x3FCF7FB4AE1021F1, 0x3FD0456B7385FBEE, 0x3FD0C98CC618FCE6, 0x3FD14C42437EA975, 0x3FD1CD8FD58FEA41, 0x3FD24D795B5EF9E0, 0x3FD2CC02A9551128, 0x3FD3492F894FC1B9, 0x3FD3C503BABDFFC6, 0x3FD43F82F2BCDBD6, 0x3FD4B8B0DC33ED7A, 0x3FD5309117F16FB9, 0x3FD5A7273CC6101A, 0x3FD61C76D7A07120, 0x3FD690836BA860F3, 0x3FD703507259C530, 0x3FD774E15B9F3C90, 0x3FD7E5398DEC772A, 0x3FD8545C6658464B, 0x3FD8C24D38B66465, 0x3FD92F0F4FB0F621, 0x3FD99AA5ECE1C522, 0x3FDA051448EB3555, 0x3FDA6E5D9390F58A, 0x3FDAD684F3D06C10, 0x3FDB3D8D87F8E00E, 0x3FDBA37A65C36052, 0x3FDC084E9A6A6854, 0x3FDC6C0D2AC1441C, 0x3FDCCEB9134B33BD, 0x3FDD305548524F20, 0x3FDD90E4B5FE2ABF, 0x3FDDF06A406A3E08, 0x3FDE4EE8C3BC0C1A, 0x3FDEAC6314390F75, 0x3FDF08DBFE5C6949, 0x3FDF645646EC5524, 0x3FDFBED4AB0F6177, 0x3FE00C2CF030B6DA, 0x3FE038744A843752, 0x3FE06441B7E47CC4, 0x3FE08F96880D4B54, 0x3FE0BA74071DEDF8, 0x3FE0E4DB7DA32817, 0x3FE10ECE30A10BC7, 0x3FE1384D619CB4F4, 0x3FE1615A4EA5E9C0, 0x3FE189F63260A056, 0x3FE1B222440E6A95, 0x3FE1D9DFB797C7B8, 0x3FE2012FBD955C61, 0x3FE2281383591142, 0x3FE24E8C32F7189D, 0x3FE2749AF34EDB00, 0x3FE29A40E813CB56, 0x3FE2BF7F31D622BD, 0x3FE2E456EE0B8449, 0x3FE308C9371788F9, 0x3FE32CD724543423, 0x3FE35081CA1A509F, 0x3FE373CA39C9B6E6, 0x3FE396B181D17C63, 0x3FE3B938ADB80C49, 0x3FE3DB60C6232A0A, 0x3FE3FD2AD0DFDDD5, 0x3FE41E97D0EA4B29, 0x3FE43FA8C67571F0, 0x3FE4605EAEF2DA27, 0x3FE480BA851A2A74, 0x3FE4A0BD40F0A9D3, 0x3FE4C067D7D0AC9A, 0x3FE4DFBB3C70ED06, 0x3FE4FEB85EEBCF91, 0x3FE51D602CC69343, 0x3FE53BB390F86E3F, 0x3FE559B373F196AE, 0x3FE57760BBA23855, 0x3FE594BC4B8156FF, 0x3FE5B1C704939DF3, 0x3FE5CE81C5721CA2, 0x3FE5EAED6A50F0D2, 0x3FE6070ACD05DE68, 0x3FE622DAC50ED512, 0x3FE63E5E279863EC, 0x3FE65995C7841B79, 0x3FE67482756EDDEF, 0x3FE68F24FFB71E33, 0x3FE6A97E32830D99, 0x3FE6C38ED7C6B8A6, 0x3FE6DD57B74A12F7, 0x3FE6F6D996AEF284, 0x3FE710153976FA73, 0x3FE7290B6109758E, 0x3FE741BCCCB920AD, 0x3FE75A2A39C9E525, 0x3FE772546376836A, 0x3FE78A3C02F62E2F, 0x3FE7A1E1CF8215F7, 0x3FE7B9467E5AE576, 0x3FE7D06AC2CE2ED3, 0x3FE7E74F4E3BC9F2, 0x3FE7FDF4D01B23FF, 0x3FE8145BF6008052, 0x3FE82A856BA22ADD, 0x3FE84071DADD9C52, 0x3FE85621EBBC9014, 0x3FE86B96447A0C32, 0x3FE880CF89875B81, 0x3FE895CE5D90FA07, 0x3FE8AA93618373CF, 0x3FE8BF1F3490365F, 0x3FE8D372743254E0, 0x3FE8E78DBC333F2C, 0x3FE8FB71A6AF6BD6, 0x3FE90F1ECC1AF572, 0x3FE92295C3462B08, 0x3FE935D721621410, 0x3FE948E37A04E7EF, 0x3FE95BBB5F2E7932, 0x3FE96E5F614C9495, 0x3FE980D00F3F540A, 0x3FE9930DF65D65CB, 0x3FE9A519A27847B2, 0x3FE9B6F39DE076E2, 0x3FE9C89C716993EB, 0x3FE9DA14A46E7B8A, 0x3FE9EB5CBCD55416, 0x3FE9FC753F138FD7, 0x3FEA0D5EAE31E440, 0x3FEA1E198BD03649, 0x3FEA2EA658297BFA, 0x3FEA3F0592179342, 0x3FEA4F37B7170E3E, 0x3FEA5F3D434AF50A, 0x3FEA6F16B1807D3E, 0x3FEA7EC47B32B727, 0x3FEA8E47188E30EB, 0x3FEA9D9F00748F98, 0x3FEAACCCA8801E5F, 0x3FEABBD0850753F7, 0x3FEACAAB09204E48, 0x3FEAD95CA6A44491, 0x3FEAE7E5CE32EFFB, 0x3FEAF646EF35EADA, 0x3FEB048077E4069F, 0x3FEB1292D544989A, 0x3FEB207E7332BDA6, 0x3FEB2E43BC6094D4, 0x3FEB3BE31A5A7144, 0x3FEB495CF58A0312, 0x3FEB56B1B53977AE, 0x3FEB63E1BF969188, 0x3FEB70ED79B5B73C, 0x3FEB7DD54794FA51, 0x3FEB8A998C1F15A4, 0x3FEB973AA92E6392, 0x3FEBA3B8FF8FCC00, 0x3FEBB014EF05AA45, 0x3FEBBC4ED64AAB2C, 0x3FEBC8671314A300, 0x3FEBD45E02175BD2, 0x3FEBE033FF075C00, 0x3FEBEBE9649CA51D, 0x3FEBF77E8C956B45, 0x3FEC02F3CFB8C4F6, 0x3FEC0E4985D95389, 0x3FEC198005D7E453, 0x3FEC2497A5A60A87, 0x3FEC2F90BA48B1F7, 0x3FEC3A6B97DAAAB9, 0x3FEC4528918F2DC7, 0x3FEC4FC7F9B45ABA, 0x3FEC5A4A21B5AE9A, 0x3FEC64AF5A1E73F7, 0x3FEC6EF7F29C2C39, 0x3FEC79243A00F25C, 0x3FEC83347E45D711, 0x3FEC8D290C8D3661, 0x3FEC9702312506E4, 0x3FECA0C03789229D, 0x3FECAA636A658987, 0x3FECB3EC13989DED, 0x3FECBD5A7C355A9B, 0x3FECC6AEEC8582F0, 0x3FECCFE9AC0BCCF9, 0x3FECD90B01860580, 0x3FECE21332EF2E4E, 0x3FECEB0285819688, 0x3FECF3D93DB8ED4A, 0x3FECFC979F544E8E, 0x3FED053DED584A75, 0x3FED0DCC6A10E6E9, 0x3FED164357139BCD, 0x3FED1EA2F54149A3, 0x3FED26EB84C82AD5, 0x3FED2F1D4525BFA7, 0x3FED37387528B4D2, 0x3FED3F3D52F2C4F1, 0x3FED472C1BFA94B3, 0x3FED4F050D0D89FD, 0x3FED56C862519DEB, 0x3FED5E76574729D8, 0x3FED660F26CAAF6F, 0x3FED6D930B169BD3, 0x3FED75023DC505E8, 0x3FED7C5CF7D167D9, 0x3FED83A3719A53D5, 0x3FED8AD5E2E32421, 0x3FED91F482D5A67F, 0x3FED98FF8803C2FA, 0x3FED9FF728691E2F, 0x3FEDA6DB996CB70D, 0x3FEDADAD0FE28029, 0x3FEDB46BC00CF4A4, 0x3FEDBB17DD9EA8B9, 0x3FEDC1B19BBBD5FC, 0x3FEDC8392CFBE35C, 0x3FEDCEAEC36AE8E2, 0x3FEDD512908B2F48, 0x3FEDDB64C556AB7D, 0x3FEDE1A5924075FA, 0x3FEDE7D527363E28, 0x3FEDEDF3B3A1B9B7, 0x3FEDF401666A1003, 0x3FEDF9FE6DF5419C, 0x3FEDFFEAF8298BDF, 0x3FEE05C7326EC8C8, 0x3FEE0B9349AFCAFA, 0x3FEE114F6A5BB601, 0x3FEE16FBC06752E8, 0x3FEE1C98774E6123, 0x3FEE2225BA14E3D8, 0x3FEE27A3B3486B93, 0x3FEE2D128D015C6D, 0x3FEE327270E430B3, 0x3FEE37C38822B817, 0x3FEE3D05FB7D5366, 0x3FEE4239F3442CEA, 0x3FEE475F97586D64, 0x3FEE4C770F2D6DB4, 0x3FEE518081C9E53D, 0x3FEE567C15C91503, 0x3FEE5B69F15BEF94, 0x3FEE604A3A4A3DC3, 0x3FEE651D15F3C039, 0x3FEE69E2A9514DEC, 0x3FEE6E9B18F5EF81, 0x3FEE7346890FF7A3, 0x3FEE77E51D6A184B, 0x3FEE7C76F96C7526, 0x3FEE80FC401DB2E4, 0x3FEE8575142403BE, 0x3FEE89E197C63100, 0x3FEE8E41ECECA1C9, 0x3FEE929635225EE8, 0x3FEE96DE919613FA, 0x3FEE9B1B231B0DBF, 0x3FEE9F4C0A2A35B3, 0x3FEEA37166E30AEF, 0x3FEEA78B590C9862, 0x3FEEAB9A00166865, 0x3FEEAF9D7B1975A7, 0x3FEEB395E8D9198C, 0x3FEEB78367C3F7F6, 0x3FEEBB6615F4E887, 0x3FEEBF3E1133DD5B, 0x3FEEC30B76F6C754, 0x3FEEC6CE646277E1, 0x3FEECA86F64B8065, 0x3FEECE3549370F2C, 0x3FEED1D9795BCA06, 0x3FEED573A2A2A68A, 0x3FEED903E0A7BFFD, 0x3FEEDC8A4EBB2AF4, 0x3FEEE00707E1C6AE, 0x3FEEE37A26D60C25, 0x3FEEE6E3C608DAF4, 0x3FEEEA43FFA243FC, 0x3FEEED9AED8251DD, 0x3FEEF0E8A941CF53, 0x3FEEF42D4C330B5C, 0x3FEEF768EF629B53, 0x3FEEFA9BAB981AED, 0x3FEEFDC59956EA2A, 0x3FEF00E6D0DEE934, 0x3FEF03FF6A2D3240, 0x3FEF070F7CFCD164, 0x3FEF0A1720C77A88, 0x3FEF0D166CC63D4B, 0x3FEF100D77F23702, 0x3FEF12FC590542D3, 0x3FEF15E3267AA7DE, 0x3FEF18C1F68FC596, 0x3FEF1B98DF44BE2E, 0x3FEF1E67F65D1F41, 0x3FEF212F5160889F, 0x3FEF23EF059B5153, 0x3FEF26A7281F2ADF, 0x3FEF2957CDC3C2BE, 0x3FEF2C010B276222, 0x3FEF2EA2F4AF8BFB, 0x3FEF313D9E89994E, 0x3FEF33D11CAB53DD, 0x3FEF365D82D38F23, 0x3FEF38E2E48ABFA9, 0x3FEF3B61552390C3, 0x3FEF3DD8E7BB78A7, 0x3FEF4049AF3B4AEF, 0x3FEF42B3BE57C98D, 0x3FEF451727923426, 0x3FEF4773FD38D5EC, 0x3FEF49CA516791EB, 0x3FEF4C1A36086DD6, 0x3FEF4E63BCD41B57, 0x3FEF50A6F7527FE0, 0x3FEF52E3F6DB3B10, 0x3FEF551ACC962B97, 0x3FEF574B897BF2BB, 0x3FEF59763E567668, 0x3FEF5B9AFBC161E4, 0x3FEF5DB9D22AA514, 0x3FEF5FD2D1D2F26E, 0x3FEF61E60ACE3B87, 0x3FEF63F38D042C57, 0x3FEF65FB6830A517, 0x3FEF67FDABE432E0, 0x3FEF69FA678486F7, 0x3FEF6BF1AA4CECCE, 0x3FEF6DE3834EBEC4, 0x3FEF6FD00171D9A6, 0x3FEF71B733750EEB, 0x3FEF739927EE95B8, 0x3FEF7575ED4C7AB1, 0x3FEF774D91D50E91, 0x3FEF792023A75393, 0x3FEF7AEDB0BB69AF, 0x3FEF7CB646E2F9AA, 0x3FEF7E79F3C99F08, 0x3FEF8038C4F550C8, 0x3FEF81F2C7C6C914, 0x3FEF83A80979EBC8, 0x3FEF855897262BE0, 0x3FEF87047DBEEFD3, 0x3FEF88ABCA13F4D1, 0x3FEF8A4E88D1B0FD, 0x3FEF8BECC681B48A, 0x3FEF8D868F8B09D7, 0x3FEF8F1BF032947E, 0x3FEF90ACF49B6F5D, 0x3FEF9239A8C7499F, 0x3FEF93C21896C2C2, 0x3FEF95464FC9C5A0, 0x3FEF96C659FFE27F, 0x3FEF984242B8A826, 0x3FEF99BA1553FC00, 0x3FEF9B2DDD12714C, 0x3FEF9C9DA5159F59, 0x3FEF9E09786076D9, 0x3FEF9F7161D79648, 0x3FEFA0D56C419D67, 0x3FEFA235A2477FD8, 0x3FEFA3920E74D6D0, 0x3FEFA4EABB3831EC, 0x3FEFA63FB2E36724, 0x3FEFA790FFABE1E9, 0x3FEFA8DEABAAF15C, 0x3FEFAA28C0DE15BD, 0x3FEFAB6F49274CFA, 0x3FEFACB24E4D5E71, 0x3FEFADF1D9FC25DC, 0x3FEFAF2DF5C4DD73, 0x3FEFB066AB1E6742, 0x3FEFB19C036595B2, 0x3FEFB2CE07DD734E, 0x3FEFB3FCC1AF89BD, 0x3FEFB52839EC2800, 0x3FEFB650798AA7E6, 0x3FEFB7758969B2CA, 0x3FEFB897724F858B, 0x3FEFB9B63CEA33CF, 0x3FEFBAD1F1CFEA89, 0x3FEFBBEA997F31CE, 0x3FEFBD003C5F2DE8, 0x3FEFBE12E2BFDFC4, 0x3FEFBF2294DA64A8, 0x3FEFC02F5AD13538, 0x3FEFC1393CB063D2, 0x3FEFC240426DDA3A, 0x3FEFC34473E9969C, 0x3FEFC445D8EDE7EF, 0x3FEFC544792FA9A4, 0x3FEFC6405C4E7EB8, 0x3FEFC73989D50C2A, 0x3FEFC830093932BD, 0x3FEFC923E1DC4832, 0x3FEFCA151B0B4FD1, 0x3FEFCB03BBFF3264, 0x3FEFCBEFCBDCF58D, 0x3FEFCCD951B5F293, 0x3FEFCDC054880C7D, 0x3FEFCEA4DB3DE5BD, 0x3FEFCF86ECAF1523, 0x3FEFD0668FA05A58, 0x3FEFD143CAC3D1BB, 0x3FEFD21EA4B927B4, 0x3FEFD2F7240DCB7B, 0x3FEFD3CD4F3D214E, 0x3FEFD4A12CB0B420, 0x3FEFD572C2C066C1, 0x3FEFD64217B2A47C, 0x3FEFD70F31BC912D, 0x3FEFD7DA170238DA, 0x3FEFD8A2CD96BEC1, 0x3FEFD9695B7C8BEA, 0x3FEFDA2DC6A57D32, 0x3FEFDAF014F310E1, 0x3FEFDBB04C3693BB, 0x3FEFDC6E72314D94, 0x3FEFDD2A8C94AD74, 0x3FEFDDE4A1027533, 0x3FEFDE9CB50CE4A3, 0x3FEFDF52CE36E44A, 0x3FEFE006F1F42F9D, 0x3FEFE0B925A97EC6, 0x3FEFE1696EACB002, 0x3FEFE217D244F082, 0x3FEFE2C455AAE4DD, 0x3FEFE36EFE08D11D, 0x3FEFE417D07AC049, 0x3FEFE4BED20EABA0, 0x3FEFE56407C4A144, 0x3FEFE607768EEA9E, 0x3FEFE6A92352323C, 0x3FEFE74912E5A959, 0x3FEFE7E74A132CF2, 0x3FEFE883CD976A81, 0x3FEFE91EA2220442, 0x3FEFE9B7CC55B529, 0x3FEFEA4F50C8745F, 0x3FEFEAE534039872, 0x3FEFEB797A83FA12, 0x3FEFEC0C28BA167E, 0x3FEFEC9D430A3183, 0x3FEFED2CCDCC772D, 0x3FEFEDBACD4D1D0F, 0x3FEFEE4745CC8333, 0x3FEFEED23B7F54B2, 0x3FEFEF5BB28EA7F3, 0x3FEFEFE3AF181E89, 0x3FEFF06A352E04C1, 0x3FEFF0EF48D770D9, 0x3FEFF172EE1061DE, 0x3FEFF1F528C9DE33, 0x3FEFF275FCEA11CA, 0x3FEFF2F56E4C6C09, 0x3FEFF37380C1BD52, 0x3FEFF3F03810544C, 0x3FEFF46B97F41ACB, 0x3FEFF4E5A41EB271, 0x3FEFF55E603790FC, 0x3FEFF5D5CFDC1C4D, 0x3FEFF64BF69FC618, 0x3FEFF6C0D80C274C, 0x3FEFF73477A11B37, 0x3FEFF7A6D8D4DA4F, 0x3FEFF817FF1414C3, 0x3FEFF887EDC20CB7, 0x3FEFF8F6A838B039, 0x3FEFF96431C8B2F4, 0x3FEFF9D08DB9A79B, 0x3FEFFA3BBF4A1906, 0x3FEFFAA5C9AFA312, 0x3FEFFB0EB0170B41, 0x3FEFFB7675A45905, 0x3FEFFBDD1D72EDDB, 0x3FEFFC42AA959D19, 0x3FEFFCA72016C377, 0x3FEFFD0A80F85E67, 0x3FEFFD6CD034231D, 0x3FEFFDCE10BB955B, 0x3FEFFE2E45781E09, 0x3FEFFE8D714B217B, 0x3FEFFEEB970E158D, 0x3FEFFF48B992977A, 0x3FEFFFA4DBA28173, 0x3FF0000000000000 };

	/**
	\ingroup Constants-Enums
	Built-in convex lookup table */
	static uint64_t convexLUT[512] = { 0x0, 0x3FB2822A7CC37D8D, 0x3FBFB1CF9E824103, 0x3FC4F901E8A698B8, 0x3FC92A94BE9D4F66, 0x3FCCB727636A0498, 0x3FCFCAC0B9947039, 0x3FD140EB812E6589, 0x3FD277F0D31D94D6, 0x3FD39163E675CA3C, 0x3FD4926819A1D19A, 0x3FD57EE53381135E, 0x3FD659E4E9B75C07, 0x3FD725D022782DEF, 0x3FD7E498722A7473, 0x3FD897D50605CD27, 0x3FD940D74BBC9DE8, 0x3FD9E0BA0444DA3E, 0x3FDA786C796F67D5, 0x3FDB08BAF7D0C66C, 0x3FDB92555037B921, 0x3FDC15D3E730545D, 0x3FDC93BBB1024244, 0x3FDD0C815D9E9E61, 0x3FDD808BE56E26DA, 0x3FDDF0369B03AAAD, 0x3FDE5BD2DC8C453D, 0x3FDEC3A9793F2ACF, 0x3FDF27FBDA409FAB, 0x3FDF8904FAE02449, 0x3FDFE6FA3974C27D, 0x3FE02106040DF61E, 0x3FE04D33418D5FEB, 0x3FE07818F8BB8078, 0x3FE0A1C999BAE7F5, 0x3FE0CA5610FCC896, 0x3FE0F1CDF09530B5, 0x3FE1183F9431F140, 0x3FE13DB84084B2F9, 0x3FE162443ECCB6F1, 0x3FE185EEF50FA96D, 0x3FE1A8C2FB795264, 0x3FE1CACA2F46A9D3, 0x3FE1EC0DC391079E, 0x3FE20C9650411F98, 0x3FE22C6BDF66B091, 0x3FE24B95F928D4AB, 0x3FE26A1BAE7B632A, 0x3FE28803A2BF8BF2, 0x3FE2A55414708B4C, 0x3FE2C212E4F8DD55, 0x3FE2DE459FC88F6C, 0x3FE2F9F180C21575, 0x3FE3151B7A1049B7, 0x3FE32FC83975E3FB, 0x3FE349FC2D24AFC2, 0x3FE363BB88290CBA, 0x3FE37D0A4674C43F, 0x3FE395EC3092F275, 0x3FE3AE64DF0DA0F0, 0x3FE3C677BD8CB5E1, 0x3FE3DE280DB4FFE3, 0x3FE3F578E9CD6736, 0x3FE40C6D472FA596, 0x3FE42307F88952C1, 0x3FE4394BAFF1924D, 0x3FE44F3B00D73D5A, 0x3FE464D861CAFDE5, 0x3FE47A262E287846, 0x3FE48F26A7A1505F, 0x3FE4A3DBF7AC91D0, 0x3FE4B84830DCC3E9, 0x3FE4CC6D501EBAA1, 0x3FE4E04D3DE303CB, 0x3FE4F3E9CF33A3CB, 0x3FE50744C6B7ACCA, 0x3FE51A5FD5A617FE, 0x3FE52D3C9CA92899, 0x3FE53FDCACB382DB, 0x3FE5524187C80817, 0x3FE5646CA1B570E7, 0x3FE5765F60C689A9, 0x3FE5881B1E67E25C, 0x3FE599A127C3B16B, 0x3FE5AAF2BE5499A2, 0x3FE5BC11186FF4D7, 0x3FE5CCFD61C83849, 0x3FE5DDB8BBE7FBAC, 0x3FE5EE443EA6214D, 0x3FE5FEA0F89393D0, 0x3FE60ECFEF630546, 0x3FE61ED2204B130A, 0x3FE62EA880632A8D, 0x3FE63E53FCFB8442, 0x3FE64DD57BF083AC, 0x3FE65D2DDBF9C5DA, 0x3FE66C5DF4F52258, 0x3FE67B66982DDDBF, 0x3FE68A4890A04893, 0x3FE69904A33A0139, 0x3FE6A79B8F170BC3, 0x3FE6B60E0DBBEF0F, 0x3FE6C45CD34D0363, 0x3FE6D2888EC31BCB, 0x3FE6E091EA1DB0B9, 0x3FE6EE798A92AFD4, 0x3FE6FC4010BC12B7, 0x3FE709E618C35BEF, 0x3FE7176C3A8B17D1, 0x3FE724D309D67CA2, 0x3FE7321B166F4401, 0x3FE73F44EC49D5CA, 0x3FE74C5113A7DB29, 0x3FE7594011394F55, 0x3FE76612663C21E4, 0x3FE772C8909A7DAC, 0x3FE77F630B07C5E4, 0x3FE78BE24D1C5A39, 0x3FE79846CB703193, 0x3FE7A490F7B45A44, 0x3FE7B0C140CB6DAF, 0x3FE7BCD812E1047A, 0x3FE7C8D5D78037B9, 0x3FE7D4BAF5A93ACD, 0x3FE7E087D1E618F8, 0x3FE7EC3CCE5EA129, 0x3FE7F7DA4AEB89D1, 0x3FE80360A528D61E, 0x3FE80ED03887856D, 0x3FE81A295E5E954F, 0x3FE8256C6DFB5E10, 0x3FE83099BCB1512C, 0x3FE83BB19DE920E6, 0x3FE846B4632F57A8, 0x3FE851A25C426593, 0x3FE85C7BD7202A56, 0x3FE86741201300FD, 0x3FE871F281BE533A, 0x3FE87C90452AB94D, 0x3FE8871AB1D1AB82, 0x3FE891920DA8C9E7, 0x3FE89BF69D2CBEBD, 0x3FE8A648A36BBFD3, 0x3FE8B088620FB2D4, 0x3FE8BAB61967F869, 0x3FE8C4D20872E1CB, 0x3FE8CEDC6CE6D441, 0x3FE8D8D5833B1DDE, 0x3FE8E2BD86B07EAB, 0x3FE8EC94B1596937, 0x3FE8F65B3C21FD6F, 0x3FE900115ED7C089, 0x3FE909B75031148F, 0x3FE9134D45D47211, 0x3FE91CD3745F6661, 0x3FE9264A0F6D589D, 0x3FE92FB1499E17BF, 0x3FE93909549C33B3, 0x3FE9425261232393, 0x3FE94B8C9F053ABF, 0x3FE954B83D316EDB, 0x3FE95DD569B8F03E, 0x3FE966E451D496AE, 0x3FE96FE521EA23CB, 0x3FE978D805915CE9, 0x3FE981BD2798FDA4, 0x3FE98A94B20B84AE, 0x3FE9935ECE33DC26, 0x3FE99C1BA4A1DEC6, 0x3FE9A4CB5D2EBB26, 0x3FE9AD6E1F013633, 0x3FE9B6041091CE22, 0x3FE9BE8D57AEBECE, 0x3FE9C70A197FE8A8, 0x3FE9CF7A7A8A9B35, 0x3FE9D7DE9EB54400, 0x3FE9E036A94B0317, 0x3FE9E882BCFF25C3, 0x3FE9F0C2FBF08884, 0x3FE9F8F787ACE106, 0x3FEA01208133F0EE, 0x3FEA093E08FAA230, 0x3FEA11503EEE0DBA, 0x3FEA195742766D20, 0x3FEA21533279F7F6, 0x3FEA29442D5FAD83, 0x3FEA312A51120B6F, 0x3FEA3905BB01B203, 0x3FEA40D68827F68B, 0x3FEA489CD5096483, 0x3FEA5058BDB82DED, 0x3FEA580A5DD68B8D, 0x3FEA5FB1D0990D57, 0x3FEA674F30C8DBAF, 0x3FEA6EE298C5E9DF, 0x3FEA766C22891A4A, 0x3FEA7DEBE7A654AE, 0x3FEA8562014E8EFD, 0x3FEA8CCE8851C927, 0x3FEA94319520FC38, 0x3FEA9B8B3FCFFD30, 0x3FEAA2DBA01753EA, 0x3FEAAA22CD56067A, 0x3FEAB160DE93594B, 0x3FEAB895EA808457, 0x3FEABFC2077A5DC9, 0x3FEAC6E54B8AFA53, 0x3FEACDFFCC6B439C, 0x3FEAD5119F8484D9, 0x3FEADC1AD9F1EE20, 0x3FEAE31B90820E78, 0x3FEAEA13D7B8450F, 0x3FEAF103C3CE29CB, 0x3FEAF7EB68B4ED73, 0x3FEAFECADA16B1A9, 0x3FEB05A22B57D8F7, 0x3FEB0C716F984F26, 0x3FEB1338B9B4CA03, 0x3FEB19F81C4802E4, 0x3FEB20AFA9ABE912, 0x3FEB275F73FACD3D, 0x3FEB2E078D10863F, 0x3FEB34A8068B8F4E, 0x3FEB3B40F1CE1FD1, 0x3FEB41D25FFF3D01, 0x3FEB485C620BC57C, 0x3FEB4EDF08A776FC, 0x3FEB555A644DEE51, 0x3FEB5BCE8543A1C8, 0x3FEB623B7B96D622, 0x3FEB68A157208E48, 0x3FEB6F00278575CD, 0x3FEB7557FC36C66F, 0x3FEB7BA8E47328BC, 0x3FEB81F2EF478FDF, 0x3FEB88362B9010DE, 0x3FEB8E72A7F8B53E, 0x3FEB94A872FE4945, 0x3FEB9AD79AEF25EE, 0x3FEBA1002DEBF6AA, 0x3FEBA72239E87B04, 0x3FEBAD3DCCAC444C, 0x3FEBB352F3D36F61, 0x3FEBB961BCCF5AA0, 0x3FEBBF6A34E75832, 0x3FEBC56C69395CAB, 0x3FEBCB6866BAAA2D, 0x3FEBD15E3A387815, 0x3FEBD74DF058974D, 0x3FEBDD37959A1353, 0x3FEBE31B3655D017, 0x3FEBE8F8DEBF24AC, 0x3FEBEED09AE472F0, 0x3FEBF4A276AFBC43, 0x3FEBFA6E7DE73345, 0x3FEC0034BC2DCAC8, 0x3FEC05F53D03C1F9, 0x3FEC0BB00BC72DD3, 0x3FEC116533B47FE8, 0x3FEC1714BFE70A9E, 0x3FEC1CBEBB5982D3, 0x3FEC226330E67F23, 0x3FEC28022B48F4A7, 0x3FEC2D9BB51CB170, 0x3FEC332FD8DED4A2, 0x3FEC38BEA0EE4461, 0x3FEC3E48178C2170, 0x3FEC43CC46DC38C5, 0x3FEC494B38E572F0, 0x3FEC4EC4F7924172, 0x3FEC54398CB10A1F, 0x3FEC59A901F49076, 0x3FEC5F1360F45D1C, 0x3FEC6478B32D2366, 0x3FEC69D902012519, 0x3FEC6F3456B89451, 0x3FEC748ABA81F3AA, 0x3FEC79DC367274B0, 0x3FEC7F28D386549B, 0x3FEC84709AA13767, 0x3FEC89B3948E814D, 0x3FEC8EF1CA01AEA1, 0x3FEC942B4396AA2E, 0x3FEC996009D2220A, 0x3FEC9E902521DAE3, 0x3FECA3BB9DDD01EA, 0x3FECA8E27C447D3F, 0x3FECAE04C8833B0A, 0x3FECB3228AAE7F25, 0x3FECB83BCAC62F82, 0x3FECBD5090B51F2D, 0x3FECC260E4515818, 0x3FECC76CCD5C639B, 0x3FECCC74538391C1, 0x3FECD1777E603F4D, 0x3FECD67655781AB3, 0x3FECDB70E03D67C2, 0x3FECE067260F4248, 0x3FECE5592E39DF8B, 0x3FECEA46FFF6CEB1, 0x3FECEF30A26D3814, 0x3FECF4161CB21B8D, 0x3FECF8F775C88DB0, 0x3FECFDD4B4A1F416, 0x3FED02ADE01E4099, 0x3FED0782FF0C2BA7, 0x3FED0C5418296D96, 0x3FED11213222F718, 0x3FED15EA539528B6, 0x3FED1AAF830C0973, 0x3FED1F70C7037C8B, 0x3FED242E25E7764F, 0x3FED28E7A6143032, 0x3FED2D9D4DD65C01, 0x3FED324F236B5645, 0x3FED36FD2D0157E1, 0x3FED3BA770B7A6E1, 0x3FED404DF49EC68C, 0x3FED44F0BEB8A6AE, 0x3FED498FD4F8D22E, 0x3FED4E2B3D449CDE, 0x3FED52C2FD7350A4, 0x3FED57571B4E59E4, 0x3FED5BE79C917342, 0x3FED607486EAD0B7, 0x3FED64FDDFFB49F8, 0x3FED6983AD56843D, 0x3FED6E05F4831B65, 0x3FED7284BAFACA77, 0x3FED7700062A9387, 0x3FED7B77DB72E703, 0x3FED7FEC4027CA67, 0x3FED845D3990FE5D, 0x3FED88CACCEA244D, 0x3FED8D34FF62E353, 0x3FED919BD61F0CBF, 0x3FED95FF5636BFF1, 0x3FED9A5F84B68DC0, 0x3FED9EBC669F9B4F, 0x3FEDA31600E7C46B, 0x3FEDA76C5879BD5D, 0x3FEDABBF72353446, 0x3FEDB00F52EEF1F8, 0x3FEDB45BFF70FA5A, 0x3FEDB8A57C7AAC51, 0x3FEDBCEBCEC0E139, 0x3FEDC12EFAEE0BDF, 0x3FEDC56F05A25712, 0x3FEDC9ABF373C3C4, 0x3FEDCDE5C8EE46B9, 0x3FEDD21C8A93E5CA, 0x3FEDD6503CDCD4C3, 0x3FEDDA80E43791D0, 0x3FEDDEAE8509018F, 0x3FEDE2D923AC8AB0, 0x3FEDE700C474313A, 0x3FEDEB256BA8B16D, 0x3FEDEF471D899A46, 0x3FEDF365DE4D679B, 0x3FEDF781B2219BE7, 0x3FEDFB9A9D2AD9B5, 0x3FEDFFB0A384FCAF, 0x3FEE03C3C9433256, 0x3FEE07D412701267, 0x3FEE0BE1830DB6E8, 0x3FEE0FEC1F15D3DE, 0x3FEE13F3EA79CEB5, 0x3FEE17F8E922D556, 0x3FEE1BFB1EF1F4E9, 0x3FEE1FFA8FC0304D, 0x3FEE23F73F5E9642, 0x3FEE27F131965743, 0x3FEE2BE86A28DB1D, 0x3FEE2FDCECCFD62F, 0x3FEE33CEBD3D5E74, 0x3FEE37BDDF1C0030, 0x3FEE3BAA560ED26B, 0x3FEE3F9425B18B1A, 0x3FEE437B51989307, 0x3FEE475FDD51197A, 0x3FEE4B41CC612798, 0x3FEE4F212247B38E, 0x3FEE52FDE27CB36A, 0x3FEE56D810712FC9, 0x3FEE5AAFAF8F5635, 0x3FEE5E84C33A8B56, 0x3FEE62574ECF7CDB, 0x3FEE662755A4332A, 0x3FEE69F4DB0822E1, 0x3FEE6DBFE2443E0D, 0x3FEE71886E9B0534, 0x3FEE754E83489820, 0x3FEE79122382C682, 0x3FEE7CD352792048, 0x3FEE8092135505D1, 0x3FEE844E6939B7E7, 0x3FEE880857446781, 0x3FEE8BBFE08C455E, 0x3FEE8F7508229159, 0x3FEE9327D112A9A8, 0x3FEE96D83E6219D2, 0x3FEE9A865310A983, 0x3FEE9E3212186B2A, 0x3FEEA1DB7E6DCA73, 0x3FEEA5829AFF9A83, 0x3FEEA9276AB72417, 0x3FEEACC9F078336B, 0x3FEEB06A2F2125FF, 0x3FEEB408298AF82C, 0x3FEEB7A3E289528A, 0x3FEEBB3D5CEA9738, 0x3FEEBED49B77EEEE, 0x3FEEC269A0F555F2, 0x3FEEC5FC7021A8DD, 0x3FEEC98D0BB6B13C, 0x3FEECD1B76693209, 0x3FEED0A7B2E8F3FD, 0x3FEED431C3E0D1C5, 0x3FEED7B9ABF6C3FE, 0x3FEEDB3F6DCBED23, 0x3FEEDEC30BFCA549, 0x3FEEE244892085BC, 0x3FEEE5C3E7CA7478, 0x3FEEE9412A88AF7B, 0x3FEEECBC53E4D804, 0x3FEEF0356663FD9B, 0x3FEEF3AC6486A905, 0x3FEEF72150C8E71C, 0x3FEEFA942DA25376, 0x3FEEFE04FD8622FC, 0x3FEF0173C2E32E58, 0x3FEF04E08023FC4D, 0x3FEF084B37AECBE3, 0x3FEF0BB3EBE59E86, 0x3FEF0F1A9F2641F6, 0x3FEF127F53CA5A2A, 0x3FEF15E20C276B07, 0x3FEF1942CA8EE208, 0x3FEF1CA1914E1FC3, 0x3FEF1FFE62AE814E, 0x3FEF235940F56994, 0x3FEF26B22E644A88, 0x3FEF2A092D38AE3A, 0x3FEF2D5E3FAC3FDA, 0x3FEF30B167F4D49F, 0x3FEF3402A8447492, 0x3FEF375202C96338, 0x3FEF3A9F79AE283C, 0x3FEF3DEB0F1997E2, 0x3FEF4134C52EDB7B, 0x3FEF447C9E0D79AC, 0x3FEF47C29BD15EB2, 0x3FEF4B06C092E47C, 0x3FEF4E490E66DAC1, 0x3FEF5189875E8EE6, 0x3FEF54C82D87D3F0, 0x3FEF580502ED0A39, 0x3FEF5B4009952734, 0x3FEF5E794383BD00, 0x3FEF61B0B2B901F4, 0x3FEF64E65931D813, 0x3FEF681A38E7D46B, 0x3FEF6B4C53D1465E, 0x3FEF6E7CABE13EDB, 0x3FEF71AB4307977E, 0x3FEF74D81B30F9A0, 0x3FEF78033646E554, 0x3FEF7B2C962FB84F, 0x3FEF7E543CCEB4BD, 0x3FEF817A2C040808, 0x3FEF849E65ACD180, 0x3FEF87C0EBA32905, 0x3FEF8AE1BFBE258D, 0x3FEF8E00E3D1E3A1, 0x3FEF911E59AF8BC5, 0x3FEF943A232558D5, 0x3FEF975441FE9E48, 0x3FEF9A6CB803CE6A, 0x3FEF9D8386FA807F, 0x3FEFA098B0A576DA, 0x3FEFA3AC36C4A4E6, 0x3FEFA6BE1B153517, 0x3FEFA9CE5F518ED1, 0x3FEFACDD05315C3F, 0x3FEFAFEA0E69901B, 0x3FEFB2F57CAC6B64, 0x3FEFB5FF51A98307, 0x3FEFB9078F0DC578, 0x3FEFBC0E36838041, 0x3FEFBF1349B26578, 0x3FEFC216CA3F9136, 0x3FEFC518B9CD8EEE, 0x3FEFC81919FC5EC8, 0x3FEFCB17EC697ADF, 0x3FEFCE1532AFDC7F, 0x3FEFD110EE680144, 0x3FEFD40B2127F042, 0x3FEFD703CC833F0F, 0x3FEFD9FAF20B16BF, 0x3FEFDCF0934E38E6, 0x3FEFDFE4B1D90478, 0x3FEFE2D74F357AA7, 0x3FEFE5C86CEB43B2, 0x3FEFE8B80C7FB3AD, 0x3FEFEBA62F75CF34, 0x3FEFEE92D74E5017, 0x3FEFF17E0587A9FE, 0x3FEFF467BB9E0EF9, 0x3FEFF74FFB0B7409, 0x3FEFFA36C54795A1, 0x3FEFFD1C1BC7FC14, 0x3FF0000000000000 };

	/**
	\ingroup Constants-Enums
	Built-in reverse-convex lookup table */
	static uint64_t reverseconvexLUT[512] = { 0x0, 0x3F06C9175FA32C60, 0x3F16E8CDAD10AC81, 0x3F21468F1EA72923, 0x3F2728EB4DE855DE, 0x3F2D1BA87E1F7028, 0x3F318F7A23551FD1, 0x3F34997E5EE716EB, 0x3F37ABF83D0CC24D, 0x3F3AC6FF49E444B1, 0x3F3DEAAB5317370F, 0x3F408B8A34488E4F, 0x3F4226296E9BE843, 0x3F43C53FA3D2FA6E, 0x3F4568D94173B577, 0x3F471102D79BE93A, 0x3F48BDC91961912D, 0x3F4A6F38DD342A9B, 0x3F4C255F1D3F19A6, 0x3F4DE048F7CD2139, 0x3F4FA003AFACEF68, 0x3F50B24E564B60CB, 0x3F519710BDC991D2, 0x3F527E4FE7B16689, 0x3F536812C073CFB4, 0x3F54546047C76431, 0x3F55433F90DE0659, 0x3F5634B7C29B1DE2, 0x3F5728D017CA68EC, 0x3F581F8FDF5766C9, 0x3F5918FE7C855B0E, 0x3F5A15236727EE0C, 0x3F5B14062BDC6A5D, 0x3F5C15AE6C4399B2, 0x3F5D1A23DF3C436E, 0x3F5E216E511E4C4C, 0x3F5F2B95A3F67CDD, 0x3F601C50E7E1768E, 0x3F60A44D71580BF9, 0x3F612DC480AB4CC7, 0x3F61B8BA337CCC98, 0x3F624532B2E2F055, 0x3F62D3323388D1D5, 0x3F6362BCF5CE7C2D, 0x3F63F3D745E981CB, 0x3F6486857C05ED38, 0x3F651ACBFC678DC8, 0x3F65B0AF378BA083, 0x3F664833AA4AD6EA, 0x3F66E15DDDFBBD23, 0x3F677C3268957EAB, 0x3F6818B5ECD30CC5, 0x3F68B6ED1A56A65A, 0x3F6956DCADCDC2C9, 0x3F69F8897115610B, 0x3F6A9BF83B5EBAF5, 0x3F6B412DF1545FDD, 0x3F6BE82F853FB5DB, 0x3F6C9101F72EE339, 0x3F6D3BAA551B21EF, 0x3F6DE82DBB0F7D8C, 0x3F6E9691534FFCFE, 0x3F6F46DA56813957, 0x3F6FF90E0BD062CE, 0x3F705698E48DDA96, 0x3F70B1A5798DAE52, 0x3F710DAF7EC56680, 0x3F716AB9B5A94598, 0x3F71C8C6E75935AA, 0x3F7227D9E4B622A6, 0x3F7287F586778F49, 0x3F72E91CAD4166BE, 0x3F734B5241BA0AB4, 0x3F73AE9934A09ED9, 0x3F7412F47EE392A7, 0x3F74786721B76957, 0x3F74DEF426ADC1F0, 0x3F75469E9FCC9F1E, 0x3F75AF69A7A5EFA1, 0x3F761958616F589C, 0x3F76846DF91A41F3, 0x3F76F0ADA36C2591, 0x3F775E1A9E17226B, 0x3F77CCB82FD2D3DD, 0x3F783C89A8756E39, 0x3F78AD92610D2124, 0x3F791FD5BBF9C0DF, 0x3F7993572506B679, 0x3F7A081A118538C4, 0x3F7A7E220066CE09, 0x3F7AF5727A581763, 0x3F7B6E0F11DBE6EE, 0x3F7BE7FB6366A136, 0x3F7C633B1579EAE0, 0x3F7CDFD1D8C0A37E, 0x3F7D5DC3682B2E0C, 0x3F7DDD13890C0845, 0x3F7E5DC60B34B1BB, 0x3F7EDFDEC912E310, 0x3F7F6361A7CE16B3, 0x3F7FE852976563AC, 0x3F80375AC966D5F0, 0x3F807B4750080EF8, 0x3F80BFF0E8348600, 0x3F810559A0338C60, 0x3F814B838C055D6F, 0x3F819270C5730C30, 0x3F81DA236C1E9D2B, 0x3F82229DA5934D5E, 0x3F826BE19D56064D, 0x3F82B5F184F5FFD9, 0x3F8300CF941D907D, 0x3F834C7E08A32C50, 0x3F8398FF269A9345, 0x3F83E65538662F67, 0x3F8434828EC8A332, 0x3F84838980F688F3, 0x3F84D36C6CA8639A, 0x3F85242DB62CC158, 0x3F8575CFC87A90C4, 0x3F85C8551543A904, 0x3F861BC0150785B4, 0x3F867013472636BE, 0x3F86C55131F384E7, 0x3F871B7C62CA4BD4, 0x3F8772976E2009D7, 0x3F87CAA4EF98A61E, 0x3F8823A78A1A6DE6, 0x3F887DA1E7E24988, 0x3F88D896BA98299B, 0x3F893488BB63ACE6, 0x3F89917AAB00FFED, 0x3F89EF6F51D5F689, 0x3F8A4E6980076037, 0x3F8AAE6C0D8E97D8, 0x3F8B0F79DA4F4F5D, 0x3F8B7195CE2D9814, 0x3F8BD4C2D9242897, 0x3F8C3903F35AE056, 0x3F8C9E5C1D3D8A05, 0x3F8D04CE5F92DD3E, 0x3F8D6C5DCB93C080, 0x3F8DD50D7B02CB8A, 0x3F8E3EE090440B3E, 0x3F8EA9DA367507D9, 0x3F8F15FDA1850DEC, 0x3F8F834E0E4DBAEC, 0x3F8FF1CEC2ABCDED, 0x3F9030C186CC1EEC, 0x3F90693723A0CA96, 0x3F90A249E892CA23, 0x3F90DBFB8B158D9D, 0x3F91164DC55E2DD6, 0x3F9151425670A9C9, 0x3F918CDB022D48EA, 0x3F91C919915E2290, 0x3F9205FFD1C4CB25, 0x3F92438F96282771, 0x3F9281CAB6626647, 0x3F92C0B30F6F2119, 0x3F93004A8379A3EA, 0x3F934092F9EB5D12, 0x3F93818E5F7A7513, 0x3F93C33EA6388F05, 0x3F9405A5C5A1B240, 0x3F9448C5BAAB5D64, 0x3F948CA087D3C365, 0x3F94D138353132E5, 0x3F95168ED081A89E, 0x3F955CA66D3A8D13, 0x3F95A38124989DF1, 0x3F95EB2115B003E4, 0x3F963388657C9523, 0x3F967CB93EF24538, 0x3F96C6B5D30DC29D, 0x3F97118058E5427A, 0x3F975D1B0DB97B32, 0x3F97A9883506CE4D, 0x3F97F6CA1896A203, 0x3F9844E30890EB18, 0x3F9893D55B8DE78F, 0x3F98E3A36EA80AD6, 0x3F99344FA58E1BA6, 0x3F9985DC6A95844C, 0x3F99D84C2ECCD62D, 0x3F9A2BA16A0E8093, 0x3F9A7FDE9B13BBB3, 0x3F9AD5064787A82F, 0x3F9B2B1AFC1AA41C, 0x3F9B821F4C95D5A5, 0x3F9BDA15D3EEEC0D, 0x3F9C3301345C17CC, 0x3F9C8CE417683A2C, 0x3F9CE7C12E074D32, 0x3F9D439B30AB041D, 0x3F9DA074DF57A593, 0x3F9DFE5101B91FA0, 0x3F9E5D3267385686, 0x3F9EBD1BE710AEDA, 0x3F9F1E106065D373, 0x3F9F8012BA59B80D, 0x3F9FE325E422D954, 0x3FA023A66A915D4C, 0x3FA05645467E5125, 0x3FA0897109D64ACE, 0x3FA0BD2B3CCF4A38, 0x3FA0F1756BE30AC6, 0x3FA1265127DAE224, 0x3FA15BC005DBC047, 0x3FA191C39F7250BA, 0x3FA1C85D929F3DAB, 0x3FA1FF8F81E3951D, 0x3FA2375B144D50AC, 0x3FA26FC1F5840026, 0x3FA2A8C5D5D59758, 0x3FA2E2686A435F96, 0x3FA31CAB6C8F0D3D, 0x3FA357909B47F9A2, 0x3FA39319B9D881E0, 0x3FA3CF4890938ABE, 0x3FA40C1EECC22A4B, 0x3FA4499EA0B1778F, 0x3FA487C983C0808D, 0x3FA4C6A1726E6733, 0x3FA506284E68A584, 0x3FA5465FFE9979A5, 0x3FA5874A6F3679D9, 0x3FA5C8E991CF5114, 0x3FA60B3F5D5CA4CA, 0x3FA64E4DCE4F2400, 0x3FA69216E69EC05C, 0x3FA6D69CADDA1182, 0x3FA71BE13135E36B, 0x3FA761E6839CEFF4, 0x3FA7A8AEBDBFC420, 0x3FA7F03BFE24D1BB, 0x3FA838906938AD9E, 0x3FA881AE295E7B38, 0x3FA8CB976F0085D0, 0x3FA9164E70A107E2, 0x3FA961D56AEB2138, 0x3FA9AE2EA0C3FC67, 0x3FA9FB5C5B5C23C4, 0x3FAA4960EA4106B9, 0x3FAA983EA36EAFCA, 0x3FAAE7F7E361AC22, 0x3FAB388F0D2924B2, 0x3FAB8A068A7929B2, 0x3FABDC60CBBD314B, 0x3FAC2FA0482AC991, 0x3FAC83C77DD47E8B, 0x3FACD8D8F1BCF4C1, 0x3FAD2ED72FEA392C, 0x3FAD85C4CB7946CB, 0x3FADDDA45EB1C276, 0x3FAE36788B19EDC8, 0x3FAE9043F98AD17D, 0x3FAEEB095A449FEF, 0x3FAF46CB65035058, 0x3FAFA38CD913737C, 0x3FB000A83EB3A109, 0x3FB0300C9055F31F, 0x3FB05FF4CCAF7FE2, 0x3FB0906262F23249, 0x3FB0C156C64E0EB8, 0x3FB0F2D36DFC5029, 0x3FB124D9D54AA414, 0x3FB1576B7BA685B7, 0x3FB18A89E4A8B8F0, 0x3FB1BE369820E51E, 0x3FB1F2732221501C, 0x3FB22741130ABA3B, 0x3FB25CA1FF985AD7, 0x3FB2929780EBFEAE, 0x3FB2C923349A478F, 0x3FB30046BCB70E88, 0x3FB33803BFE1E82F, 0x3FB3705BE952CC06, 0x3FB3A950E8E6DEEC, 0x3FB3E2E4732D6153, 0x3FB41D184174C135, 0x3FB457EE11D7D0BB, 0x3FB49367A74B2160, 0x3FB4CF86C9AA8480, 0x3FB50C4D45C6B13B, 0x3FB549BCED7310A5, 0x3FB587D79793B013, 0x3FB5C69F202B5A60, 0x3FB606156869D872, 0x3FB6463C56BA5968, 0x3FB68715D6D202C1, 0x3FB6C8A3D9BEA952, 0x3FB70AE855F5B2E4, 0x3FB74DE547632193, 0x3FB7919CAF78C8AB, 0x3FB7D610953DAC54, 0x3FB81B43055D8B8A, 0x3FB86136123895B1, 0x3FB8A7EBD3F34BB5, 0x3FB8EF6668868D87, 0x3FB937A7F3CFD3FA, 0x3FB980B29FA19836, 0x3FB9CA889BD3E875, 0x3FBA152C1E552B27, 0x3FBA609F633B1091, 0x3FBAACE4ACD3B3C8, 0x3FBAF9FE43B6EB12, 0x3FBB47EE76D7C8DC, 0x3FBB96B79B964CF3, 0x3FBBE65C0DD14775, 0x3FBC36DE2FF86D1B, 0x3FBC88406B1E9E36, 0x3FBCDA852F0C6044, 0x3FBD2DAEF2528B28, 0x3FBD81C0325D2A28, 0x3FBDD6BB738691BD, 0x3FBE2CA3412AAA30, 0x3FBE837A2DBA7042, 0x3FBEDB42D2CFABC2, 0x3FBF33FFD140DD67, 0x3FBF8DB3D13563B0, 0x3FBFE8618239D84D, 0x3FC02205CDAA52EB, 0x3FC0505A6D8D6B8B, 0x3FC07F3003E28FFD, 0x3FC0AE87F7A290B6, 0x3FC0DE63B3AD73FB, 0x3FC10EC4A6D5534D, 0x3FC13FAC43E956E8, 0x3FC1711C01C0CFFF, 0x3FC1A3155B4671B3, 0x3FC1D599CF83A96F, 0x3FC208AAE1AC16BA, 0x3FC23C4A19292311, 0x3FC2707901A5B9DF, 0x3FC2A5392B1A2148, 0x3FC2DA8C29D7F3B6, 0x3FC3107396963AEE, 0x3FC346F10E7DACA8, 0x3FC37E0633350968, 0x3FC3B5B4AAED9D93, 0x3FC3EDFE206FE581, 0x3FC426E443285495, 0x3FC46068C7344014, 0x3FC49A8D656EEDB8, 0x3FC4D553DB7EC6DC, 0x3FC510BDEBE2B021, 0x3FC54CCD5DFF8681, 0x3FC58983FE2DC19E, 0x3FC5C6E39DC73C53, 0x3FC604EE1335235E, 0x3FC643A539FE0B06, 0x3FC6830AF2D42BD5, 0x3FC6C32123A3C707, 0x3FC703E9B7A1B2F2, 0x3FC745669F5A1015, 0x3FC78799D0BF26D9, 0x3FC7CA8547386F02, 0x3FC80E2B03B1C0A2, 0x3FC8528D0CAAAFA7, 0x3FC897AD6E4611D6, 0x3FC8DD8E3A59B050, 0x3FC92431887E2475, 0x3FC96B99761EE136, 0x3FC9B3C8268A68D0, 0x3FC9FCBFC302AFD5, 0x3FCA46827ACDADA4, 0x3FCA911283461B37, 0x3FCADC7217EC6041, 0x3FCB28A37A77AFC0, 0x3FCB75A8F2E753DA, 0x3FCBC384CF942A36, 0x3FCC1239654250A3, 0x3FCC61C90F330351, 0x3FCCB2362F36AC7A, 0x3FCD03832DBF2682, 0x3FCD55B279F230C0, 0x3FCDA8C689BC17E4, 0x3FCDFCC1D9E291F6, 0x3FCE51A6EE17CF35, 0x3FCEA778510DBFA9, 0x3FCEFE3894898EB4, 0x3FCF55EA51775488, 0x3FCFAE9027FDFEB7, 0x3FD004165FC9B800, 0x3FD0316163886C1C, 0x3FD05F2A7A63A258, 0x3FD08D73034A3512, 0x3FD0BC3C60FBD40F, 0x3FD0EB87FA13A3A0, 0x3FD11B573912F928, 0x3FD14BAB8C6C35B7, 0x3FD17C86668DBEA3, 0x3FD1ADE93DED14E3, 0x3FD1DFD58D120B19, 0x3FD2124CD2A21AF6, 0x3FD24550916BDA11, 0x3FD278E250728EB3, 0x3FD2AD039AF9E4CA, 0x3FD2E1B60091C399, 0x3FD316FB1522441E, 0x3FD34CD470F7C90C, 0x3FD38343B0CF3825, 0x3FD3BA4A75E255DC, 0x3FD3F1EA65F4432D, 0x3FD42A252B5E1E5C, 0x3FD462FC751BC6B9, 0x3FD49C71F6D8C419, 0x3FD4D68768FD51FE, 0x3FD5113E88BB8F57, 0x3FD54C99181CD2A4, 0x3FD58898DE0F2382, 0x3FD5C53FA672D978, 0x3FD6028F422860DE, 0x3FD64089871E25F1, 0x3FD67F30505EA6C9, 0x3FD6BE857E1EAC56, 0x3FD6FE8AF5CBAB18, 0x3FD73F42A21A4BAF, 0x3FD780AE73151C1F, 0x3FD7C2D05E2B69AB, 0x3FD805AA5E404456, 0x3FD8493E73B9ABE7, 0x3FD88D8EA48FE76E, 0x3FD8D29CFC5D0737, 0x3FD9186B8C6C9235, 0x3FD95EFC6BCB5EBF, 0x3FD9A651B75797BA, 0x3FD9EE6D91D0EE0B, 0x3FDA375223E8F76C, 0x3FDA81019C53BA84, 0x3FDACB7E2FD86955, 0x3FDB16CA196249FC, 0x3FDB62E79A11CEC3, 0x3FDBAFD8F94DDD78, 0x3FDBFDA084D5473C, 0x3FDC4C4090D0708D, 0x3FDC9BBB77E32AD4, 0x3FDCEC139B3EBF4E, 0x3FDD3D4B62B42C80, 0x3FDD8F653CC69613, 0x3FDDE2639EBDE870, 0x3FDE364904B9AFCE, 0x3FDE8B17F1C4240F, 0x3FDEE0D2EFE56953, 0x3FDF377C90370677, 0x3FDF8F176AF79159, 0x3FDFE7A61F9E924B, 0x3FE02095AA784F42, 0x3FE04DD4DC89D56D, 0x3FE07B9200D1CB59, 0x3FE0A9CE75E37845, 0x3FE0D88B9E21F9F0, 0x3FE107CADFCAE0FC, 0x3FE1378DA500EA9E, 0x3FE167D55BD6D86F, 0x3FE198A3765A6620, 0x3FE1C9F96A9F5DF2, 0x3FE1FBD8B2CACBD4, 0x3FE22E42CD1E4FD7, 0x3FE261393C038FF7, 0x3FE294BD8617C9F7, 0x3FE2C8D136378539, 0x3FE2FD75DB8A6555, 0x3FE332AD098F1D6D, 0x3FE36878582784EF, 0x3FE39ED963A4CDCB, 0x3FE3D5D1CCD3DCDA, 0x3FE40D633909C468, 0x3FE4458F523061B8, 0x3FE47E57C6D31D65, 0x3FE4B7BE4A2BCF86, 0x3FE4F1C4942FC76E, 0x3FE52C6C619CF7F2, 0x3FE567B774074821, 0x3FE5A3A791E60942, 0x3FE5E03E86A19213, 0x3FE61D7E22A1001D, 0x3FE65B683B581F21, 0x3FE699FEAB55776C, 0x3FE6D9435250830E, 0x3FE7193815380ADF, 0x3FE759DEDE40AB42, 0x3FE79B399CF3818C, 0x3FE7DD4A463D0207, 0x3FE82012D47BF783, 0x3FE863954790AC69, 0x3FE8A7D3A4EC3E38, 0x3FE8ECCFF7A01B77, 0x3FE9328C506DACFB, 0x3FE9790AC5D62A8F, 0x3FE9C04D742A9BE4, 0x3FEA08567D9C05D2, 0x3FEA51280A4BC4E4, 0x3FEA9AC4485C151A, 0x3FEAE52D6C00C806, 0x3FEB3065AF902915, 0x3FEB7C6F53941130, 0x3FEBC94C9EDB2998, 0x3FEC16FFDE8A5F16, 0x3FEC658B662E857B, 0x3FECB4F18FCE2C7A, 0x3FED0534BBFBA5E1, 0x3FED565751E73E45, 0x3FEDA85BBF71A81C, 0x3FEDFB44793E9A64, 0x3FEE4F13FAC7A2DE, 0x3FEEA3CCC66F2CE6, 0x3FEEF9716593BD0A, 0x3FEF500468A3625F, 0x3FEFA788672F5DC0, 0x3FF0000000000000 };

	/**
	@readHexLUT
	\ingroup SynthFunctions
	\brief
	Read a table that has been encoded as uint64_t HEX values

	\param table the table to read
	\param xn the fractional location within the table to read

	\return the interpolated value as a double
	*/
	inline double readHexLUT(uint64_t* table, double xn)
	{
		uint32_t readIndex = uint32_t(xn * (xformLUTLen - 1));
		double fracPart = (xn * (xformLUTLen - 1)) - readIndex;
		uint32_t nextReadIndex = readIndex + 1; 
		if (nextReadIndex >= xformLUTLen)nextReadIndex = 0;

		uint64_t u0 = table[readIndex];
		uint64_t u1 = table[nextReadIndex];
		double d0 = *(reinterpret_cast<double*>(&u0));
		double d1 = *(reinterpret_cast<double*>(&u1));
		return doLinearInterpolation(d0, d1, fracPart);
	}

	/**
	@concaveXForm
	\ingroup SynthFunctions
	\brief
	Perform the MMA concave tranform on a unipolar value
	- option to use the calculation or the lookup table (LUT)

	\param xn the normalized value to transform
	\param useLUT set to true to use lookup table instead of direct calculation

	\return the convex transformed value 
	*/
	inline double concaveXForm(double xn, bool useLUT = false)
	{
		if (xn >= 1.0) return 1.0;
		if (xn <= 0.0) return 0.0;

		double transformed = 0.0;
		if (useLUT)
			transformed = readHexLUT(&concaveLUT[0], xn);
		else
			transformed = -kCTCoefficient*kCTCorrFactorAntiLogScale*log10(1.0 - xn + kCTCorrFactorZero) + kCTCorrFactorAntiLog;

		boundValue(transformed, 0.0, +1.0);
		return transformed;
	}

	/**
	@bipolarConcaveXForm
	\ingroup SynthFunctions
	\brief
	Perform the MMA concave tranform on a bipolar value
	- option to use the calculation or the lookup table (LUT)

	\param xn the normalized bipolar value to transform
	\param useLUT set to true to use lookup table instead of direct calculation

	\return the convex transformed value
	*/
	inline double bipolarConcaveXForm(double xn, bool useLUT = false)
	{
		if (xn >= 0.0)
			return  concaveXForm(xn, useLUT);
		else
			return -concaveXForm(-xn, useLUT);

		return xn;
	}

	/**
	@reverseConcaveXForm
	\ingroup SynthFunctions
	\brief
	Perform the MMA reverse concave tranform on a unipolar value
	- option to use the calculation or the lookup table (LUT)

	\param xn the unipolar bipolar value to transform
	\param useLUT set to true to use lookup table instead of direct calculation

	\return the reverse convex transformed value
	*/
	inline double reverseConcaveXForm(double xn, bool useLUT = false)
	{
		double transformed = xn;
		if (useLUT)
			transformed = readHexLUT(&reverseconcaveLUT[0], xn);
		else
			transformed = (kCTCorrFactorAntiUnity)*(-pow(10.0, (-xn / kCTCoefficient)) + 1.0);
		
		boundValue(transformed, 0.0, +1.0);
		return transformed;
	}

	/**
	@bipolarReverseConcaveXForm
	\ingroup SynthFunctions
	\brief
	Perform the MMA reverse concave tranform on a bipolar value
	- option to use the calculation or the lookup table (LUT)

	\param xn the bipolar bipolar value to transform
	\param useLUT set to true to use lookup table instead of direct calculation

	\return the reverse convex transformed value
	*/
	inline double bipolarReverseConcaveXForm(double xn, bool useLUT = false)
	{
		if (xn >= 0.0)
		{
			return  reverseConcaveXForm(xn, useLUT);
		}
		else
		{
			return -reverseConcaveXForm(-xn, useLUT);
		}
		return xn;
	}

	/**
	@convexXForm
	\ingroup SynthFunctions
	\brief
	Perform the MMA convex tranform on a unipolar value
	- option to use the calculation or the lookup table (LUT)

	\param xn the unipolar value to transform
	\param useLUT set to true to use lookup table instead of direct calculation

	\return the convex transformed value
	*/
	inline double convexXForm(double xn, bool useLUT = false)
	{
		if (xn <= 0.0) return 0.0;
		
		double transformed = xn;
		if (useLUT)
			transformed = readHexLUT(&convexLUT[0], xn);
		else
			transformed = kCTCorrFactorUnity*(1.0 + kCTCoefficient*log10(xn + kCTCorrFactorZero));
		
		boundValue(transformed, 0.0, +1.0);
		return transformed;
	}

	/**
	@bipolarConvexXForm
	\ingroup SynthFunctions
	\brief
	Perform the MMA convex tranform on a bipolar value
	- option to use the calculation or the lookup table (LUT)

	\param xn the bipolar value to transform
	\param useLUT set to true to use lookup table instead of direct calculation

	\return the convex transformed value
	*/
	inline double bipolarConvexXForm(double xn, bool useLUT = false)
	{
		if (xn >= 0.0)
		{
			return  convexXForm(xn, useLUT);// useLUT);
		}
		else
		{
			return -convexXForm(-xn, useLUT); //useLUT);
		}
		return xn;
	}

	/**
	@reverseConvexXForm
	\ingroup SynthFunctions
	\brief
	Perform the MMA reverse convex tranform on a unipolar value
	- option to use the calculation or the lookup table (LUT)

	\param xn the unipolar value to transform
	\param useLUT set to true to use lookup table instead of direct calculation

	\return the reverse convex transformed value
	*/
	inline double reverseConvexXForm(double xn, bool useLUT = false)
	{
		double transformed = xn;
		if (useLUT)
			transformed = readHexLUT(&reverseconvexLUT[0], xn);
		else
			transformed = kCTCorrFactorAnitZero * (pow(10.0, (xn - 1) / kCTCoefficient) - kCTCorrFactorZero);
		
		boundValue(transformed, -1.0, +1.0);
		return transformed;
	}

	/**
	@bipolarReverseConvexXForm
	\ingroup SynthFunctions
	\brief
	Perform the MMA reverse convex tranform on a bipolar value
	- option to use the calculation or the lookup table (LUT)

	\param xn the unipolar value to transform
	\param useLUT set to true to use lookup table instead of direct calculation

	\return the bipolar convex transformed value
	*/
	inline double bipolarReverseConvexXForm(double xn, bool useLUT = false)
	{
		if (xn >= 0.0)
		{
			return  reverseConvexXForm(xn);
		}
		else
		{
			return -reverseConvexXForm(-xn);
		}
		return xn;
	}

	/**
	@doPolyBLEP_2
	\ingroup SynthFunctions
	\brief
	Calculates the 2nd order polynomial BLEP correction factor

	\param mcounter the modulo counter value that is clocking the oscillator
	\param phaseInc counter's phase increment value
	\param height normalized height of the discontinuity to correction
	\param risingEdge true if discontinuity is a rising edge, false if falling edge

	\return the polyBLEP correction factor
	*/
	inline double doPolyBLEP_2(double mcounter, double phaseInc, double height, bool risingEdge)
	{
		// --- return value
		double blepCorrection = 0.0;

		// --- LEFT side of discontinuity
		//	   -1 < t < 0
		if (mcounter > 1.0 - phaseInc)
		{
			// --- calculate distance
			double t = (mcounter - 1.0) / phaseInc;

			// --- calculate residual
			blepCorrection = height*(t*t + 2.0*t + 1.0);
		}
		// --- RIGHT side of discontinuity
		//     0 <= t < 1
		else if (mcounter < phaseInc)
		{
			// --- calculate distance
			double t = mcounter / phaseInc;

			// --- calculate residual
			blepCorrection = height*(2.0*t - t*t - 1.0);
		}

		// --- subtract for falling, add for rising edge
		if (!risingEdge)
			blepCorrection *= -1.0;

		return blepCorrection;
	}

	/**
	@doBLEP_N
	\ingroup SynthFunctions
	\brief
	Calculates the BLEP correction factor for 1, 2, 3 or 4 points of correction
	on each side of the waveform discontinuity.
	- uses residual tables found in bleptables.h that have the BLEP residual encoded with a variety of windows

	\param tableLength lenght of the BLEP residual table
	\param modCounter the modulo counter value that is clocking the oscillator
	\param phaseInc counter's phase increment value
	\param height normalized height of the discontinuity to correction
	\param risingEdge true if discontinuity is a rising edge, false if falling edge
	\param pointsPerSide number of points per side to correct
	\param interpolate set true to interpolate BLEP tables

	\return the BLEP correction factor
	*/
	inline double doBLEP_N(uint32_t tableLength, double modCounter, double phaseInc, double height,
		bool risingEdge, uint32_t pointsPerSide, bool interpolate = false)
	{
		const double* blepTable = nullptr;
		if (pointsPerSide == 1)
			blepTable = &dBLEPTable[0];
		else
			blepTable = &dBLEPTable_8_BLKHAR[0];

		if (!blepTable) return 0.0;

		// return value
		double blepCorrection = 0.0;

		// t = the distance from the discontinuity
		double t = 0.0;

		// --- find the center of table (discontinuity location)
		double tableCenter = tableLength / 2.0 - 1;

		// LEFT side of edge
		// -1 < t < 0
		for (uint32_t i = 1; i <= pointsPerSide; i++)
		{
			if (modCounter > 1.0 - i*phaseInc)
			{
				// --- calculate distance
				t = (modCounter - 1.0) / (pointsPerSide*phaseInc);

				// --- get index
				float index = (1.0 + t)*tableCenter;
				uint32_t indexN = uint32_t(index);

				// --- truncation
				if (!interpolate)
				{
					if (indexN < tableLength)
						blepCorrection = blepTable[indexN];
				}
				else if (indexN < tableLength && indexN + 1 < tableLength)
				{
					float index = (1.0 + t)*tableCenter;
					float frac = index - indexN;
					blepCorrection = doLinearInterpolation(0, 1, blepTable[indexN], blepTable[indexN + 1], frac);
				}

				// --- subtract for falling, add for rising edge
				if (!risingEdge)
					blepCorrection *= -1.0;

				return height*blepCorrection;
			}
		}

		// RIGHT side of discontinuity
		// 0 <= t < 1
		for (uint32_t i = 1; i <= pointsPerSide; i++)
		{
			if (modCounter < (double)i*phaseInc)
			{
				// calculate distance
				t = modCounter / (pointsPerSide*phaseInc);

				// --- get index
				float index = t*tableCenter + (tableCenter + 1.0);
				uint32_t indexN = uint32_t(index);

				// truncation
				if (!interpolate)
				{
					if (indexN < tableLength)
						blepCorrection = blepTable[indexN];
				}
				else if (indexN < tableLength && indexN + 1 < tableLength)
				{
					float frac = index - indexN;
					if (indexN + 1 >= tableLength)
						blepCorrection = doLinearInterpolation(0, 1, blepTable[indexN], blepTable[0], frac);
					else
						blepCorrection = doLinearInterpolation(0, 1, blepTable[indexN], blepTable[indexN + 1], frac);
				}

				// subtract for falling, add for rising edge
				if (!risingEdge)
					blepCorrection *= -1.0;

				return height*blepCorrection;
			}
		}
		return 0.0;
	}
	
	/**
	@initDMConfig
	\ingroup MIDIFunctions
	\brief
	Initializes the MIDI aux data with configuration information; for DM only

	\param midiInputData shared pointer to MIDI input data array
	*/
	inline void initDMConfig(std::shared_ptr<MidiInputData> midiInputData, DMConfig* config)
	{
		// --- default setting, these match the book
		midiInputData->setAuxDAWDataUINT(kDMBuild, 0);
		midiInputData->setAuxDAWDataUINT(kDualMonoFilters, 0);
		midiInputData->setAuxDAWDataUINT(kHalfSampleSet, 0);
		midiInputData->setAuxDAWDataUINT(kReduceUnisonVoices, 0);
		midiInputData->setAuxDAWDataUINT(kAnalogFGNFilters, 0);
			
		if (!config) return;

		if (config->dm_build)
			midiInputData->setAuxDAWDataUINT(kDMBuild, 1);
		if (config->dual_mono_filters)
			midiInputData->setAuxDAWDataUINT(kDualMonoFilters, 1);
		if (config->half_sample_set)
			midiInputData->setAuxDAWDataUINT(kHalfSampleSet, 1);
		if (config->reduced_unison_count)
			midiInputData->setAuxDAWDataUINT(kReduceUnisonVoices, 1);
		if (config->analog_fgn_filters)
			midiInputData->setAuxDAWDataUINT(kAnalogFGNFilters, 1);
	}

	/**
	@initMIDIInputData
	\ingroup MIDIFunctions
	\brief
	Initializes the MIDI input data arrays with values that prevent the synth from appearing to be
	malfunctioning. This includes bringing volume controls up to unity gain (max output) and pan
	to the center as well as setting pitch bend range and overall tuning (known as master volume 
	and master tuning in the MMA documentation)

	\param midiInputData shared pointer to MIDI input data array
	*/
	inline void initMIDIInputData(std::shared_ptr<MidiInputData> midiInputData)
	{
		// --- this is recommended by the MIDI Manufacturer's Asssociation to force a reset
		//     so that the state is known
		midiInputData->setGlobalMIDIData(kCurrentMIDINoteNumber, 60);	// ---
		midiInputData->setGlobalMIDIData(kLastMIDINoteNumber, 60);		// --- 

		midiInputData->setGlobalMIDIData(kMIDIPitchBendDataLSB, 0);		// --- this is for 0 pitchbend at startup
		midiInputData->setGlobalMIDIData(kMIDIPitchBendDataMSB, 64);	// --- this is for 0 pitchbend at startup

		midiInputData->setGlobalMIDIData(kMIDIMasterPBSensCoarse, 1);	// --- this is for 0 kMIDIMasterPitchBendCoarse at startup
		midiInputData->setGlobalMIDIData(kMIDIMasterPBSensFine, 0);		// --- this is for 0 kMIDIMasterPitchBendFine at startup

		midiInputData->setGlobalMIDIData(kMIDIMasterTuneCoarseLSB, 0);	// --- this is for 0 kMIDIMasterPitchBendCoarse at startup
		midiInputData->setGlobalMIDIData(kMIDIMasterTuneCoarseMSB, 64);	// --- this is for 0 kMIDIMasterPitchBendFine at startup

		midiInputData->setGlobalMIDIData(kMIDIMasterTuneFineLSB, 0);	// --- this is for 0 kMIDIMasterPitchBendCoarse at startup
		midiInputData->setGlobalMIDIData(kMIDIMasterTuneFineMSB, 64);	// --- this is for 0 kMIDIMasterPitchBendFine at startup

		midiInputData->setGlobalMIDIData(kMIDIMasterVolumeLSB, 0);	// --- this is for 0 kMIDIMasterVolumeLSB at startup
		midiInputData->setGlobalMIDIData(kMIDIMasterVolumeMSB, 64);	// --- this is for 0 kMIDIMasterVolumeMSB at startup

		// --- volume/pan need to be preset so synth is not accidentally silent
		// --- CCs
		midiInputData->setCCMIDIData(VOLUME_CC07, 127);	// --- MIDI VOLUME; default this to ON
		midiInputData->setCCMIDIData(PAN_CC10, 64);		// --- MIDI PAN; default this to CENTER	}
	}
} // namespace

#endif
