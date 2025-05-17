#include "ofApp.h"
#include "ofGraphics.h"
#include "ofImage.h"
#include "ofTexture.h"
#include "ofUtils.h"
#include <cmath>

//--------------------------------------------------------------
void ofApp::setup() {
	// OpenFrameworks Global Settings // ------------------------------------------
	//
	ofEnableDepthTest();
	ofDisableAlphaBlending();
	ofEnableArbTex();

	// Images //
	//
	ofLoadImage(woodTex, "textures/wood.jpg");
	ofLoadImage(orangeTex, "textures/orange.png");
	ofLoadImage(blankTex, "textures/blank.png");
	noiseTex.load("textures/noise3.jpg"); // noiseTex is different b/c we need to directly access the pixels and im lazy.

	// Scene setup // ------------------------------------------
	//
	camera.setPosition(0, 10, 20);
	sphere.setPosition(glm::vec3(-4, 3, 0));
	sphere.setResolution(20);
	sphere.setRadius(3);
	sphere.mapTexCoordsFromTexture(orangeTex);
	sphere.getMesh().setColorForIndices(0, sphere.getMesh().getNumIndices(), ofColor(0, 0, 122, 122));

	controlSphere.setPosition(4, 3, 0);
	controlSphere.setResolution(20);
	controlSphere.setRadius(3);
	ofMesh& sphereMesh = controlSphere.getMesh();
	controlSphere.mapTexCoordsFromTexture(noiseTex.getTexture());

	for(int i = 0; i < sphereMesh.getNumVertices(); i++) { // Set vertex colors for render parameters
		glm::vec2 texcoord = sphereMesh.getTexCoord(i);
		ofColor noise = noiseTex.getColor(texcoord.x, texcoord.y);
		float magnitude = (noise.r + noise.g + noise.b + noise.a) / 4.0f;
		sphereMesh.addColor(ofColor(0, 0, 255, magnitude));
	}

	controlSphere.mapTexCoordsFromTexture(orangeTex);

	backPlane.setWidth(planeSize);
	backPlane.setHeight(planeSize);
	backPlane.mapTexCoordsFromTexture(woodTex);
	backPlane.getMesh().setColorForIndices(0, backPlane.getMesh().getNumIndices(), ofColor(100, 100, 100, 100));

	// FBO Setup //
	//
	MRTSettings.width = ofGetWidth();
	MRTSettings.height = ofGetHeight();
	MRTSettings.internalformat = GL_RGBA;
	MRTSettings.useDepth = true;
	MRTSettings.depthStencilAsTexture = true;

	sceneFBO.allocate(MRTSettings);
	sceneFBO.createAndAttachTexture(GL_RGBA, 1);
	sceneFBO.createAndAttachTexture(GL_RGBA, 2);

	// Intermediate Image FBO's are allocated with half resolution
	intermediateSettings.width = ofGetWidth();
	intermediateSettings.height = ofGetHeight();
	intermediateSettings.useDepth = true;
	intermediateSettings.internalformat = GL_RGBA;
	gaussBlurFBO.allocate(intermediateSettings);
	bleedFBO.allocate(intermediateSettings);

	// Shader Setup //
	//
	firstPass = shared_ptr<ofShader>(new ofShader());
	firstPass->load("shaders/objSpace");

	GLint err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Load Shader came back with GL error:	" << err;
	}

	gaussBlurPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	gaussBlurPass->load("shaders/gaussianBlur");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Load Shader came back with GL error:	" << err;
	}

	hBleedPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	hBleedPass->load("shaders/bleed.vert", "shaders/horizontalBleed.frag");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Load Shader came back with GL error:	" << err;
	}

	isShaderDirty = false;
	drawToScreen = 1;

}

