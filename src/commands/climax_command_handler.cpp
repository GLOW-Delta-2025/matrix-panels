#include "commands/climax_command_handler.h"
#include "config.h"
#include "stars.h"

#include <stdlib.h>
#include <math.h>

// ─────────────────────────────────────────────────────────────────────────────
// Global state for climax animations
// ─────────────────────────────────────────────────────────────────────────────
static bool           climaxBuildupActive   = false;
static bool           climaxSpiralActive    = false;
static unsigned long  climaxStartTime       = 0;
static float          climaxDuration        = 0;        // milliseconds
static float          targetSpeedMultiplier = 1.0f;     // reused as spiralSpeed during spiral mode
static float          originalMinSpeed      = 0;
static float          originalMaxSpeed      = 0;

static int*           originalRows          = nullptr;  // per-star starting row (for time-based lerp)
static float*         originalStarSpeeds    = nullptr;  // per-star horizontal speed backup
static float*         originalBrightness    = nullptr;  // per-star starting brightness

// Extra control for slight vertical emphasis on very wide matrices
static float          verticalBias          = 1.0f;     // small bias to increase perceived upward motion

// For optional buildup (kept from your original)
static float          originalFadeFactor    = 0;
static int            buildupStarsAdded     = 0;

// ─────────────────────────────────────────────────────────────────────────────
// Helper: Clear all stars + (optionally) the physical LEDs
// ─────────────────────────────────────────────────────────────────────────────
static inline void clearAllStarsAndLeds() {
    if (starsArr) {
        for (int i = 0; i < activeStarCount; i++) {
            starsArr[i].bright = 0.0f;
        }
    }
    activeStarCount = 0;

    // If you have a dedicated LED clear function in your renderer, call it:
    // extern void clearAllLeds();
    // clearAllLeds();
}

// ─────────────────────────────────────────────────────────────────────────────
// Command handling
// ─────────────────────────────────────────────────────────────────────────────
void ClimaxCommandHandler::handle(const cmdlib::Command &cmd, cmdlib::Command &response) {
    if (cmd.command == "BUILDUP_CLIMAX_CENTER") {
        handleBuildUp(cmd, response);
    } else if (cmd.command == "START_CLIMAX_CENTER") {
        handleStart(cmd, response);
    }
}

void ClimaxCommandHandler::handleBuildUp(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Parse duration parameter (in seconds)
    float duration = cmd.getNamed("duration", "10.0").toFloat();
    if (duration <= 0 || duration > 120) {
        buildError(response, cmd.command, "Invalid duration. Must be between 0 and 120 seconds.", cmd.getHeader(0));
        return;
    }

    // Parse target speed multiplier
    targetSpeedMultiplier = cmd.getNamed("speedMultiplier", "5.0").toFloat();
    if (targetSpeedMultiplier < 1.0f || targetSpeedMultiplier > 20.0f) {
        targetSpeedMultiplier = 5.0f;
    }

    // Store original speeds and fade factor
    originalMinSpeed   = minSpeedColsPerSec;
    originalMaxSpeed   = maxSpeedColsPerSec;
    originalFadeFactor = fadeFactor;

    // Allocate memory for backups
    if (!originalStarSpeeds) originalStarSpeeds = (float*) malloc(sizeof(float) * MAX_STARS);
    if (!originalBrightness) originalBrightness = (float*) malloc(sizeof(float) * MAX_STARS);
    if (!originalRows)       originalRows       = (int*)   malloc(sizeof(int)   * MAX_STARS);

    // Store per-star originals
    if (starsArr && originalStarSpeeds && originalBrightness && originalRows) {
        for (int i = 0; i < activeStarCount; i++) {
            originalStarSpeeds[i] = starsArr[i].vx;
            originalBrightness[i]  = starsArr[i].bright;
            originalRows[i]        = starsArr[i].row;
        }
    }

    // Reset buildup stars counter
    buildupStarsAdded = 0;

    // Activate buildup mode
    climaxBuildupActive = true;
    climaxSpiralActive  = false;
    climaxStartTime     = millis();
    climaxDuration      = duration * 1000.0f; // ms

    buildResponse(response, cmd.command, "MASTER");
}

