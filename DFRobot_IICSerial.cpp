#include "DFRobot_IICSerial.h"
#include "I2Cdev.h"
#include <stdio.h>
#include <cstring>

#define DEBUG 0

#if DEBUG
 #define _DEBUG 
#else
 #define _DEBUG for(;0;)
#endif



DFRobot_IICSerial::DFRobot_IICSerial(I2Cdev &i2cdev,  uint8_t subUartChannel, uint8_t IA1, uint8_t IA0){
  _i2cdev = &i2cdev;
  _addr = (IA1 << 6) | (IA0 << 5) | IIC_ADDR_FIXED;
  _subSerialChannel = subUartChannel;
  _rx_buffer_head = 0;
  _rx_buffer_tail = 0;
  memset(_rx_buffer, 0, sizeof(_rx_buffer));
}

DFRobot_IICSerial::~DFRobot_IICSerial(){
  
}

int DFRobot_IICSerial::begin(long unsigned baud, uint8_t format, eCommunicationMode_t mode, eLineBreakOutput_t opt){
  _rx_buffer_head = _rx_buffer_tail;
  //_pWire->begin();
  uint8_t val = 0;
  uint8_t channel = subSerialChnnlSwitch(SUBUART_CHANNEL_1);
  if(readReg(REG_WK2132_GENA, &val, 1) != 1){
      printf("READ BYTEERROR!\n");
      return ERR_READ;
  }
  subSerialChnnlSwitch(channel);
  subSerialConfig(_subSerialChannel);
  setSubSerialBaudRate(baud);
  setSubSerialConfigReg(format, mode, opt);
  printf("DFRobot_IICSerial init OK\n");
  return ERR_OK;
}

void DFRobot_IICSerial::end(){
  subSerialGlobalRegEnable(_subSerialChannel, rst);
}

int DFRobot_IICSerial::available(void){
  int index;
  uint8_t val = 0;
  sFsrReg_t fsr;
  if(readReg(REG_WK2132_RFCNT, &val, 1) != 1){
      printf("READ BYTE SIZE ERROR!\n");
      return 0;
  }
  index = (int)val;
  if(index == 0){
      fsr = readFIFOStateReg();
      if(fsr.rDat == 1){
          index = 256;
      }
  }
  return (index + ((unsigned int)(SERIAL_RX_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail)) % SERIAL_RX_BUFFER_SIZE);
}

int DFRobot_IICSerial::peek(void){
  int num = available() - (((unsigned int)(SERIAL_RX_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail)) % SERIAL_RX_BUFFER_SIZE);
  for(int i = 0; i < num; i++){
      rx_buffer_index_t j = (rx_buffer_index_t)(_rx_buffer_head + 1) % SERIAL_RX_BUFFER_SIZE;
      if(j != _rx_buffer_tail){
          uint8_t val = 0;
          readReg(REG_WK2132_FDAT, &val, 1);
          _rx_buffer[_rx_buffer_head] = val;
          _rx_buffer_head = j;
      }else{
          break;
      }
  }
  if(_rx_buffer_head == _rx_buffer_tail){
      return -1;
  }
  return _rx_buffer[_rx_buffer_tail];
}

int DFRobot_IICSerial::read(void){
  int num = available() - ((unsigned int)(SERIAL_RX_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail)) % SERIAL_RX_BUFFER_SIZE;
  for(int i = 0; i < num; i++){
      rx_buffer_index_t j = (rx_buffer_index_t)(_rx_buffer_head + 1) % SERIAL_RX_BUFFER_SIZE;
      if(j != _rx_buffer_tail){
          uint8_t val = 0;
          readReg(REG_WK2132_FDAT, &val, 1);
          _rx_buffer[_rx_buffer_head] = val;
          _rx_buffer_head = j;
      }else{
          break;
      }
  }
  if(_rx_buffer_head == _rx_buffer_tail){
      return -1;
  }
  unsigned char c = _rx_buffer[_rx_buffer_tail];
  _rx_buffer_tail = (rx_buffer_index_t)(_rx_buffer_tail + 1) % SERIAL_RX_BUFFER_SIZE;
  return c;
}

