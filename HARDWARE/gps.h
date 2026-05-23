#ifndef __GPS_H
#define __GPS_H

#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>

void GPS_Init(uint32_t baud);
void GPS_IRQHandler(void);  // ??????
void parseGpsBuffer(void);
double convertLatitude(char *lat, char NS);
double convertLongitude(char *lon, char EW);
void getUTCTime(uint8_t *hour, uint8_t *minute, uint8_t *second);
void getLocalTime(uint8_t *hour, uint8_t *minute, uint8_t *second, int8_t timezone);

#endif