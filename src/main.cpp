// OctoWS2811 - Horizontal star transfer with trail across 5 curtains (20x26 each)
// Assumes: 5 curtains side-by-side => total width = 5 * 20 = 100 columns
// Orange star(s) moving left->right on same row, with fade trail.
// Tweak the top configuration block for star count, speed, fade, mapping, row selection.

#include <OctoWS2811.h>
#include <Arduino.h>

// ---------------- USER CONFIG - EDIT THESE ----------------
const int CURTAINS = 5;             // number of data pins / curtains
const int CURTAIN_WIDTH = 20;       // columns per curtain
const int CURTAIN_HEIGHT = 26;      // rows per curtain
// Combined
const int TOTAL_WIDTH = CURTAINS * CURTAIN_WIDTH;
const int TOTAL_HEIGHT = CURTAIN_HEIGHT;
const int LEDS_PER_CURTAIN = CURTAIN_WIDTH * CURTAIN_HEIGHT;

// Octo pins: set USE_PINLIST true if you want custom pins (Teensy 4.x). Example defaults below for 5 pins.
const bool USE_PINLIST = false;
const byte pinList[CURTAINS] = { 2, 14, 7, 8, 6 }; // change if using custom wiring / Teensy4
// Mapping flags - set to match how the curtain is wired internally:
// LAYOUT_ROWS = true -> index advances by columns across a ROW then next row (row-major).
// If false -> column-major (advance down column then next column).
const bool SERPENTINE = false;       // common curtain wiring is serpentine (every other row reversed)

// If a curtain is wired upside-down, set invertCurtain[i] = true for that curtain.
bool invertCurtain[CURTAINS] = { false, false, false, false, false };

// Star effect tuning:
const int STAR_COUNT = 45;          // simultaneous stars
const float MIN_SPEED_COLS_PER_SEC = 10; // columns per second (global columns)
const float MAX_SPEED_COLS_PER_SEC = 50;
const float FADE_FACTOR = 0.86f;    // 0..1 per-frame fade (closer to 1 = longer trails)
const unsigned long FRAME_TARGET_MS = 20; // target frame time (ms) - ~50 FPS
const bool RANDOM_ROWS = true;     // false => use ROW_TO_USE; true => pick random rows for each star
const bool WRAP_STARS = false;      // when a star reaches right edge: wrap or respawn on left

// Orange color for stars
const uint8_t STAR_R = 255;
const uint8_t STAR_G = 191;
const uint8_t STAR_B = 0;
// ------------------------------------------------------------

// OctoWS memory (must be sized to LEDs per curtain)
DMAMEM int displayMemory[LEDS_PER_CURTAIN * 6];
int drawingMemory[LEDS_PER_CURTAIN * 6];
const int config = WS2811_GRB | WS2811_800kHz;

#if USE_PINLIST
OctoWS2811 leds(LEDS_PER_CURTAIN, displayMemory, drawingMemory, config, CURTAINS, (byte*)pinList);
#else
OctoWS2811 leds(LEDS_PER_CURTAIN, displayMemory, drawingMemory, config);
#endif

// Software RGB buffer for smooth trails
const int NUM_PIXELS = CURTAINS * LEDS_PER_CURTAIN;
uint8_t *pixBuf = nullptr; // NUM_PIXELS * 3 bytes

struct Star {
  float x;       // global continuous column position (0 .. TOTAL_WIDTH)
  int row;       // row (0 .. TOTAL_HEIGHT-1)
  float vx;      // columns per second (speed)
  float bright;  // 0..1 multiplier
};
Star *stars = nullptr;

unsigned long lastMicros = 0;

// Utility: return linear pixel index inside a curtain given local (col,row)
// according to LAYOUT_ROWS and SERPENTINE settings.
int localIndexInCurtain(int col, int row) {
    if (SERPENTINE && (col % 2 == 1)) {
      return col * CURTAIN_HEIGHT + (CURTAIN_HEIGHT - 1 - row);
    } else {
      return col * CURTAIN_HEIGHT + row;
    }
}

// Convert global (curtainIndex, localIndex) to Octo global index:
int globalOctoIndex(int curtainIdx, int localIndex) {
  // global index used by OctoWS: curtainIdx * LEDS_PER_CURTAIN + localIndex
  return curtainIdx * LEDS_PER_CURTAIN + localIndex;
}

// Add RGB to soft buffer (with clipping)
inline void addPixelRGB_soft(int globalPixelIdx, float r, float g, float b) {
  if (globalPixelIdx < 0 || globalPixelIdx >= NUM_PIXELS) return;
  int base = globalPixelIdx * 3;
  int v;
  v = (int)pixBuf[base + 0] + (int)r; if (v > 255) v = 255; pixBuf[base + 0] = (uint8_t)v;
  v = (int)pixBuf[base + 1] + (int)g; if (v > 255) v = 255; pixBuf[base + 1] = (uint8_t)v;
  v = (int)pixBuf[base + 2] + (int)b; if (v > 255) v = 255; pixBuf[base + 2] = (uint8_t)v;
}

