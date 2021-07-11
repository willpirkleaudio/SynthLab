#pragma once

#include "synthbase.h"
#include "synthfunctions.h"
#include "ksocore.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   ksoscillator.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class KSOscillator
	\ingroup SynthModules
	\brief
	Physical modeling oscillator fearuring Karplus-Strong plucked string algorithm
	- loads and supports up to NUM_MODULE_CORES (4) cores
	- it is mainly a wrapper/container for the module cores that do the acutal processing/rendering.

	The cores include (zero-indexed):
	0. KSOCore the single KS algorithm core for this module
	1. --- EMPTY ---
	2. --- EMPTY ---
	3. --- EMPTY ---

	Base Class: SynthModule
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.

	Databases: None

	GUI Parameters: KSOscParameters
	- getParameters() function allows direct access to std::shared_ptr<KSOscParameters>

	std::shared_ptr<KSOscParameters> getParameters()

	- call the getParameters() function
	- set the parameters in the PCMOscParameters structure with new values, typically from a GUI

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
	- Modulation Input values (modulators)

	Writes:
	- AudioBuffer Output samples

	Construction:

	(1) For use within a synth project, the constructor
	is specialized to use shared recources for:
	- MidiInputData
	- KSOscParameters

	The owning object (SynthVoice for the SynthLab projects) must pass these valid pointers
	to the object at construction time. Typically the engine or voice will be the primary synthesizers
	of these resources. See the 2nd Edition Synth Book for more information.

	(2) Standalone:

	To use in standalone mode, call the constructor with the shared resoure pointers as null:

	KSscillator(nullptr, nullptr, 64);

	In standalone mode, the object creates and maintains these resources:
	- MidiInputData: this has no read access so you cannot access its data. Ordinarily, you
	pass this shared pointer into the object so that it has access to the current MIDI input data.
	The object does not write data into this resource, so it is functionally non-opeational.

	- KSOscParameters: in standalone mode only, these are synthesized locally on the object,
	and then the owning object may obtain a shared pointer to them to read/write the parameters directly.

	Render:
	- renders into its own AudioBuffers object; see SynthModule::getAudioBuffers()
	- renders stereo by default; for mono samples, the output is dual-mono.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class KSOscillator : public SynthModule
	{
	public:
		// --- constructor/destructor
		KSOscillator(std::shared_ptr<MidiInputData> _midiInputData,
			std::shared_ptr<KSOscParameters> _parameters,
			uint32_t blockSize = 32);

		virtual ~KSOscillator() {}/* D-TOR */

		// --- SynthModule
		virtual bool reset(double _sampleRate);
		virtual bool update();
		virtual bool render(uint32_t samplesToProcess = 1);
		virtual bool doNoteOn(MIDINoteEvent& noteEvent);
		virtual bool doNoteOff(MIDINoteEvent& noteEvent);

	protected:
		// --- parameters
		std::shared_ptr<KSOscParameters> parameters = nullptr;
	};


}
