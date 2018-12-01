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

  processRendersFolder(hecateOutputExists);

  ofAddListener(HecateEvent::events, this, &Vid::hecateEvent);
  // ofAddListener(ofEvents().keyPressed, this, &Vid::keyPressed);

  verdana14.load("verdana.ttf", 12, true, true);
  verdana14.setLineHeight(18.0f);
  verdana14.setLetterSpacing(1.037);

  gui.setup();
  gui.add(minArea.set("Min area", 0, 1, 5000));
  gui.add(maxArea.set("Max area", 5000, 1, 5000));
  gui.add(threshold.set("Threshold", 0, 0, 255));
  gui.add(holes.set("Holes", true));
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

void Vid::processRendersFolder(bool hecateOutputExists) {
  vector<ofFile> renders = rendersDirectory.getFiles();
  if (hecateOutputExists && renders.size() > 0) {
    for (auto file : renders) {
      string renderName    = ofFilePath::getBaseName(file.getAbsolutePath());
      vector<string> split = ofSplitString(renderName, "_");
      if (split.size() > 0) {
        string frameNumber = split[split.size() - 1];
        frameNumber.erase(0, min(frameNumber.find_first_not_of('0'), frameNumber.size() - 1));
        int fnum        = stoi(frameNumber);
        bool isKeyframe = (keyframesMap.find(fnum) != keyframesMap.end());
        if (isKeyframe) {
          keyframesMap[fnum] = true;
        } else {
          ofFile::removeFile(file.getAbsolutePath(), false);
        }
      }
    }
  }

  if (!hecateOutputExists && renders.size() > 0) {
    for (auto file : renders) {
      ofFile::removeFile(file.getAbsolutePath(), false);
    }
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

  frameBuffer.allocate(128, 128, GL_RGB);
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
    calculateCoordinates(width, height, wn, hn, left, top);
    // int w, int h, int& wn, int& hn, int& left, int& top
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

    int limit     = keyframes.size() - 1;
    int currFrame = video.getCurrentFrame();

    int temp = keyframeIndex;
    for (int i = 0; i < keyframes.size(); i++) {
      if (keyframes[i] > currFrame) {
        keyframeIndex = i;
        break;
      }
    }

    // if (keyframeIndex < limit) {
    //   keyframeIndex++;
    // } else {
    //   keyframeIndex = limit;
    // }
    cout << stats.toString() << "Keyframe: " << keyframes[keyframeIndex]
         << endl;

    if (keyframeIndex != temp) {
      video.setFrame(keyframes[keyframeIndex]);
      video.update();
    }
  } else {
    prepareFrameByFrame();
    alignIndexToFrame('n');
  }
}

void Vid::previousKeyframe() {
  if (isReadyForKeyframeNavigation()) {
    prepareFrameByFrame();

    // if (keyframeIndex > 0) {
    //   keyframeIndex--;
    // } else {
    //   keyframeIndex = 0;
    // }

    int currFrame = video.getCurrentFrame();

    int temp = keyframeIndex;

    for (int i = keyframes.size() - 1; i >= 0; i--) {
      if (keyframes[i] < currFrame) {
        keyframeIndex = i;
        break;
      }
    }

    cout << stats.toString() << "Keyframe: " << keyframes[keyframeIndex]
         << endl;

    if (keyframeIndex != temp) {
      video.setFrame(keyframes[keyframeIndex]);
    }
  } else {
    prepareFrameByFrame();
    alignIndexToFrame('p');
  }
}

void Vid::prepareBuffer() {
  frameBuffer.begin();
  ofClear(0, 0, 0);
  ofSetHexColor(0xffffff);
  calculateCoordinates(frameBuffer.getWidth(), frameBuffer.getHeight(), fboWidth, fboHeight, fboLeft, fboTop);
  video.draw(0, 0, fboWidth, fboHeight);
  frameBuffer.end();

  frameBuffer.readToPixels(saveImage.getPixelsRef());
  saveImage.update();

  contourFinder.setMinAreaRadius(minArea);
  contourFinder.setMaxAreaRadius(maxArea);
  contourFinder.setThreshold(threshold);
  contourFinder.findContours(saveImage);
  contourFinder.setFindHoles(false);
}