//--------------------------------------------------------------
void ofApp::update() {
	// Support reloading for any shader I'm currently working on.
	if(isShaderDirty) {
		ofLogNotice("Reloading Shader");
		gaussBlurPass = shared_ptr<ofShader>(new ofShader());
		gaussBlurPass->load("shaders/gaussianBlur");

		GLint err = glGetError();
		if (err != GL_NO_ERROR){
			ofLogNotice() << "Load Shader came back with GL error:	" << err;
		}

		isShaderDirty = false;
	}
}

void ofApp::drawScene() { // TODO: Possibly just pass in the shader to set the uniform texture. Doesn't break right now, just bad coding practice.
	camera.begin();

	firstPass->setUniformTexture("materialTex", orangeTex, 0);
	sphere.draw();
	controlSphere.draw();

	ofPushMatrix();
	ofRotateXDeg(90);
	firstPass->setUniformTexture("materialTex", woodTex, 0);
	backPlane.draw();
	ofPopMatrix();

	camera.end();
}

//--------------------------------------------------------------
void ofApp::draw(){
	// Draw the scene into both fbos with their respective shaders.

	// Render scene normally //
	sceneFBO.begin();
	sceneFBO.activateAllDrawBuffers();
	ofClear(0, 0, 0, 0);

	firstPass->begin();
	firstPass->setUniform1f("time", ofGetElapsedTimef());
	firstPass->setUniform1f("speed", 1.0f);
	firstPass->setUniform1f("freq", 1.0f);
	firstPass->setUniform1f("tremor", 50.0f);
	firstPass->setUniform1f("pixelSize", 1.0f / ofGetWidth());
	firstPass->setUniform3f("cameraPos", camera.getGlobalPosition());
	firstPass->setUniform3f("lightPosition", glm::vec3(5.0f, 5.0f ,5.0f));

	drawScene();

	firstPass->end();
	sceneFBO.end();

	// Render gaussian blur //
	// TODO: Fix output of gaussian blur. It got messed up with the MRT.
	gaussBlurFBO.begin();
	ofClear(0, 0, 0, 0);
	gaussBlurPass->begin();
	gaussBlurPass->setUniformTexture("image", sceneFBO.getTexture(0), 0);
	ofTexture& result = sceneFBO.getTexture(0);
	result.draw(0, 0);
	gaussBlurPass->end();
	gaussBlurFBO.end();

	switch(drawToScreen) {
		case 1:
			sceneFBO.getTexture(0).draw(0, 0);
			break;
		case 2:
			sceneFBO.getTexture(1).draw(0, 0);
			break;
		case 3:
			sceneFBO.getTexture(2).draw(0, 0);
			break;
		case 4:
			gaussBlurFBO.draw(0, 0);
			break;
	}


	string str;
	str += "Frame Rate: " + std::to_string(ofGetFrameRate());
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str, ofGetWindowWidth() -170, 15);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch (key) {
	case 'C':
	case 'c':
		if (camera.getMouseInputEnabled())
			camera.disableMouseInput();
		else
			camera.enableMouseInput();
		break;
    case 's':
		sceneFBO.getTexture(0).readToPixels(pixels);
		ofSaveImage(pixels, "fbo.png", OF_IMAGE_QUALITY_BEST);
		break;
	case 'R':
	case 'r':
		isShaderDirty = true;
		break;
	case '1':
		drawToScreen = 1;
		break;
	case '2':
		drawToScreen = 2;
		break;
	case '3':
		drawToScreen = 3;
		break;
	case '4':
		drawToScreen = 4;
		break;
	}
}

void ofApp::exit() {}

void ofApp::keyReleased(int key) {}

void ofApp::mouseMoved(int x, int y ) {}

void ofApp::mouseDragged(int x, int y, int button) {}

void ofApp::mousePressed(int x, int y, int button) {}

void ofApp::mouseReleased(int x, int y, int button) {}

void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {}

void ofApp::mouseEntered(int x, int y) {}

void ofApp::mouseExited(int x, int y) {}

void ofApp::windowResized(int w, int h) {}

void ofApp::gotMessage(ofMessage msg) {}

void ofApp::dragEvent(ofDragInfo dragInfo) {}
