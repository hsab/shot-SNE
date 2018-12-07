#pragma once

#include "ofMain.h"
// #include "ofxImGui.h"
#include "guiMain.h"
#include "vid.h"

class ofApp : public ofBaseApp {
  public:
   void setup();
   void update();
   void draw();
   void exit();

   void keyPressed(int key);
   void keyReleased(int key);
   void mouseMoved(int x, int y);
   void mouseDragged(int x, int y, int button);
   void mousePressed(int x, int y, int button);
   void mouseReleased(int x, int y, int button);
   void mouseEntered(int x, int y);
   void mouseExited(int x, int y);
   void windowResized(int w, int h);
   void dragEvent(ofDragInfo dragInfo);
   void gotMessage(ofMessage msg);

   // Vid myvid;
   //  ofxImGui::Gui gui;
   //  ImVec4 backgroundColor;
   VidmanGui gui;
   bool mouseOverGui = false;

   std::vector<std::string> fileNames;
   vector<std::shared_ptr<Vid>> videos;
   std::shared_ptr<Vid> activeVid = nullptr;
   static int selected;

   bool videoAvailable();
   void ShowHelpMarker(const char* desc);

   bool doLock;
   string hecatePath;
   string dataPath;
};