void ClimaxCommandHandler::handleStart(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Parse duration parameter (in seconds)
    float duration = cmd.getNamed("duration", "15.0").toFloat();
    if (duration <= 0 || duration > 120) {
        buildError(response, cmd.command, "Invalid duration. Must be between 0 and 120 seconds.", cmd.getHeader(0));
        return;
    }

    // Parse spiral "wobble" speed (only affects the slight vertical wobble, not the time-based climb)
    float spiralSpeed = cmd.getNamed("spiralSpeed", "0.5").toFloat(); // rows per second influence
    if (spiralSpeed <= 0 || spiralSpeed > 5.0f) {
        spiralSpeed = 0.5f;
    }

    // Parse horizontal speed multiplier (kept for your star field's x-velocity feel)
    float speedMultiplier = cmd.getNamed("speedMultiplier", "5.0").toFloat();
    if (speedMultiplier < 1.0f || speedMultiplier > 10.0f) {
        speedMultiplier = 5.0f;
    }

    // Slight extra push for very wide canvases (optional)
    verticalBias = cmd.getNamed("verticalBias", "1.2").toFloat();
    if (verticalBias < 1.0f) verticalBias = 1.0f;

    // Store original global speeds
    originalMinSpeed = minSpeedColsPerSec;
    originalMaxSpeed = maxSpeedColsPerSec;

    // Allocate backups
    if (!originalRows)       originalRows       = (int*)   malloc(sizeof(int)   * MAX_STARS);
    if (!originalStarSpeeds) originalStarSpeeds = (float*) malloc(sizeof(float) * MAX_STARS);
    if (!originalBrightness) originalBrightness = (float*) malloc(sizeof(float) * MAX_STARS);

    // Backup per-star data + apply horizontal speed boost
    if (starsArr && originalRows && originalStarSpeeds) {
        for (int i = 0; i < activeStarCount; i++) {
            originalRows[i]        = starsArr[i].row;
            originalStarSpeeds[i]  = starsArr[i].vx;
            originalBrightness[i]  = starsArr[i].bright;
            starsArr[i].vx         = originalStarSpeeds[i] * speedMultiplier;
        }
    }

    // Set global speeds to high values for horizontal motion feel
    minSpeedColsPerSec = originalMinSpeed * speedMultiplier;
    maxSpeedColsPerSec = originalMaxSpeed * speedMultiplier;

    // Activate spiral mode
    climaxSpiralActive    = true;
    climaxBuildupActive   = false;
    climaxStartTime       = millis();
    climaxDuration        = duration * 1000.0f; // ms
    targetSpeedMultiplier = spiralSpeed;        // wobble influence

    buildResponse(response, cmd.command, "MASTER");
}

