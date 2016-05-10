#include "ofxDeckLinkInput.h"

OFX_DECKLINK_API_BEGIN_NAMESPACE

Input::Input() : pDL(NULL), pDLInput(NULL), mutex(NULL), valid_signal(false), do_auto_search(false), auto_search_tick(0), display_mode_index(0), width(0), height(0),draw_mode(DRAWMODE_PROGRESSIVE)
{
}

Input::~Input()
{
	close();
}

#define STRINGIFY(A) #A

bool Input::setup(int device_id)
{
	close();
	
	if (mutex == NULL)
		mutex = new ofMutex;
	
	string frag = STRINGIFY
	(
	 uniform sampler2DRect tex;
	 uniform int use_odd;
	 
	 void main (void){
		 float isodd_x = mod(gl_TexCoord[0].x, 2.0);
		 float isodd_y = mod(gl_TexCoord[0].y, 2.0);
		 vec2 texcoord0 = gl_TexCoord[0].xy;
		 vec2 texcoord1 = texcoord0 + vec2(1.0, 0.0);
		 float y = 0.0;
		 float u = 0.0;
		 float v = 0.0;
		 
		 vec4 evenfield;
		 if((bool(use_odd) && isodd_y < 1.0) || (!bool(use_odd) && isodd_y >= 1.0)){
			 evenfield = texture2DRect(tex, vec2(texcoord0.x, texcoord0.y + 1.0));
			 vec4 evenfield_2 = texture2DRect(tex, vec2(texcoord0.x, texcoord0.y - 1.0));
			 y = mix(evenfield.a, evenfield_2.a, 0.5);
			 if (isodd_x >= 1.0) {
				 v = mix(evenfield.r, evenfield_2.r, 0.5) - 0.5;
				 u = texture2DRect(tex, vec2(texcoord1.x, texcoord1.y + 1.0)).r - 0.5;
			 } else {
				 u = mix(evenfield.r, evenfield_2.r, 0.5) - 0.5;
				 v = texture2DRect(tex, vec2(texcoord1.x, texcoord1.y + 1.0)).r - 0.5;
			 }
		 } else {
			 evenfield = texture2DRect(tex, texcoord0);
			 y = evenfield.a;
			 if (isodd_x >= 1.0) {
				 v = evenfield.r - 0.5;
				 u = texture2DRect(tex, texcoord1).r - 0.5;
			 } else {
				 u = evenfield.r - 0.5;
				 v = texture2DRect(tex, texcoord1).r - 0.5;
			 }
		 }
		 y = 1.164 * y - 0.0625;
		 gl_FragColor.r = y + 1.596 * v;
		 gl_FragColor.g = y - 0.391 * u - 0.813 * v;
		 gl_FragColor.b = y + 2.018 * u;
		 gl_FragColor.a = 1.0;
	 }
	 );
	
	string frag_prog = STRINGIFY
	(
	 uniform sampler2DRect tex;
	 
	 void main (void){
		 float isodd_x = mod(gl_TexCoord[0].x, 2.0);
		 vec2 texcoord0 = gl_TexCoord[0].xy;
		 vec2 texcoord1 = texcoord0 + vec2(1.0, 0.0);
		 float y = 0.0;
		 float u = 0.0;
		 float v = 0.0;
		 
		 vec4 evenfield = texture2DRect(tex, texcoord0);
		 y = evenfield.a;
		 if (isodd_x >= 1.0) {
			 v = evenfield.r - 0.5;
			 u = texture2DRect(tex, texcoord1).r - 0.5;
		 } else {
			 u = evenfield.r - 0.5;
			 v = texture2DRect(tex, texcoord1).r - 0.5;
		 }
		 y = 1.164 * y - 0.0625;
		 gl_FragColor.r = y + 1.596 * v;
		 gl_FragColor.g = y - 0.391 * u - 0.813 * v;
		 gl_FragColor.b = y + 2.018 * u;
		 gl_FragColor.a = 1.0;
	 }
	 );
	
	shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag);
	shader.linkProgram();

	shader_prog.setupShaderFromSource(GL_FRAGMENT_SHADER, frag_prog);
	shader_prog.linkProgram();

	return initDeckLink(device_id);
}

void Input::close()
{
	stop();
	
	if (pDLInput != NULL)
	{
		pDLInput->Release();
		pDLInput = NULL;
	}
	
	if (pDL != NULL)
	{
		pDL->Release();
		pDL = NULL;
	}
	
	if (mutex != NULL)
	{
		delete mutex;
		mutex = NULL;
	}
}

