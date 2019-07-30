#ifndef __WL_DEBUG_H__
#define __WL_DEBUG_H__

#include <linux/printk.h>

#define WL_DEBUG

// this function does nothing but to mark a break point for GDB
void wl_break_point(void);
#define wl_printk(fmt, ...) \
	printk("\n#####################################"); \
	printk(fmt, ##__VA_ARGS__); \
	printk("#####################################\n"); \
	wl_break_point();

#endif
