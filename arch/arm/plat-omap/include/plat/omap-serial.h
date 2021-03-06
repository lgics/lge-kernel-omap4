/*
 * Driver for OMAP-UART controller.
 * Based on drivers/serial/8250.c
 *
 * Copyright (C) 2010 Texas Instruments.
 *
 * Authors:
 *	Govindraj R	<govindraj.raja@ti.com>
 *	Thara Gopinath	<thara@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __OMAP_SERIAL_H__
#define __OMAP_SERIAL_H__

#include <linux/serial_core.h>
#include <linux/platform_device.h>

#include <plat/control.h>
#include <plat/mux.h>

#define DRIVER_NAME	"omap-hsuart"

/*
 * Use tty device name as ttyO, [O -> OMAP]
 * in bootargs we specify as console=ttyO0 if uart1
 * is used as console uart.
 */
#define OMAP_SERIAL_NAME	"ttyO"

#define OMAP_MDR1_DISABLE	0x07
#define OMAP_MDR1_MODE13X	0x03
#define OMAP_MDR1_MODE16X	0x00
#define OMAP_MODE13X_SPEED	230400

#define DMA_FOR_UART2

#ifdef DMA_FOR_UART2
 /*
 * From OMAP4430 ES 2.0 onwards set
 * tx_threshold while using UART in DMA Mode
 * and ensure tx_threshold + tx_trigger <= 63
 */
#define UART_MDR3		0x20
#define SET_DMA_TX_THRESHOLD	0x04
#define UART_TX_DMA_THRESHOLD	0x21
/* Setting TX Threshold Level to 62 */
#define TX_FIFO_THR_LVL	0x3E
#endif

/*
 * From OMAP4430 ES 2.0 onwards set
 * tx_threshold while using UART in DMA Mode
 * and ensure tx_threshold + tx_trigger <= 63
 */
#define UART_MDR3		0x20
#define SET_DMA_TX_THRESHOLD	0x04
#define UART_TX_DMA_THRESHOLD	0x21
/* Setting TX Threshold Level to 62 */
#define TX_FIFO_THR_LVL	0x3E

/*
 * LCR = 0XBF: Switch to Configuration Mode B.
 * In configuration mode b allow access
 * to EFR,DLL,DLH.
 * Reference OMAP TRM Chapter 17
 * Section: 1.4.3 Mode Selection
 */
#define OMAP_UART_LCR_CONF_MDB	0xBF
#define OMAP_UART_LCR_CONF_MOPER 0x00
#define OMAP_UART_LCR_CONF_MDA  0x80

/* WER = 0x7F
 * Enable module level wakeup in WER reg
 */
#define OMAP_UART_WER_MOD_WKUP	0X7F

/* Enable XON/XOFF flow control on output */
#define OMAP_UART_SW_TX		0x04

/* Enable XON/XOFF flow control on input */
#define OMAP_UART_SW_RX		0x04

#define OMAP_UART_SYSC_RESET	0X07
#define OMAP_UART_TCR_TRIG	0X0F
#define OMAP_UART_SW_CLR	0XF0
#define OMAP_UART_FIFO_CLR	0X06

#define OMAP_UART_DMA_CH_FREE	-1

#define RX_TIMEOUT		(3 * HZ)
#define OMAP_MAX_HSUART_PORTS	4
#define UART1                  (0x0)
#define UART2                  (0x1)
#define UART3                  (0x2)
#define UART4                  (0x3)

#define MSR_SAVE_FLAGS		UART_MSR_ANY_DELTA

struct omap_uart_port_info {
	unsigned long		reserved1;	/* Reserved 1 */
	void __iomem		*membase;	/* ioremap cookie or NULL */
	resource_size_t		mapbase;	/* resource base */
	unsigned int		reserved2;	/* Reserved 2 */
	unsigned long		irqflags;	/* request_irq flags */
	unsigned int		uartclk;	/* UART clock rate */
	void			*reserved3;	/* Reserved 3 */
	unsigned char		regshift;	/* register shift */
	unsigned char		reserved4;	/* Reserved 4 */
	unsigned char		reserved5;	/* Reserved 5 */
	upf_t			flags;		/* UPF_* flags */

	/* Till here the structure has been made similar
	 * to platform serial 8250. This would allow the
	 * both the drivers to co-exist in future.
	 */
	/* beyond this is the platform specific fields */
	int                     use_dma;        /* DMA Enable / Disable */
	int                     dma_rx_buf_size;/* DMA Rx Buffer Size */
	int                     dma_rx_timeout; /* DMA RX timeout */
	unsigned int            idle_timeout;   /* Omap Uart Idle Time out */
#ifdef DMA_FOR_UART2
	u8			omap4_tx_threshold;
#endif
};

struct uart_omap_dma {
	u8			uart_dma_tx;
	u8			uart_dma_rx;
	int			rx_dma_channel;
	int			tx_dma_channel;
	dma_addr_t		rx_buf_dma_phys;
	dma_addr_t		tx_buf_dma_phys;
	unsigned int		uart_base;
	/*
	 * Buffer for rx dma.It is not required for tx because the buffer
	 * comes from port structure.
	 */
	unsigned char		*rx_buf;
	unsigned int		prev_rx_dma_pos;
	int			tx_buf_size;
	int			tx_dma_used;
	int			rx_dma_used;
	spinlock_t		tx_lock;
	spinlock_t		rx_lock;
	/* timer to poll activity on rx dma */
	struct timer_list	rx_timer;
	int			rx_buf_size;
	int			rx_timeout;
#ifdef DMA_FOR_UART2
	u8			tx_threshold;
#endif

};

struct uart_omap_port {
	struct uart_port	port;
	struct uart_omap_dma	uart_dma;
	struct platform_device	*pdev;

	unsigned char		ier;
	unsigned char		lcr;
	unsigned char		mcr;
	unsigned char		fcr;
	unsigned char		efr;

	int			use_dma;
	/*
	 * Some bits in registers are cleared on a read, so they must
	 * be saved whenever the register is read but the bits will not
	 * be immediately processed.
	 */
	unsigned int		lsr_break_flag;
	unsigned char		msr_saved_flags;
	char			name[20];
	unsigned long		port_activity;
};

int omap_uart_active(int num);
#ifdef CONFIG_PM
void omap_uart_enable_clock_from_irq(int uart_num);
void omap_uart_mdr1_errataset(int uart_no, u8 mdr1_val, u8 fcr_val);
#endif
#endif /* __OMAP_SERIAL_H__ */
