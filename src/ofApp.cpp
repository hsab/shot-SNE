#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
  dataPath = ofToDataPath("", true);
  hecatePath = ofToDataPath("", true) + "/bin/hecate";

  myvid.setup(dataPath + "/foo.mp4");
  doLock = false;
}

void ofApp::exit() {}

// string test = ofSystem(cmd.c_str());

//--------------------------------------------------------------
void ofApp::update() { myvid.update(); }

//--------------------------------------------------------------
void ofApp::draw() { myvid.draw(); }

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  if (key == ' ') {
    myvid.color.set(0);
    myvid.hecate(hecatePath);
  }
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
