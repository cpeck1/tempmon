#ifndef __INC_DEVTYPES_H
#define __INC_DEVTYPES_H

#include "cJSON.h"
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEM710_BAUDRATE 19200

typedef enum {
  SEM_COMMANDS_cACK = 0xA,
  SEM_COMMANDS_cNAK = 0xB,
  SEM_COMMANDS_cREAD_CAL = 0x0,
  SEM_COMMANDS_cREAD_CONFIG = 0x1,
  SEM_COMMANDS_cREAD_PROCESS = 0x2,
  SEM_COMMANDS_cSELF_CAL_0mv = 0x3,
  SEM_COMMANDS_cSELF_CAL_50mv = 0x4,
  SEM_COMMANDS_cSELF_CAL_100R = 0x30,
  SEM_COMMANDS_cSELF_CAL_300R = 0x31,
  SEM_COMMANDS_cSELF_CAL_20mA = 0x32,
  SEM_COMMANDS_cSELF_CAL_0mA = 0x33,
  SEM_COMMANDS_cSELF_CAL_200mV = 0x34,
  SEM_COMMANDS_cSELF_CAL_1V = 0x35,
  SEM_COMMANDS_cSELF_CAL_10V = 0x36,
  SEM_COMMANDS_cSELF_CAL_slide_wire = 0x37,
  SEM_COMMANDS_cPRESET_4ma_COUNT = 0x5,
  SEM_COMMANDS_cPRESET_12ma_COUNT = 0x6,
  SEM_COMMANDS_cPRESET_20ma_COUNT = 0x7,
  SEM_COMMANDS_cPRESET_ENABLE = 0x8,
  SEM_COMMANDS_cSET_CAL = 0x10,
  SEM_COMMANDS_cSET_CONFIG = 0x11,
  SEM_COMMANDS_cREAD_RANGEA = 0x50,
  SEM_COMMANDS_cREAD_RANGEB = 0x51,
  SEM_COMMANDS_cREAD_RANGEC = 0x52,
  SEM_COMMANDS_cREAD_RANGED = 0x53,
  SEM_COMMANDS_cWRITE_RANGEA = 0x40,
  SEM_COMMANDS_cWRITE_RANGEB = 0x41,
  SEM_COMMANDS_cWRITE_RANGEC = 0x42,
  SEM_COMMANDS_cWRITE_RANGED = 0x43,
  SEM_COMMANDS_cidentify = 0x60
} SEM_COMMANDS;



typedef enum {
  WAITING_FOR_START,
  WAITING_FOR_REPLY_START,
  WAITING_FOR_COMMAND,
  WAITING_FOR_LEN,
  WAITING_FOR_DATA,
  WAITING_FOR_CRC_LO,
  WAITING_FOR_CRC_HI,
  WAITING_FOR_END,
  RX_OVERFLOW
} RX_FRAMING;

typedef enum {
WAITING,
  GET_FUNCTION,
  GET_CODE,
  start_address,
  NO_BYTES,
  GET_BYTES,
  GET_CRC_LOW,
  GET_CRC_HIGH,
  ENGAGED,
  MESSAGE_NOT_VALID
} COMMS_RX_STATE;

typedef struct {
  float ADC_VALUE;
  char *STATUS;
  /* float ELEC_VALUE; */
  /* float PROCESS_VARIABLE; */
  /* float MA_OUT; */
  /* float CJ_TEMP; */
} SEM710_READINGS;

typedef struct {
  uint8_t tc_code;
  uint8_t up_scale;

  uint8_t units;
  uint8_t model_type;
  uint8_t vout_range;

  uint8_t action_A;
  uint8_t action_B;
  uint8_t spare;
  float low_range;
  float high_range;
  float low_trim;
  float high_trim;

  float setpoint_A;
  float hyst_A;

  float setpoint_B;
  float hyst_B;

  char Serial_number[16];
  char comment[256];
} CONFIG_DATA;

typedef struct {
uint8_t straight_from_programming;/* always 0, used on first power up */

  uint8_t dummy;
  float lo_mv;
  float hi_mv;
  float lo_ma;
  float hi_ma;
  float lo_rtd;
  float hi_rtd;
  float lo_ma_in;
  float hi_ma_in;
  float hi_200mv_in;
  float hi_1v_in;
  float hi_10v_in;
  float hi_cal_slide_wire;
  float hi_voltage_output;
} UNIVERSAL_CALIBRATION;


typedef struct {
   float *fp;
   float *config_input_float;
   short *config_input_byte;
   char *title;
   char *units;
   float *config_output_floats;
   short *config_output_int;
   char *tag_number;
} CONFIG_BLOCK;

uint8_t get_confirmation_byte(SEM_COMMANDS c);

char *get_device_read_status(uint8_t *byte_array, float temp_reading,
			     float temp_exp, float temp_range);

void get_readings(SEM710_READINGS *readings, float temp_exp, float temp_range,
		  uint8_t *byte_array, int array_len);

void display_readings(SEM710_READINGS *readings);

void pack_readings(SEM710_READINGS *readings, char *filename);

void get_config(CONFIG_DATA *cal, uint8_t *input_array, int array_len);

void display_config(CONFIG_DATA *cal);

void CONFIG_BLOCK_init(CONFIG_BLOCK *config_block);

void CONFIG_BLOCK_destroy(CONFIG_BLOCK *config_block);

#ifdef __cplusplus
}
#endif

#endif /* __INC_DEVTYPES_H*/
