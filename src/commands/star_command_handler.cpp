#include "../../include/commands/star_command_handler.h"
#include "config.h"
#include "stars.h"


// format STAR_[FUNC]
// FUNC = add, remove, reset, info
void StarCommandHandler::handle(const cmdlib::Command &cmd, cmdlib::Command &response) {
    if (cmd.command == "ADD_STAR") {
        handleAdd(cmd, response);
    }
    else {

    }
}

void StarCommandHandler::handleAdd(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Get the count parameter, default to 1
    int count = cmd.getNamed("count", "1").toInt();
    
    // Validate count
    if (count <= 0) {
        buildError(response, cmd.command, "Count must be positive, got: " + String(count));
        return;
    }
    
    // Check if we have room
    int available = MAX_STARS - activeStarCount;
    if (available <= 0) {
        buildError(response, cmd.command,"Already at maximum stars (" + String(MAX_STARS) + ")");
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
    buildResponse(response, cmd.command);
}