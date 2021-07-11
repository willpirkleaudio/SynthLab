#pragma once

#include "synthbase.h"
#include "guiconstants.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   modmatrix.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class ModMatrix
	\ingroup SynthObjects
	\brief 
	Very customizable modulation matrix object
	- has no base class
	- implements a modulation matrix using row/column paradigm 
	- See the ModMatrixParameters structure also

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class ModMatrix
	{
	public:
		/** default constructor */
		ModMatrix(std::shared_ptr<ModMatrixParameters> _parameters);
		
		/** empty destructor */
		virtual ~ModMatrix() {}/* D-TOR */

		// --- mod-matrix specific functions see .cpp implementation
		void addModSource(uint32_t sourceArrayIndex, double* sourceModPtr);
		void clearModSource(uint32_t sourceArrayIndex);
		void addModDestination(uint32_t destArrayIndex, double* destModPtr, uint32_t transform = kNoMMTransform);
		void clearModDestination(uint32_t destArrayIndex);
		void clearModMatrixArrays();
		void runModMatrix();

		/** for standalone operation only */
		std::shared_ptr<ModMatrixParameters> getParameters() { return parameters; }

	protected:
		// --- parameters
		std::shared_ptr<ModMatrixParameters> parameters = nullptr;

		// --- arrays to hold source/destination
		double* modSourceData[kNumberModSources];
		double* modDestinationData[kNumberModDestinations];
		uint32_t modDestTransform[kNumberModDestinations];

	};

}
