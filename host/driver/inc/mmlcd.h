#ifndef MMLCD_H
#define MMLCD_H

#include <linux/usb.h>
#include <linux/kernel.h> 
#include <linux/proc_fs.h>
#include <libmmlcd/libmmlcd_ipc.h>

/* --- Type definitions -------------------------------- */

struct mmlcd_ctx {
	struct usb_device				*dev;			/* the usb device for this device */
	struct usb_interface			*interface;		/* the interface for this device */
	struct urb 						*int_urb;
	void        					*int_data;
	dma_addr_t						int_dma;
    liblcd_ipc_btn_state            last_btn_state[liblcd_ipc_btn_count];
    struct proc_dir_entry           *proc_file_dir_entry[liblcd_ipc_btn_count];
	struct usb_endpoint_descriptor 	*int_in_ep;		/* int in endpoint descriptor */
	struct usb_endpoint_descriptor 	*bulk_out_ep;	/* bulk out endpoint descriptor */
};

/* --- Extern variables -------------------------------- */

extern struct usb_driver mmlcd_driver;

/* --- Public functions declarations ------------------- */

struct mmlcd_ctx *mmlcd_get_ctx_by_inode(struct inode *inode);

#endif