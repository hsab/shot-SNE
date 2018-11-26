#ifndef _VID
#define _VID
#include "hecateEvent.h"
#include "hecateThread.h"
#include "ofMain.h"

class Vid {
 public:
  void setup(string file, string *hecate);
  //   void setup(string file);

  void update();
  void draw();
  void hecate(string);
  void hecateClose();
  void closeVideo();
  void openVideo();
  int vidStat();
  //   bool ready();
  void play();
  void stop();
  void setViewed(bool flag);
  bool isViewed();

  float x;
  float y;
  float speedY;
  float speedX;
  int dim;
  ofColor color;

  HecateThread *hecateThread = nullptr;

  string *hecatePath;
  ofParameter<string> filePath;
  ofParameter<bool> inView;
  ofParameter<int> currentFrame;
  ofVideoPlayer video;

  vector<tuple<int, int>> shots;
  vector<int> keyframes;

  void hecateEvent(HecateEvent &e);
  void keyPressed(ofKeyEventArgs &e);
  void keyPressed(int key);

  Vid();
  ~Vid();

 private:
};
#endif