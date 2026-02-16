/*
 * ppide.c - Parallel Port IDE interface routines for Timewarp platform
 * Ref: https://www.pjrc.com/tech/8051/ide/wesley.html
 *
 */

#include <kernel.h>
#include <blkdev.h>
#include <devide.h>
#include <printf.h>


#ifndef CONFIG_MULTI_IDE
#define ppide_readb		    devide_readb
#define ppide_writeb		devide_writeb
#define ppide_read_data		devide_read_data
#define ppide_write_data	devide_write_data
#endif

#ifdef CONFIG_PPIDE

#define PPIDE_REG_DATA		0
#define PPIDE_REG_STATUS	0x98    // 7 status/command translated
#define PPIDE_RESET         0x01


void out(uint16_t port, uint8_t val)
{
    *((volatile uint8_t *)PPIDE_BASE+port) = val;
}
uint8_t in(uint16_t port)
{
    return *((volatile uint8_t *)PPIDE_BASE+port);
}

void ppide_init(void)
{
    kputs("ppide_init\n");
    out(PPIDE_CONTROL, PPIDE_PPI_BUS_READ);
    //out(PPIDE_PORTC, PPIDE_RESET);
    out(PPIDE_PORTC, PPIDE_REG_STATUS);
}

uint_fast8_t ppide_readb(uint_fast8_t p)
{
    uint8_t r;

    out(PPIDE_PORTC, p | PPIDE_CS0_LINE);
    out(PPIDE_PORTC, p | PPIDE_CS0_LINE | PPIDE_RD_LINE);
    r = in(PPIDE_PORTA);
    out(PPIDE_PORTC, p | PPIDE_CS0_LINE); // CS may only go inactive after RD
    out(PPIDE_PORTC, 0);
    //kprintf("ppide_readb %x=%x\n", p, r);
    return r;
}

void ppide_writeb(uint8_t p, uint_fast8_t v)
{
    //kprintf("ppide_writeb %x=%x\n", p, v);
    out(PPIDE_CONTROL, PPIDE_PPI_BUS_WRITE);
    out(PPIDE_PORTC, p | PPIDE_CS0_LINE);
    out(PPIDE_PORTA, v);
    out(PPIDE_PORTB, 0);
    out(PPIDE_PORTC, p | PPIDE_WR_LINE | PPIDE_CS0_LINE);
    out(PPIDE_PORTC, p | PPIDE_CS0_LINE); // CS may only go inactive after WR
    out(PPIDE_PORTC, 0);
    out(PPIDE_CONTROL, PPIDE_PPI_BUS_READ);
}

/* Flat memory model so this is not too difficult */

void ppide_read_data(void)
{
    unsigned int ct = 256;
    uint8_t *p = blk_op.addr;
    //kputs("ppide_read_data\n");
    while(ct--) {
        out(PPIDE_PORTC, PPIDE_REG_DATA|PPIDE_RD_LINE|PPIDE_CS0_LINE);
        *p++ = in(PPIDE_PORTA);
        *p++ = in(PPIDE_PORTB);
        out(PPIDE_PORTC, PPIDE_REG_DATA|PPIDE_CS0_LINE);
    }
    out(PPIDE_PORTC, 0);
}

void ppide_write_data(void)
{
    unsigned int ct = 256;
    uint8_t *p = blk_op.addr;
    //kputs("ppide_write_data\n");
    out(PPIDE_PORTC, PPIDE_REG_DATA | PPIDE_CS0_LINE);
    out(PPIDE_CONTROL, PPIDE_PPI_BUS_WRITE);
    while(ct--) {
        out(PPIDE_PORTA, *p++);
        out(PPIDE_PORTB, *p++);
        out(PPIDE_PORTC, PPIDE_REG_DATA | PPIDE_WR_LINE);
        out(PPIDE_PORTC, PPIDE_REG_DATA | PPIDE_CS0_LINE);
    }
    out(PPIDE_CONTROL, PPIDE_PPI_BUS_READ);
}    

#endif
