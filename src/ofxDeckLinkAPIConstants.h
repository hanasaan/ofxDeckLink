#pragma once

#include "ofMain.h"

#include "DeckLinkAPI.h"

#define OFX_DECKLINK_API_BEGIN_NAMESPACE namespace ofx { namespace DeckLinkAPI {
#define OFX_DECKLINK_API_END_NAMESPACE } }

OFX_DECKLINK_API_BEGIN_NAMESPACE

const char* toString(_BMDDisplayMode mode);
const char* toString(_BMDPixelFormat mode);
const char* toString(_BMDFieldDominance mode);

OFX_DECKLINK_API_END_NAMESPACE

namespace ofxDeckLinkAPI = ofx::DeckLinkAPI;