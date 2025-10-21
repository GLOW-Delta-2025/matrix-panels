#ifndef CLIMAX_COMMAND_HANDLER_H
#define CLIMAX_COMMAND_HANDLER_H

#include "base_command_handler.h"

class ClimaxCommandHandler : public BaseCommandHandler {
public:
    bool canHandle(const String &command) const override {
        return command == "BUILDUP_CLIMAX_CENTER" || command == "START_CLIMAX_CENTER";
    }
    
    String getName() const override {
        return "ClimaxHandler";
    }
    
    void handle(const cmdlib::Command &cmd, cmdlib::Command &response) override;

private:
    void handleBuildUp(const cmdlib::Command &cmd, cmdlib::Command &response);
    void handleStart(const cmdlib::Command &cmd, cmdlib::Command &response);
};

#endif // CLIMAX_COMMAND_HANDLER_H