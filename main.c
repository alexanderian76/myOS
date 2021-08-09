#include "keyboard_map.h"

#include "utils.h"
#include "main.h"
#include "test_process.h"
#include "consts.h"

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01
/* there are 25 lines each of 80 columns; each element takes 2 bytes */



struct IDT_entry {
	unsigned short offset_lowerbits;
	unsigned short selector;
    unsigned char ist;
	unsigned char type_attr;
    unsigned short offset_mid;
	unsigned int offset_higherbits;
    unsigned int zero;
};
//*********************************


extern struct IDT_entry IDT[IDT_SIZE];


extern unsigned long long int isr1;
extern void LoadIDT(void);

void load_idt_entry()
{
   for(unsigned long long int t = 0; t < 256; t++) {
    IDT[t].offset_lowerbits = (unsigned short)(((unsigned long long int)&isr1 & 0x000000000000ffff));
    IDT[t].offset_mid = (unsigned short)(((unsigned long long int)&isr1 & 0x00000000ffff0000) >> 16);
    IDT[t].offset_higherbits = (unsigned int)(((unsigned long long int)&isr1 & 0xffffffff00000000) >> 32);
    IDT[t].selector = 0x08;
    IDT[t].type_attr = 0x8e;
    IDT[t].zero = 0;
    IDT[t].ist = 0;
    
    

    RemapPic();

	outb(0x21, 0xfd);
	outb(0xa1, 0xff);
    
    
    LoadIDT();
   }
}

void outb(unsigned short port, unsigned char val){
  asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

unsigned char inb(unsigned short port){
  unsigned char returnVal;
  asm volatile ("inb %1, %0"
  : "=a"(returnVal)
  : "Nd"(port));
  return returnVal;
}

void RemapPic(){
  unsigned char a1, a2;

  a1 = inb(PIC1_DATA);
  a2 = inb(PIC2_DATA);
  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  outb(PIC1_DATA, 0);
  outb(PIC2_DATA, 8);
  outb(PIC1_DATA, 4);
  outb(PIC2_DATA, 2);
  outb(PIC1_DATA, ICW4_8086);
  outb(PIC2_DATA, ICW4_8086);

  outb(PIC1_DATA, a1);
  outb(PIC2_DATA, a2);

}

void isr1_handler2(void)
{
	kprintch(inb(0x60));
	/* write EOI */
	outb(0x20, 0x20);
	outb(0xa0, 0x20);
}


void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x02;
	}
	set_cursor(current_loc);
}


void kprintch(char str)
{
		vidptr[current_loc++] = str;
		vidptr[current_loc++] = 0x02;
	set_cursor(current_loc);
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
	current_loc = 0;
	set_cursor(current_loc);
}


char tmpCode;
char tmpCodeSecond;
int flag = 0;
int tmp = 0;
int menu[2][10];

