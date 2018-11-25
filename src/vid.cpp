#include "vid.h"

Vid::Vid() {}

Vid::~Vid() { hecateThread.stop(); }

void Vid::setup(string path) {
  filePath = path;
  x = ofRandom(0, ofGetWidth());  // give some random positioning
  y = ofRandom(0, ofGetHeight());

  speedX = ofRandom(-1, 1);  // and random speed and direction
  speedY = ofRandom(-1, 1);

  dim = 20;

  color.set(0);
  ofAddListener(HecateEvent::events, this, &Vid::hecateEvent);
}

void Vid::update() {
  if (x < 0) {
    x = 0;
    speedX *= -1;
  } else if (x > ofGetWidth()) {
    x = ofGetWidth();
    speedX *= -1;
  }

  if (y < 0) {
    y = 0;
    speedY *= -1;
  } else if (y > ofGetHeight()) {
    y = ofGetHeight();
    speedY *= -1;
  }

  x += speedX;
  y += speedY;

  if (hecateThread.isProcessed() & hecateThread.isThreadRunning())
    hecateThread.stop();
}

void Vid::draw() {
  ofSetColor(color);
  ofDrawCircle(x, y, dim);
}

void Vid::hecate(string hecatePath) {
  string cmd = hecatePath + " -i " + filePath +
               " --print_shot_info  --print_keyfrm_info";
  hecateThread.setup(cmd);
}

void Vid::hecateEvent(HecateEvent &e) {
  color.set(255);

  cout << "Shots: {";
  for (auto s : e.shots) {
    cout << "[" << get<0>(s) << ":" << get<1>(s) << "], ";
  }
  cout << "}" << endl << endl;

  cout << "Keyframes: " + ofToString(e.keyframes) << endl << endl;
  cout << "Command: " + ofToString(e.cmd) << endl << endl;
  cout << "RAW: " + ofToString(e.raw) << endl << endl;
}
