/**
  ******************************************************************
  * @file    bsp_lcd.c
  * @author  fire
  * @version V2.0
  * @date    2018-xx-xx
  * @brief   lcdӦ�ú����ӿ�
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  i.MXRT1052������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************
  */
#include "fsl_iomuxc.h"
#include "fsl_gpio.h"  
#include "fsl_elcdif.h" 
#include "fsl_clock.h"
#include "fsl_pxp.h"

	
#include "pad_config.h"  
#include "./lcd/bsp_lcd.h" 



/*******************************************************************************
 * ����
 ******************************************************************************/

/* ֡�жϱ�־ */
volatile bool s_frameDone = false;

/* �Դ� */
AT_NONCACHEABLE_SECTION_ALIGN( uint32_t s_psBufferLcd[2][LCD_PIXEL_HEIGHT][LCD_PIXEL_WIDTH], FRAME_BUFFER_ALIGN);

/*���ڴ洢��ǰѡ��������ʽ*/
static sFONT *LCD_Currentfonts = &Font24x48;
/* ���ڴ洢��ǰ������ɫ�����屳����ɫ�ı���*/
static uint32_t CurrentTextColor   = 0x00FFFFFF;
static uint32_t CurrentBackColor   = 0x00000000;

/* ָ��ǰ���Դ� */
static uint32_t CurrentFrameBuffer = (uint32_t)s_psBufferLcd[0];

/*******************************************************************************
 * ��
 ******************************************************************************/
/* �������ž�ʹ��ͬ����PAD���� */
#define LCD_PAD_CONFIG_DATA            (SRE_0_SLOW_SLEW_RATE| \
                                        DSE_6_R0_6| \
                                        SPEED_1_MEDIUM_100MHz| \
                                        ODE_0_OPEN_DRAIN_DISABLED| \
                                        PKE_1_PULL_KEEPER_ENABLED| \
                                        PUE_0_KEEPER_SELECTED| \
                                        PUS_0_100K_OHM_PULL_DOWN| \
                                        HYS_0_HYSTERESIS_DISABLED)   
    /* ����˵�� : */
    /* ת������: ת��������
        ����ǿ��: R0/6 
        �������� : medium(100MHz)
        ��©����: �ر� 
        ��/����������: ʹ��
        ��/������ѡ��: ������
        ����/����ѡ��: 100Kŷķ����(ѡ���˱�������������Ч)
        �ͻ�������: ��ֹ */
        
/*******************************************************************************
 * ����
 ******************************************************************************/
static void LCD_IOMUXC_MUX_Config(void);
static void LCD_IOMUXC_PAD_Config(void);
static void LCD_ELCDIF_Config(void);


/**
* @brief  ��ʼ��LCD���IOMUXC��MUX��������
* @param  ��
* @retval ��
*/
static void LCD_IOMUXC_MUX_Config(void)
{
    /* �������ž�������SION���� */
    /* ʱ������ź��� */
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_00_LCD_CLK, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_01_LCD_ENABLE, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_02_LCD_HSYNC, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_03_LCD_VSYNC, 0U);
  
    /* RGB565�����ź��ߣ�
     DATA0~DATA4:B3~B7
     DATA5~DATA10:G2~G7
     DATA11~DATA15:R3~R7 */
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_04_LCD_DATA00, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_05_LCD_DATA01, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_06_LCD_DATA02, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_07_LCD_DATA03, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_08_LCD_DATA04, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_09_LCD_DATA05, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_10_LCD_DATA06, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_11_LCD_DATA07, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_12_LCD_DATA08, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_13_LCD_DATA09, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_14_LCD_DATA10, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B0_15_LCD_DATA11, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_00_LCD_DATA12, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_01_LCD_DATA13, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_02_LCD_DATA14, 0U);                                    
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_03_LCD_DATA15, 0U);   
    
    /* LCD_BL��������ź��� */
    IOMUXC_SetPinMux(LCD_BL_IOMUXC, 0U); 
}


