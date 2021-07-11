#pragma once

#include "synthbase.h"
#include "synthfunctions.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   noiseoscillator.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class NoiseOscillator
	\ingroup SynthModules
	\brief
	Noise oscillator module.
	- CAN load and support up to NUM_MODULE_CORES (4) cores
	- there are NO cores currently defined as the object is so simple
	- this is an example of a SynthModule that implements its functionality
	DIRECTLY and without ModuleCores

	Base Class: SynthModule
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.

	Databases: None

	GUI Parameters: NoiseOscillatorParameters
	- getParameters() function allows direct access to std::shared_ptr<NoiseOscillatorParameters>

	std::shared_ptr<NoiseOscillatorParameters> getParameters()

	- call the getParameters() function
	- set the parameters in the NoiseOscillatorParameters structure with new values, typically from a GUI
	To apply these new parameters either:
	- (a) call the module's update() function OR
	- (b) call the render() function which in turn calls the update() method.

	Ordinarily, this operation happens just prior to calling the render() function so that is the preferred
	method of operation to avoid multiple calls to the update() function, which is usually the most CPU intensive
	function of the SynthModule.

	Access to Modulators
	- std::shared_ptr<Modulators> getModulationInput()
	- std::shared_ptr<Modulators> getModulationOutput()

	Access to audio buffers (I/O)
	- std::shared_ptr<AudioBuffer> getAudioBuffers()

	Reads:
	- Modulation Input Values (modulators)

	Writes:
	- AudioBuffer Output samples

	Construction:

	(1) For use within a synth project, the constructor
	is specialized to use shared recources for:
	- MidiInputData
	- NoiseOscillatorParameters

	The owning object (SynthVoice for the SynthLab projects) must pass these valid pointers
	to the object at construction time. Typically the engine or voice will be the primary synthesizers
	of these resources. See the 2nd Edition Synth Book for more information.

	(2) Standalone:

	To use in standalone mode, call the constructor with the shared resoure pointers as null:

	NoiseOscillator(nullptr, nullptr, 64);

	In standalone mode, the object creates and maintains these resources:
	- MidiInputData: this has no read access so you cannot access its data. Ordinarily, you
	pass this shared pointer into the object so that it has access to the current MIDI input data.
	The object does not write data into this resource, so it is functionally non-opeational.

	- NoiseOscillatorParameters: in standalone mode only, these are synthesized locally on the object,
	and then the owning object may obtain a shared pointer to them to read/write the parameters directly.

	Render:
	- renders into its own AudioBuffers object; see SynthModule::getAudioBuffers()
	- renders stereo by default

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class NoiseOscillator : public SynthModule
	{
	public:
		/** One and only specialized constructor; pointers may be null for stanalone */
		NoiseOscillator(std::shared_ptr<MidiInputData> _midiInputData,
			std::shared_ptr<NoiseOscillatorParameters> _parameters,
			uint32_t blockSize = 32);
		
		/** Destructor is empty: all resources are smart pointers */
		virtual ~NoiseOscillator() {}/* D-TOR */

		/** SynthModule Overrides */
		virtual bool reset(double _sampleRate) override;
		virtual bool update() override;
		virtual bool render(uint32_t samplesToProcess = 1) override;
		virtual bool doNoteOn(MIDINoteEvent& noteEvent) override;
		virtual bool doNoteOff(MIDINoteEvent& noteEvent) override;

		/** For standalone operation only; not used in SynthLab synth projects */
		std::shared_ptr<NoiseOscillatorParameters> getParameters() { return parameters; }

	protected:
		/** For standalone operation only; not used in SynthLab synth projects */
		std::shared_ptr<NoiseOscillatorParameters> parameters = nullptr;

		// --- sample rate
		double sampleRate = 0.0;			///< sample rate

		// --- output scalar
		double outputAmplitude = 0.0;		///< raw value 

		// --- timebase
		SynthClock synthClock;				///< timebase

		// --- the noise generator is general purpose; here used as the core
		NoiseGenerator noiseGen;			///< the actual generator
	};


} // namespace