void isr1_handler(void)
{
	unsigned char status;
	char keycode;
    unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	/* write EOI */
    outb(0x20, 0x20);
	outb(0xa0, 0x20);

	status = inb(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = inb(KEYBOARD_DATA_PORT);
		if(keycode < 0){
            flag = 0;
            if(tmpCode == CTRL_KEY_CODE && tmpCodeSecond == ENTER_KEY_CODE) {
         //   kprint("CTRL and ENTER button was pushed");
                        if ((int)tmp/10 > 0){
                kprintch((int)tmp/10 + '0');
                kprintch(tmp%10 + '0');
            } else {
                kprintch(tmp + '0');
            }
            tmp = 0;
            }
            tmpCode = "";
            tmpCodeSecond = "";

            return;
        }
        
        switch (keycode) {
            case UP:
                current_loc = (current_loc/160 - 1)*80*2 + current_loc%160;
                set_cursor(current_loc);

                return;
                break;
            case DOWN:
                current_loc = (current_loc/160 + 1)*80*2 + current_loc%160;
                set_cursor(current_loc);

                return;
                break;
            case LEFT:
                current_loc = current_loc - 2;
                set_cursor(current_loc);

                return;
                break;
            case RIGHT:
                current_loc = current_loc + 2;
                set_cursor(current_loc);

                return;
                break;
            default:
                break;
        }
        
        if(flag != 1) {
            tmpCode = keycode;
            flag = 1;
        } else {
            tmpCodeSecond = keycode;
            flag = 0;
        }
        
        
   /*     
        if(tmpCode == CTRL_KEY_CODE && keycode == ENTER_KEY_CODE) {
            clear_screen();
            
            kprint("CTRL and ENTER button was pushed");
            set_cursor(current_loc);
            return;
        }*/
        
		if(keycode == ENTER_KEY_CODE) {
            
            if(menu[0][current_loc / 160] != 0) {
                kprintch(menu[0][current_loc / 160]);

                return;
            }
            
            if(current_loc % 80 == 2 ) {
        
            if(vidptr[current_loc - 2] == 's') {
                kprint("FUNCTION s");
                process();
            } 
        }
            
			kprint_newline();
            set_cursor(current_loc);

			return;
		}
		
		if(keycode == 0x01) {
            clear_screen();
            set_cursor(0);
            current_loc = 0;

            return;
        }
		
        if(keycode == CTRL_KEY_CODE) {
           // clear_screen();
            
          //  kprint("CTRL button was pushed");
            //set_cursor(current_loc);
            draw_box(BOX_DOUBLELINE, 0, 0, BOX_MAX_WIDTH, BOX_MAX_HEIGHT, BRIGHT_GREEN, BLACK);
            draw_box(BOX_SINGLELINE, 5, 3, 20, 5, YELLOW, BLACK);
  
  fill_box(NULL, 36, 5, 30, 10, RED);
  gotoxy(10, 6);
  kprint("Hello World");
  menu[0][current_loc / 160] = current_loc/160;

            return;
        }
        
		vidptr[current_loc++] = keyboard_map[(unsigned char) keycode];
		vidptr[current_loc++] = 0x07;
        set_cursor(current_loc);
        
        tmp = (int)(tmp + (int)keyboard_map[(unsigned char) keycode] - '0');

      //  test_func();
       // vidptr[current_loc++] = tmp + '0';
		//vidptr[current_loc++] = 0x03;
	}
}


