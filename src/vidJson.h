#include "hecateEvent.h"
#include "ofMain.h"
#include "ofxJSON.h"

#ifndef _VIDDIRINFO
#define _VIDDIRINFO
struct VidDirectoryInfo {
   string baseName;
   string extension;
   string enclosingDirectoryPath;
   string vidmanDirectoryPath;
   string rendersDirectoryPath;
   ofDirectory vidmanDirectory;
   ofDirectory rendersDirectory;
   ofFile analysisSavedOutput;
   string analysisSavedOutputPath;
};
#endif

#ifndef _VIDDATA
#define _VIDDATA
class VidJson {
  private:
   ofxJSON* json;
   string filePath;
   VidDirectoryInfo vd;

  public:
   VidJson& operator=(const VidJson& rhs) {
      if (this != &rhs) {
         json = rhs.json;
         filePath = rhs.filePath;
         vd = rhs.vd;
      }
      return *this;
   }

   ~VidJson() {}

   void setup(ofxJSON* _json, string _filePath) {
      json = _json;
      filePath = _filePath;
   }

   bool detectExistingData() {
      bool analysisExists = prepareDataFolder();
      if (analysisExists) {
         json->clear();
         json->open(vd.analysisSavedOutputPath);
      } else {
         populateInfo();
      }

      processRendersFolder(analysisExists);
      return analysisExists;
   }

   bool prepareDataFolder() {
      vd.baseName = ofFilePath::getBaseName(filePath);
      vd.extension = ofFilePath::getFileExt(filePath);

      vd.enclosingDirectoryPath = ofFilePath::getEnclosingDirectory(filePath, false);

      vd.vidmanDirectoryPath = ofFilePath::join(vd.enclosingDirectoryPath, vd.baseName + "_vidman");
      vd.vidmanDirectoryPath = ofFilePath::getPathForDirectory(vd.vidmanDirectoryPath);

      vd.rendersDirectoryPath = ofFilePath::join(vd.vidmanDirectoryPath, "renders");
      vd.rendersDirectoryPath = ofFilePath::getPathForDirectory(vd.rendersDirectoryPath);

      vd.vidmanDirectory = ofDirectory(vd.vidmanDirectoryPath);
      vd.rendersDirectory = ofDirectory(vd.rendersDirectoryPath);

      if (!vd.vidmanDirectory.isDirectory()) {
         ofDirectory::createDirectory(vd.vidmanDirectoryPath, false);
         vd.vidmanDirectory.createDirectory(vd.rendersDirectoryPath, false);
      }

      if (!vd.rendersDirectory.isDirectory()) {
         ofDirectory::createDirectory(vd.rendersDirectoryPath, false);
      }

      vd.analysisSavedOutputPath = vd.vidmanDirectoryPath + "analysis.json";
      // hecateSavedOutputPath = vidmanDirectoryPath + ".hecateOutput.txt";
      vd.analysisSavedOutput = ofFile(vd.analysisSavedOutputPath, ofFile::ReadWrite, false);

      if (vd.analysisSavedOutput.exists()) {
         return true;
      } else {
         return false;
      }
   }

   void processRendersFolder(bool analysisExists) {
      vector<ofFile> renders = vd.rendersDirectory.getFiles();

      if (analysisExists && renders.size() > 0) {
         for (auto file : renders) {
            string renderName = ofFilePath::getBaseName(file.getAbsolutePath());

            if (renderName.find('_') != std::string::npos) {
               vector<string> split = ofSplitString(renderName, "_");
               if (split.size() > 0) {
                  string frameNumber = split[split.size() - 1];
                  frameNumber.erase(0, min(frameNumber.find_first_not_of('0'), frameNumber.size() - 1));
                  int fnum = stoi(frameNumber);
                  bool alreadyRendered = false;

                  for (auto shot : (*json)["shots"])
                     if (findInArray(shot["keyframes"], fnum)) {
                        alreadyRendered = true;
                        break;
                     }

                  if (alreadyRendered && findInArray((*json)["renderableKeyframes"], fnum)) {
                     (*json)["renderedKeyframes"][fnum] = 1;  // add when redering too00000000000000000
                  } else
                     ofFile::removeFile(file.getAbsolutePath(), false);
               }
            } else {
               ofFile::removeFile(file.getAbsolutePath(), false);
            }
         }
      }

      if (!analysisExists && renders.size() > 0)
         for (auto file : renders)
            ofFile::removeFile(file.getAbsolutePath(), false);
   }

