#pragma once

#include "pluginstructures.h"
#include "../../examples/synthlab_examples/synthengine.h"


// --------------------------------------
//	--- OPTIONAL SynthLab SDK File --- // 
//  -------------------------------------
/**
\file   synthlabds.h
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------

//@{
/**
\ingroup Constants-Enums
These are custom parameter ID values for special GUI controls in ASPiK and RAFX that are used
for dynamic string loaded controls.
- This is not part of SynthLab. It is only for ASPiK and RAFX integration and can be ignored for any
other framework.

*/
const uint32_t CUSTOM_OPTION_MENU = 131072;
const uint32_t CUSTOM_KNOB_LABEL = 131073;
const uint32_t CUSTOM_KNOB_LABEL_INDEX = 131074;
const uint32_t TOTAL_CUSTOM_VIEWS = 64;
const uint32_t CUSTOM_BITS = 31;
//@}


/**
\ingroup DynamicStringStructures
Information about custom view data. Note that this is ASPiK-specific and not part of SynthLab
directly. You can use it to setup your own dynamic string loading GUI controls. 
- This is not part of SynthLab. It is only for ASPiK and RAFX integration and can be ignored for any
other framework.

*/
struct CustomViewInfo
{
	void clear()
	{
		droplistMask = 0;
		droplistControl_ASPiK = nullptr;
		droplistControl_RAFX = nullptr;
		modKnobMask = 0;
		controlStringCount = 0;

		for (uint32_t i = 0; i < 4; i++)
		{
			modKnobLabel_RAFX[i] = nullptr;
			modKnobLabel_ASPiK[i] = nullptr;
		}
	}

	// --- need to allow BOTH RackAFX and the custom GUI to store info here
	uint32_t droplistMask = 0;
	ICustomView* droplistControl_RAFX = nullptr;
	ICustomView* droplistControl_ASPiK = nullptr;
	moodycamel::ReaderWriterQueue<std::vector<std::string>, 2> droplistDataQueue;

	uint32_t modKnobMask = 0;
	ICustomView* modKnobLabel_RAFX[4] = { nullptr, nullptr, nullptr, nullptr };
	ICustomView* modKnobLabel_ASPiK[4] = { nullptr, nullptr, nullptr, nullptr };
	moodycamel::ReaderWriterQueue<std::string, 2> modKnobLabelDataQueue[4];

	// --- for drop-list type controls
	uint32_t controlStringCount = 0;
};

/**
\ingroup DynamicStringStructures
This contains specialized aux data for the ASPiK PluginParamter objects, which allow
you to store an unlimited amount of extra data along with the control. This data does NOT
affect the parameter operation whatsoever, and is only there for storing added information.
- This is not part of SynthLab. It is only for ASPiK and RAFX integration and can be ignored for any
other framework.

*/
struct moduleAuxParamSet
{
	moduleAuxParamSet(uint32_t _attValCoreMenu, uint32_t _attValWaveMenu, uint32_t _attValModKnobs,
		uint32_t _coreMenuCtrlID, uint32_t _waveMenuCtrlID,
		uint32_t _modKnob_A_CtrlID, uint32_t _modKnob_B_CtrlID,
		uint32_t _modKnob_C_CtrlID, uint32_t _modKnob_D_CtrlID)
		: attValCoreMenu(_attValCoreMenu)
		, attValWaveMenu(_attValWaveMenu)
		, attValModKnobs(_attValModKnobs)
		, coreMenuCtrlID(_coreMenuCtrlID)
		, waveMenuCtrlID(_waveMenuCtrlID)
		, modKnob_A_CtrlID(_modKnob_A_CtrlID)
		, modKnob_B_CtrlID(_modKnob_B_CtrlID)
		, modKnob_C_CtrlID(_modKnob_C_CtrlID)
		, modKnob_D_CtrlID(_modKnob_D_CtrlID) {}

