#include "commands/climax_command_handler.h"

void ClimaxCommandHandler::handle(const cmdlib::Command &cmd, cmdlib::Command &response) {
    if (cmd.command == "BUILDUP_CLIMAX_CENTER") {
        handleBuildUp(cmd, response);
    } else if (cmd.command == "START_CLIMAX_CENTER") {
        handleStart(cmd, response);

    }
}

void ClimaxCommandHandler::handleBuildUp(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Todo add the buildup
    // All the stars are moving out of the centerpiece into the top piece. (with parameters: climax time)
}

void ClimaxCommandHandler::handleStart(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Todo
    // Build up of climax animations (stars speeds up) (with parameters: Time for buildup)
}