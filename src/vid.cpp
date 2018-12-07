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
   vidData.setup(&json, file);
   hecateDone = vidData.detectExistingData();

   if (!hecateDone) {
      string ffstr = ffprobe(file);
      vidData.populateFFProbe(ffstr);
   }

   initVidStats();
   setupCoordinates(_w, _h);
   calculateCoordinates(width, height, wn, hn, left, top);

   ofAddListener(HecateEvent::events, this, &Vid::hecateEvent);

   infoFont.load("verdana.ttf", 12, true, true);
   infoFont.setLineHeight(18.0f);
   infoFont.setLetterSpacing(1.037);

   // gui.setup();
   parameters.add(minArea.set("Min area", 0, 1, 5000));
   parameters.add(maxArea.set("Max area", 5000, 1, 5000));
   parameters.add(threshold.set("Threshold", 0, 0, 255));
   parameters.add(showContour.set("Show", true));
}

string Vid::ffprobe(string filePath) {
   string ffcmd = "ffprobe -v quiet -print_format json -show_format -show_streams -show_packets " + filePath;
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
   calculateCoordinates(width, height, wn, hn, left, top);

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

void Vid::initVidStats() {
   stats.width = json["width"].asInt();
   stats.height = json["height"].asInt();
   stats.frames = json["frames"].asInt();
   stats.duration = json["duration"].asFloat();
   stats.fps = json["fps"].asFloat();
   stats.ratio = json["ratio"].asFloat();
}

void Vid::updateVidStats() {
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
      for (i = 0; i < json["keyframes"].size(); i++) {
         if (json["keyframes"][i].asInt() == curr) {
            same = true;
            break;
         }
         if (json["keyframes"][i].asInt() > curr)
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
   b = !json["keyframes"].empty() && !json["shots"].empty();

   return (a && b);
}

void Vid::nextKeyframe() {
   if (isReadyForKeyframeNavigation()) {
      prepareFrameByFrame();

      int limit = json["keyframes"].size() - 1;
      int currFrame = video.getCurrentFrame();

      int temp = keyframeIndex;
      for (int i = 0; i < json["keyframes"].size(); i++) {
         if (json["keyframes"][i].asInt() > currFrame) {
            keyframeIndex = i;
            break;
         }
      }

      cout << stats.toString() << "Keyframe: " << json["keyframes"][keyframeIndex].asString() << endl;

      if (keyframeIndex != temp) {
         video.setFrame(json["keyframes"][keyframeIndex].asInt());
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

      int currFrame = video.getCurrentFrame();

      int temp = keyframeIndex;

      for (int i = json["keyframes"].size() - 1; i >= 0; i--) {
         if (json["keyframes"][i].asInt() < currFrame) {
            keyframeIndex = i;
            break;
         }
      }

      if (keyframeIndex != temp) {
         video.setFrame(json["keyframes"][keyframeIndex].asInt());
      }
   } else {
      prepareFrameByFrame();
      alignIndexToFrame('p');
   }
}

void Vid::updateBuffer() {
   bool keyframeExists = false;

   if (video.getCurrentFrame() >= 0)
      keyframeExists = json["renderedKeyframes"][video.getCurrentFrame()].asBool();

   if (keyframeExists)
      prepareBuffer();
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
            cx = rc.x;       // cx = (rc.x > cx) ? rc.x : cx;
            cy = rc.y;       // cy = (rc.y > cy) ? rc.y : cy;
            cw = rc.width;   // cw = (rc.width < cw) ? rc.width : cw;
            ch = rc.height;  // ch = (rc.height < ch) ? rc.height : ch;
         }
      }
   }

   saveImage.crop(cx, cy, cw, ch);
   saveImage.update();

   char filename[1024];
   sprintf(filename,
           "%s/frame_%05d.jpeg",
           json["info"]["rendersDir"].asCString(),
           video.getCurrentFrame());
   saveImage.setImageType(OF_IMAGE_COLOR);
   saveImage.save(filename);
   json["renderedKeyframes"][video.getCurrentFrame()] = true;
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
   updateVidStats();
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
   if (inView) {
      ofSetHexColor(0xffffff);

      video.draw(left, top, wn, hn);

      if (video.isPlaying()) {
         int currframe = video.getCurrentFrame();

         bool currentFrameIsKeyframe = false;
         if (video.getCurrentFrame() >= 0)
            currentFrameIsKeyframe = json["renderedKeyframes"][video.getCurrentFrame()].asBool();

         float x, y, x2, fullW;
         x = tlo;
         x2 = ofGetWidth() - tlo;
         y = ofGetHeight() - tlo - tlh;
         fullW = ofGetWidth() - (tlo * 2);

         ofSetColor(150, 150, 150);
         for (auto shot : json["shots"]) {
            x = ((float)shot["start"].asFloat() / (float)stats.frames) * fullW + tlo;
            x2 = ((float)shot["end"].asFloat() / (float)stats.frames) * fullW + tlo;
            ofDrawRectangle(x, y + tlh / 5, x2 - x, tlh / 5 * 3);
         }

         ofEnableAlphaBlending();
         ofSetColor(255, 255, 255, 150);
         ofSetLineWidth(1.0f);

         for (auto key : json["keyframes"]) {
            int temp = key.asInt();
            x = ((float)temp / (float)stats.frames) * fullW + tlo;
            if (json["renderedKeyframes"][temp].asBool()) {
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

         string keyframeStr = (currentFrameIsKeyframe) ? "Keyframe: " + json["keyframes"][keyframeIndex].asString() : "N/A";
         char text[1024];
         sprintf(text,
                 "%s\n%d / %d\n%.2f / %.2fs\n%d / \%100\n%dx%d\n%d detected shots\n%d detected keyframes",
                 keyframeStr.c_str(),
                 currframe, stats.frames,
                 stats.currentTime, stats.duration,
                 (int)(stats.position * 100),
                 stats.width, stats.height,
                 json["shots"].size(), json["keyframes"].size());
         ofRectangle rect = infoFont.getStringBoundingBox(text, 0, 0);
         ofSetHexColor(0x222222);
         infoFont.drawString(text, tlo, y - tlo - rect.height - rect.y);

         // if (contourFinder.size() > 0 && video.isPaused() && holes && currentFrameIsKeyframe && keyframesMap[currframe]) {
         if (contourFinder.size() > 0 && showContour && currentFrameIsKeyframe) {
            ofEnableAlphaBlending();
            ofSetColor(255, 0, 0, 150);
            ofTranslate(left, top);
            ofScale(float(wn) / fboWidth, float(wn) / fboWidth, 1);
            contourFinder.draw();
            ofTranslate(0, 0);
            ofScale(0, 0, 0);
            ofDisableAlphaBlending();
            ofSetHexColor(0xffffff);
         }
      }
   }
}

void Vid::drawGui() {
   if (inView) {
      ImGui::SetNextWindowPos(ImVec2(ofGetWidth() - 20 - 350, 20), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(350, 0.0f), ImGuiCond_Always);
      ImGui::Begin("Contour Settings", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
      {
         ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
         ofxPreset::Gui::AddParameter(minArea);
         ofxPreset::Gui::AddParameter(maxArea);
         ofxPreset::Gui::AddParameter(threshold);
         ofxPreset::Gui::AddParameter(showContour);
         //this will change the app background color
      }
      ImGui::End();
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
      hecateDone = vidData.processEvent(e);
      if (!hecateDone)
         ofLogError("FAILED TO SAVE FILE");
   }
}

ofxJSON Vid::data() {
   return json;
}