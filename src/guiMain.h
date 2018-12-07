#include "ofMain.h"
#include "ofParameter.h"
#include "ofParameterGroup.h"
#include "ofxImGui.h"
#include "ofxPreset.h"
#include "ofxXmlSettings.h"
#include "vid.h"

class VidmanGui {
  public:
   ofxImGui::Gui gui;
   // ImVec4 backgroundColor;

   std::vector<std::string>* fileNames;
   vector<std::shared_ptr<Vid>>* videos;
   int* selected;
   string* hecatePath;
   bool* mouseOverGui;
   std::shared_ptr<Vid>* activeVid;

   ofParameter<int> videoWidth;
   ofParameter<int> videoHeight;
   ofParameter<string> hecatePathParam;
   ofParameter<ofFloatColor> backgroundColor;
   ofParameterGroup parameters;
   ofxXmlSettings settings;

   void setup(vector<std::shared_ptr<Vid>>* _videos, std::vector<std::string>* _fileNames, std::shared_ptr<Vid>* _activeVid, string* _hecatePath, int* _selected, bool* _mouseOverGui) {
      videos = _videos;
      fileNames = _fileNames;
      selected = _selected;
      hecatePath = _hecatePath;
      activeVid = _activeVid;
      mouseOverGui = _mouseOverGui;
      gui.setup();
      // backgroundColor = ofColor(114, 144, 154);

      parameters.setName("settings");
      parameters.add(hecatePathParam.set("Hecate Path", "/home/hirad/Code/oF/apps/myApps/vidman/hecate/bin/hecate"));
      (*hecatePath) = hecatePathParam;
      parameters.add(videoWidth.set("Display Width", 900, 100, 2000));
      parameters.add(videoHeight.set("Display Height", 900, 100, 2000));
      parameters.add(backgroundColor.set("Background", ofColor(20, 20, 20), ofColor(0, 0), ofColor(255, 255)));
   }

   void begin() {
      gui.begin();
   }
   void end() {
      (*mouseOverGui) = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
      gui.end();
   }

   bool videoAvailable() {
      return (videos->size() > 0 && *selected != -1);
   }

   void draw() {
      ofSetBackgroundColor(backgroundColor.get());
      ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(350, ofGetHeight() - 110), ImGuiCond_Always);
      ImGui::Begin("APP", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
      {
         ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
         ImGui::Text("\n");

         ImGui::Columns(2, "mycolumns2", false);
         if (ImGui::Button("LOAD PROJECT", ImVec2(ImGui::GetColumnWidth(), 24.0f))) {
            loadProject();
         }
         ImGui::NextColumn();
         if (ImGui::Button("SAVE PROJECT", ImVec2(ImGui::GetColumnWidth(), 24.0f))) {
            saveProject();
         }

         ImGui::Columns(1);
         ImGui::Text("\n");
         drawSettingsInfo();
         drawVideoList();
      }
      ImGui::End();
   }

