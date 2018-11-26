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
  stats.closed = false;
  stats.stopped = true;
}

void Vid::closeVideo() {
  inView = false;
  stats.closed = true;
  stats.stopped = true;
  video.closeMovie();
}

void Vid::initStats() { updateStats(); }

void Vid::updateStats() {
  if (!stats.closed) {
    stats.updateDimension(video.getWidth(), video.getHeight());
    stats.updateTotalFrames(video.getTotalNumFrames());
    if (!stats.stopped) {
      stats.updateDuration(video.getDuration());
      // stats.fps = stats.frames / stats.duration;
      // stats.position = video.getPosition();
      // stats.currentTime = stats.position * stats.duration;
      // stats.currentFrame = stats.currentTime * stats.fps;
      stats.updateCurrentTimeInfo(video.getPosition());
    } else {
      stats.resetCurrentTime();
    }
  } else {
    stats.resetCurrentTime();
  }
}

void Vid::play() {
  stats.stopped = false;
  if (!stats.closed) {
    video.play();
    updateStats();
  }
}
void Vid::stop() {
  stats.stopped = true;
  if (!stats.closed) {
    video.stop();
    updateStats();
  }
}

void Vid::pause() {
  if (stats.stopped) {
    play();
    return;
  }
  video.setPaused(!video.isPaused());
  updateStats();
}

void Vid::setup(string file, string* hecate) {
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

void Vid::keyPressed(ofKeyEventArgs& e) {}

void Vid::keyPressed(int key) {
  if (key == 'h') {
    hecate(*hecatePath);
  }

  if (key == 'H') {
    hecateClose();
  }

  if (key == 'i') {
    vidStatVerbose();
    updateStats();
    cout << stats.toString();
  }

  if (key == 'v') {
    setViewed(!isViewed());
  }

  if (key == 'p') {
    play();
  }

  if (key == 's') {
    stop();
  }

  if (key == ' ') {
    pause();
  }

  if (key == OF_KEY_LEFT) {
  }

  if (key == OF_KEY_RIGHT) {
  }

  if (key == 'o') {
    openVideo();
  }

  if (key == 'c') {
    closeVideo();
  }
}

void Vid::vidStatVerbose() {
  cout << ((video.isPlaying()) ? "PLAYING" : "NOT PLAYING");
  cout << " // "
       << ((video.isPaused()) ? "PAUSED // FRAME: " : "NOT PAUSED // FRAME: ");
  cout << video.getCurrentFrame() << endl;
}

void Vid::hecate(string hecatePath) {
  hecateThread = new HecateThread();
  hecateThread->setup(hecatePath, filePath.get());
}

void Vid::hecateClose() {
  delete hecateThread;
  hecateThread = nullptr;
}

void Vid::hecateEvent(HecateEvent& e) {
  cout << "Shots: {";
  for (auto s : e.shots) {
    cout << "[" << get<0>(s) << ":" << get<1>(s) << "], ";
  }
  cout << "}" << endl << endl;

  cout << "Keyframes: " + ofToString(e.keyframes) << endl << endl;
  cout << "Command: " + ofToString(e.cmd) << endl << endl;
  // cout << "RAW: " + ofToString(e.raw) << endl << endl;
}
