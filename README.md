# ofxDeckLink
Blackmagic DeckLink SDK addon for openFrameworks

- Original entire code was written by [Satoru Higa](https://github.com/satoruhiga)
- Some optimizations, GPU color conversion and GPU deinterlace functions by [Yuya Hanai](https://github.com/hanasaan)

### Features
- Features are similar to [ofxBlackmagic](https://github.com/kylemcdonald/ofxBlackmagic), but this addon supports
 - DeckLink Output for video output
 - DeckLink Input with Fast GPU color conversion and deinterlace with one shader pass (but working in progress..)

### Known Issues
- In input, render with scaling does not work. (Shader issue)
 - If you want to do scale, first draw to fbo whose size is same as video input and then draw the fbo with scaling.

### Supported System
- oF 0.9.0 + DesktopVideo_10.5.2 on OSX
- Tested on 
 - Blackmagic Mini Recorder + OSX 10.10 on Mac Book Pro Retina 2015 
 - Blackmagic Ultra Studio 3D + OSX 10.10 on Mac Book Pro Retina 2015 
 - Blackmagic Mini Recorder + OSX 10.9 on Mac Pro 2013
 - Blackmagic Ultra Studio 3D + OSX 10.9 on Mac Pro 2013
 - Blackmagic Ultra Studio 4k + OSX 10.9 on Mac Pro 2013
 - Blackmagic Ultra Studio 4k Extreme + OSX 10.9 on Mac Pro 2013

### Installation
- Go to [Blackmagic Support](https://www.blackmagicdesign.com/support) for downloading DesktopVideo Software.
- This addon was tested with DesktopVideo version 10.5.2.
