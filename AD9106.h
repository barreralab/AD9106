/******************************************************************************
    @file:  ad9106.h

    @brief: Library for EVAL-AD9106 DAC arbitrary wave-form generator designed
            to work as header for Arduino UNO R3
******************************************************************************/

#ifndef __ad9106_h__
#define __ad9106_h__

#include "Arduino.h"

class AD9106 {
 public:
  int cs;
  int reset;

  /*** SPI register addresses ***/
  uint16_t reg_add[66] = {
      0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008,
      0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x001f, 0x0020, 0x0022,
      0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b,
      0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034,
      0x0035, 0x0036, 0x0037, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0043,
      0x0044, 0x0045, 0x0047, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055,
      0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e,
      0x005f, 0x001e, 0x001d};

  /*** 4-Wire SPI, Reset, and Trigger configuration & constructor ***/
  AD9106(int CS = 10, int RESET = 8, int TRIGGER = 7, int EN_CVDDX = 2);

  // Function to initialize GPIO pins on Arduino
  void begin();

  // Function to reset register values
  void reg_reset();

  // Function to start pattern generation
  void start_pattern();

  // Function to stop pattern generation
  void stop_pattern();

  // Function to update pattern with new register values
  void update_pattern();

  // Function to configure registers to output DDS sourced sinewave on specified
  // channel
  int set_sine(int channel, uint16_t gain, uint16_t offset);

  // Function to setup SPI with communication speed of [hz]
  void spi_init(uint32_t hz);

  // SPI write function
  int16_t spi_write(uint16_t addr, int16_t data);

  // SPI read function
  int16_t spi_read(uint16_t addr);

  // // Function to display register data
  // void print_data(uint16_t addr, uint16_t data);

  // // Function to write to SRAM
  // void AD910x_update_sram(int16_t data[]);

  // // Function to display n SRAM data
  // void AD910x_print_sram(uint16_t n);

  // // Function to write to device SPI registers and display updated register
  // // values
  // void AD910x_update_regs(uint16_t data[]);

  //   Error Codes
  enum ErrorCode {
    SUCCESS,
    MEM_READ_ERR,
    ODD_ADDR_ERR,
    PERIOD_SHORT_ERR,
    DOUT_START_SHORT_ERR,
    PAT_DLY_SHORT_ERR,
    DOUT_START_LG_ERR
  };

 private:
  int _en_cvddx;
  int _trigger;

  static const uint16_t CFG_ERROR = 0x0060;

  AD9106::ErrorCode check_error() {
    int16_t code = spi_read(CFG_ERROR) & 0x003F;  // Mask for last 6 bits
    if (code & 0x0001)
      return AD9106::MEM_READ_ERR;
    if (code & 0x0002)
      return AD9106::ODD_ADDR_ERR;
    if (code & 0x0004)
      return AD9106::PERIOD_SHORT_ERR;
    if (code & 0x0008)
      return AD9106::DOUT_START_SHORT_ERR;
    if (code & 0x0010)
      return AD9106::PAT_DLY_SHORT_ERR;
    if (code & 0x0020)
      return AD9106::DOUT_START_LG_ERR;
    return AD9106::SUCCESS;
  }
};

#endif
