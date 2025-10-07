#ifndef CONFIG_COMMAND_HANDLER_H
#define CONFIG_COMMAND_HANDLER_H

#include "base_command_handler.h"

class ConfigCommandHandler : public BaseCommandHandler {
public:
    bool canHandle(const String &type) const override {
        return type == "config";
    }
    
    String getName() const override {
        return "ConfigHandler";
    }
    
    void handle(const cmdlib::Command &cmd, cmdlib::Command &response) override;
};

#endif // CONFIG_COMMAND_HANDLER_H