// ─────────────────────────────────────────────────────────────────────────────
// Main updater
// ─────────────────────────────────────────────────────────────────────────────
void updateClimaxEffects() {
    if (!climaxBuildupActive && !climaxSpiralActive) return;

    unsigned long now     = millis();
    unsigned long elapsed = now - climaxStartTime;

    if (climaxBuildupActive) {
        if (elapsed < climaxDuration) {
            float progress = (climaxDuration > 0.0f) ? (float)elapsed / climaxDuration : 1.0f;
            float speedMultiplier;

            // First 70%: gradual acceleration; last 30%: full speed
            if (progress < 0.7f) {
                float accelProgress = progress / 0.7f; // 0..1
                speedMultiplier = 1.0f + (targetSpeedMultiplier - 1.0f) * accelProgress;
            } else {
                speedMultiplier = targetSpeedMultiplier;
            }

            // Apply to globals
            minSpeedColsPerSec = originalMinSpeed * speedMultiplier;
            maxSpeedColsPerSec = originalMaxSpeed * speedMultiplier;

            // Apply to stars based on ORIGINAL speeds
            if (starsArr && originalStarSpeeds) {
                for (int i = 0; i < activeStarCount; i++) {
                    starsArr[i].vx = originalStarSpeeds[i] * speedMultiplier;
                }
            }
        } else {
            // Restore
            minSpeedColsPerSec = originalMinSpeed;
            maxSpeedColsPerSec = originalMaxSpeed;

            if (starsArr && originalStarSpeeds) {
                for (int i = 0; i < activeStarCount; i++) {
                    starsArr[i].vx = originalStarSpeeds[i];
                }
            }

            cmdlib::Command finishCommand;
            finishCommand.addHeader("MASTER");
            finishCommand.msgKind = "REQUEST";
            finishCommand.command = "CLIMAX_READY";
            CommunicationSerial.println(finishCommand.toString());

            climaxBuildupActive = false;
        }
    }

    if (climaxSpiralActive) {
        if (elapsed < climaxDuration) {
            // Normalized progress 0..1 across the runtime
            float progress = (climaxDuration > 0.0f) ? (float)elapsed / climaxDuration : 1.0f;

            // Brightness fade driven by duration: slow at first, finishing at the end.
            // Adjust exponent (>1 = slower fade initially).
            const float fadeExp = 1.5f;
            float fade = 1.0f - powf(progress, fadeExp);
            if (fade < 0.0f) fade = 0.0f;

            // Optional wide-matrix vertical emphasis (just affects wobble magnitude)
            float aspect = 1.0f;
            #if defined(TOTAL_WIDTH) && defined(CURTAIN_HEIGHT)
                if (CURTAIN_HEIGHT > 0) {
                    float ratio = (float)TOTAL_WIDTH / (float)CURTAIN_HEIGHT;
                    if (ratio > 1.0f) aspect = ratio;
                }
            #endif

            if (starsArr && originalRows && originalBrightness) {
                // We compute vertical position as a time-based lerp so all stars
                // reach the TOP (row 0) exactly when progress -> 1.0
                for (int i = 0; i < activeStarCount; i++) {
                    Star &s = starsArr[i];

                    float startRow  = (float)originalRows[i];
                    float targetRow = 0.0f; // reach top at the end of duration
                    float newRow    = startRow + (targetRow - startRow) * progress;

                    // Subtle vertical "wobble" to preserve spiral feel, scaled by aspect & user speed
                    // Wobble is gentle so it won't fight the time-based climb.
                    float wobbleAmp = 0.5f * ((float)CURTAIN_HEIGHT / fmaxf(1.0f, (float)TOTAL_WIDTH));
                    #if !defined(TOTAL_WIDTH)
                        float TOTAL_WIDTH = 100.0f; // safe fallback if not defined
                    #endif
                    #if !defined(CURTAIN_HEIGHT)
                        float CURTAIN_HEIGHT = 26.0f; // safe fallback if not defined
                    #endif
                    float wobble = sinf((s.x / (float)TOTAL_WIDTH) * 6.28318f + progress * targetSpeedMultiplier)
                                   * wobbleAmp * verticalBias * (1.0f - progress); // taper wobble near the end
                    newRow += wobble;

                    // Convert to int row for rendering with clamping (no wrap)
                    int rowInt = (int)floorf(newRow + 0.5f); // round to nearest
                    if (rowInt < 0) rowInt = 0; // keep visible until the end (hits top at t=1)
                    #if defined(CURTAIN_HEIGHT)
                        if (rowInt >= CURTAIN_HEIGHT) rowInt = CURTAIN_HEIGHT - 1;
                    #else
                        if (rowInt >= 26) rowInt = 25;
                    #endif
                    s.row = rowInt;

                    // Apply duration-driven brightness fade
                    s.bright = originalBrightness[i] * fade;
                }
            }
        } else {
            // Time's up — restore globals, then clear everything visually
            minSpeedColsPerSec = originalMinSpeed;
            maxSpeedColsPerSec = originalMaxSpeed;

            // Restore horizontal speeds (not critical since we clear next)
            if (starsArr && originalStarSpeeds) {
                for (int i = 0; i < activeStarCount; i++) {
                    starsArr[i].vx = originalStarSpeeds[i];
                }
            }

            // Hard clear: stars + LEDs
            clearAllStarsAndLeds();

            climaxSpiralActive = false;

            // Send buildup finished command
            cmdlib::Command finishCommand;
            finishCommand.addHeader("MASTER");
            finishCommand.msgKind = "REQUEST";
            finishCommand.command = "CLIMAX_DONE_CENTER";
            CommunicationSerial.println(finishCommand.toString());
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Cleanup function (optional - call when shutting down)
// ─────────────────────────────────────────────────────────────────────────────
void cleanupClimaxEffects() {
    if (originalRows)        { free(originalRows);        originalRows = nullptr; }
    if (originalStarSpeeds)  { free(originalStarSpeeds);  originalStarSpeeds = nullptr; }
    if (originalBrightness)  { free(originalBrightness);  originalBrightness = nullptr; }
}

// Note: Add this to main.cpp loop:
//   extern void updateClimaxEffects();
//   updateClimaxEffects(); // call each frame