	uint32_t attValCoreMenu = SynthLab::LFO1_SOURCE;
	uint32_t attValWaveMenu = SynthLab::LFO1_WAVEFORMS;
	uint32_t attValModKnobs = SynthLab::LFO1_MOD_KNOBS;

	uint32_t coreMenuCtrlID = 0;
	uint32_t waveMenuCtrlID = 0;
	uint32_t modKnob_A_CtrlID = 0;
	uint32_t modKnob_B_CtrlID = 0;
	uint32_t modKnob_C_CtrlID = 0;
	uint32_t modKnob_D_CtrlID = 0;
};

/**
\class DynamicStringManager
\ingroup DynamicStringObjects
\brief
Dynamic string manager - this object connects plugin parameters and their GUI controls with new
or different sets of strings to display. 
- This is not part of SynthLab. It is only for ASPiK and RAFX integration and can be ignored for any
other framework, or can be used to demonstrate how to maintain these controls. 

\author Will Pirkle http://www.willpirkle.com
\remark This object is included and described in further detail in
Designing Software Synthesizer Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2021 / 04 / 26
*/
class DynamicStringManager
{
public:
	/** Specialized string management constructor */
	DynamicStringManager(std::shared_ptr<SynthLab::SynthEngine> _synthEngine, void* _pluginCore);
	virtual ~DynamicStringManager() {}
	
	//@{
	/**
	\brief
	Functions for dealing with a custom update code uint32_t where each bit encodes a particular
	GUI control that needs updating. 
	*/
	bool setCustomUpdateCodes(uint32_t customUpdateCode_1, uint32_t customUpdateCode_2);
	bool isCustomViewDataQueueEnabled() const { return queueEnabler.load(std::memory_order_relaxed); }			///< set atomic variable with float
	void enableCustomViewDataQueue(bool value) { queueEnabler.store(value, std::memory_order_relaxed); }	///< get atomic variable as float
	bool haveCustomUpdates() const { return updateEnabler.load(std::memory_order_relaxed); }			///< set atomic variable with float
	void enableCustomUpdates(bool value) { updateEnabler.store(value, std::memory_order_relaxed); }	///< get atomic variable as float
	bool updateCustomView(uint32_t slot);
	bool setModKnobLabelView(uint32_t controlID, ICustomView* modKnobLabelControl, bool isRAFXView);
	bool setCustomDroplistView(uint32_t controlID, ICustomView* _droplistControl, bool isRAFXView, bool isCoreList);
	//@}

	/**
	\brief
	This demonstrates how to get the module core names from the engine.
	- for ASPiK plugins, this is called at startup and when the user changes cores

	\param moduleType the kind of module to get core names for; see synthconstants.h

	\return a vector of the core names; there will be four (4) of them and they may be 
	empty strings or "-----"

	*/
	inline std::vector<std::string> getModuleCoreNames(uint32_t moduleType)
	{
		std::vector<std::string> names = synthEngine->getModuleCoreNames(moduleType);
		return names;
	}

	/**
	\brief
	simple clearing of data
	*/
	inline void clearCustomViewInfo()
	{
		for (uint32_t i = 0; i < TOTAL_CUSTOM_VIEWS; i++)
		{
			customViewInfo[i].clear();
		}
	}

protected:
	std::shared_ptr<SynthLab::SynthEngine> synthEngine = nullptr;
	void* pluginCore = nullptr;
	uint32_t customUpdateCode_1 = 0.0;
	CustomViewInfo customViewInfo[TOTAL_CUSTOM_VIEWS]; ///< one slot per bit * 2 because MSB sets more stuff


	/**
	\brief
	Converts a custom update code (bits in a uint32_t) into an index for the custom view information array

	\param code the 32-bit custom update code
	\param mask a mask containing the bit that is being indexed
	\param upper32 set to TRUE to query for mod knobs, FALSE to query for module strings
	*/
	inline int32_t customUpdateCodeToIndex(uint32_t code, uint32_t mask, bool upper32)
	{
		if ((code & mask) == 0)
			return -1;

		int32_t tz = SynthLab::countTrailingZero(mask);
		if (upper32)
			return 31 + tz;

		return tz;
	}

