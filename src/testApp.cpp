#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup()
{
	
	kinect.init();
	kinect.setVerbose(true);
	kinect.open();
	
#ifdef TARGET_OSX    
	// Get the absolute location of the executable file in the bundle.
	CFBundleRef appBundle     = CFBundleGetMainBundle();
	CFURLRef   executableURL = CFBundleCopyExecutableURL(appBundle);
	char execFile[4096];
	if (CFURLGetFileSystemRepresentation(executableURL, TRUE, (UInt8 *)execFile, 4096))
	{
		// Strip out the filename to just get the path
		string strExecFile = execFile;
		int found = strExecFile.find_last_of("/");
		string strPath = strExecFile.substr(0, found);
		
		// Change the working directory to that of the executable
		if(-1 == chdir(strPath.c_str())) {
			ofLog(OF_LOG_ERROR, "Unable to change working directory to executable's directory.");
		}
	}
	else {
		ofLog(OF_LOG_ERROR, "Unable to identify executable's directory.");
	}
	CFRelease(executableURL);
#endif
	

	//Loading images of Make letters
	m_ltr.loadImage("m.jpg");	
	a_ltr.loadImage("a.jpg");
	k_ltr.loadImage("k.jpg");
	e_ltr.loadImage("e.jpg");
    
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayCanny.allocate(kinect.width,kinect.height);
	grayThresh.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	cvimg.allocate(kinect.width,kinect.height);
	edges.allocate(kinect.width,kinect.height);
	debugImage.allocate(kinect.width,kinect.height);
	depthOverlay.allocate(kinect.width,kinect.height);
	depthImg.allocate(kinect.width,kinect.height);
	depthImg2.allocate(kinect.width,kinect.height);
	
	
	//Setting threshold values for person detection
	nearThreshold = 15;
	farThreshold  = 200;
	bThreshWithOpenCV = true;
	
	ofSetFrameRate(60);
	
	angle = 0;
	kinect.setCameraTiltAngle(angle);
	
	drawPC = false;
	
	//Initializing Box2D variables
	physics.init();
	physics.createBounds();
	physics.setGravity(0, 10);
	physics.checkBounds(true);
	
	//Canny Edge Finding on?
	cannyOn = false;
	
	//Drawing Depth field on?
	depthDrawOn = true;
	
	numRect=0;
}



//--------------------------------------------------------------
void testApp::update()
{
	//ofBackground(33,33,33);
	ofBackground(255, 255, 255);
	
	kinect.update();
    
	grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
	grayImage.mirror(false, true);
	
	depthImg = grayImage;
	
	if(cannyOn) cannyDetect();
	if(depthDrawOn) calcDepthOverlay();
	
	
	if( bThreshWithOpenCV ){
		grayThreshFar = grayImage;
		grayThresh = grayImage;
		grayThreshFar.threshold(farThreshold, true);
		grayThresh.threshold(nearThreshold);
		cvAnd(grayThresh.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
	}else{
		
		unsigned char * pix = grayImage.getPixels();
		int numPixels = grayImage.getWidth() * grayImage.getHeight();
		
		for(int i = 0; i < numPixels; i++){
			if( pix[i] > nearThreshold && pix[i] < farThreshold ){
				pix[i] = 255;
			}else{
				pix[i] = 0;
			}
		}
	}
	
	//update the cv image
	grayImage.flagImageChanged();
	
	//find the contours
    contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height), 20, true);
	
	//Update mask image for canny overlay.
	debugImage.set(0);
	for (int i = 0; i < contourFinder.nBlobs; i++){
		drawBlobInsideMe(debugImage, contourFinder.blobs[i]);
	}
	
	// smoothContour();
	triangulateContour();
	
	while(numRect<=100){
		
		ofxBox2dRect rectangle;
		rectangle.setPhysics(ofRandom(2.0, 10.0), ofRandom(0.5, 1.0), 0.08);
        
		int use = (rand()%4)+1;
//		float dim = 64;
        int dim = ofRandom(10,40);
        
        ofLog(OF_LOG_WARNING, "dim: " + ofToString(dim));
        
		rectangle.setup(physics.getWorld(), ofGetWidth()/2.0, 20, dim, dim);
		rects.push_back(rectangle);
		rects_ltr.push_back(use);
        rects_dim.push_back(dim);
		numRect++;
        
	}
	
	physics.update();
	
	
	
}

