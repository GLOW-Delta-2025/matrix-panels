#include "config.h"
#include "octo_wrapper.h"
#include "renderer.h"
#include "stars.h"
#include "command_handler.h"
#include "../lib/PingPong.cpp"

unsigned long lastMicros = 0;

void setup() {
  PingPong.init(30000, &Serial1);
  commandHandlerInit();

  rendererInit();
  starsInit();
  octoBegin();

  lastMicros = micros();
}

extern void updateClimaxEffects();

void loop() {
  PingPong.update();

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
  // TODO Add idle state
  // if (PING_IDLE) {
  //   Serial.println("No ping ping");
  // }

  unsigned long frameMicros = micros() - now;
  unsigned long targetMicros = frameTargetMs * 1000UL;
  delay(10);
}
