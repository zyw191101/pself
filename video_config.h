
#ifndef SRC_VIDEO_CONFIG_H_
#define SRC_VIDEO_CONFIG_H_
//Ip Ctrl Reg head Addr

#define VDMA_REG_ADDR_TV 							0x43000000	//VDMA
#define VDMA_REG_ADDR_IR							0x43010000
#define VDMA_REG_ADDR_PIP							0x43020000
#define VDMA_REG_ADDR_SDI_MIPI						0x43030000

#define ADDING_SDI_INFO_USING_BRAM		//	20230724 BRAM adding sdi info

#ifdef ADDING_SDI_INFO_USING_BRAM
	#define SDI_INFO_BRAM_ADDR						0x42000000
#endif

#define PHOTO_IN_DVP_OUT
#ifdef PHOTO_IN_DVP_OUT
	#define VDMA_REG_ADDR_PHOTO_IN_DVP_OUT					0x43040000
//	#define VTC_DVP_RESET
//    #define VTC_DVP_1080P
#endif

#define ADD_dot_line_mixer

//2023.01.05  to handle randomly happened gray screen--------------fengyachun & yangyifan
// enforece reset video in to axis-stream IPcore in init step
#define RST_UART_IN_PS_PROC
#define RST_VID_IN_PS_PROC

#define AXI_GPIO_RST_UART_ADDR						0x412e0000  //GPIO
#define RST_GPIO_RST_UART_OFFSET					0x00

#define AXI_GPIO_RST_VIDEO_INTO_AXIS 				0x41200000	//GPIO
#define RST_VIDEO_INTO_AXI4S_OFFSET					0x00
#define RST_IR_VIDEO_INTO_AXI4S_OFFSET				0x08

#define DETECT_VIDEO_INPUT							// 2023.01.31 added by fengyachun to detect video input & switch video output clk src
#define AXI_GPIO_DETECT_VIDEO_AND_SWITCH_CLK 		0x412D0000	//GPIO
#define DETECT_VIDEO_INPUT_STATUS_OFFSET			0x00
#define SWITCH_VIDEO_CLK_SRC						0x08
#define	EXTERN_REF_CLK  1
#define	AVT_GEN_CLK  	0


#define XADC						// 2023.01.31 added by fengyachun to report the temperature of FMQL-7045
#define XADC_ADDR 0x43C70000

#define GAMMA_LUT
#define BLACK_WHITE_SWITCH_IN_AVT
#define IR_WHITE_HOT								1
#define IR_BLACK_HOT								0
#define GAMMA_LUT_ADDR_TV							0x43c80000
#define GAMMA_LUT_ADDR_IR							0x43c90000

//// 20250526
//#define IR_COLORMAP
//
//#ifdef IR_COLORMAP
//#define IR_COLORMAP_ADDR							0x43c60000
//
//#define IR_COLOMAP_AUTO								0x00
//#define IR_COLOMAP_EN								0x10
//#define IR_COLOMAP_VER								0x18
//#define IR_COLOMAP_HOR								0x20
//
//#endif


#define GAMMA_LUT_ADDR_PIP							0x43CA0000

// #define only_median
// #define only_denoise
#define median_denoise

#ifdef only_median
	#define MEDIAN_MIPI_ADDR 0x43CB0000
#endif

#ifdef only_denoise
	#define DENOISE_MIPI_ADDR 0x43CB0000
#endif

#ifdef median_denoise
	#define MEDIAN_MIPI_ADDR 0x43CB0000
	#define DENOISE_MIPI_ADDR 0x43CC0000
#endif

#define GAMMA_CTRL									0x0000
#define GAMMA_WIDTH_IN_ACTIVE_PIXEL 				0x0010
#define GAMMA_HEIGHT_IN_ACTIVE_LINE 				0x0018
#define GAMMA_VIDEO_FORMATE 						0x0020			//0:RGB    		1:YUV
#define GAMMA_CHANNEL_RED_START_OFFSET				0x0800
#define GAMMA_CHANNEL_GREEN_START_OFFSET			0x1000
#define GAMMA_CHANNEL_BLUE_START_OFFSET				0x1800


