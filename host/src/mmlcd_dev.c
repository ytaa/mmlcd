#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h> /* Needed for the macros */ 
#include <linux/kernel.h> /* Needed for pr_info() */ 
#include <linux/slab.h>

#include "mmlcd.h"
#include "mmlcd_dev.h"
#include "mmlcd_proc.h"

/* --- Macros ------------------------------------------ */

#define MAX_USER_DATA_LEN 255u
#define MMLCD_DEV_MINOR_BASE 0u

/* --- Private functions declarations ------------------ */

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);

/* --- Global variables -------------------------------- */

static struct file_operations mmlcd_fops = {
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

/* --- Private functions definitions ------------------- */

static int dev_open(struct inode *inode, struct file *file){
    struct mmlcd_ctx * ctx = NULL;

    pr_debug("dev_open");

    ctx = mmlcd_get_ctx_by_inode(inode);
    if(!ctx){
        return -ENODEV;
    }

    file->private_data = ctx;

    return 0;
}

static int dev_release(struct inode *inode, struct file *file){
    pr_debug("dev_release");
    file->private_data = NULL;
    return 0;
}

static ssize_t dev_write(struct file *file, const char __user *user_buffer, size_t length, loff_t *offset){
    u8 *data = NULL; /* plus one byte for null terminator */
    ssize_t ret = -1;
    int bulk_retval = 0;
    struct mmlcd_ctx * ctx = NULL;
    int transmitted_length = 0u;

    pr_debug("dev_write");
    
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
                       usb_sndbulkpipe(ctx->dev, ctx->bulk_out_ep->bEndpointAddress),
                       data,
                       length,
                       &transmitted_length, 5000);
    if(bulk_retval){
        pr_err("Failed to send data (code %d)", -bulk_retval);
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