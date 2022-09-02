#ifndef MMLCD_PROC_H
#define MMLCD_PROC_H

#include <linux/usb.h>
#include <libmmlcd/libmmlcd_ipc.h>

#include <mmlcd.h>

/* --- Macros and constants ---------------------------- */

#define MMLCD_PROC_INT_IN_DATA_SIZE sizeof(liblcd_ipc_btn_event)

/* --- Type definitions -------------------------------- */

/* --- Extern variables -------------------------------- */

/* --- Public functions declarations ------------------- */

int mmlcd_proc_init(struct mmlcd_ctx *ctx);

void mmlcd_proc_deinit(struct mmlcd_ctx *ctx);

void mmlcd_proc_int_in_irq(struct urb *urb);

#endif