   void populateInfo() {
      // json->clear();
      (*json)["info"]["baseName"] = vd.baseName;
      (*json)["info"]["extension"] = vd.extension;
      (*json)["info"]["enclosingDir"] = vd.enclosingDirectoryPath;
      (*json)["info"]["vidmanDir"] = vd.vidmanDirectoryPath;
      (*json)["info"]["rendersDir"] = vd.rendersDirectoryPath;
      (*json)["info"]["analysisPath"] = vd.analysisSavedOutputPath;
   }

   void populateFFProbe(string ffraw) {
      ofxJSON ffprobe;
      ffprobe.parse(ffraw);
      int t1 = ffprobe["streams"][0]["width"].asInt();
      int t2 = ffprobe["streams"][0]["height"].asInt();
      string sampleRatio = ffprobe["streams"][0]["sample_aspect_ratio"].asString();
      string displayRatio = ffprobe["streams"][0]["display_aspect_ratio"].asString();
      string fps = ffprobe["streams"][0]["r_frame_rate"].asString();
      string duration = ffprobe["streams"][0]["duration"].asString();
      string frames = ffprobe["streams"][0]["nb_frames"].asString();

      vector<string> t;

      (*json)["width"] = t1;
      (*json)["height"] = t2;

      t = ofSplitString(sampleRatio, ":");
      t1 = stof(t[0]);
      t2 = stof(t[1]);
      (*json)["sampleRatio"] = t1 / t2;

      t = ofSplitString(displayRatio, ":");
      t1 = stof(t[0]);
      t2 = stof(t[1]);
      (*json)["ratio"] = t1 / t2;

      t = ofSplitString(fps, "/");
      t1 = stof(t[0]);
      t2 = stof(t[1]);
      (*json)["fps"] = t1 / t2;

      (*json)["duration"] = stof(duration);

      if (frames == "") {
         (*json)["frames"] = ffprobe["packets"].size();
      } else
         (*json)["frames"] = stoi(frames);
   }

   bool processEvent(HecateEvent& e) {
      populateInfo();

      // populateFFProbe(e.ffraw);

      processHecateResults(e.hecraw);
      bool analysisSuccessful = prepareHecateOutput();
      processRendersFolder(analysisSuccessful);
      return analysisSuccessful;
   }

   bool prepareHecateOutput() {
      bool analysisSavedOutputExists = prepareDataFolder();
      string analysisSavedOutputPath = vd.analysisSavedOutput.path();

      if (analysisSavedOutputExists)
         vd.analysisSavedOutput.removeFile(analysisSavedOutputPath, false);

      //   hecateSavedOutput = ofFile(hecateSavedOutputPath, ofFile::ReadWrite, false);
      //   hecateSavedOutput.create();
      //   hecateSavedOutput.open(hecateSavedOutputPath, ofFile::ReadWrite, false);

      bool saveSuccessful = json->save(analysisSavedOutputPath, true);
      return saveSuccessful;
   }

   vector<string> cleanHecateResults(string result) {
      vector<string> clean = ofSplitString(result, "shots:");
      clean = ofSplitString(clean[1], "keyframes:");

      // Clean shots' brackets
      ofStringReplace(clean[0], "[", "");
      ofStringReplace(clean[0], "]", "");
      // Clean keyframes' brackets
      ofStringReplace(clean[1], "[", "");
      ofStringReplace(clean[1], "]", "");

      return clean;
   }

   void prepareHecate() {
      Json::Value jshots(Json::arrayValue);
      (*json)["shots"] = jshots;

      Json::Value jkeyframes(Json::arrayValue);
      (*json)["keyframesShot"] = jkeyframes;

      Json::Value jallkeyframes(Json::arrayValue);
      (*json)["keyframes"] = jallkeyframes;

      Json::Value jvalidkeyframes(Json::arrayValue);
      (*json)["renderableKeyframes"] = jvalidkeyframes;

      ofLogNotice("JSON prepared.");
   }

