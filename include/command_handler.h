#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "../lib/CmdLib.h"

// Initialize command handler
void commandHandlerInit();

// Process incoming serial data
void processSerialCommands();

// Handle a parsed command
void handleCommand(const cmdlib::Command &cmd);

// Send a response back via serial
void sendResponse(const String &type, const String &status, const String &message);

// Utility: estimate free memory
int freeMemory();

#endif // COMMAND_HANDLER_H