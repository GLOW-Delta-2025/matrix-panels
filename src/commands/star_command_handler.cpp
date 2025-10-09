#include "commands/climax_command_handler.h"
#include "config.h"
#include "stars.h"
#include "renderer.h"
#include "mapping.h"

// Global state for climax animations
static bool climaxActive = false;
static unsigned long climaxStartTime = 0;
static float climaxDuration = 0;
static float originalMinSpeed = 0;
static float originalMaxSpeed = 0;
static bool climaxRising = false;
static float riseSpeed = 0;
static float spinIntensity = 0;

// Structure to track original star states
struct OriginalStarState {
    int row;
    float vx;
    float bright;
};
static OriginalStarState* originalStates = nullptr;
static int savedStarCount = 0;

void ClimaxCommandHandler::handle(const cmdlib::Command &cmd, cmdlib::Command &response) {
    if (cmd.command == "BUILDUP_CLIMAX_CENTER") {
        handleBuildUp(cmd, response);
    } else if (cmd.command == "START_CLIMAX_CENTER") {
        handleStart(cmd, response);
    }
}

void ClimaxCommandHandler::handleBuildUp(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Parse parameters
    float climaxTime = cmd.getNamed("time", "5.0").toFloat();
    float speedMultiplier = cmd.getNamed("speedMultiplier", "5.0").toFloat();
    float riseRate = cmd.getNamed("riseRate", "2.0").toFloat(); // Rows per second to rise
    String burstMode = cmd.getNamed("burst", "true"); // Whether to add burst of new stars

    if (climaxTime <= 0 || climaxTime > 60) {
        buildError(response, cmd.command, "Invalid time parameter. Must be between 0 and 60 seconds.");
        return;
    }

    if (speedMultiplier < 1.0 || speedMultiplier > 20.0) {
        speedMultiplier = 5.0;
    }

    // Save original star states
    if (!originalStates) {
        originalStates = (OriginalStarState*) malloc(sizeof(OriginalStarState) * MAX_STARS);
    }

    if (originalStates && starsArr) {
        savedStarCount = activeStarCount;
        for (int i = 0; i < activeStarCount; i++) {
            originalStates[i].row = starsArr[i].row;
            originalStates[i].vx = starsArr[i].vx;
            originalStates[i].bright = starsArr[i].bright;
        }
    }

    // Store original speeds
    originalMinSpeed = minSpeedColsPerSec;
    originalMaxSpeed = maxSpeedColsPerSec;

    // Activate climax mode
    climaxActive = true;
    climaxRising = true;
    climaxStartTime = millis();
    climaxDuration = climaxTime * 1000.0f;
    riseSpeed = riseRate;
    spinIntensity = speedMultiplier;

    // Set new speed ranges for spinning effect
    minSpeedColsPerSec = originalMinSpeed * speedMultiplier;
    maxSpeedColsPerSec = originalMaxSpeed * speedMultiplier * 1.5f; // Extra boost for max speed

    // Add burst of stars if requested
    if (burstMode == "true") {
        int burstCount = min(30, MAX_STARS - activeStarCount);
        for (int i = 0; i < burstCount; i++) {
            if (activeStarCount >= MAX_STARS) break;

            Star &s = starsArr[activeStarCount];
            // Start from random positions across the width
            s.x = random(0, TOTAL_WIDTH * 100) / 100.0f;
            // Start from bottom half of display
            s.row = random(CURTAIN_HEIGHT / 2, CURTAIN_HEIGHT);
            // High speed for spinning effect, alternating directions for chaos
            float baseSpeed = random((int)(minSpeedColsPerSec * 100), (int)(maxSpeedColsPerSec * 100)) / 100.0f;
            s.vx = (i % 3 == 0) ? -baseSpeed : baseSpeed; // Some stars go backwards for spin effect
            s.bright = 1.0f; // Full brightness for dramatic effect

            // Use bright white/yellow for climax
            s.r = 255;
            s.g = random(200, 256);
            s.b = random(100, 201);

            activeStarCount++;
        }
    }

    // Transform existing stars for climax
    if (starsArr) {
        for (int i = 0; i < savedStarCount; i++) {
            Star &s = starsArr[i];

            // Dramatically increase speed with some randomness
            float speedBoost = speedMultiplier * random(80, 121) / 100.0f;
            s.vx = s.vx * speedBoost;

            // Alternate some directions for spinning chaos
            if (random(0, 100) < 30) { // 30% chance to reverse
                s.vx = -s.vx;
            }

            // Boost brightness
            s.bright = min(1.0f, s.bright * 1.5f);

            // Shift colors toward warm/bright for climax
            s.r = min(255, (int)(s.r * 1.3f));
            s.g = min(255, (int)(s.g * 1.2f));
            s.b = min(255, (int)(s.b * 0.8f)); // Reduce blue for warmer tone
        }
    }

    response.setNamed("status", "climax_buildup_started");
    response.setNamed("duration", String(climaxTime));
    response.setNamed("speedMultiplier", String(speedMultiplier));
    response.setNamed("riseRate", String(riseRate));
    response.setNamed("totalStars", String(activeStarCount));
}

