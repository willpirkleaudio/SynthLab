#include "synthlabds.h"
#include "plugincore.h"
#pragma warning(disable: 4996)	
// --------------------------------------
//	--- OPTIONAL SynthLab SDK File --- // 
//  -------------------------------------
/**
\file   synthlabds.cpp
\author Will Pirkle
\brief  See also Designing Software Synthesizers in C++ 2nd Ed. by Will Pirkle
\date   20-April-2021
- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------

/**
\brief
Constructs the manager object; the manager needs access to the synth engine to query it for
custom GUI strings and labels. It needs a pointer to the plugincore in order to connect the 
engine's custom strings to the plugin parameters, which ultimately represent the GUI controls.
- This is not part of SynthLab. It is only for ASPiK and RAFX integration and can be ignored for any
other framework.

\param _synthEngine shared pointer to the engine
\param _pluginCore cloaked void* (to avoid cyclic #includes)
*/
DynamicStringManager::DynamicStringManager(std::shared_ptr<SynthLab::SynthEngine> _synthEngine, 
										   void* _pluginCore)
{
	synthEngine = _synthEngine;
	pluginCore = _pluginCore;
}

/**
\brief
Adds a custom view interface pointer to the list of custom controls for drop-lists or list controls. 
- This is not part of SynthLab. It is only for ASPiK and RAFX integration and can be ignored for any 
other framework. 

\param controlID the ID of the parameter connected to the GUI control
\param _droplistControl a custom view interface pointer (in ASPiK, once registered, a custom view
	pointer will NEVER go out of scope and can be safely used at anytime prior to termination.
\param isRAFXView this is true only for the RackAFX prototyping panel
\param isCoreList true if this custom control holds a list of four (4) cores
*/
bool DynamicStringManager::setCustomDroplistView(uint32_t controlID, ICustomView* _droplistControl, 
	bool isRAFXView, bool isCoreList)
{
	if (!pluginCore) return false;
	PluginCore* core = (PluginCore*)pluginCore;

	AuxParameterAttribute* customControlAttribute = core->getPluginParameterByControlID(controlID)->getAuxAttribute(CUSTOM_OPTION_MENU);
	PluginParameter* piparam = core->getPluginParameterByControlID(controlID);

	if (customControlAttribute)
	{
		int32_t index = customUpdateMaskToIndex(customControlAttribute->getUintAttribute(), false);
		if (index < 0) return false;
		if (isRAFXView)
			customViewInfo[index].droplistControl_RAFX = _droplistControl;
		else
			customViewInfo[index].droplistControl_ASPiK = _droplistControl;

		customViewInfo[index].droplistMask = customControlAttribute->getUintAttribute();
		if(piparam)
			customViewInfo[index].controlStringCount = piparam->getStringCount();
		else
			customViewInfo[index].controlStringCount = 1;

		// --- mark as dirty
		if (!isCoreList)
		{
			core->voiceParameters->updateCodeDroplists |= customViewInfo[index].droplistMask;
		}

		return true;
	}
	return false;

}

/**
\brief
Adds a custom view interface pointer to the list of custom controls for mod knob labels.
- This is not part of SynthLab. It is only for ASPiK and RAFX integration and can be ignored for any
other framework.

\param controlID the ID of the parameter connected to the GUI control
\param modKnobLabelControl a custom view interface pointer(in ASPiK, once registered, a custom view
	pointer will NEVER go out of scope and can be safely used at anytime prior to termination.
\param isRAFXView this is true only for the RackAFX prototyping panel
\param isCoreList true if this custom control holds a list of four(4) cores

\return true if sucessful, false otherwise
*/
bool  DynamicStringManager::setModKnobLabelView(uint32_t controlID, ICustomView* modKnobLabelControl, bool isRAFXView)
{
	if (!pluginCore) return false;
	PluginCore* core = (PluginCore*)pluginCore;

	AuxParameterAttribute* customControlAttribute = core->getPluginParameterByControlID(controlID)->getAuxAttribute(CUSTOM_KNOB_LABEL);
	AuxParameterAttribute* customControlIndex = core->getPluginParameterByControlID(controlID)->getAuxAttribute(CUSTOM_KNOB_LABEL_INDEX);
	if (customControlAttribute && customControlIndex)
	{
		int32_t slot = customUpdateMaskToIndex(customControlAttribute->getUintAttribute(), true);
		if (slot < 0) return false;

		uint32_t index = customControlIndex->getUintAttribute();

		if (isRAFXView)
			customViewInfo[slot].modKnobLabel_RAFX[index] = modKnobLabelControl;
		else
			customViewInfo[slot].modKnobLabel_ASPiK[index] = modKnobLabelControl;

		customViewInfo[slot].modKnobMask = customControlAttribute->getUintAttribute();

		// --- mark as dirty
		core->voiceParameters->updateCodeKnobs |= customViewInfo[slot].modKnobMask;

		return true;
	}
	return false;
}

