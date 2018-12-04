#pragma once
#include "ofMain.h"

class HecateEvent : public ofEventArgs {
 public:
  bool terminated = false;
  string path = "";
  string hecraw = "";
  string ffraw = "";
  string heccmd;
  string ffcmd;

  HecateEvent() {}

  static ofEvent<HecateEvent> events;
};
