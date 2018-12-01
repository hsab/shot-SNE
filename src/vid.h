#ifndef _VID
#define _VID
#include "hecateEvent.h"
#include "hecateThread.h"
#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"

struct VidStat {
  bool closed       = true;
  bool stopped      = true;
  int width         = -1;
  int height        = -1;
  int frames        = -1;
  float duration    = -1.0;
  int currentFrame  = -1;
  float currentTime = -1;
  float position    = -1.0;
  float fps         = -1.0;
  int keyframeIndex = -1.0;

  string toString() {
    std::ostringstream stream_out;
    stream_out << "Closed: " << closed << "\nStopped: " << stopped
               << "\nWidth: " << width << "\nHeight: " << height
               << "\nFrames: " << frames << "\nDuration: " << duration
               << "\nCurrent Frame: " << currentFrame
               << "\nCurrent Time: " << currentTime
               << "\nPosition: " << position << "\nFPS: " << fps
               << "\nKeyframe Index: " << keyframeIndex << endl;
    return stream_out.str();
  };

  void resetCurrentTime() {
    position     = -1.0;
    currentTime  = -1.0;
    currentFrame = -1.0;
  };

  void updateDimension(float w, float h) {
    width  = (w > 0) ? w : width;
    height = (h > 0) ? h : height;
  };

  void updateTotalFrames(float f) { frames = (f > 0) ? f : frames; };

  void updateDuration(float d) {
    duration = (d > 0) ? d : duration;
    fps      = ((frames / d) > 0) ? (frames / d) : fps;
  };

  void updateCurrentTimeInfo(float p) {
    position     = p;
    currentTime  = position * duration;
    currentFrame = currentTime * fps;
  }
};

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
  void keyPressed(ofKeyEventArgs& e);
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

  ofFbo frameBuffer;
  ofImage saveImage;
  ofxCv::ContourFinder contourFinder;
  ofxPanel gui;
  ofParameter<float> minArea, maxArea, threshold;
  ofParameter<bool> holes;
  string renderFolder;

  ofTrueTypeFont verdana14;

 private:
};
#endif