bool Input::start(BMDDisplayMode mode)
{
	stop();
	
	bNewBuffer = false;
	bNewFrame = false;
	timestamp = 0;
	lastFrameNo = -1;
	
	this->mode = (_BMDDisplayMode)mode;
	
	BMDPixelFormat pixelFormat = bmdFormat8BitYUV;
	
	IDeckLinkDisplayMode* pDLDisplayMode = NULL;
	BMDDisplayModeSupport result;
	if (pDLInput->DoesSupportVideoMode(mode, pixelFormat, bmdVideoInputEnableFormatDetection, &result, &pDLDisplayMode) != S_OK)
		return false;
	
	if (result != bmdDisplayModeSupported || pDLDisplayMode == NULL)
	{
		ofLogError("ofxDeckLinkAPI::Input") << "invalid display mode";
		return false;
	}
	
	width = pDLDisplayMode->GetWidth();
	height = pDLDisplayMode->GetHeight();
	fieldDominance = (_BMDFieldDominance)pDLDisplayMode->GetFieldDominance();
	
	vuy_back.allocate(width, height, 2);
	vuy_front.allocate(width, height, 2);

	
	pDLDisplayMode->GetFrameRate(&frameDuration, &frameTimescale);
	
	pDLInput->DisableAudioInput();

	if (pDLInput->EnableVideoInput(pDLDisplayMode->GetDisplayMode(), pixelFormat, bmdVideoInputFlagDefault) != S_OK)
		return false;
	
	pDLInput->StartStreams();
	
	shader.begin();
	shader.setUniform1i("use_odd", 0);
	shader.end();
	
	return true;
}

void Input::pause()
{
	if (pDLInput)
	{
		pDLInput->PauseStreams();
	}
}


void Input::resume()
{
	if (pDLInput)
	{
		pDLInput->StartStreams();
	}
}

void Input::stop()
{
	if (pDLInput)
	{
		pDLInput->StopStreams();
	}
}

//

bool Input::initDeckLink(int device_id)
{
	bool bSuccess = FALSE;
	IDeckLinkIterator* pDLIterator = NULL;
	
	pDLIterator = CreateDeckLinkIteratorInstance();
	if (pDLIterator == NULL)
	{
		ofLogError("ofxDeckLinkAPI::Input") << "This application requires the DeckLink drivers installed." << "\n" << "Please install the Blackmagic DeckLink drivers to use the features of this application.";
		goto error;
	}
	
	for (int i = 0; i < device_id + 1; i++)
	{
		if (pDLIterator->Next(&pDL) != S_OK)
		{
			pDL = NULL;
			break;
		}
	}
	
	if (pDL == NULL)
	{
		ofLogError("ofxDeckLinkAPI::Input") << "This application requires a DeckLink device." << "\n" << "You will not be able to use the features of this application until a DeckLink PCI card is installed.";
		goto error;
	}
	
	{
		CFStringRef deviceNameCFString = NULL;
		
		HRESULT result = pDL->GetModelName(&deviceNameCFString);
		if (result == S_OK)
		{
			char name[64];
			CFStringGetCString(deviceNameCFString, name, sizeof(name), kCFStringEncodingUTF8);
			device_name = name;
			CFRelease(deviceNameCFString);
		}
	}
	
	if (pDL->QueryInterface(IID_IDeckLinkInput, (void**)&pDLInput) != S_OK)
		goto error;
	
	if (pDLInput->SetCallback(this) != S_OK)
		goto error;
	
	bSuccess = TRUE;
	
error:
	
	if (!bSuccess)
	{
		if (pDLInput != NULL)
		{
			pDLInput->Release();
			pDLInput = NULL;
		}
		if (pDL != NULL)
		{
			pDL->Release();
			pDL = NULL;
		}
	}
	
	if (pDLIterator != NULL)
	{
		pDLIterator->Release();
		pDLIterator = NULL;
	}
	
	return bSuccess;
}

void Input::listDisplayMode()
{
	assert(pDLInput);
	
	IDeckLinkDisplayModeIterator* pDLDisplayModeIterator = NULL;
	IDeckLinkDisplayMode* pDLDisplayMode = NULL;
	HRESULT result;
	
	if (pDLInput->GetDisplayModeIterator(&pDLDisplayModeIterator) != S_OK)
	{
		ofLogError("ofxDeckLinkAPI::Input") << "no input device found";
		return;
	}
	
	cout << "==== Input::listDisplayMode() -> " << device_name << " ====" << endl;
	
	while (pDLDisplayModeIterator->Next(&pDLDisplayMode) == S_OK)
	{
		CFStringRef displayModeCFString = NULL;
		result = pDLDisplayMode->GetName(&displayModeCFString);
		if (result == S_OK)
		{
			BMDTimeValue frameRateDuration;
			BMDTimeScale frameRateScale;
			pDLDisplayMode->GetFrameRate(&frameRateDuration, &frameRateScale);
			
			cout << toString((_BMDDisplayMode)pDLDisplayMode->GetDisplayMode()) << " : ";
			cout << pDLDisplayMode->GetWidth() << "x" << pDLDisplayMode->GetHeight() << " @ " << ((double)frameRateScale / (double)frameRateDuration);
			cout << endl;
			
			CFRelease(displayModeCFString);
		}
		
		pDLDisplayMode->Release();
	}
	
	cout << "====================================" << endl << endl;
	
	pDLDisplayModeIterator->Release();
}

