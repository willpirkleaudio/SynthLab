#pragma once

#include "synthbase.h"
#include "synthfunctions.h"
#include "pcmlegacycore.h"
#include "mellotroncore.h"
#include "waveslicecore.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   pcmoscillator.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class PCMOscillator
	\ingroup SynthModules
	\brief
	PCM sample based oscillator module.
	- loads and supports up to NUM_MODULE_CORES (4) cores
	- it is mainly a wrapper/container for the module cores that do the acutal processing/rendering.

	The cores are specialized and parse their PCM samples from a set of WAV files that must
	be present in the plugin folder (this location may be changed based on your own preferences). 
	The cores demonstrate the variations as documented in Designing Software Synthsizers in C++ 2nd Ed. by Will Pirkle.

	The cores include (zero-indexed):
	0. LegacyPCMCore PCM samples from the first edition of Will Pirkle's synth book
	1. MellotronCore free (and long) samples from Mellotron tape-based synth
	2. WaveSliceCore uses slices of waveforms that are made with the aubio software tools
	(or any other slicing app). 
	3. --- EMPTY --- (note: there is an unused drumloops folder in the SynthLabSamples directory - use it!)

	Base Class: SynthModule
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.

	Databases: PCMSampleDatabase
	- Accesses and holds shared pointer to PCMSampleDatabase
	- registers its samples with the database
	- recalls samples from the database during the update() function to use for rendering

	GUI Parameters: PCMOscParameters
	- getParameters() function allows direct access to std::shared_ptr<PCMOscParameters>

	std::shared_ptr<PCMOscParameters> getParameters()

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
	- Modulation Input Values (modulators)

	Writes:
	- AudioBuffer Output samples

	Construction:

	(1) For use within a synth project, the constructor
	is specialized to use shared recources for:
	- MidiInputData
	- PCMOscParameters
	- PCMSampleDatabase

	The owning object (SynthVoice for the SynthLab projects) must pass these valid pointers
	to the object at construction time. Typically the engine or voice will be the primary synthesizers
	of these resources. See the 2nd Edition Synth Book for more information.

	(2) Standalone:

	To use in standalone mode, call the constructor with the shared resoure pointers as null:

	PCMOscillator(nullptr, nullptr, nullptr, 64);

	In standalone mode, the object creates and maintains these resources:
	- MidiInputData: this has no read access so you cannot access its data. Ordinarily, you
	pass this shared pointer into the object so that it has access to the current MIDI input data.
	The object does not write data into this resource, so it is functionally non-opeational.

	- PCMSampleDatabase: this has no read access so you cannot access its data. Ordinarily, you
	pass this shared pointer into the object so that it can share these resources with other like-objects.
	However, in standalone operation the object will create the database and then register its samples
	with the object and use this local database for rendering operations. The code is the same for all operations
	so the object does not know if it is using a local database or a shared one.

	- PCMOscParameters: in standalone mode only, these are synthesized locally on the object, 
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
	class PCMOscillator : public SynthModule
	{
	public:
		/** One and only specialized constructor; pointers may be null for stanalone */
		PCMOscillator(std::shared_ptr<MidiInputData> _midiInputData,
			std::shared_ptr<PCMOscParameters> _parameters,
			std::shared_ptr<PCMSampleDatabase> _sampleDatabase,
			uint32_t blockSize = 64);

		/** Destructor is empty: all resources are smart pointers */
		virtual ~PCMOscillator() {}/* D-TOR */

		/** SynthModule Overrides */
		virtual bool reset(double _sampleRate) override;
		virtual bool update() override;
		virtual bool render(uint32_t samplesToProcess = 1) override;
		virtual bool doNoteOn(MIDINoteEvent& noteEvent) override;
		virtual bool doNoteOff(MIDINoteEvent& noteEvent) override;

		/** For standalone operation only; not used in SynthLab synth projects */
		std::shared_ptr<PCMOscParameters> getParameters() { return parameters; }

	protected:
		/** For standalone operation only; not used in SynthLab synth projects */
		std::shared_ptr<PCMOscParameters> parameters = nullptr;

		/** For standalone operation only; not used in SynthLab synth projects */
		std::shared_ptr<PCMSampleDatabase> sampleDatabase;
	};


}
