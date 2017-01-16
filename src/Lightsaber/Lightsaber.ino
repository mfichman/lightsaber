/*
 * Copyright (c) 2014 Matt Fichman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, APEXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
 
// This program controls a lightsaber board with an attached LED, speaker, 
// switch, and clash sensor (piezo element). When the switch is open, the 
// Arduino is in low-power sleep mode. Otherwise, the speaker plays the idle
// sound until a vibration is detected on the piezo, when it plays the clash 
// sound. There is also a power-up/power-down sound that's played.
//
// (C) 2015 by Matt Fichman

#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "pcm.h"

// Sound data (stored in program memory). The data in these headers files are 
// auto-generated from WAV-format files using wav2c.
#include "on0.h"
#include "off0.h"
#include "strike1.h"
#include "strike2.h"
#include "idle1.h"

int const SWITCH = 2; // MUST be pin 2 or 3 (for interrupts)
int const LED = 9; // Control pin for the LED
int const SPEAKER = 11; // Speaker output pin. MUST be pin 11 (see PCM.c)
int const PIEZO = A0; // MUST be any analog input

int const PIEZO_THRESHOLD = 30;
int const HIGH_AUDIO_FREQ = 6400;
int const LOW_AUDIO_FREQ = 4000;

int sleepSoundPlaying = 1;

void playOn() {
  startPlayback((uint8_t*)on0_data, on0_length, HIGH_AUDIO_FREQ);
  sleepSoundPlaying = 0;
}

void playOff() {
  startPlayback((uint8_t*)off0_data, off0_length, HIGH_AUDIO_FREQ);
  sleepSoundPlaying = 1;
}

void playClash1() {
  startPlayback((uint8_t*)strike1_data, strike1_length, HIGH_AUDIO_FREQ);
  sleepSoundPlaying = 0;
}

void playClash2() {
  startPlayback((uint8_t*)strike2_data, strike2_length, HIGH_AUDIO_FREQ);
  sleepSoundPlaying = 0;
}

void playIdle() {
  startPlayback((uint8_t*)idle1_data, idle1_length, LOW_AUDIO_FREQ);
  sleepSoundPlaying = 0;
}

void clash() {
  for(int i = 0; i < 2; ++i) {
    delay(50);
    digitalWrite(LED, LOW);
    delay(25);
    digitalWrite(LED, HIGH);
  }
}

void flash() {
  for(int i = 0; i < 3; ++i) {
    digitalWrite(LED, LOW);
    delay(50);
    digitalWrite(LED, HIGH);
    delay(100);
  }
}

void setup() {
  pinMode(SWITCH, INPUT_PULLUP);
  // Avoid using a 10K resistor on the switch by enabling the internal pullup 
  // resistor.
  pinMode(LED, OUTPUT);
  // Set LED control pin for output mode.
  pinMode(SPEAKER, OUTPUT);
  // Set speaker control pin for output mode.
  pinMode(PIEZO, INPUT);

  //digitalWrite(LED, LOW);
  digitalWrite(SPEAKER, LOW);
  // Initialize outputs

  if (digitalRead(SWITCH) == LOW) {
     digitalWrite(LED, HIGH);
     playOn();
     flash();
  }
}

void wake() {
  // Called when the switch is LOW. Do nothing here, since the switch interrupt
  // will bring the Arduino out of sleep mode on its own.
}

void sleep() {
  // Enter sleep mode. Make sure the LED is off, the audio interrupts are 
  // disabled, and the audio output is off.
  digitalWrite(LED, LOW);
  stopPlayback();

  // Disable A-D converter, comparator.
  ADCSRA = ADCSRA & B01111111;

  // Disable the analog comparator by setting the ACD bit (bit 7) of the ACSR 
  // register to one.
  ACSR = B10000000;

  // Disable digital input buffers on all analog input pins by setting bits 0-5 
  // of the DIDR0 register to one.
  DIDR0 = DIDR0 | B00111111;

  // Attach an interrupt so that when the switch is depressed, the Arduino comes 
  // back out of sleep mode.
  attachInterrupt(digitalPinToInterrupt(SWITCH), wake, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_bod_disable();
  sleep_mode();

  ADCSRA = ADCSRA | B10000000;
  ACSR = B00000000;
  DIDR0 = DIDR0 & B11000000;

  // At this point, the Arduino has woken back up again. Re-enable the LED, and 
  // play the powerup sound.
  detachInterrupt(digitalPinToInterrupt(SWITCH));
  digitalWrite(LED, HIGH);
  playOn();
  flash();

  
}

void loop() {
  // Active state. Detect whether we should be playing the sleep/power down sound
  // or clash sound. Otherwise, play the idle sound if the previous sound finished.
  if (digitalRead(SWITCH) == HIGH) {
    if (sleepSoundPlaying) {
      if (donePlayback()) {
        sleep(); // Enter sleep mode when the powerdown sound is done.
      }
    } else {
      playOff();
      flash();
    }
  } else if (analogRead(PIEZO) > PIEZO_THRESHOLD) {
    // Immediately interrupt the playing sound in order to play the clash sound.
    int const r = random(0, 1000);
    if (r < 500) {
      playClash1();
    } else {
      playClash2();
    }
    clash();
    delay(25); // debounce the clash sensor
  } else if (donePlayback()) {
    playIdle();
  } else {
    // Nothing to do: idle sound is currently playing.
  }
}