/**
* @brief  ��ʼ��LCD���IOMUXC��PAD��������
* @param  ��
* @retval ��
*/
static void LCD_IOMUXC_PAD_Config(void)
{  
    /* �������ž�ʹ��ͬ����PAD���� */
    /* ʱ������ź��� */
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_00_LCD_CLK,LCD_PAD_CONFIG_DATA);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_01_LCD_ENABLE, LCD_PAD_CONFIG_DATA);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_02_LCD_HSYNC, LCD_PAD_CONFIG_DATA);  
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_03_LCD_VSYNC, LCD_PAD_CONFIG_DATA); 

    /* RGB565�����ź��ߣ�
     DATA0~DATA4:B3~B7
     DATA5~DATA10:G2~G7
     DATA11~DATA15:R3~R7 */
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_04_LCD_DATA00, LCD_PAD_CONFIG_DATA); 
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_05_LCD_DATA01, LCD_PAD_CONFIG_DATA); 
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_06_LCD_DATA02, LCD_PAD_CONFIG_DATA);  
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_07_LCD_DATA03, LCD_PAD_CONFIG_DATA); 
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_08_LCD_DATA04, LCD_PAD_CONFIG_DATA); 
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_09_LCD_DATA05, LCD_PAD_CONFIG_DATA);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_10_LCD_DATA06, LCD_PAD_CONFIG_DATA);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_11_LCD_DATA07, LCD_PAD_CONFIG_DATA);  
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_12_LCD_DATA08, LCD_PAD_CONFIG_DATA);  
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_13_LCD_DATA09, LCD_PAD_CONFIG_DATA);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_14_LCD_DATA10, LCD_PAD_CONFIG_DATA); 
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_15_LCD_DATA11, LCD_PAD_CONFIG_DATA);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_00_LCD_DATA12, LCD_PAD_CONFIG_DATA);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_01_LCD_DATA13, LCD_PAD_CONFIG_DATA);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_02_LCD_DATA14, LCD_PAD_CONFIG_DATA); 
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_03_LCD_DATA15, LCD_PAD_CONFIG_DATA); 
    
    /* LCD_BL��������ź��� */
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_15_GPIO1_IO15, LCD_PAD_CONFIG_DATA);
}

/**
* @brief  ��ʼ��ELCDIF����
* @param  ��
* @retval ��
*/
static void LCD_ELCDIF_Config(void)
{	
    const elcdif_rgb_mode_config_t config = {
        .panelWidth = LCD_PIXEL_WIDTH,
        .panelHeight = LCD_PIXEL_HEIGHT,
        .hsw = APP_HSW,
        .hfp = APP_HFP,
        .hbp = APP_HBP,
        .vsw = APP_VSW,
        .vfp = APP_VFP,
        .vbp = APP_VBP,
        .polarityFlags = APP_POL_FLAGS,
        .bufferAddr = (uint32_t)s_psBufferLcd[0],
        .pixelFormat = kELCDIF_PixelFormatXRGB8888,
        .dataBus = APP_LCDIF_DATA_BUS,
    };
 
  ELCDIF_RgbModeInit(APP_ELCDIF, &config);
  ELCDIF_RgbModeStart(APP_ELCDIF);
}

