#ifndef __SCREEN_H__
#define __SCREEN_H__



#define RGB565(r,g,b) ((r>>3)<<11)|((g>>2)<<5)|(b>>3)

#define RED     RGB565(255,0,0)
#define GREEN   RGB565(0,255,0)
#define BLUE    RGB565(0,0,255)
#define YELLOW  RGB565(255,255,0)
#define CYAN    RGB565(0,255,255)
#define MAGENTA RGB565(255,0,255)
#define BLACK   RGB565(0,0,0)
#define WHITE   RGB565(255,255,255)
#define GRAY    RGB565(128,128,128)


#define SCR_000		0
#define SCR_001		1
#define SCR_002		2
#define SCR_003		3
#define SCR_004		4
#define SCR_005		5
#define SCR_006		6
#define SCR_007		7
#define SCR_008		8
#define SCR_009		9

static void LCD_DisPlayScreen(void);
static void LCD_DisPlayScreen000(void);
static void LCD_DisPlayScreen001(void);
static void LCD_DisPlayScreen002(void);
static void LCD_DisPlayScreen003(void);
static void LCD_DisPlayScreen004(void);


static void LCD_UpdateScreen(void);//界面更新函数
static void LCD_UpdateScreen001(void);
static void LCD_UpdateScreen002(void);
static void LCD_UpdateScreen003(void);
static void LCD_UpdateScreen004(void);

static void LCD_UpdateTime(void);//更新时间


static char* LCD_Ltoa(char *buf, unsigned long i, int len, char dec);

#endif

