#include "Console.h"
#include "hal_uart.h"

unsigned char itoaBuffer[] = {0,0,0,0,0,0,0,0,0,0};

void println(unsigned char*s){
  print(s);
  print("\r\n");
}


void print(unsigned char *s){
  while(*s != '\0'){
    HalUARTWrite(HAL_UART_PORT_0, s, 1);
    s++;
  }
}

unsigned char* itoa(int i){
    unsigned char const digit[] = "0123456789";
    unsigned char* p = itoaBuffer;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return p;
}