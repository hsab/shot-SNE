#include "ofApp.h"

int ofApp::selected = -1;

//--------------------------------------------------------------
void ofApp::setup() {
   // ofSetLogLevel(OF_LOG_VERBOSE);

   // gui.setup();
   //  backgroundColor = ofColor(114, 144, 154);

   dataPath = ofToDataPath("", true);
   hecatePath = "";
   // hecatePath = ofToDataPath("", true) + "/bin/hecate";
   // vector<string> temp = ofSplitString(hecatePath, "vidman");
   // hecatePath = temp[0];
   // hecatePath = ofFilePath::addTrailingSlash(hecatePath);
   // hecatePath = ofFilePath::join(hecatePath, "vidman");
   // hecatePath = ofFilePath::join(hecatePath, "hecate");
   // hecatePath = ofFilePath::join(hecatePath, "bin");
   // hecatePath = ofFilePath::join(hecatePath, "hecate");

   gui.setup(&videos, &fileNames, &activeVid, &hecatePath, &selected, &mouseOverGui);
}

void ofApp::exit() {}

// string test = ofSystem(cmd.c_str());

//--------------------------------------------------------------
void ofApp::update() {
   std::stringstream strm;
   strm << "fps: " << ofGetFrameRate();
   ofSetWindowTitle(strm.str());

   if (videoAvailable()) {
      videos[selected]->update();
   }
}

//--------------------------------------------------------------
void ofApp::draw() {
   if (videoAvailable()) {
      videos[selected]->draw();
   }

   gui.begin();

   gui.draw();

   if (videoAvailable()) {
      videos[selected]->drawGui();
   }

   gui.end();
   cout << mouseOverGui << endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
   int val = key - '0';
   if (val >= 0 && val < videos.size()) {
      videos[selected]->closeVideo();
      selected = val;
      videos[selected]->openVideo();
   }

   if (videoAvailable() && !mouseOverGui) {
      videos[selected]->keyPressed(key);
   }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
   if (videoAvailable() && !mouseOverGui) {
      videos[selected]->mouseDragged(x, y, button);
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
   if (videoAvailable()) {
      videos[selected]->windowResized();
   }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo info) {
   cout << info.files.size() << endl;
   for (int k = 0; k < info.files.size(); k++) {
      cout << info.files[k] << endl;
   }

   if (info.files.size() > 0) {
      for (unsigned int k = 0; k < info.files.size(); k++) {
         std::shared_ptr<Vid> myvid(new Vid);
         myvid->setup(info.files[k], &hecatePath, 900, 900);
         videos.push_back(myvid);
         string filename = myvid->json["info"]["baseName"].asString();
         string extention = myvid->json["info"]["extension"].asString();
         fileNames.push_back(filename + "." + extention);
      }
   }
}

bool ofApp::videoAvailable() {
   return (videos.size() > 0 && selected != -1);
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
void ofApp::ShowHelpMarker(const char* desc) {
   ImGui::TextDisabled("(?)");
   if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::TextUnformatted(desc);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
   }
}