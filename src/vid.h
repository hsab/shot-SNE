#ifndef _VID
#define _VID
// #include "hecateEvent.h"
#include "hecateThread.h"
#include "ofMain.h"
#include "ofxCv.h"
// #include "ofxGui.h"
#include "ofxImGui.h"
#include "ofxPreset.h"
#include "vidJson.h"
#include "vidStat.h"

class Vid {
  public:
   Vid();
   ~Vid();

   void setup(string file, string* hecate, int _w, int _h);

   void update();
   void draw();
   void drawGui();

   void hecate(string);
   void hecateClose();

   void closeVideo();
   void openVideo();

   ofVideoPlayer video;
   ofGstVideoPlayer* player;
   VidStat stats;

   void play();
   void stop();
   void pause();
   bool isViewed();
   void setViewed(bool flag);
   void updateVidStats();
   void initVidStats();

   ofParameter<string> filePath;
   ofParameter<bool> inView;
   ofParameter<bool> frameByframe;
   bool hecateDone = false;
   //  ofParameterGroup parameters;

   int keyframeIndex = -1;

   bool isReadyForKeyframeNavigation();
   void prepareFrameByFrame();
   void alignIndexToFrame(char flag);
   void nextKeyframe();
   void previousKeyframe();

   ofFbo frameBuffer;
   ofImage saveImage;
   ofxCv::ContourFinder contourFinder;

   void prepareBuffer();
   void updateBuffer();
   void renderCurrentFrame();

   string ffprobe(string filePath);

   void keyPressed(int key);
   void mouseDragged(int x, int y, int button);
   void windowResized();
   void hecateEvent(HecateEvent& e);

   HecateThread* hecateThread = nullptr;
   string* hecatePath;

   int width;
   int height;
   int wn, hn, top, left;
   int fboWidth, fboHeight, fboLeft, fboTop;
   int tlh = 50;
   int tlo = 20;

   void setupCoordinates(int w, int h);
   void refreshCoordinates(int w, int h);
   void calculateCoordinates(int w, int h, int& wn, int& hn, int& left, int& top);

   //  ofxPanel gui;
   ofParameter<float> minArea, maxArea, threshold;
   ofParameter<bool> showContour;
   ofParameterGroup parameters;

   ofTrueTypeFont infoFont;

   VidJson vidData;
   ofxJSON json;
   ofxJSON data();

  private:
};
#endif