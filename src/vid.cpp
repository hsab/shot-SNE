#include "vid.h"

Vid::Vid() {}

Vid::~Vid() {
  closeVideo();
  hecateClose();
}

void Vid::openVideo() {
  video.loadAsync(filePath.get());
  video.setLoopState(OF_LOOP_NONE);
  video.stop();
  inView = true;
}

void Vid::closeVideo() {
  inView = false;
  video.closeMovie();
}

void Vid::play() { video.play(); }
void Vid::stop() { video.stop(); }

void Vid::setup(string file, string *hecate) {
  // void Vid::setup(string file) {
  filePath = file;
  hecatePath = hecate;

  ofAddListener(HecateEvent::events, this, &Vid::hecateEvent);
  // ofAddListener(ofEvents().keyPressed, this, &Vid::keyPressed);
}

void Vid::setViewed(bool flag) { inView = flag; }
bool Vid::isViewed() { return inView; }

void Vid::update() {
  if (inView) video.update();
  if (hecateThread != nullptr && hecateThread->isProcessed() &&
      hecateThread->isThreadRunning()) {
    hecateClose();
  }
}

void Vid::draw() {
  if (inView) video.draw(20, 20, 500, 300);
}

void Vid::keyPressed(ofKeyEventArgs &e) {}

void Vid::keyPressed(int key) {
  if (key == 'h') {
    hecate(*hecatePath);
  }

  if (key == 'H') {
    hecateClose();
  }

  if (key == 'i') {
    cout << video.isPlaying() << endl;
  }

  if (key == 'v') {
    setViewed(!isViewed());
  }

  if (key == 'p') {
    cout << video.isPlaying() << endl;
    play();
  }

  if (key == 's') {
    cout << video.isPlaying() << endl;
    stop();
  }

  if (key == ' ') {
    cout << video.isPlaying() << endl;
    video.setPaused(!video.isPaused());
  }

  if (key == OF_KEY_LEFT) {
  }

  if (key == OF_KEY_RIGHT) {
  }

  if (key == 'c') {
    closeVideo();
  }
}

int Vid::vidStat() { return video.isPlaying(); }

void Vid::hecate(string hecatePath) {
  hecateThread = new HecateThread();
  hecateThread->setup(hecatePath, filePath.get());
}

void Vid::hecateClose() {
  delete hecateThread;
  hecateThread = nullptr;
  // if (hecateThread != nullptr) {
  //   if (safeClose) {
  //     if (hecateThread->isProcessed() & hecateThread->isThreadRunning())
  //       delete hecateThread;
  //   } else {
  //     delete hecateThread;
  //   }
  // }
}

void Vid::hecateEvent(HecateEvent &e) {
  cout << "Shots: {";
  for (auto s : e.shots) {
    cout << "[" << get<0>(s) << ":" << get<1>(s) << "], ";
  }
  cout << "}" << endl << endl;

  cout << "Keyframes: " + ofToString(e.keyframes) << endl << endl;
  cout << "Command: " + ofToString(e.cmd) << endl << endl;
  // cout << "RAW: " + ofToString(e.raw) << endl << endl;
}
