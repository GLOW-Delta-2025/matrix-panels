#ifndef BASE_COMMAND_HANDLER_H
#define BASE_COMMAND_HANDLER_H

#include <Arduino.h>
#include "../../lib/CmdLib.h"

// Base class for all command handlers
class BaseCommandHandler {
public:
    virtual ~BaseCommandHandler() {}
    
    // Check if this handler can process the given command type
    virtual bool canHandle(const String &type) const = 0;
    
    // Process the command and return response
    virtual void handle(const cmdlib::Command &cmd, cmdlib::Command &response) = 0;
    
    // Get handler name for debugging
    virtual String getName() const = 0;

protected:
    // Helper to build response
    void buildResponse(cmdlib::Command &resp, const String &type, 
                      const String &status, const String &message) {
        resp.type = type;
        resp.setParam("status", status);
        resp.setParam("message", message);
    }
    
    // Helper to build error response
    void buildError(cmdlib::Command &resp, const String &type,
                   const String &status, const String &message) {
        buildResponse(resp, type, status, message);
    }
};

#endif // BASE_COMMAND_HANDLER_H