/**
* @brief  ��ʼ��ELCDIFʹ�õ�ʱ��
* @param  ��
* @retval ��
*/
void LCD_InitClock(void)
{
    /*
     * Ҫ��֡�����ó�60Hz����������ʱ��Ƶ��Ϊ:
     * ˮƽ����ʱ�Ӹ�����(APP_IMG_WIDTH + APP_HSW + APP_HFP + APP_HBP ) 
     * ��ֱ����ʱ�Ӹ�����(APP_IMG_HEIGHT + APP_VSW + APP_VFP + APP_VBP)
     * 
     * ����ʱ��Ƶ�ʣ�(800 + 1 + 10 + 46) * (480 + 1 + 22 + 23) * 60 = 27.05M.
     * ���������� LCDIF ����ʱ��Ƶ��Ϊ 27M.
     */

    /*
     * ��ʼ�� Video PLL.
     * Video PLL ���Ƶ��Ϊ 
     * OSC24M * (loopDivider + (denominator / numerator)) / postDivider = 108MHz.
     */
    clock_video_pll_config_t config = {
        .loopDivider = 36, .postDivider = 8, .numerator = 0, .denominator = 0,
    };

    CLOCK_InitVideoPll(&config);

    /*
     * 000 derive clock from PLL2
     * 001 derive clock from PLL3 PFD3
     * 010 derive clock from PLL5
     * 011 derive clock from PLL2 PFD0
     * 100 derive clock from PLL2 PFD1
     * 101 derive clock from PLL3 PFD1
     */
    /* ѡ��Ϊvedio PLL*/
    CLOCK_SetMux(kCLOCK_LcdifPreMux, 2);

    /* ���÷�Ƶ */  
    CLOCK_SetDiv(kCLOCK_LcdifPreDiv, 1);

    CLOCK_SetDiv(kCLOCK_LcdifDiv, 1);
}

/**
* @brief  ��ʼ���������Ų�����
* @param  ��
* @retval ��
*/
void LCD_BackLight_ON(void)
{    
    /* ���⣬�ߵ�ƽ���� */
    gpio_pin_config_t config = {
      kGPIO_DigitalOutput, 
      1,
      kGPIO_NoIntmode
    };

    GPIO_PinInit(LCD_BL_GPIO, LCD_BL_GPIO_PIN, &config);
}



/**
* @brief  ��ʼ��Һ����
* @param  enableInterrupt ���Ƿ�ʹ���ж�
* @retval ��
*/
void LCD_Init(bool enableInterrupt)
{
  *((uint32_t *)0x41044100) = 0x0000000f;
	*((uint32_t *)0x41044104) = 0x0000000f;

  LCD_IOMUXC_MUX_Config();
  LCD_IOMUXC_PAD_Config();
  LCD_ELCDIF_Config();
  LCD_InitClock();
  LCD_BackLight_ON();
  
  if(enableInterrupt)
  {
    LCD_InterruptConfig();
  }
}

/***************************�ж����******************************/
/**
* @brief  ����ELCDIF�ж�
* @param  ��
* @retval ��
*/
void LCD_InterruptConfig(void)
{
  /* ʹ���ж� */
  EnableIRQ(LCDIF_IRQn);
   
  /* ����ELCDIFΪCurFrameDoneInterrupt�ж� */
  ELCDIF_EnableInterrupts(APP_ELCDIF, kELCDIF_CurFrameDoneInterruptEnable);
}

/**
* @brief  ELCDIF�жϷ�����
* @param  ��
* @retval ��
*/
void LCDIF_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = ELCDIF_GetInterruptStatus(APP_ELCDIF);

    ELCDIF_ClearInterruptStatus(APP_ELCDIF, intStatus);

    if (intStatus & kELCDIF_CurFrameDone)
    {
        s_frameDone = true;
    }

    /* ���²�����Ϊ ARM �Ŀ���838869��ӵ�, 
       �ô���Ӱ�� Cortex-M4, Cortex-M4F�ںˣ�       
       �����洢�����ص��쳣�����·��ز������ܻ�ָ�������ж�
        CM7����Ӱ�죬�˴������ô���
    */  
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

/***************************��ʾӦ�����******************************/

/***************************��ʾ�ַ����******************************/

/**
  * @brief  �����������ɫ������ı�����ɫ
  * @param  TextColor: ������ɫ
  * @param  BackColor: ����ı�����ɫ
  * @retval None
  */
void LCD_SetColors(uint32_t TextColor, uint32_t BackColor) 
{
  CurrentTextColor = TextColor; 
  CurrentBackColor = BackColor;
}

/**
  * @brief ��ȡ��ǰ���õ�������ɫ������ı�����ɫ
  * @param  TextColor: ָ��������ɫ��ָ��
  * @param  BackColor: ָ�����屳����ɫ��ָ��
  * @retval None
  */
