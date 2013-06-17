#ifndef _USBOP
#define _USBOP

#include <ftdi.h>
#include <libusb-1.0/libusb.h>
#include "devtypes.h"
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
    for (j = 1; j <= 8; j++) {
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

  return 0;
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
  i++;
  output[i] = COMMAND;

  // length
  i++;
  output[i] = byte_array_upper_index;
  for (x = 0; x <= byte_array_upper_index; x++) {
    i++;
    output[i] = byte_array[x];
  }

  // crc
  crc = make_crc(output, i);
  printf("Sent crc: %d\n", crc);
  lbyte = (crc >> 8) & 0xFF;
  rbyte = (crc) & 0xFF;

  // put CRC on byte_array, little Endian
  i++;
  output[i] = rbyte;
  i++;
  output[i] = lbyte;

  // add end byte;
  i++;
  output[i] = 0xAA;
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


int SEM710_read_process(struct ftdi_context *ctx, SEM710_READINGS *readings)
{
  /* 
     reads the process of the SEM710 device, which includes: 
       -ADC value: analog-to-digital converter value
       -ELEC value: ?
       -PROCESS variable: ?
       -MA_OUT: Amperage of the device
       -CJ_TEMP: Adjusted temperature of the device.

  */
  uint8_t write_array[280];
  int len = -1;
  int i;
  write_array[0] = 0;
  uint8_t read_array[280];
  
  int written;
  int received;

  for (i = 0; i < 280; i++) {
    write_array[i] = 0;
    read_array[i] = 0;
  }

  len = generate_message(SEM_COMMANDS_cREAD_PROCESS, write_array, 0);
  if (len == -1) {
    printf("Failed to generate message.\n");
    return -1;
  }
  for (i = 0; (i < 40 && (read_array[1] != 34)); i++) { 
    // run until positive response received, or 40 negative replies (10 seconds)
    written = ftdi_write_data(ctx, write_array, len);
    if (written < 0) {
      printf("Error: ftdi_write_data(%d)\n", written);
      return written;
    }
   
    usleep(250000); // give the device some time to transmit process readings

    received = ftdi_read_data(ctx, read_array, 280);
    if (received < 0) {
      printf("Error: ftdi_read_data(%d)\n", received);
      return received;
    }
  }

  printf("Rec:");
  for (i = 0; i < received; i++) {
    printf("%u, ", read_array[i]);
  }
  printf("\n");

  printf("CRC passes... ");
  if (crc_pass(read_array, received - 2)) {
    printf("Yes.\n");
  }
  else {
    printf("No.\n");
  }

  readings->ADC_VALUE = float_from_byte_array(read_array, 3);
  readings->ELEC_VALUE = float_from_byte_array(read_array, 7);
  readings->PROCESS_VARIABLE = float_from_byte_array(read_array, 11);
  readings->MA_OUT = float_from_byte_array(read_array, 15);
  readings->CJ_TEMP = float_from_byte_array(read_array, 19);

  return 0;
}


int SEM710_read_config(struct ftdi_context *ctx, CONFIG_DATA *cal)
{
  uint8_t write_array[280];
  uint8_t read_array[280];

  int i;
  int len = -1;
  int written;
  int received;

  for (i = 0; i < 280; i++) {
    write_array[i] = 0;
    read_array[i] = 0;
  }
  
  len = generate_message(SEM_COMMANDS_cREAD_CONFIG, write_array, 0);
  if (len == -1) {
    printf("Failed to generate message.\n");
    return -1;
  }

  for (i = 0; (i < 40 && (read_array[1] != 33)); i++) { 
    // run until positive response received, or 40 negative replies (10 seconds)
    written = ftdi_write_data(ctx, write_array, len);
    if (written < 0) {
      printf("Error: ftdi_write_data %d (%s)\n", written,
    	     ftdi_get_error_string(ctx));
      return written;
    }
   
    printf("Bytes written: %d\n", written);

    usleep(250000); // give the device some time to transmit process readings

    received = ftdi_read_data(ctx, read_array, 280);
    if (received < 0) {
      printf("Error: ftdi_read_data %d (%s)\n", 
	     received, ftdi_get_error_string(ctx));
      return received;
    }
    printf("Bytes received: %d\n", received);
  }

  array_to_CONFIG_DATA(cal, read_array);
  
  return 0;
}

int SEM710_auto_cal(struct ftdi_context *ctx)
{
  
}

#endif
