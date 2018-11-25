#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
  myvid.setup();
  doLock = false;
}

void ofApp::exit() {
  // threadedObject.stop();
  // threadedObject2.stop();
}

// string test = ofSystem(cmd.c_str());

//--------------------------------------------------------------
void ofApp::update() {
  myvid.update();
  // threadedObject.updateNoLock();
  // threadedObject2.updateNoLock();
}

//--------------------------------------------------------------
void ofApp::draw() {
  myvid.draw();
  // threadedObject.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  if (key == ' ') {
    myvid.color.set(0);
    string dataPath = ofFilePath::getAbsolutePath(ofToDataPath(""));
    string cmd = dataPath + "/bin/hecate -i " + dataPath + "foo.mp4" +
                 " --print_shot_info  --print_keyfrm_info";
    string cmd2 = dataPath + "/bin/hecate -i " + dataPath + "foo2.mp4" +
                  " --print_shot_info  --print_keyfrm_info";
    myvid.hecate(cmd);
    // threadedObject2.setup(cmd2);
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
