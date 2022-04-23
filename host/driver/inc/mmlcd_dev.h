#ifndef MMLCD_DEV_H
#define MMLCD_DEV_H

#include <linux/fs.h>
#include <linux/usb.h>

/* --- Type definitions -------------------------------- */

struct mmlcd_dev_ctx {
	struct usb_device		*dev;					/* the usb device for this device */
	struct usb_interface	*interface;				/* the interface for this device */
	__u8 					bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
	__u8					bulk_out_endpointAddr;	/* the address of the bulk out endpoint */
};

/* --- Extern variables -------------------------------- */

extern struct usb_class_driver mmlcd_dev_class_driver;

/* --- Public functions declarations ------------------- */

struct mmlcd_dev_ctx * mmlcd_dev_create_ctx(struct usb_interface *interface);

void mmlcd_dev_destroy_ctx(struct mmlcd_dev_ctx *ctx);

#endif