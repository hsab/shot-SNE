#include "vid.h"

Vid::Vid() {}

Vid::~Vid() {
   closeVideo();
   hecateClose();
}

void Vid::setup(string file, string* hecate, int _w, int _h) {
   filePath = file;
   hecatePath = hecate;

   // setup viddata
   D.setup(&json, &renderedKeyframes, file);
   bool analysisExists = D.detectExistingData();

   string ffstr = ffprobe(file);
   D.populateFFProbe(ffstr);
   initStats();
   setupCoordinates(_w, _h);
   calculateCoordinates(width, height, wn, hn, left, top);

   ofAddListener(HecateEvent::events, this, &Vid::hecateEvent);

   verdana14.load("verdana.ttf", 12, true, true);
   verdana14.setLineHeight(18.0f);
   verdana14.setLetterSpacing(1.037);

   gui.setup();
   gui.add(minArea.set("Min area", 0, 1, 5000));
   gui.add(maxArea.set("Max area", 5000, 1, 5000));
   gui.add(threshold.set("Threshold", 0, 0, 255));
   gui.add(holes.set("Holes", true));
}

string Vid::ffprobe(string filePath) {
   string ffcmd = "ffprobe -v quiet -print_format json -show_format -show_streams -show_packets " + filePath;
   //  const char* command = ffcmd.c_str();
   std::array<char, 256> buffer;
   std::string result;
   std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(ffcmd.c_str(), "r"), pclose);
   if (!pipe)
      throw std::runtime_error("popen() failed!");
   while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
      result += buffer.data();

   if (result != "") {
      return result;
   } else
      throw std::runtime_error("FFPROBE failed to execute");
}

void Vid::setupCoordinates(int w, int h) {
   width = w;
   height = h;
}

void Vid::openVideo() {
   player = new ofGstVideoPlayer;
   player->setFrameByFrame(true);
   video.setPlayer(ofPtr<ofGstVideoPlayer>());

   video.loadAsync(filePath.get());
   video.setLoopState(OF_LOOP_NONE);
   video.stop();
   inView = true;
   frameByframe = false;
   stats.closed = false;
   stats.stopped = true;

   frameBuffer.allocate(500, 500, GL_RGB);
}

void Vid::closeVideo() {
   inView = false;
   frameByframe = false;
   stats.closed = true;
   stats.stopped = true;
   video.closeMovie();
}

void Vid::initStats() {
   stats.width = json["width"].asInt();
   stats.height = json["height"].asInt();
   stats.frames = json["frames"].asInt();
   stats.duration = json["duration"].asFloat();
   stats.fps = json["fps"].asFloat();
   stats.ratio = json["ratio"].asFloat();
}

void Vid::updateStats() {
   //  if (!stats.closed) {
   // stats.updateDimension(video.getWidth(), video.getHeight());
   // stats.updateTotalFrames(video.getTotalNumFrames());
   // calculateCoordinates(width, height, wn, hn, left, top);
   // int w, int h, int& wn, int& hn, int& left, int& top
   //     if (!stats.stopped) {
   //        stats.updateDuration(video.getDuration());
   //        stats.updateCurrentTimeInfo(video.getPosition());
   //        stats.keyframeIndex = keyframeIndex;
   //     } else {
   //        stats.resetCurrentTime();
   //     }
   //  } else {
   //     stats.resetCurrentTime();
   //  }
   if (!stats.closed && !stats.stopped) {
      stats.updateDuration(video.getDuration());
      stats.updateCurrentTimeInfo(video.getPosition());
      stats.keyframeIndex = keyframeIndex;
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
           "%s/frame_%05d.jpeg",
           json["info"]["rendersDir"].asCString(),
           video.getCurrentFrame());
   saveImage.setImageType(OF_IMAGE_COLOR);
   saveImage.save(filename);
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
   if (hecateThread != nullptr && hecateThread->isProcessed() && hecateThread->isThreadRunning()) {
      hecateClose();
   }
   updateStats();
}

void Vid::calculateCoordinates(int w, int h, int& wn, int& hn, int& left, int& top) {
   int wv = stats.width;
   int hv = stats.height;
   float rv = wv / hv;
   float r = w / h;

   wn = r > rv ? (wv * h / hv) : (w);
   hn = r > rv ? (h) : (hv * w / wv);

   left = (ofGetWidth() - wn) / 2;
   top = (ofGetHeight() - hn) / 2;
}

