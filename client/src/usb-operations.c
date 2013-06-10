#ifndef _USBOP
#define _USBOP

#include <ftdi.h>
#include <libusb-1.0/libusb.h>
#include "../devtypes.h"
#include "array.c"

uint16_t make_crc(uint8_t *byte_array, int end_position)
{
  // make a cyclic redundancy check for the given byte array; this is per the
  // rules found in the SEM710 specifications provided by Status Instruments; 
  // the polynomial 0xA001 doesn't appear to conform to any international
  // crc standard.
  int i;
  uint16_t crc;
  char j;
  char char_in;
  int lsBit;

  /*
    The CRC is based on the following components of this message:
    
    Start
    Command
    Length
    Data
  */

  crc = 0xFFFF;
  for (i = 1; i <= end_position; i++) {
    char_in = byte_array[i];
    crc = crc ^ char_in;
    for (j = 0; j < 8; j++) {
      lsBit = crc & 1;
      crc = crc >> 1;
      if (lsBit == 1) {
	crc = crc ^ 0xA001;
      }
    }
  }
  return crc;
}

int crc_pass(uint8_t *rx_data, int rx_pointer) 
{
  // returns whether the received crc is the same as the calculated crc from
  // the byte array passed by the USB device; if it is not, then the usb 
  // transfer was faulty
  uint16_t calculated_crc = make_crc(rx_data, rx_pointer - 2);
  uint16_t rx_crc = (rx_data[rx_pointer] << 8) + (rx_data[rx_pointer - 1]);

  printf("Calculated crc: %d\n", calculated_crc);
  printf("Received crc: %d\n", rx_crc);

  return (calculated_crc == rx_crc);
}

int detach_device_kernel(int vendor_id, int product_id) 
{
  // if the device kernel with the given vendor id and product id is currently 
  // active then detach it and return whether or not this action failed. This 
  // must be done using libusb, since ftd2xx does not have this capability.

  int detach_failed;
  libusb_context *context = NULL;
  libusb_init(&context);
  libusb_set_debug(context, 3);
  libusb_device_handle *handle = NULL;

  handle = libusb_open_device_with_vid_pid(context, vendor_id, product_id);
  if (handle == NULL) {
    printf("Error opening device: device missing.\n");    
    return -1;
  }
  
  if (libusb_kernel_driver_active(handle, 0)) {
    printf("Kernel driver already attached. Detaching...\n");
    detach_failed = libusb_detach_kernel_driver(handle, 0);
    if (detach_failed) {
      printf("There was an error detaching the kernel driver.\n");
      return -1;
    }
  }
  libusb_release_interface(handle, 0);
  libusb_close(handle);
  libusb_exit(context);
  
  return 0;
}

int open_device(struct ftdi_context *ctx, int vendor_id, int product_id) 
{
  // use FTD2XX to open the device; this open device function simply opens the
  // first device it finds with the given vendor and product id, and cannot
  // differentiate between multiple devices with the same vID and pID
  int err = 0;
  
  err = ftdi_init(ctx);
  if (err) { return err; }

  err = ftdi_usb_open(ctx, vendor_id, product_id);
  if (err) { return err; }
  
  // open successful
  return 0;
}


int prepare_device(struct ftdi_context *ctx) 
{
  /* struct ftdi_bits_type btype = BITS_8; */
  /* struct ftdi_stop_bits_type sbits = STOP_BIT_1; */
  /* struct ftdi_parity_type parity = NONE; */

  int err = 0;

  err =  ftdi_usb_purge_rx_buffer(ctx);
  if (err) {
    printf("Error: ftdi_purge_rx_buffer(%d)\n", err);
    return err;
  }

  err =  ftdi_usb_purge_tx_buffer(ctx);
  if (err) {
    printf("Error: ftdi_purge_tx_buffer(%d)\n", err);
    return err;
  }

  err = ftdi_set_baudrate(ctx, 19200);
  if (err) {
    printf("Error: ftdi_set_baudrate(%d)\n", err);
    return err;
  }

  err = ftdi_set_line_property(ctx, 8, 0, 0);
  if (err) {
    printf("Error: ftdi_set_line_property(%d)\n", err);
    return err;
  }

  err = ftdi_setflowctrl(ctx, SIO_DISABLE_FLOW_CTRL);
  if (err) {
    printf("Error: ftdi_set_flow_ctrl(%d)\n", err);
    return err;
  }

  err = ftdi_set_latency_timer(ctx, 3);
  if (err) {
    printf("Error: ftdi_set_latency_timer(%d)\n", err);
    return err;
  }
}