void LCD_GetColors(uint32_t *TextColor, uint32_t *BackColor)
{
  *TextColor = CurrentTextColor;
  *BackColor = CurrentBackColor;
}

/**
  * @brief  ����������ɫ
  * @param  Color: ������ɫ
  * @retval None
  */
void LCD_SetTextColor(uint32_t Color)
{
  CurrentTextColor = Color;
}

/**
  * @brief  ��������ı�����ɫ
  * @param  Color: ����ı�����ɫ
  * @retval None
  */
void LCD_SetBackColor(uint32_t Color)
{
  CurrentBackColor = Color;
}

/**
  * @brief  ���������ʽ(Ӣ��)
  * @param  fonts: ѡ��Ҫ���õ������ʽ
  * @retval None
  */
void LCD_SetFont(sFONT *fonts)
{
  LCD_Currentfonts = fonts;
}

/**
  * @brief  ��ȡ��ǰ�����ʽ(Ӣ��)
  * @param  None.
  * @retval ��ǰӦ�õĸ�ʽ
  */
sFONT *LCD_GetFont(void)
{
  return LCD_Currentfonts;
}


/**
  * @brief  ����ʾ������ʾһ��Ӣ���ַ�
  * @param  Xpos ���ַ�����ʼX����
  * @param  Ypos ���ַ�����ʼY����
  * @param  Ascii: Ҫ��ʾ���ַ���ASCII��
  * @retval None
  */
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, char Ascii)
{
  uint16_t fontLength;	
  uint16_t page, column;

	uint16_t relativePositon;
	uint8_t *pfont;
  
  uint32_t yBufferPos = 0;
  uint32_t xPixelPos = 0;
  
  /*yBufferPos��ʾ��ǰ�е��Դ�ƫ��λ��*/
  yBufferPos = Ypos*LCD_PIXEL_WIDTH*APP_BPP;
  
  /*xpixelPos��ʾ�������ص�λ��
    APP_BPP*xPixelPos + yBufferPos ���ǵ�ǰ���ص���Դ�λ��
  */
  xPixelPos += Xpos;
	
	//��ascii���ƫ�ƣ���ģ������ASCII���ǰ32����ͼ�η��ţ�
	relativePositon = Ascii - ' ';
	
	//ÿ����ģ���ֽ���
	fontLength = (LCD_Currentfonts->Width*LCD_Currentfonts->Height)/8;
		
	//��ģ�׵�ַ
	/*ascii���ƫ��ֵ����ÿ����ģ���ֽ����������ģ��ƫ��λ��*/
	pfont = (uint8_t *)&LCD_Currentfonts->table[relativePositon * fontLength];
	
  //ÿ����ģ��LCD_Currentfonts->Height�У�����ÿһ��
  for ( page = 0; page < LCD_Currentfonts->Height; page++ )
	{    
    //ÿ����ģ��LCD_Currentfonts->Width/8 ���ֽڣ�����ÿ���ֽ�
    for ( column = 0; column < LCD_Currentfonts->Width/8; column++ ) 
		{	
      uint8_t bitCount = 0;

      //ÿ���ֽ���8������λ������ÿ������λ
      for(bitCount=0; bitCount<8; bitCount++)
      {
        if(*pfont & (0x80>>bitCount))
        {
         //����ɫ
          #if 0
            //RGB888��ʾ��ʽ  
            *(__IO uint16_t*)(CurrentFrameBuffer + (APP_BPP*xPixelPos) + yBufferPos) = (0x00FFFF & CurrentTextColor);        //GB
            *(__IO uint8_t*)(CurrentFrameBuffer + (APP_BPP*xPixelPos) + yBufferPos+2) = (0x00FF0000 & CurrentTextColor) >> 16; //R
          #else
            //XRGB8888��ʾ��ʽ
            *(__IO uint32_t*)(CurrentFrameBuffer + (APP_BPP*xPixelPos) + yBufferPos) = CurrentTextColor;        //XRGB
          #endif          
        }
        else
        {
          //����ɫ
         #if 0
            //RGB888��ʾ��ʽ 
            *(__IO uint16_t*)(CurrentFrameBuffer + (APP_BPP*xPixelPos) + yBufferPos) = (0x00FFFF & CurrentBackColor);        //GB
            *(__IO uint8_t*)(CurrentFrameBuffer + (APP_BPP*xPixelPos) + yBufferPos+2) = (0x00FF0000 & CurrentBackColor) >> 16; //R
         #else
            //XRGB8888��ʾ��ʽ
            *(__IO uint32_t*)(CurrentFrameBuffer + (APP_BPP*xPixelPos) + yBufferPos) = CurrentBackColor;        //XRGB
         #endif
        }
        /*ָ��ǰ�е���һ����*/
        xPixelPos++;		
      }
      
      /* ָ����ģ���ݵ�һ�¸��ֽ� */
      pfont++;
    }      
    /*��ʾ��һ��*/
    /*ָ���ַ���ʾ������һ�еĵ�һ�����ص�*/
    xPixelPos += (LCD_PIXEL_WIDTH - LCD_Currentfonts->Width);		
  }
}