   void drawSettingsInfo() {
      if (ImGui::CollapsingHeader("Settings & Info")) {
         ImGui::Text("Running at %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
         //this will change the app background color
         char InputBuf[256];
         strcpy(InputBuf, (*hecatePath).c_str());
         if (ImGui::InputText("Hecate Path", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            (*hecatePath) = InputBuf;
            hecatePathParam = (*hecatePath);
         }

         ofxPreset::Gui::AddParameter(videoWidth);
         ofxPreset::Gui::AddParameter(videoHeight);

         if (ImGui::Button("APPLY SIZE", ImVec2(ImGui::GetContentRegionAvailWidth(), 24.0f))) {
            for (int i = 0; i < videos->size(); i++) {
               (*videos)[i]->refreshCoordinates(videoWidth, videoHeight);
            }
         }
         ofxPreset::Gui::AddParameter(backgroundColor);

         // ImGui::ColorEdit3("Background Color", (float*)&backgroundColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
      }
   }

   void drawVideoList() {
      if (!videos->empty()) {
         if (ImGui::CollapsingHeader("Videos")) {
            static ImGuiTextFilter filter;
            ImGui::BeginChild("Child1", ImVec2(ImGui::GetContentRegionAvailWidth(), 200), false, ImGuiWindowFlags_HorizontalScrollbar);
            {
               for (int i = 0; i < fileNames->size(); i++) {
                  ImGui::Columns(1);
                  if (filter.PassFilter((*fileNames)[i].c_str())) {
                     ImGui::Columns(2, "vidcolumns", false);
                     // ImGui::BulletText("%s", lines[i]);
                     if (ImGui::Selectable((*fileNames)[i].c_str(), *selected == i)) {
                        *selected = i;
                        if (*activeVid == nullptr) {
                           *activeVid = (*videos)[*selected];
                        } else {
                           (*activeVid)->closeVideo();
                           *activeVid = (*videos)[*selected];
                        }
                        (*activeVid)->openVideo();
                     }
                     ImGui::NextColumn();
                     ImGui::Text((*videos)[i]->hecateDone ? "PROCESSED" : "NOT PROCESSED");
                  }
                  ImGui::Columns(1);
               }
            }
            ImGui::EndChild();
            ImGui::Text("Search:");
            ImGui::SameLine();
            showHelpMarker(
                "Search usage:\n"
                "  \"\"         display all\n"
                "  \"xxx\"      display videos containing \"xxx\"\n"
                "  \"xxx,yyy\"  display videos containing \"xxx\" or \"yyy\"\n"
                "  \"-xxx\"     hide videos containing \"xxx\"");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            filter.Draw("");
            ImGui::PopItemWidth();

            if (*selected >= 0)
               if (ImGui::Button("PROCESS SELECTED", ImVec2(ImGui::GetContentRegionAvailWidth(), 24.0f))) {
                  (*videos)[*selected]->hecate(*hecatePath);
               }
            if (ImGui::Button("PROCESS REMAINING", ImVec2(ImGui::GetContentRegionAvailWidth(), 24.0f))) {
               processAll(false);
            }
            if (ImGui::Button("(RE)PROCESS ALL", ImVec2(ImGui::GetContentRegionAvailWidth(), 24.0f))) {
               processAll(true);
            }
         }
      }
   }

   void processAll(bool force = false) {
      for (int i = 0; i < videos->size(); i++) {
         if (force)
            (*videos)[i]->hecate(*hecatePath);
         else {
            if (!(*videos)[i]->hecateDone)
               (*videos)[i]->hecate(*hecatePath);
         }
      }
   }

   void showHelpMarker(const char* desc) {
      ImGui::TextDisabled("(?)");
      if (ImGui::IsItemHovered()) {
         ImGui::BeginTooltip();
         ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
         ImGui::TextUnformatted(desc);
         ImGui::PopTextWrapPos();
         ImGui::EndTooltip();
      }
   }

   void saveProject() {
      settings.clear();
      ofSerialize(settings, parameters);
      settings.addTag("videos");
      settings.pushTag("videos");
      for (int i = 0; i < videos->size(); i++) {
         settings.addTag("video");
         settings.pushTag("video", i);
         settings.addValue("filePath", (*videos)[i]->filePath);
         settings.popTag();
      }
      settings.popTag();

      ofFileDialogResult saveFileResult = ofSystemSaveDialog(ofGetTimestampString() + ".xml", "Save project");
      if (saveFileResult.bSuccess) {
         settings.saveFile(saveFileResult.filePath);
      }
   }

   void loadProject() {
      ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a jpg or png");
      if (openFileResult.bSuccess) {
         ofLogVerbose("User selected a file");

         settings.loadFile(openFileResult.getPath());
         ofDeserialize(settings, parameters);

         *selected = -1;
         videos->clear();
         fileNames->clear();

         settings.pushTag("videos");
         int numberOfSavedVideos = settings.getNumTags("video");
         for (int i = 0; i < numberOfSavedVideos; i++) {
            settings.pushTag("video", i);

            std::shared_ptr<Vid> myvid(new Vid);
            myvid->setup(settings.getValue("filePath", ""), hecatePath, videoWidth.get(), videoHeight.get());
            videos->push_back(myvid);
            string filename = myvid->json["info"]["baseName"].asString();
            string extention = myvid->json["info"]["extension"].asString();
            fileNames->push_back(filename + "." + extention);

            settings.popTag();
         }

         settings.popTag();  //pop position

      } else {
         ofLogVerbose("User hit cancel");
      }
   }
};