#include "ofxDeckLinkInput.h"

OFX_DECKLINK_API_BEGIN_NAMESPACE

Input::Input() : pDL(NULL), pDLInput(NULL), mutex(NULL), valid_signal(false), do_auto_search(false), auto_search_tick(0), display_mode_index(0), width(0), height(0),draw_mode(DRAWMODE_PROGRESSIVE), device_id(0), auto_search_tick_interval(80),
    b_use_rgb_colorspace(false)
{
	if (mutex == NULL)
		mutex = new ofMutex;
}

Input::~Input()
{
	close();
	
	if (mutex != NULL)
	{
		delete mutex;
		mutex = NULL;
	}
}

#define STRINGIFY(A) #A

bool Input::setup(int device_id)
{
	close();
	
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
				 v = mix(evenfield.r, evenfield_2.r, 0.5);
				 u = texture2DRect(tex, vec2(texcoord1.x, texcoord1.y + 1.0)).r;
			 } else {
				 u = mix(evenfield.r, evenfield_2.r, 0.5);
				 v = texture2DRect(tex, vec2(texcoord1.x, texcoord1.y + 1.0)).r;
			 }
		 } else {
			 evenfield = texture2DRect(tex, texcoord0);
			 y = evenfield.a;
			 if (isodd_x >= 1.0) {
				 v = evenfield.r;
				 u = texture2DRect(tex, texcoord1).r;
			 } else {
				 u = evenfield.r;
				 v = texture2DRect(tex, texcoord1).r;
			 }
		 }
         y = clamp(y, 0.06274509803922, 0.94117647058824);
         u = clamp(u, 0.06274509803922, 0.92156862745098) - 0.5;
         v = clamp(v, 0.06274509803922, 0.92156862745098) - 0.5;
         y = 1.164 * (y - 0.06274509803922);
         gl_FragColor = gl_Color;
         gl_FragColor.r *= clamp(y + 1.793 * v, 0.0, 1.0);
         gl_FragColor.g *= clamp(y - 0.213 * u - 0.534 * v, 0.0, 1.0);
         gl_FragColor.b *= clamp(y + 2.155 * u, 0.0, 1.0);
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
			 v = evenfield.r;
			 u = texture2DRect(tex, texcoord1).r;
		 } else {
			 u = evenfield.r;
			 v = texture2DRect(tex, texcoord1).r;
		 }
         y = clamp(y, 0.06274509803922, 0.94117647058824);
         u = clamp(u, 0.06274509803922, 0.92156862745098) - 0.5;
         v = clamp(v, 0.06274509803922, 0.92156862745098) - 0.5;
         y = 1.164 * (y - 0.06274509803922);
         gl_FragColor = gl_Color;
         gl_FragColor.r *= clamp(y + 1.793 * v, 0.0, 1.0);
         gl_FragColor.g *= clamp(y - 0.213 * u - 0.534 * v, 0.0, 1.0);
         gl_FragColor.b *= clamp(y + 2.155 * u, 0.0, 1.0);
	 }
	 );
	
    string frag_argb = STRINGIFY
    (
     uniform sampler2DRect tex;
     uniform int use_odd;
     
     void main (void){
         float isodd_y = mod(gl_TexCoord[0].y, 2.0);
         vec2 texcoord0 = gl_TexCoord[0].xy;
         vec3 rgb;
         vec4 evenfield;
         if((bool(use_odd) && isodd_y < 1.0) || (!bool(use_odd) && isodd_y >= 1.0)){
             evenfield = texture2DRect(tex, vec2(texcoord0.x, texcoord0.y + 1.0));
             vec4 evenfield_2 = texture2DRect(tex, vec2(texcoord0.x, texcoord0.y - 1.0));
             rgb = mix(evenfield.rgb, evenfield_2.rgb, 0.5);
         } else {
             evenfield = texture2DRect(tex, texcoord0);
             rgb = evenfield.rgb;
         }
         gl_FragColor = gl_Color;
         gl_FragColor.rgb *= rgb;
     }
     );
    
    string frag_argb_prog = STRINGIFY
    (
     uniform sampler2DRect tex;
     
     void main (void){
         vec2 texcoord0 = gl_TexCoord[0].xy;
         gl_FragColor = gl_Color;
         gl_FragColor.rgb *= texture2DRect(tex, texcoord0).rgb;
     }
     );
    
    
    string vert150 = "#version 150\n";
    vert150 += STRINGIFY
    (
     in vec4 position;
     in vec2 texcoord;
     
     out vec2 texCoordVarying;
     
     uniform mat4 modelViewProjectionMatrix;
     
     void main()
     {
         texCoordVarying = texcoord;
         gl_Position = modelViewProjectionMatrix * position;
     }
     );
    
    string frag150 = "#version 150\n";
    frag150 += STRINGIFY
    (
     in vec2 texCoordVarying;
     
     out vec4 fragColor;
     
     uniform sampler2DRect tex;
     uniform int use_odd;
     
     void main (void){
         float isodd_x = mod(texCoordVarying.x, 2.0);
         float isodd_y = mod(texCoordVarying.y, 2.0);
         vec2 texcoord0 = texCoordVarying.xy;
         vec2 texcoord1 = texcoord0 + vec2(1.0, 0.0);
         float y = 0.0;
         float u = 0.0;
         float v = 0.0;
         
         vec4 evenfield;
         if((bool(use_odd) && isodd_y < 1.0) || (!bool(use_odd) && isodd_y >= 1.0)){
             evenfield = texture(tex, vec2(texcoord0.x, texcoord0.y + 1.0));
             vec4 evenfield_2 = texture(tex, vec2(texcoord0.x, texcoord0.y - 1.0));
             y = mix(evenfield.a, evenfield_2.a, 0.5);
             if (isodd_x >= 1.0) {
                 v = mix(evenfield.r, evenfield_2.r, 0.5);
                 u = texture(tex, vec2(texcoord1.x, texcoord1.y + 1.0)).r;
             } else {
                 u = mix(evenfield.r, evenfield_2.r, 0.5);
                 v = texture(tex, vec2(texcoord1.x, texcoord1.y + 1.0)).r;
             }
         } else {
             evenfield = texture(tex, texcoord0);
             y = evenfield.a;
             if (isodd_x >= 1.0) {
                 v = evenfield.r;
                 u = texture(tex, texcoord1).r;
             } else {
                 u = evenfield.r;
                 v = texture(tex, texcoord1).r;
             }
         }

         y = clamp(y, 0.06274509803922, 0.94117647058824);
         u = clamp(u, 0.06274509803922, 0.92156862745098) - 0.5;
         v = clamp(v, 0.06274509803922, 0.92156862745098) - 0.5;
         y = 1.164 * (y - 0.06274509803922);
         fragColor.r = clamp(y + 1.793 * v, 0.0, 1.0);
         fragColor.g = clamp(y - 0.213 * u - 0.534 * v, 0.0, 1.0);
         fragColor.b = clamp(y + 2.155 * u, 0.0, 1.0);
         fragColor.a = 1.0;
     }
     );
    
    string frag_prog150 = "#version 150\n";
    frag_prog150 += STRINGIFY
    (
     in vec2 texCoordVarying;
     
     out vec4 fragColor;
     
     uniform sampler2DRect tex;
     
     void main (void){
         float isodd_x = mod(texCoordVarying.x, 2.0);
         vec2 texcoord0 = texCoordVarying.xy;
         vec2 texcoord1 = texcoord0 + vec2(1.0, 0.0);
         float y = 0.0;
         float u = 0.0;
         float v = 0.0;
         
         vec4 evenfield = texture(tex, texcoord0);
         y = evenfield.a;
         if (isodd_x >= 1.0) {
             v = evenfield.r;
             u = texture(tex, texcoord1).r;
         } else {
             u = evenfield.r;
             v = texture(tex, texcoord1).r;
         }
         y = clamp(y, 0.06274509803922, 0.94117647058824);
         u = clamp(u, 0.06274509803922, 0.92156862745098) - 0.5;
         v = clamp(v, 0.06274509803922, 0.92156862745098) - 0.5;
         y = 1.164 * (y - 0.06274509803922);
         fragColor.r = clamp(y + 1.793 * v, 0.0, 1.0);
         fragColor.g = clamp(y - 0.213 * u - 0.534 * v, 0.0, 1.0);
         fragColor.b = clamp(y + 2.155 * u, 0.0, 1.0);
         fragColor.a = 1.0;
     }
     );

    
    string frag_argb_150 = "#version 150\n";
    frag_argb_150 += STRINGIFY
    (
     in vec2 texCoordVarying;
     
     out vec4 fragColor;
     
     uniform sampler2DRect tex;
     uniform int use_odd;
     
     void main (void){
         float isodd_y = mod(texCoordVarying.y, 2.0);
         vec2 texcoord0 = texCoordVarying.xy;
         vec3 rgb;
         vec4 evenfield;
         if((bool(use_odd) && isodd_y < 1.0) || (!bool(use_odd) && isodd_y >= 1.0)){
             evenfield = texture(tex, vec2(texcoord0.x, texcoord0.y + 1.0));
             vec4 evenfield_2 = texture(tex, vec2(texcoord0.x, texcoord0.y - 1.0));
             rgb = mix(evenfield.rgb, evenfield_2.rgb, 0.5);
         } else {
             evenfield = texture2DRect(tex, texcoord0);
             rgb = evenfield.rgb;
         }
         fragColor.rgb = rgb;
         fragColor.a = 1.0;
     }
     );
    
    string frag_argb_prog_150 = "#version 150\n";
    frag_argb_prog_150 += STRINGIFY
    (
     in vec2 texCoordVarying;
     
     out vec4 fragColor;
     
     uniform sampler2DRect tex;
     
     void main (void){
         fragColor = texture(tex,texCoordVarying);
         fragColor.a = 1.0;
     }
     );
    
    if(ofIsGLProgrammableRenderer()){
        shader.setupShaderFromSource(GL_VERTEX_SHADER, vert150);
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag150);
        shader.bindDefaults();
        
        shader_prog.setupShaderFromSource(GL_VERTEX_SHADER, vert150);
        shader_prog.setupShaderFromSource(GL_FRAGMENT_SHADER, frag_prog150);
        shader_prog.bindDefaults();
        
        shader_argb.setupShaderFromSource(GL_VERTEX_SHADER, vert150);
        shader_argb.setupShaderFromSource(GL_FRAGMENT_SHADER, frag_argb_150);
        shader_argb.bindDefaults();
        
        shader_argb_prog.setupShaderFromSource(GL_VERTEX_SHADER, vert150);
        shader_argb_prog.setupShaderFromSource(GL_FRAGMENT_SHADER, frag_argb_prog_150);
        shader_argb_prog.bindDefaults();
        
    }else{
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag);
        shader_prog.setupShaderFromSource(GL_FRAGMENT_SHADER, frag_prog);
        
        shader_argb.setupShaderFromSource(GL_FRAGMENT_SHADER, frag_argb);
        shader_argb_prog.setupShaderFromSource(GL_FRAGMENT_SHADER, frag_argb_prog);
    }
    shader.linkProgram();
    shader_prog.linkProgram();
    shader_argb.linkProgram();
    shader_argb_prog.linkProgram();

    
	this->device_id = device_id;

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
}