/**
 * @brief  ����ʾ������ʾ��Ӣ���ַ���,����Һ�����ʱ���Զ����С�
 * @param  Xpos ���ַ�����ʼX����
 * @param  Ypos ���ַ�����ʼY����
 * @param  pStr ��Ҫ��ʾ���ַ������׵�ַ
 * @retval ��
 */
void LCD_DispString(uint16_t Xpos, uint16_t Ypos, const uint8_t * pStr )
{
	while( * pStr != '\0' )
	{	
    /*�Զ�����*/
    if ( ( Xpos + LCD_Currentfonts->Width ) > LCD_PIXEL_WIDTH )
    {
      Xpos = 0;
      Ypos += LCD_Currentfonts->Height;
    }
    
    if ( ( Ypos + LCD_Currentfonts->Height ) > LCD_PIXEL_HEIGHT )
    {
      Xpos = 0;
      Ypos = 0;
    }			
        
    /* ��ʾ�����ַ� */
    LCD_DisplayChar(Xpos,Ypos,*pStr);
    
    Xpos += LCD_Currentfonts->Width;
  
    pStr ++;
  }		
} 

/**
  * @brief  ��ʾ�ַ���(Ӣ��)
  * @param  Line: ���ݵ�ǰ���������к�
  *     @arg Line(1),Line(2)��
  * @param  *ptr: Ҫ��ʾ���ַ���
  * @retval None
  */
void LCD_DisplayStringLine(uint16_t Line, uint8_t *ptr)
{  
  uint16_t refcolumn = 0;
  /* ѭ����ʾ�ַ���ֱ�������ַ���������
    ��ֱ��������ʾ�����ַ�
  */
  while ((refcolumn < LCD_PIXEL_WIDTH) && ((*ptr != 0) & 
    (((refcolumn + LCD_Currentfonts->Width) & 0xFFFF) >= LCD_Currentfonts->Width)))
  {
    /* ��ʾ�����ַ� */
    LCD_DisplayChar(refcolumn,Line , *ptr);
    /* ƫ���ַ���ʾλ�� */
    refcolumn += LCD_Currentfonts->Width;
    /* ָ����һ���ַ� */
    ptr++;
  }
}

/**
  * @brief  ���ָ���е��ַ�
  * @param  Line: Ҫ�������,ע��LINE���Ǹ��ݵ�ǰ��������
  *     @arg LINE(1),LINE(2)
  * @retval None
  */
