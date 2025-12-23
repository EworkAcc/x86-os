#ifndef INCLUDE_PIC_H
#define INCLUDE_PIC_H

#define PIC_1 0x20
#define PIC_2 0xa0
#define PIC_1_COMMAND	PIC_1
#define PIC_1_DATA	(PIC_1+1)
#define PIC_2_COMMAND	PIC_2
#define PIC_2_DATA	(PIC_2+1)
#define PIC_1_OFFSET 0x20
#define PIC_2_OFFSET 0x28
#define PIC_2_END PIC_2_OFFSET + 7
#define PIC_1_COMMAND_PORT 0x20
#define PIC_2_COMMAND_PORT 0xa0
#define PIC_ACKNOWLEDGE 0x20

#define PIC_ICW1_ICW4            0x01
#define PIC_ICW1_SINGLE          0x02
#define PIC_ICW1_INTERVAL4       0x04
#define PIC_ICW1_LEVEL           0x08
#define PIC_ICW1_INIT            0x10
#define PIC_ICW4_8086            0x01
#define PIC_ICW4_AUTO            0x02
#define PIC_ICW4_BUF_SLAVE       0x08
#define PIC_ICW4_BUF_MASTER      0x0c
#define PIC_ICW4_SFNM            0x10

void pic_remap(int offset1, int offset2);
void pic_acknowledge(unsigned int interrupt);

#endif
