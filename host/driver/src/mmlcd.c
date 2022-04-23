#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h> /* Needed for the macros */ 
#include <linux/kernel.h> /* Needed for pr_info() */ 
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/usb.h>

#include "mmlcd_dev.h"

MODULE_AUTHOR("Tristan Dobrowolski"); 
MODULE_DESCRIPTION("USB device driver for miniature multifunctional liquid crystal display"); 
MODULE_LICENSE("GPL");
 
/* private functions declarations ---------------- */
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

/* private functions definitions ----------------- */
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
	struct mmlcd_dev_ctx * ctx = NULL;

	pr_info("New device probe (%04X:%04X) (minor %d)\n", id->idVendor, id->idProduct, interface->minor);
	
	/* Create new context for the interface */
	ctx = mmlcd_dev_create_ctx(interface);
	if(!ctx){
		pr_err("Failed to create new context");
		return -ENOMEM;
	}
	usb_set_intfdata(interface, ctx);

	res = usb_register_dev(interface, &mmlcd_dev_class_driver);
	pr_info("USB device register result: %d (minor %d)\n", res, interface->minor);

	return res;
}

static void mmlcd_disconnect(struct usb_interface *interface){
	struct mmlcd_dev_ctx *ctx = NULL;

	pr_info("Device disconnected\n");

	usb_deregister_dev(interface, &mmlcd_dev_class_driver);
	
	ctx = usb_get_intfdata(interface);
	mmlcd_dev_destroy_ctx(ctx);
}

module_init(mmlcd_init); 
module_exit(mmlcd_cleanup);