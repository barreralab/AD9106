
#include "Arduino.h"

#include <SPI.h>
#include "AD9106.h"

// Definitions of static const uint16_t members
const uint16_t AD9106::RAMUPDATE = 0x001d;
const uint16_t AD9106::PAT_STATUS = 0x001e;
const uint16_t AD9106::PAT_TYPE = 0x001f;
const uint16_t AD9106::WAV4_3CONFIG = 0x0026;
const uint16_t AD9106::WAV2_1CONFIG = 0x0027;
const uint16_t AD9106::PAT_PERIOD = 0X0029;
const uint16_t AD9106::DDSTW_MSB = 0x003E;
const uint16_t AD9106::DDSTW_LSB = 0x003F;
const uint16_t AD9106::SAW4_3CONFIG = 0x0036;
const uint16_t AD9106::SAW2_1CONFIG = 0x0037;
const uint16_t AD9106::TRIG_TW_SEL = 0x0044;
const uint16_t AD9106::CFG_ERROR = 0x0060;

AD9106::AD9106(int CS, int RESET, int TRIGGER, int EN_CVDDX, int SHDN)
    : cs(CS),
      reset(RESET),
      _trigger(TRIGGER),
      _en_cvddx(EN_CVDDX),
      _shdn(SHDN) {
  _last_error = NO_ERROR;
}

/**
 * @brief Initialize GPIO and SPI pins on Arduino
 *
 * @param OP_AMPS true iff board configured for on board op amps
 * @param FCLK NULL if default clock used, frequency of external clock if not
 *
 * @return none
 */
void AD9106::begin(bool OP_AMPS, float FCLK) {
  pinMode(cs, OUTPUT);
  pinMode(reset, OUTPUT);
  pinMode(_trigger, OUTPUT);
  pinMode(_en_cvddx, OUTPUT);

  digitalWrite(cs, HIGH);
  digitalWrite(_trigger, HIGH);
  digitalWrite(reset, HIGH);

  if (FCLK == 0) {
    // Enable on-board oscillators and set fclk to crystal oscillator frequency
    digitalWrite(_en_cvddx, HIGH);
    fclk = 156250000;
  } else {
    digitalWrite(_en_cvddx, LOW);
    fclk = FCLK;
  }

  // Set power to op amps if enabled
  if (OP_AMPS) {
    pinMode(_shdn, OUTPUT);
    digitalWrite(_shdn, HIGH);
  }
}

/**
 * @brief Reset AD9106 registers
 * @param none
 * @return none
 */
void AD9106::reg_reset() {
  digitalWrite(reset, LOW);
  delay(1);
  digitalWrite(reset, HIGH);
}

/**
 * @brief Start pattern generation by setting AD9106 trigger pin to 0
 * @param none
 * @return none
 */
void AD9106::start_pattern() {
  digitalWrite(_trigger, LOW);
}

/**
 * @brief Stop pattern generation by setting AD9106 trigger pin to 1
 * @param none
 * @return none
 */
void AD9106::stop_pattern() {
  digitalWrite(_trigger, HIGH);
}

/**
 * @brief Update running pattern by writing register values in shadow set
 * to active set
 * @param none
 * @return none
 */
void AD9106::update_pattern() {
  this->stop_pattern();
  delay(1);
  this->spi_write(RAMUPDATE, 0x0001);   // Trigger RAM Update
  this->spi_write(PAT_STATUS, 0x0001);  // Toggle Run Bit
  delay(1);
  this->start_pattern();

  // this->update_last_error();
}

/**
 * @brief End waveform generation entirely
 * @param none
 * @return none
 */
void AD9106::end() {
  this->stop_pattern();

  // Deactivate oscillator and op-amps
  digitalWrite(_en_cvddx, LOW);
  digitalWrite(_shdn, LOW);
}

/**
 * Sets the property of a DAC channel to a specified value.
 *
 * @param property The property of the DAC channel to set.
 * @param dac The DAC channel to set the property for.
 * @param value The value to set the property to.
 *
 * @return 0 if the property was set successfully, or an error code if an error
 * occurred.
 */
int AD9106::set_CHNL_prop(CHNL_PROP property, CHNL dac, int16_t value) {
  uint16_t dac_addr = this->get_dac_addr(property, dac);
  this->spi_write(dac_addr, value);
  return 0;
}

// Wrapper Functions for set_DAC_prop
int AD9106::set_CHNL_DOFFSET(CHNL chnl, int16_t offset) {
  return this->set_CHNL_prop(DOFFSET, chnl, offset);
}

int AD9106::set_CHNL_DGAIN(CHNL chnl, int16_t gain) {
  return this->set_CHNL_prop(DGAIN, chnl, gain);
}

int AD9106::set_CHNL_DDS_PHASE(CHNL chnl, int16_t phase) {
  return this->set_CHNL_prop(DDS_PHASE, chnl, phase);
}