   void populateShots(string shotsStr) {
      vector<string> temp;
      vector<string> temp2;
      temp = ofSplitString(shotsStr, ",");
      for (auto s : temp) {
         temp2 = ofSplitString(s, ":");

         ofxJSON shot;
         int start = stoi(temp2[0]);
         int end = stoi(temp2[1]);
         shot["start"] = start;
         shot["end"] = end;
         shot["frames"] = end - start;
         shot["length"] = shot["frames"].asFloat() / (*json)["fps"].asFloat();
         Json::Value kfs(Json::arrayValue);
         shot["keyframes"] = kfs;
         shot["bestFrame"] = "NONE";
         (*json)["shots"].append(shot);
         //   shots.push_back(make_tuple(stoi(temp2[0]), stoi(temp2[1])));
      }

      for (auto s : (*json)["shots"]) {
         cout << s["start"].asString() + " " +
                 s["end"].asString() + " " + s["length"].asString()
              << endl;
      }

      ofLogNotice("Shots populated.");
   }

   void populateKeyframes(string keyframesStr) {
      stringstream stream(keyframesStr);

      int key = 0;
      while (stream >> key) {
         bool keyframeInShot = false;
         (*json)["keyframes"].append(key);

         int i = 0;
         for (auto shot : (*json)["shots"]) {
            float start = shot["start"].asFloat();
            float end = shot["end"].asFloat();
            float ideal = (end - start) / 2;

            if (key >= start && key <= end) {
               keyframeInShot = true;
               //   keyframesToShotsMap[key] = i;
               (*json)["shots"][i]["keyframes"].append(key);

               if (shot["bestFrame"] == "NONE") {
                  (*json)["shots"][i]["bestFrame"] = key;
               } else {
                  float currDist = ideal - (key - start);
                  currDist = abs(currDist);
                  int lastBest = shot["bestFrame"].asInt();
                  float lasDist = ideal - (lastBest - start);
                  lasDist = abs(lasDist);

                  if (currDist < lasDist)
                     (*json)["shots"][i]["bestFrame"] = key;
               }
               break;
            }
            i++;
         }

         if (keyframeInShot) {
            ofxJSON jkey;
            jkey["frame"] = key;
            jkey["shot"] = i;
            (*json)["keyframesShot"].append(jkey);
         }

         if (stream.peek() == ',' || stream.peek() == ' ')
            stream.ignore();
      }
      ofLogNotice("Keyframes populated.");
   }

   void populateRenderableKeyframes() {
      for (auto shot : (*json)["shots"]) {
         int bestFrame = shot["bestFrame"].asInt();
         (*json)["renderableKeyframes"].append(bestFrame);
      }

      for (auto key : (*json)["keyframes"]) {
         (*json)["renderedKeyframes"][key.asInt()] = false;
      }

      ofLogNotice("Renderable keyframes populated.");
   }

   bool processHecateResults(string result) {
      vector<string> clean = cleanHecateResults(result);

      prepareHecate();
      populateShots(clean[0]);
      populateKeyframes(clean[1]);
      string analysisSavedOutputPath = vd.analysisSavedOutput.path();
      bool saveSuccessful = json->save(analysisSavedOutputPath, true);

      populateRenderableKeyframes();

      return true;
   }

   bool findInArray(const Json::Value& _json, int value) {
      assert(_json.isArray());
      for (int i = 0; i != _json.size(); i++)
         if (_json[i].asString() == to_string(value))
            return true;

      return false;
   }

   bool findInArray(const Json::Value& _json, float value) {
      assert(_json.isArray());
      for (int i = 0; i != _json.size(); i++)
         if (_json[i].asString() == to_string(value))
            return true;

      return false;
   }

   bool findInArray(const Json::Value& _json, string value) {
      assert(_json.isArray());
      for (int i = 0; i != _json.size(); i++)
         if (_json[i].asString() == value)
            return true;

      return false;
   }
};
#endif