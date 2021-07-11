#include "modmatrix.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   modmatrix.cpp
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
	Clears out matrix arrays for initial state

	\param _parameters shared parameters 

	\return the processed sample
	*/
	ModMatrix::ModMatrix(std::shared_ptr<ModMatrixParameters> _parameters)
		: parameters(_parameters)
	{
		if (!parameters)
			parameters.reset(new ModMatrixParameters);

		clearModMatrixArrays();
	}	/* C-TOR */

	/**
	\brief
	Adds a modulation source to the matrix 
	- each source is uniquely identified and placed into an array slot

	\param sourceArrayIndex unique index in the source array
	\param sourceModPtr pointer to the modulation source value connected to the slot

	\return the processed sample
	*/
	void ModMatrix::addModSource(uint32_t sourceArrayIndex, double* sourceModPtr)
	{
		if (sourceArrayIndex >= kNumberModSources) return;
		modSourceData[sourceArrayIndex] = sourceModPtr;
	}

	/**
	\brief
	Removes a modulation source to the matrix
	- each source is uniquely identified and placed into an array slot

	\param sourceArrayIndex unique index in the source array

	\return the processed sample
	*/
	void ModMatrix::clearModSource(uint32_t sourceArrayIndex)
	{
		if (sourceArrayIndex >= kNumberModSources) return;
		modSourceData[sourceArrayIndex] = nullptr;
	}

	/**
	\brief
	Adds a modulation destination to the matrix
	- each destination is uniquely identified and placed into an array slot
	- a transform is optionally applied during modulation calculation

	\param destArrayIndex unique index in the source array
	\param destModPtr pointer to the modulation source value connected to the slot
	\param transform optional transform, see \synthconstants.h

	\return the processed sample
	*/
	void ModMatrix::addModDestination(uint32_t destArrayIndex, double* destModPtr, uint32_t transform)
	{
		if (destArrayIndex >= kNumberModDestinations) return;
		modDestinationData[destArrayIndex] = destModPtr;
		modDestTransform[destArrayIndex] = transform;
	}

	/**
	\brief
	Removes a modulation destination to the matrix
	- each destination is uniquely identified and placed into an array slot

	\param destArrayIndex unique index in the source array 

	\return the processed sample
	*/
	void ModMatrix::clearModDestination(uint32_t destArrayIndex)
	{
		if (destArrayIndex >= kNumberModDestinations) return;
		modDestinationData[destArrayIndex] = nullptr;
		modDestTransform[destArrayIndex] = kNoMMTransform;
	}

	/**
	\brief
	Clears out all source and destination pointers from the array
	- usually only done once during the initialization process

	\return the processed sample
	*/
	void ModMatrix::clearModMatrixArrays()
	{
		memset(&modSourceData[0], 0, sizeof(double*)*kNumberModSources);
		memset(&modDestinationData[0], 0, sizeof(double*)*kNumberModDestinations);
		memset(&modDestTransform[0], 0, sizeof(uint32_t)*kNumberModDestinations);
	}

	/**
	\brief
	Runs the modulation matrix (see Synth book)
	- loops over source-destination pairs that are programmed during voice construction
	or as cores are loaded/unloaded
	- fetches source value and accumulates with destination modulation value
	- may apply a transform if needed

	\return the processed sample
	*/
	void ModMatrix::runModMatrix()
	{
		// --- sources are bound via pointers
		//
		// --- loop over destination columns
		// --- now calcualate the column modulation value
		double modDestinationValue = 0.0;

		for (int col = 0; col < kNumberModDestinations; col++)
		{
			ModDestination destination = parameters->modDestinationColumns->at(col);
			modDestinationValue = destination.defautValue;

			for (int row = 0; row < kNumberModSources; row++)
			{
				if (!modSourceData[row] || destination.channelEnable[row] == 0)
					continue;

				// --- get local modulation value 
				double modSourceValue = *modSourceData[row];
				uint32_t modTransform = modDestTransform[col];

				ModSource source = parameters->modSourceRows->at(row);

				// --- scale it
				if (!destination.channelHardwire[row])
					modSourceValue *= source.intensity;

				// --- transform it
				if (modTransform == kMMTransformBipolar)
					bipolarXForm(modSourceValue);
				else if (modTransform == kMMTransformUnipolar)
					unipolarXForm(modSourceValue);

				// --- without channel intensity
				if (destination.channelHardwire[row])
					modDestinationValue += modSourceValue*destination.hardwireIntensity[row];
				else
					modDestinationValue += destination.intensity*modSourceValue;
			}

			// --- write to the output array
			if (modDestinationData[col])
				*modDestinationData[col] = modDestinationValue;
		}
	}
} // namespace