void Vid::updateBuffer() {
  bool keyframeExists = (keyframesMap.find(video.getCurrentFrame()) != keyframesMap.end());
  if (keyframeExists) {
    prepareBuffer();
  }
}

void Vid::renderCurrentFrame() {
  prepareBuffer();

  int cx = 0, cy = 0, cw = frameBuffer.getWidth(), ch = frameBuffer.getHeight();
  cv::Rect rc;
  int area = 0;
  if (contourFinder.size() > 0) {
    for (int i = 0; i < contourFinder.size(); i++) {
      rc = contourFinder.getBoundingRect(i);
      if (rc.area() > area) {
        area = rc.area();
        cout << "RECT: " << cx << " " << cy << " " << cw << " " << ch << endl;

        cx = rc.x;       // cx = (rc.x > cx) ? rc.x : cx;
        cy = rc.y;       // cy = (rc.y > cy) ? rc.y : cy;
        cw = rc.width;   // cw = (rc.width < cw) ? rc.width : cw;
        ch = rc.height;  // ch = (rc.height < ch) ? rc.height : ch;
      }
    }
  }

  saveImage.crop(cx, cy, cw, ch);
  saveImage.update();
  cout << "FIN: " << cx << " " << cy << " " << cw << " " << ch << endl;

  char filename[1024];
  sprintf(filename,
          "%s/frame_%05d.png",
          rendersDirectoryPath.c_str(),
          video.getCurrentFrame());
  saveImage.saveImage(filename);
  keyframesMap[video.getCurrentFrame()] = true;
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
  if (inView) {
    video.update();
    updateBuffer();
  }
  if (hecateThread != nullptr && hecateThread->isProcessed() &&
      hecateThread->isThreadRunning()) {
    hecateClose();
  }
  updateStats();
}

