OBJECTS = loader.o drivers/IO/io.o memory/SEG/gdt.o drivers/INTERRUPTS/idt.o drivers/INTERRUPTS/interrupt_handler.o kmain.o drivers/FB/fb_driver.o memory/SEG/seg_driver.o drivers/SP/sp_driver.o drivers/INTERRUPTS/interrupt_driver.o drivers/INTERRUPTS/pic_driver.o drivers/INTERRUPTS/kb_driver.o memory/PAGING/common_driver.o memory/PAGING/ep.o memory/PAGING/kheap_driver.o memory/PAGING/paging_driver.o
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf32

all: kernel.elf

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R\
		-b boot/grub/stage2_eltorito\
		-no-emul-boot\
		-boot-load-size 4\
		-A os\
		-input-charset utf8\
		-quiet\
		-boot-info-table\
		-o os.iso\
		iso

run: os.iso
	bochs -f bochsrc.txt -q

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o kernel.elf os.iso bochslog.txt co1.out
	rm -rf iso/boot/kernel.elf
	rm -rf iso/modules/*.o
	rm -rf drivers/*/*.o
	rm -rf memory/*/*.o