/**
\brief
Store the two custom update code values, uin32_t variables where each bit encodes a type
of control. 
- This is not part of SynthLab. It is only for ASPiK and RAFX integration and can be ignored for any
other framework.

\param customUpdateCode_1 the uint32_t custom code for drop-lists (core names and module strings)
\param customUpdateCode_2  the uint32_t custom code for mod-knobs

\return true if sucessful, false otherwise
*/
bool DynamicStringManager::setCustomUpdateCodes(uint32_t customUpdateCode_1, uint32_t customUpdateCode_2)
{
	bool haveUpdate = false;

	if (customUpdateCode_1 > 0)
	{
		uint32_t seed = 0x1;
		for (uint32_t i = 0; i < CUSTOM_BITS; i++)
		{
			int32_t index = customUpdateCodeToIndex(customUpdateCode_1, seed << i, false);
			if (index >= 0)
			{
				enqueueCustomData(index);// , voiceParameters);
				haveUpdate = true; // sticky
			}
		}
	}

	if (customUpdateCode_2 > 0)
	{
		uint32_t seed = 0x1;
		for (uint32_t i = 0; i < CUSTOM_BITS; i++)
		{
			int32_t index = customUpdateCodeToIndex(customUpdateCode_2, seed << i, true);
			if (index >= 0)
			{
				enqueueCustomData(index);// , voiceParameters);
				haveUpdate = true; // sticky
			}
		}
	}

	if (haveUpdate)
		// --- set the atomic flag for updating
		enableCustomUpdates(true);

	return haveUpdate;
}

/**
\brief
Sets information about the custom view including the strings for the controls. 
- This is not part of SynthLab. It is only for ASPiK and RAFX integration and can be ignored for any
other framework.

\param slot location within the custom view array

\return true if sucessful, false otherwise
*/
bool DynamicStringManager::updateCustomView(uint32_t slot)
{
	if (slot >= TOTAL_CUSTOM_VIEWS) return false;

	std::vector<std::string> waveforms;
	if (getWaveformStrings(slot, waveforms))
	{
		if (customViewInfo[slot].droplistControl_RAFX) //isRAFXClient)
		{
			VSTGUI::RAFX2CustomViewMessage viewMessage;

			// --- send the view the data as a string
			viewMessage.message = VSTGUI::MESSAGE_SET_STRINGLIST;
			viewMessage.stringCount = waveforms.size();

			char** arr = new char*[waveforms.size()];
			for (size_t i = 0; i < waveforms.size(); i++) {
				arr[i] = new char[waveforms[i].size() + 1];
				strcpy(arr[i], waveforms[i].c_str());
			}

			viewMessage.stringList = arr;

			// --- send the message
			customViewInfo[slot].droplistControl_RAFX->sendMessage(&viewMessage);

			// --- clean up 
			for (size_t i = 0; i < waveforms.size(); i++) {
				delete[] arr[i];
			}
			delete[] arr;

			customViewInfo[slot].droplistControl_RAFX->updateView();
		}

		if (customViewInfo[slot].droplistControl_ASPiK)
		{
			VSTGUI::TextDisplayViewMessage viewMessage;

			// --- send the view the data as a string
			viewMessage.message = VSTGUI::MESSAGE_SET_STRINGLIST;
			viewMessage.stringList = waveforms;
			viewMessage.controlStringCount = customViewInfo[slot].controlStringCount;

			// --- send the message
			customViewInfo[slot].droplistControl_ASPiK->sendMessage(&viewMessage);

			// --- tell view to repaint and empty it's ring buffer (if needed)
			customViewInfo[slot].droplistControl_ASPiK->updateView();

			if (pluginCore)
			{
				PluginCore* core = (PluginCore*)pluginCore;
				uint32_t mask = customViewInfo[slot].droplistMask;
				core->voiceParameters->updateCodeDroplists &= ~mask;
			}
		}

		return true;
	}
	else // try mod labels
	{
		std::string labelString;

		for (int index = 0; index < 4; index++)
		{
			if (getModKnobLabel(slot, index, labelString))
			{
				if (customViewInfo[slot].modKnobLabel_RAFX[index])
				{
					VSTGUI::RAFX2CustomViewMessage viewMessage;

					// --- send the view the data as a string
					viewMessage.message = VSTGUI::MESSAGE_SET_STRING;
					viewMessage.labelString = labelString.c_str();

					// --- send the message
					customViewInfo[slot].modKnobLabel_RAFX[index]->sendMessage(&viewMessage);

					// --- tell view to repaint and empty it's ring buffer (if needed)
					customViewInfo[slot].modKnobLabel_RAFX[index]->updateView();
				}

				if (customViewInfo[slot].modKnobLabel_ASPiK[index])
				{
					VSTGUI::TextDisplayViewMessage viewMessage;

					// --- send the view the data as a string
					viewMessage.message = VSTGUI::MESSAGE_SET_STRING;
					viewMessage.labelString = labelString;

					// --- send the message
					customViewInfo[slot].modKnobLabel_ASPiK[index]->sendMessage(&viewMessage);

					// --- tell view to repaint and empty it's ring buffer (if needed)
					customViewInfo[slot].modKnobLabel_ASPiK[index]->updateView();

					if (pluginCore)
					{
						PluginCore* core = (PluginCore*)pluginCore;
						uint32_t mask = customViewInfo[slot].modKnobMask;
						core->voiceParameters->updateCodeKnobs &= ~mask;
					}
				}
			}
		}
		return true;
	}
	return false;
}