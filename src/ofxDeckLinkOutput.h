#pragma once

#include "ofxDeckLinkAPIConstants.h"

OFX_DECKLINK_API_BEGIN_NAMESPACE

class Output : public IDeckLinkVideoOutputCallback
{
public:
	
	Output();
    ~Output();
	
	bool setup(int device_id = 0);
	void close();
	
	void listDisplayMode();
	bool start(BMDDisplayMode mode = bmdModeHD1080p30);
	
	void publishTexture(ofTexture &tex);
	void publishPixels(ofPixels &pix);
	
protected:
	
	IDeckLink* pDL;
	IDeckLinkOutput* pDLOutput;
	IDeckLinkMutableVideoFrame*	pDLVideoFrame;
	
	uint32_t uiFrameWidth;
	uint32_t uiFrameHeight;
	
	uint32_t uiFPS;
	uint32_t uiTotalFrames;
	
	BMDTimeValue frameDuration;
	BMDTimeScale frameTimescale;
	
	ofPixels pixels[2];
	ofPixels *front_buffer, *back_buffer;
	ofMutex *mutex;
	bool has_new_frame;
	
	bool initDeckLink(int device_id);
	bool startDeckLink(BMDDisplayMode mode);
	
	void resetFrame();
	void setPreroll();
	
public:
	
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
    virtual ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
    virtual ULONG STDMETHODCALLTYPE Release(void) { return 1; }
	
	virtual HRESULT ScheduledFrameCompleted (/* in */ IDeckLinkVideoFrame *completedFrame, /* in */ BMDOutputFrameCompletionResult result);
    virtual HRESULT ScheduledPlaybackHasStopped (void) { return S_OK; }
};

OFX_DECKLINK_API_END_NAMESPACE
