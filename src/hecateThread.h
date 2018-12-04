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
         cout << logPrefix << "Name:   " << file.getFileName() << endl;
      }

      else
         cout << logPrefix << title << " [ " << file.getFileName() << " ]" << endl;
   }

   void setup(string hecatePath, string filePath) {
      if (processing == false) {
         path = filePath;
         cmd = hecatePath + " -i " + path +
             " --print_shot_info  --print_keyfrm_info";

         ffcmd = "ffprobe -v quiet -print_format json -show_format -show_streams " + filePath;
         file = ofFile(path);
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
      processed = false;
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
            // string ffstr = ffprobe(ffcmd.c_str());
            string ffstr = "";
            string hecstr = exec(cmd.c_str());
            processed = true;

            condition.wait(lock);
         }
      }
   }

   // string ffprobe(const char* command) {
   //    std::array<char, 256> buffer;
   //    std::string result;
   //    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
   //    if (!pipe)
   //       throw std::runtime_error("popen() failed!");
   //    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
   //       result += buffer.data();

   //    if (result != "") {
   //       hecateLog("FFPROBE Analysis Complete", false);
   //       return result;
   //    } else
   //       throw std::runtime_error("FFPROBE failed to execute");
   // }

   string exec(const char* command) {
      std::array<char, 128> buffer;
      std::string result;
      std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
      if (!pipe)
         throw std::runtime_error("popen() failed!");
      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
         result += buffer.data();

      processing = false;

      if (result != "") {
         emitEvent(result);
         hecateLog("Hecate Analysis Complete", false);
         return result;
      } else
         throw std::runtime_error("HECATE failed to execute");
   }

   // void emitEvent(string ffstr, string hecstr) {
   void emitEvent(string hecstr) {
      static HecateEvent newEvent;
      // newEvent.ffraw = ffstr;
      newEvent.hecraw = hecstr;
      newEvent.ffcmd = ffcmd;
      newEvent.heccmd = cmd;
      newEvent.path = path;

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

   void draw() {}

   bool isProcessed() { return processed; }

  protected:
   string cmd;
   string ffcmd;
   string path;
   bool processing = false;
   bool processed = false;
   ofFile file;
   ofFile hecateBin;
   string logPrefix = "[VIDMAN ANALYSIS] ";

   std::condition_variable condition;
   int threadFrameNum = 0;
};