#define IMAGE_RESIZE_REG_ADDR_STREAM_TV					0x43C10000	//Resize
#define IMAGE_RESIZE_REG_ADDR_STREAM_IR					0x43C20000
#define IMAGE_RESIZE_REG_ADDR_STREAM_PIP				0x43C30000	//Resize

#define MIXER_REG_ADDR									0x43C00000	//video Mxier

#define VTC_RESET
#define VTC_MIPI_RESET
//#define VTC_DVP_RESET			//2023.07.14 added by fengyachun reset DVP video timing controller  VTC IP core when input video clk detected or invalid

#ifdef VTC_RESET
	#define VTC_DISP_ADDR								0x43C40000	//video time controller	 -- DISP-1080P-30fps
	#ifdef VTC_MIPI_RESET
		#define VTC_MIPI_ADDR							0x43C50000	//video time controller  -- MIPI-1080P-60fps
	#endif
	#ifdef VTC_DVP_RESET
		#define VTC_DVP_ADDR							0x43C60000	// video time controller  -- DVP--1920*1252-30fps
	#endif
#endif
//*******PL image proc ctrl gpio port---start
#define AXI_GPIO_CROSS_CTRL								0x41280000	//gpio
#define AXI_GPIO_TAR_GRAPH_COLOR_ADDR 					0x41270000

#define CTRL_OFFSET_PORT1		0x00
#define CTRL_OFFSET_PORT2		0x08

#define GPIO_CROSS_MAIN 	0x00
#define GPIO_CROSS_PIP 		0x08

#define	IR_HOT_BLACK			0
#define IR_HOT_WHITE			1
#define IMAGE_ENHANCE_TV_OFF 	0
#define IMAGE_ENHANCE_TV_ON 	1
#define IMAGE_ENHANCE_IR_OFF 	0
#define IMAGE_ENHANCE_IR_ON 	1
//*******PL image proc ctrl gpio port---end

//***************VDMA register addr and reg offset ***************//
#define VDMA_REG_OFFSET_S2MM_VDMACR					0x30
#define VDMA_REG_OFFSET_S2MM_START_ADDR				0xAC
#define VDMA_REG_OFFSET_S2MM_STRIDE_IN_BYTE			0xA8
#define VDMA_REG_OFFSET_S2MM_ACITVE_WIDTH_IN_BYTE	0xA4
#define VDMA_REG_OFFSET_S2MM_ACITVE_ROWS			0xA0

#define VDMA_REG_OFFSET_MM2S_VDMACR					0x00
#define VDMA_REG_OFFSET_MM2S_START_ADDR				0x5c
#define VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE			0x58
#define VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE	0x54
#define VDMA_REG_OFFSET_MM2S_ACITVE_ROWS			0x50

//***************Image Resize register addr and reg offset ***************//
#define IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT  		0x14	//step1
#define IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH  			0x1c	//step1
#define IMAGE_RESIZE_REG_OFFSET_DST_HEIGHT  		0x24	//step2
#define IMAGE_RESIZE_REG_OFFSET_DST_WIDTH  			0x2c	//step2
#define IMAGE_RESIZE_REG_OFFSET_AP_CTRL  			0x00	//step3

//***************Video Mixer register addr and reg offset ***************//
#define	MIXER_REG_OFFSET_LAYER_ENABLE				0x40	// step 1 (set 0x00   --  0b0000 0000)
#define MIXER_REG_OFFSET_BG_WIDTN					0x10 	// step 2 (set 1920)
#define MIXER_REG_OFFSET_BG_HEIGHT					0x18	// step 2 (set 1080)
#define MIXER_REG_OFFSET_BG_COLOR_Y					0x28
#define MIXER_REG_OFFSET_BG_COLOR_U					0x30
#define MIXER_REG_OFFSET_BG_COLOR_V					0x38
#define MIXER_REG_OFFSET_AP_CTRL					0x00	//0x81