size_t DFRobot_IICSerial::write(uint8_t value){
  sFsrReg_t fsr;
  fsr = readFIFOStateReg();
  if(fsr.tFull == 1){
      printf("FIFO full!\n");
      return -1;
  }
  writeReg(REG_WK2132_FDAT, &value, 1);
  return 1;
}

/*size_t DFRobot_IICSerial::write(const uint8_t *pBuf, size_t size){
  if(pBuf == NULL){
    printf("pBuf ERROR!! : null pointer");
    return 0;
  }
  uint8_t *_pBuf = (uint8_t *)pBuf;
  sFsrReg_t fsr;
  uint8_t val = 0;
  fsr = readFIFOStateReg();
  if(fsr.tFull == 1){
      printf("FIFO full!");
      return 0;
  }
  writeFIFO(_pBuf, size);
  return size;
}
*/

size_t DFRobot_IICSerial::read(void *pBuf, size_t size){
  if(pBuf == NULL){
    printf("pBuf ERROR!! : null pointer\n");
    return 0;
  }
  uint8_t *_pBuf = (uint8_t *)pBuf;
  size = available() - (((unsigned int)(SERIAL_RX_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail)) % SERIAL_RX_BUFFER_SIZE);
  readFIFO(_pBuf, size);
  return size;
}
void DFRobot_IICSerial::flush(void){
  sFsrReg_t fsr = readFIFOStateReg();
  while(fsr.tDat == 1);
}


void DFRobot_IICSerial::subSerialConfig(uint8_t subUartChannel){
  _DEBUG printf("Sub UART clock enable\n");
  subSerialGlobalRegEnable(subUartChannel, clock);
  _DEBUG printf("Software reset sub UART\n");
  subSerialGlobalRegEnable(subUartChannel, rst);
  _DEBUG printf("Sub UART global interrupt enable\n");
  subSerialGlobalRegEnable(subUartChannel, intrpt);
  _DEBUG printf("Sub UART page register setting (default PAGE0)\n");
  subSerialPageSwitch(page0);
  _DEBUG printf("Sub interrupt setting\n");
  sSierReg_t sier = {.rFTrig = 0x01, .rxOvt = 0x01, .tfTrig = 0x01, .tFEmpty = 0x01, .rsv = 0x00, .fErr = 0x01};
  subSerialRegConfig(REG_WK2132_SIER, &sier);
  _DEBUG printf("enable transmit/receive FIFO\n");
  sFcrReg_t fcr = {.rfRst = 0x01, .tfRst = 0x00, .rfEn = 0x01, .tfEn = 0x01, .rfTrig = 0x00, .tfTrig = 0x00};
  subSerialRegConfig(REG_WK2132_FCR, &fcr);
  _DEBUG printf("Sub UART reiceive/transmit enable\n");
  sScrReg_t scr = {.rxEn = 0x01, .txEn = 0x01, .sleepEn = 0x00, .rsv = 0x00 };
  subSerialRegConfig(REG_WK2132_SCR, &scr);
}

void DFRobot_IICSerial::subSerialGlobalRegEnable(uint8_t subUartChannel, eGlobalRegType_t type){
  if(subUartChannel > SUBUART_CHANNEL_ALL)
  {
      printf("SUBSERIAL CHANNEL NUMBER ERROR!");
      return;
  }
  uint8_t val = 0;
  uint8_t regAddr = getGlobalRegType(type);
  uint8_t channel = subSerialChnnlSwitch(SUBUART_CHANNEL_1);
  _DEBUG printf("reg");
  _DEBUG printf("%02x\n", regAddr);
  if(readReg(regAddr, &val, 1) != 1){
        printf("READ BYTE SIZE ERROR!\n");
      return;
  }
  _DEBUG printf("before:");
  _DEBUG printf("%02x\n", val);
  switch(subUartChannel){
      case SUBUART_CHANNEL_1:
                             val |= 0x01;
                             break;
      case SUBUART_CHANNEL_2:
                             val |= 0x02;
                             break;
      default:
              val |= 0x03;
              break;
  }
  writeReg(regAddr, &val, 1);
  readReg(regAddr, &val, 1);
  _DEBUG printf("after:");
  _DEBUG printf("%02x\n", val);
  subSerialChnnlSwitch(channel);
}

