#define LAYOUT_GERMAN
#include "DigiKeyboard.h"
#include <Adafruit_NeoPixel.h>

#define DEBOUNCE_COUNT     3
#define WS2812_PIN         PB0
#define WS2812_COUNT       3
#define WS2812_BRIGHTNESS  20
#define WS2812_SATURATION  220
#define HUE_PER_STEP       64

#define KEY_AUML 0x34  // ä
#define KEY_OUML 0x33  // ö
#define KEY_UUML 0x2F  // ü
#define KEY_SZLIG 0x2D // ß

/****************
 * Short Manual *
 ****************

   LED an- oder ausschalten
      digitalWrite(PB0, LOW); // aus
      digitalWrite(PB0, HIGH); // an
   Pause for x milliseconds, 1000ms = 1sec
      DigiKeyboard.delay(5000); // 5 seconds
   Type text:
      DigiKeyboard.print("Text");
   Press [Enter] (can also be included into text):
      DigiKeyboard.print("\n");
   Press a single key:
      DigiKeyboard.sendKeyStroke(KEY_ENTER, 0);
   Instead of 0 you can also set one or more modifiers:
      DigiKeyboard.sendKeyStroke(KEY_F, 0); // kleines f
      DigiKeyboard.sendKeyStroke(KEY_F, MODIFIERKEY_LEFT_SHIFT); // großes F
   These are the modifiers:
      0                           = none
      MODIFIERKEY_LEFT_SHIFT      = left Shift
      MODIFIERKEY_LEFT_CTRL       = left Ctrl
      MODIFIERKEY_LEFT_ALT        = left Alt
      MODIFIERKEY_LEFT_GUI        = left Windows
   Here are the most common keys:
      KEY_NOKEY                   = No key (e.g. to press a modifier key alone)
      KEY_SYSTEM_POWER_DOWN       = Power key
      KEY_MEDIA_PLAY              = Media-Control Play
      KEY_MEDIA_PAUSE             = Media-Control Pause
      KEY_MEDIA_STOP              = Media-Control Stop
      KEY_MEDIA_PLAY_PAUSE        = Media-Control Play/Pause
      KEY_MEDIA_MUTE              = Media-Control Volume mute
      KEY_MEDIA_VOLUME_INC        = Media-Control Volume+
      KEY_MEDIA_VOLUME_DEC        = Media-Control Volume-
      KEY_ENTER                   = Enter
      KEY_ESC                     = Escape
      KEY_BACKSPACE               = Backspace
      KEY_TAB                     = Tabulator
      KEY_SPACE                   = Space
      KEY_A to KEY_Z              = Letters
      KEY_0 to KEY_9              = Digits
      KEY_F1 bis KEY_F12          = Function keys
      KEY_HOME                    = Home
      KEY_END                     = End
      KEY_UP                      = Arrow key up
      KEY_DOWN                    = Arrow key down
      KEY_LEFT                    = Arrow key left
      KEY_RIGHT                   = Arrow key right
*/


// This stores if we're currently waiting for all keys to be let go
bool _wait = true;
// This stores the last read key code
int _btnValue = 0;
// This stores the debounce counter
uint8_t _debounce = 0;
// This stores the current hue value
uint32_t _hueValue = 0;
// This stores the current counter to adjust the huew
uint8_t _hueCounter = 0;


