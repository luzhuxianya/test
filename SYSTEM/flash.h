#ifndef __FLASH_H
#define __FLASH_H 			   
#include "sys.h"

#define FLASH_START_ADDR		0x0801f000
#define	CHECK_FLASH					18
void FLASH_W(u32 add,u16 dat);
u16 FLASH_R(u32 add);

#endif





























