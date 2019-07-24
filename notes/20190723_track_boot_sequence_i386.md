# 20190723

Start with 

> arch/x86/kernel/head_32.S

Kernel Offset is expected to be `0xc0000000`

After GDB with QEMU, the entry is `startup_32 = 0xc1000000`

But in real mode, only by setting  

> (gdb) break * 0x1000000 

which is `startup_32 - Kernel Offset` will hit the real `startup_32`

Scan the code with gdb command 

> si
>
> (gdb) x/1ib eip 

will show the disassembly



On hitting `startup_32` the `cr0` is `0x11`, with `ET` and `PE` enabled!

**This explains why the NEMU kernel code should be different from the Linux kernel here. In NEMU, when the kernel is first loaded, it is not in the PE mode.  And there is no operation to enable PE in Linux kernel here.**

>eip            0x1000000	0x1000000
>eflags         0x46	[ IOPL=0 ZF PF ]
>cs             0x10	16	0x00010 0 00
>ss             0x18	24	0x00011 0 00
>ds             0x18	24	0x00011 0 00
>es             0x18	24	0x00011 0 00
>fs             0x18	24	0x00011 0 00
>gs             0x18	24	0x00011 0 00  // keeps the same for data segments after the following mov 
>fs_base        0x0	0
>gs_base        0x0	0
>k_gs_base      0x0	0
>cr0            0x11	[ ET PE ]
>cr2            0x0	0
>cr3            0x0	[ PDBR=0 PCID=0 ]
>cr4            0x0	[ ]
>cr8            0x0	0



Then the data segment registers are set to `0x18`, with the following GDT loaded to `gdtr`

>ENTRY(boot_gdt)e
>.fill GDT_ENTRY_BOOT_CS,8,0
>.quad 0x00cf9a000000ffff	/* kernel 4GB code at 0x00000000 */
>.quad 0x00cf92000000ffff	/* kernel 4GB data at 0x00000000 */

by parsing the above GDT, we can find that the  `code` and `data` segments are both in flat mode:

> base = 0x00000000, limit = 0xfffff