Adafruit_NeoPixel WS2812B(WS2812_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// This is called when a button has been pressed (debounced)
void btnTriggerAction(int btn) {
  switch (btn) {
    case 1: // Button 1
      // Press the [Windows] key to open the start menu
      DigiKeyboard.sendKeyStroke(KEY_NOKEY, MODIFIERKEY_LEFT_GUI);
      // Wait a little to let the menu open
      DigiKeyboard.delay(500);
      // Type "Note" and press [Enter]
      DigiKeyboard.print("Note\n");
      // Wait for 3 seconds to let hopefully notepad open
      DigiKeyboard.delay(3000);
      // Type a little message
      DigiKeyboard.print("Hallo,\n\ndies ist eine kleine Nachricht an dich.\nViel Freude mit der Macro-Tastatur!\n");
      break;

    case 2: // Button 2
      // Press [Ctrl]+[T] to open console
      DigiKeyboard.sendKeyStroke(KEY_T, MODIFIERKEY_LEFT_CTRL);
      DigiKeyboard.delay(500);
      // Type command and press [Enter]
      DigiKeyboard.print("/home\n");
      break;

    case 3: // Button 1 and 2 simultaneously
      // Press [Alt]+[F4]
      DigiKeyboard.sendKeyStroke(KEY_F4, MODIFIERKEY_LEFT_ALT);
      break;

    default: // Combination of keys
      // Type the pressed key combination value
      DigiKeyboard.print("Key value ");
      DigiKeyboard.print(btn);
      DigiKeyboard.print("\n");
      break;
  }
}


void updateHue() {
  // Rotate the 3 LEDs through the hue circle but with offset to each other
  WS2812B.setPixelColor(0, Adafruit_NeoPixel::ColorHSV( _hueValue          & 0xFFFF, WS2812_SATURATION, WS2812_BRIGHTNESS));
  WS2812B.setPixelColor(1, Adafruit_NeoPixel::ColorHSV((_hueValue + 21845) & 0xFFFF, WS2812_SATURATION, WS2812_BRIGHTNESS));
  WS2812B.setPixelColor(2, Adafruit_NeoPixel::ColorHSV((_hueValue + 43690) & 0xFFFF, WS2812_SATURATION, WS2812_BRIGHTNESS));
  WS2812B.show();

  // Increade the hue value for next time with roll-over
  _hueValue = (_hueValue + HUE_PER_STEP) & 0xFFFF;
}


// Called to wait until no key is pressed for long enough
void btnWaitForRelease(int btn) {
  if (btn == 0) {
    // All buttons released -> increase counter
    _debounce++;
  } else { 
    // Something still pressed -> reset counter
    _debounce = 0;
  }
  
  if (_debounce > DEBOUNCE_COUNT) { 
    // No key pressed for long enough -> switch to wait for stable key combination
    _btnValue = 0;
    _debounce = 0;
    _wait = false;
  }
}

// Called when waiting for a key combination being stable long enough
void btnWaitForCombination(int btn) {
  if (btn == 0 || btn != _btnValue) {
    // Key value gone or changed -> reset counter
    _btnValue = btn;
    _debounce = 0;
  } else {
    // Key value still the same -> increase counter
    _debounce++;
  }
  
  if (_debounce > DEBOUNCE_COUNT) {
    // Key value stable for long enough -> trigger action and switch to waiting
    btnTriggerAction(_btnValue);
    _debounce = 0;
    _btnValue = 0;
    _wait = true;
  }
}


// Called oned on start-up
void setup() {
  pinMode(PB0, OUTPUT);
  pinMode(PB1, INPUT_PULLUP);
  pinMode(PB2, INPUT_PULLUP);
  // PB3 and PB4 are used for USB
  pinMode(PB5, INPUT_PULLUP);
}


// This is calles over and over again
void loop() {
  // Read the current push button states and store them as a single bit-wise value
  int btn = 0;
  if (digitalRead(PB1) == LOW) btn |= 1;
  if (digitalRead(PB2) == LOW) btn |= 2;
  if (analogRead(0) < 1000) btn |= 4;    // PB5 is analog 0

  if (_wait) {
    // Currently waiting for all buttons to be released
    btnWaitForRelease(btn);
  } else {
    // Waiting for a button combination being pressed for a long enough time and
    // then trigger the action for it
    btnWaitForCombination(btn);
  }

  // Adjust RGB color every so often
  if (_hueCounter > 3) {
    updateHue();
    // Reset the counter
    _hueCounter = 0;
  }

  // Wait for a short time to let the USB part do its thing
  DigiKeyboard.delay(10);
}
