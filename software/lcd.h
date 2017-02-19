#ifndef _LCD_H_
#define _LCD_H_


//----- DEFINES -----
#define SMALLFONT                      (0)
#define SMALLFONT_NAME                 font1
#define SMALLFONT_START                FONT1_START
#define SMALLFONT_WIDTH                FONT1_WIDTH
#define SMALLFONT_HEIGHT               FONT1_HEIGHT
#define NORMALFONT                     (1)
#define NORMALFONT_NAME                font2
#define NORMALFONT_START               FONT2_START
#define NORMALFONT_WIDTH               FONT2_WIDTH
#define NORMALFONT_HEIGHT              FONT2_HEIGHT
#define TIMEFONT                       (2)
#define TIMEFONT_NAME                  font3
#define TIMEFONT_START                 FONT3_START
#define TIMEFONT_WIDTH                 FONT3_WIDTH
#define TIMEFONT_HEIGHT                FONT3_HEIGHT

#if defined(LCD_MIO283QT1) || defined(LCD_MIO283QT2)
# define _LCD_WIDTH                     (320)
# define _LCD_HEIGHT                    (240)
#else //S65 Displays: L2F50, LPH88, LS020
# define _LCD_WIDTH                     (176)
# define _LCD_HEIGHT                    (132)
#endif
#ifdef LCD_ROTATE
# define LCD_WIDTH                     _LCD_HEIGHT
# define LCD_HEIGHT                    _LCD_WIDTH
#else
# define LCD_WIDTH                     _LCD_WIDTH
# define LCD_HEIGHT                    _LCD_HEIGHT
#endif

#define RGB(r,g,b)                     (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue
#define GET_RED(x)                     ((x>>8)&0xF8) //5 red
#define GET_GREEN(x)                   ((x>>3)&0xFC) //6 green
#define GET_BLUE(x)                    ((x<<3)&0xF8) //5 blue


typedef struct __attribute__((packed))
{
  uint8_t  magic[2];
  uint32_t size;
  uint16_t rsrvd1;
  uint16_t rsrvd2;
  uint32_t offset;
} BMP_Header;


typedef struct __attribute__((packed))
{
  uint32_t size;
  uint32_t width;
  uint32_t height;
  uint16_t nplanes;
  uint16_t bitspp;
  uint32_t compress;
  uint32_t isize;
  uint32_t hres;
  uint32_t vres;
  uint32_t colors;
  uint32_t impcolors;
} BMP_DIPHeader;


//----- PROTOTYPES -----
unsigned int                           lcd_checkbit(const unsigned long *data, unsigned int nr);

void                                   lcd_drawbmp(char *file, unsigned int x, unsigned int y);
void                                   lcd_drawimg32(int x, unsigned int y, const unsigned char *img, unsigned int color, unsigned int bgcolor);
void                                   lcd_putlinebr(unsigned int x, unsigned int y, const char *s, unsigned int font, unsigned int size, unsigned int color, unsigned int bgcolor);
void                                   lcd_putline(unsigned int x, unsigned int y, const char *s, unsigned int font, unsigned int size, unsigned int color, unsigned int bgcolor);
unsigned int                           lcd_puts(unsigned int x, unsigned int y, const char *s, unsigned int font, unsigned int size, unsigned int color, unsigned int bgcolor);
unsigned int                           lcd_putc(unsigned int x, unsigned int y, unsigned int c, unsigned int font, unsigned int size, unsigned int color, unsigned int bgcolor);
void                                   lcd_fillcircle(unsigned int x0, unsigned int y0, unsigned int radius, unsigned int color);
void                                   lcd_drawcircle(unsigned int x0, unsigned int y0, unsigned int radius, unsigned int color);
void                                   lcd_fillrect(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int color);
void                                   lcd_drawrect(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int color);
void                                   lcd_drawline(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int color);
void                                   lcd_drawpixel(unsigned int x, unsigned int y, unsigned int color);
void                                   lcd_clear(unsigned int color);
void                                   lcd_init(void);


#endif //_LCD_H_
