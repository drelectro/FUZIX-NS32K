#include "../cpu-ns32k/kernel-ns32k.def"
/*
 *	Lots left to fill in
 */
	.globl	_plt_reboot
	.globl	init_early
	.globl	init_hardware
	.globl  init_icu
	.globl	_program_vectors
	.globl	outchar
	.globl	_plt_monitor
	.globl	_udata_block
	.globl	_get_usp
	.globl	_set_usp

#
#	Put early as needs to be in the low 64K
#

	.align	2
module:
sbase:	.long	0
	.long	0
	.long	0
	.long	0
inttab:
	# Non vectored external IRQ (all in our case)
	.word	0
	.word	interrupt_handler
	# Non maskable interrupt (not wired)
	.word	0
	.word	nmi_handler
	.word	0
	.word	abort_handler
	.word	0
	.word	slave_handler
	.word	0
	.word	illegal_handler
	.word	0
	.word	syscall_handler
	.word	0
	.word	divzero_handler
	.word	0
	.word	flag_handler
	.word	0
	.word	breakpoint_handler
	.word	0
	.word	trace_handler
	.word	0
	.word	undefined_handler
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	# Vectored interrupts follow (non used)
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler
	.word	0
	.word	interrupt_handler


_plt_reboot:
_plt_monitor:
	sprw	psr,r0		# Interrupts back off
	andw	0xF7FF,r0
	lprw	psr,r0
	br		_plt_monitor

init_early:
	addr	_udata_block(pc),_udata_shadow(pc)
	addr	inttab,r0	
	lprd	intbase,r0
	addr	module,r0
	lprw	mod,r0
	movd	sbase,r0
	lprd	sb,r0
	ret	0

#
#	FIXME: could be in discard if we wanted
#
init_hardware:
# set system RAM size(hardcode hacks for now) TODO
	movw	256,_ramsize(pc)
	movw	256-64,_procmem(pc)		# guesses for now
	bsr	_program_vectors
# TODO timer enable etc
	ret	0

init_icu:

	# 
	# This is all a bit odd
	# I can't get the upper interupts (IR8-15) to work at all
	# Furthermore if I write anything to IMSKH (even 0xFF - Mask All)  
	# then we only get a single interrupt from IR0-7 
	#

	movb 	0xFF, @0xFFFE04 	# ELTG - IR0-7 Level Sensitive 
	movb 	0xFF, @0xFFFE08 	# ELTG - IR8-15 Level Sensitive

	movb 	0xFF, @0xFFFE10 	# TPL - Configure IR0-7 as active high 
	movb 	0xFF, @0xFFFE14 	# TPL - Configure IR8-15 as active high

	movb 	0x40, @0xFFFE58 	# CCTL - Enable Clock/4 prescaler
	movb  	0x00, @0xFFFE40 	# MCTL - Enable ICU Clock

	# movb    0xDF, @0xFFFE28 	# IMSKL Unmask IR5 (UART0) 
	movb    0xFF, @0xFFFE28 	# IMSKL Mask IR0-7 

	 movb    0x7F, @0xFFFE2C 	# IMSKH Unmask IR15 (UART0)
	# movb    0xFF, @0xFFFE2C 	# IMSKH Mask IR8-15

	ret 0

#
#	Nothing to do in NS32K - all set up once at boot
#
_program_vectors:
	ret	0

#
#	We do no banking so we need to do nothing here.
#
map_process_always:
map_process:
map_kernel:
map_restore:
map_save:
	ret	0


# outchar: Wait for UART TX idle, then print the char in r0
outchar:
	enter 	[r1],0
	addr 	@0xFE0000, r1
outcharw:
	tbitb   5, 0x14(r1)
	bfc		outcharw
	movb	r0, 0x00(r1)
	exit	[r1]
	ret	0


#
#	Later versions of the CPU will need these
#
_flush_icache:
#	cinv	A,I,r0
#	ret	0
_flush_dcache:
#	cinv	A,D,r0
#	ret	0
_enable_icache:
_enable_dcache:
	ret	0

_get_usp:
	bispsrw	SPSR_S		# Select SP1 (User)
	sprd	sp,r0		# R1 = SP
	bicpsrw	SPSR_S		# Back to SP0 (Supervisor)
	ret	0

_set_usp:
	enter	[],0
	movd	4(fp),r0
	bispsrw	SPSR_S		# Select SP1 (User)
	lprd	sp,r0		# R1 = SP
	bicpsrw	SPSR_S		# Back to SP0 (Supervisor)
	ret	0
