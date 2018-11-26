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
  frameByframe = false;
  stats.closed = false;
  stats.stopped = true;

  frameBuffer.allocate(width, height, GL_RGB);
  renderFolder = "renders/";
}

void Vid::closeVideo() {
  inView = false;
  frameByframe = false;
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
      stats.updateCurrentTimeInfo(video.getPosition());
      stats.keyframeIndex = keyframeIndex;
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
    video.setPaused(false);
    frameByframe = false;
    // updateStats();
  }
}

void Vid::stop() {
  stats.stopped = true;
  if (!stats.closed) {
    video.stop();
    // updateStats();
  }
}

void Vid::pause() {
  if (stats.stopped) {
    play();
    return;
  }
  video.setPaused(!video.isPaused());
  // updateStats();
}

void Vid::prepareFrameByFrame() {
  if (!stats.closed) {
    frameByframe = true;
    if (isReadyForKeyframeNavigation()) {
      if (stats.stopped) play();
      video.setPaused(true);
      // updateStats();
    }
  }
}

void Vid::alignIndexToFrame(char flag) {
  int i;
  bool same = false;

  int curr = stats.currentFrame;
  if (curr == -1) {
    i = 0;
  } else {
    for (i = 0; i < keyframes.size(); i++) {
      if (keyframes[i] == curr) {
        same = true;
        break;
      }
      if (keyframes[i] > curr) break;
    }
  }

  if (flag == 'n') {
    keyframeIndex = i - 1;
    if (same) {
      keyframeIndex = i;
    }
    if (isReadyForKeyframeNavigation()) nextKeyframe();
    return;
  }

  if (flag == 'p') {
    keyframeIndex = i;
    // if (same) {
    //   keyframeIndex = i;
    // }
    if (isReadyForKeyframeNavigation()) previousKeyframe();
    return;
  }
}

bool Vid::isReadyForKeyframeNavigation() {
  bool one, two;
  one = frameByframe && !stats.closed && !stats.stopped && hecateDone;
  two = !keyframes.empty() && !shots.empty();
  // return (frameByframe && !stats.closed &&
  // !stats.stopped && hecateDone &&
  //         !keyframes.empty() && !shots.empty());
  return (one && two);
}

void Vid::nextKeyframe() {
  if (isReadyForKeyframeNavigation()) {
    prepareFrameByFrame();

    int limit = keyframes.size() - 1;
    if (keyframeIndex < limit) {
      keyframeIndex++;
    } else {
      keyframeIndex = limit;
    }
    cout << stats.toString() << "Keyframe: " << keyframes[keyframeIndex]
         << endl;

    video.setFrame(keyframes[keyframeIndex]);
    video.update();
  } else {
    prepareFrameByFrame();
    alignIndexToFrame('n');
  }
}

void Vid::previousKeyframe() {
  if (isReadyForKeyframeNavigation()) {
    prepareFrameByFrame();

    if (keyframeIndex > 0) {
      keyframeIndex--;
    } else {
      keyframeIndex = 0;
    }
    cout << stats.toString() << "Keyframe: " << keyframes[keyframeIndex]
         << endl;

    video.setFrame(keyframes[keyframeIndex]);
  } else {
    prepareFrameByFrame();
    alignIndexToFrame('p');
  }
}

void Vid::renderCurrentFrame() {
  // draw the video with the shader into the frame buffer
  frameBuffer.begin();
  ofClear(0, 0, 0);
  video.draw(20, 20, width, height);
  frameBuffer.end();

  ofImage saveImage;
  frameBuffer.readToPixels(saveImage.getPixelsRef());
  char filename[1024];
  sprintf(filename, "%s/frame_%05d.png", renderFolder.c_str(),
          stats.currentFrame);
  saveImage.saveImage(filename);
}

void Vid::renderKeyframes() {
  if (hecateDone && !keyframes.empty() && !shots.empty()) {
    prepareFrameByFrame();
    video.firstFrame();
    video.setPaused(true);
    for (auto i : keyframes) {
      video.setFrame(i);
      video.update();
      renderCurrentFrame();
    }
  }
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
  updateStats();
}

void Vid::draw() {
  if (inView) video.draw(20, 20, width, height);
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
    // updateStats();
    cout << stats.toString() << endl;
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

  if (key == 'r') {
    renderCurrentFrame();
  }

  if (key == 'R') {
    renderKeyframes();
  }

  if (key == ' ') {
    pause();
  }

  if (key == OF_KEY_LEFT) {
    previousKeyframe();
  }

  if (key == OF_KEY_RIGHT) {
    nextKeyframe();
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
  hecateDone = true;
  shots = e.shots;
  keyframes = e.keyframes;
  cout << "Shots: {";
  for (auto s : e.shots) {
    cout << "[" << get<0>(s) << ":" << get<1>(s) << "], ";
  }
  cout << "}" << endl << endl;

  cout << "Keyframes: " + ofToString(e.keyframes) << endl << endl;
  cout << "Command: " + ofToString(e.cmd) << endl << endl;
  // cout << "RAW: " + ofToString(e.raw) << endl << endl;
}
