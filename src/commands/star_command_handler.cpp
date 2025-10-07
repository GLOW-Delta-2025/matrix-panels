#include "../../include/commands/star_command_handler.h"
#include "config.h"
#include "stars.h"


// format STAR_[FUNC]
// FUNC = add, remove, reset, info
void StarCommandHandler::handle(const cmdlib::Command &cmd, cmdlib::Command &response) {
    String func = cmd.command.substring(5);
    if (func == "add") {
        handleAdd(cmd, response);
    }
    else if (func == "remove") {
        handleRemove(cmd, response);
    }
    else if (func == "reset") {
        handleReset(cmd, response);
    }
    else if (func == "info") {
        handleInfo(cmd, response);
    }
    else {
        buildError(response, "star", "unknown_command", 
                  "Unknown command: " + cmd.command + ". Available: add, remove, reset, info");
    }
}

void StarCommandHandler::handleAdd(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Get the count parameter, default to 1
    int count = cmd.getParam("count", "1").toInt();
    
    // Validate count
    if (count <= 0) {
        buildError(response, "star", "invalid_param", 
                  "Count must be positive, got: " + String(count));
        return;
    }
    
    // Check if we have room
    int available = MAX_STARS - activeStarCount;
    if (available <= 0) {
        buildError(response, "star", "max_reached", 
                  "Already at maximum stars (" + String(MAX_STARS) + ")");
        return;
    }
    
    // Limit count to available space
    if (count > available) {
        count = available;
    }
    
    // Add stars
    int added = 0;
    for (int i = 0; i < count && activeStarCount < MAX_STARS; i++) {
        if (addStar()) {
            added++;
        }
    }
    
    String msg = "Added " + String(added) + " star(s). ";
    msg += "Active: " + String(activeStarCount) + "/" + String(MAX_STARS);
    
    buildResponse(response, "star", "ok", msg);
}

void StarCommandHandler::handleRemove(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Get the count parameter, default to 1
    int count = cmd.getParam("count", "1").toInt();
    
    // Validate count
    if (count <= 0) {
        buildError(response, "star", "invalid_param", 
                  "Count must be positive, got: " + String(count));
        return;
    }
    
    // Check if we have any stars to remove
    if (activeStarCount <= 0) {
        buildError(response, "star", "none_active", 
                  "No active stars to remove");
        return;
    }
    
    // Limit count to active stars
    if (count > activeStarCount) {
        count = activeStarCount;
    }
    
    // Remove stars by decreasing the active count
    int removed = 0;
    for (int i = 0; i < count && activeStarCount > 0; i++) {
        activeStarCount--;
        removed++;
    }
    
    String msg = "Removed " + String(removed) + " star(s). ";
    msg += "Active: " + String(activeStarCount) + "/" + String(MAX_STARS);
    
    buildResponse(response, "star", "ok", msg);
}

void StarCommandHandler::handleReset(const cmdlib::Command &cmd, cmdlib::Command &response) {
    if (activeStarCount <= 0) {
        buildResponse(response, "star", "ok", "No active stars to reset");
        return;
    }
    
    // Reset all active stars
    for (int i = 0; i < activeStarCount; i++) {
        resetStar(starsArr[i], true);
    }
    
    String msg = "Reset " + String(activeStarCount) + " star(s) to new positions";
    buildResponse(response, "star", "ok", msg);
}

void StarCommandHandler::handleInfo(const cmdlib::Command &cmd, cmdlib::Command &response) {
    String info = "Active: " + String(activeStarCount) + "/" + String(MAX_STARS);
    info += " Speed: " + String(minSpeedColsPerSec) + "-" + String(maxSpeedColsPerSec) + " cols/s";
    info += " RandomRows: " + String(randomRows ? "true" : "false");
    info += " Wrap: " + String(wrapStars ? "true" : "false");
    
    buildResponse(response, "star", "ok", info);
}