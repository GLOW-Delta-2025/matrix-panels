#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "../lib/CmdLib.h"
#include "commands/base_command_handler.h"

// Initialize command handler
void commandHandlerInit();

// Register a handler
void registerHandler(BaseCommandHandler *handler);

// Process incoming serial data
void processSerialCommands();

// Handle a parsed command using registered handlers
void handleCommand(const cmdlib::Command &cmd);

// Send a response back via serial
void sendResponse(const cmdlib::Command &response);

// Utility: estimate free memory
int freeMemory();

#endif // COMMAND_HANDLER_H