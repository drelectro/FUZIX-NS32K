OUTPUT_ARCH(ns32k)

SEARCH_DIR(.)


MEMORY
{
   ram (rwx) : ORIGIN = 0x02000, LENGTH = 0x6E000 
   rom (rx) : ORIGIN = 0x70000, LENGTH = 0x10000  

   /* ram (rwx) : ORIGIN = 0x02000, LENGTH = 0x7E000 
   rom (rx) : ORIGIN = 0xF02000, LENGTH = 0x1E000 */
}

/*
 * Place "code" in ROM section and data in RAM 
 * 
 */
SECTIONS
{
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    *(.text .text.*)

    . = ALIGN(0x4);

    *(.rodata .rodata.*)

    . = ALIGN(0x4);
    _etext = .;
    *(.lit)
  } > rom

  /*
   * The ns32k toolchain will otherwise keep advancing '.' from the ROM
   * placement of .text, so explicitly reset '.' to RAM before placing
   * the run-time data sections.
   */
  . = ORIGIN(ram);

  .data :
  {
    . = ALIGN(0x4);
    __data_start = .;
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.*)
    . = ALIGN(0x4);
    __data_end = .;
    _edata = .;
  } > ram AT > rom

  __data_load = LOADADDR(.data);

  .bss (NOLOAD) :
  {
    . = ALIGN(0x4);
    __bss_start = . ;
    *(.shbss)
    *(.bss .bss.*)
    *(COMMON)
    _end =  ALIGN (0x8);
    __end = _end;
  } > ram

  .stab 0 (NOLOAD) :
  {
    *(.stab)
  }

  .stabstr 0 (NOLOAD) :
  {
    *(.stabstr)
  }
}