#define MIXER_REG_OFFSET_LAYER2_G_ALPHA				0x0200
#define MIXER_REG_OFFSET_LAYER2_START_COLS			0x0208
#define MIXER_REG_OFFSET_LAYER2_START_ROWS			0x0210
#define MIXER_REG_OFFSET_LAYER2_WIDTH				0x0218
#define MIXER_REG_OFFSET_LAYER2_HEIGHT				0x0228
#define MIXER_REG_OFFSET_LAYER2_SCALE				0x0230	//(set 0----No scaling,   1----2x scaling,  2----4x scaling)

#define MIXER_REG_OFFSET_LAYER3_G_ALPHA				0x0300
#define MIXER_REG_OFFSET_LAYER3_START_COLS			0x0308
#define MIXER_REG_OFFSET_LAYER3_START_ROWS			0x0310
#define MIXER_REG_OFFSET_LAYER3_WIDTH				0x0318
#define MIXER_REG_OFFSET_LAYER3_STRIDE				0x0320
#define MIXER_REG_OFFSET_LAYER3_HEIGHT				0x0328
#define MIXER_REG_OFFSET_LAYER3_SCALE				0x0330	//(set 0----No scaling,   1----2x scaling,  2----4x scaling)
#define MIXER_REG_OFFSET_LAYER3_BUF     			0x0340

#define MIXER_REG_OFFSET_LAYER4_G_ALPHA				0x0400
#define MIXER_REG_OFFSET_LAYER4_START_COLS			0x0408
#define MIXER_REG_OFFSET_LAYER4_START_ROWS			0x0410
#define MIXER_REG_OFFSET_LAYER4_WIDTH				0x0418
#define MIXER_REG_OFFSET_LAYER4_STRIDE				0x0420
#define MIXER_REG_OFFSET_LAYER4_HEIGHT				0x0428
#define MIXER_REG_OFFSET_LAYER4_SCALE				0x0430	//(set 0----No scaling,   1----2x scaling,  2----4x scaling)
#define MIXER_REG_OFFSET_LAYER4_BUF     			0x0440

#define MIXER_REG_OFFSET_LAYER5_G_ALPHA				0x0500
#define MIXER_REG_OFFSET_LAYER5_START_COLS			0x0508
#define MIXER_REG_OFFSET_LAYER5_START_ROWS			0x0510
#define MIXER_REG_OFFSET_LAYER5_WIDTH				0x0518
#define MIXER_REG_OFFSET_LAYER5_STRIDE				0x0520
#define MIXER_REG_OFFSET_LAYER5_HEIGHT				0x0528
#define MIXER_REG_OFFSET_LAYER5_SCALE				0x0530	//(set 0----No scaling,   1----2x scaling,  2----4x scaling)
#define MIXER_REG_OFFSET_LAYER5_BUF     			0x0540

#define MIXER_REG_OFFSET_LAYER6_G_ALPHA				0x0600
#define MIXER_REG_OFFSET_LAYER6_START_COLS			0x0608
#define MIXER_REG_OFFSET_LAYER6_START_ROWS			0x0610
#define MIXER_REG_OFFSET_LAYER6_WIDTH				0x0618
#define MIXER_REG_OFFSET_LAYER6_STRIDE				0x0620
#define MIXER_REG_OFFSET_LAYER6_HEIGHT				0x0628
#define MIXER_REG_OFFSET_LAYER6_SCALE				0x0630	//(set 0----No scaling,   1----2x scaling,  2----4x scaling)
#define MIXER_REG_OFFSET_LAYER6_BUF     			0x0640



#define TXT_LAYER_CHANNEL							4
#define TXT_LAYER_WIDTH								1920
#define TXT_LAYER_STRIDE							7680		//1920*4
#define TXT_LAYER_HEIGHT							1080
#define TXT_LAYER_FB_LENGTH 						8294400		//1920*4*1080


