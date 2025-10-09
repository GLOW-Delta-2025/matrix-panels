#include "command_handler.h"
#include "../include/commands/star_command_handler.h"
#include "../include/commands/climax_command_handler.h"

// Serial command buffer
static String cmdBuffer = "";

// Handler registry
static BaseCommandHandler* handlers[20];
static int handlerCount = 0;

// Helper to build error response
void buildError(cmdlib::Command &resp, const String &command, const String &message) {
    resp.command = command;
    resp.msgKind = "ERROR";
    resp.setNamed("message", message);
}

void registerHandler(BaseCommandHandler *handler) {
    handlers[handlerCount++] = handler;
    Serial.print("Registered handler: ");
    Serial.println(handler->getName());
}

void commandHandlerInit() {
    Serial.begin(9600);
    while (!Serial && millis() < 3000) {}
    // Wait up to 3 seconds for serial


    static StarCommandHandler starHandler;
    registerHandler(&starHandler);
    static ClimaxCommandHandler climaxHandler;
    registerHandler(&climaxHandler);
}

void processSerialCommands() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        cmdBuffer += c;

        // Check for complete command
        if (cmdBuffer.endsWith("##")) {
            cmdlib::Command cmd;
            String error;

            if (cmdlib::parse(cmdBuffer, cmd, error)) {
                handleCommand(cmd);
            } else {
                cmdlib::Command errResp;
                buildError(errResp, cmd.command, "Parse failed: " + error);
                sendResponse(errResp);
            }

            cmdBuffer = "";
        }
    }
}

void handleCommand(const cmdlib::Command &cmd) {
    // Try each registered handler
    for (int i = 0; i < handlerCount; i++) {
        if (handlers[i]->canHandle(cmd.command)) {
            cmdlib::Command response;
            handlers[i]->handle(cmd, response);
            sendResponse(response);
            return;
        }
    }

    // No handler found
    cmdlib::Command response;
    response.msgKind = "ERROR";
    response.command = cmd.command;
    response.setNamed("message", "No handler for type: " + cmd.command);
    sendResponse(response);
}

void sendResponse(const cmdlib::Command &response) {
    Serial.println(response.toString());
}

// Simple free memory estimation (Teensy)
int freeMemory() {
    char top;
    extern char *__brkval;
    extern char __bss_end;
    return __brkval ? &top - __brkval : &top - &__bss_end;
}