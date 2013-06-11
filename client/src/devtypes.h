#ifndef _DEVTYPES
#define _DEVTYPES
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "array.c"

enum SEM_COMMANDS {
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
};

uint8_t get_confirmation_byte(enum SEM_COMMANDS c) {
  /* Returns: the first byte after start byte in incoming message from device
     Ugly, ugly function... I'll leave the unused commands at 0 in case someone
     down the line wanted to use them, but with the SEM710 you'll likely only
     use a few of these.
  */
  switch(c) {
  case SEM_COMMANDS_cACK:
    return 0;
  case SEM_COMMANDS_cNAK:
    return 0;
  case SEM_COMMANDS_cREAD_CAL:
    return 0;
  case SEM_COMMANDS_cREAD_CONFIG:
    return 33;
  case SEM_COMMANDS_cREAD_PROCESS:
    return 34;
  case SEM_COMMANDS_cSELF_CAL_0mv:
    return 0;
  case SEM_COMMANDS_cSELF_CAL_50mv:
    return 0;
  case SEM_COMMANDS_cSELF_CAL_100R:
    return 0;
  case SEM_COMMANDS_cSELF_CAL_300R:
    return 0;
  case SEM_COMMANDS_cSELF_CAL_20mA:
    return 0;
  case SEM_COMMANDS_cSELF_CAL_0mA:
    return 0;
  case SEM_COMMANDS_cSELF_CAL_200mV:
    return 0;
  case SEM_COMMANDS_cSELF_CAL_1V:
    return 0;
  case SEM_COMMANDS_cSELF_CAL_10V:
    return 0;
  case SEM_COMMANDS_cSELF_CAL_slide_wire:
    return 0; 
  case SEM_COMMANDS_cPRESET_4ma_COUNT:
    return 0; 
  case SEM_COMMANDS_cPRESET_12ma_COUNT:
    return 0; 
  case SEM_COMMANDS_cPRESET_20ma_COUNT:
    return 0; 
  case SEM_COMMANDS_cPRESET_ENABLE:
    return 0; 
  case SEM_COMMANDS_cSET_CAL:
    return 0; 
  case SEM_COMMANDS_cSET_CONFIG:
    return 0; 
  case SEM_COMMANDS_cREAD_RANGEA:
    return 0; 
  case SEM_COMMANDS_cREAD_RANGEB:
    return 0; 
  case SEM_COMMANDS_cREAD_RANGEC:
    return 0; 
  case SEM_COMMANDS_cREAD_RANGED:
    return 0; 
  case SEM_COMMANDS_cWRITE_RANGEA:
    return 0; 
  case SEM_COMMANDS_cWRITE_RANGEB:
    return 0; 
  case SEM_COMMANDS_cWRITE_RANGEC:
    return 0; 
  case SEM_COMMANDS_cWRITE_RANGED:
    return 0; 
  case SEM_COMMANDS_cidentify:
    return 0; 
  }
  return 0;
}

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
  float ELEC_VALUE;
  float PROCESS_VARIABLE;
  float MA_OUT;
  float CJ_TEMP;
} SEM710_READINGS;

void get_readings(SEM710_READINGS *readings, 
			 uint8_t *byte_array, int array_len)
{
  assert (array_len > 23);
  
  readings->ADC_VALUE = float_from_byte_array(byte_array, 3);
  readings->ELEC_VALUE = float_from_byte_array(byte_array, 7);
  readings->PROCESS_VARIABLE = float_from_byte_array(byte_array, 11);
  readings->MA_OUT = float_from_byte_array(byte_array, 15);
  readings->CJ_TEMP = float_from_byte_array(byte_array, 19);
}

void display_readings(SEM710_READINGS *readings)
{
  printf("ADC_VALUE=%f\n", readings->ADC_VALUE);
  printf("ELEC_VALUE=%f\n", readings->ELEC_VALUE);
  printf("PROCESS_VARIABLE=%f\n", readings->PROCESS_VARIABLE);
  printf("MA_OUT=%f\n", readings->MA_OUT);
  printf("CJ_TEMP=%f\n", readings->CJ_TEMP);
}

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

void get_config_data(CONFIG_DATA *cal, uint8_t *input_array, int input_len)
{
  /* transfers the contents of the given byte array into the CONFIG_DATA */
  assert (input_len > 43);

  cal->tc_code = input_array[3];
  cal->up_scale = input_array[4];
  cal->units = input_array[5];
  cal->model_type = input_array[6];
  cal->vout_range = input_array[7];
  cal->action_A = input_array[8];
  cal->action_B = input_array[9];
  cal->spare = input_array[10];
  cal->low_range = float_from_byte_array(input_array, 11);
  cal->high_range = float_from_byte_array(input_array, 15);
  cal->low_trim = float_from_byte_array(input_array, 19);
  cal->high_trim = float_from_byte_array(input_array, 23);
  cal->setpoint_A = float_from_byte_array(input_array, 27);
  cal->hyst_A = float_from_byte_array(input_array, 31);
  cal->setpoint_B = float_from_byte_array(input_array, 35);
  cal->hyst_B = float_from_byte_array(input_array, 39);
}

void display_config_data(CONFIG_DATA *cal)
{
  printf("tc_code = %d\n", cal->tc_code);
  printf("up_scale = %d\n", cal->up_scale);
  printf("units = %d\n", cal->units);
  printf("model_type = %d\n", cal->model_type);
  printf("vout_range = %d\n", cal->vout_range);
  printf("action_A = %d\n", cal->action_A);
  printf("action_B = %d\n", cal->action_B);
  printf("spare = %d\n", cal->spare);
  printf("low_range = %f\n", cal->low_range);
  printf("high_range = %f\n", cal->high_range);
  printf("low_trim = %f\n", cal->low_trim);
  printf("high_trim = %f\n", cal->high_trim);
  printf("setpoint_A = %f\n", cal->setpoint_A);
  printf("hyst_A = %f\n", cal->hyst_A);
  printf("setpoint_B = %f\n", cal->setpoint_B);
  printf("hyst_b = %f\n", cal->hyst_B);
}

typedef struct {
  uint8_t straight_from_programming; 
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

int CONFIG_BLOCK_init(CONFIG_BLOCK *config_block)
{
  config_block->fp = (float *) malloc((sizeof(float)*48));
  config_block->config_input_float = (float *) malloc(sizeof(float)*4);
  config_block->config_input_byte = (short *) malloc(sizeof(short)*4);
  config_block->title = (char *) malloc(sizeof(char)*16);
  config_block->units = (char *) malloc(sizeof(char)*4);
  config_block->config_output_floats = (float *) malloc(sizeof(float)*8);
  config_block->config_output_int = (short *) malloc(sizeof(short)*4);
  config_block->tag_number = (char *) malloc(sizeof(char)*20);

  if ( /* make sure malloc did its job */
      (config_block->fp == NULL)                   ||
      (config_block->config_input_float == NULL)   ||
      (config_block->config_input_byte == NULL)    ||
      (config_block->title == NULL)                ||
      (config_block->units == NULL)                ||
      (config_block->config_output_floats == NULL) ||
      (config_block->config_output_int == NULL)    ||
      (config_block->tag_number == NULL)           
      ) 
    {
      return -1;
    }
  return 0;
}

void CONFIG_BLOCK_destroy(CONFIG_BLOCK *config_block)
{
  free(config_block->fp);
  free(config_block->config_input_float);
  free(config_block->config_input_byte);
  free(config_block->title);
  free(config_block->units);
}

#endif