void Vid::draw() {
   gui.draw();

   if (inView) {
      ofSetHexColor(0xffffff);

      video.draw(left, top, wn, hn);

      if (video.isPlaying()) {
         int currframe = video.getCurrentFrame();
         bool currentFrameIsKeyframe = (keyframesMap.find(currframe) != keyframesMap.end());

         float x, y, x2, fullW;
         x = tlo;
         x2 = ofGetWidth() - tlo;
         y = ofGetHeight() - tlo - tlh;
         fullW = ofGetWidth() - (tlo * 2);

         ofSetColor(150, 150, 150);
         for (auto key : shots) {
            x = ((float)get<0>(key) / (float)stats.frames) * fullW + tlo;
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

         x = tlo;
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

void Vid::windowResized() {
   calculateCoordinates(width, height, wn, hn, left, top);
}

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
   if (e.path == filePath.get()) {
      hecateDone = D.processEvent(e);
      if (!hecateDone)
         ofLogError("FAILED TO SAVE FILE");
   }
}

// void Vid::processEvent(HecateEvent& e) {
//    json["info"]["baseName"] = baseName;
//    json["info"]["extension"] = extension;
//    json["info"]["enclosingDir"] = enclosingDirectoryPath;
//    json["info"]["vidmanDir"] = vidmanDirectoryPath;
//    json["info"]["rendersDir"] = rendersDirectoryPath;

//    ofxJSON ffprobe;
//    ffprobe.parse(e.ffraw);
//    float t1 = ffprobe["streams"][0]["width"].asInt();
//    float t2 = ffprobe["streams"][0]["height"].asInt();
//    string sampleRatio = ffprobe["streams"][0]["sample_aspect_ratio"].asString();
//    string displayRatio = ffprobe["streams"][0]["display_aspect_ratio"].asString();
//    string fps = ffprobe["streams"][0]["r_frame_rate"].asString();
//    string duration = ffprobe["streams"][0]["duration"].asString();
//    string frames = ffprobe["streams"][0]["nb_frames"].asString();

//    vector<string> t;

//    json["width"] = t1;
//    json["height"] = t2;

//    t = ofSplitString(sampleRatio, ":");
//    t1 = stof(t[0]);
//    t2 = stof(t[1]);
//    json["sampleRatio"] = t1 / t2;

//    t = ofSplitString(displayRatio, ":");
//    t1 = stof(t[0]);
//    t2 = stof(t[1]);
//    json["ratio"] = t1 / t2;

//    t = ofSplitString(fps, "/");
//    t1 = stof(t[0]);
//    t2 = stof(t[1]);
//    json["fps"] = t1 / t2;

//    json["duration"] = stof(duration);
//    json["frames"] = stof(frames);

//    processHecateResults(e.hecraw);
//    prepareHecateOutput();

//    ofBuffer resultBuffer;
//    resultBuffer.set(e.hecraw.c_str(), e.hecraw.size());
//    hecateSavedOutput.writeFromBuffer(resultBuffer);
//    json.save(analysisSavedOutputPath, true);
// }

// void Vid::processHecateResults(string result) {
//   // Remove Hecate header info
//   Json::Value jshots(Json::arrayValue);
//   json["shots"] = jshots;
//   Json::Value jkeyframes(Json::arrayValue);
//   json["keyframes"] = jshots;

//   vector<string> clean = ofSplitString(result, "shots:");
//   clean = ofSplitString(clean[1], "keyframes:");

//   // Clean shots' brackets
//   ofStringReplace(clean[0], "[", "");
//   ofStringReplace(clean[0], "]", "");
//   // Clean keyframes' brackets
//   ofStringReplace(clean[1], "[", "");
//   ofStringReplace(clean[1], "]", "");

//   vector<string> temp;
//   vector<string> temp2;
//   temp = ofSplitString(clean[0], ",");
//   for (auto s : temp) {
//     temp2 = ofSplitString(s, ":");

//     ofxJSON shot;
//     int start = stoi(temp2[0]);
//     int end = stoi(temp2[1]);
//     shot["start"] = start;
//     shot["end"] = end;
//     shot["frames"] = end - start;
//     shot["length"] = shot["frames"].asFloat() / json["fps"].asFloat();
//     Json::Value kfs(Json::arrayValue);
//     shot["keyframes"] = kfs;
//     shot["bestFrame"] = "NONE";
//     json["shots"].append(shot);
//     shots.push_back(make_tuple(stoi(temp2[0]), stoi(temp2[1])));
//   }

//   for (auto s : json["shots"]) {
//     cout << s["start"].asString() + " " +
//             s["end"].asString() + " " + s["length"].asString()
//          << endl;
//   }

//   // Prepare keyframes
//   stringstream ssk(clean[1]);

//   int key = 0;
//   while (ssk >> key) {
//     bool keyframeInShot = false;

//     int i = 0;
//     for (auto s : shots) {
//       float start = json["shots"][i]["start"].asFloat();
//       float end = json["shots"][i]["end"].asFloat();
//       float ideal = (end - start) / 2;

//       if (key >= get<0>(s) && key <= get<1>(s)) {
//         keyframeInShot = true;
//         keyframesToShotsMap[key] = i;
//         json["shots"][i]["keyframes"].append(key);

//         if (json["shots"][i]["bestFrame"] == "NONE")
//           json["shots"][i]["bestFrame"] = key;
//         else {
//           float currDist = ideal - (key - start);
//           currDist = abs(currDist);
//           float lastBest = json["shots"][i]["bestFrame"].asFloat();
//           float lasDist = ideal - (lastBest - start);
//           lasDist = abs(lasDist);

//           if (currDist < lasDist)
//             json["shots"][i]["bestFrame"] = key - start;
//         }
//         break;
//       }
//       i++;
//     }

//     if (keyframeInShot) {
//       keyframes.push_back(key);
//       ofxJSON jkey;
//       jkey["frame"] = key;
//       jkey["shot"] = i;
//       json["keyframes"].append(jkey);
//     }

//     if (ssk.peek() == ',' || ssk.peek() == ' ')
//       ssk.ignore();
//   }

//   for (auto kf : keyframes)
//     keyframesMap[kf] = false;

//   hecateDone = true;
// }