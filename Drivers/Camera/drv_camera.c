#include "pin_mux.h"
#include "fsl_csi_camera_adapter.h"
#include "fsl_camera_receiver.h"
#include "fsl_camera_device.h"
#include "drv_camera_device.h"
#include "drv_camera.h"

#include "fsl_gpio.h"


/*******************************************************************************
* Variables
******************************************************************************/
uint32_t activeFrameAddr = (uint32_t)&csi_frameBuffer[0];
uint32_t inactiveFrameAddr = (uint32_t)&csi_frameBuffer[0];
uint32_t CSI_FPS = 0;

#if defined(__CC_ARM) || defined(__GNUC__)
	AT_NONCACHEABLE_SECTION_ALIGN(volatile static uint16_t csi_frameBuffer[CAMERA_FRAME_BUFFER_COUNT][CAMERA_HEIGHT*CAMERA_WIDTH*CAMERA_BPP/sizeof(uint16_t)], FRAME_BUFFER_ALIGN);
#elif defined(__ICCARM__)
	#pragma location = ".csi_data"
	#pragma pack(8)
	volatile static uint16_t csi_frameBuffer[CAMERA_FRAME_BUFFER_COUNT][CAMERA_HEIGHT*CAMERA_WIDTH*CAMERA_BPP/sizeof(uint16_t)];
#endif

static csi_private_data_t csiPrivateData;


/* CSI������������ */
camera_receiver_handle_t cameraReceiver = {
    .resource = &(csi_resource_t){.csiBase = CSI},
    .ops = &csi_ops,
    .privateData = &csiPrivateData,
};

/* Camera�豸�������� */
const camera_device_handle_t cameraDevice = {
    .ops = &MT9V034_ops,
};

/* Camera�豸�������� */
const camera_config_t cameraConfig = {
    .pixelFormat = kVIDEO_PixelFormatRGB565,			//THIS PARAM HAS NO MEANNING
    .bytesPerPixel = CAMERA_BPP,
    .resolution = FSL_VIDEO_RESOLUTION(CAMERA_WIDTH, CAMERA_HEIGHT),
    .frameBufferLinePitch_Bytes = CAMERA_WIDTH*CAMERA_BPP,
    .interface = kCAMERA_InterfaceGatedClock,
    .controlFlags = CAMERA_CONTROL_FLAGS,
    .framePerSec = 50,									//THIS PARAM HAS NO MEANNING
};

/*******************************************************************************
* Code
******************************************************************************/
void CSI_IRQHandler(void)
{
    CSI_DriverIRQHandler();
	GPIO_WritePinOutput(GPIO1, 15U, !GPIO_PinRead(GPIO1, 15U));
	//CSI_FPS++;

	/////////////////////////////////////////////////////////

	CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &inactiveFrameAddr);
	CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, activeFrameAddr);

	activeFrameAddr = inactiveFrameAddr;
	LCDIF->CUR_BUF = inactiveFrameAddr;
}

void Camera_Start(void)
{
	BOARD_InitCSIPins();

	//Clear Video Frame Buffer
	//memset(csi_frameBuffer, 0, sizeof(csi_frameBuffer));
	volatile uint16_t * ptr_uint16 = (uint16_t *)csi_frameBuffer;
	for(uint32_t i=0; i <(sizeof(csi_frameBuffer)/sizeof(uint16_t)); i++)
	{
		*ptr_uint16++ = 0x0000;
	}

    CAMERA_RECEIVER_Init(&cameraReceiver, &cameraConfig, NULL, NULL);
    CAMERA_DEVICE_Init((camera_device_handle_t*)&cameraDevice, &cameraConfig);
    CAMERA_DEVICE_Start((camera_device_handle_t*)&cameraDevice);

	/* ����֡�������ύ������������. */
    for (uint32_t i = 0; i < CAMERA_FRAME_BUFFER_COUNT; i++)
    {
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)(csi_frameBuffer[i]));
    }
	//CSI_DisableInterrupts(CSI, 0xFFFFFFFF);
	//CSI_EnableInterrupts(CSI, CSI_CSICR1_EOF_INT_EN(true));

	CAMERA_RECEIVER_Start(&cameraReceiver);

	 /* �ȴ���ȡ����֡����������ʾ */
    while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &activeFrameAddr));
    while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &inactiveFrameAddr));
}