#include "commands/climax_command_handler.h"
#include "config.h"
#include "stars.h"

// Global state for climax animations (add these to a header if needed)
static bool climaxActive = false;
static unsigned long climaxStartTime = 0;
static float climaxDuration = 0;
static float originalMinSpeed = 0;
static float originalMaxSpeed = 0;

void ClimaxCommandHandler::handle(const cmdlib::Command &cmd, cmdlib::Command &response) {
    if (cmd.command == "BUILDUP_CLIMAX_CENTER") {
        handleBuildUp(cmd, response);
    } else if (cmd.command == "START_CLIMAX_CENTER") {
        handleStart(cmd, response);
    }
}

void ClimaxCommandHandler::handleBuildUp(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Parse climax time parameter (in seconds)
    float climaxTime = cmd.getNamed("time", "5.0").toFloat();

    if (climaxTime <= 0 || climaxTime > 60) {
        buildError(response, cmd.command, "Invalid time parameter. Must be between 0 and 60 seconds.");
        return;
    }

    // Parse optional speed multiplier (how much faster during climax)
    float speedMultiplier = cmd.getNamed("speedMultiplier", "3.0").toFloat();
    if (speedMultiplier < 1.0 || speedMultiplier > 10.0) {
        speedMultiplier = 3.0;
    }

    // Store original speeds and activate climax mode
    originalMinSpeed = minSpeedColsPerSec;
    originalMaxSpeed = maxSpeedColsPerSec;
    climaxActive = true;
    climaxStartTime = millis();
    climaxDuration = climaxTime * 1000.0f; // Convert to milliseconds

    // Immediately increase speeds for all stars
    minSpeedColsPerSec = originalMinSpeed * speedMultiplier;
    maxSpeedColsPerSec = originalMaxSpeed * speedMultiplier;

    // Update existing stars to new speeds
    if (starsArr) {
        for (int i = 0; i < activeStarCount; i++) {
            Star &s = starsArr[i];
            // Scale current velocity by multiplier
            s.vx = s.vx * speedMultiplier;
            // Optionally move stars toward center rows
            int centerRow = CURTAIN_HEIGHT / 2;
            if (cmd.getNamed("centerRows", "false") == "true") {
                // Gradually move to center third of display
                int targetRow = centerRow + random(-CURTAIN_HEIGHT/6, CURTAIN_HEIGHT/6);
                s.row = targetRow;
            }
        }
    }

    response.setNamed("status", "climax_started");
    response.setNamed("duration", String(climaxTime));
    response.setNamed("speedMultiplier", String(speedMultiplier));

    // Note: To restore speeds after climax, you'll need to call this from your main loop:
    // if (climaxActive && (millis() - climaxStartTime) >= climaxDuration) {
    //     minSpeedColsPerSec = originalMinSpeed;
    //     maxSpeedColsPerSec = originalMaxSpeed;
    //     climaxActive = false;
    // }
}

void ClimaxCommandHandler::handleStart(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Parse buildup time parameter (in seconds)
    float buildupTime = cmd.getNamed("time", "10.0").toFloat();

    if (buildupTime <= 0 || buildupTime > 120) {
        buildError(response, cmd.command, "Invalid time parameter. Must be between 0 and 120 seconds.");
        return;
    }

    // Parse target speed multiplier
    float targetMultiplier = cmd.getNamed("targetSpeed", "5.0").toFloat();
    if (targetMultiplier < 1.0 || targetMultiplier > 10.0) {
        targetMultiplier = 5.0;
    }

    // Calculate speed increment per frame (assuming ~50 FPS from frameTargetMs)
    float framesPerSecond = 1000.0f / frameTargetMs;
    float totalFrames = buildupTime * framesPerSecond;
    float speedIncrement = (targetMultiplier - 1.0f) / totalFrames;

    // Store buildup state
    climaxActive = true;
    climaxStartTime = millis();
    climaxDuration = buildupTime * 1000.0f;
    originalMinSpeed = minSpeedColsPerSec;
    originalMaxSpeed = maxSpeedColsPerSec;

    // Immediate approach: set target speeds now
    // For gradual buildup, you'll need to update speeds in main loop
    minSpeedColsPerSec = originalMinSpeed * targetMultiplier;
    maxSpeedColsPerSec = originalMaxSpeed * targetMultiplier;

    // Update all active stars to gradually increase speed
    if (starsArr) {
        for (int i = 0; i < activeStarCount; i++) {
            Star &s = starsArr[i];
            // Gradually increase velocity toward target
            float currentMultiplier = s.vx / originalMinSpeed;
            s.vx = s.vx * (targetMultiplier / currentMultiplier);
        }
    }

    response.setNamed("status", "buildup_started");
    response.setNamed("duration", String(buildupTime));
    response.setNamed("targetSpeed", String(targetMultiplier));
    response.setNamed("speedIncrement", String(speedIncrement));

    // Note: For gradual buildup, add this to your main loop:
    // if (climaxActive) {
    //     unsigned long elapsed = millis() - climaxStartTime;
    //     if (elapsed < climaxDuration) {
    //         float progress = elapsed / climaxDuration;
    //         float currentMultiplier = 1.0f + (targetMultiplier - 1.0f) * progress;
    //         minSpeedColsPerSec = originalMinSpeed * currentMultiplier;
    //         maxSpeedColsPerSec = originalMaxSpeed * currentMultiplier;
    //     } else {
    //         climaxActive = false;
    //     }
    // }
}