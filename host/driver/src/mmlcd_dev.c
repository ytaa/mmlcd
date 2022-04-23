#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h> /* Needed for the macros */ 
#include <linux/kernel.h> /* Needed for pr_info() */ 
#include <linux/slab.h>

#include "mmlcd.h"
#include "mmlcd_dev.h"

/* --- Macros ------------------------------------------ */

#define MAX_USER_DATA_LEN 255u
#define MMLCD_DEV_MINOR_BASE 0u

/* --- Private functions declarations ------------------ */

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);
static struct mmlcd_dev_ctx *get_ctx_by_inode(struct inode *);

/* --- Global variables -------------------------------- */

static struct file_operations mmlcd_fops = {
	.read = dev_read,
	.write = dev_write,
	.open = dev_open,
	.release = dev_release,
};

struct usb_class_driver mmlcd_dev_class_driver = {
	.name = "mmlcd%d",
	.fops = &mmlcd_fops,
	.minor_base = MMLCD_DEV_MINOR_BASE,
};

/* --- Public functions definitions -------------------- */

struct mmlcd_dev_ctx * mmlcd_dev_create_ctx(struct usb_interface *interface){
	struct mmlcd_dev_ctx *ctx = NULL;
	struct usb_endpoint_descriptor *bulk_in = NULL, *bulk_out = NULL;
	int retval = 0;

	if(!interface){
		pr_err("Invalid argument");
		return NULL;
	}

	/* allocate new context */
	ctx = kzalloc(sizeof(struct mmlcd_dev_ctx), GFP_KERNEL);
	if (!ctx) {
		pr_err("Out of memory");
		return NULL;
	}

	ctx->dev = usb_get_dev(interface_to_usbdev(interface));
	ctx->interface = interface;

	retval = usb_find_common_endpoints(interface->cur_altsetting,
			&bulk_in, &bulk_out, NULL, NULL);
	if (retval) {
		pr_err("Could not find both bulk-in and bulk-out endpoints");
		mmlcd_dev_destroy_ctx(ctx);
		return NULL;
	}

	ctx->bulk_in_endpointAddr = bulk_in->bEndpointAddress;
	ctx->bulk_out_endpointAddr = bulk_out->bEndpointAddress;

	return ctx;
}

void mmlcd_dev_destroy_ctx(struct mmlcd_dev_ctx *ctx){
	if(!ctx){
		pr_err("Invalid argument");
		return;
	}

	usb_put_dev(ctx->dev);
	kfree(ctx);
}

/* --- Private functions definitions ------------------- */

static int dev_open(struct inode *inode, struct file *file){
	struct mmlcd_dev_ctx * ctx = NULL;

	pr_debug("device_open");

	ctx = get_ctx_by_inode(inode);
	if(!ctx){
		return -ENODEV;
	}

	/*retval = usb_autopm_get_interface(interface);
	if (retval)
		goto exit;*/

	/* save our object in the file's private structure */
	file->private_data = ctx;

	return 0;
}

static int dev_release(struct inode *inode, struct file *file){
	pr_debug("device_release");
	return 0;
}

static ssize_t dev_read(struct file *file, char __user *user_buffer, size_t length, loff_t *offset){
	char *data = NULL;
	ssize_t ret = -1;
	int bulk_retval = 0;
	struct mmlcd_dev_ctx * ctx = NULL;
	int received_length = 0u;
	
	//pr_debug("device_read");

	if(0u != *offset){
		return 0u;
	}

	ctx = file->private_data;
	if(!ctx){
		return -ENODEV;
	}

	data = kmalloc(MAX_USER_DATA_LEN, GFP_KERNEL);

	/* do an immediate bulk read to receive data from the device */
	bulk_retval = usb_bulk_msg (ctx->dev,
                       usb_rcvbulkpipe(ctx->dev, ctx->bulk_in_endpointAddr),
                       data,
                       MAX_USER_DATA_LEN,
                       &received_length, 5000);
	switch(bulk_retval){
		case 0:{
			break;
		}
		case -EAGAIN:{
			kfree(data);
			return 0;
		}
		default:{
			pr_err("Failed to receive data (code %d)", bulk_retval);
			kfree(data);
			return -EFAULT;
		}
	}
   
	pr_debug("length=%lu, offset=%llu", length, *offset);
	*offset = received_length;
	if(0 == copy_to_user(user_buffer, data, received_length)){
		ret = received_length;
	}

	pr_debug("ret: %ld, new_offset: %llu", ret, *offset);

	kfree(data);

	return ret;
}

static ssize_t dev_write(struct file *file, const char __user *user_buffer, size_t length, loff_t *offset){
	u8 *data = NULL; /* plus one byte for null terminator */
	ssize_t ret = -1;
	int bulk_retval = 0;
	struct mmlcd_dev_ctx * ctx = NULL;
	int transmitted_length = 0u;

	pr_debug("device_write");
	
	data = kmalloc(length, GFP_KERNEL);
	if(0 == copy_from_user(data, user_buffer, length)){
		ret = length;
		*offset += length;
	}
	//*offset = 5u;
	pr_debug("size=%lu, payload=N/A, ret=%ld, new_offset=%llu\n", length, ret, *offset);

	ctx = file->private_data;
	if(!ctx){
		pr_err("Unable to send data - private context is missing");
		ret = -ENODEV;
		goto L_ERR;
	}
	/* do an immediate bulk write to send data to the device */
	bulk_retval = usb_bulk_msg (ctx->dev,
                       usb_sndbulkpipe(ctx->dev, ctx->bulk_out_endpointAddr),
                       data,
                       length,
                       &transmitted_length, 5000);
	if(bulk_retval){
		pr_err("Failed to send data (code %d)", bulk_retval);
		ret = -ENODEV;
		goto L_ERR;
	}

	ret = length;

L_ERR:
	if(data){
		kfree(data);
	}

	return ret;
}

static struct mmlcd_dev_ctx *get_ctx_by_inode(struct inode *inode){
	struct mmlcd_dev_ctx *ctx = NULL;
	struct usb_interface *interface = NULL;
	int subminor = -1;

	if(!inode){
		pr_err("Invalid argument");
		return NULL;
	}

	subminor = iminor(inode);

	interface = usb_find_interface(&mmlcd_driver, subminor);
	if (!interface) {
		pr_err("Cannot find device for minor %d\n", subminor);
		return NULL;
	}

	ctx = usb_get_intfdata(interface);
	if (!ctx) {
		return NULL;
	}

	return ctx;
}