#include "ofxDeckLink.h"

OFX_DECKLINK_API_BEGIN_NAMESPACE

vector<Device> listDevice()
{
    vector<Device> devinfo;
    
	IDeckLinkIterator* deckLinkIterator = CreateDeckLinkIteratorInstance();
	if (deckLinkIterator == NULL)
	{
		ofLogError("ofxDeckLinkAPI::Monitor") << "This application requires the DeckLink drivers installed." << "\n" << "Please install the Blackmagic DeckLink drivers to use the features of this application.";
	}
	
	cout << "==== ofxDeckLinkAPI::listDevice() ====" << endl;
	
	IDeckLink* deckLink = NULL;
	HRESULT result;
	int num_device = 0;
	
	while (deckLinkIterator->Next(&deckLink) == S_OK)
	{
        Device dev;
		CFStringRef deviceNameCFString = NULL;
		
		result = deckLink->GetModelName(&deviceNameCFString);
		if (result == S_OK)
		{
			char deviceName[64];
			CFStringGetCString(deviceNameCFString, deviceName, sizeof(deviceName), kCFStringEncodingUTF8);
			cout << num_device << ": " << deviceName;
			CFRelease(deviceNameCFString);
            dev.model_name = (string)deviceName;
		}
        IDeckLinkAttributes* attr = NULL;
        deckLink->QueryInterface(IID_IDeckLinkAttributes, (void**)&attr);
        if (attr != NULL) {
            {
                int64_t v = 0;
                if (attr->GetInt(BMDDeckLinkPersistentID, &v) == S_OK) {
                    dev.persistent_id = v;
                    cout << ", pid:" << v;
                }
            }
            {
                int64_t v = 0;
                if (attr->GetInt(BMDDeckLinkTopologicalID, &v) == S_OK) {
                    dev.topological_id = v;
                    cout << ", tid:" << v;
                }
            }
        }
        cout << endl;
        devinfo.push_back(dev);
		
		deckLink->Release();
		
		num_device++;
	}
	
	if (num_device == 0)
		cout << "device not found" << endl;
	
	cout << "======================================" << endl << endl;
	
	if (deckLinkIterator != NULL)
	{
		deckLinkIterator->Release();
		deckLinkIterator = NULL;
	}
    
    return devinfo;
}

OFX_DECKLINK_API_END_NAMESPACE
