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

#ifdef __cplusplus
}
#endif

#endif /* __INC_USB_OPERATIONS_H */