void Vid::calculateCoordinates(int w, int h, int& wn, int& hn, int& left, int& top) {
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
  gui.draw();

  if (inView) {
    ofSetHexColor(0xffffff);

    video.draw(left, top, wn, hn);

    if (video.isPlaying()) {
      int currframe               = video.getCurrentFrame();
      bool currentFrameIsKeyframe = (keyframesMap.find(currframe) != keyframesMap.end());

      float x, y, x2, fullW;
      x     = tlo;
      x2    = ofGetWidth() - tlo;
      y     = ofGetHeight() - tlo - tlh;
      fullW = ofGetWidth() - (tlo * 2);

      ofSetColor(150, 150, 150);
      for (auto key : shots) {
        x  = ((float)get<0>(key) / (float)stats.frames) * fullW + tlo;
        x2 = ((float)get<1>(key) / (float)stats.frames) * fullW + tlo;
        ofDrawRectangle(x, y + tlh / 5, x2 - x, tlh / 5 * 3);
      }

      ofEnableAlphaBlending();
      ofSetColor(255, 255, 255, 150);
      ofSetLineWidth(1.0f);

      for (auto key : keyframes) {
        x = ((float)key / (float)stats.frames) * fullW + tlo;
        if (keyframesMap[key]) {
          ofSetHexColor(0x00ff00);
          ofDrawLine(x, y - (tlo / 8), x, y + tlh + (tlo / 4));
          ofSetHexColor(0xffffff);
        } else {
          ofDrawLine(x, y, x, y + tlh);
        }
      }
      ofDisableAlphaBlending();

      x  = tlo;
      x2 = ofGetWidth() - tlo;

      ofSetHexColor(0x222222);
      ofDrawCircle(x, y + tlh / 2, 5);
      ofDrawCircle(x2, y + tlh / 2, 5);
      ofDrawLine(x, y, x, y + tlh);
      ofDrawLine(x2, y, x2, y + tlh);

      ofSetColor(255, 0, 0, 0);
      ofSetLineWidth(2.0f);
      x = (currframe / (float)stats.frames) * fullW + tlo;
      ofDrawLine(x, y, x, y + tlh);
      ofSetHexColor(0xffffff);

      string keyframeStr = (currentFrameIsKeyframe) ? "Keyframe: " + to_string(keyframes[keyframeIndex]) : "N/A";
      char text[1024];
      sprintf(text,
              "%s\n%d / %d\n%.2f / %.2fs\n%d / \%100\n%dx%d\n%d detected shots\n%d detected keyframes",
              keyframeStr.c_str(),
              currframe, stats.frames,
              stats.currentTime, stats.duration,
              (int)(stats.position * 100),
              stats.width, stats.height,
              shots.size(), keyframes.size());
      ofRectangle rect = verdana14.getStringBoundingBox(text, 0, 0);
      ofSetHexColor(0x222222);
      verdana14.drawString(text, tlo, y - tlo - rect.height - rect.y);

      // if (contourFinder.size() > 0 && video.isPaused() && holes && currentFrameIsKeyframe && keyframesMap[currframe]) {
      if (contourFinder.size() > 0 && holes && currentFrameIsKeyframe) {
        ofEnableAlphaBlending();
        ofSetColor(255, 0, 0, 150);
        ofTranslate(left, top);
        ofScale(float(wn) / fboWidth, float(wn) / fboWidth, 1);
        contourFinder.draw();
        ofTranslate(0, 0);
        ofScale(1, 1, 1);
        ofDisableAlphaBlending();
        ofSetHexColor(0xffffff);
      }
    }
  }
}

void Vid::keyPressed(ofKeyEventArgs& e) {}

void Vid::keyPressed(int key) {
  if (key == 'h')
    hecate(*hecatePath);

  if (key == 'H')
    hecateClose();

  if (key == 'i') {
    vidStatVerbose();
    cout << stats.toString() << endl;
  }

  if (key == 'v')
    setViewed(!isViewed());

  if (key == 'p')
    play();

  if (key == 's')
    stop();

  if (key == 'r')
    renderCurrentFrame();

  if (key == 'R')
    renderKeyframes();

  if (key == ' ')
    pause();

  if (key == OF_KEY_UP)
    nextKeyframe();

  if (key == OF_KEY_DOWN)
    previousKeyframe();

  if (key == OF_KEY_LEFT)
    video.previousFrame();

  if (key == OF_KEY_RIGHT)
    video.nextFrame();

  if (key == 'o')
    openVideo();

  if (key == 'c')
    closeVideo();
}

void Vid::mouseDragged(int x, int y, int button) {
  if (button == 0 && x >= tlo && x <= (ofGetWidth() - tlo)) {
    float pct = (float)(x - tlo) / float(ofGetWidth() - tlo - tlo);
    video.setPosition(pct);
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
    bool keyframeInShot = false;

    int k = 0;
    for (auto s : shots) {
      if (i >= get<0>(s) && i <= get<1>(s)) {
        keyframeInShot         = true;
        keyframesToShotsMap[i] = k;
        break;
      }
      k++;
    }

    if (keyframeInShot)
      keyframes.push_back(i);

    if (ssk.peek() == ',' || ssk.peek() == ' ') {
      ssk.ignore();
    }
  }

  for (auto kf : keyframes) {
    keyframesMap[kf] = false;
  }

  hecateDone = true;

  cout << "Shots: {";
  for (auto s : shots) {
    cout << "[" << get<0>(s) << ":" << get<1>(s) << "], ";
  }
  cout << "}" << endl
       << endl;

  cout << "Keyframes: " + ofToString(keyframes) << endl
       << endl;
}
