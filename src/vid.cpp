#include "vid.h"

Vid::Vid() {}

Vid::~Vid() {
  closeVideo();
  hecateClose();
}

void Vid::setup(string file, string* hecate) {
  filePath   = file;
  hecatePath = hecate;

  bool hecateOutputExists = prepareDataFolder(true);
  if (hecateOutputExists) {
    ofBuffer saved = hecateSavedOutput.readToBuffer();
    processHecateResults(saved.getText());
  }

  ofAddListener(HecateEvent::events, this, &Vid::hecateEvent);
  // ofAddListener(ofEvents().keyPressed, this, &Vid::keyPressed);
}

void Vid::setupCoordinates(int w, int h) {
  width  = w;
  height = h;
}

bool Vid::prepareDataFolder(bool readIfExists) {
  baseName               = ofFilePath::getBaseName(filePath.get());
  enclosingDirectoryPath = ofFilePath::getEnclosingDirectory(filePath.get(), false);

  vidmanDirectoryPath = ofFilePath::join(enclosingDirectoryPath, baseName + "_vidman");
  vidmanDirectoryPath = ofFilePath::getPathForDirectory(vidmanDirectoryPath);

  rendersDirectoryPath = ofFilePath::join(vidmanDirectoryPath, "renders");
  rendersDirectoryPath = ofFilePath::getPathForDirectory(rendersDirectoryPath);

  vidmanDirectory  = ofDirectory(vidmanDirectoryPath);
  rendersDirectory = ofDirectory(rendersDirectoryPath);

  if (!vidmanDirectory.isDirectory()) {
    ofDirectory::createDirectory(vidmanDirectoryPath, false);
    vidmanDirectory.createDirectory(rendersDirectoryPath, false);
  }

  if (!rendersDirectory.isDirectory()) {
    ofDirectory::createDirectory(rendersDirectoryPath, false);
  }

  string hecateSavedOutputPath = vidmanDirectoryPath + ".hecateOutput.txt";
  hecateSavedOutput            = ofFile(hecateSavedOutputPath, ofFile::ReadWrite, false);

  if (hecateSavedOutput.exists()) {
    hecateSavedOutput.open(hecateSavedOutputPath, ofFile::ReadWrite, false);
    return true;
  } else {
    return false;
  }
}

void Vid::prepareHecateOutput() {
  bool hecateOutputExists      = prepareDataFolder(true);
  string hecateSavedOutputPath = hecateSavedOutput.path();

  if (hecateOutputExists) {
    hecateSavedOutput.removeFile(hecateSavedOutputPath, false);
  }

  hecateSavedOutput = ofFile(hecateSavedOutputPath, ofFile::ReadWrite, false);
  hecateSavedOutput.create();
  hecateSavedOutput.open(hecateSavedOutputPath, ofFile::ReadWrite, false);
}

void Vid::openVideo() {
  player = new ofGstVideoPlayer;
  player->setFrameByFrame(true);
  video.setPlayer(ofPtr<ofGstVideoPlayer>());

  video.loadAsync(filePath.get());
  video.setLoopState(OF_LOOP_NONE);
  video.stop();
  inView        = true;
  frameByframe  = false;
  stats.closed  = false;
  stats.stopped = true;

  frameBuffer.allocate(width, height, GL_RGB);
  renderFolder = "renders/";
}

void Vid::closeVideo() {
  inView        = false;
  frameByframe  = false;
  stats.closed  = true;
  stats.stopped = true;
  video.closeMovie();
}

void Vid::initStats() {
  updateStats();
}

void Vid::updateStats() {
  if (!stats.closed) {
    stats.updateDimension(video.getWidth(), video.getHeight());
    stats.updateTotalFrames(video.getTotalNumFrames());
    calculateCoordinates(width, height);
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
  }
}

void Vid::stop() {
  stats.stopped = true;
  if (!stats.closed) {
    video.stop();
  }
}

void Vid::pause() {
  if (stats.stopped) {
    play();
    return;
  }
  video.setPaused(!video.isPaused());
}

void Vid::prepareFrameByFrame() {
  if (!stats.closed) {
    frameByframe = true;
    if (isReadyForKeyframeNavigation()) {
      if (stats.stopped)
        play();
      video.setPaused(true);
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
      if (keyframes[i] > curr)
        break;
    }
  }

  if (flag == 'n') {
    keyframeIndex = i - 1;
    if (same)
      keyframeIndex = i;
    if (isReadyForKeyframeNavigation())
      nextKeyframe();
    return;
  }

  if (flag == 'p') {
    keyframeIndex = i;
    if (same)
      keyframeIndex = i;
    if (isReadyForKeyframeNavigation())
      previousKeyframe();
    return;
  }
}

