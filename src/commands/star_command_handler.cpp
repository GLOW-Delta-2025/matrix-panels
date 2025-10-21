#include "../../include/commands/star_command_handler.h"
#include "config.h"
#include "stars.h"

void StarCommandHandler::handle(const cmdlib::Command &cmd, cmdlib::Command &response) {
    if (cmd.command == "ADD_STAR") {
        handleAdd(cmd, response);
    }
    else {

    }
}

void StarCommandHandler::handleAdd(const cmdlib::Command &cmd, cmdlib::Command &response) {
    int count = cmd.getNamed("count", "1").toInt();
    int speed = cmd.getNamed("speed", "50").toInt();      // Default speed: 50
    String colorStr = cmd.getNamed("color", "0xffc003");
    int brightness = cmd.getNamed("brightness", "255").toInt(); // Default brightness: 255
    int size = cmd.getNamed("size", "1").toInt();         // Default size: 1

    if (count <= 0) {
        buildError(response, cmd.command, "Count must be positive, got: " + String(count));
        return;
    }

    // Validate other parameters as needed
    if (speed < 0 || speed > 100) {
        buildError(response, cmd.command, "Speed must be between 0 and 100, got: " + String(speed));
        return;
    }

    if (brightness < 0 || brightness > 255) {
        buildError(response, cmd.command, "Brightness must be between 0 and 255, got: " + String(brightness));
        return;
    }

    if (size <= 0) {
        buildError(response, cmd.command, "Size must be positive, got: " + String(size));
        return;
    }

    int available = MAX_STARS - activeStarCount;
    if (available <= 0) {
        buildError(response, cmd.command,"Already at maximum stars (" + String(MAX_STARS) + ")");
        return;
    }

    if (count > available) {
        count = available;
    }

    int added = 0;
    for (int i = 0; i < count && activeStarCount < MAX_STARS; i++) {
        int hexColor;
        if (colorStr.startsWith("0x")) {
            hexColor = (int)strtol(colorStr.c_str() + 2, NULL, 16);
        } else {
            hexColor = colorStr.toInt();
        }

        // Assuming addStar function needs to be modified to accept these parameters
        if (addStar(speed, hexColor, brightness, size)) {
            added++;
        }
    }
    buildResponse(response, cmd.command);
}