//--------------------------------------------------------------
void testApp::calcDepthOverlay(){
	
	unsigned char* maskPix = debugImage.getPixels();
	unsigned char* depthPix = depthImg.getPixels();
	unsigned char* depthPix2 = depthImg2.getPixels();
	
	for(int i=0;i<(debugImage.width*debugImage.height);i++){
		
		if(maskPix[i] == 0){
			depthPix[i] = 0;
			
			depthPix2[i*3] =255;
			depthPix2[i*3 +1] =255;
			depthPix2[i*3 +2] =255;
			
			
		}
		
		
		else{
			
			float colorVal = ofMap(depthPix[i], 0, 255, 125, 255);
			depthPix2[i*3] = colorVal;
			depthPix2[i*3 +1] = colorVal;
			depthPix2[i*3 +2] = colorVal;
			
		}
		
		
	}
	
	depthImg2.setFromPixels(depthPix2, kinect.width, kinect.height);
	depthImg2.flagImageChanged();
	
}

//--------------------------------------------------------------
void testApp::drawDepthOverlay(){
	ofEnableAlphaBlending();
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	glEnable(GL_LINE_SMOOTH);
	//depthOverlay.invert();
	//depthOverlay.draw(0,0,ofGetWidth(),ofGetHeight());
	
	depthImg2.contrastStretch();
	depthImg2.draw(0, 0,ofGetWidth(),ofGetHeight());
    
	ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void testApp::cannyDetect(){
	
	colorImg = kinect.getCalibratedRGBPixels();
	grayCanny = colorImg;
	grayCanny.mirror(0, 1);
	cvimg = grayCanny;
	
	int lowThresh = 50;
	int highThresh = 100;
	int aperture = 2;
	
	
	// Run cvCanny.
	cvCanny(
			cvimg.getCvImage(), // input image
			edges.getCvImage(), // edges output
			80,
			ofMap(mouseY, 0, ofGetHeight(), 0, 200),3); // default = 3
	
	
	unsigned char* maskPix = debugImage.getPixels();
	unsigned char* cannyPix = edges.getPixels();
    
	
	for(int i=0;i<(debugImage.width*debugImage.height);i++){
		
		if(maskPix[i] == 0){
			cannyPix[i] = 0;
			
			
		}
		
	}
	
	edges.setFromPixels(cannyPix, kinect.width, kinect.height);
	edges.flagImageChanged();
	
}

//--------------------------------------------------------------
void testApp::triangulateContour(){
	
	triangle.clear();
	
	int i;
	
	for (i=0; i<contourFinder.nBlobs; i++)
	{
		triangle.triangulate(contourFinder.blobs[i], max( 3.0f, (float)contourFinder.blobs[i].pts.size()/12));
	}
	
	// update physics
	
	for (i=polys.size()-1; i>=0; i--) {
		physics.world->DestroyBody(polys[i].body);
	}
	
	polys.clear();
	
	
	//Triangulate contour in order to add to box2d
	ofxTriangleData* tData;
	for (int i=triangle.triangles.size()-1; i>=0; i--) {
		
		tData = &triangle.triangles[i];
		
		ofxBox2dPolygon poly;
		
		ofPoint t1,t2,t3;
		
		t1.x=ofMap(tData->a.x, 0, grayImage.width, 0, ofGetWidth());
		t1.y=ofMap(tData->a.y, 0, grayImage.height, 0, ofGetHeight());
		t2.x = ofMap(tData->b.x, 0, grayImage.width, 0, ofGetWidth());
		t2.y=ofMap(tData->b.y, 0, grayImage.height, 0, ofGetHeight());
		
		t3.x = ofMap(tData->c.x, 0, grayImage.width, 0, ofGetWidth());
		t3.y = ofMap(tData->c.y, 0, grayImage.height, 0, ofGetHeight());
		
		poly.addVertex(t1.x,t1.y);
		poly.addVertex(t2.x,t2.y);
		poly.addVertex(t3.x,t3.y);
		
        
		if(poly.validateShape()) {
			poly.init();
			poly.createShape(physics.world, 0.0f, 0.0f);				
			polys.push_back(poly);
		}
	}
}

//--------------------------------------------------------------
void testApp::smoothContour(){
	
	int numBlobs = contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, true);
	
	//Simplifying contour..
	
	 /*if(numBlobs > 0){			
	 
	 for(int i=0;i<numBlobs;i++){
	 
	 int length_of_contour = contourFinder.blobs[i].pts.size();
	 
	 contourReg.clear();
	 contourReg.assign(length_of_contour, ofVec2f());
	 contourSmooth.clear();
	 contourSmooth.assign(length_of_contour, ofVec2f());
	 
	 //lets make a copy for ourselves
	 for(int j = 0; j < length_of_contour; j++){
	 contourReg[j] = contourFinder.blobs[i].pts[j];
	 }
	 
	 contourSimp.smooth(contourReg, contourSmooth, 0.2);
	 
	 
	 
	 }
	 
	 }*/
	 
	//lets get out the contour data
	
	//clear the old contours
	
	
}

