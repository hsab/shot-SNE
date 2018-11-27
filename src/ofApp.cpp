#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
  // ofSetLogLevel(OF_LOG_VERBOSE);

  dataPath   = ofToDataPath("", true);
  hecatePath = ofToDataPath("", true) + "/bin/hecate";

  myvid.setup(dataPath + "/foo.mp4", &hecatePath);
  myvid.setupCoordinates(600, 300);
  myvid.openVideo();
  doLock = false;
}

void ofApp::exit() {}

// string test = ofSystem(cmd.c_str());

//--------------------------------------------------------------
void ofApp::update() {
  myvid.update();
}

//--------------------------------------------------------------
void ofApp::draw() {
  myvid.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  myvid.keyPressed(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {}
