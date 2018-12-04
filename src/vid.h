#ifndef _VID
#define _VID
#include "hecateEvent.h"
#include "hecateThread.h"
#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxJSON.h"
#include "vidStat.h"

class Vid {
 public:
  void setup(string file, string* hecate);
  //   void setup(string file);

  void update();
  void draw();
  void hecate(string);
  void hecateClose();
  void closeVideo();
  void openVideo();
  void vidStatVerbose();
  void play();
  void stop();
  void pause();
  void updateStats();
  void initStats();
  void setViewed(bool flag);
  bool isViewed();
  void alignIndexToFrame(char flag);
  void nextKeyframe();
  void previousKeyframe();
  void prepareFrameByFrame();
  void prepareBuffer();
  void updateBuffer();
  void renderCurrentFrame();
  void renderKeyframes();
  bool isReadyForKeyframeNavigation();
  void processHecateResults(string result);
  bool prepareDataFolder(bool readIfExists);
  void processRendersFolder(bool hecateOutputExists);
  void prepareHecateOutput();

  HecateThread* hecateThread = nullptr;

  string* hecatePath;
  ofParameter<string> filePath;
  ofParameter<bool> inView;
  ofParameter<bool> frameByframe;
  ofParameter<bool> hecateDone;
  ofParameter<int> currentFrame;
  ofParameterGroup parameters;

  ofVideoPlayer video;
  ofGstVideoPlayer* player;
  VidStat stats;

  string baseName;
  string extension;
  string enclosingDirectoryPath;
  string vidmanDirectoryPath;
  string rendersDirectoryPath;
  ofFile hecateSavedOutput;

  ofDirectory vidmanDirectory;
  ofDirectory rendersDirectory;

  vector<tuple<int, int>> shots;
  vector<int> keyframes;
  map<int, bool> keyframesMap;
  map<int, int> keyframesToShotsMap;

  int keyframeIndex = -1;

  void hecateEvent(HecateEvent& e);
  void processEvent(HecateEvent& e);
  void keyPressed(int key);
  void mouseDragged(int x, int y, int button);

  Vid();
  ~Vid();

  int width;
  int height;
  int wn, hn, top, left;
  int fboWidth, fboHeight, fboLeft, fboTop;
  int tlh = 50;
  int tlo = 20;

  void setupCoordinates(int w, int h);
  void calculateCoordinates(int w, int h, int& wn, int& hn, int& left, int& top);

  void saveJpg(string fileName, int quality);

  ofFbo frameBuffer;
  ofImage saveImage;
  ofxCv::ContourFinder contourFinder;
  ofxPanel gui;
  ofParameter<float> minArea, maxArea, threshold;
  ofParameter<bool> holes;
  string renderFolder;

  ofTrueTypeFont verdana14;

  string hecateSavedOutputPath;

  ofxJSON json;
  string analysisSavedOutputPath;

 private:
};
#endif