// Render a star at continuous global x position onto the soft buffer on integer row `row`
// with horizontal blending between columns for smooth motion.
void renderStarToBuffer(const Star &s) {
  float fx = s.x;
  int leftCol = (int)floorf(fx);
  float frac = fx - leftCol;
  float wl = 1.0f - frac;
  float wr = frac;

  // color scaled by brightness (0..1). Convert to 0..255 space then multiply weights.
  float br = s.bright;
  float rL = STAR_R * br * wl;
  float gL = STAR_G * br * wl;
  float bL = STAR_B * br * wl;
  float rR = STAR_R * br * wr;
  float gR = STAR_G * br * wr;
  float bR = STAR_B * br * wr;

  // left column
  if (leftCol >= 0 && leftCol < TOTAL_WIDTH) {
    int curtainL = leftCol / CURTAIN_WIDTH;
    int localColL = leftCol % CURTAIN_WIDTH;
    int row = s.row;
    // handle per-curtain inversion of rows
    if (curtainL >= 0 && curtainL < CURTAINS) {
      int rowOut = invertCurtain[curtainL] ? (CURTAIN_HEIGHT - 1 - row) : row;
      int localIndex = localIndexInCurtain(localColL, rowOut);
      int globalIdx = globalOctoIndex(curtainL, localIndex);
      addPixelRGB_soft(globalIdx, rL, gL, bL);
    }
  }

  // right column (leftCol+1)
  int rightCol = leftCol + 1;
  if (rightCol >= 0 && rightCol < TOTAL_WIDTH) {
    int curtainR = rightCol / CURTAIN_WIDTH;
    int localColR = rightCol % CURTAIN_WIDTH;
    int row = s.row;
    if (curtainR >= 0 && curtainR < CURTAINS) {
      int rowOut = invertCurtain[curtainR] ? (CURTAIN_HEIGHT - 1 - row) : row;
      int localIndex = localIndexInCurtain(localColR, rowOut);
      int globalIdx = globalOctoIndex(curtainR, localIndex);
      addPixelRGB_soft(globalIdx, rR, gR, bR);
    }
  }
}

void fadeBuffer() {
  int total = NUM_PIXELS * 3;
  for (int i = 0; i < total; i++) {
    float v = (float)pixBuf[i] * FADE_FACTOR;
    if (v < 0.5f) v = 0.0f;
    pixBuf[i] = (uint8_t)min(255.0f, v);
  }
}

// Copy the soft buffer into Octo drawing buffer (leds.setPixel)
void copyBufferToOcto() {
  for (int curtain = 0; curtain < CURTAINS; curtain++) {
    for (int localIndex = 0; localIndex < LEDS_PER_CURTAIN; localIndex++) {
      int globalIdx = curtain * LEDS_PER_CURTAIN + localIndex;
      int base = globalIdx * 3;
      uint8_t r = pixBuf[base + 0];
      uint8_t g = pixBuf[base + 1];
      uint8_t b = pixBuf[base + 2];
      leds.setPixel(globalIdx, r, g, b);
    }
  }
}

void resetStar(Star &s, bool randomRowAllowed=true) {
  // spawn slightly left of 0 so it slides in smoothly
  s.x = - (random(0, 50) / 25.0f); // -0 .. -2
  if (RANDOM_ROWS && randomRowAllowed)
    s.row = random(0, CURTAIN_HEIGHT);

  s.vx = random((int)(MIN_SPEED_COLS_PER_SEC * 100.0f), (int)(MAX_SPEED_COLS_PER_SEC * 100.0f)) / 100.0f;
  s.bright = random(1, 101) / 100.0f; // 0.7 .. 1.0
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { } // wait for USB serial
  Serial.println("OctoWS - horizontal orange-star transfer across curtains");
  Serial.print("Display: "); Serial.print(TOTAL_WIDTH); Serial.print("x"); Serial.println(TOTAL_HEIGHT);

  // allocate buffers
  pixBuf = (uint8_t*) malloc((size_t)NUM_PIXELS * 3);
  if (!pixBuf) {
    Serial.println("ERROR: not enough RAM for pixBuf");
    while (1) delay(1000);
  }
  memset(pixBuf, 0, (size_t)NUM_PIXELS * 3);

  stars = (Star*) malloc(sizeof(Star) * STAR_COUNT);
  if (!stars) {
    Serial.println("ERROR: not enough RAM for stars array");
    while (1) delay(1000);
  }

  randomSeed(analogRead(A0) ^ micros());
  for (int i = 0; i < STAR_COUNT; i++) {
    resetStar(stars[i], true);
    // spread initial x so they don't all appear at once
    stars[i].x = random(0, TOTAL_WIDTH * 100) / 100.0f;
  }

  leds.begin();
  leds.show(); // clear
  delay(50);
  lastMicros = micros();
}

void loop() {
  unsigned long now = micros();
  float dt = (now - lastMicros) / 1000000.0f;
  if (dt <= 0.0f) dt = 0.001f;
  lastMicros = now;
  if (dt > 0.1f) dt = 0.1f;

  // fade previous frame -> creates trails
  fadeBuffer();

  // update stars (move horizontally only)
  for (int i = 0; i < STAR_COUNT; i++) {
    Star &s = stars[i];
    s.x += s.vx * dt; // vx is columns/sec

    // render only if inside or near visible area
    if (s.x > -2.0f && s.x < TOTAL_WIDTH + 2.0f) {
      renderStarToBuffer(s);
    }

    if (s.x > TOTAL_WIDTH + 1.0f) {
      // off-screen
      if (WRAP_STARS) {
        s.x -= (TOTAL_WIDTH + 2.0f); // wrap left
      } else {
        resetStar(s, true);
      }
    }
  }

  // push to Octo drawing buffer and show
  copyBufferToOcto();
  leds.show();

  // frame pacing to target FRAME_TARGET_MS
  unsigned long frameMicros = micros() - now;
  unsigned long targetMicros = FRAME_TARGET_MS * 1000UL;
  if (frameMicros < targetMicros)
    delay((targetMicros - frameMicros) / 1000);
}