void set_cursor(int offset) {
    offset /= 2; // Covert from cell offset to char offset.

    // This is similar to get_cursor, only now we write
    // bytes to those internal device registers.
    outb(REG_SCREEN_CTRL, 14);
    outb(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    outb(REG_SCREEN_CTRL, 15);
    outb(REG_SCREEN_DATA, offset);
}



void MarkLines() 
{
    int row = (current_loc / 160);
    for(int i = 1; i < 160 * 25; i = i + 2)
    {
        if(i % 160 != 1 && i % 160 < 120)
        {
            if (current_loc / 160 == i / 160){
                vidptr[i] = 0x13;
            } 
            else 
            {
                vidptr[i] = 0x02;
            }
        }
    }
    
}


//**********************************************************************************
//**********************************************************************************
//**********************************************************************************

//умножение на 2 практически везде может показаться странным, но это нужно, так как для отображения символа требуется два адреса, первый отвечает за символ, второй за цвет. Поэтому и индексы по два считаем

uint16 get_box_draw_char(uint8 chn, uint8 fore_color, uint8 back_color)
{
  uint16 ax = 0;
  uint8 ah = 0;

  ah = back_color;
  ah <<= 4;
  ah |= fore_color;
  ax = ah;
  ax <<= 8;
  ax |= chn;
 // return '_' + 0x02;
  return ax;
}


void gotoxy(uint16 x, uint16 y)
{
  current_loc = 80*y*2;
  current_loc += x*2;
}

void draw_generic_box(uint16 x, uint16 y, 
                      uint16 width, uint16 height,
                      uint8 fore_color, uint8 back_color,
                      uint8 topleft_ch,
                      uint8 topbottom_ch,
                      uint8 topright_ch,
                      uint8 leftrightside_ch,
                      uint8 bottomleft_ch,
                      uint8 bottomright_ch)
{
  uint32 i;

  //increase vga_index to x & y location
  current_loc = 80*y*2;
  current_loc += x*2;

  //draw top-left box character
 // vidptr[current_loc] = get_box_draw_char(topleft_ch, fore_color, back_color);
kprintch(topleft_ch);
//  current_loc++;
  //draw box top characters, -
  for(i = 0; i < width; i++){
  //  vidptr[current_loc] = get_box_draw_char(topbottom_ch, fore_color, back_color);
    //current_loc++;
      kprintch(topbottom_ch);
  }

  //draw top-right box character
 // vidptr[current_loc] = get_box_draw_char(topright_ch, fore_color, back_color);
kprintch(topright_ch);
  // increase y, for drawing next line
  y++;
  // goto next line
  current_loc = 80*y*2;
  current_loc += x*2;

  //draw left and right sides of box
  for(i = 0; i < height; i++){
    //draw left side character
  //  vidptr[current_loc] = get_box_draw_char(leftrightside_ch, fore_color, back_color);
    //current_loc++;
      kprintch(leftrightside_ch);
    //increase vga_index to the width of box
    current_loc += width*2;
    //draw right side character
  //  vidptr[current_loc] = get_box_draw_char(leftrightside_ch, fore_color, back_color);
    kprintch(leftrightside_ch);
    //goto next line
    y++;
    current_loc = 80*y*2;
    current_loc += x*2;
  }
  //draw bottom-left box character
  //vidptr[current_loc] = get_box_draw_char(bottomleft_ch, fore_color, back_color);
  //current_loc++;
  kprintch(bottomleft_ch);
  //draw box bottom characters, -
  for(i = 0; i < width; i++){
  //  vidptr[current_loc] = get_box_draw_char(topbottom_ch, fore_color, back_color);
    //current_loc++;
      kprintch(topbottom_ch);
  }
  //draw bottom-right box character
  //vidptr[current_loc] = get_box_draw_char(bottomright_ch, fore_color, back_color);
kprintch(bottomright_ch);
  current_loc = 0;
}

void draw_box(uint8 boxtype, 
              uint16 x, uint16 y, 
              uint16 width, uint16 height,
              uint8 fore_color, uint8 back_color)
{
  switch(boxtype){
    case BOX_SINGLELINE : 
      draw_generic_box(x, y, width, height, 
                      fore_color, back_color, 
                      218, 196, 191, 179, 192, 217);
      break;

    case BOX_DOUBLELINE : 
      draw_generic_box(x, y, width, height, 
                      fore_color, back_color, 
                      201, 205, 187, 186, 200, 188);
      break;
  }
}

void fill_box(uint8 ch, uint16 x, uint16 y, uint16 width, uint16 height, uint8 color)
{
  uint32 i,j;

  for(i = 0; i < height; i++){
    //increase vga_index to x & y location
    current_loc = 80*y*2;
    current_loc += x*2;

    for(j = 0; j < width; j++){
      vidptr[current_loc++] = 0;
      vidptr[current_loc++] = color;
      //get_box_draw_char(ch, 0, color);
     // current_loc++;
    }
    y++;
  }
}

//**********************************************************************************
//**********************************************************************************
//**********************************************************************************

void main()
{
    const char *str = "my first kernel";
    char *vidptr = (char*)0xb8000;
    unsigned int i = 0;
    unsigned int j = 0;

    while(j < 80 * 25 * 2) {
        vidptr[j] = ' ';
        vidptr[j+1] = 0x07;         
        j = j + 2;
    }

    j = 0;

    while(str[j] != '\0') {
        vidptr[i] = str[j];
        vidptr[i+1] = 0x02;
        ++j;
        i = i + 2;
    }
    
	kprint_newline();
	kprint_newline();

    load_idt_entry();

	while(1)__asm__("hlt\n\t");
    return;
} 
