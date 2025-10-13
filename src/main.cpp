#include "config.h"
#include "octo_wrapper.h"
#include "renderer.h"
#include "stars.h"
#include "command_handler.h"
#include "commands/climax_command_handler.h"

unsigned long lastMicros = 0;

void setup() {
  commandHandlerInit();

  rendererInit();
  starsInit();
  octoBegin();

  lastMicros = micros();
}

extern void updateClimaxEffects();

void loop() {
  // Handle serial commands
  processSerialCommands();

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
