#ifndef _DEVTYPES
#define _DEVTYPES

#define SEM_COMMANDS_cACK 0xA
#define SEM_COMMANDS_cNAK 0xB
#define SEM_COMMANDS_cREAD_CAL 0x0
#define SEM_COMMANDS_cREAD_CONFIG 0x1
#define SEM_COMMANDS_cREAD_PROCESS 0x2
#define SEM_COMMANDS_cSELF_CAL_0mv 0x3
#define SEM_COMMANDS_cSELF_CAL_50mv 0x4
#define SEM_COMMANDS_cSELF_CAL_100R 0x30
#define SEM_COMMANDS_cSELF_CAL_300R 0x31
#define SEM_COMMANDS_cSELF_CAL_20mA 0x32
#define SEM_COMMANDS_cSELF_CAL_0mA 0x33  
#define SEM_COMMANDS_cSELF_CAL_200mV 0x34
#define SEM_COMMANDS_cSELF_CAL_1V 0x35
#define SEM_COMMANDS_cSELF_CAL_10V 0x36
#define SEM_COMMANDS_cSELF_CAL_slide_wire 0x37
#define SEM_COMMANDS_cPRESET_4ma_COUNT 0x5
#define SEM_COMMANDS_cPRESET_12ma_COUNT 0x6
#define SEM_COMMANDS_cPRESET_20ma_COUNT 0x7
#define SEM_COMMANDS_cPRESET_ENABLE 0x8
#define SEM_COMMANDS_cSET_CAL 0x10
#define SEM_COMMANDS_cSET_CONFIG 0x11
#define SEM_COMMANDS_cREAD_RANGEA 0x50
#define SEM_COMMANDS_cREAD_RANGEB 0x51
#define SEM_COMMANDS_cREAD_RANGEC 0x52
#define SEM_COMMANDS_cREAD_RANGED 0x53
#define SEM_COMMANDS_cWRITE_RANGEA 0x40
#define SEM_COMMANDS_cWRITE_RANGEB 0x41
#define SEM_COMMANDS_cWRITE_RANGEC 0x42
#define SEM_COMMANDS_cWRITE_RANGED 0x43
#define SEM_COMMANDS_cidentify 0x60

#define SEM_COMMANDS_cREAD_SEM160_CAL 0x60
#define SEM_COMMANDS_cREAD_SEM160_CONFIG 0x61
#define SEM_COMMANDS_cREAD_SEM160_ALIGNMENT 0x62
#define SEM_COMMANDS_cREAD_SEM160_PROCESS 0x63
#define SEM_COMMANDS_cREAD_SEM160_DEVICE 0x64
#define SEM_COMMANDS_cPRESET_ch2_4ma_COUNT 0x70
#define SEM_COMMANDS_cPRESET_ch2_12ma_COUNT 0x71
#define SEM_COMMANDS_cPRESET_ch2_20ma_COUNT 0x72
#define SEM_COMMANDS_cPRESET_ch2_ENABLE 0x73

#define SEM_COMMANDS_cALIGN_RH_1  0x65
#define SEM_COMMANDS_cALIGN_RH_2  0x66
#define SEM_COMMANDS_cALIGN_T_1  0x67
#define SEM_COMMANDS_cALIGN_T_2 0x68

#define SEM_COMMANDS_cWRITE_SEM160_CALIBRATION 0x6A
#define SEM_COMMANDS_cWRITE_SEM160_CONFIG 0x6B
#define SEM_COMMANDS_cWRITE_SEM160_ALIGNMENT 0x6C
#define SEM_COMMANDS_cWRITE_SEM160_DEVICE 0x6D

#define SEM_COMMANDS_cWRITE_TTR_4_5MA_CAL 0xA3
#define SEM_COMMANDS_cWRITE_TTR_19_5MA_CAL 0xA4
#define SEM_COMMANDS_cWRITE_TTR_1_5V_CAL 0xA5
#define SEM_COMMANDS_cWRITE_TTR_9_5V_CAL 0xA6
#define SEM_COMMANDS_cWRITE_TTx200_IN_GAIN_1 0x30
#define SEM_COMMANDS_cWRITE_TTx200_IN_GAIN_2 0x31
#define SEM_COMMANDS_cWRITE_TTx200_IN_GAIN_4 0xA7

#define SEM_COMMANDS_cWRITE_BLOCK_2_0_MA_CAL 0xA3
#define SEM_COMMANDS_cWRITE_BLOCK_19_5_MA_CAL 0xA4
#define SEM_COMMANDS_cWRITE_BLOCK_0_V_CAL 0xA5
#define SEM_COMMANDS_cWRITE_BLOCK_10V_CAL 0xA6
#define SEM_COMMANDS_cWRITE_BLOCK_CALIBRATION 0xA0
#define SEM_COMMANDS_cREAD_BLOCK_CALIBRATION 0xA8
#define SEM_COMMANDS_cWRITE_block_INPUT_CONFIG 0xA1
#define SEM_COMMANDS_cWRITE_BLOCK_OUTPUT_CONFIG 0xA2
#define SEM_COMMANDS_cREAD_BLOCK_INPUT_CONFIG 0xA9
#define SEM_COMMANDS_cREAD_BLOCK_OUTPUT_CONFIG 0xAA
#define SEM_COMMANDS_cREAD_BLOCK_PROCESS 0x2

#define SEM_COMMANDS_cWRITE_DM2000_CONFIG 0x7B

#define SEM_COMMANDS_cREAD_DM2000_CONFIG 0x81

#define SEM_COMMANDS_cRESET_TOTAL 0xF0

#define COMMAND_FUNCTION_READ_DATA 4
#define COMMAND_FUNCTION_WRITE_DATA 16
#define COMMAND_FUNCTION_SELF_CAL_IN 7
#define COMMAND_FUNCTION_PRESET 17
#define COMMAND_FUNCTION_CAL_OUT 18

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
  MESSAGE_NOT_VALID,
} COMMS_RX_STATE;

typedef struct {
  float ADC_VALUE;
  float ELEC_VALUE;
  float PROCESS_VARIABLE;
  float MA_OUT;
  float CJ_TEMP;
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
  float *fp;
  float *config_input_float;
  int *config_input_byte;
  char *title;
  char *units;
} CONFIG_BLOCK;

void CONFIG_BLOCK_init(CONFIG_BLOCK *config_block)
{
  config_block->fp = (float *) malloc(48);
  config_block->config_input_float = (float *) malloc(4);
  config_block->config_input_byte = (int *) malloc(4);
  config_block->title = (char *) malloc(16);
  config_block->units = (char *) malloc(14);
}

void CONFIG_BLOCK_destroy(CONFIG_BLOCK *config_block)
{
  free(config_block->fp);
  free(config_block->config_input_float);
  free(config_block->config_input_byte);
  free(config_block->title);
  free(config_block->units);
}

int get_endianness()
{
   int a = 0x12345678;
   unsigned char *c = (unsigned char*)(&a);
   if (*c == 0x78) {
      return 0;
   }
   else {
      return 1;
   }
}



#endif
