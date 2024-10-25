#include <AD9106.h>

// Initialize AD9106 with CS 10 and default reset, trigger, en_cvddx
AD9106 device(10);

// Toggle OP_AMPS to true/false for on-board amplifiers.
// Set FCLK to 0 to use on-board oscillator, otherwise set to frequency of
// external clock.
const bool OP_AMPS = true;
const float FCLK = 0;

char stop_start = 's';
bool started = false;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  Serial.println("*** Serial Port Ready ***");

  //  Begin AD9106.
  device.begin(OP_AMPS, FCLK);

  // Start SPI communication at 14MHz
  device.spi_init(14000000);

  // Reset AD9106 registers
  device.reg_reset();
  delay(1);
  // Configure sinewaves on all channels
  for (int i = 0; i < 4; i++) {
    device.setDDSsine(CHNL(i + 1));
    device.set_CHNL_DGAIN(CHNL(i + 1), 0x2000);  // Gain of 1/2
  }

  device.set_CHNL_DOFFSET(CHNL_1, 0x1000);
  device.set_CHNL_DDS_PHASE(CHNL_2, 0x4000);
  device.set_CHNL_START_DELAY(CHNL_3, 0x1500);

  //   Set DDS frequency
  device.setDDSfreq(50000);

  // Update pattern to start
  device.update_pattern();
  started = true;
  Serial.println(F("Pattern started. Press 's' to start/stop."));
}

void loop() {
  while (!Serial.available()) {
    ;
  }
  stop_start = Serial.read();
  if (stop_start == 's') {
    started = !started;
    if (started) {
      device.start_pattern();
    } else {
      device.stop_pattern();
    }
  }
}