#pragma once
#include "ofMain.h"

class HecateEvent : public ofEventArgs {
 public:
  bool terminated = false;
  string path;
  string raw;
  string cmd;
  string clip;
  vector<tuple<int, int>> shots;
  vector<int> keyframes;

  HecateEvent() {}

  static ofEvent<HecateEvent> events;
};
