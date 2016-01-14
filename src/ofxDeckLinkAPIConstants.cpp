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

OFX_DECKLINK_API_END_NAMESPACE