void DFRobot_IICSerial::subSerialPageSwitch(ePageNumber_t page){
  if(page >= pageTotal){
      return;
  }
  uint8_t val = 0;
  if(readReg(REG_WK2132_SPAGE, &val, 1) != 1){
      printf("READ BYTE SIZE ERROR!");
      return;
  }
  switch(page){
      case page0:
                 val &= 0xFE;
                 break;
      case page1:
                 val |= 0x01;
                 break;
      default:
              break;
  }
  _DEBUG printf("before: "); 
  _DEBUG printf("%02x\n", val);
  writeReg(REG_WK2132_SPAGE, &val, 1);
  readReg(REG_WK2132_SPAGE, &val, 1);
  _DEBUG printf("after: ");
  _DEBUG printf("%02x\n", val);
}

void DFRobot_IICSerial::subSerialRegConfig(uint8_t reg, void *pValue){
  uint8_t val = 0;
  readReg(reg, &val, 1);
  _DEBUG printf("before: "); 
  _DEBUG printf("%02x\n", val);
  val |= *(uint8_t *)pValue;
  writeReg(reg, &val, 1);
  readReg(reg, &val, 1);
  _DEBUG printf("after: ");
  _DEBUG printf("%02x\n", val);
}

uint8_t DFRobot_IICSerial::getGlobalRegType(eGlobalRegType_t type){
  if((type < clock) || (type > intrpt)){
      printf("Global Reg Type Error!");
      return 0;
  }
  uint8_t regAddr = 0;
  switch(type){
      case clock:
                 regAddr = REG_WK2132_GENA;
                 break;
      case rst:
                 regAddr = REG_WK2132_GRST;
                 break;
      default:
              regAddr = REG_WK2132_GIER;
              break;
  }
  return regAddr;
}

void DFRobot_IICSerial::setSubSerialBaudRate(unsigned long baud){
  uint8_t scr = 0x00,clear = 0x00;
  readReg(REG_WK2132_SCR, &scr, 1);
  subSerialRegConfig(REG_WK2132_SCR, &clear);
  uint8_t baud1 = 0,baud0 = 0, baudPres = 0;
  uint16_t valIntger  = FOSC/(baud * 16) - 1;
  uint16_t valDecimal = (FOSC%(baud * 16))/(baud * 16); 
  baud1 = (uint8_t)(valIntger >> 8);
  baud0 = (uint8_t)(valIntger & 0x00ff);
  while(valDecimal > 0x0A){
      valDecimal /= 0x0A;
  }
  baudPres = (uint8_t)(valDecimal);
  subSerialPageSwitch(page1);
  subSerialRegConfig(REG_WK2132_BAUD1, &baud1);
  subSerialRegConfig(REG_WK2132_BAUD0, &baud0);
  subSerialRegConfig(REG_WK2132_PRES, &baudPres);

  readReg(REG_WK2132_BAUD1, &baud1, 1);
  readReg(REG_WK2132_BAUD0, &baud0, 1);
  readReg(REG_WK2132_PRES, &baudPres, 1);
  _DEBUG printf("baud1 %02x \n", baud1);
  _DEBUG printf("baud0 %02x \n", baud0);
  _DEBUG printf("baudPres %02x \n", baudPres);
  subSerialPageSwitch(page0);
  subSerialRegConfig(REG_WK2132_SCR, &scr);
}