int AD9106::set_CHNL_START_DELAY(CHNL chnl, int16_t delay) {
  return this->set_CHNL_prop(START_DELAY, chnl, delay);
}

/**
 * Gets the current value of a DAC channel property.
 *
 * @param property The property of the DAC channel to read.
 * @param dac The DAC channel to set the property for.
 *
 * @return reg_data data stored in corresponding register
 */
int16_t AD9106::get_CHNL_prop(CHNL_PROP property, CHNL dac) {
  return this->spi_read(get_dac_addr(property, dac));
}

/**
 * Sets the DDS frequency to specified value
 *
 * @param freq the desired DDS frequency
 *
 * @return none, sets _last_error if freq invalid
 */

void AD9106::setDDSfreq(float freq) {
  if (freq > fclk || freq < 0) {
    _last_error = INVALID_PARAM;
    return;
  }

  // calculate required DDSTW
  const float factor = pow(2, 24) / fclk;
  uint32_t DDSTW = ((uint32_t)round(factor * freq));

  // partition TW bytes into MSB/LSB regs
  int16_t msb = DDSTW >> 8;
  int16_t lsb = (DDSTW & 0xff) << 8;
  this->spi_write(DDSTW_MSB, msb);
  this->spi_write(DDSTW_LSB, lsb);
}

/**
 * Get the current DDS frequency
 *
 * @return DDS frequency
 */

float AD9106::getDDSfreq() {
  // get DDSTW from TW registers
  uint16_t msb = this->spi_read(DDSTW_MSB);
  uint16_t lsb = this->spi_read(DDSTW_LSB);
  uint32_t DDSTW = (msb << 8) | (lsb >> 8);

  // calculate frequency
  float freq = DDSTW * fclk / pow(2, 24);
  return freq;
}

/**
 * Configure registers for prestored DDS output with start delay and patter
 * period on specified channel
 *
 * @param chnl to configure
 * @return none
 */
void AD9106::setDDSsine(CHNL chnl) {
  uint16_t wav_config_addr = WAV2_1CONFIG - (chnl > 2);

  // set wav_config register to DDS output using start_delay and pat_period
  int offset = (chnl - 1) % 2;
  uint16_t mask = 0x00ff << (8 * offset);
  int16_t curr_config = spi_read(wav_config_addr) & ~mask;
  int16_t new_config = curr_config | (0x3232 & mask);
  spi_write(wav_config_addr, new_config);

  // Apply start delay to first pattern only
  spi_write(TRIG_TW_SEL, 0x0002);

  // Set Pattern type to continuous
  spi_write(PAT_TYPE, 0x0000);
}

/*********************************************************/
// SPI FUNCTIONS
/*********************************************************/

/**
 * @brief Initialize AD9106 SPI communication at [hz] speed
 * @param hz - SPI bus frequency in hz
 * @return none
 */
void AD9106::spi_init(uint32_t hz) {
  spi_speed = hz;
  SPI.begin();
}

/**
 * @brief Write 16-bit data to AD9106 SPI/SRAM register
 * @param addr - SPI/SRAM address
 * @param data - data to be written to register address
 * @return none
 */
int16_t AD9106::spi_write(uint16_t addr, int16_t data) {
  int16_t out;

  SPI.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);

  SPI.transfer16(addr);
  out = SPI.transfer16(data);

  digitalWrite(cs, HIGH);
  SPI.endTransaction();
  delay(1);

  return out;
};

/**
 * @brief Read 16-bit data from AD9106 SPI/SRAM register
 * @param addr - SPI/SRAM address
 * @return reg_data - data returned by AD9106
 */
int16_t AD9106::spi_read(uint16_t addr) {
  int16_t out;
  uint16_t read_add;

  SPI.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
  read_add = addr | 0x8000;
  SPI.transfer16(read_add);
  out = SPI.transfer16(0);

  digitalWrite(cs, HIGH);
  SPI.endTransaction();
  delay(1);

  return out;
}

/*********************************************************/
// ERROR HANDLING
/*********************************************************/

/**
 * @brief Check for the highest priority error from the config register and
 * update _last_error field
 * @param none
 * @return none
 */
void AD9106::check_cfg_error() {
  // Error flags in least significant 6 bits
  int16_t err_val = spi_read(CFG_ERROR) & 0x3f;
  if (err_val == 0) {
    _last_error = NO_ERROR;
  } else {
    for (int i = 0; i < 6; i++) {
      if (err_val & (1 << i)) {
        _last_error = static_cast<AD9106::ErrorCode>(i + 1);
        break;
      }
    }
  }
}

/**
 * @brief Get the current system error and update _last_error field
 * @param none
 * @return none
 */
void AD9106::update_last_error() {
  // clear past cfg register error flags and update _last_error
  spi_write(CFG_ERROR, 0x8000);
  this->check_cfg_error();
}