void ClimaxCommandHandler::handleStart(const cmdlib::Command &cmd, cmdlib::Command &response) {
    // Parse parameters for gradual buildup to climax
    float buildupTime = cmd.getNamed("time", "10.0").toFloat();
    float targetSpeed = cmd.getNamed("targetSpeed", "8.0").toFloat();
    float finalRiseRate = cmd.getNamed("finalRiseRate", "5.0").toFloat(); // Final rise speed
    String addStars = cmd.getNamed("addStars", "true");

    if (buildupTime <= 0 || buildupTime > 120) {
        buildError(response, cmd.command, "Invalid time parameter. Must be between 0 and 120 seconds.");
        return;
    }

    if (targetSpeed < 1.0 || targetSpeed > 20.0) {
        targetSpeed = 8.0;
    }

    // Save current states if not already saved
    if (!originalStates) {
        originalStates = (OriginalStarState*) malloc(sizeof(OriginalStarState) * MAX_STARS);
    }

    if (originalStates && starsArr) {
        savedStarCount = activeStarCount;
        for (int i = 0; i < activeStarCount; i++) {
            originalStates[i].row = starsArr[i].row;
            originalStates[i].vx = starsArr[i].vx;
            originalStates[i].bright = starsArr[i].bright;
        }
    }

    // Store original speeds
    originalMinSpeed = minSpeedColsPerSec;
    originalMaxSpeed = maxSpeedColsPerSec;

    // Activate gradual climax mode
    climaxActive = true;
    climaxRising = true;
    climaxStartTime = millis();
    climaxDuration = buildupTime * 1000.0f;
    riseSpeed = finalRiseRate;
    spinIntensity = targetSpeed;

    // Gradually add stars throughout buildup if requested
    if (addStars == "true") {
        int starsToAdd = min(20, MAX_STARS - activeStarCount);
        int starsPerPhase = starsToAdd / 3; // Add in 3 phases

        // Phase 1: Add some stars immediately
        for (int i = 0; i < starsPerPhase && activeStarCount < MAX_STARS; i++) {
            Star &s = starsArr[activeStarCount];
            s.x = random(-TOTAL_WIDTH/2, TOTAL_WIDTH + TOTAL_WIDTH/2) / 1.0f;
            s.row = random(CURTAIN_HEIGHT * 2/3, CURTAIN_HEIGHT); // Start from bottom third
            float speed = random((int)(minSpeedColsPerSec * 150), (int)(maxSpeedColsPerSec * 150)) / 100.0f;
            s.vx = (random(0, 2) == 0) ? speed : -speed;
            s.bright = random(60, 101) / 100.0f;

            // Gradually shifting colors
            s.r = random(200, 256);
            s.g = random(150, 230);
            s.b = random(50, 150);

            activeStarCount++;
        }
    }

    // Start with moderate speed increase, will build up over time
    minSpeedColsPerSec = originalMinSpeed * 1.5f;
    maxSpeedColsPerSec = originalMaxSpeed * 2.0f;

    // Begin transformation of existing stars
    if (starsArr) {
        for (int i = 0; i < savedStarCount; i++) {
            Star &s = starsArr[i];

            // Start with moderate speed increase
            s.vx = s.vx * 1.5f;

            // Some stars start spinning opposite direction
            if (random(0, 100) < 20) {
                s.vx = -s.vx;
            }

            // Gradually increase brightness
            s.bright = min(1.0f, s.bright * 1.2f);
        }
    }

    response.setNamed("status", "climax_gradual_started");
    response.setNamed("buildupTime", String(buildupTime));
    response.setNamed("targetSpeed", String(targetSpeed));
    response.setNamed("finalRiseRate", String(finalRiseRate));
    response.setNamed("totalStars", String(activeStarCount));
}

