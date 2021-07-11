// --- Synth Core v1.0
//
#include "vafilters.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   vafilters.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	VA1Filter::VA1Filter(){ }

	/** reset members to initialized state */
	bool VA1Filter::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		halfSamplePeriod = 1.0 / (2.0 * sampleRate);
		sn = 0.0;
		output.clearData();
		return true;
	}

	void VA1Filter::setFilterParams(double _fc, double _Q)
	{
		if (fc != _fc)
		{
			fc = _fc;
			update();
		}
	}

	bool VA1Filter::update()
	{
		double g = tan(kTwoPi*fc*halfSamplePeriod); // (2.0 / T)*tan(wd*T / 2.0);
		coeffs.alpha = g / (1.0 + g);

		return true;
	}

	FilterOutput* VA1Filter::process(double xn)
	{
		// --- create vn node
		double vn = (xn - sn)*coeffs.alpha;

		// --- form LP output
		output.filter[LPF1] = ((xn - sn)*coeffs.alpha) + sn;

		// --- form the HPF = INPUT = LPF
		output.filter[HPF1] = xn - output.filter[LPF1];

		// --- form the APF = LPF - HPF
		output.filter[APF1] = output.filter[LPF1] - output.filter[HPF1];

		// --- anm output
		output.filter[ANM_LPF1] = output.filter[LPF1] + coeffs.alpha*output.filter[HPF1];

		// --- update memory
		sn = vn + output.filter[LPF1];

		return &output;
	}


	VASVFilter::VASVFilter() {}

	/** reset members to initialized state */
	bool VASVFilter::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		halfSamplePeriod = 1.0 / (2.0 * sampleRate);
		integrator_z[0] = 0.0;
		integrator_z[1] = 0.0;
		output.clearData();

		return true;
	}

	void VASVFilter::setFilterParams(double _fc, double _Q)
	{
		// --- use mapping function for Q -> K
		mapDoubleValue(_Q, 1.0, 0.707, SVF_Q_SLOPE);

		if (fc != _fc || Q != _Q)
		{
			fc = _fc;
			Q = _Q;
			update();
		}
	}

	bool VASVFilter::update()
	{
		// --- prewarp the cutoff- these are bilinear-transform filters
		coeffs.alpha = tan(kTwoPi*fc*halfSamplePeriod);

		// --- note R is the traditional analog damping factor zeta
		double R = 1.0 / (2.0*Q);
		coeffs.alpha0 = 1.0 / (1.0 + 2.0*R*coeffs.alpha + coeffs.alpha*coeffs.alpha);

		// --- for max Q, go to self-oscillation (HOMEWORK!)
		if (Q > 24.9)
			coeffs.rho = coeffs.alpha;
		else
			coeffs.rho = 2.0*R + coeffs.alpha;

		// --- sigma for analog matching version
		double theta_c = (sampleRate / 2.0) / fc;
		coeffs.sigma = 1.0 / (coeffs.alpha*theta_c*theta_c);
	
		return true;
	}

	FilterOutput* VASVFilter::process(double xn)
	{
		// --- form the HP output first
		output.filter[HPF2] = coeffs.alpha0*(xn - coeffs.rho*integrator_z[0] - integrator_z[1]);

		// --- BPF Out
		output.filter[BPF2] = coeffs.alpha*output.filter[HPF2] + integrator_z[0];

		// --- LPF Out
		output.filter[LPF2] = coeffs.alpha*output.filter[BPF2] + integrator_z[1];

		// --- BSF Out
		output.filter[BSF2] = output.filter[HPF2] + output.filter[LPF2];

		// --- finite gain at Nyquist; slight error at VHF
		double sn = integrator_z[0];

		output.filter[ANM_LPF2] = output.filter[LPF2] + coeffs.sigma*(sn);

		// update memory
		integrator_z[0] = coeffs.alpha*output.filter[HPF2] + output.filter[BPF2];
		integrator_z[1] = coeffs.alpha*output.filter[BPF2] + output.filter[LPF2];

		return &output;
	}


	VAKorg35Filter::VAKorg35Filter() {}

	/** reset members to initialized state */
	bool VAKorg35Filter::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		halfSamplePeriod = 1.0 / (2.0 * sampleRate);

		for (uint32_t i = 0; i < KORG_SUBFILTERS; i++)
		{
			lpfVAFilters[i].reset(_sampleRate);
			hpfVAFilters[i].reset(_sampleRate);
		}

		output.clearData();
		return true;
	}

	void VAKorg35Filter::setFilterParams(double _fc, double _Q)
	{
		// --- use mapping function for Q -> K
		mapDoubleValue(_Q, 1.0, 0.707, KORG35_Q_SLOPE);

		if (fc != _fc || coeffs.K != _Q)
		{
			fc = _fc;
			coeffs.K = _Q;
			update();
		}
	}

	bool VAKorg35Filter::update()
	{
		coeffs.g = tan(kTwoPi*fc*halfSamplePeriod); // (2.0 / T)*tan(wd*T / 2.0);
		coeffs.alpha = coeffs.g / (1.0 + coeffs.g);

		// --- alpha0 same for LPF, HPF
		coeffs.alpha0 = 1.0 / (1.0 - coeffs.K*coeffs.alpha + coeffs.K*coeffs.alpha*coeffs.alpha);

		// --- three sync-tuned filters
		for (uint32_t i = 0; i < KORG_SUBFILTERS; i++)
		{
			lpfVAFilters[i].setAlpha(coeffs.alpha);
			hpfVAFilters[i].setAlpha(coeffs.alpha);
		}

		// --- set filter beta values
		lpfVAFilters[FLT2].setBeta((coeffs.K * (1.0 - coeffs.alpha)) / (1.0 + coeffs.g));
		lpfVAFilters[FLT3].setBeta(-1.0 / (1.0 + coeffs.g));

		hpfVAFilters[FLT2].setBeta(-coeffs.alpha / (1.0 + coeffs.g));
		hpfVAFilters[FLT3].setBeta(1.0 / (1.0 + coeffs.g));

		return true;
	}

	FilterOutput* VAKorg35Filter::process(double xn)
	{
		// --- lowpass
		// --- process input through LPF1
		FilterOutput* tempOut;
		tempOut = lpfVAFilters[FLT1].process(xn);

		// --- form S35
		double S35 = lpfVAFilters[FLT2].getFBOutput() + lpfVAFilters[FLT3].getFBOutput();

		// --- calculate u
		double u = coeffs.alpha0*(tempOut->filter[ANM_LPF1] + S35);

		// --- feed it to LPF2
		tempOut = lpfVAFilters[FLT2].process(u);

		// --- output = LPF*K
		output.filter[LPF2] = tempOut->filter[LPF1] * coeffs.K;
		output.filter[ANM_LPF2] = tempOut->filter[ANM_LPF1] * coeffs.K;

		// --- feed output to HPF, no need to gather it's output
		lpfVAFilters[FLT3].process(output.filter[LPF2]);

		// --- HIGHPASS: 
		// --- process input through HPF1
		tempOut = hpfVAFilters[FLT1].process(xn);

		// --- then: form feedback and feed forward values (read before write)
		S35 = hpfVAFilters[FLT2].getFBOutput() + hpfVAFilters[FLT3].getFBOutput();

		// --- calculate u
		u = coeffs.alpha0*(tempOut->filter[HPF1] + S35);

		//---  form HPF output
		output.filter[HPF2] = coeffs.K*u;

		// --- process through feedback path
		tempOut = hpfVAFilters[FLT2].process(output.filter[HPF2]);

		// --- continue to LPF, no need to gather it's output
		hpfVAFilters[FLT3].process(tempOut->filter[HPF1]);

		// auto-normalize
		if (coeffs.K > 0)
		{
			output.filter[ANM_LPF2] *= 1.0 / coeffs.K;
			output.filter[LPF2] *= 1.0 / coeffs.K;
			output.filter[HPF2] *= 1.0 / coeffs.K;
		}

		return &output;
	}

	VAMoogFilter::VAMoogFilter()
	{
	}

	/** reset members to initialized state */
	bool VAMoogFilter::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		halfSamplePeriod = 1.0 / (2.0 * sampleRate);

		for (uint32_t i = 0; i < MOOG_SUBFILTERS; i++)
		{
			subFilter[i].reset(_sampleRate);
			subFilterFGN[i].reset(_sampleRate);
		}

		output.clearData();
		return true;
	}

	void VAMoogFilter::setFilterParams(double _fc, double _Q)
	{
		// --- use mapping function for Q -> K
		mapDoubleValue(_Q, 1.0, 0.0, MOOG_Q_SLOPE);

		if (fc != _fc || coeffs.K != _Q)
		{
			fc = _fc;
			coeffs.K = _Q;
			update();
		}
	}

	bool VAMoogFilter::update()
	{
		coeffs.g = tan(kTwoPi*fc*halfSamplePeriod); // (2.0 / T)*tan(wd*T / 2.0);
		coeffs.alpha = coeffs.g / (1.0 + coeffs.g);

		// --- alpha0 
		coeffs.alpha0 = 1.0 / (1.0 + coeffs.K*coeffs.alpha*coeffs.alpha*coeffs.alpha*coeffs.alpha);
		double kernel = 1.0 / (1.0 + coeffs.g);

		// --- pre-calculate for distributing to subfilters
		coeffs.subFilterBeta[FLT4] = kernel;
		coeffs.subFilterBeta[FLT3] = coeffs.alpha * coeffs.subFilterBeta[FLT4];
		coeffs.subFilterBeta[FLT2] = coeffs.alpha * coeffs.subFilterBeta[FLT3];
		coeffs.subFilterBeta[FLT1] = coeffs.alpha * coeffs.subFilterBeta[FLT2];

		// --- four sync-tuned filters
		for (uint32_t i = 0; i < MOOG_SUBFILTERS; i++)
		{
			// --- set alpha - no calculation required
			subFilter[i].setAlpha(coeffs.alpha);
			subFilterFGN[i].setAlpha(coeffs.alpha);

			// --- set beta - no calculation required
			subFilter[i].setBeta(coeffs.subFilterBeta[i]);
			subFilterFGN[i].setBeta(coeffs.subFilterBeta[i]);
		}

		return true;
	}

	FilterOutput* VAMoogFilter::process(double xn)
	{
		// --- 4th order MOOG:
		double sigma = 0.0;

		for (uint32_t i = 0; i < MOOG_SUBFILTERS; i++)
			sigma += subFilter[i].getFBOutput();

		// --- gain comp 
		xn *= 1.0 + coeffs.bassComp*coeffs.K; // --- bassComp is hard coded

		// --- now figure out u(n) = alpha0*[x(n) - K*sigma]
		double u = coeffs.alpha0*(xn - coeffs.K * sigma);

		// --- send u -> LPF1 and then cascade the outputs to form y(n)
		FilterOutput* subFltOut[4];
		FilterOutput* subFltOutFGN[4];

		subFltOut[FLT1] = subFilter[FLT1].process(u);
		subFltOut[FLT2] = subFilter[FLT2].process(subFltOut[FLT1]->filter[LPF1]);
		subFltOut[FLT3] = subFilter[FLT3].process(subFltOut[FLT2]->filter[LPF1]);
		subFltOut[FLT4] = subFilter[FLT4].process(subFltOut[FLT3]->filter[LPF1]);

		// --- optional outputs 1,2,3
		output.filter[LPF1] = subFltOut[FLT1]->filter[LPF1];
		output.filter[LPF2] = subFltOut[FLT2]->filter[LPF1];
		output.filter[LPF3] = subFltOut[FLT3]->filter[LPF1];

		// --- MOOG LP4 output
		output.filter[LPF4] = subFltOut[FLT4]->filter[LPF1];

		// --- OPTIONAL: analog nyquist matched version
		subFltOutFGN[FLT1] = subFilterFGN[FLT1].process(u);
		subFltOutFGN[FLT2] = subFilterFGN[FLT2].process(subFltOutFGN[FLT1]->filter[ANM_LPF1]);
		subFltOutFGN[FLT3] = subFilterFGN[FLT3].process(subFltOutFGN[FLT2]->filter[ANM_LPF1]);
		subFltOutFGN[FLT4] = subFilterFGN[FLT4].process(subFltOutFGN[FLT3]->filter[ANM_LPF1]);

		// --- optional outputs 1,2,3
		output.filter[ANM_LPF1] = subFltOutFGN[FLT1]->filter[ANM_LPF1];
		output.filter[ANM_LPF2] = subFltOutFGN[FLT2]->filter[ANM_LPF1];
		output.filter[ANM_LPF3] = subFltOutFGN[FLT3]->filter[ANM_LPF1];

		// --- MOOG LP4 output
		output.filter[ANM_LPF4] = subFltOutFGN[FLT4]->filter[ANM_LPF1];

		return &output;
	}


	VADiodeSubFilter::VADiodeSubFilter()
	{
	}

	/** reset members to initialized state */
	bool VADiodeSubFilter::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		halfSamplePeriod = 1.0 / (2.0 * sampleRate);
		sn = 0.0;
		feedbackIn = 0.0;
		output.clearData();

		return true;
	}

	void VADiodeSubFilter::setFilterParams(double _fc, double _Q)
	{
		if (fc != _fc)
		{
			fc = _fc;
			update();
		}
	}

	bool VADiodeSubFilter::update()
	{
		double g = tan(kTwoPi*fc*halfSamplePeriod);
		coeffs.alpha = g / (1.0 + g);

		return true;
	}

	FilterOutput* VADiodeSubFilter::process(double xn)
	{
		// --- specific to diode
		xn = (xn * coeffs.gamma) + feedbackIn + (coeffs.epsilon * getFBOutput());

		// --- v(n)
		double vn = ((coeffs.alpha0 * xn) - sn) * coeffs.alpha;

		// --- form LP output
		output.filter[LPF1] = vn + sn;

		// --- update memory
		sn = vn + output.filter[LPF1];

		// --- do the HPF
		output.filter[HPF1] = xn - output.filter[LPF1];

		// --- will this work?
		output.filter[ANM_LPF1] = output.filter[LPF1] + coeffs.alpha * output.filter[HPF1];

		return &output;
	}

	VADiodeFilter::VADiodeFilter()
	{
		coeffs.diodeCoeffs[FLT1].alpha0 = 1.0;
		coeffs.diodeCoeffs[FLT2].alpha0 = 0.5;
		coeffs.diodeCoeffs[FLT3].alpha0 = 0.5;
		coeffs.diodeCoeffs[FLT4].alpha0 = 0.5;

		coeffs.diodeCoeffs[FLT4].delta = 0.0;
		coeffs.diodeCoeffs[FLT4].epsilon = 0.0;
		coeffs.beta[3] = 1.0;
	}


	/** reset members to initialized state */
	bool VADiodeFilter::reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		halfSamplePeriod = 1.0 / (2.0 * sampleRate);

		for (uint32_t i = 0; i < MOOG_SUBFILTERS; i++)
		{
			subFilter[i].reset(_sampleRate);
			subFilterFGN[i].reset(_sampleRate);
		}

		output.clearData();
		return true;
	}

	void VADiodeFilter::setFilterParams(double _fc, double _Q)
	{
		// --- use mapping function for Q -> K
		mapDoubleValue(_Q, 1.0, 0.0, DIODE_Q_SLOPE);

		if (fc != _fc || coeffs.K != _Q)
		{
			fc = _fc;
			coeffs.K = _Q;
			update();
		}
	}

	const double fgn_a = 1.005381;
	const double fgn_b = 0.8783896;
	const double fgn_c = 1.113067;
	const double fgn_d = -0.2110344;

	bool VADiodeFilter::update()
	{
		double g = tan(kTwoPi*fc*halfSamplePeriod);

		double G4 = 0.5*g / (1.0 + g);
		double G3 = 0.5*g / (1.0 + g - 0.5*g*G4);
		double G2 = 0.5*g / (1.0 + g - 0.5*g*G3);
		double G1 = g / (1.0 + g - g*G2);

		// --- our coeffs
		coeffs.gamma = G4*G3*G2*G1;
		coeffs.alpha0 = 1.0 / (1.0 + coeffs.K * coeffs.gamma);

		coeffs.beta[0] = G4*G3*G2;
		coeffs.beta[1] = G4*G3;
		coeffs.beta[2] = G4;

		// --- analog 
		double f = kTwoPi*fc / sampleRate;
		double e = pow((f / fgn_c), fgn_b);
		coeffs.alpha1 = fgn_d + (fgn_a - fgn_d) / (1.0 + e);

		// --- diode sub-filter coeffs
		// 
		// -- alphas
		double alpha = g / (1.0 + g);

		coeffs.diodeCoeffs[FLT1].alpha = alpha;
		coeffs.diodeCoeffs[FLT2].alpha = alpha;
		coeffs.diodeCoeffs[FLT3].alpha = alpha;
		coeffs.diodeCoeffs[FLT4].alpha = alpha;

		// --- betas
		coeffs.diodeCoeffs[FLT1].beta = 1.0 / (1.0 + g - g*G2);
		coeffs.diodeCoeffs[FLT2].beta = 1.0 / (1.0 + g - 0.5*g*G3);
		coeffs.diodeCoeffs[FLT3].beta = 1.0 / (1.0 + g - 0.5*g*G4);
		coeffs.diodeCoeffs[FLT4].beta = 1.0 / (1.0 + g);

		// --- gammas
		coeffs.diodeCoeffs[FLT1].gamma = 1.0 + G1*G2;
		coeffs.diodeCoeffs[FLT2].gamma = 1.0 + G2*G3;
		coeffs.diodeCoeffs[FLT3].gamma = 1.0 + G3*G4;
		coeffs.diodeCoeffs[FLT4].gamma = 1.0;

		// --- deltas
		coeffs.diodeCoeffs[FLT1].delta = g;
		coeffs.diodeCoeffs[FLT2].delta = 0.5*g;
		coeffs.diodeCoeffs[FLT3].delta = 0.5*g;

		// set epsilons
		coeffs.diodeCoeffs[FLT1].epsilon = G2;
		coeffs.diodeCoeffs[FLT2].epsilon = G3;
		coeffs.diodeCoeffs[FLT3].epsilon = G4;

		// --- update subfilter coeffs
		for (uint32_t i = 0; i < DIODE_SUBFILTERS; i++)
		{
			subFilter[i].setCoeffs(coeffs.diodeCoeffs[i]);
			subFilterFGN[i].setCoeffs(coeffs.diodeCoeffs[i]);
		}

		return true;
	}

	FilterOutput* VADiodeFilter::process(double xn) 
	{
		// --- must be processed in this order, with function calls in args
		subFilter[FLT4].setFBInput(0.0);
		subFilterFGN[FLT4].setFBInput(0.0);

		subFilter[FLT3].setFBInput(subFilter[FLT4].getFBOutput());
		subFilterFGN[FLT3].setFBInput(subFilter[FLT4].getFBOutput());

		subFilter[FLT2].setFBInput(subFilter[FLT3].getFBOutput());
		subFilterFGN[FLT2].setFBInput(subFilter[FLT3].getFBOutput());

		subFilter[FLT1].setFBInput(subFilter[FLT2].getFBOutput());
		subFilterFGN[FLT1].setFBInput(subFilter[FLT2].getFBOutput());

		// form input
		double sigma = coeffs.beta[0] * subFilter[FLT1].getFBOutput() +
			coeffs.beta[1] * subFilter[FLT2].getFBOutput() +
			coeffs.beta[2] * subFilter[FLT3].getFBOutput() +
			coeffs.beta[3] * subFilter[FLT4].getFBOutput();

		// --- gain comp 
		xn *= 1.0 + (coeffs.bassComp * coeffs.K); // --- bassComp is hard coded

		// form input
		double u = (xn - (coeffs.K * sigma)) * coeffs.alpha0;

		// cascade of four filters
		FilterOutput* subFltOut[4];
		FilterOutput* subFltOutFGN[4];

		subFltOut[FLT1] = subFilter[FLT1].process(u);
		subFltOut[FLT2] = subFilter[FLT2].process(subFltOut[FLT1]->filter[LPF1]);
		subFltOut[FLT3] = subFilter[FLT3].process(subFltOut[FLT2]->filter[LPF1]);
		subFltOut[FLT4] = subFilter[FLT4].process(subFltOut[FLT3]->filter[LPF1]);

		subFltOutFGN[FLT1] = subFilterFGN[FLT1].process(u);
		subFltOutFGN[FLT2] = subFilterFGN[FLT2].process(subFltOutFGN[FLT1]->filter[ANM_LPF1]);
		subFltOutFGN[FLT3] = subFilterFGN[FLT3].process(subFltOutFGN[FLT2]->filter[ANM_LPF1]);
		subFltOutFGN[FLT4] = subFilterFGN[FLT4].process(subFltOutFGN[FLT3]->filter[ANM_LPF1]);

		// --- output is last filter's LPF1 out
		output.filter[LPF4] = subFltOut[FLT4]->filter[LPF1];
		output.filter[ANM_LPF4] = coeffs.alpha1 * subFltOutFGN[FLT4]->filter[ANM_LPF1];

		return &output;
	}
}