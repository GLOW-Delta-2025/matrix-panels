#include "../../include/commands/config_command_handler.h"
#include "config.h"

void ConfigCommandHandler::handle(const cmdlib::Command &cmd, cmdlib::Command &response) {
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
        buildResponse(response, "CONFIRM", "ok", changes);
    } else {
        buildResponse(response, "CONFIRM", "no_change", "No valid parameters");
    }
}

