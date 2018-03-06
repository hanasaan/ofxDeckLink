#pragma once

#include "ofMain.h"

#include "DeckLinkAPI.h"

#define OFX_DECKLINK_API_BEGIN_NAMESPACE namespace ofx { namespace DeckLinkAPI {
#define OFX_DECKLINK_API_END_NAMESPACE } }

OFX_DECKLINK_API_BEGIN_NAMESPACE

const char* toString(_BMDDisplayMode mode);
const char* toString(_BMDPixelFormat mode);
const char* toString(_BMDFieldDominance mode);

struct Timecode
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t frames;
    bool b_drop_frame;
    Timecode();
    
    int toFrameNum(int timebase) const;
    string toString() const;
    
    bool operator==( const Timecode& vec ) const;
    bool operator!=( const Timecode& vec ) const;
};

OFX_DECKLINK_API_END_NAMESPACE

namespace ofxDeckLinkAPI = ofx::DeckLinkAPI;