int generate_message(uint8_t COMMAND, uint8_t* byte_array,
		     int byte_array_upper_index) 
{
  // Generate a message to send to the device, based on the given command and
  // byte array; returns the size of the message
  uint8_t output[259];
  int i = 0;
  uint8_t x;
  uint16_t crc;
  uint8_t lbyte;
  uint8_t rbyte;

  // start byte
  output[i] = 0x55;

  // command
  output[++i] = COMMAND;

  // length
  output[++i] = byte_array_upper_index;
  for (x = 0; x <= byte_array_upper_index; x++) {
    output[++i] = byte_array[x];
  }

  // crc
  crc = make_crc(output, i);
  lbyte = (crc >> 8);
  rbyte = (crc) & 0xFF;

  // put CRC on byte_array, little Endian
  output[++i] = rbyte;
  output[++i] = lbyte;

  // add end byte;
  output[++i] = 0xAA;
  for (x = 0; x <= i; x++) {
    byte_array[x] = output[x];
  }

  return (i + 1);
}

int generate_block_message(uint8_t device_address, int command, 
			   uint8_t byte_count, long block_address, 
			   uint8_t *byte_array, uint8_t read)
{
  // generate a block message and return the length of said block message
  uint8_t output[280];

  int i = 0;
  uint8_t x = 0;

  uint16_t crc;
  uint8_t lbyte;
  uint8_t rbyte;

  long temp;

  // data enters in the byte_array
  output[0] = device_address;
  output[1] = command;
  
  output[2] = byte_count;
  // start address Msb then lsb
  temp = block_address & 0xFF00;
  output[4] = block_address - (temp);
  output[3] = temp / (2 << 8);

  i = 4;
  if (!read) {
    for (x = 0; x < byte_count; x++) {
      i++;
      output[i] = byte_array[x];
    }
  }
  
  crc = make_crc(output, i);
  lbyte = (crc >> 8);
  rbyte = (crc) & 0xFF;

  // put CRC on byte_array, little Endian
  output[++i] = rbyte;
  output[++i] = lbyte;

  for (x = 0; x <= i; x++) {
    byte_array[x] = output[x];
  }

  return i + 1;
}



/* int SEM710_read_config_block(ftdi_context *ctx, CONFIG_BLOCK *cal,  */
/* 			     uint8_t device_address) */
/* { */
/*   uint8_t byte_array[280]; */
/*   uint8_t output_array[280]; */
/*   byte_array[0] = 0; */
/*   int i; */
/*   int len; */
/*   int err; */
  
/*   len = generate_block_message(device_address, COMMAND_FUNCTION_READ_DATA, */
/* 			       0xEC, 0xE000, byte_array, 1); */
/*   printf("Message: [%u,", byte_array[0]); */
/*   for (i = 1; i < len - 1; i++) { */
/*     printf(" %u,", byte_array[i]); */
/*   } */
/*   printf(" %u]\n", byte_array[len - 1]); */

/* } */

int SEM710_read_process(struct ftdi_context *ctx, SEM710_READINGS *readings)
{
  /* 
     reads the process of the SEM710 device, which includes: 
       -ADC value: analog-to-digital converter value
       -ELEC value: ?
       -PROCESS variable: ?
       -MA_OUT: Amperature reading of the device
       -CJ_TEMP: Adjusted temperature of the device.

  */
  uint8_t byte_array[280];
  int len = -1;
  int i;
  int err;
  byte_array[0] = 0;
  uint8_t response_array[28];

  for (i = 0; i < 280; i++) {
    byte_array[i] = 0;
  }
  for (i = 0; i < 28; i++) {
    response_array[i] = 0;
  }

  len = generate_message(SEM_COMMANDS_cREAD_PROCESS, byte_array, 0);
  if (len == -1) {
    printf("Failed to generate message.\n");
    return -1;
  }
  for (i = 0; (i < 40 && (response_array[1] != 34)); i++) { 
    // run until positive response received, or 40 negative replies (10 seconds)
    err = ftdi_write_data(ctx, byte_array, len);
    if (err < 0) {
      printf("Error: ftdi_write_data(%d)\n", err);
      return err;
    }
   
    usleep(250000); // give the device some time to transmit process readings

    err = ftdi_read_data(ctx, response_array, 28);
    if (err < 0) {
      printf("Error: ftdi_read_data(%d)\n", err);
      return err;
    }
  }

  printf("Rec:");
  for (i = 0; i < 28; i++) {
    printf("%u, ", response_array[i]);
  }
  printf("\n");

  printf("CRC passes... ");
  if (crc_pass(response_array, 27)) {
    printf("Yes.\n");
  }
  else {
    printf("No.\n");
  }

  readings->ADC_VALUE = float_from_byte_array(response_array, 3);
  readings->ELEC_VALUE = float_from_byte_array(response_array, 7);
  readings->PROCESS_VARIABLE = float_from_byte_array(response_array, 11);
  readings->MA_OUT = float_from_byte_array(response_array, 15);
  readings->CJ_TEMP = float_from_byte_array(response_array, 19);

  return 0;
}

