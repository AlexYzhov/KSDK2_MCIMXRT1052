#ifndef _DRV_DISPLAY_
#define _DRV_DISPLAY_

#define DISPLAY_WIDTH		(752)
#define DISPLAY_HEIGHT		(480)

/* DSI Timing */
#define DSI_HSW 			25		//min = 24
#define DSI_HFP 			10		//min = 5
#define DSI_HBP 			10		//min = 5
#define DSI_VSW 			2 		//min = 2
#define DSI_VFP 			4 		//min = 2
#define DSI_VBP 			4 		//min = 2
#define DSI_POL_FLAGS 	(kELCDIF_DataEnableActiveHigh|	\
						 kELCDIF_VsyncActiveLow|		\
						 kELCDIF_HsyncActiveLow|		\
						 kELCDIF_DriveDataOnFallingClkEdge)

////////////////////////////////////////////////////////////////////////////////

/******������ɫ*****/
#define RGB(r,g,b) ( (r>>3)<<11|(g>>2)<<5|(b>>3) )

#define RED     0XF800    			//��ɫ
#define GREEN   0X07E0    			//��ɫ
#define BLUE    0X001F    			//��ɫ
#define BRED    0XF81F
#define GRED    0XFFE0    			//��ɫ
#define GBLUE   0X07FF    			//
#define BLACK   0X0000    			//��ɫ
#define GRAY    RGB(192, 192, 192)  // ��ɫ
#define WHITE   0XFFFF    			//��ɫ
#define YELLOW  0xFFE0    			//��ɫ

//��ȡRGB��3Ԫ�أ�rgb565������16λ
#define RGB565_R(rgb565)                ((uint8_t)(((uint16_t)(rgb565)>>11) &0x1F))
#define RGB565_G(rgb565)                ((uint8_t)(((uint16_t)(rgb565)>> 5) &0x3F))
#define RGB565_B(rgb565)                ((uint8_t)( (uint16_t)(rgb565)      &0x1F))

#define GRAY_2_RGB565(gray)             ((uint16_t)((((uint8_t)(gray)>>3)<<11)|(((uint8_t)(gray)>>2)<<5)|((uint8_t)(gray)>>3)))
#define GRAY_2_RGB888(gray)             ((uint32_t)((((uint8_t)gray)<<16)|(((uint8_t)gray)<<8)|(((uint8_t)gray)<<0)))
#define RGB565_2_GRAY(rgb565)           ((uint8_t)(((RGB565_R(rgb565)*235+RGB565_G(rgb565)*613+RGB565_B(rgb565)*625)+1)>>8))  //  31*235+63*613+31*625+1  = 255*256

////////////////////////////////////////////////////////////////////////////////

extern void Display_Init(void);

#endif