// This function should be called from main loop to update climax animation
void updateClimaxAnimation(float dt) {
    if (!climaxActive) return;

    unsigned long elapsed = millis() - climaxStartTime;
    float progress = min(1.0f, elapsed / climaxDuration);

    if (elapsed >= climaxDuration) {
        // Climax ending - restore original settings
        climaxActive = false;
        climaxRising = false;

        minSpeedColsPerSec = originalMinSpeed;
        maxSpeedColsPerSec = originalMaxSpeed;

        // Restore original star states if saved
        if (originalStates && starsArr && savedStarCount > 0) {
            for (int i = 0; i < min(savedStarCount, activeStarCount); i++) {
                // Gradually restore original properties
                starsArr[i].row = originalStates[i].row;
                starsArr[i].vx = originalStates[i].vx;
                starsArr[i].bright = originalStates[i].bright;

                // Restore original colors
                starsArr[i].r = STAR_R;
                starsArr[i].g = STAR_G;
                starsArr[i].b = STAR_B;
            }
        }

        // Remove extra stars added during climax
        if (activeStarCount > savedStarCount) {
            activeStarCount = savedStarCount;
        }

        return;
    }

    // During climax: make stars rise and spin
    if (climaxRising && starsArr) {
        for (int i = 0; i < activeStarCount; i++) {
            Star &s = starsArr[i];

            // Rising effect - move stars toward top (row 0)
            if (s.row > 0) {
                float riseAmount = riseSpeed * dt * progress; // Accelerate rise over time
                s.row -= (int)riseAmount;
                if (s.row < 0) s.row = 0;
            }

            // Spinning effect - oscillate speed and occasionally reverse direction
            float speedOscillation = sin(elapsed * 0.005f + i * 0.5f) * 0.3f + 1.0f;
            float currentSpeedMult = 1.0f + (spinIntensity - 1.0f) * progress;

            // Apply oscillating speed
            if (abs(s.vx) < maxSpeedColsPerSec * 2) { // Prevent runaway speeds
                s.vx = s.vx * (1.0f + (speedOscillation - 1.0f) * 0.02f);
            }

            // Occasional direction changes for chaos
            if (random(0, 1000) < 5) { // 0.5% chance per frame
                s.vx = -s.vx;
            }

            // Pulsing brightness
            float brightPulse = sin(elapsed * 0.01f + i * 0.3f) * 0.2f + 0.8f;
            s.bright = min(1.0f, s.bright * brightPulse);

            // Color shift toward white as climax progresses
            if (progress > 0.5f) {
                float whiteShift = (progress - 0.5f) * 2.0f; // 0 to 1 in second half
                s.r = min(255, (int)(s.r + (255 - s.r) * whiteShift * 0.5f));
                s.g = min(255, (int)(s.g + (255 - s.g) * whiteShift * 0.5f));
                s.b = min(255, (int)(s.b + (255 - s.b) * whiteShift * 0.3f));
            }

            // When stars reach the top, they can wrap around or bounce
            if (s.row == 0 && progress > 0.7f) {
                // 50% chance to wrap to bottom, 50% to bounce back down slightly
                if (random(0, 100) < 50) {
                    s.row = CURTAIN_HEIGHT - 1; // Wrap to bottom
                    s.vx = s.vx * 0.8f; // Slow down a bit after wrap
                } else {
                    s.row = random(1, 4); // Bounce down slightly
                    s.vx = -s.vx * 1.2f; // Reverse and speed up
                }
            }
        }

        // Gradually increase speed limits during climax
        float speedProgress = 1.0f + (spinIntensity - 1.0f) * progress;
        minSpeedColsPerSec = originalMinSpeed * speedProgress;
        maxSpeedColsPerSec = originalMaxSpeed * speedProgress * 1.5f;

        // Add occasional new stars during climax for extra drama
        if (progress > 0.3f && progress < 0.8f && random(0, 100) < 2) { // 2% chance
            if (activeStarCount < MAX_STARS) {
                Star &s = starsArr[activeStarCount];
                s.x = random(0, TOTAL_WIDTH * 100) / 100.0f;
                s.row = CURTAIN_HEIGHT - 1; // Start from bottom
                s.vx = random(0, 2) == 0 ? maxSpeedColsPerSec : -maxSpeedColsPerSec;
                s.bright = 1.0f;
                s.r = 255;
                s.g = 255;
                s.b = random(200, 256);
                activeStarCount++;
            }
        }
    }
}

// Add this to the header file or make it accessible to main.cpp
bool isClimaxActive() {
    return climaxActive;
}

float getClimaxProgress() {
    if (!climaxActive) return 0.0f;
    unsigned long elapsed = millis() - climaxStartTime;
    return min(1.0f, elapsed / climaxDuration);
}