/* int SEM710_write_config(, CONFIG_DATA *cal) */
/* { */
/*   uint8_t byte_array[41]; */
/*   int i; */
/*   int len = -1; */
/*   uint8_t output_array[280]; */
/*   int err; */
  
/*   byte_array[0] = cal->tc_code; */
/*   byte_array[1] = cal->up_scale; */
/*   byte_array[2] = cal->units; */
/*   byte_array[3] = cal->model_type; */
/*   byte_array[4] = cal->vout_range; */
/*   byte_array[5] = cal->action_A; */
/*   byte_array[6] = cal->action_B; */
/*   byte_array[7] = cal->spare; */
/*   i = float_into_byte_array(byte_array, 8, cal->low_range); */
/*   i = float_into_byte_array(byte_array, i, cal->high_range); */
/*   i = float_into_byte_array(byte_array, i, cal->low_trim); */
/*   i = float_into_byte_array(byte_array, i, cal->high_trim); */
/*   i = float_into_byte_array(byte_array, i, cal->setpoint_A); */
/*   i = float_into_byte_array(byte_array, i, cal->hyst_A); */
/*   i = float_into_byte_array(byte_array, i, cal->setpoint_B); */
/*   i = float_into_byte_array(byte_array, i, cal->hyst_B);   */

/*   len = generate_message(SEM_COMMANDS_cSET_CONFIG, byte_array, i-1); */
/*   if (len <= 0) { */
/*     printf("Error generating message"); */
/*     return -1; */
/*   } */

/*   printf("Message: [%u,", byte_array[0]); */
/*   for (i = 1; i < len - 1; i++) { */
/*     printf(" %u,", byte_array[i]); */
/*   } */
/*   printf(" %u]\n", byte_array[len - 1]); */
  
/* } */

/* int SEM710_read_cal(, UNIVERSAL_CALIBRATION *cal) */
/* { */
/*   uint8_t byte_array[280]; */
/*   uint8_t output_array[280]; */
/*   byte_array[0] = 0; */
/*   int len = -1; */
/*   int err; */
  
/*   len = generate_message(SEM_COMMANDS_cREAD_CAL, byte_array, 0); */
/*   if (len <= 0) { */
/*     printf("Error generating message.\n"); */
/*     return -1; */
/*   } */

/* } */

/* int SEM710_set_cal(, UNIVERSAL_CALIBRATION *cal) */
/* { */
/*   uint8_t byte_array[280]; */
/*   uint8_t output_array[280]; */
/*   int err; */
/*   int len = -1; */
/*   int i = 0; */
  
/*   byte_array[i] = 0; */
/*   byte_array[++i] = 0; */
/*   i = float_into_byte_array(byte_array, i, cal->lo_mv); */
/*   i = float_into_byte_array(byte_array, i, cal->hi_mv); */
/*   i = float_into_byte_array(byte_array, i, cal->lo_ma); */
/*   i = float_into_byte_array(byte_array, i, cal->hi_ma); */
/*   i = float_into_byte_array(byte_array, i, cal->lo_rtd); */
/*   i = float_into_byte_array(byte_array, i, cal->hi_rtd); */
/*   i = float_into_byte_array(byte_array, i, cal->lo_ma_in); */
/*   i = float_into_byte_array(byte_array, i, cal->hi_ma_in); */

/*   len = generate_message(SEM_COMMANDS_cSET_CAL, byte_array, i - 1); */
/*   if (len <= 0) { */
/*     printf("Error generating message\n"); */
/*     return -1; */
/*   } */

/* } */

/* int SEM710_enable_retran() */
/* { */
/*   uint8_t byte_array[280]; */
/*   uint8_t output_array[280]; */
/*   int len = -1; */
/*   int err; */

/*   byte_array[0] = 0; */
  
/*   len = generate_message(SEM_COMMANDS_cPRESET_ENABLE, byte_array, 0); */
/*   if (len <= 0) { */
/*     printf("Error generating message\n"); */
/*     return -1; */
/*   } */

/* } */

/* int SEM710_self_cal(, int command) */
/* { */
/*   /\* */
/*     cal commands are: */
/*     SEM_COMMANDS_cSELF_CAL_100R */
/*     SEM_COMMANDS_cSELF_CAL_300R */
/*     SEM_COMMANDS_cSELF_CAL_0mV */
/*     SEM_COMMANDS_cSELF_CAL_50mv */
/*     SEM_COMMANDS_cSELF_CAL_0mA */
/*     SEM_COMMANDS_cSELF_CAL_20mA */
/*    *\/ */
/*   uint8_t byte_array[280]; */
/*   uint8_t output_array[280]; */
/*   int len = -1; */
/*   int err; */

/*   byte_array[0] = 0; */
  
/*   len = generate_message(command, byte_array, 0); */
/*   if (len <= 0) { */
/*     printf("Error generating message\n"); */
/*     return -1; */
/*   } */
  

/* } */

#endif
