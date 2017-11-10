/*
 *  Name:     shiftlight
 *  Version:  0.0.1-alpha
 *  
 *  NOTE: This is a WIP/POC. Not functional with rpm signal yet.
 */

#include <Adafruit_NeoPixel.h>

#define PIN 6         // Data pin to LEDs
#define NUM_LEDS 15   // Number of LEDs in strip

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

///////////////////////////////////////////////////////////////////////
// ---------- BEGIN LIGHT BEHAVIOR CONFIGURABLE VARIABLES ---------- //
///////////////////////////////////////////////////////////////////////
/*
 *  Min/max rpm: The values where the light begins
 *  to display (min) and when to shift (max).
 */
#define MIN_RPM 0
#define MAX_RPM 15000

/* 
 *  Mode: How the lights are displayed
 *    0:  Light up individual lights from left to right.
 *        Flashes at MAX_RPM or higher.
 *    
 *    1:  Light up in three groups of lights from left to right.
 *        This mode does not flash. 3rd color lights up at MAX_RPM.
 *        
 *    2:  Light up from the edges toward the center.
 *        Flashes at MAX_RPM or higher.
 */
#define MODE 0

/* 
 *  Number of LEDs for each color (FOR MODES 0/1 ONLY).
 *    Total must equal NUM_LEDS!
 */
#define NUM_FIRST_COLOR 5
#define NUM_SECOND_COLOR 5
#define NUM_THIRD_COLOR 5

/*
 *  How many milliseconds to sleep for flashing.
 *    Essentially the flash speed (lower = faster).
 */
#define FLASH_DELAY 75

/*
 *  RGB values for the three colors (optionally different flash color)
 */
uint32_t first_color = strip.Color(0, 32, 0);   // dim green
uint32_t second_color = strip.Color(32, 0, 0);  // dim red
uint32_t third_color = strip.Color(0, 0, 32);   // dim blue
uint32_t flash_color = third_color;

///////////////////////////////////////////////////////////////////////
// ----------- END LIGHT BEHAVIOR CONFIGURABLE VARIABLES ----------- //
///////////////////////////////////////////////////////////////////////

uint32_t off_color = strip.Color(0, 0, 0);      // no color
uint32_t next_color;

uint16_t rpm = MIN_RPM;     // Temp value for demonstration purposes
uint16_t increment;         // The incremet to light up each LED segment

// These are to divide the three colors for MODE = 2
uint8_t half_leds = NUM_LEDS / 2 + (NUM_LEDS % 2);  // Rounded up
boolean mod3 = half_leds % 3 == 0;
uint8_t first_second_count;
uint8_t third_count;

void setup() {
  switch (MODE) {
  case 0:
    increment = (MAX_RPM - MIN_RPM) / NUM_LEDS;
    break;
  case 1:
    increment = (MAX_RPM - MIN_RPM) / 2;
    break;
  case 2:
    increment = (MAX_RPM - MIN_RPM) / (NUM_LEDS / 2);
    first_second_count = (mod3) ? half_leds / 3 : half_leds / 3 + 1;
    third_count = (mod3) ? first_second_count : half_leds / 3;
    break;
  default:
    return -1; // Invalid mode specified
  }
    
  // Initialize NeoPixel library
  strip.begin();
}

void loop() {
  //getRpm()  TODO: get rpm from vehicle
  
  updateStrip();

///////////////////////////////////////////////////////////////////////
// ---------------- Demonstration stuff (NOT FINAL) ---------------- //
///////////////////////////////////////////////////////////////////////
  delay(100); // Delay for demonstration purposes

  // Used to simulate increasing rpm
  if (rpm >= MAX_RPM) {
    rpm = MIN_RPM;
  } else {
    rpm += 1000;
  }

  // Used to simulate decreasing rpm
//  if (rpm <= MIN_RPM) {
//    rpm = MAX_RPM;
//  } else {
//    rpm -= 1000;
//  }
///////////////////////////////////////////////////////////////////////
// ------------------- End Demonstration stuff  -------------------- //
///////////////////////////////////////////////////////////////////////
}

