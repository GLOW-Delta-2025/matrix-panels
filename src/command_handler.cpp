#include "command_handler.h"
#include "config.h"
#include "stars.h"

// Serial command buffer
static String cmdBuffer = "";

void commandHandlerInit() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait up to 3 seconds for serial
  
  Serial.println("OctoStars Command Interface Ready");
  Serial.println("Format: !!type:command:{key=value,key=value}##");
  Serial.println("Try: !!config:get:{}## or !!sys:info:{}##");
}

void processSerialCommands() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    cmdBuffer += c;
    
    // Check for complete command
    if (cmdBuffer.endsWith("##")) {
      cmdlib::Command cmd;
      String error;
      
      if (cmdlib::parse(cmdBuffer, cmd, error)) {
        handleCommand(cmd);
      } else {
        sendResponse("error", "parse_failed", error);
      }
      
      cmdBuffer = "";
    }
    
    // Prevent buffer overflow
    if (cmdBuffer.length() > 256) {
      sendResponse("error", "buffer_overflow", "Command too long");
      cmdBuffer = "";
    }
  }
}

void handleCommand(const cmdlib::Command &cmd) {
  // Config commands
  if (cmd.type == "config" || cmd.type == "cfg") {
    
    if (cmd.command == "set") {
      // Set configuration parameters
      bool changed = false;
      String changes = "";
      
      // Star count
      if (cmd.getParam("stars") != "") {
        int val = cmd.getParam("stars").toInt();
        if (val >= 0 && val <= MAX_STARS) {
          activeStarCount = val;
          changes += "stars=" + String(activeStarCount) + " ";
          changed = true;
        }
      }
      
      // Speed range
      if (cmd.getParam("minSpeed") != "") {
        float val = cmd.getParam("minSpeed").toFloat();
        if (val > 0) {
          minSpeedColsPerSec = val;
          changes += "minSpeed=" + String(minSpeedColsPerSec) + " ";
          changed = true;
        }
      }
      
      if (cmd.getParam("maxSpeed") != "") {
        float val = cmd.getParam("maxSpeed").toFloat();
        if (val > 0) {
          maxSpeedColsPerSec = val;
          changes += "maxSpeed=" + String(maxSpeedColsPerSec) + " ";
          changed = true;
        }
      }
      
      // Fade factor
      if (cmd.getParam("fade") != "") {
        float val = cmd.getParam("fade").toFloat();
        if (val >= 0.0f && val <= 1.0f) {
          fadeFactor = val;
          changes += "fade=" + String(fadeFactor) + " ";
          changed = true;
        }
      }
      
      // Frame time
      if (cmd.getParam("frameMs") != "") {
        unsigned long val = cmd.getParam("frameMs").toInt();
        if (val > 0 && val < 1000) {
          frameTargetMs = val;
          changes += "frameMs=" + String(frameTargetMs) + " ";
          changed = true;
        }
      }
      
      // Random rows
      if (cmd.getParam("randomRows") != "") {
        String val = cmd.getParam("randomRows");
        val.toLowerCase();
        if (val == "true" || val == "1") {
          randomRows = true;
          changes += "randomRows=true ";
          changed = true;
        } else if (val == "false" || val == "0") {
          randomRows = false;
          changes += "randomRows=false ";
          changed = true;
        }
      }
      
      // Wrap stars
      if (cmd.getParam("wrap") != "") {
        String val = cmd.getParam("wrap");
        val.toLowerCase();
        if (val == "true" || val == "1") {
          wrapStars = true;
          changes += "wrap=true ";
          changed = true;
        } else if (val == "false" || val == "0") {
          wrapStars = false;
          changes += "wrap=false ";
          changed = true;
        }
      }
      
      // Star color
      if (cmd.getParam("r") != "") {
        int val = cmd.getParam("r").toInt();
        if (val >= 0 && val <= 255) {
          STAR_R = val;
          changes += "r=" + String(STAR_R) + " ";
          changed = true;
        }
      }
      
      if (cmd.getParam("g") != "") {
        int val = cmd.getParam("g").toInt();
        if (val >= 0 && val <= 255) {
          STAR_G = val;
          changes += "g=" + String(STAR_G) + " ";
          changed = true;
        }
      }
      
      if (cmd.getParam("b") != "") {
        int val = cmd.getParam("b").toInt();
        if (val >= 0 && val <= 255) {
          STAR_B = val;
          changes += "b=" + String(STAR_B) + " ";
          changed = true;
        }
      }
      
      if (changed) {
        sendResponse("config", "ok", changes);
      } else {
        sendResponse("config", "no_change", "No valid parameters");
      }
    }
    
    else if (cmd.command == "get") {
      // Get current configuration
      String info = "stars=" + String(activeStarCount) + 
                    " minSpeed=" + String(minSpeedColsPerSec) + 
                    " maxSpeed=" + String(maxSpeedColsPerSec) +
                    " fade=" + String(fadeFactor) +
                    " frameMs=" + String(frameTargetMs) +
                    " randomRows=" + String(randomRows ? "true" : "false") +
                    " wrap=" + String(wrapStars ? "true" : "false") +
                    " rgb=" + String(STAR_R) + "," + String(STAR_G) + "," + String(STAR_B);
      sendResponse("config", "ok", info);
    }
    
    else {
      sendResponse("config", "unknown_command", cmd.command);
    }
  }
  
  // Star commands
  else if (cmd.type == "star" || cmd.type == "stars") {
    
    if (cmd.command == "add") {
      int count = cmd.getParam("count", "1").toInt();
      int added = 0;
      
      for (int i = 0; i < count && activeStarCount < MAX_STARS; i++) {
        if (addStar()) added++;
      }
      
      sendResponse("star", "ok", "Added " + String(added) + " stars, total=" + String(activeStarCount));
    }
    
    else if (cmd.command == "remove") {
      int count = cmd.getParam("count", "1").toInt();
      int removed = 0;
      
      for (int i = 0; i < count && activeStarCount > 0; i++) {
        activeStarCount--;
        removed++;
      }
      
      sendResponse("star", "ok", "Removed " + String(removed) + " stars, total=" + String(activeStarCount));
    }
    
    else if (cmd.command == "reset") {
      for (int i = 0; i < activeStarCount; i++) {
        resetStar(starsArr[i], true);
      }
      sendResponse("star", "ok", "Reset all stars");
    }
    
    else {
      sendResponse("star", "unknown_command", cmd.command);
    }
  }
  
  // System commands
  else if (cmd.type == "sys" || cmd.type == "system") {
    
    if (cmd.command == "info") {
      String info = "CURTAINS=" + String(CURTAINS) + 
                    " WIDTH=" + String(CURTAIN_WIDTH) +
                    " HEIGHT=" + String(CURTAIN_HEIGHT) +
                    " TOTAL=" + String(NUM_PIXELS) +
                    " MAX_STARS=" + String(MAX_STARS) +
                    " RAM=" + String(freeMemory());
      sendResponse("system", "ok", info);
    }
    
    else if (cmd.command == "ping") {
      sendResponse("system", "pong", "OK");
    }
    
    else {
      sendResponse("system", "unknown_command", cmd.command);
    }
  }
  
  else {
    sendResponse("error", "unknown_type", cmd.type);
  }
}

void sendResponse(const String &type, const String &status, const String &message) {
  cmdlib::Command resp;
  resp.type = type;
  resp.command = "response";
  resp.setParam("status", status);
  resp.setParam("message", message);
  Serial.println(resp.toString());
}

// Simple free memory estimation (Teensy)
int freeMemory() {
  char top;
  extern char *__brkval;
  extern char __bss_end;
  return __brkval ? &top - __brkval : &top - &__bss_end;
}