#ifndef _VIDEO_HAND_FINGER_FINDER_
#define _VIDEO_HAND_FINGER_FINDER_

#include "ofMain.h"
#include "ofxCvMain.h"
#include "ofxVectorMath.h"



//-----------------------------------------------|
class ofxFingerDetector{
	
	public:
	
	ofxFingerDetector();
	
	bool findFingers(ofxCvBlob blob);
	bool findHands(ofxCvBlob smblob);
	void draw(float x, float y);
	void drawhands(float x, float y);
	
	
	void findFarthestPoint(vector<ofVec2f> hand, ofPoint centroid, int position, int i);
	
	float dlh,max;
	
	int handPositions[2];
	
	vector  <ofVec2f>		fingerPoints;
	vector  <ofVec2f>		handPoints;
	
	vector	<float>				fingersPointCurve;
	vector	<float>				handPointCurve;
	
	vector	<bool>				bfingerRuns;
	
	vector  <ofVec2f>		leftHand;
	vector  <ofVec2f>		rightHand;
	
	ofxVec2f	v1, v2, aux1;
	 
	ofxVec3f	v1D, vxv;
	ofxVec3f	v2D;
	 
	int k,smk;
	
	ofPoint handCentroid;
	
	 float teta,lhd;

};
//-----------------------------------------------|


#endif	
