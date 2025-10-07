#include "serial_reader.h"
#include "../include/config.h"
#include "stars.h"

/* Available commands
 *  ADD_STAR - Add a star to the matrix.
 *    identifier
 *    color       - Used for COLOR uses default if empty
 *    speed       - Collision speed of the star
 *    arm         - Used to determine where to start the star
 *
 *  REMOVE_STAR
 *    identifier
 *
 *  CLEAR
 *
 *  CLIMAX
 *    Effect?
 *
 *  CONFIG
 *  direction
 *  laneSwitch
 *  defaultColor
 *  minStarSpeed
 *  maxStarSpeed
 *
 *
 * */


void serialInit() {
  Serial.begin(9600);
  while (!Serial) { /* wait for USB */ }
  Serial.println("Serial reader ready. Commands: COUNT n | SPEED min max | FADE x | WRAP 0/1 | RANDOM_ROWS 0/1 | ROW r");
}

static void handleLine(char *line) {
  // trim newline
  char *p = line;
  while (*p == ' ' || *p == '\t') p++;
  if (*p == 0) return;

  char *cmd = strtok(p, " \t\r\n");
  if (!cmd) return;


  if (strcasecmp(cmd, "COUNT") == 0) {
    char *arg = strtok(NULL, " \t\r\n");
    if (arg) {
      int v = atoi(arg);
      if (v < 1) v = 1;
      if (v > MAX_STARS) v = MAX_STARS;
      activeStarCount = v;
      Serial.printf("activeStarCount=%d\n", activeStarCount);
    }
    else if (strcasecmp(cmd, "ADD_STAR")) {


    }
  } else if (strcasecmp(cmd, "SPEED") == 0) {
    char *a1 = strtok(NULL, " \t\r\n");
    char *a2 = strtok(NULL, " \t\r\n");
    if (a1 && a2) {
      float mi = atof(a1);
      float ma = atof(a2);
      if (mi < 0) mi = 0;
      if (ma < mi) ma = mi;
      minSpeedColsPerSec = mi;
      maxSpeedColsPerSec = ma;
      Serial.printf("speed range: %.2f - %.2f cols/s\n", minSpeedColsPerSec, maxSpeedColsPerSec);
    }
  } else if (strcasecmp(cmd, "FADE") == 0) {
    char *a1 = strtok(NULL, " \t\r\n");
    if (a1) {
      float f = atof(a1);
      if (f < 0.0f) f = 0.0f;
      if (f > 1.0f) f = 1.0f;
      fadeFactor = f;
      Serial.printf("fadeFactor=%.3f\n", fadeFactor);
    }
  } else if (strcasecmp(cmd, "WRAP") == 0) {
    char *a1 = strtok(NULL, " \t\r\n");
    if (a1) {
      wrapStars = (atoi(a1) != 0);
      Serial.printf("wrapStars=%d\n", wrapStars ? 1 : 0);
    }
  } else if (strcasecmp(cmd, "RANDOM_ROWS") == 0) {
    char *a1 = strtok(NULL, " \t\r\n");
    if (a1) {
      randomRows = (atoi(a1) != 0);
      Serial.printf("randomRows=%d\n", randomRows ? 1 : 0);
    }
  } else if (strcasecmp(cmd, "ROW") == 0) {
    char *a1 = strtok(NULL, " \t\r\n");
    if (a1) {
      int r = atoi(a1);
      if (r < 0) r = 0;
      if (r >= CURTAIN_HEIGHT) r = CURTAIN_HEIGHT - 1;
      // set all stars to this row & disable random rows
      for (int i = 0; i < MAX_STARS; i++) {
        // if stars allocated, set rows immediately; otherwise future resets will use randomRows flag
        // we won't include stars.h here to avoid circular includes; user can send ROW while running.
      }
      randomRows = false;
      Serial.printf("set fixed row=%d and randomRows=0 (existing stars may still be on previous rows)\n", r);
    }
  } else {
    Serial.print("Unknown cmd: "); Serial.println(cmd);
  }
}

void serialLoop() {
  static char linebuf[96];
  static size_t idx = 0;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      linebuf[idx] = '\0';
      handleLine(linebuf);
      idx = 0;
    } else {
      if (idx < sizeof(linebuf)-1) linebuf[idx++] = c;
    }
  }
}