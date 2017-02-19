#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/inc/hw_ints.h"
#include "lmi/driverlib/sysctl.h"
#include "lmi/driverlib/gpio.h"
#include "lmi/driverlib/interrupt.h"
#include "lmi/driverlib/uart.h"
#include "debug.h"
#include "tools.h"


#ifdef DEBUG
void __error__(char *pcFilename, unsigned long ulLine) //called if the DriverLib encounters an error
{
  DEBUGOUT("DLib: %s:%i\n", pcFilename, ulLine);

  return;
}
#endif


void debug_out(const char *s, ...)
{
#ifdef DEBUG
  unsigned int i, move;
  char c, str[16], *ptr;
  va_list ap;

  va_start(ap, s);

  for(;;)
  {
    c = *s++;

    if(c == 0)
    {
      break;
    }
    else if(c == '%')
    {
      c = *s++;
      if(isdigit(c) > 0)
      {
        move = c-'0';
        c = *s++;
      }
      else
      {
        move = 0;
      }

      switch(c)
      {
        case 's':
          ptr = va_arg(ap, char *);
          uart_puts(ptr);
          break;
        case 'b': //bin
          ltoa(va_arg(ap, long), str, 2);
          if(move)
          {
            for(i=0; str[i]; i++);
            for(; move>i; move--)
            {
              uart_putc('0');
            }
          }
          uart_puts(str);
          break;
        case 'i': //dec
          ltoa(va_arg(ap, long), str, 10);
          if(move)
          {
            for(i=0; str[i]; i++);
            for(; move>i; move--)
            {
              uart_putc('0');
            }
          }
          uart_puts(str);
          break;
        case 'u': //unsigned dec
          ultoa(va_arg(ap, unsigned long), str, 10);
          if(move)
          {
            for(i=0; str[i]; i++);
            for(; move>i; move--)
            {
              uart_putc('0');
            }
          }
          uart_puts(str);
          break;
        case 'x': //hex
          ltoa(va_arg(ap, long), str, 16);
          if(move)
          {
            for(i=0; str[i]; i++);
            for(; move>i; move--)
            {
              uart_putc('0');
            }
          }
          uart_puts(str);
          break;
      }
    }
    else
    {
      uart_putc(c);
    }
  }

  va_end(ap);
#endif
  return;
}


void uart_puts(const char *s)
{
  while(*s)
  {
    uart_putc(*s++);
  }

  return;
}


void uart_putc(unsigned int c)
{
  UARTCharPut(DEBUGUART, c);

  return;
}


void nmi_fault(void)   { DEBUGOUT("NMI fault\n");    while(1); }
void hard_fault(void)  { DEBUGOUT("HARD fault\n");   while(1); }
void mpu_fault(void)   { DEBUGOUT("MPU fault\n");    while(1); }
void bus_fault(void)   { DEBUGOUT("BUS fault\n");    while(1); }
void usage_fault(void) { DEBUGOUT("USAGE fault\n");  while(1); }
void svcall_fault(void){ DEBUGOUT("SVCALL fault\n"); while(1); }
void debug_fault(void) { DEBUGOUT("DEBUG fault\n");  while(1); }
void pendsv_fault(void){ DEBUGOUT("PENDSV fault\n"); while(1); }


void debug_init(void)
{
#ifdef DEBUG

  //init uart
# if (DEBUGUART == UART1_BASE)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3); //uart1-tx = output
  GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_3);
#  define DEBUGUART_PERIPH SYSCTL_PERIPH_UART1
# elif (DEBUGUART == UART2_BASE)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
  GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_1); //uart2-tx = output
  GPIOPinTypeUART(GPIO_PORTG_BASE, GPIO_PIN_1);
#  define DEBUGUART_PERIPH SYSCTL_PERIPH_UART2
# else
#  warning "DEBUGUART unknown"
# endif
  SysCtlPeripheralEnable(DEBUGUART_PERIPH);
  SysCtlPeripheralReset(DEBUGUART_PERIPH);
  UARTConfigSetExpClk(DEBUGUART, SysCtlClockGet(), DEBUGBAUD, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
  UARTEnable(DEBUGUART);

  //init fault ints
  IntRegister(FAULT_NMI,    nmi_fault);    IntEnable(FAULT_NMI);
  IntRegister(FAULT_HARD,   hard_fault);   IntEnable(FAULT_HARD);
  IntRegister(FAULT_MPU,    mpu_fault);    IntEnable(FAULT_MPU);
  IntRegister(FAULT_BUS,    bus_fault);    IntEnable(FAULT_BUS);
  IntRegister(FAULT_USAGE,  usage_fault);  IntEnable(FAULT_USAGE);
  IntRegister(FAULT_SVCALL, svcall_fault); IntEnable(FAULT_SVCALL);
  IntRegister(FAULT_DEBUG,  debug_fault);  IntEnable(FAULT_DEBUG);
  IntRegister(FAULT_PENDSV, pendsv_fault); IntEnable(FAULT_PENDSV);

  return;
#endif
}
