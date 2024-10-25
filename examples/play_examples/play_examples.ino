/*******************************************************************************
    @file:   play_examples.ino

    @brief:  Play AD9106 example programs from datasheet

    @note:   Use no-line ending in Arduino IDE. Tested on Arduino UNO R3.
*******************************************************************************/

#include <AD9106.h>
#include "config.h"

// user controls
char ext_clk = 'n';
char amp_out = 'n';
char stop = 'n';
char exit_prog = 'n';
char example = '3';  // Default to example 3
bool connected = true;
const int AD9106_CS = 10;

// Initialize AD9106 instance with CS 10 and default reset, trigger, en_cvddx
AD9106 device(AD9106_CS);
const float FCLK = 0;
const float SPI_SPEED = 14000000;
bool OP_AMPS;

void setup() {
  Serial.begin(9600);
  print_title();

  while (connected) {
    print_prompt1();
    while (!Serial.available()) {
      ;
    }
    ext_clk = Serial.read();
    if (ext_clk == 'y') {
      Serial.println(F("Please connect external clock source and update FCLK"));
    } else {
      Serial.println(F("On-board oscillator supply is enabled."));
    }
    delay(100);
    setup_device();
    print_prompt2();
    while (!Serial.available()) {
      ;
    }
    amp_out = Serial.read();
    if (amp_out == 'y') {
      Serial.println(F("On-board amplifier supply is enabled."));
      OP_AMPS = true;
    } else {
      Serial.println(F("Amplifier is disabled."));
      OP_AMPS = false;
    }
    print_menu();
    while (!Serial.available()) {
      ;
    }
    example = Serial.read();
    Serial.print("Selected example: ");
    Serial.println(example);

    switch (example) {
      case '3':
        play_example3();
        break;
      case '4':
        play_example4();
        break;
      case '6':
        play_example6();
        break;
      default:
        Serial.println(F("****Invalid Entry****"));
        break;
    }

    print_prompt3();
    while (!Serial.available()) {
      ;
    }
    stop = Serial.read();
    if (stop == 'y') {
      stop_example();
    }

    print_prompt4();
    while (!Serial.available()) {
      ;
    }
    exit_prog = Serial.read();
    if (exit_prog == 'y') {
      connected = false;
      Serial.println(F("Exiting program..."));
    } else {
      stop = 'n';
    }
  }
}

void loop() {
  // Empty loop
}

void setup_device() {
  device.begin(OP_AMPS, FCLK);
  device.reg_reset();
  device.spi_init(SPI_SPEED);
}

void print_title() {
  Serial.println(
      F("**********************************************************************"
        "*"));
  Serial.println(F(
      "* AD910x Demonstration Program                                         "
      "*"));
  Serial.println(F(
      "* Demonstrates waveform generation with AD910x using example setups.   "
      "*"));
  Serial.println(
      F("**********************************************************************"
        "*"));
}

void print_prompt1() {
  Serial.println(F("Using external clock source? (y/n)"));
}

void print_prompt2() {
  Serial.println(F("Connected DAC outputs to on-board amplifiers? (y/n)"));
}

void print_menu() {
  Serial.println(F("Example Summary:"));
  Serial.println(F(
      "   3 - 4 Pulsed DDS-Generated Sine Waves with Different Start Delays"));
  Serial.println(
      F("   4 - Pulsed DDS-Generated Sine Wave and 3 Sawtooth Generator "
        "Waveforms"));
  Serial.println(F("   6 - DDS-Generated Sine Wave and 3 Sawtooth Waveforms"));
  Serial.println(F("Select an example: "));
}

void print_prompt3() {
  Serial.println(F("Stop pattern? (y/n)"));
}

void print_prompt4() {
  Serial.println(F("Exit program? (y/n)"));
}

void play_example3() {
  Serial.println(F("Playing Example 3..."));
  for (int i = 0; i < NUMREGS; i++) {
    device.spi_write(device.reg_add[i], AD9106_example3_regval[i]);
  }
  device.update_pattern();
}

void play_example4() {
  Serial.println(F("Playing Example 4..."));
  for (int i = 0; i < NUMREGS; i++) {
    device.spi_write(device.reg_add[i], AD9106_example4_regval[i]);
  }
  device.update_pattern();
}

void play_example6() {
  Serial.println(F("Playing Example 6..."));
  for (int i = 0; i < NUMREGS; i++) {
    device.spi_write(device.reg_add[i], AD9106_example6_regval[i]);
  }
  device.update_pattern();
}

void stop_example() {
  Serial.println(F("Pattern stopped."));
  device.stop_pattern();
}