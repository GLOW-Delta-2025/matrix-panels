#include "config.h"
#include "octo_wrapper.h"
#include "renderer.h"
#include "stars.h"
#include "command_handler.h"
#include "../lib/PingPong.cpp"

unsigned long lastMicros = 0;
// Idle-cycle helpers
static bool wasIdle = false;
static unsigned long idleEnterMs = 0;
static unsigned long lastSeedMs = 0;
static bool buildupSent = false;
static bool climaxSent = false;
static unsigned long buildupStartMs = 0;

// Timing constants for idle animation in case of no PINGs
const unsigned long idleWarmupMs = 30000;       // 30s of normal starfield before buildup
const unsigned long buildupDurationMs = 10000;  // 10s buildup (match command param)
const unsigned long climaxDurationMs = 15000;   // 15s climax (match command param)

void handlePingIdleAnimation();
extern void updateClimaxEffects();

void setup() {
    PingPong.init(30000, &Serial1);
    commandHandlerInit();

    rendererInit();
    starsInit();
    octoBegin();

    lastMicros = micros();
}

void loop() {
    PingPong.update();

    // Handle serial commands
    processSerialCommands();
    
    handlePingIdleAnimation();

    // Main animation loop
    unsigned long now = micros();
    float dt = (now - lastMicros) / 1000000.0f;
    if (dt <= 0.0f) dt = 0.001f;
    lastMicros = now;
    if (dt > 0.1f) dt = 0.1f;

    updateClimaxEffects();

    fadeBuffer();
    updateAndRenderStars(dt);
    copyBufferToOcto();
    octoShow();

    unsigned long frameMicros = micros() - now;
    unsigned long targetMicros = frameTargetMs * 1000UL;
    delay(10);
}

void handlePingIdleAnimation() {
    // Idle flow: normal starfield -> buildup -> climax -> reset (repeat while idle)
    if (PING_IDLE) {
        unsigned long nowMs = millis();

        if (!wasIdle) {
            wasIdle = true;
            idleEnterMs = nowMs;
            lastSeedMs = 0;
            buildupSent = false;
            climaxSent = false;
        }

        // Seed a normal starfield while idle (up to a baseline)
        const int baselineStars = 15;
        if (activeStarCount < baselineStars && (lastSeedMs == 0 || (nowMs - lastSeedMs) >= 200)) {
            int hexColor = ((int)STAR_R << 16) | ((int)STAR_G << 8) | (int)STAR_B;
            // size=2 for visible small trail, leave speed/brightness default
            addStar(-1, hexColor, -1, 2);
            lastSeedMs = nowMs;
        }

        if (!buildupSent && (nowMs - idleEnterMs) >= idleWarmupMs) {
            Serial.println("Triggering buildup");
            cmdlib::Command buildupCmd;
            buildupCmd.addHeader("MASTER");
            buildupCmd.msgKind = "REQUEST";
            buildupCmd.command = "BUILDUP_CLIMAX_CENTER";
            buildupCmd.setNamed("duration", "10.0");
            buildupCmd.setNamed("speedMultiplier", "4.0");
            handleCommand(buildupCmd);

            buildupSent = true;
            buildupStartMs = nowMs;
        }

        // After buildup finishes, trigger climax once
        if (buildupSent && !climaxSent && (nowMs - buildupStartMs) >= buildupDurationMs) {
            Serial.println("Triggering climax");
            cmdlib::Command climaxCmd;
            climaxCmd.addHeader("MASTER");
            climaxCmd.msgKind = "REQUEST";
            climaxCmd.command = "START_CLIMAX_CENTER";
            climaxCmd.setNamed("duration", "15.0");
            climaxCmd.setNamed("speedMultiplier", "4.0");
            climaxCmd.setNamed("spiralSpeed", "0.2");
            handleCommand(climaxCmd);

            climaxSent = true;
        }

        // After climax time + a small buffer, reset the cycle (stay idle)
        if (climaxSent && (nowMs - buildupStartMs) >= (buildupDurationMs + climaxDurationMs + 1000)) {
            // allow a new cycle if still idle
            idleEnterMs = nowMs;
            lastSeedMs = 0;
            buildupSent = false;
            climaxSent = false;
        }
    } else {
        // Not idle -> clear flags; normal starfield continues as usual
        wasIdle = false;
        buildupSent = false;
        climaxSent = false;
        lastSeedMs = 0;
    }
}