//--------------------------------------------------------------
void testApp::drawBlobInsideMe(ofxCvGrayscaleImage &img, ofxCvBlob &blob) {
	
	int color = 255;
	
	if( blob.pts.size() > 0 ){ //&& blob.hole==true) {
		CvPoint* pts = new CvPoint[blob.nPts];
		for( int i=0; i < blob.nPts ; i++ ) {
			pts[i].x = (int)blob.pts[i].x;
			pts[i].y = (int)blob.pts[i].y;
		}
		int nPts = blob.nPts;
		
		cvFillPoly( img.getCvImage(), &pts, &nPts, 1, CV_RGB(color,color,color));
		//cvPolyLine( img.getCvImage(), &pts, &nPts, 1, false, CV_RGB(color,color,color), 4);
		delete pts;
	}
	
}

//--------------------------------------------------------------
void testApp::drawCanny(){
	ofEnableAlphaBlending();
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	glEnable(GL_LINE_SMOOTH);
	edges.invert();
	edges.draw(0,0,ofGetWidth(),ofGetHeight());
	ofDisableAlphaBlending();	
}

//--------------------------------------------------------------
void testApp::drawLetters(){
	
	ofLog(OF_LOG_WARNING, "drawing letters... ");
    
	int nbRects = rects.size();

	for(int i=0;i<nbRects;i++){
        		ofPushMatrix();
		ofTranslate(rects[i].getPosition().x, rects[i].getPosition().y, 0);
		ofRotate(rects[i].getRotation());
		ofSetColor(255, 255, 255);
		
		int use = rects_ltr[i];
		
		
		switch (use) {
			case 1:
				m_ltr.draw(-rects_dim[i],-rects_dim[i], rects_dim[i]*2, rects_dim[i]*2);
				break;
			case 2:
				a_ltr.draw(-rects_dim[i],-rects_dim[i], rects_dim[i]*2, rects_dim[i]*2);
				break;
			case 3:
				k_ltr.draw(-rects_dim[i],-rects_dim[i], rects_dim[i]*2, rects_dim[i]*2);
				break;
			case 4:
				e_ltr.draw(-rects_dim[i],-rects_dim[i], rects_dim[i]*2, rects_dim[i]*2);
				break;
		}
		
		
		for(int j=0;j<1;j++){
			
			ofSetColor(60*j, 60*j, 60*j);	
			
			float pctW = 0.05;
			float tL = ofRandom(-rects_dim[i]*pctW, rects_dim[i]*pctW);
			float tR = ofRandom(-rects_dim[i]*pctW, rects_dim[i]*pctW);
			float bL = ofRandom(-rects_dim[i]*pctW, rects_dim[i]*pctW);
			float bR = ofRandom(-rects_dim[i]*pctW, rects_dim[i]*pctW);
			ofBeginShape();
			ofVertex(-rects_dim[i]+tL, -rects_dim[i]+tR);
			ofVertex(rects_dim[i]+bL, -rects_dim[i]+tR);
			ofVertex(rects_dim[i]+tL, rects_dim[i]+bR);
			ofVertex(-rects_dim[i]+bL, rects_dim[i]+tL);
			ofEndShape(true);
		}
		
		
		
		
		
		ofPopMatrix();
		
	}
    
	
}