//video-base info

// TV Info
#define PARA_IMG_CHANNELS_TV 						3
#define PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880			8640        //2880*3
#define PARA_IMG_BUF_ACITVE_ROWS_TV_1620			1620		//for image fusion
#define fbLegnth_TV									13996800	//2880*3*1620
#define PARA_IMG_BUF_STRIDE_IN_PIXEL_TV_2880		2880
#define PARA_IMG_BUF_ACITVE_WIDTH_IN_BYTE_TV		5760		//1920*3
#define PARA_IMG_BUF_ACITVE_PIXEL_TV				1920
#define PARA_IMG_BUF_ACITVE_ROWS_TV					1080




// IR Info
#define PARA_IMG_CHANNELS_IR 						3
#define PARA_IMG_BUF_STRIDE_IN_BYTE_IR				5760		//1920*3
#define PARA_IMG_BUF_STRIDE_IN_PIXEL_IR				1920
#define PARA_IMG_BUF_ACITVE_WIDTH_IN_BYTE_IR		1920		//640*3
#define PARA_IMG_BUF_ACITVE_PIXEL_IR				640
#define PARA_IMG_BUF_ACITVE_ROWS_IR					512
#define PARA_IMG_BUF_FOR_MIPI_ROWS_IR				1080
#define fbLegnth_IR									6220800 //6220800(1080*3*1920)		//1920*3*1080 		6291456(0x00600000)


//DISP info
#define DISP_WIDTH_TV			1920
#define DISP_HEIGHT_TV			1080
#define DISP_WIDTH_IR			1280
#define DISP_HEIGHT_IR			1024

//PIP info
#define PIP_SRC_HEIGHT_IR 		180
#define PIP_SRC_WIDTH_IR 		320

#define PIP_SRC_HEIGHT_TV 		360
#define PIP_SRC_WIDTH_TV		640


//#define VDMA_IMG_BUF_ADDR_TV_1						0x31000000	//start: 0x31000000
//#define VDMA_IMG_BUF_ADDR_TV_2						0x31E00000	//start: 0x31E00000 = 0x31000000 + 0x00E00000    ----- 1920 * 1.5 * 3 * 1080 * 1.5 = 2880*3*1620 (dec)= 0x00D59300 (hex) need approximate to 0x00E00000 to avoid segmentation fault
//#define VDMA_IMG_BUF_ADDR_TV_3						0x32C00000	//start: 0x32C00000 = 0x31E00000 + 0x00E00000     end 0x32C00000 + 0x00E00000 =  0x33A00000
//
//#define VDMA_IMG_BUF_ADDR_IR_1						0x33A00000	//start: 0x33A00000
//#define VDMA_IMG_BUF_ADDR_IR_2						0x33FEEC00	//start: 0x33FEEC00 = 0x33A00000 + 0x005EEC00      5EEC00 =1920*3*1080
//#define VDMA_IMG_BUF_ADDR_IR_3						0x345DD800	//start: 0x345DD800 = 0x33FEEC00 + 0x005EEC00      5EEC00 =1920*3*1080
//#define VDMA_IMG_BUF_ADDR_IR_4						0x34BCC400	//start: 0x34BCC400 = 0x345DD800 + 0x005EEC00      5EEC00 =1920*3*1080

//************************************************************************************important notice**********************************************************************//
// when using linux system func mmap to map physic addr to virtual addr(or pointer), the start addr must be the integer multiple of 4k(4*1024=4096(dec)=1000(hex))
#define VDMA_IMG_BUF_ADDR_TV_1						0x31000000	//start: 0x31000000
#define VDMA_IMG_BUF_ADDR_TV_2						0x31E00000	//start: 0x31E00000 = 0x31000000 + 0x00E00000    ----- 1920 * 1.5 * 3 * 1080 * 1.5 = 2880*3*1620 (dec)= 0x00D59300 (hex) "*****0x00D59300 need approximate to 0x00E00000 to avoid segmentation fault*****"
#define VDMA_IMG_BUF_ADDR_TV_3						0x32C00000	//start: 0x32C00000 = 0x31E00000 + 0x00E00000     end 0x32C00000 + 0x00E00000 =  0x33A00000

