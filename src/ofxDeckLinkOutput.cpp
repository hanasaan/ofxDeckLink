#include "ofxDeckLinkOutput.h"

OFX_DECKLINK_API_BEGIN_NAMESPACE

Output::Output() : pDL(NULL), pDLOutput(NULL), pDLVideoFrame(NULL), has_new_frame(false), mutex(NULL)
{
}

bool Output::setup(int device_id)
{
	close();
	
	mutex = new ofMutex;
	
	return initDeckLink(device_id);
}

bool Output::start(BMDDisplayMode mode)
{
	return startDeckLink(mode);
}

void Output::close()
{
	if (pDLOutput != NULL)
	{
		pDLOutput->Release();
		pDLOutput = NULL;
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

bool Output::initDeckLink(int device_id)
{
	bool bSuccess = FALSE;
	IDeckLinkIterator* pDLIterator = NULL;
	
	pDLIterator = CreateDeckLinkIteratorInstance();
	if (pDLIterator == NULL)
	{
		ofLogError("ofxDeckLinkAPI::Output") << "This application requires the DeckLink drivers installed." << "\n" << "Please install the Blackmagic DeckLink drivers to use the features of this application.";
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
		ofLogError("ofxDeckLinkAPI::Output") << "This application requires a DeckLink device." << "\n" << "You will not be able to use the features of this application until a DeckLink PCI card is installed.";
		goto error;
	}
	
	if (pDL->QueryInterface(IID_IDeckLinkOutput, (void**)&pDLOutput) != S_OK)
		goto error;
	
	if (pDLOutput->SetScheduledFrameCompletionCallback(this) != S_OK)
		goto error;
	
	bSuccess = TRUE;
	
error:
	
	if (!bSuccess)
	{
		if (pDLOutput != NULL)
		{
			pDLOutput->Release();
			pDLOutput = NULL;
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

bool Output::startDeckLink(BMDDisplayMode mode)
{
	IDeckLinkDisplayModeIterator* pDLDisplayModeIterator = NULL;
	IDeckLinkDisplayMode* pDLDisplayMode = NULL;
	
	if (pDLOutput->GetDisplayModeIterator(&pDLDisplayModeIterator) == S_OK)
	{
		while (pDLDisplayModeIterator->Next(&pDLDisplayMode) == S_OK)
		{
			if (pDLDisplayMode->GetDisplayMode() == mode)
			{
				break;
			}
		}
		
		pDLDisplayModeIterator->Release();
	}
	
	if (!pDLDisplayMode)
	{
		ofLogError("ofxDeckLinkAPI::Output") << "invalid display mode";
		return false;
	}
	
	uiFrameWidth = pDLDisplayMode->GetWidth();
	uiFrameHeight = pDLDisplayMode->GetHeight();
	
	pixels[0].allocate(uiFrameWidth, uiFrameHeight, 4);
	pixels[1].allocate(uiFrameWidth, uiFrameHeight, 4);
	
	front_buffer = &pixels[0];
	back_buffer = &pixels[1];
	
	pDLDisplayMode->GetFrameRate(&frameDuration, &frameTimescale);
	
	uiFPS = ((frameTimescale + (frameDuration-1))  /  frameDuration);
	
	if (pDLOutput->EnableVideoOutput(pDLDisplayMode->GetDisplayMode(), bmdVideoOutputFlagDefault) != S_OK)
		return false;
	
	if (pDLOutput->CreateVideoFrame(uiFrameWidth, uiFrameHeight, uiFrameWidth*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &pDLVideoFrame) != S_OK)
		return false;
	
	uiTotalFrames = 0;
	
	resetFrame();
	setPreroll();
	
	pDLOutput->StartScheduledPlayback(0, frameTimescale, 1);
	
	return true;
}

void Output::resetFrame()
{
	void* pFrame;
	pDLVideoFrame->GetBytes((void**)&pFrame);
	memset(pFrame, 0x00, pDLVideoFrame->GetRowBytes() * uiFrameHeight);
}

void Output::setPreroll()
{
	for (uint32_t i=0; i < 3; i++)
	{
		if (pDLOutput->ScheduleVideoFrame(pDLVideoFrame, (uiTotalFrames * frameDuration), frameDuration, frameTimescale) != S_OK)
			return;
		uiTotalFrames++;
	}
}

//

void Output::listDisplayMode()
{
	IDeckLinkDisplayModeIterator* pDLDisplayModeIterator = NULL;
	IDeckLinkDisplayMode* pDLDisplayMode = NULL;
	HRESULT result;
	
	// Get first available video mode for Output
	if (pDLOutput->GetDisplayModeIterator(&pDLDisplayModeIterator) != S_OK)
	{
		ofLogError("ofxDeckLinkAPI::Output") << "no output device found";
		return;
	}
	
	cout << "==== Output::listDisplayMode() ====" << endl;
	
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


void Output::publishTexture(ofTexture &tex)
{
	assert(mutex);
	
	if (tex.getWidth() == uiFrameWidth
		&& tex.getHeight() == uiFrameHeight)
	{
		mutex->lock();
		tex.readToPixels(*back_buffer);
		
		if (back_buffer->getNumChannels() != 4)
			back_buffer->setNumChannels(4);
		
		has_new_frame = true;
		
		mutex->unlock();
	}
	else
		ofLogError("ofxDeckLinkAPI::Output") << "invalid texture size";
}

void Output::publishPixels(ofPixels &pix)
{
	assert(mutex);
	
	if (pix.getWidth() == uiFrameWidth
		&& pix.getHeight() == uiFrameHeight)
	{
		mutex->lock();
		*back_buffer = pix;
		
		if (back_buffer->getNumChannels() != 4)
			back_buffer->setNumChannels(4);
		
		has_new_frame = true;
		
		mutex->unlock();
	}
	else
		ofLogError("ofxDeckLinkAPI::Output") << "invalid pixel size";
}

//

HRESULT Output::ScheduledFrameCompleted(IDeckLinkVideoFrame *completedFrame, BMDOutputFrameCompletionResult result)
{
	void* pFrame = NULL;
	pDLVideoFrame->GetBytes((void**)&pFrame);
	assert(pFrame != NULL);
	
	int num_schedule_frame = 1;
	
	if (result != bmdOutputFrameCompleted)
	{
		num_schedule_frame = 2;
	}
	
	mutex->lock();
	if (has_new_frame)
	{
		has_new_frame = false;
		std::swap(*front_buffer, *back_buffer);
	}
	mutex->unlock();
	
	memcpy(pFrame, front_buffer->getPixels(), uiFrameWidth * uiFrameHeight * front_buffer->getNumChannels());
	
	for (int i = 0; i < num_schedule_frame; i++)
	{
		if (pDLOutput->ScheduleVideoFrame(pDLVideoFrame, (uiTotalFrames * frameDuration), frameDuration, frameTimescale) == S_OK)
			uiTotalFrames++;
	}
	
	return S_OK;
}

OFX_DECKLINK_API_END_NAMESPACE