void DFRobot_IICSerial::setSubSerialConfigReg(uint8_t format, eCommunicationMode_t mode, eLineBreakOutput_t opt){
  uint8_t _mode = (uint8_t)mode;
  uint8_t _opt = (uint8_t)opt;
  uint8_t val = 0;
  _addr = updateAddr(_addr, _subSerialChannel, OBJECT_REGISTER);
  if(readReg(REG_WK2132_LCR, &val, 1) != 1){
      printf("Read Byte ERROR！");
      return;
  }
  _DEBUG printf("before: "); 
  _DEBUG printf("%02x\n", val);
  sLcrReg_t lcr = *((sLcrReg_t *)(&val));
  lcr.format = format;
  lcr.irEn = _mode;
  lcr.lBreak = _opt;
  val = *(uint8_t *)&lcr;
  writeReg(REG_WK2132_LCR, &val, 1);
  readReg(REG_WK2132_LCR, &val, 1);
  _DEBUG printf("after: "); 
  _DEBUG printf("%02x\n", val);
}

uint8_t DFRobot_IICSerial::updateAddr(uint8_t pre, uint8_t subUartChannel, uint8_t obj){
  sIICAddr_t addr ={.type = obj, .uart = subUartChannel, .addrPre = (uint8_t)((int)pre >> 3)};
  return *(uint8_t *)&addr;
}

DFRobot_IICSerial::sFsrReg_t DFRobot_IICSerial::readFIFOStateReg(){
  sFsrReg_t fsr;
  readReg(REG_WK2132_FSR, &fsr, sizeof(fsr));
  return fsr;
}

uint8_t DFRobot_IICSerial::subSerialChnnlSwitch(uint8_t subUartChannel){
  uint8_t channel = _subSerialChannel;
  _subSerialChannel = subUartChannel;
  return channel;
}

void DFRobot_IICSerial::sleep(){
  
}

void DFRobot_IICSerial::wakeup(){

}

void DFRobot_IICSerial::writeReg(uint8_t reg, const void* pBuf, size_t size){
  if(pBuf == NULL){
      printf("pBuf ERROR!! : null pointer");
  }

  _addr = updateAddr(_addr, _subSerialChannel, OBJECT_REGISTER);
  uint8_t * _pBuf = (uint8_t *)pBuf;

  _i2cdev->writeBytes(_addr, reg, size, _pBuf);
}

uint8_t DFRobot_IICSerial::readReg(uint8_t reg, void* pBuf, size_t size){
  if(pBuf == NULL){
    printf("pBuf ERROR!! : null pointer");
    return 0;
  }

  _addr = updateAddr(_addr, _subSerialChannel, OBJECT_REGISTER);
  uint8_t * _pBuf = (uint8_t *)pBuf;
  _addr &= 0xFE;

  _i2cdev->readBytes(_addr, reg, size, _pBuf);
  return size;
}

uint8_t DFRobot_IICSerial::readFIFO(void* pBuf, size_t size){
  if(pBuf == NULL){
    printf("pBuf ERROR!! : null pointer");
    return 0;
  }

  _addr = updateAddr(_addr, _subSerialChannel, OBJECT_FIFO);
  uint8_t *_pBuf = (uint8_t *)pBuf;
  size_t left = size,num = 0;

  while(left){
    num = (left > IIC_BUFFER_SIZE) ?  IIC_BUFFER_SIZE : left;
    _i2cdev->readBytesNoRegAddress(_addr, num, _pBuf);
    left -=num;
    _pBuf += num;
  }

  return (uint8_t)size;
}

/*
void DFRobot_IICSerial::writeFIFO(void *pBuf, size_t size){
  if(pBuf == NULL){
      printf("pBuf ERROR!! : null pointer");
      return;
  }

  _addr = updateAddr(_addr, _subSerialChannel, OBJECT_FIFO);
  uint8_t *_pBuf = (uint8_t *)pBuf;
  size_t left = size;

  while(left){
    size = (left > IIC_BUFFER_SIZE) ? IIC_BUFFER_SIZE: left;
    
    _pWire->beginTransmission(_addr);
    _pWire->write(_pBuf, size);
    if(_pWire->endTransmission() != 0){
        return;
    }
    
    _i2cdev->writeBytes(_addr, reg, size, pBuf);
    delay(10);
    left -= size;
    _pBuf = _pBuf + size;
  }
}
*/