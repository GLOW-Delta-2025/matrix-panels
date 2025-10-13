#include "commands/climax_command_handler.h"
#include "config.h"
#include "stars.h"

// Global state for climax animations
static bool climaxBuildupActive = false;
static bool climaxSpiralActive = false;
static unsigned long climaxStartTime = 0;
static float climaxDuration = 0;
static float targetSpeedMultiplier = 1.0f;
static float originalMinSpeed = 0;
static float originalMaxSpeed = 0;
static int* originalRows = nullptr;
static float* originalStarSpeeds = nullptr;  // Store original star speeds

void ClimaxCommandHandler::handle(const cmdlib::Command &cmd, cmdlib::Command &response) {
    if (cmd.command == "BUILDUP_CLIMAX_CENTER") {
        handleBuildUp(cmd, response);
    } else if (cmd.command == "START_CLIMAX_CENTER") {
        handleStart(cmd, response);
    }
}

// Global variables for enhanced buildup effect
static float originalFadeFactor = 0;
static float* originalBrightness = nullptr;
static int buildupStarsAdded = 0;

void ClimaxCommandHandler::handleBuildUp(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Parse duration parameter (in seconds)
    float duration = cmd.getNamed("duration", "10.0").toFloat();
    if (duration <= 0 || duration > 120) {
        buildError(response, cmd.command, "Invalid duration. Must be between 0 and 120 seconds.");
        return;
    }

    // Parse target speed multiplier
    targetSpeedMultiplier = cmd.getNamed("speedMultiplier", "5.0").toFloat();
    if (targetSpeedMultiplier < 1.0f || targetSpeedMultiplier > 20.0f) {
        targetSpeedMultiplier = 5.0f;
    }

    // Store original speeds and fade factor
    originalMinSpeed = minSpeedColsPerSec;
    originalMaxSpeed = maxSpeedColsPerSec;
    originalFadeFactor = fadeFactor;

    // Allocate memory for original star speeds and brightness if needed
    if (!originalStarSpeeds) {
        originalStarSpeeds = (float*) malloc(sizeof(float) * MAX_STARS);
    }
    if (!originalBrightness) {
        originalBrightness = (float*) malloc(sizeof(float) * MAX_STARS);
    }
    if (!originalRows) {
        originalRows = (int*) malloc(sizeof(int) * MAX_STARS);
    }

    // Store each star's original speed, brightness, and row
    if (starsArr && originalStarSpeeds && originalBrightness && originalRows) {
        for (int i = 0; i < activeStarCount; i++) {
            originalStarSpeeds[i] = starsArr[i].vx;
            originalBrightness[i] = starsArr[i].bright;
            originalRows[i] = starsArr[i].row;
        }
    }

    // Reset buildup stars counter
    buildupStarsAdded = 0;

    // Activate buildup mode
    climaxBuildupActive = true;
    climaxSpiralActive = false;
    climaxStartTime = millis();
    climaxDuration = duration * 1000.0f; // Convert to milliseconds

    buildResponse(response, cmd.command);
}

void ClimaxCommandHandler::handleStart(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Parse duration parameter (in seconds)
    float duration = cmd.getNamed("duration", "15.0").toFloat();
    if (duration <= 0 || duration > 120) {
        buildError(response, cmd.command, "Invalid duration. Must be between 0 and 120 seconds.");
        return;
    }

    // Parse spiral speed (how fast stars move upward)
    float spiralSpeed = cmd.getNamed("spiralSpeed", "0.5").toFloat(); // rows per second
    if (spiralSpeed <= 0 || spiralSpeed > 5.0f) {
        spiralSpeed = 0.5f;
    }

    // Parse horizontal speed multiplier
    float speedMultiplier = cmd.getNamed("speedMultiplier", "5.0").toFloat();
    if (speedMultiplier < 1.0f || speedMultiplier > 10.0f) {
        speedMultiplier = 5.0f;
    }

    // Store original speeds and positions
    originalMinSpeed = minSpeedColsPerSec;
    originalMaxSpeed = maxSpeedColsPerSec;

    // Allocate memory to store original row positions and speeds if needed
    if (!originalRows) {
        originalRows = (int*) malloc(sizeof(int) * MAX_STARS);
    }
    if (!originalStarSpeeds) {
        originalStarSpeeds = (float*) malloc(sizeof(float) * MAX_STARS);
    }

    // Store original row positions and speeds, then set high speeds
    if (starsArr && originalRows && originalStarSpeeds) {
        for (int i = 0; i < activeStarCount; i++) {
            originalRows[i] = starsArr[i].row;
            originalStarSpeeds[i] = starsArr[i].vx;
            // Set high horizontal speed based on original
            starsArr[i].vx = originalStarSpeeds[i] * speedMultiplier;
        }
    }

    // Set global speeds to high values
    minSpeedColsPerSec = originalMinSpeed * speedMultiplier;
    maxSpeedColsPerSec = originalMaxSpeed * speedMultiplier;

    // Activate spiral mode
    climaxSpiralActive = true;
    climaxBuildupActive = false;
    climaxStartTime = millis();
    climaxDuration = duration * 1000.0f;
    targetSpeedMultiplier = spiralSpeed; // Reuse variable for spiral speed

    buildResponse(response, cmd.command);
}

