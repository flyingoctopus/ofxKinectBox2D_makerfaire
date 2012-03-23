#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxBox2d.h"
#include "ofxTriangle.h"
#include "contourSimplify.h"

class testApp : public ofBaseApp
{
	
public:
    
    typedef struct {
        
        vector <ofVec2f> contourSmooth2;
        
    } contourContainer;
    
    void setup();
    void update();
    void draw();
    void reset();
    void exit();
    
    void keyPressed  (int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    
    void drawCanny();
    void triangulateContour();
    void cannyDetect();
    void smoothContour();
    void drawPencilContours();
    void drawBlobInsideMe(ofxCvGrayscaleImage &img, ofxCvBlob &blob);
    void drawLetters();
    
    void calcDepthOverlay();
    void drawDepthOverlay();
    
    
    ofxKinect                   kinect;
    ofTexture                   texture;
    
    ofxCvColorImage             colorImg;
    ofxCvGrayscaleImage         grayImage;
    ofxCvGrayscaleImage         grayThresh;
    ofxCvGrayscaleImage         grayThreshFar;
    ofxCvGrayscaleImage         grayCanny;
    ofxCvGrayscaleImage         debugImage;
    ofxCvGrayscaleImage         depthImg;
    ofxCvColorImage             depthImg2;
    ofxCvGrayscaleImage         cvimg;
    ofxCvGrayscaleImage         edges;
    ofxCvGrayscaleImage         depthOverlay;
    
    ofImage                     m_ltr;
    ofImage                     a_ltr;
    ofImage                     k_ltr;
    ofImage                     e_ltr;
    
    contourSimplify             contourSimp;
    
    vector <ofVec2f>            contourReg;
    vector <ofVec2f>            contourSmooth;
    vector <contourContainer>   testCont;
    
    ofxCvContourFinder          contourFinder;
    
    bool                        bThreshWithOpenCV;
    bool                        drawPC;
    bool                        depthDrawOn;
    bool                        cannyOn;
    
    int                         nearThreshold;
    int                         farThreshold;
    int                         angle;
    int                         numRect;
    
    ofxBox2d                    physics;
    vector <ofxBox2dRect>       rects;
    vector <ofxBox2dCircle>     circles;
    vector <ofxBox2dPolygon>    polys;
    int                         nbCircles;
    ofxTriangle                 triangle;
    
    vector <int>                rects_dim;
    
    vector <int>                rects_ltr;
};

#endif