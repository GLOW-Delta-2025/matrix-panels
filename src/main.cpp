#include "config.h"
#include "octo_wrapper.h"
#include "renderer.h"
#include "stars.h"
#include "serial_reader.h"


unsigned long lastMicros = 0;


void setup() {
  serialInit();
  Serial.println("OctoStars starting...");


  rendererInit();
  starsInit();
  octoBegin();


  lastMicros = micros();
}


void loop() {
  unsigned long now = micros();
  float dt = (now - lastMicros) / 1000000.0f;
  if (dt <= 0.0f) dt = 0.001f;
  lastMicros = now;
  if (dt > 0.1f) dt = 0.1f;


  serialLoop(); // non-blocking serial commands


  fadeBuffer();
  updateAndRenderStars(dt);
  copyBufferToOcto();
  octoShow();


  unsigned long frameMicros = micros() - now;
  unsigned long targetMicros = frameTargetMs * 1000UL;
  if (frameMicros < targetMicros) delay((targetMicros - frameMicros) / 1000);
}