#ifndef LYCOS_C_GRAPHIC_H
#define LYCOS_C_GRAPHIC_H

#define COLOR8_BLACK 0
#define COLOR8_BRIGHT_RED 1
#define COLOR8_BRIGHT_GREEN 2
#define COLOR8_BRIGHT_YELLOW 3
#define COLOR8_BRIGHT_BLUE 4
#define COLOR8_BRIGHT_PURPLE 5
#define COLOR8_LIGHT_BRIGHT_BLUE 6
#define COLOR8_WHITE 7
#define COLOR8_BRIGHT_GREY 8
#define COLOR8_DARK_RED 9
#define COLOR8_DARK_GREEN 10
#define COLOR8_DARK_YELLOW 11
#define COLOR8_DARK_CYAN 12
#define COLOR8_DARK_PURPLE 13
#define COLOR8_LIGHT_DARK_BLUE 14
#define COLOR8_DARK_GREY 15

extern char ascfont[4096];

void set_palette(int start, int end, unsigned char *rgb);
void init_palette();
void box_fill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void put_ascii_font8(unsigned char *vram, int xsize, int x, int y, char c, char *font);
void put_ascii_str8(unsigned char *vram, int xsize, int x, int y, char c, char *str);

#endif