void LCD_ClearLine(uint16_t Line)
{
  uint16_t refcolumn = 0;
  /* ѭ����ʾ����Ļ���Ҳ� */
  while ((refcolumn < LCD_PIXEL_WIDTH) && 
    (((refcolumn + LCD_Currentfonts->Width)& 0xFFFF) >= LCD_Currentfonts->Width))
  {
    /* ��ʾ�ո��൱�������Ч���� */
    LCD_DisplayChar(refcolumn, Line, ' ');
    /* ƫ���ַ���ʾλ�� */
    refcolumn += LCD_Currentfonts->Width;
  }
}

/**
  * @brief  ������ʾ����
  * @param  Xpos: x����
  * @param  Ypos: y����
  * @retval �Դ�ĵ�ַ
  */
uint32_t LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{  
  return CurrentFrameBuffer + APP_BPP*(Xpos + (LCD_PIXEL_WIDTH*Ypos));
}

/***************************��ʾͼ�����******************************/
/**
  * @brief ѡ��ǰҪ�������Դ�����
  * @param  index: 0��1
  * @retval None
  */
void LCD_SetFrameBuffer(uint8_t index)
{
  CurrentFrameBuffer = (uint32_t)s_psBufferLcd[index];
}

/**
  * @brief ���ú���Ҫ��ʾ���Դ�����
  * @param  index: 0��1
  * @retval None
  */
void LCD_SetDisplayBuffer(uint8_t index)
{
  /* ����ELCDIF����һ����������ַ */
  ELCDIF_SetNextBufferAddr(APP_ELCDIF, (uint32_t)s_psBufferLcd[index]);

}

/**
  * @brief ʹ�õ�ǰ��ɫ��ָ����λ�û���һ�����ص�
  * @param  Xpos: x����
  * @param  Ypos: y����
  * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
  * @retval None
  */
void PutPixel(uint16_t Xpos, uint16_t Ypos)
{   
	if ( ( Xpos < LCD_PIXEL_WIDTH ) && ( Ypos < LCD_PIXEL_HEIGHT ) )
  {
		*(uint32_t *)(CurrentFrameBuffer + APP_BPP*(Xpos + (LCD_PIXEL_WIDTH*Ypos))) = CurrentTextColor;
	}
}

/**
  * @brief  �Ե�ǰ������ɫ���������Ļ
  * @param  ��
  * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
  * @retval ��
  */
void LCD_Clear(uint32_t Color)
{
  /* ������������� */
  uint16_t page, column;  
  
  /* ָ����ε�һ�����ص���Դ�λ�� */
  uint32_t *pRectImage = (uint32_t*)CurrentFrameBuffer ;
  
  /* ����ÿһ�� */
  for ( page = 0; page < LCD_PIXEL_HEIGHT; page++ )
  {    
    /* ����ÿһ�� */
    for ( column = 0; column < LCD_PIXEL_WIDTH; column++ ) 
    {	
      *pRectImage = Color;
      
      /* ָ����һ�����ص���Դ�λ�� */
      pRectImage++;
    }      
  }
}

/**
  * @brief ��ʾһ��ֱ��
  * @param Xpos: ֱ������x����
  * @param Ypos: ֱ������y����
  * @param Length: ֱ�ߵĳ���
  * @param Direction: ֱ�ߵķ��򣬿�����
      @arg LINE_DIR_HORIZONTAL(ˮƽ����) 
      @arg LINE_DIR_VERTICAL(��ֱ����).
  * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
  * @retval None
  */
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, bool Direction)
{
  uint16_t index; 
  
  uint16_t realLength;
  
  /* ָ��ֱ�ߵ�һ�����ص���Դ�λ�� */
  uint32_t *pLineImage = (uint32_t*)(CurrentFrameBuffer + APP_BPP*(Xpos + (LCD_PIXEL_WIDTH*Ypos)));

  if(Direction == LINE_DIR_HORIZONTAL)
  {
    realLength = LCD_PIXEL_WIDTH-Xpos-Length > 0 ? Length : LCD_PIXEL_WIDTH - Xpos;
    
    /* ����ÿһ�� */
    for ( index = 0; index < realLength; index++ ) 
    {
        *pLineImage = CurrentTextColor;
        
        /* ָ����һ�����ص���Դ�λ�� */
        pLineImage++;
    }
  }
  else
  {
    realLength = LCD_PIXEL_HEIGHT-Ypos-Length > 0 ? Length : LCD_PIXEL_HEIGHT - Ypos;
    
    /* ����ÿһ�� */
    for ( index = 0; index < realLength; index++ ) 
    {
        *pLineImage = CurrentTextColor;
        
        /* ָ����һ�����ص���Դ�λ�� */
        pLineImage += LCD_PIXEL_WIDTH;
    }
  }   
}

