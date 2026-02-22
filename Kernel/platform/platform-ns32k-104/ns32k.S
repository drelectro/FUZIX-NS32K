#include "../cpu-ns32k/kernel-ns32k.def"

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

#define INT_NVI 0
#define INT_NMI 1
#define INT_ABT 2
#define INT_SLV 3
#define INT_ILL 4
#define INT_SVC 5
#define INT_DVZ 6
#define INT_FLG 7
#define INT_BPT 8
#define INT_TRC 9
#define INT_UND 10

#define INT_VEC0 16
#define INT_VEC1 17
#define INT_VEC2 18
#define INT_VEC3 19
#define INT_VEC4 20
#define INT_VEC5 21
#define INT_VEC6 22
#define INT_VEC7 23
#define INT_VEC8 24
#define INT_VEC9 25
#define INT_VEC10 26
#define INT_VEC11 27
#define INT_VEC12 28
#define INT_VEC13 29
#define INT_VEC14 30
#define INT_VEC15 31

_plt_reboot:
_plt_monitor:
	sprw	psr,r0		# Interrupts back off
	andw	0xF7FF,r0
	lprw	psr,r0
	br		_plt_monitor

set_interrupt_table_entry:
	sprd    intbase, r3			# Get intbase
	movw r2, r3 [r0:d]			# Set module
	subd start, r1				# Adjust the handler address to be an offset from the module "program base"
								# In our case "program base" always == start.
	movw r1, 2(r3) [r0:d]		# Set handler
	ret 0


init_early:
	addr	_udata_block(pc),_udata_shadow(pc)
	
	# Set up interrupt table
	#
	# At present all interrupt table entries use same module table entry and thus the same 64K block
	# 
	#
	movw 0x0020,r2				# r2 = module (same for all interupts at present)

	movd INT_NVI, r0 			# r0 = Index	
	movd nvi_handler, r1  		# r1 = Handler 
	jsr  set_interrupt_table_entry

	movd INT_NMI, r0 				
	movd nmi_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_ABT, r0 				
	movd abort_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_SLV, r0 				
	movd slave_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_ILL, r0 				
	movd illegal_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_SVC, r0 				
	movd syscall_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_DVZ, r0 				
	movd divzero_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_FLG, r0 				
	movd flag_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_BPT, r0 				
	movd breakpoint_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_TRC, r0 				
	movd trace_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_UND, r0 				
	movd undefined_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_VEC0, r0 				
	movd timer_handler, r1  	
	jsr  set_interrupt_table_entry

	movd INT_VEC15, r0 				
	movd interrupt_handler, r1  	
	jsr  set_interrupt_table_entry

	ret	0

#
#	FIXME: could be in discard if we wanted
#
init_hardware:
	# set system RAM size(hardcode hacks for now) TODO
	movw	512,_ramsize(pc)
	movw	512-64,_procmem(pc)		# guesses for now
	bsr	_program_vectors

	ret	0

init_icu:

	#
	# Set up System timer
	#
	movb 	0xC0, @0xFFFE58 	# CCTL  - CFPNS Enable Clock/4 prescaler
								#		- CCON Concatenate H and L timers
								#		- !CRUNH Run H timer
								# CCTL is not affected by reset, halt the H timer
								# Whilst we jigger with it


	# Set 32 bit timer to to 600,000 = 0x0927C0
	# Apparently the counter isn't affected by the prescaler.
	#
	movb	0xC0, @0xFFFE60 # LCSV
	movb	0x27, @0xFFFE64 
	movb 	0x09, @0xFFFE68 # HCSV
	movb	0x00, @0xFFFE6C

	movb	0xC0, @0xFFFE70 # LCCV
	movb	0x27, @0xFFFE74
	movb	0x09, @0xFFFE78 # HCCV
	movb	0x00, @0xFFFE7C 

	movb 	0x00, @0xFFFE48 # CIPTR - Set H & L timer to Interrupt 0
	movb 	0x30, @0xFFFE5C # CICTL - Enable H timer interrupt 

	#
	# Set up Interrupts
	# 

	#movb 	0x00, @0xFFFE20 	# Clear ISRVL 
	#movb 	0x00, @0xFFFE24 	# Clear ISRVH 

	movb 	0x10, @0xFFFE04		# SVCT - Set vector "bias" to 0001xxxx (+16)

	movb 	0xFF, @0xFFFE08 	# ELTG - IR0-7 Level Sensitive 
	movb 	0xFF, @0xFFFE0C 	# ELTG - IR8-15 Level Sensitive

	movb 	0xFF, @0xFFFE10 	# TPL - Configure IR0-7 as active high 
	movb 	0xFF, @0xFFFE14 	# TPL - Configure IR8-15 as active high

	#
	# Enable clock and timers
	#

	movb 	0xC8, @0xFFFE58 	# CCTL  - CFPNS Enable Clock/4 prescaler
								#		- CCON Concatenate H and L timers
								#		- CRUNH Run H timer

	movb  	0x00, @0xFFFE40 	# MCTL  - COUTD - Enable ICU Clock

	# PDIR - Set G0 as output not required, all other pins input by default
	movb	0x01, @0xFFFE44 	# OCASN - Send timer output to G0
	movb	0xFE, @0xFFFE50		# IPS - Set G0 as I/O (disable pin interrupt)

	#
	# Enable Interrupts
	#

	# movb    0xDF, @0xFFFE28 	# IMSKL Unmask IR5 (UART0) 
	movb    0xF7, @0xFFFE28 	# IMSKL Unmask IR0 (ICU) Mask IR1-7 

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
