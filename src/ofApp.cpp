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
	ofDisableArbTex();

	// Images //
	//
	orangeTex.load("textures/orange.png");
	planeColorImg.load("textures/planecolors.png");
	noiseTex.load("textures/noise.jpg");
	fabricTex.load("textures/fabric.jpg");
	paperTex.load("textures/paper.png");
	paperNormalTex.load("textures/paper-normal.png");
	dogTex.load("dog.png");
	dogModel.load("dog.fbx");

	camera.setPosition(0, 10, 30);

	sphere.setPosition(glm::vec3(-4, 3, 0));
	sphere.setResolution(20);
	sphere.setRadius(3);
	sphere.mapTexCoordsFromTexture(orangeTex.getTexture());
	sphere.getMesh().setColorForIndices(0, sphere.getMesh().getNumIndices(), ofColor(0, 0, 122, 122));

	controlSphere.setPosition(4, 3, 0);
	controlSphere.setResolution(20);
	controlSphere.setRadius(3);
	controlSphere.mapTexCoordsFromTexture(noiseTex.getTexture());


	ofMesh& sphereMesh = controlSphere.getMesh();
	for(int i = 0; i < sphereMesh.getNumVertices(); i++) { // Set vertex colors for render parameters
		glm::vec2 texcoord = sphereMesh.getTexCoord(i); // Using normalized texcoords means need to convert to pixels
		int u = texcoord.x * (noiseTex.getWidth() - 1);
		int v = texcoord.y * (noiseTex.getHeight() - 1);
		ofColor noise = noiseTex.getColor(u, v);
		float magnitude = (noise.r + noise.g + noise.b + noise.a) / 4.0f;
		sphereMesh.addColor(ofColor(0, 0, 255, magnitude));
	}
	controlSphere.mapTexCoordsFromTexture(orangeTex.getTexture());

	groundPlane.set(planeSize, planeSize, 50, 50);
	groundPlane.mapTexCoordsFromTexture(planeColorImg.getTexture());

	ofMesh& planeMesh = groundPlane.getMesh();
	for(int i = 0; i < planeMesh.getNumVertices(); i++) {
		glm::vec2 texcoord = planeMesh.getTexCoord(i);
		int u = texcoord.x * (planeColorImg.getWidth() - 1);
		int v = texcoord.y * (planeColorImg.getHeight() - 1);
		ofColor col = planeColorImg.getColor(u, v);
		float magnitude = glm::dot(glm::vec3(col.r, col.g, col.b), glm::vec3(0.33));
		planeMesh.addColor(ofColor(col.r, col.g, col.b, magnitude));
	}
	groundPlane.mapTexCoordsFromTexture(fabricTex.getTexture());

	backPlane.set(planeSize, planeSize, 50, 50);
	backPlane.getMesh().setColorForIndices(0, backPlane.getMesh().getNumIndices(), ofColor(255, 255, 255, 255));

	for(int i = 0; i < dogModel.getMesh(0).getNumVertices(); i++) {
		dogModel.getMesh(0).addColor(ofColor(0, 0, 255, 255));
	}

	// FBO Setup //
	//
	MRTSettings.width = 1024;
	MRTSettings.height = 1024;
	MRTSettings.internalformat = GL_RGBA;
	MRTSettings.useDepth = true;
	MRTSettings.depthStencilAsTexture = true;
	MRTSettings.textureTarget = GL_TEXTURE_2D; // OH MY GODDD THE OFFBO INITIALIZES WITH ARB TEXES HAHA!!!!!!!!! OF COURSE IT DOES!!! WHY NOT!!!


	sceneFBO.allocate(MRTSettings);
	paperFBO.allocate(MRTSettings);
	sceneFBO.createAndAttachTexture(GL_RGBA, 1);
	sceneFBO.createAndAttachTexture(GL_RGBA, 2);

	// Intermediate Image FBO's are allocated with half resolution (NOT REALLY; they CAN be but lowkey for ease of use they're the same size.)
	intermediateSettings.width = 1024;
	intermediateSettings.height = 1024;
	intermediateSettings.internalformat = GL_RGBA;
	intermediateSettings.useDepth = true;
	intermediateSettings.depthStencilAsTexture = true;
	intermediateSettings.textureTarget = GL_TEXTURE_2D;

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
		ofLogNotice() << "ObjSpace Shader came back with GL error:	" << err;
	}

	gaussBlurPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	gaussBlurPass->load("shaders/gaussianBlur");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Gaussian Blur Shader came back with GL error:	" << err;
	}

	hBleedPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	hBleedPass->load("shaders/bleed.vert", "shaders/horizontalBleed.frag");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Horionztal Bleed Shader came back with GL error:	" << err;
	}

	vBleedPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	vBleedPass->load("shaders/bleed.vert", "shaders/verticalBleed.frag");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Vertical Bleed Shader came back with GL error:	" << err;
	}

	stylizePass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	stylizePass->load("shaders/stylize");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Stylize Shader came back with GL error:	" << err;
	}

	paperPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	paperPass->load("shaders/paper");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Paper Shader came back with GL error:	" << err;
	}

	isShaderDirty = false;
	drawToScreen = 7;
}