#define VDMA_IMG_BUF_ADDR_IR_1						0x33A00000	//start: 0x33A00000
#define VDMA_IMG_BUF_ADDR_IR_2						0x34000000	//start: 0x34000000 = 0x33A00000 + 0x00600000      0x00600000 > 5EEC00 =1920*3*1080 									"*****0x005EEC00 need approximate to 0x00600000 to avoid segmentation fault*****"
#define VDMA_IMG_BUF_ADDR_IR_3						0x34600000	//start: 0x34600000 = 0x34000000 + 0x00600000
#define VDMA_IMG_BUF_ADDR_IR_4						0x34C00000	//start: 0x34c00000 = 0x34600000 + 0x00600000
#define VDMA_IMG_BUF_ADDR_IR_5						0x35200000	//start: 0x35200000 = 0x34C00000 + 0x00600000
#define VDMA_IMG_BUF_ADDR_IR_6						0x35800000	//start: 0x35800000 = 0x35200000 + 0x00600000
#define VDMA_IMG_BUF_ADDR_IR_7						0x35E00000	//start: 0x35E00000 = 0x35800000 + 0x00600000
#define VDMA_IMG_BUF_ADDR_IR_8						0x36400000	//start: 0x36400000 = 0x35E00000 + 0x00600000

//#define CLEAR_IR_BUF

#ifdef CLEAR_IR_BUF
	#define VDMA_IMG_BUF_ADDR_IR_9						0x36A00000	//start: 0x36A00000 = 0x36400000 + 0x00600000
#endif
#define VDMA_IMG_BUF_ADDR_TV_RK3399_1				0x36A00000	//start: 0x36A00000 = 0x36400000 + 0x00600000
#define VDMA_IMG_BUF_ADDR_TV_RK3399_2				0x370F0000	//start: 0x370F0000 = 0x36A00000 + 0x006F0000		1920*1252*3=0x6E0A00 ~= 0x6f0000
#define VDMA_IMG_BUF_ADDR_TV_RK3399_3				0x377E0000	//start: 0x377E0000 = 0x370F0000 + 0x006F0000

#define ARROW_WIDTH		80
#define ARROW_HEIGHT	80
#define MIXER_NORTH_ARROW_BUF						0x37ED0000	//start: 0x37ED0000 = 0x377E0000 + 0x006F0000
																//end:   0x37EE0000 = 0x37ED0000 +    0x10000
#define MIXER_TXT_BUF_ADDR							0x37EE0000  //0x386C9000 = 0x37EE0000 + 0x007E9000(4*1920*1080)
//2023.07.06 for added state msg from HOST to YUV444 Y channel
#define MIXER_HOST_INFO_BUF_ADDR					0x386D0000  //0x386E9000 = 0x386C9000 + 0x00020000(4*64*512 = 0x20000)
//#define SDI_ADD_INFO

#ifdef PHOTO_IN_DVP_OUT
	#define SEND_PHOTO_CMD_STAT_MEM_SHARED					0x23FFF000
	#define VDMA_IMG_BUF_ADDR_PHOTO_1						0x24000000	//start: 0x21000000 end:0x27C00000 = 0x24000000 + 0x03C00000
	#define VDMA_IMG_BUF_ADDR_PHOTO_2						0x27C00000	//start: 0x24C00000 end:0x2B800000 = 0x27C00000 + 0x03C00000
	#define VDMA_IMG_BUF_ADDR_PHOTO_3						0x2B800000	//start: 0x28800000 end:0x2F400000 = 0x2B800000 + 0x03C00000
#endif

//#define IR_CLOSE_VDMA_S2MM_20240125

#endif /* SRC_VIDEO_CONFIG_H_ */
