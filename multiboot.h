#define MULTIBOOT_HEADER_MAGIC 0x1badb002

#ifdef __ELF__
#define MULTIBOOT_HEADER_FLAGS 0x00000003
#else
#define MULTIBOOT_HEADER_FLAGS 0x00010003
#endif

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2badb002
#define STACK_SIZE 0x4000

#ifdef HAVE_ASM_USCORE
#define EXT_C(sym) _ ## sym
#else
#define EXT_C(sym) sym
#endif

#ifndef ASM

typedef struct multiboot_header {
  unsigned long magic;
  unsigned long flags;
  unsigned long checksum;
  unsigned long header_addr;
  unsigned long long_addr;
  unsigned long long_end_addr;
  unsigned long bss_end_addr;
  unsigned long entry_addr;
} multiboot_header_t;

typedef struct aout_symbol_table {
  unsigned long tabsize;
  unsigned long strsize;
  unsigned long addr;
  unsigned long reserved;
} aout_symbol_table_t;

typedef struct elf_section_header_table {
  unsigned long num;
  unsigned long size;
  unsigned long addr;
  unsigned long shndx;
} elf_section_header_table_t;

typedef struct multiboot_info {
  unsigned long flags;
  unsigned long mem_lower;
  unsigned long mem_upper;
  unsigned long boot_device;
  unsigned long cmdline;
  unsigned long mods_count;
  unsigned long mods_addr;
  union {
    aout_symbol_table_t auout_sym;
    elf_section_header_table_t elf_sec;
  } u;
  unsigned long mmap_length;
  unsigned long mmap_addr;
} multiboot_info_t;

typedef struct module {
  unsigned long mod_start;
  unsigned long mod_end;
  unsigned long string;
  unsigned long reserved;
} module_t;

typedef struct memory_map {
  unsigned long size;
  unsigned long base_addr_low;
  unsigned long base_addr_high;
  unsigned long length_low;
  unsigned long length_high;
  unsigned long type;
} memory_map_t;

#endif
{/*
#define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit))) 
  printf("flags = 0x%x\n", (unsigned) mbi->flags);
  if(CHECK_FLAG(mbi->flags, 0)) 
    printf("mem_lower =%uKB, mem_dupper = %uKB\n", (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
  if(CHECK_FLAG(mbi->flags, 1))
    printf("boot_device = 0x%x\n", (unsigned) mbi->boot_device);
  if(CHECK_FLAG(mbi->flags, 2)) 
    printf("cmdline = %s\n", (char *) mbi->cmdline);
  if(CHECK_FLAG(mbi->flags, 3)) {
    module_t *mod;
    int i;
    printf("mods_count = %d, mods_addr = 0x%x\n", (int) mbi->mods_count, (int) mbi->mods_addr);
    for(i=0, mod = (module_t *) mbi->mods_addr; i< mbi->mods_count; i++, mod += sizeof(module_t)) 
      printf("mod_start = 0x%x, mod_end = 0x%x, string = %s\n", (unsigned) mod->mod_start, (unsigned) mod->mod_end, (char *) mod->string);
  }
  if(CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
    printf("Both bits 4 and 5 are set.\n");
    return;
  }
  if(CHECK_FLAG(mbi->flags, 4)) {
    aout_symbol_table_t *aout_sym = &(mbi->u.aout_sym);
    printf("aout_symbol_table: tabsize = 0x%0x, strsize = 0x%x, addr = 0x%x\n", (unsigned) aout_sym->tabsize, (unsigned) aout_sym->strsize, (unsigned) aout_sym->addr);
  }
  if(CHECK_FLAG(mbi->flags, 5)) {
    elf_section_header_table_t *elf_sec = &(mbi->u.elf_sec);
    printf("elf_sec: num = %u, size = 0x%x, addr = 0x%x, shndx = 0x%x\n", (unsigned) elf_sec->num, (unsigned) elf_sec->size, (unsigned) elf_sec->addr, (unsigned elf_sec->shndx));
  }
  if(CHECK_FLAG(mbi->flags, 6)) {
    memory_map_t *mmap;
    printf("mmap_addr = 0x%x, mmap_length = 0x%x\n", (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
    for(mmap = (memory_map_t *) mbi->mmap_addr; (unsigned long) mmap < mbi->mmap addr + mbi->mmap_length; mmap = (memory_map_t *) ((unsigned long) mmap + mmap->size +sizeof(mmap->size))) 
      printf("size = 0x%x, base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n", (unsigned) mmap->size, (unsigned) mmap->base_addr_high, (unsigned) mmap->base_addr_low, (unsigned) mmap->length_high, (unsigned) mmap->length_low, (unsigned) mmap->type);
  }
}
*/}
