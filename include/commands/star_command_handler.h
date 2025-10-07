#ifndef STAR_COMMAND_HANDLER_H
#define STAR_COMMAND_HANDLER_H

#include "base_command_handler.h"

class StarCommandHandler : public BaseCommandHandler {
public:
    bool canHandle(const String &type) const override {
        return (type.startsWith("star") || type.startsWith("stars"));
    }
    
    String getName() const override {
        return "StarHandler";
    }
    
    void handle(const cmdlib::Command &cmd, cmdlib::Command &response) override;

private:
    void handleAdd(const cmdlib::Command &cmd, cmdlib::Command &response);
    void handleRemove(const cmdlib::Command &cmd, cmdlib::Command &response);
    void handleReset(const cmdlib::Command &cmd, cmdlib::Command &response);
    void handleInfo(const cmdlib::Command &cmd, cmdlib::Command &response);
};

#endif // STAR_COMMAND_HANDLER_H