//--------------------------------------------------------------
void testApp::draw()
{
//    m_ltr.draw(0,0,0);
//    a_ltr.draw(65,0,0);
//    k_ltr.draw(129,0,0);
//    e_ltr.draw(181,0,0);
    
	ofSetColor(255, 255, 255);
	//depthImg.draw(0, 0,ofGetWidth(),ofGetHeight());
	
	if(cannyOn) drawCanny();
	if(depthDrawOn) drawDepthOverlay();
	/*
     char reportStr[1024];
     sprintf(reportStr, "using opencv threshold = %i (press spacebar)\nset near threshold %i (press: + -)\nset far threshold %i (press: < >) num blobs found %i, fps: %f",bThreshWithOpenCV, nearThreshold, farThreshold, contourFinder.nBlobs, ofGetFrameRate());
     ofDrawBitmapString(reportStr, 20, 690);
     */
	
	//Drawing Circles
	int i;
	nbCircles = circles.size();
	
	for(i=0; i<nbCircles; i++){
		ofSetColor(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
		ofFill();
		ofCircle(circles[i].getPosition().x, circles[i].getPosition().y, circles[i].getRadius());
		ofNoFill();
	}
    
	//Drawing Rectangles
	drawLetters();
	
	//Drawing Outlines of world
	drawPencilContours();
	
	ofSetHexColor(0xFFFFFF);
	
}

//--------------------------------------------------------------
void testApp::drawPencilContours(){
	
	//contourFinder.drawAlt(0,0,ofGetWidth(),ofGetHeight());	
	
	int numBlobs = contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, true);
	
	for(int i=0;i<6;i++){
		
		
		if(numBlobs > 0){			
			
			
			for(int j=0;j<numBlobs;j++){
				
				//lets get out the contour data
				int length_of_contour = contourFinder.blobs[j].pts.size();
				
				//clear the old contours
				contourReg.clear();
				contourReg.assign(length_of_contour, ofVec2f());
				contourSmooth.clear();
				contourSmooth.assign(length_of_contour, ofVec2f());
				
				//lets make a copy for ourselves
				for(int k = 0; k < length_of_contour; k++){
					contourReg[k] = contourFinder.blobs[j].pts[k];
				}
				
				contourSimp.smooth(contourReg, contourSmooth, 0.1*i);
				
				
				glPushMatrix();
				
				
//				float p = 255/6;
                float p = 42;
				
				ofSetLineWidth(0.25);
				
				ofSetColor(0,0,p*i);		
				ofBeginShape();
				
				for(int i = 0; i < contourSmooth.size(); i++){
					float xC = ofMap(contourSmooth[i].x, 0, 640, 0, ofGetWidth());
					float yC = ofMap(contourSmooth[i].y, 0, 480, 0, ofGetHeight());
					ofVertex(xC,yC);
				}
				
				ofEndShape(true);
				ofNoFill();
				glPopMatrix();
				
			}
		}
	}
	
	
}

//--------------------------------------------------------------
void testApp::reset(){
	numRect = 0;
    for(int i=0; i < rects.size(); ++i) {
//        rects[i] = 0;
    }
}


//--------------------------------------------------------------
void testApp::exit(){
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key)
{
	switch (key)
	{
			
			
		case 'c': {
			ofxBox2dCircle circle;
			
			circle.setPhysics(ofRandom(2.0, 10.0), ofRandom(0, 1.0), 0.08);
			//circle.setPhysics(3.0, 0.53, 0.1);
			circle.setup(physics.getWorld(), mouseX, mouseY,ofRandom(0, 20));
			circles.push_back(circle);
			break;
		}	
			
		case 'b': {
			//ofxBox2dCircle circle;
			ofxBox2dRect rectangle;
			rectangle.setPhysics(ofRandom(2.0, 10.0), ofRandom(0.5, 1.0), 0.08);
			
			int use = (rand()%4)+1;
			
			
			int dim = ofRandom(0, 20);
			rectangle.setup(physics.getWorld(), mouseX, mouseY, dim, dim);
            rects_dim.push_back(dim);
			rects.push_back(rectangle);
			rects_ltr.push_back(use);
            
			
			break;
		}
			
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
			break;
		case'p':
			drawPC = !drawPC;
			break;
			
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
		case '<':		
		case ',':		
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
		case '-':		
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
			
		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
            
        case 'f':
			ofToggleFullscreen();
			break;
            
        case 'r':
//            reset();
            break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}

