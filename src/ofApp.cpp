#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
   // ofSetLogLevel(OF_LOG_VERBOSE);

   dataPath = ofToDataPath("", true);
   hecatePath = ofToDataPath("", true) + "/bin/hecate";
}

void ofApp::exit() {}

// string test = ofSystem(cmd.c_str());

//--------------------------------------------------------------
void ofApp::update() {
   std::stringstream strm;
   strm << "fps: " << ofGetFrameRate();
   ofSetWindowTitle(strm.str());

   if (videos.size() > 0) {
      videos[videoIndex]->update();
   }
}

//--------------------------------------------------------------
void ofApp::draw() {
   // videos[videoIndex]video.draw(0, 0);
   if (videos.size() > 0) {
      videos[videoIndex]->draw();
   }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
   int val = key - '0';
   if (val >= 0 && val < videos.size()) {
      videos[videoIndex]->closeVideo();
      videoIndex = val;
      videos[videoIndex]->openVideo();
   }

   if (videos.size() > 0) {
      videos[videoIndex]->keyPressed(key);
   }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
   if (videos.size() > 0) {
      videos[videoIndex]->mouseDragged(x, y, button);
   }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
   if (videos.size() > 0) {
      videos[videoIndex]->windowResized();
   }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo info) {
   if (info.files.size() > 0) {
      for (unsigned int k = 0; k < info.files.size(); k++) {
         std::shared_ptr<Vid> myvid(new Vid);
         myvid->setup(info.files[k], &hecatePath, 900, 900);
         videos.push_back(myvid);
      }
   }
}
