#pragma once

#include "synthbase.h"
#include "synthfunctions.h"
#include "wtoscillator.h"

// -----------------------------
//	--- SynthLab SDK File --- // 
//  ----------------------------
/**
\file   wsoscillator.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
namespace SynthLab
{
	/**
	\class WSOscillator
	\ingroup SynthModules
	\brief
	This is an aggregate of 4 wavetable oscillators being controlled with the single object
	that appears as an oscillator to the voice objecgt
	- demonstrates grouping objects together while still safely sharing data
	- demonstrates modifying and calling functions on these objects
	- does NOT implement the ModuleCores - this is a single object that is self-contained

	Base Class: SynthModule
	- Overrides the five (5) common functions plus a special getParameters() method to
	return a shared pointer to the parameters structure.

	Databases: WavetableDatabase
	- Accesses and holds shared pointer to WavetableDatabase
	- passes this database to internal oscillators which then registers theirwavetables with the database

	GUI Parameters: WTOscParameters
	- getParameters() function allows direct access to std::shared_ptr<WTOscParameters>

	std::shared_ptr<WTOscParameters> getParameters()

	- call the getParameters() function
	- set the parameters in the WTOscParameters structure with new values, typically from a GUI
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
	- WTOscParameters
	- WavetableDatabase

	The owning object (SynthVoice for the SynthLab projects) must pass these valid pointers
	to the object at construction time. Typically the engine or voice will be the primary synthesizers
	of these resources. See the 2nd Edition Synth Book for more information.

	(2) Standalone:

	To use in standalone mode, call the constructor with the shared resoure pointers as null:

	WTOscillator(nullptr, nullptr, nullptr, 64);

	In standalone mode, the object creates and maintains these resources:
	- MidiInputData: this has no read access so you cannot access its data. Ordinarily, you
	pass this shared pointer into the object so that it has access to the current MIDI input data.
	The object does not write data into this resource, so it is functionally non-opeational.

	- WavetableDatabase: this has no read access so you cannot access its data. Ordinarily, you
	pass this shared pointer into the object so that it can share these resources with other like-objects.
	However, in standalone operation the object will create the database and then register its wavetables
	with the object and use this local database for rendering operations. The code is the same for all operations
	so the object does not know if it is using a local database or a shared one.

	- WTOscParameters: in standalone mode only, these are synthesized locally on the object,
	and then the owning object may obtain a shared pointer to them to read/write the parameters directly.

	Render:
	- renders into its own AudioBuffers object; see SynthModule::getAudioBuffers()
	- renders stereo by default. For wavetables this really means dual-mono where the
	left buffer is copied to the right.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included and described in further detail in
	Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2021 / 04 / 26
	*/
	class WSOscillator : public SynthModule
	{
	public:
		/** One and only specialized constructor; pointers may be null for stanalone */
		WSOscillator(std::shared_ptr<MidiInputData> _midiInputData,
			std::shared_ptr<WSOscParameters> _parameters,
			std::shared_ptr<WavetableDatabase> _waveTableDatabase,
			uint32_t blockSize = 64);

		/** Destructor is empty: all resources are smart pointers */
		virtual ~WSOscillator() {}/* D-TOR */

		/** SynthModule Overrides */
		virtual bool reset(double _sampleRate) override;
		virtual bool update() override;
		virtual bool render(uint32_t samplesToProcess = 1) override;
		virtual bool doNoteOn(MIDINoteEvent& noteEvent) override;
		virtual bool doNoteOff(MIDINoteEvent& noteEvent) override;
		virtual bool startGlideModulation(GlideInfo& glideInfo) override;
		
		/** functions for dealing with multiple oscillators */
		void makeWaveStringMap();
		void mixOscBuffers(std::shared_ptr<AudioBuffer> oscBuffers, uint32_t samplesInBlock, double scaling);
		
		/** functions for setting or swapping oscillators */
		bool oscIsFree(uint32_t oscIndex, uint32_t waveAIndex, uint32_t waveBIndex);
		bool oscHasWaveIndex(uint32_t oscIndex, uint32_t waveIndex);
		void setNewOscWaveA(uint32_t oscIndex, uint32_t waveAIndex, double oscAMixCoeff);
		void setNewOscWaveB(uint32_t oscIndex, uint32_t waveBIndex, double oscBMixCoeff);
	
		/** functions for accessing oscillators and modulation inputs */
		std::shared_ptr<Modulators> getWSOscModulationInput(uint32_t oscIndex);
		std::shared_ptr<WTOscillator> getWTOscillator(uint32_t index);
	
		/** part of update() phase; this is only for the two oscillators that may be playing */
		void updateActiveOscillators();

	protected:
		// --- parameters
		std::shared_ptr<WSOscParameters> parameters = nullptr;
		
		// --- four WT Oscillators
		std::shared_ptr<WTOscillator> waveSeqOsc[NUM_WS_OSCILLATORS] = { nullptr, nullptr, nullptr, nullptr };
		std::shared_ptr<WTOscParameters> waveSeqParams[NUM_WS_OSCILLATORS] = { nullptr, nullptr, nullptr, nullptr };

		// --- stores the active pair of osc indexes
		uint32_t activeOsc[2] = { 0, 1 }; ///< active pair
		int32_t currSoloWave = -1; // -1 = nothing is being soloed

		// --- mising coefficients for the oscillators
		double oscMixCoeff[NUM_WS_OSCILLATORS] = { 0.0, 0.0, 0.0, 0.0 };
		
		// --- initializer for one-time setup
		bool initRoundRobin = true;

		// --- for connecting waveform strings to the oscillator cores that implement them
		std::vector<WaveStringData> waveStringFinder;
	};


}