bool Input::start(BMDDisplayMode mode, bool use_rgb_colorspace)
{
	stop();
	
	bNewBuffer = false;
	bNewFrame = false;
	timestamp = 0;
	lastFrameNo = -1;
    b_use_rgb_colorspace = use_rgb_colorspace;
	
	this->mode = (_BMDDisplayMode)mode;
	
    BMDPixelFormat pixelFormat = b_use_rgb_colorspace ? bmdFormat8BitARGB : bmdFormat8BitYUV;
	
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
	
    if (b_use_rgb_colorspace) {
        pix_back.allocate(width, height, 4);
        pix_back.set(255);
        pix_front.allocate(width, height, 4);
        pix_front.set(255);
    } else {
        pix_back.allocate(width, height, 2);
        pix_front.allocate(width, height, 2);
    }
	
	pDLDisplayMode->GetFrameRate(&frameDuration, &frameTimescale);
	
	pDLInput->DisableAudioInput();
    
    // check features
    IDeckLinkAttributes* attr;
    bool format_detection = false;
    if (pDL->QueryInterface(IID_IDeckLinkAttributes, (void**)&attr) == S_OK) {
        attr->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &format_detection);
        ofLogVerbose() << "Input format detection feature : " << format_detection;
    }

    if (pDLInput->EnableVideoInput(pDLDisplayMode->GetDisplayMode(), pixelFormat, format_detection ? bmdVideoInputEnableFormatDetection : bmdVideoInputFlagDefault) != S_OK) {
        ofLogError("ofxDeckLinkAPI::Input") << "invalid enable video input";
        return false;
    }
	
    if (pDLInput->StartStreams() != S_OK) {
        ofLogError("ofxDeckLinkAPI::Input") << "invalid start streams";
        return false;
    }
	
	shader.begin();
	shader.setUniform1i("use_odd", 0);
	shader.end();
	
    shader_argb.begin();
    shader_argb.setUniform1i("use_odd", 0);
    shader_argb.end();
    
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
    BMDPixelFormat pixelFormat = bmdFormat8BitYUV;
    
    // Check for video field changes
    if (notificationEvents & bmdVideoInputFieldDominanceChanged)
    {
        BMDFieldDominance fieldDominance;
        
        fieldDominance = newDisplayMode->GetFieldDominance();
        printf("Input field dominance changed to ");
        switch (fieldDominance) {
            case bmdUnknownFieldDominance:
                printf("unknown\n");
                break;
            case bmdLowerFieldFirst:
                printf("lower field first\n");
                break;
            case bmdUpperFieldFirst:
                printf("upper field first\n");
                break;
            case bmdProgressiveFrame:
                printf("progressive\n");
                break;
            case bmdProgressiveSegmentedFrame:
                printf("progressive segmented frame\n");
                break;
            default:
                break;
        }
    }
    
    // Check if the pixel format has changed
    if (notificationEvents & bmdVideoInputColorspaceChanged)
    {
        printf("Input color space changed to ");
        if (detectedSignalFlags == bmdDetectedVideoInputYCbCr422)
        {
            printf("YCbCr422\n");
            pixelFormat = bmdFormat8BitYUV;
            b_use_rgb_colorspace = false;
        }
        if (detectedSignalFlags == bmdDetectedVideoInputRGB444)
        {
            printf("RGB444\n");
            pixelFormat = bmdFormat8BitARGB;
            b_use_rgb_colorspace = true;
        }
    }
    
    // Check if the video mode has changed
    if (notificationEvents & bmdVideoInputDisplayModeChanged)
    {
        width = newDisplayMode->GetWidth();
        height = newDisplayMode->GetHeight();
    }
    
    if (b_use_rgb_colorspace) {
        pix_back.allocate(width, height, 4);
        pix_back.set(255);
        pix_front.allocate(width, height, 4);
        pix_front.set(255);
    } else {
        pix_back.allocate(width, height, 2);
        pix_front.allocate(width, height, 2);
    }
    
    
    // Pause video capture
    pDLInput->PauseStreams();
    
    // Enable video input with the properties of the new video stream
    pDLInput->EnableVideoInput(newDisplayMode->GetDisplayMode(), pixelFormat, bmdVideoInputEnableFormatDetection);
    
    // Flush any queued video frames
    pDLInput->FlushStreams();
    
    // Start video capture
    pDLInput->StartStreams();
}

