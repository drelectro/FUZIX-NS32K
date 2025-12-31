#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define UART0_BASE 0xFE0000
#define UART0_RBR  (UART0_BASE + 0x00) // Receiver Buffer Register
#define UART0_THR  (UART0_BASE + 0x00) // Transmitter Holding Register
#define UART0_IER  (UART0_BASE + 0x04) // Interrupt Enable Register
#define UART0_IIR  (UART0_BASE + 0x08) // Interrupt Identification Register
#define UART0_FCR  (UART0_BASE + 0x08) // FIFO Control Register
#define UART0_LCR  (UART0_BASE + 0x0C) // Line Control Register
#define UART0_MCR  (UART0_BASE + 0x10) // Modem Control Register
#define UART0_LSR  (UART0_BASE + 0x14) // Line Status Register
#define UART0_MSR  (UART0_BASE + 0x18) // Modem Status Register
#define UART0_SCR  (UART0_BASE + 0x1C) // Scratch Register
#define UART0_DLL  (UART0_BASE + 0x00) // Divisor Latch Low
#define UART0_DLH  (UART0_BASE + 0x04) // Divisor Latch High

#define UART1_BASE 0xFE0081
#define UART1_RBR  (UART1_BASE + 0x00) // Receiver Buffer Register
#define UART1_THR  (UART1_BASE + 0x00) // Transmitter Holding Register
#define UART1_IER  (UART1_BASE + 0x04) // Interrupt Enable Register
#define UART1_IIR  (UART1_BASE + 0x08) // Interrupt Identification Register
#define UART1_FCR  (UART1_BASE + 0x08) // FIFO Control Register
#define UART1_LCR  (UART1_BASE + 0x0C) // Line Control Register
#define UART1_MCR  (UART1_BASE + 0x10) // Modem Control Register
#define UART1_LSR  (UART1_BASE + 0x14) // Line Status Register
#define UART1_MSR  (UART1_BASE + 0x18) // Modem Status Register
#define UART1_SCR  (UART1_BASE + 0x1C) // Scratch Register
#define UART1_DLL  (UART1_BASE + 0x00) // Divisor Latch Low
#define UART1_DLH  (UART1_BASE + 0x04) // Divisor Latch High

#define UART_REG_SIZE 0x20 // Size of each UART register block

#endif