bool Vid::isReadyForKeyframeNavigation() {
  bool a, b;
  a = frameByframe && !stats.closed && !stats.stopped && hecateDone;
  b = !keyframes.empty() && !shots.empty();

  return (a && b);
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
  frameBuffer.begin();
  ofClear(0, 0, 0);
  video.draw(0, 0, width, height);
  frameBuffer.end();

  ofImage saveImage;
  frameBuffer.readToPixels(saveImage.getPixelsRef());
  char filename[1024];
  sprintf(filename,
          "%s/frame_%05d.png",
          renderFolder.c_str(),
          video.getCurrentFrame());
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

void Vid::setViewed(bool flag) {
  inView = flag;
}
bool Vid::isViewed() {
  return inView;
}

void Vid::update() {
  if (inView)
    video.update();
  if (hecateThread != nullptr && hecateThread->isProcessed() &&
      hecateThread->isThreadRunning()) {
    hecateClose();
  }
  updateStats();
}

void Vid::calculateCoordinates(int w, int h) {
  int wv   = stats.width;
  int hv   = stats.height;
  float rv = wv / hv;
  float r  = w / h;

  wn = r > rv ? (wv * h / hv) : (w);
  hn = r > rv ? (h) : (hv * w / wv);

  left = (ofGetWidth() - wn) / 2;
  top  = (ofGetHeight() - hn) / 2;
}

void Vid::draw() {
  if (inView) {
    ofSetHexColor(0xffffff);
    video.draw(left, top, wn, hn);

    int tlh = 50;
    int tlo = 20;

    if (video.isPlaying()) {
      ofSetHexColor(0x222222);

      ofDrawCircle(left, top + hn + (tlh / 2) + tlo, 5);
      ofDrawCircle(left + wn, top + hn + (tlh / 2) + tlo, 5);

      ofEnableAlphaBlending();
      ofSetColor(255, 255, 255, 150);
      ofSetLineWidth(1.0f);
      float x, y, x2;
      y = top + hn + tlo;
      for (auto key : keyframes) {
        x = ((float)key / (float)stats.frames) * wn + left;
        ofDrawLine(x, y, x, y + tlh);
      }
      ofDisableAlphaBlending();

      for (auto key : shots) {
        x  = ((float)get<0>(key) / (float)stats.frames) * wn + left;
        x2 = ((float)get<1>(key) / (float)stats.frames) * wn + left;
        ofDrawRectangle(x, top + hn + tlo + (tlh / 4), x2 - x, tlh / 2);
      }

      ofSetColor(255, 0, 0, 0);
      ofSetLineWidth(2.0f);
      x = (video.getCurrentFrame() / (float)stats.frames) * wn + left;
      ofDrawLine(x, y, x, y + 50);
    }
  }
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
  // shots = e.shots;
  // keyframes = e.keyframes;
  processHecateResults(e.raw);

  prepareHecateOutput();

  ofBuffer resultBuffer;
  resultBuffer.set(e.raw.c_str(), e.raw.size());
  hecateSavedOutput.writeFromBuffer(resultBuffer);

  cout << "Shots: {";
  for (auto s : e.shots) {
    cout << "[" << get<0>(s) << ":" << get<1>(s) << "], ";
  }
  cout << "}" << endl
       << endl;

  cout << "Keyframes: " + ofToString(e.keyframes) << endl
       << endl;
  cout << "Command: " + ofToString(e.cmd) << endl
       << endl;
  // cout << "RAW: " + ofToString(e.raw) << endl << endl;
}

void Vid::processHecateResults(string result) {
  // Remove Hecate header info
  vector<string> clean = ofSplitString(result, "shots:");
  clean                = ofSplitString(clean[1], "keyframes:");

  // Clean shots' brackets
  ofStringReplace(clean[0], "[", "");
  ofStringReplace(clean[0], "]", "");
  // Clean keyframes' brackets
  ofStringReplace(clean[1], "[", "");
  ofStringReplace(clean[1], "]", "");

  vector<string> temp;
  vector<string> temp2;
  temp = ofSplitString(clean[0], ",");
  for (auto s : temp) {
    temp2 = ofSplitString(s, ":");
    shots.push_back(make_tuple(stoi(temp2[0]), stoi(temp2[1])));
  }

  // Prepare keyframes
  stringstream ssk(clean[1]);

  int i = 0;
  while (ssk >> i) {
    keyframes.push_back(i);

    if (ssk.peek() == ',' || ssk.peek() == ' ') {
      ssk.ignore();
    }
  }

  hecateDone = true;
}
