OUTPUT_FORMAT("pei-i386")
OUTPUT_ARCH(i386)
ENTRY(efi_main)
SECTIONS
{
  . = 0;
  PROVIDE(_GLOBAL_OFFSET_TABLE_ = .);
  ImageBase = .;
  . = 1;

  . = ALIGN(4096);
  .text :
  {
   _text = .;
   *(.text)
   *(.text.*)
   . = ALIGN(16);
   _etext = .;
  }

  . = ALIGN(4096);
  .data :
  {
   _data = .;
   *(.rodata*)
   *(.data)
   *(.data1)
   *(.data.*)
   *(.got.plt)
   *(.got)
   *(.bss)
   *(COMMON)
   _edata = .;
  }

  . = ALIGN(4096);
  .reloc :		/* This is the PECOFF .reloc section! */
  {
    *(.reloc)
  }

  /DISCARD/ :
  {
    *(.rel.reloc)
    *(.eh_frame)
    *(.note.GNU-stack)
    *(.comment)
    *(.debug*)
  }
}
