// OctoWS2811 - Independent effects per output (5 curtains)
// Change: LEDS_PER_STRIP, NUM_STRIPS_USED, and stripConfig[] to fit your setup.
// If using Teensy 4.x with custom pins, enable USE_PINLIST and fill pinList[].

#include <OctoWS2811.h>

// ---------------- USER CONFIG ----------------
const int NUM_STRIPS_USED = 5;     // you said 5 curtains
const int LEDS_PER_STRIP   = 520;  // <-- change to your curtain height

// If you use Teensy 4.x and want a custom pin mapping, set to true and edit pinList.
// If false, the library uses default Octo pins (good with PJRC Octo adapter and Teensy 3.x).
const bool USE_PINLIST = false;
const byte pinListExample[NUM_STRIPS_USED] = { 2, 14, 7, 8, 6 }; // example pin mapping (change if needed)
// ------------------------------------------------

// --- OctoWS memory (do NOT change unless you know what you're doing) ---
DMAMEM int displayMemory[LEDS_PER_STRIP * 6];
int drawingMemory[LEDS_PER_STRIP * 6];
const int config = WS2811_GRB | WS2811_800kHz;

#if USE_PINLIST
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config, NUM_STRIPS_USED, (byte*)pinListExample);
#else
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config);
#endif

// ---------------- Effect system ----------------
enum Effect {
  EF_OFF = 0,
  EF_SOLID,
  EF_MOVING_DOT,
  EF_RAINBOW,
  EF_THEATER_CHASE,
  EF_SPARKLE,
  EF_GRADIENT_WIPE,
  EF_COUNT
};

struct StripConfig {
  Effect effect;
  uint8_t r1, g1, b1; // primary color
  uint8_t r2, g2, b2; // secondary color / accent
  uint16_t speed;     // lower = faster (ms per step)
};

// per-strip configuration: set desired effect and colors here
StripConfig stripConfig[NUM_STRIPS_USED] = {
  { EF_RAINBOW,       255,0,0,   0,0,0,   20 },  // strip 0
  { EF_MOVING_DOT,    0,255,0,   0,0,0,   10 },  // strip 1
  { EF_THEATER_CHASE, 0,0,255,   60,60,60, 60 },  // strip 2
  { EF_SPARKLE,       255,200,0, 0,0,0,   30 },  // strip 3
  { EF_GRADIENT_WIPE, 255,0,255, 0,0,0,   25 }   // strip 4
};

// per-strip runtime state
struct StripState {
  uint32_t lastUpdateMs;
  uint16_t pos;       // generic position counter
  uint8_t  seed;      // for sparkle randomness
} states[NUM_STRIPS_USED];

// utility: color wheel (0..255) -> 24-bit RGB
uint32_t colorWheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) return ((255 - pos * 3) << 16) | (0 << 8) | (pos * 3);
  if (pos < 170) { pos -= 85; return ((0) << 16) | ((pos*3) << 8) | (255 - pos*3); }
  pos -= 170; return ((pos*3) << 16) | ((255 - pos*3) << 8) | 0;
}

inline void setStripPixelColor(int stripIndex, int pixelIndex, uint8_t r, uint8_t g, uint8_t b) {
  if (stripIndex < 0 || stripIndex >= NUM_STRIPS_USED || pixelIndex < 0 || pixelIndex >= LEDS_PER_STRIP) return;
  int globalIndex = stripIndex * LEDS_PER_STRIP + pixelIndex;
  leds.setPixel(globalIndex, r, g, b);
}

inline void clearStrip(int s) {
  for (int i=0;i<LEDS_PER_STRIP;i++) setStripPixelColor(s,i,0,0,0);
}

// EFFECT IMPLEMENTATIONS:
// Each function fills only pixels for strip `s`. Use stripConfig[s] for parameters and states[s] for runtime.

void effect_off(int s) {
  clearStrip(s);
}

void effect_solid(int s) {
  auto &cfg = stripConfig[s];
  for (int i=0;i<LEDS_PER_STRIP;i++) setStripPixelColor(s,i,cfg.r1,cfg.g1,cfg.b1);
}

void effect_moving_dot(int s) {
  auto &cfg = stripConfig[s];
  StripState &st = states[s];
  // move 1 pixel down the curtain
  int pos = st.pos % LEDS_PER_STRIP;
  clearStrip(s);
  // tail pixels for nicer look
  int tail = 4;
  for (int i=0;i<tail;i++) {
    int p = pos - i;
    if (p < 0) p += LEDS_PER_STRIP; // wrap
    uint8_t scale = 255 - (i * (255 / (tail+1)));
    setStripPixelColor(s, p, (cfg.r1 * scale) >> 8, (cfg.g1 * scale) >> 8, (cfg.b1 * scale) >> 8);
  }
  // advance position handled by caller timing
}

