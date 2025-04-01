#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h> /* Needed for the macros */ 
#include <linux/kernel.h> /* Needed for pr_info() */ 
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>


#include "mmlcd_proc.h"
#include "mmlcd.h"

/* --- Macros ------------------------------------------ */

#define MMLCD_PROC_FILE_NAME_BASE "mmlcd"
#define MMLCD_PROC_FILE_NAME_BTN "btn"
#define MMLCD_PROC_FILE_NAME_STATE "state"
#define MMLCD_PROC_FILE_NAME_SEP "_"
#define MMLCD_PROC_FILE_NAME_MAX_SIZE 64u
#define MMLCD_PROC_STATUS_STR_MAX_SIZE 10u

/* --- Private functions declarations ------------------ */

static int mmlcd_proc_open(struct inode *inode, struct file *file);
static int mmlcd_proc_release(struct inode *inode, struct file *file);
static ssize_t mmlcd_proc_read(struct file *filePointer, char __user *buffer, size_t buffer_length, loff_t *offset);
static void mmlcd_create_proc_file_name(struct mmlcd_ctx *ctx, liblcd_ipc_btn_idx btn_idx, char *name_buf, unsigned int name_buf_size);
static liblcd_ipc_btn_idx mmlcd_get_btn_idx_by_file(struct file *file);

/* --- Global variables -------------------------------- */

static const struct proc_ops proc_file_fops = {
    .proc_open = mmlcd_proc_open,
    .proc_release = mmlcd_proc_release,
    .proc_read = mmlcd_proc_read,
};

/* --- Public functions definitions -------------------- */

int mmlcd_proc_init(struct mmlcd_ctx *ctx){
    liblcd_ipc_btn_idx idx;
    char proc_name[MMLCD_PROC_FILE_NAME_MAX_SIZE];

    for(idx = liblcd_ipc_btn_first; idx < liblcd_ipc_btn_count; ++idx){
        mmlcd_create_proc_file_name(ctx, idx, proc_name, sizeof(proc_name));
        ctx->proc_file_dir_entry[idx] = proc_create(proc_name, 0644, NULL, &proc_file_fops);
        if(!(ctx->proc_file_dir_entry[idx])){
            pr_err("Failed to initialize proc file entry /proc/%s\n", proc_name);
            return -ENOMEM;
        }

        pr_info("Created proc file entry /proc/%s\n", proc_name);

        ctx->last_btn_state[idx] = liblcd_ipc_btn_released;
    }

    return 0;
}

void mmlcd_proc_deinit(struct mmlcd_ctx *ctx){
    liblcd_ipc_btn_idx idx;
    char proc_name[MMLCD_PROC_FILE_NAME_MAX_SIZE];

    for(idx = liblcd_ipc_btn_first; idx < liblcd_ipc_btn_count; ++idx){
        mmlcd_create_proc_file_name(ctx, idx, proc_name, sizeof(proc_name));
        if(ctx->proc_file_dir_entry[idx]){
            proc_remove(ctx->proc_file_dir_entry[idx]);
            pr_info("Remove proc file entry /proc/%s\n", proc_name);
        }
    }
}


void mmlcd_proc_int_in_irq(struct urb *urb){
    struct mmlcd_ctx *ctx = urb->context;
    liblcd_ipc_btn_event *ev = NULL;
    int res = -1;

    pr_debug("mmlcd_dev_int_in_irq");
    pr_debug("urb->status: %d", urb->status);

    switch (urb->status) {
    case 0:			
        /* success */
        break;
    case -ECONNRESET:
    case -ENOENT:
    case -ESHUTDOWN:
        /* unlink */
        return;
    default:
        /* error */
        pr_err("Unexpected int in ep URB status (%d)\n", urb->status);
        goto resubmit;
    }

    ev = ctx->int_data;
    pr_debug("Reveiced interrupt data: {btn_idx: %d, btn_state: %d}\n", ev->idx, ev->state);
    ctx->last_btn_state[ev->idx] = ev->state;

resubmit:
    if ((res = usb_submit_urb (urb, GFP_ATOMIC))){
        pr_err("Failed to resubmit interrupt URB (code %d)\n", res);
    }
}

/* --- Private functions definitions ------------------- */

static int mmlcd_proc_open(struct inode *inode, struct file *file){
    struct mmlcd_ctx * ctx = NULL;

    pr_debug("mmlcd_proc_open");

    ctx = mmlcd_get_ctx_by_inode(inode);
    if(!ctx){
        return -ENODEV;
    }

    /* save our object in the file's private structure */
    file->private_data = ctx;

    return 0;
}

static int mmlcd_proc_release(struct inode *inode, struct file *file){
    pr_debug("mmlcd_proc_release");
    file->private_data = NULL;
    return 0;
}

static ssize_t mmlcd_proc_read(struct file *file, char __user *ubuf, size_t ubuf_len, loff_t *offset){
    char kbuf[MMLCD_PROC_STATUS_STR_MAX_SIZE];
    ssize_t kbuf_len = sizeof(kbuf);
    struct mmlcd_ctx * ctx = NULL;
    liblcd_ipc_btn_idx btn_idx = liblcd_ipc_btn_idx_invlid;

    pr_debug("mmlcd_proc_read");

    if(0 != *offset){
        return 0;
    }

    ctx = file->private_data;
    btn_idx = mmlcd_get_btn_idx_by_file(file);
    if(liblcd_ipc_btn_idx_invlid <= btn_idx || liblcd_ipc_btn_first > btn_idx){
        pr_err("Failed to get button index\n");
        return 0;
    }

    kbuf_len = snprintf(kbuf, sizeof(kbuf), "%d\n", ctx->last_btn_state[btn_idx]);
    
    if(ubuf_len < kbuf_len || copy_to_user(ubuf, kbuf, kbuf_len)){
        pr_err("Failed to copy data to user buffer\n");
        return 0;
    }
    else{
        *offset += kbuf_len;
    }

    return kbuf_len;
}

static void mmlcd_create_proc_file_name(struct mmlcd_ctx *ctx, liblcd_ipc_btn_idx btn_idx, char *name_buf, unsigned int name_buf_size){
    /* Add 1 in order to start button names from btn1 and not from btn0 */
    btn_idx += 1;

    snprintf(name_buf, name_buf_size, "%s%d%s%d%s", 
        MMLCD_PROC_FILE_NAME_BASE, 
        ctx->interface->minor, 
        MMLCD_PROC_FILE_NAME_SEP MMLCD_PROC_FILE_NAME_BTN, 
        btn_idx, 
        MMLCD_PROC_FILE_NAME_SEP MMLCD_PROC_FILE_NAME_STATE
    );
}

static liblcd_ipc_btn_idx mmlcd_get_btn_idx_by_file(struct file *file){
    liblcd_ipc_btn_idx idx;
    char cur_name[MMLCD_PROC_FILE_NAME_MAX_SIZE];
    struct mmlcd_ctx * ctx = NULL;

    ctx = file->private_data;

    for(idx = liblcd_ipc_btn_first; idx < liblcd_ipc_btn_count; ++idx){
        mmlcd_create_proc_file_name(ctx, idx, cur_name, sizeof(cur_name));
        if(0 == strcmp(cur_name, file->f_path.dentry->d_name.name))
        {
            break;
        }
    }

    return idx;
}