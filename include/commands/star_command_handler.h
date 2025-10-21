#ifndef STAR_COMMAND_HANDLER_H
#define STAR_COMMAND_HANDLER_H

#include "base_command_handler.h"

class StarCommandHandler : public BaseCommandHandler {
public:
    bool canHandle(const String &command) const override {
        return (command == "ADD_STAR");
    }
    
    String getName() const override {
        return "StarHandler";
    }
    
    void handle(const cmdlib::Command &cmd, cmdlib::Command &response) override;

private:
    void handleAdd(const cmdlib::Command &cmd, cmdlib::Command &response);
};

#endif // STAR_COMMAND_HANDLER_H