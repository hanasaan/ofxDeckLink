#include "ofxDeckLinkAPIConstants.h"

OFX_DECKLINK_API_BEGIN_NAMESPACE

#define TOSTR(ENUM) case ENUM: return #ENUM;

const char* toString(_BMDDisplayMode mode)
{
	switch(mode)
	{
		TOSTR(bmdModeNTSC);
		TOSTR(bmdModeNTSC2398);
		TOSTR(bmdModePAL);
		TOSTR(bmdModeNTSCp);
		TOSTR(bmdModePALp);
		TOSTR(bmdModeHD1080p2398);
		TOSTR(bmdModeHD1080p24);
		TOSTR(bmdModeHD1080p25);
		TOSTR(bmdModeHD1080p2997);
		TOSTR(bmdModeHD1080p30);
		TOSTR(bmdModeHD1080i50);
		TOSTR(bmdModeHD1080i5994);
		TOSTR(bmdModeHD1080i6000);
		TOSTR(bmdModeHD1080p50);
		TOSTR(bmdModeHD1080p5994);
		TOSTR(bmdModeHD1080p6000);
		TOSTR(bmdModeHD720p50);
		TOSTR(bmdModeHD720p5994);
		TOSTR(bmdModeHD720p60);
		TOSTR(bmdMode2k2398);
		TOSTR(bmdMode2k24);
		TOSTR(bmdMode2k25);
		TOSTR(bmdMode4K2160p2398);
		TOSTR(bmdMode4K2160p24);
		TOSTR(bmdMode4K2160p25);
		TOSTR(bmdMode4K2160p2997);
		TOSTR(bmdMode4K2160p30);
		TOSTR(bmdModeUnknown);
		default: return "invalid enum";
	}
}

const char* toString(_BMDPixelFormat mode)
{
	switch(mode)
	{
		TOSTR(bmdFormat8BitYUV);
		TOSTR(bmdFormat10BitYUV);
		TOSTR(bmdFormat8BitARGB);
		TOSTR(bmdFormat8BitBGRA);
		TOSTR(bmdFormat10BitRGB);
		default: return "invalid enum";
	}
}

const char* toString(_BMDFieldDominance mode)
{
	switch(mode)
	{
		TOSTR(bmdUnknownFieldDominance);
		TOSTR(bmdLowerFieldFirst);
		TOSTR(bmdUpperFieldFirst);
		TOSTR(bmdProgressiveFrame);
		TOSTR(bmdProgressiveSegmentedFrame);
		default: return "invalid enum";
	}
}
#undef TOSTR

Timecode::Timecode()
{
    hours = 0;
    minutes = 0;
    seconds = 0;
    frames = 0;
    b_drop_frame = false;
}

int Timecode::toFrameNum(int timebase) const
{
    int num = frames + timebase * seconds + timebase * 60 * minutes + timebase * 3600 * hours;
    if (b_drop_frame) {
        num -= hours * 108;
        for (int i=0; i<=minutes; ++i) {
            if (i % 10 != 0) {
                num -= 2;
            }
        }
    }
    return num;
}

string Timecode::toString() const
{
    if (b_drop_frame) {
        return ofVAArgsToString("%02d:%02d:%02d;%02d",
                                hours,
                                minutes,
                                seconds,
                                frames);
    } else {
        return ofVAArgsToString("%02d:%02d:%02d:%02d",
                                hours,
                                minutes,
                                seconds,
                                frames);
    }
}

bool Timecode::operator==( const Timecode& vec ) const {
    return (hours == vec.hours)
    && (minutes == vec.minutes)
    && (seconds == vec.seconds)
    && (frames == vec.frames);
}

bool Timecode::operator!=( const Timecode& vec ) const {
    return (hours != vec.hours)
    || (minutes != vec.minutes)
    || (seconds != vec.seconds)
    || (frames != vec.frames);
}


OFX_DECKLINK_API_END_NAMESPACE