#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>
#include <sys/socket.h>

#define DEVICE_FILE "product.ini"
#define ID1 "DEVICE_ID_VENDOR"
#define ID2 "DEVICE_ID_PRODUCT"

void handle_error(int err_no) {
  switch (err_no) {
  case 0xE01:
    printf("Error: Unable to open device file.\n");
    break;
  case 0xE02:
    printf("Error: Bad or malformed device file.\n");
    break;
  case 0xE03:
    printf("Error: No devices available.\n");
    break;
  case 0xE04:
    printf("Error: Device missing from list of devices.\n");
    break;
  case 0xE05:
    printf("Error: Problem opening device.\n");
    break;
  default:
    printf("Error: Unknown error. \n");
    break;
  }
  error();
}

// get index of first occurrence of chr in string; not present returns -1 
int get_char_index(char *string, char chr)
{
  int index = 0;

  while (string[index] != '\0') {
    if (string[index] == chr) {
      return index;
    }
    index++;
  }
  return -1; // not found
}

// get the ID equal to the string phrase in the given line
// returns 1 if id found, and 0 if file is malformed
int get_id_value(char *line, char *phrase, int *id_val) 
{
  int eq_index = get_char_index(line, '=');
  int eol_index = get_char_index(line, '\n');

  if (eq_index == -1 || eol_index == -1) {
    return 0;
  }

  int diff = eol_index - eq_index;
  char val_str[diff];
  
  int i;
  for (i = 0; i < diff - 1; i++) {
    val_str[i] = line[eq_index+1 + i];
  }
  val_str[diff - 1] = '\0';
  *id_val = atoi(val_str);

  return 1;
}

int get_specified_id (FILE *f, char *id_phrase, int *id_val)
{
  int found = 0;
  
  rewind(f);
  char line[256];
  while (!feof(f)) {
    if (fgets(line, 256, f) != NULL) {
      if (strstr(line, id_phrase)) {
	found = get_id_value(line, id_phrase, id_val);
	break;
      }
    }
  }
  return found;
}

int get_device_ids(int *device_id, int *vendor_id)
{
  FILE *dfile = fopen(DEVICE_FILE, "r");
  if (!dfile) {
    return 0xE01;
  }

  int found_dev_id = get_specified_id(dfile, ID1, vendor_id);
  int found_vend_id = get_specified_id(dfile, ID2, device_id);

  if (found_dev_id && found_vend_id) {
    return 0;
  }
  else {
    return 0xE02;
  }
}

//Check for our VID & PID
int is_desired_product(libusb_device *dev, int product_id, int vendor_id)
{
  struct libusb_device_descriptor desc;
  int r = libusb_get_device_descriptor(dev, &desc);
  /*  
  printf("desc.idVendor = %d\n", desc.idVendor);
  printf("desc.idProduct = %d\n", desc.idProduct);
  printf("product_id = %d\n", product_id);
  printf("vendor_id = %d\n", vendor_id);
  */
  return (desc.idVendor == vendor_id && desc.idProduct == product_id);
}

int get_device(libusb_device **dev, libusb_context *ctx, 
	       libusb_device **dev_list, ssize_t num_devices, 
	       int product_id, int vendor_id) 
{  
  
  if (num_devices <= 0) {
    return 0xE03;
  }
  ssize_t i = 0;
  
  for (i = 0; i < num_devices; i++) {
    // search devices for our product
    if (is_desired_product(dev_list[i], product_id, vendor_id)) {
      *dev = dev_list[i];
      break;
    }
  }
  if (dev) {
    return 0;
  }
  else {
    return 0xE04;
  }  
}
     
void show_user_options()
{
  printf("\nPlease enter number corresponding to selection.\n");
  printf("0: Exit\n");
  printf("1: Connect to host\n");
  printf("2: Monitor Temperature\n");
}

int get_user_selection()
{
  char text[20];
  fputs("Enter selection: ", stdout);
  fflush(stdout);
  if ( fgets(text, sizeof text, stdin) )
    {
      int number;
      if ( sscanf(text, "%d", &number) == 1 )
	{
	  return number;
	}
    }
}

int main(int argc, char **argv)
{
  // Get device ids from file:
  int product_id;
  int vendor_id;

  int err = 0;

  err = get_device_ids(&product_id, &vendor_id);
  if (err) {
    handle_error(err);
  }

  // Open desired device from available devices:
  
  libusb_device *dev = NULL;
  libusb_device **dev_list;
  
  libusb_context *context = NULL;
  libusb_init(&context);
  libusb_set_debug(context, 3);
  /*
  ssize_t num_devices = libusb_get_device_list(context, &dev_list);

  err = get_device(&dev, context, dev_list, num_devices, product_id, vendor_id);
  if (err) {
    handle_error(err);
  }
  */


  // Prepare the device:
  printf("Device found, opening...\n");

  libusb_device_handle *handle;
  handle = libusb_open_device_with_vid_pid(context, vendor_id, product_id);
  /*
  err = libusb_open(dev, &handle);
  if (err){
    handle_error(0xE05);
  }
  */

  err = libusb_detach_kernel_driver(handle, 0);
  if (err) {
    error(1);
  }
  err = libusb_set_configuration(handle, 1);
  if (err) {
    error(1);
  }
  err = libusb_claim_interface(handle, 0);
  if (err < 0) {
    error(1);
  }


  ////////////////////// Main body loop: /////////////////////////
  int sel = 0;
  int looping = 1;
  char *data1;
  char *data2;
  while(looping) { 
    show_user_options();
    sel = get_user_selection();
    switch(sel) {
    case 9:
      looping = 0;
      break;
    case 1:
      printf("Please wait while connection is established...");
      printf("The connection was unsuccessful as no protocol has been ");
      printf("implemented for such a connection.\n");
      break;
    case 2:
      printf("Monitoring, interrupt using CTRL+C\n");
      libusb_control_transfer(handle, 0x40, 0, 0, 0, data1, 2, 0);
      /*
	TO DEVICE:
	libusb_control_transfer(handle, 0x40, 0, 0, 0, data1, 2, 0);
	
	FROM DEVICE:
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x0040, [TRANSFERRED], 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x0002, [TRANSFERRED], 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x1000, [TRANSFERRED], 0);
	[Vendor device, whatever that is]
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x1000, [TRANSFERRED], 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x1000, [TRANSFERRED], 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x1000, [TRANSFERRED], 0);
	libusb_control_transfer(handle, 0x60, 05, 0, 0, data2, 2, 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x0002, [TRANSFERRED], 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x1000, [TRANSFERRED], 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x0002, [TRANSFERRED], 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x0002, [TRANSFERRED], 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x0002, [TRANSFERRED], 0);
	libusb_bulk_transfer(handle, 0x02, [DATA], 0x0002, [TRANSFERRED], 0);
	[Vendor device, whatever that is]
      */
      libusb_control_transfer(handle, 0x60, 05, 0, 0, data2, 2, 0);
      break;
    default:
      printf("Invalid selection\n");
      break;
    }
  }

  printf("Exiting...\n");
  libusb_release_interface(handle, 0);
  libusb_attach_kernel_driver(handle, 0);
  libusb_reset_device(handle);
  libusb_close(handle);
  libusb_free_device_list(dev_list, 1);
  libusb_exit(context);
}
