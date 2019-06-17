/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "application.h"
#line 1 "g:/dev/PorchLights/src/PorchLights.ino"
// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>

// IMPORTANT: Set pixel COUNT, PIN and TYPE
uint32_t CRGB(uint8_t r, uint8_t g, uint8_t b);
void load_config();
void save_config();
void setup();
void fill(uint32_t c);
int set_color(String color);
int set_on_time(String on_time);
int set_off_time(String off_time);
int set_brightness(String bright);
void loop();
#line 5 "g:/dev/PorchLights/src/PorchLights.ino"
#define PIXEL_PIN D0
#define PIXEL_COUNT 10
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// neopixel library is dumb and doesn't let you change color order
uint32_t CRGB(uint8_t r, uint8_t g, uint8_t b) { return strip.Color(g, r, b); }

#define C_RED   CRGB(255, 0, 0)
#define C_GREEN CRGB(0, 255, 0)
#define C_BLUE  CRGB(0, 0, 255)
#define C_OFF   CRGB(0, 0, 0)
#define C_BLACK C_OFF


struct DisplayConfig {
    int       check;
    int    tz;
    uint32_t  color;
    int   brightness;
    uint8_t   on_hr;
    uint8_t   on_min;
    uint8_t   off_hr;
    uint8_t   off_min;
};

DisplayConfig cfg;

String _on_time;
String _off_time;
String _cur_time;
bool _is_on = true;

#define CFG_CHECK 7

void load_config() {
    EEPROM.get(0, cfg);
    if(cfg.check != CFG_CHECK) {
        cfg.check = CFG_CHECK;
        cfg.tz = -4;
        cfg.color = C_BLUE;
        cfg.brightness = 64;
        cfg.on_hr = 19;
        cfg.on_min = 0;
        cfg.off_hr = 6;
        cfg.off_min = 0;

        save_config();
    }

    _on_time = String::format("%02d:%02d", cfg.on_hr, cfg.on_min);
    _off_time = String::format("%02d:%02d", cfg.off_hr, cfg.off_min);
}

void save_config() {
    EEPROM.put(0, cfg);
}

void setup()
{
    load_config();

    Time.zone(cfg.tz);

    Particle.function("color", set_color);
    Particle.function("on_time", set_on_time);
    Particle.function("off_time", set_off_time);
    Particle.function("brightness", set_brightness);

    Particle.variable("tz", cfg.tz);
    Particle.variable("color", cfg.color);
    Particle.variable("brightness", cfg.brightness);
    Particle.variable("on_time", _on_time);
    Particle.variable("off_time", _off_time);

    strip.begin();
    strip.setBrightness(64);
    strip.show(); // Initialize all pixels to 'off'
}

void fill(uint32_t c){
    static uint16_t i;
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
    }
}

int set_color(String color) {
    Particle.publish("s_color", color, PRIVATE);
    long int rgb=strtol(color.c_str(),0,16);
    uint8_t r=(uint8_t)(rgb>>16);
    uint8_t g=(uint8_t)(rgb>>8);
    uint8_t b=(uint8_t)(rgb);

    cfg.color = CRGB(r, g, b);

    save_config();

    return 1;
}

int set_on_time(String on_time) {
    static int h, m;
    static long t;
    if(on_time.length() != 4){ return 0; }
    t = strtol(on_time.c_str(), 0, 10);
    m = t % 100;
    h = t / 100;

    if(m >= 60 || h >= 24) return -1;
    cfg.on_hr = h;
    cfg.on_min = m;

    save_config();
    load_config(); // just to reload some strings
    Particle.publish("on_time", _on_time, PRIVATE);

    return 1;
}

int set_off_time(String off_time) {
    static int h, m;
    static long t;
    if(off_time.length() != 4){ return 0; }
    t = strtol(off_time.c_str(), 0, 10);
    m = t % 100;
    h = t / 100;

    if(m >= 60 || h >= 24) return 0;
    cfg.off_hr = h;
    cfg.off_min = m;

    save_config();
    load_config(); // just to reload some strings
    Particle.publish("off_time", _off_time, PRIVATE);

    return 1;
}

int set_brightness(String bright) {
    cfg.brightness = bright.toInt();
    strip.setBrightness(cfg.brightness);
    save_config();

    return 1;
}

void loop()
{
  if(_is_on) {
    fill(cfg.color);
  }
  else
  {
    fill(C_OFF);
  }

  strip.show();
  delay(250);

  static int cur_min, on_min, off_min;

  if(Time.now() % 15 == 0){
      cur_min = (Time.hour() * 60) + Time.minute();
      on_min = (cfg.on_hr * 60) + cfg.on_min;
      off_min = (cfg.off_hr * 60) + cfg.off_min;

      if(off_min < on_min){
        _is_on = !(cur_min >= off_min && cur_min < on_min);
      }
      else {
        _is_on = (cur_min >= on_min && cur_min < off_min);
      }
  }
}