//--------------------------------------------------------------
void ofApp::update() {
	// Support reloading for the shader I'm currently working on.
	if(isShaderDirty) {
		ofLogNotice() << "Reloading stylize Shader" << "\n";
		stylizePass = shared_ptr<ofShader>(new ofShader());
		stylizePass->load("shaders/stylize");

		GLint err = glGetError();
		if (err != GL_NO_ERROR){
			ofLogNotice() << "Stylize Shader came back with GL error:	" << err;
		}

		isShaderDirty = false;
	}

	// Blank full screen quad to make the shaders render.
    fullscreenQuad.clear();
    fullscreenQuad.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

    fullscreenQuad.addVertex(glm::vec3(0, 0, 0));
    fullscreenQuad.addTexCoord(glm::vec2(0, 0));

    fullscreenQuad.addVertex(glm::vec3(ofGetWidth(), 0, 0));
    fullscreenQuad.addTexCoord(glm::vec2(1, 0));

    fullscreenQuad.addVertex(glm::vec3(0, ofGetHeight(), 0));
    fullscreenQuad.addTexCoord(glm::vec2(0, 1));

    fullscreenQuad.addVertex(glm::vec3(ofGetWidth(), ofGetHeight(), 0));
    fullscreenQuad.addTexCoord(glm::vec2(1, 1));

}

void ofApp::drawScene() {
	camera.begin();

	sphere.draw();
	controlSphere.draw();

	ofPushMatrix();
	ofTranslate(glm::vec3(0, 0, 20));
	orangeTex.bind();
	sphere.draw();
	controlSphere.draw();
	orangeTex.unbind();
	ofPopMatrix();

	ofPushMatrix();
	ofTranslate(glm::vec3(0, 0, -planeSize / 2));
	ofScale(2);
	backPlane.draw();
	ofPopMatrix();


	ofPushMatrix();
	ofRotateXDeg(90);
	fabricTex.bind();
	groundPlane.draw();
	fabricTex.unbind();
	ofPopMatrix();

	if(drawDog) {
		ofPushMatrix();
		ofRotateXDeg(180);
		ofScale(0.05);
		dogTex.bind();
		dogModel.drawFaces();
		dogTex.unbind();
		ofPopMatrix();
	}


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

	drawScene();

	firstPass->end();
	sceneFBO.end();

	// Render gaussian blur //
	gaussBlurFBO.begin();
	ofClear(0, 0, 0, 0);

	gaussBlurPass->begin();
	gaussBlurPass->setUniformTexture("tex0", sceneFBO.getTexture(0), 0);
	gaussBlurPass->setUniform2f("pixelSize", 1.0f / ofGetWidth(), 1.0f / ofGetHeight());

	fullscreenQuad.draw();

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
	hBleedPass->setUniform1f("pixelSize", 1.0f / ofGetWidth());

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
	vBleedPass->setUniform1f("pixelSize", 1.0f / ofGetHeight());

	fullscreenQuad.draw();

	vBleedPass->end();
	finalBleedFBO.end();

	// Compute paper image for final processing
	paperFBO.begin();
	ofClear(0, 0, 0, 0);
	paperPass->begin();
	paperPass->setUniformTexture("paperDiff", paperTex, 0);
	paperPass->setUniformTexture("paperNorm", paperNormalTex, 1);

	fullscreenQuad.draw();

	paperPass->end();
	paperFBO.end();

	// Final stylization pass: color bleed, edge darkening, paper tex
	stylizeFBO.begin();
	ofClear(0, 0, 0, 0);

	stylizePass->begin();
	stylizePass->setUniformTexture("colorImage", sceneFBO.getTexture(0), 1);
	stylizePass->setUniformTexture("blurImage", gaussBlurFBO.getTexture(), 2);
	stylizePass->setUniformTexture("bleedImage", finalBleedFBO.getTexture(0), 3);
	stylizePass->setUniformTexture("controlImage", finalBleedFBO.getTexture(1), 4);
	stylizePass->setUniformTexture("paperImage", paperFBO.getTexture(), 5);
	stylizePass->setUniformTexture("paperTex", paperTex.getTexture(), 6);
	stylizePass->setUniform2f("pixelSize", 1.0f / ofGetWidth(), 1.0f / ofGetHeight());

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
	case 'd':
		drawDog = !drawDog;
		break;
    case 's':
		stylizeFBO.getTexture().readToPixels(pixels);
		ofSaveImage(pixels, "render_output.png", OF_IMAGE_QUALITY_BEST);
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
