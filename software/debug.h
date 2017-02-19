#ifndef _DEBUG_H_
#define _DEBUG_H_


//----- DEFINES -----
#ifndef DEBUG
//# define DEBUG                        //switch debug output on
#endif

#define DEBUGBAUD                       115200   //uart baud rate: 115200
#define DEBUGUART                       UART2_BASE //uart: UART1_BASE UART2_BASE

#define COMPILE_YEAR  ((((__DATE__ [7]-'0')*10+(__DATE__[8]-'0'))*10+(__DATE__ [9]-'0'))*10+(__DATE__ [10]-'0'))

#define COMPILE_MONTH ( (__DATE__[2] == 'n') ? (__DATE__[1] == 'a' ? 1 : 6) \
                      : (__DATE__[2] == 'b') ? 2  \
                      : (__DATE__[2] == 'r') ? (__DATE__[1] == 'a' ? 3 : 4) \
                      : (__DATE__[2] == 'y') ? 5  \
                      : (__DATE__[2] == 'l') ? 7  \
                      : (__DATE__[2] == 'g') ? 8  \
                      : (__DATE__[2] == 'p') ? 9  \
                      : (__DATE__[2] == 't') ? 10 \
                      : (__DATE__[2] == 'v') ? 11 : 12)

#define COMPILE_DAY   ((((__DATE__ [4]==' ')?0:(__DATE__[4]-'0'))*10)+(__DATE__[5]-'0')) 

//----- PROTOTYPES -----
#ifndef __GNUC__
/* keep old behaviour for non gcc (strict ANSI)  */
#define DEBUGOUT                        debug_out
#else
# ifdef DEBUG
/* this is not strict ANSI, but gcc can handle variable parameter macros */
#  define DEBUGOUT(x,y...)                debug_out(x, ##y);
#else
/* remove corresponding code entirely from the binary */
#  define DEBUGOUT(x,y...)
#endif
#endif
void                                   debug_out(const char *s, ...);
void                                   uart_puts(const char *s);
void                                   uart_putc(unsigned int c);
void                                   debug_init(void);


#endif //_DEBUG_H_