/**
 * @brief  ��Һ������ʹ�� Bresenham �㷨���߶Σ��������㣩 
 * @param  Xpos1 ���߶ε�һ���˵�X����
 * @param  Ypos1 ���߶ε�һ���˵�Y����
 * @param  Xpos2 ���߶ε���һ���˵�X����
 * @param  Ypos2 ���߶ε���һ���˵�Y����
 * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
 * @retval ��
 */
void LCD_DrawUniLine ( uint16_t Xpos1, uint16_t Ypos1, uint16_t Xpos2, uint16_t Ypos2 )
{
	uint16_t us; 
	uint16_t usX_Current, usY_Current;
	
	int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
	int32_t lIncrease_X, lIncrease_Y; 	
	
	
	lDelta_X = Xpos2 - Xpos1; //������������ 
	lDelta_Y = Ypos2 - Ypos1; 
	
	usX_Current = Xpos1; 
	usY_Current = Ypos1; 
	
	
	if ( lDelta_X > 0 ) 
		lIncrease_X = 1; //���õ������� 
	
	else if ( lDelta_X == 0 ) 
		lIncrease_X = 0;//��ֱ�� 
	
	else 
  { 
    lIncrease_X = -1;
    lDelta_X = - lDelta_X;
  } 

	
	if ( lDelta_Y > 0 )
		lIncrease_Y = 1; 
	
	else if ( lDelta_Y == 0 )
		lIncrease_Y = 0;//ˮƽ�� 
	
	else 
  {
    lIncrease_Y = -1;
    lDelta_Y = - lDelta_Y;
  } 

	
	if (  lDelta_X > lDelta_Y )
		lDistance = lDelta_X; //ѡȡ�������������� 
	
	else 
		lDistance = lDelta_Y; 

	
	for ( us = 0; us <= lDistance + 1; us ++ )//������� 
	{  
		PutPixel ( usX_Current, usY_Current );//���� 
		
		lError_X += lDelta_X ; 
		lError_Y += lDelta_Y ; 
		
		if ( lError_X > lDistance ) 
		{ 
			lError_X -= lDistance; 
			usX_Current += lIncrease_X; 
		}  
		
		if ( lError_Y > lDistance ) 
		{ 
			lError_Y -= lDistance; 
			usY_Current += lIncrease_Y; 
		} 
		
	}  	
	
}   

/**
  * @brief  ���ƿ��ľ���
  * @param  Xpos ���������ϽǶ˵�X����
  * @param  Ypos ���������ϽǶ˵�Y����
  * @param  Width �����ο�
  * @param  Height �����θ�
  * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
  * @retval ��
  */
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{  
  uint16_t realHeight,realWidth;
  
  realHeight = LCD_PIXEL_HEIGHT-Ypos-Height > 0 ? Height : LCD_PIXEL_HEIGHT - Ypos;
  realWidth = LCD_PIXEL_WIDTH-Xpos-Width > 0 ? Width : LCD_PIXEL_WIDTH - Xpos;
  
  LCD_DrawLine(Xpos, Ypos, realWidth, LINE_DIR_HORIZONTAL);
  LCD_DrawLine(Xpos, Ypos, realHeight, LINE_DIR_VERTICAL);
  LCD_DrawLine(Xpos + realWidth - 1, Ypos, realHeight, LINE_DIR_VERTICAL);
  LCD_DrawLine(Xpos, Ypos + realHeight - 1, realWidth, LINE_DIR_HORIZONTAL);
}

