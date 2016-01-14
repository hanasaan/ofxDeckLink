#pragma once

#include "ofxDeckLinkAPIConstants.h"
#include "ofxDeckLinkInput.h"
#include "ofxDeckLinkOutput.h"

OFX_DECKLINK_API_BEGIN_NAMESPACE

struct Device
{
	int persistent_id;
	int topological_id;
	string model_name;
	Device() : persistent_id(0), topological_id(0) {}
};

vector<Device> listDevice();

OFX_DECKLINK_API_END_NAMESPACE