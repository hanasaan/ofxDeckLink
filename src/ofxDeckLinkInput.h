#pragma once

#include "ofxDeckLinkAPIConstants.h"

OFX_DECKLINK_API_BEGIN_NAMESPACE

class Input : public IDeckLinkInputCallback, public ofBaseDraws
{
public:
	enum DrawMode
	{
		DRAWMODE_PROGRESSIVE,
		DRAWMODE_UPPERFIELD,
		DRAWMODE_LOWERFIELD,
		DRAWMODE_AUTOFIELD // draw upperfield --> lowerfield automatically if interlace
	};
	
	Input();
	~Input();
	
	bool setup(int device_id = 0);
	void close();
	
	void listDisplayMode();
	bool start(BMDDisplayMode mode = bmdModeHD1080p30);
	void pause();
	void resume();
	void stop();
	
	void startAutoDisplayMode() { do_auto_search = true; auto_search_tick = 0; }
	string getCurrentDisplayMode() const;
	string getFieldDominance() const;
	
	bool hasValidSignal() const { return valid_signal; }
	
	float getWidth() const { return width; }
	float getHeight() const { return height; }
	
	const string& getDeviceName() const { return device_name; }

	void update();
	
	float getFps() const { return (float)frameTimescale / frameDuration; }
	
	uint64_t getTimestamp() const { return timestamp; }

	bool isFrameNew() const { return bNewFrame; }
	
	void draw(float x, float y) const;
	void draw(float x, float y, float w, float h) const;
	
	DrawMode getDrawMode() const { return draw_mode; }
	void setDrawMode(DrawMode m) { draw_mode = m; }
	
	ofTexture& getTexture() { return tex; }
	const ofTexture& getTexture() const { return tex; }

	ofPixels& getPixels() { return vuy_front; }
	const ofPixels& getPixels() const { return vuy_front; }
protected:
	
	IDeckLink* pDL;
	IDeckLinkInput* pDLInput;
	
	string device_name;
	
	uint32_t width;
	uint32_t height;
	
	_BMDDisplayMode mode;
	BMDTimeValue frameDuration;
	BMDTimeScale frameTimescale;
	_BMDFieldDominance fieldDominance;
	
	bool valid_signal;
	
	bool do_auto_search;
	int auto_search_tick;
	int display_mode_index;
	
	ofMutex *mutex;
	ofMutex *mutex_op;
	
	bool initDeckLink(int device_id);
	
	uint64_t timestamp;
	
	bool bNewBuffer;
	bool bNewFrame;
	int lastFrameNo;

	ofPixels vuy_front;
	ofPixels vuy_back;
	ofTexture tex;
	ofShader shader;
	ofShader shader_prog;
	
	DrawMode draw_mode;
public:
	
	HRESULT VideoInputFormatChanged (/* in */ BMDVideoInputFormatChangedEvents notificationEvents, /* in */ IDeckLinkDisplayMode *newDisplayMode, /* in */ BMDDetectedVideoInputFormatFlags detectedSignalFlags);
	HRESULT VideoInputFrameArrived (/* in */ IDeckLinkVideoInputFrame* videoFrame, /* in */ IDeckLinkAudioInputPacket* audioPacket);
	
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
	ULONG STDMETHODCALLTYPE AddRef(void) { return 1; };
	ULONG STDMETHODCALLTYPE Release(void) { return 1; };

};

OFX_DECKLINK_API_END_NAMESPACE