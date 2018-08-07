/**
  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
  Unless required by applicable law or agreed to in writing,
  software distributed under the License is distributed on an
  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
  KIND, either express or implied.  See the License for the
  specific language governing permissions and limitations
  under the License.
 *******************************************************************************
 * @license License under APACHE 2.0.
 * @file main.c
 * @author Ânderson Ignacio da Silva
 * @date 08 Ago 2018
 * @brief Program that runs in user-linux space to read RTC through i2c /dev/i2c-1
 * @see https://blog.aignacio.com
 * @target Cyclone V - 5CSEBA6U23I7
 * @todo
**/
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "hwlib.h"
#include "hps_0.h"

#include "alt_dma.h"
#include "alt_globaltmr.h"
#include "socal/hps.h"
#include "socal/socal.h"
#include "alt_clock_manager.h"
#include "alt_generalpurpose_io.h"
#include "alt_globaltmr.h"
#include "hwlib.h"
#include "socal/alt_gpio.h"

// LED PIO Macros
#define HPS_TO_FPGA_LW_BASE 0xFF200000
#define HPS_TO_FPGA_LW_SPAN 0x0020000

// I2C Macros
#define RTC_I2C_ADDR 0x68

#define TEMP_REG_1 0x11
#define TEMP_REG_2 0x12
#define SECO_REG   0x00
#define MINU_REG   0x01
#define HOUR_REG   0x02
#define DOW_REG    0x03
#define DAY_REG    0x04
#define MONTH_REG  0x05
#define YEAR_REG   0x06

#define PATH_I2C "/dev/i2c-1" //We're using the channel 1

static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command,
                                     int size, union i2c_smbus_data *data){
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;
	return ioctl(file,I2C_SMBUS,&args);
}

static inline __s32 i2c_smbus_read_byte_data(int file, __u8 command){
	union i2c_smbus_data data;
	if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
	                     I2C_SMBUS_BYTE_DATA,&data))
		return -1;
	else
		return 0x0FF & data.byte;
}

uint8_t bcdtodec(const uint8_t val){
    return ((val / 16 * 10) + (val % 16));
}

uint8_t get_data(int fp, uint8_t reg){
 uint8_t data, status;
 status = ioctl(fp, I2C_SLAVE, RTC_I2C_ADDR);
 if (status < 0)
  printf("\nI2C Address not available...0x%x", RTC_I2C_ADDR);// printf("\nI2C Address correct...0x%x", RTC_I2C_SLAVE_ADDRESS);
 data = i2c_smbus_read_byte_data(fp, reg);
 return bcdtodec(data);
}

char* day_of_week(uint8_t dow){
  switch (dow) {
   case 1:
    return "Monday";
   break;
   case 2:
    return "Tuesday";
   break;
   case 3:
    return "Wednesday";
   break;
   case 4:
    return "Thursday";
   break;
   case 5:
    return "Friday";
   break;
   case 6:
    return "Saturday";
   break;
   default:
    return "Sunday";
   break;
  }
}

int main(int argc, char *argv[]){
 uint8_t temp[2],
         seconds,
         minutes,
         hour,
         day,
         month,
         dow;
 int year;

 int file_i2c, devmem_fd = 0;
 bool toggle = 0;

 void * lw_bridge_map = 0;
 uint32_t * led_pio_map = 0;

 // LEDs PIO
 devmem_fd = open("/dev/mem", O_RDWR | O_SYNC);
 if(devmem_fd < 0) {
     perror("devmem open");
     exit(EXIT_FAILURE);
 }

 lw_bridge_map = (uint32_t*)mmap(NULL, HPS_TO_FPGA_LW_SPAN, PROT_READ|PROT_WRITE, MAP_SHARED, devmem_fd, HPS_TO_FPGA_LW_BASE);
 if(lw_bridge_map == MAP_FAILED) {
     perror("devmem mmap");
     close(devmem_fd);
     exit(EXIT_FAILURE);
 }
 led_pio_map = (uint32_t*)(lw_bridge_map + LED_PIO_BASE);
 *led_pio_map = 0xFF;

 // I2C device file
 file_i2c = open(PATH_I2C, O_RDWR);
 file_i2c < 0 ? printf("\nError opening I2C1, path=%s",PATH_I2C) : printf("I2C1 Path ok");

 while (true) {
  *led_pio_map = 0x00;
  temp[0] = get_data(file_i2c,TEMP_REG_1);
  temp[1] = get_data(file_i2c,TEMP_REG_2);
  seconds = get_data(file_i2c,SECO_REG);
  minutes = get_data(file_i2c,MINU_REG);
  hour = get_data(file_i2c,HOUR_REG);
  day = get_data(file_i2c,DAY_REG);
  month = get_data(file_i2c,MONTH_REG);
  year = get_data(file_i2c,YEAR_REG)+2000;
  dow = get_data(file_i2c,DOW_REG);

  printf("\nTemp:%d.%dC Time now: %02d:%02d:%02d Date: %02d/%02d/%02d - %s",\
  temp[0],temp[1]&(0x7f),\
  hour,minutes,seconds,\
  day,month,year, day_of_week(dow));
  if (toggle)
   *led_pio_map = 0xFF;
  else
   *led_pio_map = 0x00;
  toggle = !toggle;
  usleep(100000);
  // sleep(0.5);
 }
 return 0;
}