	/**
	\brief
	Converts a custom update mask (bits in a uint32_t) into an index for the custom view information array

	\param mask a mask containing the bit that is being indexed
	\param upper32 set to TRUE to query for mod knobs, FALSE to query for module strings
	*/
	inline int32_t customUpdateMaskToIndex(uint32_t mask, bool upper32)
	{
		int32_t tz = SynthLab::countTrailingZero(mask);
		if (upper32)
			return 31 + tz;

		return tz;
	}


	// --- helpers to prevent memory-creep
	std::atomic<bool> queueEnabler;			///< atomic bool for enabling/disabling the queue
	std::atomic<bool> updateEnabler;		///< atomic bool for enabling/disabling the queue

	/**
	\brief
	Queues up GUI dynamic strings for reloading into the GUI. The operation is asynchronous
	thus the queue. For ASPiK, this update occurs on the next GUI timer ping. See your
	plugin framework for non ASPiK implementations. 
	- demonstrates how to ask the engine for a particular set of module strings
	- demonstrates how to use a lock free ring buffer to enqueue the information

	\param index the location (slot) in the custom view array
	*/
	inline void enqueueCustomData(uint32_t index)
	{
		if (!isCustomViewDataQueueEnabled()) return;

		if (customViewInfo[index].droplistControl_RAFX || customViewInfo[index].droplistControl_ASPiK)
		{
			uint32_t droplistMask = customViewInfo[index].droplistMask;
			
			// peeking is not allowed from the producer side!
			// if(!customViewInfo[index].droplistDataQueue.peek())
			size_t queueSize = customViewInfo[index].droplistDataQueue.size_approx();
			if (queueSize == 0)
			{
				customViewInfo[index].droplistDataQueue.enqueue(synthEngine->getModuleStrings(droplistMask));
			}
		}
		else
		{
			uint32_t modKnobMask = customViewInfo[index].modKnobMask;
			std::vector<std::string> knobLabels = synthEngine->getModKnobStrings(modKnobMask);
			uint32_t numLabels = knobLabels.size();
			numLabels = fmin(numLabels, 4);
			for (int i = 0; i < numLabels; i++)
			{
			//	if (!customViewInfo[index].modKnobLabelDataQueue[i].peek())
				size_t queueSize = customViewInfo[index].modKnobLabelDataQueue[i].size_approx();
				if (queueSize == 0)
				{
					customViewInfo[index].modKnobLabelDataQueue[i].enqueue(knobLabels[i]);
				}
			}
		}
	}

	/**
	\brief
	De-queue the lists to see if there are waveform strings for a GUI control.
	- demonstrates how to use a lock free ring buffer to dequeue the information

	\param index the location (slot) in the custom view array
	*/
	inline bool getWaveformStrings(uint32_t index, std::vector<std::string>& strings)
	{
		if (!customViewInfo[index].droplistControl_RAFX && !customViewInfo[index].droplistControl_ASPiK)
		{
			return false;
		}

		bool success = customViewInfo[index].droplistDataQueue.try_dequeue(strings);

		// --- pop all
		while (customViewInfo[index].droplistDataQueue.try_dequeue(strings))
			/* NOOP */;

		return success;
	}

	/**
	\brief
	De-queue the lists to see if there are mod knob labels for a GUI control.
	- demonstrates how to use a lock free ring buffer to dequeue the information

	\param index the location (slot) in the custom view array
	*/
	inline bool getModKnobLabel(uint32_t slot, uint32_t index, std::string& string)
	{
		if (!customViewInfo[slot].modKnobLabel_RAFX[index] && !customViewInfo[slot].modKnobLabel_ASPiK[index])
			return false;

		bool success = customViewInfo[slot].modKnobLabelDataQueue[index].try_dequeue(string);

		// --- pop all
		while (customViewInfo[slot].modKnobLabelDataQueue[index].try_dequeue(string))
			/* NOOP */;

		return success;
	}

};