// This function should be called from the main loop to update climax effects
void updateClimaxEffects() {
    if (!climaxBuildupActive && !climaxSpiralActive) return;

    unsigned long now = millis();
    unsigned long elapsed = now - climaxStartTime;

    if (climaxBuildupActive) {
        if (elapsed < climaxDuration) {
            float progress = elapsed / climaxDuration;
            float speedMultiplier;

            // First 70%: gradual acceleration
            // Last 30%: full speed
            if (progress < 0.7f) {
                // Gradual acceleration from 1.0 to target over first 70%
                float accelProgress = progress / 0.7f; // 0 to 1 over first 70%
                speedMultiplier = 1.0f + (targetSpeedMultiplier - 1.0f) * accelProgress;
            } else {
                // Full target speed for last 30%
                speedMultiplier = targetSpeedMultiplier;
            }

            // Apply speed multiplier to global speeds
            minSpeedColsPerSec = originalMinSpeed * speedMultiplier;
            maxSpeedColsPerSec = originalMaxSpeed * speedMultiplier;

            // Update existing star speeds based on ORIGINAL speeds
            if (starsArr && originalStarSpeeds) {
                for (int i = 0; i < activeStarCount; i++) {
                    // Set speed based on original speed, not current speed
                    starsArr[i].vx = originalStarSpeeds[i] * speedMultiplier;
                }
            }
        } else {
            // Buildup complete - restore original speeds
            minSpeedColsPerSec = originalMinSpeed;
            maxSpeedColsPerSec = originalMaxSpeed;

            // Restore original star speeds
            if (starsArr && originalStarSpeeds) {
                for (int i = 0; i < activeStarCount; i++) {
                    starsArr[i].vx = originalStarSpeeds[i];
                }
            }

            climaxBuildupActive = false;
        }
    }

    if (climaxSpiralActive) {
        if (elapsed < climaxDuration) {
            float dt = (elapsed > 0) ? 0.02f : 0; // Assume ~50 FPS
            float rowsPerFrame = targetSpeedMultiplier * dt; // targetSpeedMultiplier holds spiral speed

            // Move stars upward in spiral pattern
            if (starsArr) {
                for (int i = 0; i < activeStarCount; i++) {
                    Star &s = starsArr[i];

                    // Move upward slowly
                    s.row -= rowsPerFrame;

                    // Wrap around from top to bottom for continuous spiral
                    if (s.row < 0) {
                        s.row += CURTAIN_HEIGHT;
                    }

                    // Optional: Add slight sine wave for more spiral effect
                    float spiralOffset = sin((s.x / TOTAL_WIDTH) * 6.28f) * 0.5f;
                    s.row += spiralOffset * dt;

                    // Keep within bounds
                    if (s.row < 0) s.row = 0;
                    if (s.row >= CURTAIN_HEIGHT) s.row = CURTAIN_HEIGHT - 1;
                }
            }
        } else {
            // Spiral complete - restore original speeds and positions
            minSpeedColsPerSec = originalMinSpeed;
            maxSpeedColsPerSec = originalMaxSpeed;

            // Restore original speeds and row positions
            if (starsArr && originalStarSpeeds) {
                for (int i = 0; i < activeStarCount; i++) {
                    starsArr[i].vx = originalStarSpeeds[i];
                }
            }

            if (starsArr && originalRows) {
                for (int i = 0; i < activeStarCount; i++) {
                    starsArr[i].row = originalRows[i];
                }
            }

            climaxSpiralActive = false;
        }
    }
}

// Cleanup function (optional - call when shutting down)
void cleanupClimaxEffects() {
    if (originalRows) {
        free(originalRows);
        originalRows = nullptr;
    }
    if (originalStarSpeeds) {
        free(originalStarSpeeds);
        originalStarSpeeds = nullptr;
    }
}

// Note: Add this to main.cpp loop:
// extern void updateClimaxEffects();
// Then call updateClimaxEffects(); in the main loop