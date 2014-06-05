#include "devtypes.h"
#include "array.h"
#include "cJSON.h"

#include <stdio.h>
#include <assert.h>
#include <stdio.h>

uint8_t get_confirmation_byte(SEM_COMMANDS c) {
  /*
     Returns: the expected first byte after start byte in incoming message from
     device for the given command c.
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
/* (temp_exp - range) <= temp_reading <= (temp_exp + range) */
char *get_device_read_status(float temp_reading, float temp_exp, 
			     float temp_range) 
{
  if (temp_reading == -1000000.000000) {
    return "ERROR: PROBE MISSING";
  }
  else if (!((temp_exp - temp_range) <= temp_reading && 
	     temp_reading <= (temp_exp + temp_range))) {
    return "WARNING: FREEZER TEMPERATURE OUT OF EXPECTED RANGE";
  }
  else if (0 /* additional status conditions go here */) {
    return "";
  }

  return "OK";
}

void get_readings(SEM710_READINGS *readings, float temp_exp, float temp_range,
		  uint8_t *byte_array, int array_len)
{
  assert (array_len > 23);

  /* float elec_value = float_from_byte_array(byte_array, 7); */
  /* float ma_out = float_from_byte_array(byte_array, 15); */
  /* float cj_temp = float_from_byte_array(byte_array, 19); */

  readings->ADC_VALUE = float_from_byte_array(byte_array, 3);
  readings->PROCESS_VARIABLE = float_from_byte_array(byte_array, 11);
  readings->STATUS = get_device_read_status(readings->PROCESS_VARIABLE, 
					    temp_exp, temp_range);
}

void display_readings(SEM710_READINGS *readings)
{
  printf("ADC_VALUE=%f\n", readings->ADC_VALUE);
  printf("STATUS=%s\n", readings->STATUS);
  /* printf("ELEC_VALUE=%f\n", readings->ELEC_VALUE); */
  /* printf("PROCESS_VARIABLE=%f\n", readings->PROCESS_VARIABLE); */
  /* printf("MA_OUT=%f\n", readings->MA_OUT); */
  /* printf("CJ_TEMP=%f\n", readings->CJ_TEMP); */
}

void pack_auth(char *user, char *password, char *filename) {
  char json_auth[100];
  FILE *f;

  sprintf(json_auth, "{ \"email\": \"%s\", \"password\": \"%s\" }",
	  user, password);

  f = fopen(filename, "w");
  fputs(json_auth, f);
  fclose(f);

}

int pack_readings(SEM710_READINGS *readings, char *user, 
		   char *password, char *filename)
{
  char json_readings[500];
  FILE *f;
  
  sprintf(
	  json_readings, 
 "{ \"email\": \"%s\", \"password\": \"%s\", \"temperature\": %f, \"status\": \"%s\" }",
	  user,
	  password,
	  readings->PROCESS_VARIABLE, 
	  readings->STATUS
	  );
  f = fopen(filename, "w");

  if (f == NULL) {
    printf("Error opening file %s\n", filename);
    return 1;
  }

  fputs(json_readings, f);

  fclose(f);
  return 0;

  /* strcat(json_readings, "\"temperature\":"); */
  /* strcat(json_readings, temperature); */
  /* strcat(json_readings, ", \"status\":"); */
  /* strcat(json_readings, status); */
  /* strcat(json_readings, "}"); */

  /* char *retval = json_readings; */

  /* // return "{ \"temperature\":55.1, \"status\":\"ok\" }"; */

  /* return retval; */
}

void get_config(CONFIG_DATA *cal, uint8_t *byte_array, int array_len)
{
  /* transfers the contents of the given byte array into the CONFIG_DATA */
  assert (array_len > 43);

  cal->tc_code = byte_array[3];
  cal->up_scale = byte_array[4];
  cal->units = byte_array[5];
  cal->model_type = byte_array[6];
  cal->vout_range = byte_array[7];
  cal->action_A = byte_array[8];
  cal->action_B = byte_array[9];
  cal->spare = byte_array[10];
  cal->low_range = float_from_byte_array(byte_array, 11);
  cal->high_range = float_from_byte_array(byte_array, 15);
  cal->low_trim = float_from_byte_array(byte_array, 19);
  cal->high_trim = float_from_byte_array(byte_array, 23);
  cal->setpoint_A = float_from_byte_array(byte_array, 27);
  cal->hyst_A = float_from_byte_array(byte_array, 31);
  cal->setpoint_B = float_from_byte_array(byte_array, 35);
  cal->hyst_B = float_from_byte_array(byte_array, 39);
}

void display_config(CONFIG_DATA *cal)
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
  free(config_block->config_output_floats);
  free(config_block->config_output_int);
  free(config_block->tag_number);
}