HRESULT Input::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket)
{
	valid_signal = (videoFrame->GetFlags() & bmdFrameHasNoInputSource) == 0;
	if (!valid_signal) {
		return S_OK;
	}
	timestamp = ofGetSystemTime();
	
    BMDPixelFormat format = videoFrame->GetPixelFormat();
    IDeckLinkTimecode* timecode_ptr = nullptr;
    unsigned char *src = NULL;
	videoFrame->GetBytes((void**)&src);
	
	unsigned char *dst = pix_back.getData();
    if (format == bmdFormat8BitARGB) {
        memcpy(dst, &src[1], width * height * 4 - 1);
    } else {
        memcpy(dst, src, width * height * 2);
    }
	
	mutex->lock();
	pix_back.swap(pix_front);
    if (videoFrame->GetTimecode(bmdTimecodeRP188Any, &timecode_ptr) == S_OK) {
        timecode_ptr->GetComponents(&tc_back.hours,
                                    &tc_back.minutes,
                                    &tc_back.seconds,
                                    &tc_back.frames);
        BMDTimecodeFlags flags = timecode_ptr->GetFlags();
        if (flags == bmdTimecodeIsDropFrame) {
            tc_back.b_drop_frame = true;
        }
    }
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
		tex.loadData(pix_front);
        tc_front = tc_back;
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
		
		if (auto_search_tick == (int)(0.8 * auto_search_tick_interval)) {
			stop();
			close();
		}
		if (auto_search_tick == (int)(0.9 * auto_search_tick_interval)) {
			initDeckLink(device_id);
		}
		
		auto_search_tick++;
		auto_search_tick %= auto_search_tick_interval;
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
    if (b_use_rgb_colorspace) {
        bool bprog = draw_mode == DRAWMODE_PROGRESSIVE;
        const ofShader* s = bprog ? &shader_argb_prog : &shader_argb;
        s->begin();
        if (!bprog) {
            bool bufield = draw_mode == DRAWMODE_UPPERFIELD || (draw_mode == DRAWMODE_AUTOFIELD && isFrameNew());
            s->setUniform1i("use_odd", bufield ? 0 : 1);
        }
        tex.draw(x, y, w, h);
        s->end();
    } else {
        bool bprog = draw_mode == DRAWMODE_PROGRESSIVE;
        const ofShader* s = bprog ? &shader_prog : &shader;
        s->begin();
        if (!bprog) {
            bool bufield = draw_mode == DRAWMODE_UPPERFIELD || (draw_mode == DRAWMODE_AUTOFIELD && isFrameNew());
            s->setUniform1i("use_odd", bufield ? 0 : 1);
        }
        tex.draw(x, y, w, h);
        s->end();
    }
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

string Input::getTimecodeString() const
{
    if (tc_front.b_drop_frame) {
        return ofVAArgsToString("%02d:%02d:%02d;%02d",
                                tc_front.hours,
                                tc_front.minutes,
                                tc_front.seconds,
                                tc_front.frames);
    } else {
        return ofVAArgsToString("%02d:%02d:%02d:%02d",
                                tc_front.hours,
                                tc_front.minutes,
                                tc_front.seconds,
                                tc_front.frames);
    }
}


OFX_DECKLINK_API_END_NAMESPACE