#pragma once

#include <atomic>
#include "hecateEvent.h"
#include "ofMain.h"

class HecateThread : public ofThread {
 public:
  ~HecateThread() {
    stop();
    waitForThread(false);
  }

  void hecateLog(string title, bool info) {
    if (info) {
      cout << logPrefix << title << endl;
      cout << logPrefix << "Hecate: " << hecateBin.path() << endl;
      cout << logPrefix << "File:   " << file.path() << endl;
      cout << logPrefix << "Name:   " << file.getFileName() << endl
           << endl;
    }

    else {
      cout << logPrefix << title << " [ " << file.getFileName() << " ]" << endl
           << endl;
    }
  }

  void setup(string hecatePath, string filePath) {
    if (processing == false) {
      cmd = hecatePath + " -i " + filePath +
          " --print_shot_info  --print_keyfrm_info";

      ffcmd     = "ffprobe -v quiet -print_format json -show_format -show_streams" + filePath;
      file      = ofFile(filePath);
      hecateBin = ofFile(hecatePath);

      if (hecateBin.doesFileExist(hecatePath)) {
        hecateLog("INITIATED", true);
        start();
      }
    }
  }

  void start() { startThread(); }

  void stop() {
    if (processed == false && processing == true) {
      system("killall hecate");
      hecateLog("PROCESS KILLED", true);
    }

    processing = false;
    processed  = false;
    stopThread();
    hecateLog("THREAD STOPPED", false);
    std::unique_lock<std::mutex> lck(mutex);
    condition.notify_all();
  }

  void threadedFunction() {
    while (isThreadRunning()) {
      if (!processing) {
        std::unique_lock<std::mutex> lock(mutex);

        processing = true;
        exec(cmd.c_str());
        ffprobe(ffcmd.c_str());
        processed = true;

        condition.wait(lock);
      }
    }
  }

  void ffprobe(const char* command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe)
      throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
      result += buffer.data();

    processing = false;

    if (result != "") {
      emitFFprobeHecateEvent(result);
      hecateLog("FFPROBE Analysis Complete", false);
    }

    // 24000/1001
  }

  void exec(const char* command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe)
      throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
      result += buffer.data();

    processing = false;

    if (result != "") {
      emitHecateEvent(result);
      hecateLog("Hecate Analysis Complete", false);
    }
  }

  void emitHecateEvent(string result) {
    static HecateEvent newEvent;
    newEvent.raw  = result;
    newEvent.cmd  = cmd;
    newEvent.mode = "hecate";

    ofNotifyEvent(HecateEvent::events, newEvent);
  }

  void emitFFprobeHecateEvent(string result) {
    static HecateEvent newEvent;
    newEvent.ffraw = result;
    newEvent.cmd   = ffcmd;
    newEvent.mode  = "ffprobe";

    ofNotifyEvent(HecateEvent::events, newEvent);
  }

  void update() {
    std::unique_lock<std::mutex> lock(mutex);
    // CODE
    condition.notify_all();
  }

  void updateNoLock() {
    // CODE
    condition.notify_all();
  }

  void draw() {
    /// This drawing function cannot be called from the thread itself because
    /// it includes OpenGL calls
  }

  bool isProcessed() { return processed; }

 protected:
  string cmd;
  string ffcmd;
  bool processing = false;
  bool processed  = false;
  ofFile file;
  ofFile hecateBin;
  string logPrefix = "[HECATE] ";

  std::condition_variable condition;
  int threadFrameNum = 0;
};
