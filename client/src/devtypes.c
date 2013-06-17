#include "devtypes.h"
#include <stdio.h>

void SEM710_display_readings(SEM710_READINGS *readings)
{
  printf("ADC_VALUE=%f\n", readings->ADC_VALUE);
  printf("ELEC_VALUE=%f\n", readings->ELEC_VALUE);
  printf("PROCESS_VARIABLE=%f\n", readings->PROCESS_VARIABLE);
  printf("MA_OUT=%f\n", readings->MA_OUT);
  printf("CJ_TEMP=%f\n", readings->CJ_TEMP);
}

void CONFIG_DATA_init(CONFIG_DATA *block)
{
  block->tc_code = 8;
  block->up_scale = 1;
  block->units = 0;
  block->model_type = 0;
  block->vout_range = 0;

  block->action_A = 1;
  block->action_B = 1;
  block->spare = 0;
  block->low_range = 0.0;
  block->high_range = 100.0;
  block->low_trim = 0.0;
  block->high_trim = 0.0;

  block->setpoint_A = 100.0;
  block->hyst_A = 0.1;

  block->setpoint_B = 100.0;
  block->hyst_B = 0.1;
}

void array_to_CONFIG_DATA(CONFIG_DATA *cal, uint8_t *input_array)
{
  // transfers the contents of the given byte array into the CONFIG_DATA

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

void display_CONFIG_DATA(CONFIG_DATA *cal)
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


void CONFIG_BLOCK_init(CONFIG_BLOCK *config_block)
{
  config_block->fp = (float *) malloc((sizeof(float)*48));
  config_block->config_input_float = (float *) malloc(sizeof(float)*4);
  config_block->config_input_byte = (short *) malloc(sizeof(short)*4);
  config_block->title = (char *) malloc(sizeof(char)*16);
  config_block->units = (char *) malloc(sizeof(char)*4);
  config_block->config_output_floats = (float *) malloc(sizeof(float)*8);
  config_block->config_output_int = (short *) malloc(sizeof(short)*4);
  config_block->tag_number = (char *) malloc(sizeof(char)*20);
}

void CONFIG_BLOCK_destroy(CONFIG_BLOCK *config_block)
{
  free(config_block->fp);
  free(config_block->config_input_float);
  free(config_block->config_input_byte);
  free(config_block->title);
  free(config_block->units);
}