//

HRESULT Input::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode *newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags)
{
	cerr << "VideoInputFormatChanged" << endl;
}

HRESULT Input::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket)
{
	valid_signal = (videoFrame->GetFlags() & bmdFrameHasNoInputSource) == 0;
	if (!valid_signal) {
		return S_OK;
	}
	timestamp = ofGetSystemTime();
	
	void *src = NULL;
	videoFrame->GetBytes(&src);
	
	unsigned char *dst = vuy_back.getPixels();
	memcpy(dst, src, width * height * 2);
	
	mutex->lock();
	vuy_back.swap(vuy_front);
	bNewBuffer = true;
	mutex->unlock();
	return S_OK;
}

void Input::update()
{
	if (ofGetFrameNum() != lastFrameNo)
	{
		bNewFrame = false;
		lastFrameNo = ofGetFrameNum();
	}	
	if (bNewBuffer) {
		mutex->lock();
		tex.loadData(vuy_front);
		bNewBuffer = false;
		mutex->unlock();
		bNewFrame = true;
	}
	
	if (do_auto_search)
	{
		if (auto_search_tick == 0)
		{
			_BMDDisplayMode modes[] = {
				bmdModeHD1080p2398,
				bmdModeHD1080p24,
				bmdModeHD1080p25,
				bmdModeHD1080p2997,
				bmdModeHD1080p30,
				bmdModeHD1080i50,
				bmdModeHD1080i5994,
				bmdModeHD1080i6000,
				bmdModeHD1080p50,
				bmdModeHD1080p5994,
				bmdModeHD1080p6000,
				bmdModeHD720p50,
				bmdModeHD720p5994,
				bmdModeHD720p60,
				bmdMode2k2398,
				bmdMode2k24,
				bmdMode2k25,
				bmdMode4K2160p2398,
				bmdMode4K2160p24,
				bmdMode4K2160p25,
				bmdMode4K2160p2997,
				bmdMode4K2160p30,
				bmdModeNTSC,
				bmdModeNTSC2398,
				bmdModePAL,
				bmdModeNTSCp,
				bmdModePALp
			};

			int num_display_mode = sizeof(modes) / sizeof(_BMDDisplayMode);
			
			ofLogNotice("ofxDeckLinkAPI::Input") << "trying display mode: " << toString(modes[display_mode_index]);
			start(modes[display_mode_index]);
			
			display_mode_index++;
			if (display_mode_index == num_display_mode)
			{
				display_mode_index = 0;
				do_auto_search = false;
				
				ofLogError("ofxDeckLinkAPI::Input") << "no valid display mode";
			}
		}
		
		auto_search_tick++;
		auto_search_tick %= 60;
	}
	
	if (hasValidSignal())
	{
		do_auto_search = false;
	}
}

string Input::getCurrentDisplayMode() const
{
	return toString(mode);
}

string Input::getFieldDominance() const
{
	return toString(fieldDominance);
}

void Input::draw(float x, float y) const
{
	draw(x, y, getWidth(), getHeight());
}

void Input::draw(float x, float y, float w, float h) const
{
	bool bprog = draw_mode == DRAWMODE_PROGRESSIVE;
	const ofShader* s = bprog ? &shader_prog : &shader;
	s->begin();
	if (!bprog) {
		bool bufield = draw_mode == DRAWMODE_UPPERFIELD || (draw_mode == DRAWMODE_AUTOFIELD && isFrameNew());
		s->setUniform1i("use_odd", bufield ? 0 : 1);
	}
	tex.draw(x, y);
	s->end();
}

string Input::getDrawModeString() const
{
	switch (draw_mode) {
		case DRAWMODE_PROGRESSIVE:
			return "Progressive";
		case DRAWMODE_UPPERFIELD:
			return "Upperfield";
		case DRAWMODE_LOWERFIELD:
			return "Lowerfield";
		case DRAWMODE_AUTOFIELD:
			return "Autofield";
		default: break;
	}
	return "Unknown";
}

OFX_DECKLINK_API_END_NAMESPACE