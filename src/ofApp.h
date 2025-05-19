#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"

class ofApp : public ofBaseApp{

	public:
		// OpenFrameworks Functions
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;

		// My Functions //
		void drawScene();

		// Scene Objects
		ofEasyCam camera;
		ofSpherePrimitive sphere;
		ofSpherePrimitive controlSphere;
		ofPlanePrimitive groundPlane;
		ofPlanePrimitive backPlane;
		ofPlanePrimitive drawPlane;
		ofMesh fullscreenQuad;
		float planeSize = 150;
		ofxAssimpModelLoader dogModel;


		int drawToScreen; // Preview output switch var

		// Render Targets
		ofFbo sceneFBO;
		ofFbo gaussBlurFBO;
		ofFbo bleedFBO;
		ofFbo finalBleedFBO;
		ofFbo stylizeFBO;
		ofFbo paperFBO;
		ofFbo::Settings intermediateSettings;
		ofFbo::Settings MRTSettings;
		ofPixels pixels;
		bool drawDog;

		// Textures
		ofImage orangeTex;
		ofImage noiseTex;
		ofImage planeColorImg;
		ofImage fabricTex;
		ofImage paperTex;
		ofImage paperNormalTex;
		ofImage dogTex;

		// Shaders
		shared_ptr<ofShader> firstPass;
		shared_ptr<ofShader> paperPass;
		shared_ptr<ofShader> gaussBlurPass;
		shared_ptr<ofShader> hBleedPass;
		shared_ptr<ofShader> vBleedPass;
		shared_ptr<ofShader> stylizePass;
		bool isShaderDirty;


};
