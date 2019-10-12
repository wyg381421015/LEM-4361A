#ifndef __DRV_LCD_H__
#define __DRV_LCD_H__		

#include <rtdevice.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_fsmc.h"
	
#define RED     0xf800
#define GREEN   0x07e0
#define BLUE    0x001f
#define YELLOW  0xffe0
#define CYAN    0x07ff
#define MAGENTA 0xf81f
#define BLACK   0x0000
#define WHITE   0xffff
#define GRAY    0x8410

#define hi_re 0xf800
#define hi_or 0xfc00
#define hi_ye 0xffb0
#define hi_yg 0x87e0
#define hi_gr 0x07e0
#define hi_gc 0x07f0
#define hi_cy 0x07ff
#define hi_cb 0x041f
#define hi_bl 0x001f
#define hi_bm 0x801f
#define hi_ma 0xf8f1
#define hi_mr 0xf810


//扫描方向定义
#define L2R_U2D  0 		//从左到右,从上到下
#define L2R_D2U  1 		//从左到右,从下到上
#define R2L_U2D  2 		//从右到左,从上到下
#define R2L_D2U  3 		//从右到左,从下到上
#define U2D_L2R  4 		//从上到下,从左到右
#define U2D_R2L  5 		//从上到下,从右到左
#define D2U_L2R  6 		//从下到上,从左到右
#define D2U_R2L  7		//从下到上,从右到左	 
#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向


#define LCM_DOT_WIDTH 	320
#define LCM_DOT_HEIGHT	240
#define  LCM_CHAR_WIDTH  24
#define  LCM_CHAR_HEIGHT 24

// ASCII 字模宽度及高度定义
#define ASC_CHR_WIDTH		12
#define ASC_CHR_HEIGHT	36
										  
int rt_lcd_init(void);

#endif  
	 
	 



