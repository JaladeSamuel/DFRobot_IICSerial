# DFRobot_IICSerial


**This is a custom version of the original repo for Raspberry-pi compatibility using <linux/i2c-dev> library.**
**You can find the original source code in arduino branch.**

This is an IIC to dual UART module with 1Mbps IIC transmission rate, and each sub UART is able to receive/transmit independent 256 bytes FIFO hardware cache. It can be used in the applications requiring the transmission of large amounts of data. <br>
The band rate, word length, and check format of every sub UART can be set independently. The module can provide at most 2Mbps communication rate, and support 4 IIC addresses. Four such modules can be connected to one controller board to expand 8 hardware serial port. <br>



<br>
<img src="./resources/images/DFR0627svg.png">
<br>
