#ifndef __vaFilters_h__
#define __vaFilters_h__

// --- includes
#include "synthbase.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   vafilters.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{



	// --- makes all filter outputs: LPF1, LPF1A, HPF1, APF1
	class VA1Filter : public IFilterBase
	{
	public:
		// --- constructor/destructor
		VA1Filter();
		virtual ~VA1Filter() {}

		// --- these match SynthModule names
		virtual bool reset(double _sampleRate);
		virtual bool update();
		virtual FilterOutput* process(double xn); 
		virtual void setFilterParams(double _fc, double _Q);

		// --- set coeffs directly, bypassing coeff calculation
		void setAlpha(double _alpha) { coeffs.alpha = _alpha; }
		void setBeta(double _beta) { coeffs.beta = _beta; }
		void setCoeffs(VA1Coeffs& _coeffs) {
			coeffs = _coeffs;
		}

		void copyCoeffs(VA1Filter& destination) { 
			destination.setCoeffs(coeffs);
		}
		
		// --- added for MOOG & K35, need access to this output value, scaled by beta
		double getFBOutput() { return coeffs.beta * sn; }

	protected:
		FilterOutput output;
		double sampleRate = 44100.0;				///< current sample rate
		double halfSamplePeriod = 1.0;
		double fc = 0.0;

		// --- state storage
		double sn = 0.0;						///< state variables

		// --- filter coefficients
		VA1Coeffs coeffs;
	};

	struct VASVFCoeffs
	{
		// --- filter coefficients
		double alpha = 0.0;			///< alpha is (wcT/2)
		double rho = 1.0;			///< beta value, not used
		double sigma = 1.0;			///< beta value, not used
		double alpha0 = 1.0;			///< beta value, not used
	};

	// --- makes all filter outputs: LPF1, LPF1A, HPF1, APF1
	class VASVFilter : public IFilterBase
	{
	public:
		// --- constructor/destructor
		VASVFilter();
		virtual ~VASVFilter() {}

		// --- these match SynthModule names
		virtual bool reset(double _sampleRate);
		virtual bool update();
		virtual FilterOutput* process(double xn); 
		virtual void setFilterParams(double _fc, double _Q);

		// --- set coeffs directly, bypassing coeff calculation
		void setCoeffs(VASVFCoeffs& _coeffs) {
			coeffs = _coeffs;
		}

		void copyCoeffs(VASVFilter& destination) {
			destination.setCoeffs(coeffs);
		}

	protected:
		FilterOutput output;
		double sampleRate = 44100.0;				///< current sample rate
		double halfSamplePeriod = 1.0;
		double fc = 0.0;
		double Q = 0.0;

		// --- state storage
		double integrator_z[2];						///< state variables

		// --- filter coefficients
		VASVFCoeffs coeffs;
	};

	struct VAKorg35Coeffs
	{
		// --- filter coefficients
		double K = 1.0;			///< beta value, not used
		double alpha = 0.0;			///< alpha is (wcT/2)
		double alpha0 = 1.0;			///< beta value, not used
		double g = 1.0;			///< beta value, not used
	};

	// --- makes both LPF and HPF (double filter)
	class VAKorg35Filter : public IFilterBase
	{
	public:
		// --- constructor/destructor
		VAKorg35Filter();
		virtual ~VAKorg35Filter() {}

		// --- these match SynthModule names
		virtual bool reset(double _sampleRate);
		virtual bool update();
		virtual FilterOutput* process(double xn); 
		virtual void setFilterParams(double _fc, double _Q);

		// --- set coeffs directly, bypassing coeff calculation
		void setCoeffs(VAKorg35Coeffs& _coeffs) {
			coeffs = _coeffs;

			// --- three sync-tuned filters
			for (uint32_t i = 0; i < KORG_SUBFILTERS; i++)
			{
				lpfVAFilters[i].setAlpha(coeffs.alpha);
				hpfVAFilters[i].setAlpha(coeffs.alpha);
			}

			// --- set filter beta values
			double deno = 1.0 + coeffs.g;

			lpfVAFilters[FLT2].setBeta((coeffs.K * (1.0 - coeffs.alpha)) / deno);
			lpfVAFilters[FLT3].setBeta(-1.0 / deno);

			hpfVAFilters[FLT2].setBeta(-coeffs.alpha / deno);
			hpfVAFilters[FLT3].setBeta(1.0 / deno);
		//	hpfVAFilters[FLT3].setBeta(lpfVAFilters[FLT3].getBeta);
		}

		void copyCoeffs(VAKorg35Filter& destination) {
			destination.setCoeffs(coeffs);
		}

	protected:
		FilterOutput output;
		VA1Filter lpfVAFilters[KORG_SUBFILTERS];
		VA1Filter hpfVAFilters[KORG_SUBFILTERS];
		double sampleRate = 44100.0;				///< current sample rate
		double halfSamplePeriod = 1.0;
		double fc = 0.0;

		// --- filter coefficients
		VAKorg35Coeffs coeffs;

		//double K = 0.0;
		//double alpha = 0.0;			///< alpha is (wcT/2)
		//double alpha0 = 0.0;		///< input scalar, correct delay-free loop
	};

	struct VAMoogCoeffs
	{
		// --- filter coefficients
		double K = 1.0;			///< beta value, not used
		double alpha = 0.0;			///< alpha is (wcT/2)
		double alpha0 = 1.0;			///< beta value, not used
		double sigma = 1.0;			///< beta value, not used
		double bassComp = 1.0;			///< beta value, not used
		double g = 1.0;			///< beta value, not used

		// --- these are to minimize repeat calculations for left/right pairs
		double subFilterBeta[MOOG_SUBFILTERS] = { 0.0, 0.0, 0.0, 0.0 };
	};

	// --- makes both LPF and HPF (double filter)
	class VAMoogFilter : public IFilterBase
	{
	public:
		// --- constructor/destructor
		VAMoogFilter();
		virtual ~VAMoogFilter() {}

		// --- these match SynthModule names
		virtual bool reset(double _sampleRate);
		virtual bool update();
		virtual FilterOutput* process(double xn); 
		virtual void setFilterParams(double _fc, double _Q);

		// --- set coeffs directly, bypassing coeff calculation
		void setCoeffs(const VAMoogCoeffs& _coeffs) {
			coeffs = _coeffs;

			// --- four sync-tuned filters
			for (uint32_t i = 0; i < MOOG_SUBFILTERS; i++)
			{
				// --- set alpha directly
				subFilter[i].setAlpha(coeffs.alpha);
				subFilterFGN[i].setAlpha(coeffs.alpha);

				// --- set beta directly
				subFilter[i].setBeta(coeffs.subFilterBeta[i]);
				subFilterFGN[i].setBeta(coeffs.subFilterBeta[i]);
			}
		}

		void copyCoeffs(VAMoogFilter& destination) {
			destination.setCoeffs(coeffs);
		}

	protected:
		FilterOutput output;
		VA1Filter subFilter[MOOG_SUBFILTERS];
		VA1Filter subFilterFGN[MOOG_SUBFILTERS];
		double sampleRate = 44100.0;				///< current sample rate
		double halfSamplePeriod = 1.0;
		double fc = 0.0;

		// --- filter coefficients
		VAMoogCoeffs coeffs;
	};

	struct DiodeVA1Coeffs
	{
		// --- filter coefficients
		double alpha0 = 0.0;		///< input scalar, correct delay-free loop
		double alpha = 0.0;			///< alpha is (wcT/2)
		double beta = 1.0;			///< beta value, not used
		double gamma = 1.0;			///< beta value, not used
		double delta = 1.0;			///< beta value, not used
		double epsilon = 1.0;		///< beta value, not used
	};

	class VADiodeSubFilter : public IFilterBase
	{
	public:
		// --- constructor/destructor
		VADiodeSubFilter();
		virtual ~VADiodeSubFilter() {}

		// --- these match SynthModule names
		virtual bool reset(double _sampleRate);
		virtual bool update();
		virtual FilterOutput* process(double xn); 
		virtual void setFilterParams(double _fc, double _Q);

		void setCoeffs(const DiodeVA1Coeffs& _coeffs) { coeffs = _coeffs; }
		void copyCoeffs(VADiodeSubFilter& destination) {
			destination.setCoeffs(coeffs);
		}

		void setFBInput(double _feedbackIn) { feedbackIn = _feedbackIn; }
		double getFBOutput() { return coeffs.beta * (sn + feedbackIn*coeffs.delta); }

	protected:
		DiodeVA1Coeffs coeffs;
		FilterOutput output;
		double sampleRate = 44100.0;				///< current sample rate
		double halfSamplePeriod = 1.0;
		double fc = 0.0;

		// --- state storage
		double sn = 0.0;						///< state variables
		double feedbackIn = 0.0;
	};
	
	struct VADiodeCoeffs
	{
		// --- filter coefficients

		double alpha0 = 0.0;		///< input scalar, correct delay-free loop
		double gamma = 0.0;		///< input scalar, correct delay-free loop
		double beta[4] = { 0.0, 0.0, 0.0, 0.0 };
		DiodeVA1Coeffs diodeCoeffs[4];
		double bassComp = 0.0;		// --- increase for MORE bass
		double alpha1 = 1.0;		// --- FGN amp correction
		double K = 1.0;		// --- 
	};

	// --- makes both LPF and HPF (double filter)
	class VADiodeFilter
	{
	public:
		// --- constructor/destructor
		VADiodeFilter();
		virtual ~VADiodeFilter() {}

		// --- these match SynthModule names
		virtual bool reset(double _sampleRate);
		virtual bool update();
		virtual FilterOutput* process(double xn); 
		virtual void setFilterParams(double _fc, double _Q);
	
		void setCoeffs(const VADiodeCoeffs& _coeffs) {
			coeffs = _coeffs; 

			// --- update subfilter coeffs
			for (uint32_t i = 0; i < DIODE_SUBFILTERS; i++)
			{
				subFilter[i].setCoeffs(coeffs.diodeCoeffs[i]);
				subFilterFGN[i].setCoeffs(coeffs.diodeCoeffs[i]);
			}
		}

		void copyCoeffs(VADiodeFilter& destination) {
			destination.setCoeffs(coeffs);
		}

	protected:
		FilterOutput output;
		VADiodeSubFilter subFilter[DIODE_SUBFILTERS];
		VADiodeSubFilter subFilterFGN[DIODE_SUBFILTERS];

		double sampleRate = 44100.0;				///< current sample rate
		double halfSamplePeriod = 1.0;
		double fc = 0.0;

		// --- filter coefficients
		VADiodeCoeffs coeffs;
	};

}

#endif /* defined(__vaFilters_h__) */
