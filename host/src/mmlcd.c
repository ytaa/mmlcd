#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h> /* Needed for the macros */ 
#include <linux/kernel.h> /* Needed for pr_info() */ 
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/usb.h>
#include <linux/slab.h>

#include "mmlcd.h"
#include "mmlcd_dev.h"
#include "mmlcd_proc.h"

MODULE_AUTHOR("Tristan Dobrowolski"); 
MODULE_DESCRIPTION("USB device driver for miniature multifunctional liquid crystal display"); 
MODULE_LICENSE("GPL");
 
/* private functions declarations ---------------- */
static struct mmlcd_ctx * mmlcd_create_ctx(struct usb_interface *interface);
static void mmlcd_destroy_ctx(struct mmlcd_ctx *ctx);
static int __init mmlcd_init(void);
static void __exit mmlcd_cleanup(void);
static int mmlcd_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void mmlcd_disconnect(struct usb_interface *interface);

/* global variables definitions ------------------ */
static struct usb_device_id mmlcd_device_table[] = {
	{ USB_DEVICE(0xC0DE, 0xB10B) },
	{} /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, mmlcd_device_table);

struct usb_driver mmlcd_driver = {
	.name = "mmlcd", 
	.id_table = mmlcd_device_table,
	.probe = mmlcd_probe,
	.disconnect = mmlcd_disconnect,
};

/* public functions definitions ----------------- */

struct mmlcd_ctx *mmlcd_get_ctx_by_inode(struct inode *inode){
	struct mmlcd_ctx *ctx = NULL;
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

/* private functions definitions ----------------- */

static struct mmlcd_ctx * mmlcd_create_ctx(struct usb_interface *interface){
	struct mmlcd_ctx *ctx = NULL;
	int retval = 0;

	if(!interface){
		pr_err("Invalid argument");
		return NULL;
	}

	/* allocate new context */
	ctx = kzalloc(sizeof(struct mmlcd_ctx), GFP_KERNEL);
	if (!ctx) {
		pr_err("Out of memory");
		return NULL;
	}

	ctx->dev = usb_get_dev(interface_to_usbdev(interface));
	ctx->interface = interface;

	retval = usb_find_common_endpoints(interface->cur_altsetting,
			NULL, &ctx->bulk_out_ep, &ctx->int_in_ep, NULL);
	if (retval) {
		pr_err("Could not find both bulk-in and bulk-out endpoints");
		mmlcd_destroy_ctx(ctx);
		return NULL;
	}

	/* prepare irq ep data buffer */
	ctx->int_data = usb_alloc_coherent(ctx->dev, MMLCD_PROC_INT_IN_DATA_SIZE, GFP_KERNEL, &ctx->int_dma);
	if(!(ctx->int_data)){
		pr_err("Failed to allocate irq ep data buffer");
		mmlcd_destroy_ctx(ctx);
		return NULL;
	}

	/* prepare irq ep urb */
	ctx->int_urb = usb_alloc_urb(0, GFP_KERNEL);
	if(!(ctx->int_urb)){
		pr_err("Failed to allocate irq ep URB");
		mmlcd_destroy_ctx(ctx);
		return NULL;
	}

	return ctx;
}

static void mmlcd_destroy_ctx(struct mmlcd_ctx *ctx){
	if(!ctx){
		pr_err("Invalid argument");
		return;
	}

	usb_free_urb(ctx->int_urb);
	usb_free_coherent(ctx->dev, MMLCD_PROC_INT_IN_DATA_SIZE, ctx->int_data, ctx->int_dma);
	usb_put_dev(ctx->dev);
	kfree(ctx);
}

static int __init mmlcd_init(void) 
{ 
	int ret = -1;
	pr_info("Module loaded\n");
	
	ret = usb_register(&mmlcd_driver);

	pr_info("Registering USB driver %s\n", 0 == ret ? "succeeded" : "failed"); 

	return ret; 
} 
 
static void __exit mmlcd_cleanup(void) 
{ 
	pr_info("Unloading module\n");
	usb_deregister(&mmlcd_driver);
	pr_info("USB driver deregistered\n");
} 

static int mmlcd_probe(struct usb_interface *interface, const struct usb_device_id *id){
	int res = 0;
	struct mmlcd_ctx * ctx = NULL;

	pr_info("New device probe (%04X:%04X)\n", id->idVendor, id->idProduct);
	
	/* Create new context for the interface */
	ctx = mmlcd_create_ctx(interface);
	if(!ctx){
		pr_err("Failed to create new context");
		return -ENOMEM;
	}
	usb_set_intfdata(interface, ctx);

	if((res = usb_register_dev(interface, &mmlcd_dev_class_driver))){
		pr_err("Failed to register USB device (code %d)\n", res);
		mmlcd_destroy_ctx(ctx);
		return res;
	}

	/* Setup int in ep urb */
	usb_fill_int_urb(ctx->int_urb, ctx->dev, usb_rcvintpipe(ctx->dev, ctx->int_in_ep->bEndpointAddress),
			 ctx->int_data, MMLCD_PROC_INT_IN_DATA_SIZE, mmlcd_proc_int_in_irq, ctx, 1);

	if((res = usb_submit_urb (ctx->int_urb, GFP_ATOMIC))){
		pr_err("Failed to submit interrupt URB (code %d)\n", res);
		usb_deregister_dev(interface, &mmlcd_dev_class_driver);
		mmlcd_destroy_ctx(ctx);
		return -EIO;
	}

	/* setup proc entries */
	if((res = mmlcd_proc_init(ctx))){
		pr_err("Failed to initialize proc entries (code %d)\n", res);
		usb_deregister_dev(interface, &mmlcd_dev_class_driver);
		mmlcd_destroy_ctx(ctx);
		return res;
	}

	pr_info("New device registered\n");

	return res;
}

static void mmlcd_disconnect(struct usb_interface *interface){
	struct mmlcd_ctx *ctx = NULL;

	ctx = usb_get_intfdata(interface);

	pr_info("Device disconnected\n");

	mmlcd_proc_deinit(ctx);

	usb_deregister_dev(interface, &mmlcd_dev_class_driver);
	
	mmlcd_destroy_ctx(ctx);
}

module_init(mmlcd_init); 
module_exit(mmlcd_cleanup);