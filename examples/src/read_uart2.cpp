/*!
 * @file read_uart2.cpp
 * @brief Receive data via UART. Read data sent by an external UART pin via pin RX.
 * @author Onera
 * @version  V1.0
 * @date  23/04/2021
 * @get from https://www.dfrobot.com
 * @url https://github.com/JaladeSamuel/DFRobot_IICSerial
 */
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include "../library/DFRobot_IICSerial.h"
#include "DFRobot_IICSerial.h"

I2Cdev i2cdev;
DFRobot_IICSerial iicSerial2(i2cdev, /*subUartChannel =*/SUBUART_CHANNEL_2,/*IA1 = */1,/*IA0 = */1);//Construct UART2

void setup() {

  iicSerial2.begin(/*baud = */115200);/*UART2 init*/

  printf("\n+--------------------------------------------+\n");
  printf("|  Connected extarnal TX to UART2 RX         |\n");
  printf("+--------------------------------------------+\n");
}

int main(void) {
  setup();
  uint8_t byte, flag;
  flag = 0;
  
  while(true) {  
    while(iicSerial2.available()) {
      byte = iicSerial2.read();//Read data of UART2 receive buffer 
      printf("%d ",byte);
      if(flag == 0) {
        flag = 1;
      }
    }

    if(flag) {
      printf("\n---------------------------------\n");
      flag = 0;
    }
    
    usleep(10*1000);
  }
  return 0;
}
