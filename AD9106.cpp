#include "Arduino.h"

#include <SPI.h>
#include "AD9106.h"

// Static variable initializations
const uint16_t AD9106::DAC_DOF_BASE = 0x0025;
const uint16_t AD9106::DAC_DGAIN_BASE = 0x0035;
const uint16_t AD9106::DDS_PW_BASE = 0x0043;
const uint16_t AD9106::START_DLY_BASE = 0x005c;

AD9106::AD9106(int CS, int RESET, int TRIGGER, int EN_CVDDX)
    : cs(CS), reset(RESET), _trigger(TRIGGER), _en_cvddx(EN_CVDDX) {}

/*
 * @brief Initialize GPIO and SPI pins on Arduino
 * @param none
 * @return none
 */
void AD9106::begin() {
  pinMode(cs, OUTPUT);
  pinMode(reset, OUTPUT);
  pinMode(_trigger, OUTPUT);
  pinMode(_en_cvddx, OUTPUT);

  digitalWrite(cs, HIGH);
  digitalWrite(_trigger, HIGH);
  digitalWrite(reset, HIGH);

  // Enable on-board oscillators
  digitalWrite(_en_cvddx, HIGH);
}

void AD9106::reg_reset() {
  digitalWrite(reset, LOW);
  delay(10);
  digitalWrite(reset, HIGH);
}

/*
 * @brief Start pattern generation by setting AD9106 trigger pin to 0
 * @param none
 * @return none
 */
void AD9106::start_pattern() {
  digitalWrite(_trigger, LOW);
}

/*
 * @brief Stop pattern generation by setting AD9106 trigger pin to 1
 * @param none
 * @return none
 */
void AD9106::stop_pattern() {
  digitalWrite(_trigger, HIGH);
}

/*
 * @brief Update runnnig pattern by writing register values in shadow set to
 * active set
 * @param none
 * @return none
 */
void AD9106::update_pattern() {
  stop_pattern();
  delay(10);
  spi_write(0x001d, 0x0001);
  delay(10);
  start_pattern();
}

/*
 * @brief Configure registers to generate DDS sourced sine wave on output
 * channel
 * @param channel - the DAC output to generate waves on
 * @param gain - digital gain for DAC output
 * @param offset - digital offset for DAC output
 * @return 1 for success, 0 for failure
 */
int AD9106::set_sine(int channel, uint16_t gain, uint16_t offset) {}

/*********************************************************/
// SPI FUNCTIONS
/*********************************************************/

/*
 * @brief Initialize AD9106 SPI communication at [hz] speed
 * @param hz - SPI bus frequency in hz
 * @return none
 */
void AD9106::spi_init(uint32_t hz) {
  SPI.begin();
  SPI.beginTransaction(SPISettings(hz, MSBFIRST, SPI_MODE0));
}

/*
 * @brief Write 16-bit data to AD9106 SPI/SRAM register
 * @param addr - SPI/SRAM address
 * @param data - data to be written to register address
 * @return none
 */
int16_t AD9106::spi_write(uint16_t addr, int16_t data) {
  int16_t out;

  digitalWrite(cs, LOW);

  SPI.transfer16(addr);
  out = SPI.transfer16(data);

  digitalWrite(cs, HIGH);
  delay(1);

  return out;
};

/*
 * @brief Read 16-bit data from AD9106 SPI/SRAM register
 * @param addr - SPI/SRAM address
 * @return reg_data - data returned by AD9106
 */
int16_t AD9106::spi_read(uint16_t addr) {
  digitalWrite(cs, LOW);

  uint16_t read_add;
  int16_t out;

  read_add = addr | 0x8000;
  SPI.transfer16(read_add);
  out = SPI.transfer16(0);

  digitalWrite(cs, HIGH);
  delay(1);

  return out;
}

/*********************************************************/
// PRIVATE FUNCTIONS
/*********************************************************/

/*
 * @brief Get address for specific DAC based on given base address
 * @param base_addr - constant base address
 * @param dac - the specific dac channel
 * @return dac_addr - the address for dac relative to base
 */
uint16_t AD9106::get_dac_addr(uint16_t base_addr, DAC_CHNL dac) {
  // DAC specific registers after 0x0050 are grouped in 4s
  if (base_addr < 0x0050) {
    return base_addr - (dac - 1);
  }
  return base_addr - 4 * (dac - 1);
}