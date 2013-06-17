#ifndef __INC_USB_OPERATIONS_H
#define __INC_USB_OPERATIONS_H

#include <ftdi.h>
#include "devtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

int detach_device_kernel(int vendor_id, int product_id);
int open_device(struct ftdi_context *ctx, int vendor_id, int product_id);
int prepare_device(struct ftdi_context *ctx);
int read_device(struct ftdi_context *ctx, int command, uint8_t *incoming_buff);
int SEM710_read_process(struct ftdi_context *ctx, SEM710_READINGS *readings);
int SEM710_read_config(struct ftdi_context *ctx, CONFIG_DATA *cal);

#ifdef __cplusplus
}
#endif

#endif /* __INC_USB_OPERATIONS_H */