// Sets the whole LED strip to given color
void setWholeStripColor(uint32_t color) {
  for (int i = 0; i < NUM_LEDS; i++)
    strip.setPixelColor(i, color);

  strip.show();
}

// Flash shift color
void flash() {
  setWholeStripColor(flash_color);
  delay(FLASH_DELAY);
  
  setWholeStripColor(off_color);
  delay(FLASH_DELAY);
}

// Update displayed LEDs based on rpm
void updateStrip() {
  switch (MODE) {
    case 0:
      if (rpm >= MAX_RPM) {
        flash();
      } else {
        for (int i = 0; i <= NUM_LEDS; i++) {
          // If rpm is high enough then turn on next LED(s)
          if (i * increment + MIN_RPM <= rpm) {
            if (i < NUM_FIRST_COLOR) {
              strip.setPixelColor(i, first_color);
            } else if (i < NUM_FIRST_COLOR + NUM_SECOND_COLOR) {
              strip.setPixelColor(i, second_color);
            } else if (i < NUM_FIRST_COLOR + NUM_SECOND_COLOR + NUM_THIRD_COLOR) {
              strip.setPixelColor(i, third_color);
            }
          } else {
            strip.setPixelColor(i, off_color);
          }
        }
      }
      break;
    case 1:
      for (int i = 0; i < 3; i++) {
        if (i * increment + MIN_RPM <= rpm) {
          if (i == 0) {
            next_color = first_color;
          } else if (i == 1) {
            next_color = second_color;
          } else if (i == 2) {
            next_color = third_color;
          }
        } else {
          next_color = off_color;
        }

        for (int j = 0; j < NUM_FIRST_COLOR; j++)
          strip.setPixelColor(i * (NUM_LEDS / 3) + j, next_color);
      }
      break;
    case 2:
      if (rpm >= MAX_RPM) {
        flash();
      } else {
        for (int i = 0; i < half_leds; i++) {
          // If rpm is high enough then turn on next LED(s)
          if (i * increment + MIN_RPM <= rpm) {
            if (i < first_second_count) {
              strip.setPixelColor(i, first_color);
              strip.setPixelColor(NUM_LEDS - 1 - i, first_color);
            } else if (i < first_second_count * 2) {
              strip.setPixelColor(i, second_color);
              strip.setPixelColor(NUM_LEDS - 1 - i, second_color);
            } else if (i < first_second_count * 2 + third_count) {
              // If we're at the last LED(s)...
              if (i + 1 == half_leds) {
                // if two LEDs remain, light them both up
                if (NUM_LEDS % 2 == 0) {
                  strip.setPixelColor(i, third_color);
                  strip.setPixelColor(i + 1, third_color);
                } else {
                  // Otherwise just light the last LED up
                  strip.setPixelColor(i, third_color);
                }
              } else {
                // Otherwise light LEDS up like first two colors
                strip.setPixelColor(i, third_color);
                strip.setPixelColor(NUM_LEDS - 1 - i, third_color);
              }
            }
          } else {  // Otherwise turn LED(s) off
            // If we're at the last LED(s)
            if (i + 1 == half_leds) {
              // if two LEDs remain, turn them both off
              if (NUM_LEDS % 2 == 0) {
                strip.setPixelColor(i, off_color);
                strip.setPixelColor(i + 1, off_color);
              } else {  // Otherwise turn off the last LED
                strip.setPixelColor(i, off_color);
              }
            } else {  // Otherwise turn off two mirrored LEDs
              strip.setPixelColor(i, off_color);
              strip.setPixelColor(NUM_LEDS - 1 - i, off_color);
            }
          }
        }
      }
      break;
    default:
      return -1;
  }

  strip.show();
}
