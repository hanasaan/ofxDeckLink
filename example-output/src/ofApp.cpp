#include "ofMain.h"

#include "ofxDeckLink.h"

class ofApp : public ofBaseApp{
    ofxDeckLinkAPI::Output output;
    ofFbo test_fbo;
    ofImage img;
    ofTrueTypeFont font;
public:
    void setup()
    {
        ofSetVerticalSync(true);
        ofSetFrameRate(30);
        
        if (output.setup(0)) {
            output.start(bmdModeHD1080p2997);
        }
        
        font.load("verdana.ttf", 128);
        img.load("bar.png");
        test_fbo.allocate(1920, 1080, GL_RGBA, 0);
    }
    
    void update()
    {
        test_fbo.begin();
        img.draw(0, 0);
        font.drawString(ofToString(ofGetFrameNum()), 10, 200);
        test_fbo.end();
        
        output.publishTexture(test_fbo.getTexture());
    }
    
    void draw()
    {
        ofClear(0);
        test_fbo.draw(0, 0);
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
    }
    
    void keyPressed(int key)
    {
        if (key == 'f') {
            ofToggleFullscreen();
        }
    }
    
    void keyReleased(int key) {}
    void mouseMoved(int x, int y ) {}
    void mouseDragged(int x, int y, int button) {}
    void mousePressed(int x, int y, int button) {}
    void mouseReleased(int x, int y, int button) {}
    void windowResized(int w, int h) {}
    void dragEvent(ofDragInfo dragInfo) {}
    void gotMessage(ofMessage msg) {}
    
};

//========================================================================
int main( ){
    ofSetupOpenGL(1280,720,OF_WINDOW);            // <-------- setup the GL context
    
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());
    
}