void effect_rainbow(int s) {
  // scrolling rainbow down the curtain
  StripState &st = states[s];
  for (int i=0;i<LEDS_PER_STRIP;i++) {
    uint8_t wheelPos = (uint8_t)((i + st.pos) & 255);
    uint32_t col = colorWheel(wheelPos);
    setStripPixelColor(s, i, (col>>16)&0xFF, (col>>8)&0xFF, col&0xFF);
  }
}

void effect_theater_chase(int s) {
  auto &cfg = stripConfig[s];
  StripState &st = states[s];
  // simple theater chase: on/off pattern moving down
  int q = st.pos % 3;
  for (int i=0;i<LEDS_PER_STRIP;i++) {
    if ((i + q) % 3 == 0) setStripPixelColor(s,i,cfg.r1,cfg.g1,cfg.b1);
    else setStripPixelColor(s,i,0,0,0);
  }
}

void effect_sparkle(int s) {
  auto &cfg = stripConfig[s];
  StripState &st = states[s];
  // mostly background color dim, with random sparkles
  uint8_t bgdim = 20;
  for (int i=0;i<LEDS_PER_STRIP;i++) setStripPixelColor(s,i, (cfg.r1*bgdim)>>8, (cfg.g1*bgdim)>>8, (cfg.b1*bgdim)>>8 );
  // create a few sparkles based on seed
  uint8_t sparks = 3;
  uint16_t seed = (st.seed << 8) | (st.pos & 0xFF);
  for (int k=0;k<sparks;k++) {
    // simple LCG
    seed = (uint16_t)(seed * 109 + 89);
    int p = seed % LEDS_PER_STRIP;
    // brighter pixel
    setStripPixelColor(s, p, cfg.r1, cfg.g1, cfg.b1);
  }
}

void effect_gradient_wipe(int s) {
  auto &cfg = stripConfig[s];
  StripState &st = states[s];
  int limit = st.pos % (LEDS_PER_STRIP + 1);
  for (int i=0;i<LEDS_PER_STRIP;i++) {
    if (i < limit) {
      // blend r1..r2 across filled region
      float t = (limit <= 1) ? 0.0f : float(i) / float(max(1, limit-1));
      uint8_t r = uint8_t((1.0f - t) * cfg.r2 + t * cfg.r1);
      uint8_t g = uint8_t((1.0f - t) * cfg.g2 + t * cfg.g1);
      uint8_t b = uint8_t((1.0f - t) * cfg.b2 + t * cfg.b1);
      setStripPixelColor(s, i, r,g,b);
    } else {
      setStripPixelColor(s, i, 0,0,0);
    }
  }
}

// update dispatch
void updateStripFrame(int s) {
  switch (stripConfig[s].effect) {
    case EF_OFF: effect_off(s); break;
    case EF_SOLID: effect_solid(s); break;
    case EF_MOVING_DOT: effect_moving_dot(s); break;
    case EF_RAINBOW: effect_rainbow(s); break;
    case EF_THEATER_CHASE: effect_theater_chase(s); break;
    case EF_SPARKLE: effect_sparkle(s); break;
    case EF_GRADIENT_WIPE: effect_gradient_wipe(s); break;
    default: effect_off(s); break;
  }
}

// ---------------- setup / loop ----------------
void setup() {
  Serial.begin(115200);
  while (!Serial) { } // wait for serial (Teensy)
  Serial.println("OctoWS2811 - per-strip effects example (5 curtains)");
  #if USE_PINLIST
    Serial.println("Using custom pinList");
  #else
    Serial.println("Using default Octo pins (PJRC Octo adapter or Teensy default mapping)");
  #endif

  leds.begin();
  leds.show(); // blank initially
  delay(50);

  // init states
  for (int s=0;s<NUM_STRIPS_USED;s++) {
    states[s].lastUpdateMs = millis();
    states[s].pos = s * 10; // stagger initial positions
    states[s].seed = (uint8_t)(s * 73 + 37);
  }
}

void loop() {
  uint32_t now = millis();

  // For each strip, decide whether it's time to advance that strip's state,
  // and then render only that strip (keeps CPU work spread across frames).
  for (int s=0; s<NUM_STRIPS_USED; s++) {
    StripConfig &cfg = stripConfig[s];
    StripState  &st  = states[s];

    // clamp speed: if 0, treat as immediate each loop
    uint32_t period = max(1, cfg.speed);

    if ((uint32_t)(now - st.lastUpdateMs) >= period) {
      // advance logical position used by many effects
      st.pos++;
      st.seed ^= (uint8_t)(st.pos & 0xFF); // change seed slowly for sparkle
      st.lastUpdateMs = now;

      // render this strip to the drawing buffer
      updateStripFrame(s);
    }
  }

  // push buffer to LEDs (this triggers the DMA transfer for all strips)
  leds.show();

  // Optional: small yield to let USB serial etc. run; tiny delay reduces CPU hogging
  delay(1);
}
