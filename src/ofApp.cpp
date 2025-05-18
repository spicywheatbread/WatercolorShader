#include "ofApp.h"
#include "ofGraphics.h"
#include "ofImage.h"
#include "ofTexture.h"
#include "ofUtils.h"
#include <cmath>

//--------------------------------------------------------------
void ofApp::setup() {
	// OpenFrameworks Global Settings //
	//
	ofEnableDepthTest();
	ofDisableAlphaBlending();
	ofEnableArbTex(); // Currently using non power of two textures;

	// Images //
	//
	ofLoadImage(woodTex, "textures/wood.jpg");
	ofLoadImage(orangeTex, "textures/orange.png");
	ofLoadImage(blankTex, "textures/blank.png");
	noiseTex.load("textures/noise3.jpg"); // noiseTex is different b/c we need to directly access the pixels and im lazy.

	// Scene setup //
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
	controlSphere.mapTexCoordsFromTexture(noiseTex.getTexture());

	ofMesh& sphereMesh = controlSphere.getMesh();
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
	backPlane.getMesh().setColorForIndices(0, backPlane.getMesh().getNumIndices(), ofColor(100, 100, 0, 100));

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

	// Intermediate Image FBO's are allocated with half resolution (NOT REALLY; they CAN be but lowkey for ease of use they're the same size.)
	intermediateSettings.width = ofGetWidth();
	intermediateSettings.height = ofGetHeight();
	intermediateSettings.internalformat = GL_RGBA;
	intermediateSettings.useDepth = true;
	intermediateSettings.depthStencilAsTexture = true;

	gaussBlurFBO.allocate(intermediateSettings);
	bleedFBO.allocate(intermediateSettings);
	bleedFBO.createAndAttachTexture(GL_RGBA, 1);
	finalBleedFBO.allocate(intermediateSettings);
	finalBleedFBO.createAndAttachTexture(GL_RGBA, 1);
	stylizeFBO.allocate(intermediateSettings);

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

	vBleedPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	vBleedPass->load("shaders/bleed.vert", "shaders/verticalBleed.frag");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Load Shader came back with GL error:	" << err;
	}

	stylizePass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	stylizePass->load("shaders/stylize");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Load Shader came back with GL error:	" << err;
	}

	isShaderDirty = false;
	drawToScreen = 7;
}

//--------------------------------------------------------------
void ofApp::update() {
	// Support reloading for the shader I'm currently working on.
	if(isShaderDirty) {
		ofLogNotice() << "Reloading Shader" << "\n";
		stylizePass = shared_ptr<ofShader>(new ofShader());
		stylizePass->load("shaders/stylize");

		GLint err = glGetError();
		if (err != GL_NO_ERROR){
			ofLogNotice() << "Load Shader came back with GL error:	" << err;
		}

		isShaderDirty = false;
	}

	// Blank full screen quad to make the shaders render.
    fullscreenQuad.clear();
    fullscreenQuad.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

    fullscreenQuad.addVertex(glm::vec3(0, 0, 0));
    fullscreenQuad.addTexCoord(glm::vec2(0, 0));

    fullscreenQuad.addVertex(glm::vec3(ofGetWidth(), 0, 0));
    fullscreenQuad.addTexCoord(glm::vec2(ofGetWidth(), 0));

    fullscreenQuad.addVertex(glm::vec3(0, ofGetHeight(), 0));
    fullscreenQuad.addTexCoord(glm::vec2(0, ofGetHeight()));

    fullscreenQuad.addVertex(glm::vec3(ofGetWidth(), ofGetHeight(), 0));
    fullscreenQuad.addTexCoord(glm::vec2(ofGetWidth(), ofGetHeight()));
}

void ofApp::drawScene() {
	camera.begin();

	sphere.draw();
	controlSphere.draw();

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
	firstPass->setUniform1f("pixelSize", 1.0f / ofGetWidth());
	firstPass->setUniform3f("cameraPos", camera.getGlobalPosition());
	firstPass->setUniform3f("lightPosition", glm::vec3(5.0f, 5.0f ,5.0f));
	firstPass->setUniformTexture("materialTex", orangeTex, 0);

	drawScene();

	firstPass->end();
	sceneFBO.end();

	// Render gaussian blur //
	gaussBlurFBO.begin();
	ofClear(0, 0, 0, 0);

	gaussBlurPass->begin();
	gaussBlurPass->setUniformTexture("image", sceneFBO.getTexture(0), 0);

	sceneFBO.getTexture(0).draw(0, 0);

	gaussBlurPass->end();
	gaussBlurFBO.end();

	// Render horizontal bleed //
	bleedFBO.begin();
	bleedFBO.activateAllDrawBuffers();
	ofClear(0, 0, 0, 0);

	hBleedPass->begin();
	hBleedPass->setUniformTexture("colorImage", sceneFBO.getTexture(0), 1);
	hBleedPass->setUniformTexture("depthImage", sceneFBO.getDepthTexture(), 2);
	hBleedPass->setUniformTexture("controlImage", sceneFBO.getTexture(1), 3);

	fullscreenQuad.draw();

	hBleedPass->end();
	bleedFBO.end();

	// Render vertical bleed
	finalBleedFBO.begin();
	finalBleedFBO.activateAllDrawBuffers();
	ofClear(0, 0, 0, 0);

	vBleedPass->begin();
	vBleedPass->setUniformTexture("colorImage", bleedFBO.getTexture(0), 1);
	vBleedPass->setUniformTexture("depthImage", sceneFBO.getDepthTexture(), 2);
	vBleedPass->setUniformTexture("controlImage", bleedFBO.getTexture(1), 3);

	fullscreenQuad.draw();

	vBleedPass->end();
	finalBleedFBO.end();

	// Final stylization pass: color bleed, edge darkening, paper tex
	stylizeFBO.begin();
	ofClear(0, 0, 0, 0);

	stylizePass->begin();
	stylizePass->setUniformTexture("colorImage", sceneFBO.getTexture(0), 1);
	stylizePass->setUniformTexture("blurImage", gaussBlurFBO.getTexture(), 2);
	stylizePass->setUniformTexture("bleedImage", finalBleedFBO.getTexture(0), 3);
	stylizePass->setUniformTexture("controlImage", finalBleedFBO.getTexture(1), 4);

	fullscreenQuad.draw();

	stylizePass->end();
	stylizeFBO.end();

	switch(drawToScreen) { // Switch various stage outputs
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
		case 5:
			finalBleedFBO.getTexture(0).draw(0, 0);
			break;
		case 6:
			finalBleedFBO.getTexture(1).draw(0, 0);
			break;
		case 7:
			stylizeFBO.draw(0, 0);
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
		bleedFBO.getTexture(0).readToPixels(pixels);
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
	case '5':
		drawToScreen =  5;
		break;
	case '6':
		drawToScreen = 6;
		break;
	case '7':
		drawToScreen = 7;
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
