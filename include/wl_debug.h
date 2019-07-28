#ifndef __WL_DEBUG_H__
#define __WL_DEBUG_H__

#include <linux/printk.h>

#define WL_DEBUG

// this function does nothing but to mark a break point for GDB
void wl_break_point(void);
void wl_printk(const char* msg, ...);

#endif
