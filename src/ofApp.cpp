#include "ofApp.h"
#include "ofGraphics.h"
#include "ofImage.h"
#include "ofTexture.h"
#include "ofUtils.h"

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
	settings.width = ofGetWidth(); // Some of the later passes are okay with lower resolutions...
	settings.height = ofGetHeight();
	settings.useDepth = true;
	settings.depthStencilAsTexture = true;
	settings.internalformat = GL_RGBA;

	sceneFBO.allocate(settings); // Using multiple FBOs because figuring out MRT seems annoying (DOCUMENTATION SUX)
	colorFBO.allocate(settings);
	alphaFBO.allocate(settings);

	// Intermediate Image FBO's are allocated with half resolution
	intermediateSettings.width = ofGetWidth() / 2;
	intermediateSettings.height = ofGetHeight() / 2;
	intermediateSettings.useDepth = true;
	intermediateSettings.internalformat = GL_RGBA;
	gaussBlurFBO.allocate(intermediateSettings);

	// Shader Setup //
	//
	firstPass = shared_ptr<ofShader>(new ofShader());
	firstPass->load("shaders/objSpace");

	GLint err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Load Shader came back with GL error:	" << err;
	}

	colorPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	colorPass->load("shaders/colorImage");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Load Shader came back with GL error:	" << err;
	}

	alphaPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	alphaPass->load("shaders/alphaImage");

	err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Load Shader came back with GL error:	" << err;
	}

	gaussBlurPass = shared_ptr<ofShader>(new ofShader()); // This is a really simple shader so I just initialize once.
	gaussBlurPass->load("shaders/gaussianBlur");

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
	ofClear(0, 0, 0, 0);
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

	// Render vertex color control image //
	colorFBO.begin();
	colorPass->begin();
	drawScene();
	colorPass->end();
	colorFBO.end();

	// Render Alpha Image b/c it looks cool //
	alphaFBO.begin();
	alphaPass->begin();
	drawScene();
	alphaPass->end();
	alphaFBO.end();

	// Render gaussian blur //
	gaussBlurFBO.begin();
	ofClear(0, 0, 0, 0);
	gaussBlurPass->begin();
	gaussBlurPass->setUniformTexture("image", sceneFBO.getTexture(), 0);
	sceneFBO.draw(0, 0);
	gaussBlurPass->end();
	gaussBlurFBO.end();

	switch(drawToScreen) {
		case 1:
			sceneFBO.draw(0, 0);
			break;
		case 2:
			colorFBO.draw(0, 0);
			break;
		case 3:
			alphaFBO.draw(0, 0);
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
		sceneFBO.readToPixels(pixels);
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