/**
  * @brief  ����ʵ�ľ���
  * @param  Xpos ���������ϽǶ˵�X����
  * @param  Ypos ���������ϽǶ˵�Y����
  * @param  Width �����ο�
  * @param  Height �����θ�
  * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
  * @retval ��
  */
void LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  uint16_t page, column; 
  
  uint16_t realHeight,realWidth;
  
  realHeight = LCD_PIXEL_HEIGHT-Ypos-Height > 0 ? Height : LCD_PIXEL_HEIGHT - Ypos;
  realWidth = LCD_PIXEL_WIDTH-Xpos-Width > 0 ? Width : LCD_PIXEL_WIDTH - Xpos;
  
  /* ָ����ε�һ�����ص���Դ�λ�� */
  uint32_t *pRectImage = (uint32_t*)(CurrentFrameBuffer + APP_BPP*(Xpos + (LCD_PIXEL_WIDTH*Ypos)));
  
  /* ����ÿһ�� */
  for ( page = 0; page < realHeight; page++ )
  {    
    /* ����ÿһ�� */
    for ( column = 0; column < realWidth; column++ ) 
    {	
      *pRectImage = CurrentTextColor;
      
      /* ָ����һ�����ص���Դ�λ�� */
      pRectImage++;
    }      
    /*��ʾ��һ��*/
    /*ָ����һ�еĵ�һ�����ص���Դ�λ��*/
    pRectImage += (LCD_PIXEL_WIDTH - realWidth);		
  }
}

/**
 * @brief  ����һ������Բ
 * @param  Xpos: Բ��X����
 * @param  Ypos: Բ��Y����
 * @param  Radius: �뾶
 * @retval None
 */
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
   int x = -Radius, y = 0, err = 2-2*Radius, e2;
   do {
       *(__IO uint32_t*) (CurrentFrameBuffer + (APP_BPP*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor;
       *(__IO uint32_t*) (CurrentFrameBuffer + (APP_BPP*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor;
       *(__IO uint32_t*) (CurrentFrameBuffer + (APP_BPP*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;
       *(__IO uint32_t*) (CurrentFrameBuffer + (APP_BPP*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;

       e2 = err;
       if (e2 <= y) {
           err += ++y*2+1;
           if (-x == y && e2 <= x) e2 = 0;
       }
       if (e2 > x) err += ++x*2+1;
   }
   while (x <= 0);
}

/**
 * @brief  ����һ��ʵ��Բ
 * @param  Xpos: Բ��X����
 * @param  Ypos: Բ��Y����
 * @param  Radius: �뾶
 * @retval None
 */
void LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{  
  /* ����ʵ��Բ��Ҫ���ӵĲ��� */
   int32_t  D;    /* Decision Variable */
   uint32_t  CurX;/* Current X Value */
   uint32_t  CurY;/* Current Y Value */

   D = 3 - (Radius << 1);

   CurX = 0;
   CurY = Radius;

   while (CurX <= CurY)
   {
     if(CurY > 0)
     {
       LCD_DrawLine(Xpos - CurX, Ypos - CurY, 2*CurY, LINE_DIR_VERTICAL);
       LCD_DrawLine(Xpos + CurX, Ypos - CurY, 2*CurY, LINE_DIR_VERTICAL);
     }

     if(CurX > 0)
     {
       LCD_DrawLine(Xpos - CurY, Ypos - CurX, 2*CurX, LINE_DIR_VERTICAL);
       LCD_DrawLine(Xpos + CurY, Ypos - CurX, 2*CurX, LINE_DIR_VERTICAL);
     }
     if (D < 0)
     {
       D += (CurX << 2) + 6;
     }
     else
     {
       D += ((CurX - CurY) << 2) + 10;
       CurY--;
     }
     CurX++; 
    }
 
   LCD_DrawCircle(Xpos, Ypos, Radius);

}
/*********************************************END OF FILE**********************/
