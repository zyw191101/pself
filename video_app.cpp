/*
 * video_app.cpp
 *
 *  Created on: 2022-11-01
 *      Author: FYC
 */

#include "video_app.h"
#include <opencv2/opencv.hpp>
#include <math.h>
using namespace cv;

unsigned int* VDMA_TV_HANDLE 				= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(VDMA_REG_ADDR_TV+0x0));
unsigned int* VDMA_IR_HANDLE 				= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(VDMA_REG_ADDR_IR+0x0));
unsigned int* VDMA_PIP_HANDLE 				= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(VDMA_REG_ADDR_PIP+0x0));
unsigned int* VDMA_SDI_MIPI_HANDLE			= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(VDMA_REG_ADDR_SDI_MIPI+0x0));

#ifdef ADDING_SDI_INFO_USING_BRAM									//4kb=4*1024
	unsigned int* SDI_INFO_BRAM_HANDLE = (unsigned int*)mmap(NULL, 4*1024, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)SDI_INFO_BRAM_ADDR);
#endif


#ifdef PHOTO_IN_DVP_OUT
	unsigned int* VDMA_PHOTO_IN_DVP_OUT_HANDLE = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(VDMA_REG_ADDR_PHOTO_IN_DVP_OUT+0x0));
#endif

#ifdef PHOTO_IN_DVP_OUT
	bool lock_dvp_start = false;
	int photo_in_cnt = 0;
	unsigned int FrmWR_ID_PHOTO = 0;
	unsigned int FrmWR_SUB_ID_PHOTO_DVP_OUT = 0;
	VIDEO_INFO photo_info_PHOTO_IN;
	VIDEO_INFO photo_info_DVP_OUT;
	unsigned char* pChar_VDMA_IMG_BUF_PHOTO_1	= (unsigned char*)mmap(NULL, 3*3*5120*4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_PHOTO_1);
	unsigned char* pChar_VDMA_IMG_BUF_PHOTO_2	= (unsigned char*)mmap(NULL, 3*5120*4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_PHOTO_2);
	unsigned char* pChar_VDMA_IMG_BUF_PHOTO_3	= (unsigned char*)mmap(NULL, 3*5120*4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_PHOTO_3);
	unsigned char* send_pt_cmd_stat_shared 		= (unsigned char*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)SEND_PHOTO_CMD_STAT_MEM_SHARED);
	PHOTO_CMD_STAT_SHARED *photo_cmd_stat_shared;
	// #define DEBUG_MODE_SEND_PHOTO

#endif

#ifdef 	DVP_OUT_STAB_TEST
	double 	omp_time_pt_in_pre;
	double 	omp_time_pt_in_cur;
	double	pt_in_time_gap;
	bool 	enable_print_time_gap_pt_in;

	double 	omp_time_dvp_out_pre;
	double 	omp_time_dvp_out_cur;
	double	dvp_out_time_gap;
	bool 	enable_print_time_gap_dvp_out;

	double	pt_dvp_time_gap;
#endif

#ifdef RST_UART_IN_PS_PROC
// 2024.03.21  to handle randomly happened restart cause of uart bus error--------------zhangzeyu
// enforece reset uart-lite IPcore in init step
	unsigned int* AXI_GPIO_RST_UART_HANDLE	    = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_RST_UART_ADDR));
#endif

#ifdef RST_VID_IN_PS_PROC
//2023.01.05  to handle randomly happened gray screen--------------fengyachun & yangyifan
// enforece reset video in to axis-stream IPcore in init step
	unsigned int* AXI_GPIO_RST_VID_HANDLE		= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_RST_VIDEO_INTO_AXIS));
#endif

#ifdef XADC		// 2023.01.31 added by fengyachun to report the temperature of FMQL-7045
	unsigned int* XADC_HANDLE 	= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(XADC_ADDR));
	bool requst_temper = false;
#endif

#ifdef DETECT_VIDEO_INPUT		// 2023.01.31 added by fengyachun to detect video input & switch video output clk src
	//2023.01.30
	unsigned int* AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_DETECT_VIDEO_AND_SWITCH_CLK));
	CLK_INFO switch_clk;
    VIDEO_INPUT_STATUS video_status;

#endif



#ifdef GAMMA_LUT
	unsigned int* GAMMA_LUT_TV_HANDLE		 = (unsigned int*)mmap(NULL, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)GAMMA_LUT_ADDR_TV);
	unsigned int* GAMMA_LUT_IR_HANDLE		 = (unsigned int*)mmap(NULL, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)GAMMA_LUT_ADDR_IR);
#ifdef ENABLE_GAMMA_LUT_PIP
	unsigned int* GAMMA_LUT_PIP_HANDLE		 = (unsigned int*)mmap(NULL, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)GAMMA_LUT_ADDR_PIP);
#endif
	int curve_Y_tv[256] = {0};
	int curve_U_tv[256] = {0};
	int curve_V_tv[256] = {0};

	int curve_Y_ir_src[256] = {0};
	int curve_U_ir_src[256] = {0};
	int curve_V_ir_src[256] = {0};

	int curve_Y_ir_enh[256] = {0};
	bool ir_enhance_switch_happened = false;
	bool ir_enhance_on = false;
	#ifdef BLACK_WHITE_SWITCH_IN_AVT
		int curve_Y_ir_cvt[256] = {0};
		bool ir_black_white_switch_happened = false;
		bool ir_black_white = IR_BLACK_HOT; //IR_WHITE_HOT
	#endif
#endif

#ifdef only_median
		unsigned int* MEDIAN_MIPI_HANDLE = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(MEDIAN_MIPI_ADDR));
#endif

#ifdef only_denoise
		unsigned int* DENOISE_MIPI_HANDLE = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(DENOISE_MIPI_ADDR));
#endif

#ifdef median_denoise
		unsigned int* DENOISE_MIPI_HANDLE = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(DENOISE_MIPI_ADDR));
		unsigned int* MEDIAN_MIPI_HANDLE = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(MEDIAN_MIPI_ADDR));
#endif

unsigned int* IMG_RESIZE_STREAM_TV_HANDLE 	= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(IMAGE_RESIZE_REG_ADDR_STREAM_TV+0x0));
unsigned int* IMG_RESIZE_STREAM_IR_HANDLE 	= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(IMAGE_RESIZE_REG_ADDR_STREAM_IR+0x0));
unsigned int* IMG_RESIZE_STREAM_PIP_HANDLE 	= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(IMAGE_RESIZE_REG_ADDR_STREAM_PIP+0x0));
unsigned int* VIDEO_MIXER_HANDLE 			= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(MIXER_REG_ADDR+0x0));

#ifdef VTC_RESET
	unsigned int* VTC_DISP_HANDLE					= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(VTC_DISP_ADDR+0x0));
	#ifdef VTC_MIPI_RESET
		unsigned int* VTC_MIPI_HANDLE					= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(VTC_MIPI_ADDR+0x0));
	#endif
	#ifdef VTC_DVP_RESET
		unsigned int* VTC_DVP_HANDLE					= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(VTC_DVP_ADDR+0x0));
	#endif
#endif

unsigned int* GPIO_CROSS_CTRL_HANDLE		= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_CROSS_CTRL+0x0));
unsigned int* GPIO_TAR_Graph_Color_HANDLE	= (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR_GRAPH_COLOR_ADDR));

// mmap TV img buf
unsigned char* pChar_VDMA_IMG_BUF_TV_1		= (unsigned char*)mmap(NULL, fbLegnth_TV, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_TV_1);
unsigned char* pChar_VDMA_IMG_BUF_TV_2		= (unsigned char*)mmap(NULL, fbLegnth_TV, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_TV_2);
unsigned char* pChar_VDMA_IMG_BUF_TV_3		= (unsigned char*)mmap(NULL, fbLegnth_TV, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_TV_3);
// mmap IR img buf
unsigned char* pChar_VDMA_IMG_BUF_IR_1		= (unsigned char*)mmap(NULL, fbLegnth_IR, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_IR_1);
unsigned char* pChar_VDMA_IMG_BUF_IR_2		= (unsigned char*)mmap(NULL, fbLegnth_IR, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_IR_2);
unsigned char* pChar_VDMA_IMG_BUF_IR_3		= (unsigned char*)mmap(NULL, fbLegnth_IR, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_IR_3);
unsigned char* pChar_VDMA_IMG_BUF_IR_4		= (unsigned char*)mmap(NULL, fbLegnth_IR, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_IR_4);
unsigned char* pChar_VDMA_IMG_BUF_IR_5		= (unsigned char*)mmap(NULL, fbLegnth_IR, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_IR_5);
unsigned char* pChar_VDMA_IMG_BUF_IR_6		= (unsigned char*)mmap(NULL, fbLegnth_IR, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_IR_6);
unsigned char* pChar_VDMA_IMG_BUF_IR_7		= (unsigned char*)mmap(NULL, fbLegnth_IR, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_IR_7);
unsigned char* pChar_VDMA_IMG_BUF_IR_8		= (unsigned char*)mmap(NULL, fbLegnth_IR, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)VDMA_IMG_BUF_ADDR_IR_8);


// mmap Video mixer TXT layer img buf
#ifdef ADD_dot_line_mixer
	unsigned char* ptr_MIXER_LAYER_TXT_BUF		= (unsigned char*)mmap(NULL, TXT_LAYER_FB_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)MIXER_TXT_BUF_ADDR);
#endif

#ifdef	SDI_ADD_INFO
	unsigned char* ptr_MIXER_HOST_INFO_BUF_ADDR	= (unsigned char*)mmap(NULL, 4*512*64, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)MIXER_HOST_INFO_BUF_ADDR);
#endif
unsigned char* ptr_MIXER_LAYER_NORTH_ARROW_BUF = (unsigned char*)mmap(NULL, 4*ARROW_WIDTH*ARROW_HEIGHT, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)MIXER_NORTH_ARROW_BUF);
static cv::Mat arrow_patch_center_mask(cv::Size(ARROW_WIDTH,ARROW_HEIGHT),CV_8UC1);

VIDEO_INFO video_info_TV_IN;
VIDEO_INFO video_info_IR_IN;
VIDEO_INFO video_info_MIPI_OUT;
VIDEO_INFO video_info_DISP_OUT;

#ifdef SUM_DELAY
	VIDEO_INFO video_info_SUM_delay;
#endif

DISP_CTRL disp_ctrl;
MIPI_VIDEO_CTRL mipi_video_ctrl;
PL_IMG_PROC_CTRL pl_imgproc_ctrl;

CROSS_INFO cross_Main;
CROSS_INFO cross_PIP;
#ifdef	IR_CLOSE_VDMA_S2MM_20240125
	bool ir_power_on_stat = false;
	int ir_power_on_cnt = 0;
	bool cur_ir_s2mm_stat = false;
#endif

int 	ir_in_cnt 			= 0;
int 	tv_in_cnt 			= 0;
int 	disp_out_cnt 		= 0;
int 	mipi_out_cnt 		= 0;

#ifdef DEBUG_MIPI_IR_DELAY
	double 	ir_in_latest_omp_time 				= 0.0;
	double 	cur_mipi_out_omp_time 				= 0.0;
	bool	print_diff_time 					= false;
	int		mipi_ir_cnt 						= 0;
	int 	cur_read_id_mipi 					= 0;
	int 	cur_write_id_ir 					= 0;
	double	omp_start_time 						= 0.0;
	double	time_gap_MIPI_miner_IR  			= 0.0;
	bool 	switch_mipi_out_pingpong_strategy 	= false;
#endif

#ifdef 	TEST_TV_IN_TIME_STABLIZATION
	double 	omp_time_tv_in_pre				= 0.0;
    double 	omp_time_tv_in_cur				= 0.0;
    double	tv_in_time_gap					= 0.0;
	bool 	enable_print_time_gap_tv_in 	= false;
	double 	omp_time_ir_in_pre 				= 0.0;
    double 	omp_time_ir_in_cur 				= 0.0;
    double	ir_in_time_gap					= 0.0;
    bool 	enable_print_time_gap_ir_in 	= false;

	double 	omp_time_disp_out_pre			= 0.0;
    double 	omp_time_disp_out_cur			= 0.0;
    double	disp_out_time_gap				= 0.0;
	double 	omp_time_mipi_out_pre 			= 0.0;
    double 	omp_time_mipi_out_cur 			= 0.0;
    double	mipi_out_time_gap				= 0.0;

#endif


#ifdef MEASURE_LOOP_TIME
	std::chrono::system_clock::time_point time_TV_IN;
	std::chrono::system_clock::time_point time_DISP_OUT;
	std::chrono::system_clock::time_point time_MIPI_OUT;
	bool bias_ctrl_left_mipi = 0;
	bool bias_hapened = true;
	bool bias_rst_hapened = true;

#endif

//int src_height_IR 	= 512/disp_ctrl.SCALING_RATE_IR;
//int src_width_IR 	= 640/disp_ctrl.SCALING_RATE_IR;
//int CURRENT_OFFSET_ADDR_IR = (256 - 256/disp_ctrl.SCALING_RATE_IR) * PARA_IMG_BUF_STRIDE_IN_BYTE_IR + (320 - 320/disp_ctrl.SCALING_RATE_IR) * PARA_IMG_CHANNELS_IR ;
//disp_ctrl.SCALING_RATE_IR=1		CURRENT_OFFSET_ADDR_IR 	= 0 			= (256 - 256/1)*5760 + (320 - 320/1)*3	= 0
//									src_height_IR			= 512
//									src_width_IR			= 640
//disp_ctrl.SCALING_RATE_IR=2		CURRENT_OFFSET_ADDR_IR 	= 737760		= (256 - 256/2)*5760 + (320 - 320/2)*3	= 128*5760 + 160*3
//									src_height_IR			= 256
//									src_width_IR			= 320
//disp_ctrl.SCALING_RATE_IR=4		CURRENT_OFFSET_ADDR_IR 	= 1106640		= (256 - 256/4)*5760 + (320 - 320/4)*3	= 192*5760 + 240*3
//									src_height_IR			= 128
//									src_width_IR			= 160

//int src_height_TV 	= 1080/disp_ctrl.SCALING_RATE_TV;
//int src_width_TV 	= 1920/disp_ctrl.SCALING_RATE_TV;
//int CURRENT_OFFSET_ADDR_TV = (540 - 540 / disp_ctrl.SCALING_RATE_TV) * PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + (960 - 960 /disp_ctrl.SCALING_RATE_TV) * PARA_IMG_CHANNELS_TV ;
//disp_ctrl.SCALING_RATE_TV=1		CURRENT_OFFSET_ADDR_TV 	= DEFAULT_OFFSET_ADDR_TV 			= (540 - 540/1)*8640 + (960 - 960/1)*3	= 0
//									src_height_TV			= 1080
//									src_width_TV			= 1920
//disp_ctrl.SCALING_RATE_TV=2		CURRENT_OFFSET_ADDR_TV 	= DEFAULT_OFFSET_ADDR_TV + 2334240		= (540 - 540/2)*8640 + (960 - 960/2)*3	= 270*8640 + 480*3
//									src_height_TV			= 540
//									src_width_TV			= 960
//disp_ctrl.SCALING_RATE_TV=4		CURRENT_OFFSET_ADDR_TV 	= DEFAULT_OFFSET_ADDR_TV + 3501360		= (540 - 540/4)*8640 + (960 - 960/4)*3	= 405*8640 + 720*3
//									src_height_TV			= 270
//									src_width_TV			= 480

const int DEFAULT_OFFSET_ADDR_TV	= 2334240 ;     //2334240  = 2880*3*270 + 480*3
static int PIP_OFFSET_ADDR_TV		= 5446560; 	//2334240+360*2880*3+640*3;
static int PIP_OFFSET_ADDR_IR		= 956640; 	//166*5760+160*3;

int FrmWriteID_IR = 0;
int FrmWriteID_TV = 0;
int FrmWriteID_TV_RK3399 = 0;

static int pip_height_TV            = PIP_SRC_HEIGHT_TV;
static int pip_width_TV             = PIP_SRC_WIDTH_TV;
static int src_height_TV 			= 1080;
static int src_width_TV 			= 1920;
static int CURRENT_OFFSET_ADDR_TV 	= DEFAULT_OFFSET_ADDR_TV;  //2334240=270 * 2880 * 3 + 480 * 3 = 270 * PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 480 * PARA_IMG_CHANNELS_TV;
static int src_height_IR 			= 512;
static int src_width_IR 			= 640;
static int CURRENT_OFFSET_ADDR_IR 	= 0;

/////////// important    2022.12.29 fyc
bool rst_lt9211 = false;
int rst_lt9211_cnt = 0;
bool new_mipi_stream_setup = false;
//VTC ctrl register def
//	bit31:	Writing a '1' resets the core. This bit automatically clears when reset complete.
//	bit30:	Writing a '1' resets the core after the start of the next input frame. This bit automatically clears when reset complete
//	bit29~27:	Reserved
//	bit26	Field ID Polarity Source Select 0: selects generated polarity from detection register (0x002c); 1: selects generated polarity from generators register (0x006c)
//	bit25	Active Chroma Polarity Source Select 0: selects generated polarity from detection register (0x002c); 1: selects generated polarity from generator register (0x006c)
//	bit24	Active Video Polarity Source Select 0: selects generated polarity from detection register (0x002c);1: selects generated polarity from generator register (0x006c)
//	bit23	Horizontal Sync Polarity Source Select	0: selects generated polarity from detection register (0x002c)	1: selects generated polarity from generator register (0x006c)
//	bit22	Vertical Sync Polarity Source Select 	0: selects generated polarity from detection register (0x002c)	1: selects generated polarity from generator register (0x006c)
//	bit21	Horizontal Blank Polarity Source Select	0: selects generated polarity from detection register (0x002c)	1: selects generated polarity from generator register (0x006c)
//	bit20	Vertical Blank Polarity Source Select	0: selects generated polarity from detection register (0x002c)	1: selects generated polarity from generator register (0x006c)
//	bit19	RESERVED
//	bit18	Generator Chroma Polarity and Encoding Source Select 0: selects Polarity and encoding from detection registers 0x0028 and 0x002C. 1: selects Polarity and encoding from generator registers 0x0068 and 0x006C
//	bit17	Generator Vertical Blank Offset Source Select 0: selects F0_VBLANK_HSTART from detection register (0x003c) selects F0_VBLANK_HEND from detection register (0x003c) 1: selects F0_VBLANK_HSTART from generator register (0x007c) selects F0_VBLANK_HEND from generator register (0x007c)
//	bit16	Generator Vertical Sync End Source Select 0: selects F0_VSYNC_HEND from detection register (0x0044) selects F0_VSYNC_VEND from detection register (0x0040) 1: selects F0_VSYNC_HEND from generator register (0x0084) selects F0_VSYNC_VEND from generator register (0x0080)
//	bit15	Generator Vertical Sync Start Source Select 0: selects F0_VSYNC_HSTART from detection register (0x0044) selects F0_VSYNC_VSTART from detection register (0x0040) 1: selects F0_VSYNC_HSTART from generator register (0x0084) selects F0_VSYNC_VSTART from generator register (0x0080)
//	bit14	Generator Vertical Active Size Source Select 0: selects ACTIVE_VSIZE from detection register (0x0020) 1: selects ACTIVE_VSIZE from generator register (0x0060)
//	bit13	Generator Vertical Frame Size Source Select 0: selects FRAME_VSIZE from detection register (0x0034) 1: selects FRAME_VSIZE from generator register (0x0074)
//	bit12	Reserved
//	bit11	Generator Horizontal Sync End Source Select 0: selects HSYNC_END from detection register (0x0038) 1: selects HSYNC_END from generator register (0x0078)
//	bit10	Generator Horizontal Sync Start Source Select 0: selects HSYNC_START from detection register (0x0038) 1: selects HSYNC_START from generator register (0x0078)
//	bit9	Generator Horizontal Active Size Source Select 0: selects ACTIVE_HSIZE from detection register (0x0020) 1: selects ACTIVE_HSIZE from generator register (0x0060)
//	bit8	Generator Horizontal Frame Size Source Selec 0: selects FRAME_HSIZE from detection register (0x0030) 1: selects FRAME_HSIZE from generator register (0x0070)
//	bit7	Reserved
//	bit6	Reserved
//	bit5	Generator Synchronization Enable. Enables the generator to synchronize to the Detector or to the fsync_in pin. 1: Generator synchronizes to the Detector or to fsync_in 0: Generator does not synchronize.
//	bit4	Reserved
//	bit3	Detection Enable.1: Perform timing signal detection for enabled signals.0: If SW_ENABLE is '0', No detection will be performed. All 'locked' status bits will be driven low. SW_ENABLE must be '0' to utilize the DET_ENABLE bit. If SW_ENABLE is '1', both the detector and generator will be enabled
//	bit2	Generation Enable. 1: Enable hardware to generate output. Set this bit high only after the software has configured the generator registers. 0: If SW_ENABLE is '0',
				//The generation hardware will not generate video timing output signals. SW_ENABLE must be '0' to utilize the DET_ENABLE bit. If SW_ENABLE is '1', both the detector and generator will be enabled.
//	bit1		Register Update. Generator and Fsync Registers are double-buffered. 1: Update the Generator and Fsync registers at the start of next frame. 0: Do not update the Generator and Fsync registers.
//	bit0		Core Enable. 1: Enable both the Video Timing Generator and Detector. 0: Generator or Detector can be selectively enabled with bits 2 and 3 of the CONTROL register.


void reset_VTC_IN_SYNC_MODE()
{
#ifdef VTC_RESET
			VTC_DISP_HANDLE[0x00]	= 0xC0000000;	asm("NOP");	//		Page.23 <PG016 Video Timing Controller v6.1>   0xC0000000

	#ifdef 	VTC_MIPI_RESET
			VTC_MIPI_HANDLE[0x00]	= 0xC0000000;	asm("NOP");	//      0xC0000000
	#endif

	#ifdef VTC_DVP_RESET
			VTC_DVP_HANDLE[0x00]	= 0x80000000;	asm("NOP");	//      0xC0000000
	#endif

			VTC_DISP_HANDLE[0x00]	= 0x00000026;	asm("NOP");	// enable generate"           fync_in 0x00000026

	#ifdef 	VTC_MIPI_RESET
			VTC_MIPI_HANDLE[0x00]	= 0x00000026;	asm("NOP");	// enable generate"0x0000002F fync_in 0x00000026
	#endif
	rst_lt9211 = true;			/////20230102   --- important reset the LT9211 when ps_app start/restart, ensure the XJ3 get video stream
	new_mipi_stream_setup = false;
	pl_imgproc_ctrl.CTRL_CMD.LT9211_Rst 				= 0;
	GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");

	#ifdef VTC_DVP_RESET
			std::this_thread::sleep_for(std::chrono::milliseconds(8));
			VTC_DVP_HANDLE[0x00]	= 0x00000004;	asm("NOP");	// enable generate"0x0000002F fync_in 0x00000026
	#endif

#endif
//	rst_lt9211 = true;			/////20230102   --- important reset the LT9211 when ps_app start/restart, ensure the XJ3 get video stream
//	new_mipi_stream_setup = false;
//	pl_imgproc_ctrl.CTRL_CMD.LT9211_Rst 				= 0;
//    GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");
#ifdef PHOTO_IN_DVP_OUT
    lock_dvp_start = false;
    photo_in_cnt = 0;
#endif

}
void reset_VTC_IN_ASYNC_MODE()
{
#ifdef VTC_RESET
			VTC_DISP_HANDLE[0x00]	= 0x80000000;	asm("NOP");	//		Page.23 <PG016 Video Timing Controller v6.1>   0xC0000000

	#ifdef 	VTC_MIPI_RESET
			VTC_MIPI_HANDLE[0x00]	= 0x80000000;	asm("NOP");	//      0xC0000000
	#endif

	#ifdef VTC_DVP_RESET
			VTC_DVP_HANDLE[0x00]	= 0x80000000;	asm("NOP");	//      0xC0000000
	#endif

			VTC_DISP_HANDLE[0x00]	= 0x00000004;	asm("NOP");	// enable generate"           fync_in 0x00000026

	#ifdef 	VTC_MIPI_RESET
			VTC_MIPI_HANDLE[0x00]	= 0x00000004;	asm("NOP");	// enable generate"0x0000002F fync_in 0x00000026
	#endif
	rst_lt9211 = true;			/////20230102   --- important reset the LT9211 when ps_app start/restart, ensure the XJ3 get video stream
	new_mipi_stream_setup = false;
	pl_imgproc_ctrl.CTRL_CMD.LT9211_Rst 				= 0;
	GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");

	#ifdef VTC_DVP_RESET
			std::this_thread::sleep_for(std::chrono::milliseconds(8));
			VTC_DVP_HANDLE[0x00]	= 0x00000004;	asm("NOP");	// enable generate"0x0000002F fync_in 0x00000026
	#endif
#endif

#ifdef PHOTO_IN_DVP_OUT
    lock_dvp_start = false;
    photo_in_cnt = 0;
#endif
}
void video_init()
{
//	printf("enter init\n");
	//init TV PingPong frame buf
	static int bg_color_Y = 16;
	static int bg_color_U = 128;
	static int bg_color_V = 128;

	static int bg_color_Y_ir = 16;
	static int bg_color_U_ir = 128;
	static int bg_color_V_ir = 128;

	mipi_video_ctrl.is_TV = 0;

	for(int r = 0;r<PARA_IMG_BUF_ACITVE_ROWS_TV_1620;r++)
	{
		for(int c = 0;c<PARA_IMG_BUF_STRIDE_IN_PIXEL_TV_2880;c++)
		{
			pChar_VDMA_IMG_BUF_TV_1[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + c*PARA_IMG_CHANNELS_TV+0] = bg_color_V;
			pChar_VDMA_IMG_BUF_TV_1[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + c*PARA_IMG_CHANNELS_TV+1] = bg_color_U;
			pChar_VDMA_IMG_BUF_TV_1[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + c*PARA_IMG_CHANNELS_TV+2] = bg_color_Y;

			pChar_VDMA_IMG_BUF_TV_2[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + c*PARA_IMG_CHANNELS_TV+0] = bg_color_V;
			pChar_VDMA_IMG_BUF_TV_2[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + c*PARA_IMG_CHANNELS_TV+1] = bg_color_U;
			pChar_VDMA_IMG_BUF_TV_2[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + c*PARA_IMG_CHANNELS_TV+2] = bg_color_Y;

			pChar_VDMA_IMG_BUF_TV_3[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + c*PARA_IMG_CHANNELS_TV+0] = bg_color_V;
			pChar_VDMA_IMG_BUF_TV_3[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + c*PARA_IMG_CHANNELS_TV+1] = bg_color_U;
			pChar_VDMA_IMG_BUF_TV_3[r*PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + c*PARA_IMG_CHANNELS_TV+2] = bg_color_Y;
		}
	}
	//init IR PingPong frame buf
	for(int r = 0;r<PARA_IMG_BUF_FOR_MIPI_ROWS_IR;r++)
	{
		for(int c = 0;c<PARA_IMG_BUF_STRIDE_IN_PIXEL_IR;c++)
		{
			pChar_VDMA_IMG_BUF_IR_1[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+0] = bg_color_V_ir;
			pChar_VDMA_IMG_BUF_IR_1[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+1] = bg_color_U_ir;
			pChar_VDMA_IMG_BUF_IR_1[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+2] = bg_color_Y_ir;

			pChar_VDMA_IMG_BUF_IR_2[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+0] = bg_color_V_ir;
			pChar_VDMA_IMG_BUF_IR_2[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+1] = bg_color_U_ir;
			pChar_VDMA_IMG_BUF_IR_2[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+2] = bg_color_Y_ir;

			pChar_VDMA_IMG_BUF_IR_3[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+0] = bg_color_V_ir;
			pChar_VDMA_IMG_BUF_IR_3[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+1] = bg_color_U_ir;
			pChar_VDMA_IMG_BUF_IR_3[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+2] = bg_color_Y_ir;

			pChar_VDMA_IMG_BUF_IR_4[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+0] = bg_color_V_ir;
			pChar_VDMA_IMG_BUF_IR_4[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+1] = bg_color_U_ir;
			pChar_VDMA_IMG_BUF_IR_4[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+2] = bg_color_Y_ir;

			pChar_VDMA_IMG_BUF_IR_5[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+0] = bg_color_V_ir;
			pChar_VDMA_IMG_BUF_IR_5[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+1] = bg_color_U_ir;
			pChar_VDMA_IMG_BUF_IR_5[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+2] = bg_color_Y_ir;

			pChar_VDMA_IMG_BUF_IR_6[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+0] = bg_color_V_ir;
			pChar_VDMA_IMG_BUF_IR_6[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+1] = bg_color_U_ir;
			pChar_VDMA_IMG_BUF_IR_6[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+2] = bg_color_Y_ir;

			pChar_VDMA_IMG_BUF_IR_7[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+0] = bg_color_V_ir;
			pChar_VDMA_IMG_BUF_IR_7[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+1] = bg_color_U_ir;
			pChar_VDMA_IMG_BUF_IR_7[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+2] = bg_color_Y_ir;

			pChar_VDMA_IMG_BUF_IR_8[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+0] = bg_color_V_ir;
			pChar_VDMA_IMG_BUF_IR_8[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+1] = bg_color_U_ir;
			pChar_VDMA_IMG_BUF_IR_8[r*PARA_IMG_BUF_STRIDE_IN_BYTE_IR + c*PARA_IMG_CHANNELS_IR+2] = bg_color_Y_ir;

		}
	}

#ifdef ADD_dot_line_mixer
	//extern unsigned char* ptr_MIXER_LAYER_TXT_BUF;

	for(int r = 0; r<1080; r++)
	{
		for(int c = 0; c<1920; c++)
		{
			ptr_MIXER_LAYER_TXT_BUF[r*1920*4+c*4+0] = 0;
			ptr_MIXER_LAYER_TXT_BUF[r*1920*4+c*4+1] = 0;
			ptr_MIXER_LAYER_TXT_BUF[r*1920*4+c*4+2] = 0;
			ptr_MIXER_LAYER_TXT_BUF[r*1920*4+c*4+3] = 0;//alpha 0--touming 255 butouming
		}
	}
#endif



	for(int r = 0; r<ARROW_HEIGHT; r++)
	{
		for(int c = 0; c<ARROW_WIDTH; c++)
		{
			ptr_MIXER_LAYER_NORTH_ARROW_BUF[r*ARROW_WIDTH*4+c*4+0] = 0;
			ptr_MIXER_LAYER_NORTH_ARROW_BUF[r*ARROW_WIDTH*4+c*4+1] = 0;
			ptr_MIXER_LAYER_NORTH_ARROW_BUF[r*ARROW_WIDTH*4+c*4+2] = 0;
			ptr_MIXER_LAYER_NORTH_ARROW_BUF[r*ARROW_WIDTH*4+c*4+3] = 0;
		}
	}
	memset(arrow_patch_center_mask.data,1,ARROW_WIDTH*ARROW_HEIGHT);
	int radius = 14;
	int radius_2 = radius*radius;
	for (int r = ARROW_WIDTH/2-radius; r < ARROW_WIDTH/2+radius; r++)
	{
		for (int c = ARROW_HEIGHT/2-radius; c < ARROW_HEIGHT/2+radius; c++)
		{
			float temp_x = (r - ARROW_WIDTH/2)*(r - ARROW_WIDTH/2) / radius_2;
			float temp_y = (c - ARROW_HEIGHT/2)*(c - ARROW_HEIGHT/2) / radius_2;
			if (temp_x + temp_y <= 1)
			{
				arrow_patch_center_mask.data[r*ARROW_WIDTH*1+c] = 0;
			}
		}
	}

//	printf("enter shared mem init\n");
#ifdef PHOTO_IN_DVP_OUT
#ifdef DEBUG_MODE_SEND_PHOTO
		cv::Mat send_img1 = imread("/root/save_photo.jpg",CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		cv::Mat send_img2 = imread("/root/save_photo.jpg",CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		cv::Mat send_img3 = imread("/root/save_photo.jpg",CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	memcpy( pChar_VDMA_IMG_BUF_PHOTO_1  , send_img1.data,5120*4096*3);
	memcpy( pChar_VDMA_IMG_BUF_PHOTO_2  , send_img2.data,5120*4096*3);
	memcpy( pChar_VDMA_IMG_BUF_PHOTO_3  , send_img3.data,5120*4096*3);
#else
	memset( pChar_VDMA_IMG_BUF_PHOTO_1 , 0,5120*4096*3);
	memset( pChar_VDMA_IMG_BUF_PHOTO_2 , 0,5120*4096*3);
	memset( pChar_VDMA_IMG_BUF_PHOTO_3 , 0,5120*4096*3);
	#endif

#endif
#ifdef	SDI_ADD_INFO
	for(int r = 0; r<1; r++)
	{
		for(int c = 0; c<512; c++)
		{
			ptr_MIXER_HOST_INFO_BUF_ADDR[r*512*4+c*4+0] = c%(128-48) + 48 + 1;//Y.20230713, add 1 to avoid lost
			ptr_MIXER_HOST_INFO_BUF_ADDR[r*512*4+c*4+1] = 128;//U
			ptr_MIXER_HOST_INFO_BUF_ADDR[r*512*4+c*4+2] = 128;//V
			ptr_MIXER_HOST_INFO_BUF_ADDR[r*512*4+c*4+3] = 255;////A alpha,255---totally not opaque    0---fully transparent
		}
	}
	for(int r = 1; r<64; r++)
	{
		for(int c = 0; c<512; c++)
		{
			ptr_MIXER_HOST_INFO_BUF_ADDR[r*512*4+c*4+0] = 128;//Y
			ptr_MIXER_HOST_INFO_BUF_ADDR[r*512*4+c*4+1] = 128;//U
			ptr_MIXER_HOST_INFO_BUF_ADDR[r*512*4+c*4+2] = 128;//V
			ptr_MIXER_HOST_INFO_BUF_ADDR[r*512*4+c*4+3] = 0;////A alpha,255---totally not opaque    0---fully transparent
		}
	}
#endif

#ifdef ADDING_SDI_INFO_USING_BRAM

	unsigned char * ptChar_info;
	ptChar_info = new unsigned char[256];
	ptChar_info[0] = 66;
	for(int i = 1; i<256; i++)
	{
		ptChar_info[i] = 48;
	}
	update_SDI_first_line_info(ptChar_info);
	delete[] ptChar_info;
//	printf("init sdi info bram\n");
#endif



//	photo_cmd_stat_shared = (PHOTO_CMD_STAT_SHARED *)send_pt_cmd_stat_shared;//
//
//	photo_cmd_stat_shared->cmd_photo_id = 3;
//	photo_cmd_stat_shared->cmd_send_enable = 0;
//	photo_cmd_stat_shared->cmd_init_state = 1;

//	printf("start config resize\n");
	//step3.1: init Resize_stream_TV
	IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= 1080;		asm("nop");
	IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= 1920;		asm("nop");
	IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_HEIGHT>>2] 	= 1080;		asm("nop");
	IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_WIDTH>>2] 	= 1920;		asm("nop");
	IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL] 		= 0x81;		asm("nop");
	//step3.2: init Resize_stream_IR
	IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= 512;		asm("nop");
	IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= 640;		asm("nop");
	IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_HEIGHT>>2] 	= 1024;		asm("nop");
	IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_WIDTH>>2] 	= 1280;		asm("nop");
	IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL>>2] 	= 0x81;		asm("nop");
	//step3.2: init Resize_stream_PIP
	IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] = PIP_SRC_HEIGHT_IR;		asm("nop");
	IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= PIP_SRC_WIDTH_IR;			asm("nop");
	IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_HEIGHT>>2] = disp_ctrl.PIP_HEIGHT;		asm("nop");
	IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_WIDTH>>2] 	= disp_ctrl.PIP_WIDTH;		asm("nop");
	IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL] 		= 0x81;						asm("nop");
	//step4: init VDMA
	//step4.1 inti VDMA_TV
//	printf("start config vdma\n");
//	VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2]					= 0x00000004;	asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2]					= 0x00000003;					asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] 				= VDMA_IMG_BUF_ADDR_TV_1 +  DEFAULT_OFFSET_ADDR_TV;	asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_STRIDE_IN_BYTE>>2] 			= PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880;	asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_ACITVE_WIDTH_IN_BYTE>>2]	= PARA_IMG_BUF_ACITVE_WIDTH_IN_BYTE_TV;	asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_TV;	asm("nop");
//	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_VDMACR>>2]					= 0x00000004;	asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_VDMACR>>2]					= 0x00000003;					asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 				= VDMA_IMG_BUF_ADDR_TV_1 +  DEFAULT_OFFSET_ADDR_TV;	asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 			= PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880;	asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= PARA_IMG_BUF_ACITVE_WIDTH_IN_BYTE_TV;	asm("nop");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_TV;	asm("nop");
	//step4.2 inti VDMA_IR
#ifdef	IR_CLOSE_VDMA_S2MM_20240125
	cur_ir_s2mm_stat = true;
#endif
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 					= 0x00000003;					asm("nop");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] 				= VDMA_IMG_BUF_ADDR_IR_1;		asm("nop");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_STRIDE_IN_BYTE>>2] 			= PARA_IMG_BUF_STRIDE_IN_BYTE_IR;	asm("nop");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_ACITVE_WIDTH_IN_BYTE>>2]	= PARA_IMG_BUF_ACITVE_WIDTH_IN_BYTE_IR;asm("nop");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_IR;	asm("nop");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_VDMACR>>2] 					= 0x00000003;					asm("nop");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 				= VDMA_IMG_BUF_ADDR_IR_6;		asm("nop");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 			= PARA_IMG_BUF_STRIDE_IN_BYTE_IR;		asm("nop");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= PARA_IMG_BUF_ACITVE_WIDTH_IN_BYTE_IR;	asm("nop");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_IR;	asm("nop");
	//step4.3 inti VDMA_PIP init by IR

	//vdma read channel for PIP, switch to TV or IR in running
	VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_VDMACR>>2]					= 0x00000003;					asm("nop");
	VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 			= VDMA_IMG_BUF_ADDR_IR_1 + PIP_OFFSET_ADDR_IR; 	asm("nop");
	VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= PARA_IMG_BUF_STRIDE_IN_BYTE_IR; 	asm("nop");
	VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= PIP_SRC_WIDTH_IR * PARA_IMG_CHANNELS_IR; 	asm("nop");
	VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= PIP_SRC_HEIGHT_IR; 			asm("nop");

	//step4.3 init VDMA_SDI_MIPI init by TV, vdma read channel for XJ3 MIPI 60fps 1080P, switch to TV or IR in running
	VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_VDMACR>>2]				= 0x00000003;					asm("nop");
	VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 			= VDMA_IMG_BUF_ADDR_TV_1 +  DEFAULT_OFFSET_ADDR_TV;	asm("nop");
	VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880;	asm("nop");
	VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= PARA_IMG_BUF_ACITVE_WIDTH_IN_BYTE_TV;	asm("nop");
	VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_TV;	asm("nop");

#ifdef PHOTO_IN_DVP_OUT
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2]				= 0x00000003;							asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] 			= VDMA_IMG_BUF_ADDR_PHOTO_1;			asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_S2MM_STRIDE_IN_BYTE>>2] 		= 5120*3;								asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_S2MM_ACITVE_WIDTH_IN_BYTE>>2]	= 5120*3;								asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_S2MM_ACITVE_ROWS>>2] 			= 4096;
#ifdef VTC_DVP_1080P
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_VDMACR>>2]				= 0x00000003;							asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 			= VDMA_IMG_BUF_ADDR_PHOTO_1+1600*3 + 1508*5120*3;				asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= 5120*3;								asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= 1920*3;								asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= 1080;
#else
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_VDMACR>>2]				= 0x00000003;							asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 			= VDMA_IMG_BUF_ADDR_PHOTO_1;			asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= 3840*3;								asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= 3840*3;								asm("nop");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= 5462;
#endif

#endif

//	printf("start config mixer\n");
	//step2: init Video Mxier------v_mix_PIP_TXT
	//step2.1 set top reg
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER_ENABLE>>2] 		= 0b00000000;	asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_BG_WIDTN>>2] 			= 1920;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_BG_HEIGHT>>2] 			= 1080;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_BG_COLOR_Y>>2] 			= 16;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_BG_COLOR_U>>2] 			= 128;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_BG_COLOR_V>>2] 			= 128;			asm("nop");
	//step2.2 set stream video 2
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_G_ALPHA>>2] 		= 0;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_START_COLS>>2] 	= 320;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_START_ROWS>>2] 	= 28;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_WIDTH>>2] 		= 1280;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_HEIGHT>>2] 		= 1024;			asm("nop");

	//step2.3 set stream video PIP
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_G_ALPHA>>2] 		= 255;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_START_COLS>>2] 	= disp_ctrl.PIP_START_X;	asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_START_ROWS>>2] 	= disp_ctrl.PIP_START_Y;	asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_WIDTH>>2] 		= disp_ctrl.PIP_WIDTH;		asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_HEIGHT>>2] 		= disp_ctrl.PIP_HEIGHT;		asm("nop");

	//step2.5 set MM video 5 TXT layer
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER4_G_ALPHA>>2] 		= 255;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER4_START_COLS>>2] 	= 602;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER4_START_ROWS>>2] 	= 106+16-ARROW_HEIGHT/2;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER4_WIDTH>>2] 		= ARROW_WIDTH;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER4_STRIDE>>2] 		= ARROW_WIDTH*4;		asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER4_HEIGHT>>2] 		= ARROW_HEIGHT;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER4_BUF>>2] 			= MIXER_NORTH_ARROW_BUF;asm("nop");



	//step2.5 set dot line layer
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER5_G_ALPHA>>2] 		= 255;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER5_START_COLS>>2] 	= 0;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER5_START_ROWS>>2] 	= 0;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER5_WIDTH>>2] 		= 1920;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER5_STRIDE>>2] 		= 1920*4;		asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER5_HEIGHT>>2] 		= 1080;			asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER5_BUF>>2] 			= MIXER_TXT_BUF_ADDR;asm("nop");//ptr_MIXER_HOST_INFO_BUF_ADDR


	//step2.3 set top reg
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER_ENABLE>>2] 		= 0b00011011;	asm("nop");
	VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_AP_CTRL>>2] 			= 0x81;			asm("nop");

#ifdef GAMMA_LUT
//	printf("start config gamma\n");
	//for TV channel
	for(int i = 0; i < 256; i++)
	{
		curve_Y_tv[i] = i;//(int)((float)i*0.5);
		curve_U_tv[i] = i;// (int)((float)i*0.5);
		curve_V_tv[i] = i;//(int)((float)i*0.5);
	}

	GAMMA_LUT_TV_HANDLE[GAMMA_WIDTH_IN_ACTIVE_PIXEL >> 2] 	= 1920;asm("nop");
	GAMMA_LUT_TV_HANDLE[GAMMA_HEIGHT_IN_ACTIVE_LINE >> 2] 	= 1080;asm("nop");
	GAMMA_LUT_TV_HANDLE[GAMMA_VIDEO_FORMATE >> 2] 	= 0;	asm("nop");//	#FORMAT  0:RGB,1:YUV;

	for(int i = 0; i < 256; i += 2)
	{
		GAMMA_LUT_TV_HANDLE[(GAMMA_CHANNEL_RED_START_OFFSET >> 2) + (i>>1)] 	= (curve_Y_tv[i+1] << 16) + curve_Y_tv[i];	asm("nop");//R
		GAMMA_LUT_TV_HANDLE[(GAMMA_CHANNEL_GREEN_START_OFFSET >> 2) + (i>>1)] 	= (curve_U_tv[i+1] << 16) + curve_U_tv[i];	asm("nop");//B
		GAMMA_LUT_TV_HANDLE[(GAMMA_CHANNEL_BLUE_START_OFFSET >> 2) + (i>>1)] 	= (curve_V_tv[i+1] << 16) + curve_V_tv[i];	asm("nop");//G
	}
	GAMMA_LUT_TV_HANDLE[GAMMA_CTRL >> 2] 	= 0x81;	asm("nop");//auto restart + start
	
	float f;
	for(int i = 0; i < 256; i++)
	{
		curve_Y_ir_src[i] = i;//src color map
		curve_U_ir_src[i] = i;//
		curve_V_ir_src[i] = i;//
#ifdef BLACK_WHITE_SWITCH_IN_AVT
		curve_Y_ir_cvt[i] = 255 - i;//black white switch color map
#endif		
		f = (i+0.5F)/256;
		f = (float)pow(f, 1/1.8);
		curve_Y_ir_enh[i] = (uint8_t) (f*256 -0.5F);
	}
	GAMMA_LUT_IR_HANDLE[GAMMA_WIDTH_IN_ACTIVE_PIXEL >> 2] 	= 1280; wait_moment(1);
	GAMMA_LUT_IR_HANDLE[GAMMA_HEIGHT_IN_ACTIVE_LINE >> 2] 	= 1024; wait_moment(1);
	GAMMA_LUT_IR_HANDLE[GAMMA_VIDEO_FORMATE >> 2] 	= 0;	wait_moment(1);;//	#FORMAT  0:RGB,1:YUV;

	for(int i = 0; i < 256; i += 2)
	{
		GAMMA_LUT_IR_HANDLE[(GAMMA_CHANNEL_RED_START_OFFSET		>> 2) + (i>>1)] = (curve_Y_ir_src[i+1] << 16) + curve_Y_ir_src[i];	wait_moment(1);//R
		GAMMA_LUT_IR_HANDLE[(GAMMA_CHANNEL_GREEN_START_OFFSET	>> 2) + (i>>1)] = (curve_U_ir_src[i+1] << 16) + curve_U_ir_src[i];	wait_moment(1);//B
		GAMMA_LUT_IR_HANDLE[(GAMMA_CHANNEL_BLUE_START_OFFSET	>> 2) + (i>>1)] = (curve_V_ir_src[i+1] << 16) + curve_V_ir_src[i];	wait_moment(1);//G
	}
	GAMMA_LUT_IR_HANDLE[GAMMA_CTRL >> 2] 	= 0x81;	wait_moment(1);//auto restart + start
	//for IR channel
#ifdef BLACK_WHITE_SWITCH_IN_AVT
	ir_black_white_switch_happened = false;
//	ir_black_white = IR_BLACK_HOT; //IR_WHITE_HOT
#endif

#ifdef ENABLE_GAMMA_LUT_PIP
	GAMMA_LUT_PIP_HANDLE[GAMMA_WIDTH_IN_ACTIVE_PIXEL >> 2] 	= pip_width_TV;asm("nop"); //
	GAMMA_LUT_PIP_HANDLE[GAMMA_HEIGHT_IN_ACTIVE_LINE >> 2] 	= pip_height_TV;asm("nop"); //PIP_SRC_HEIGHT_TV
	GAMMA_LUT_PIP_HANDLE[GAMMA_VIDEO_FORMATE >> 2] 	= 0;	asm("nop");//	#FORMAT  0:RGB,1:YUV;

	for(int i = 0; i < 256; i += 2)
	{
		GAMMA_LUT_PIP_HANDLE[(GAMMA_CHANNEL_RED_START_OFFSET >> 2) + (i>>1)] 	= (curve_Y_tv[i+1] << 16) + curve_Y_tv[i];	asm("nop");//R
		GAMMA_LUT_PIP_HANDLE[(GAMMA_CHANNEL_GREEN_START_OFFSET >> 2) + (i>>1)] 	= (curve_U_tv[i+1] << 16) + curve_U_tv[i];	asm("nop");//B
		GAMMA_LUT_PIP_HANDLE[(GAMMA_CHANNEL_BLUE_START_OFFSET >> 2) + (i>>1)] 	= (curve_V_tv[i+1] << 16) + curve_V_tv[i];	asm("nop");//G
	}
	GAMMA_LUT_PIP_HANDLE[GAMMA_CTRL >> 2] 	= 0x81;	asm("nop");//auto restart + start
#endif
#ifdef only_median
	MEDIAN_MIPI_HANDLE[0x10>>2] = 0x01;//enable
	MEDIAN_MIPI_HANDLE[0x18>>2] = 1920;//width
	MEDIAN_MIPI_HANDLE[0x20>>2] = 1080;//height
	MEDIAN_MIPI_HANDLE[0x00>>2] = 0x81;//autorun
#endif

#ifdef only_denoise
	DENOISE_MIPI_HANDLE[0x10>>2] = 0x01;//enable
	DENOISE_MIPI_HANDLE[0x18>>2] = 1920;//width
	DENOISE_MIPI_HANDLE[0x20>>2] = 1080;//height
	DENOISE_MIPI_HANDLE[0x28>>2] = 200; //sigma
	DENOISE_MIPI_HANDLE[0x00>>2] = 0x81;//autorun
#endif

#ifdef median_denoise
	MEDIAN_MIPI_HANDLE[0x10>>2] = 0x01;asm("nop");//enable
	MEDIAN_MIPI_HANDLE[0x18>>2] = 1920;asm("nop");//width
	MEDIAN_MIPI_HANDLE[0x20>>2] = 1080;asm("nop");//height
	MEDIAN_MIPI_HANDLE[0x00>>2] = 0x81;asm("nop");//autorun

	DENOISE_MIPI_HANDLE[0x10>>2] = 0x01;asm("nop");//enable
	DENOISE_MIPI_HANDLE[0x18>>2] = 1920;asm("nop");//width
	DENOISE_MIPI_HANDLE[0x20>>2] = 1080;asm("nop");//height
	DENOISE_MIPI_HANDLE[0x28>>2] = 120;asm("nop");//sigma TV-120 IR-50
	DENOISE_MIPI_HANDLE[0x00>>2] = 0x81;asm("nop");//autorun
#endif

#endif
//	printf("cross config. \n");
	cross_Main.cross_info.info.crossEn 	 						= true;
	cross_Main.cross_info.info.cross_hor 						= 960;
	cross_Main.cross_info.info.cross_ver 						= 540;
	cross_Main.cross_info.info.reserved  						= 0;
	cross_PIP.cross_info.info.crossEn 							= false;
	cross_PIP.cross_info.info.cross_hor 						= 272;
	cross_PIP.cross_info.info.cross_ver 						= 212;
	cross_PIP.cross_info.info.reserved  						= 0;

	GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_MAIN>>2] 	= cross_Main.cross_info.send;asm("nop");
	GPIO_CROSS_CTRL_HANDLE[GPIO_CROSS_PIP>>2] 	= cross_PIP.cross_info.send;asm("nop");

	printf("IP core config done. \n");
}
void wait_moment(int nop_nums)
{
	for(int i = 0; i<nop_nums;i++)		//important when switch video clk src must wait a moment to reset VTC
	{
		asm("nop");
	}
}

void video_in_TV() 	//irq0_drv  ----VIS_CL_VS_IN			----SIGIO     ---- POLL_IN   ---1
{
	if(disp_ctrl.IR_ALIGN_KEEP==false)//2023.09.06
	{
		FrmWriteID_TV = (FrmWriteID_TV + 1) % 3;
	}
	else
	{
		FrmWriteID_TV = (FrmWriteID_TV + 1) % 2;
	}

	// FrmWriteID_TV = 0;//no pingpong

	// img buf for Display TV channel
	switch(FrmWriteID_TV)
	{
	case 0:
		VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_TV_1 +  DEFAULT_OFFSET_ADDR_TV;////no pingpong
#ifdef TV_ALIGN_ON_FMQL
		video_info_TV_IN.img_buf = pChar_VDMA_IMG_BUF_TV_3;
#endif
		break;
	case 1:
		VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_TV_2 +  DEFAULT_OFFSET_ADDR_TV;
#ifdef TV_ALIGN_ON_FMQL
		video_info_TV_IN.img_buf = pChar_VDMA_IMG_BUF_TV_1;
#endif
		break;
	case 2:
		VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_TV_3 +  DEFAULT_OFFSET_ADDR_TV;
#ifdef TV_ALIGN_ON_FMQL
		video_info_TV_IN.img_buf = pChar_VDMA_IMG_BUF_TV_2;
#endif
		break;
	default:
		break;
	}
	asm("NOP");
	VDMA_TV_HANDLE[VDMA_REG_OFFSET_S2MM_ACITVE_ROWS>>2] = PARA_IMG_BUF_ACITVE_ROWS_TV;		asm("NOP");

//	video_info_TV_IN.ready = true;
#ifdef DEBUG_MIPI_IR_DELAY

#endif

#ifdef 	TEST_TV_IN_TIME_STABLIZATION
    	omp_time_tv_in_cur = omp_get_wtime();
		tv_in_time_gap = 1000.0f*(omp_time_tv_in_cur-omp_time_tv_in_pre);
		omp_time_tv_in_pre = omp_time_tv_in_cur;
		enable_print_time_gap_tv_in = true;
#endif

	tv_in_cnt++;
	video_info_TV_IN.ready = true;
#ifdef TV_ALIGN_ON_FMQL
	video_info_TV_IN.ready_TV_ALIGN = true;
#endif

#ifdef MEASURE_LOOP_TIME
	time_TV_IN =  std::chrono::system_clock::now();
#endif
}

void video_in_IR()  	//irq2_drv  ----NIR_CL_VS_IN			----SIGIO     ---- POLL_MSG   ---3
{


	if(disp_ctrl.IR_ALIGN_KEEP==false)
	{
		FrmWriteID_IR = (FrmWriteID_IR+1) % 8;
	}
	else
	{
		FrmWriteID_IR = (FrmWriteID_IR+1) % 7;
	}

	// FrmWriteID_IR = 0;

	// img buf for Display IR channel
	switch(FrmWriteID_IR)
	{
		case 0:
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_1 ;
			//video_info_IR_IN.img_buf = pChar_VDMA_IMG_BUF_IR_4;
			break;
		case 1:
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_2 ;
			//video_info_IR_IN.img_buf = pChar_VDMA_IMG_BUF_IR_1;
			break;
		case 2:
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_3 ;
			//video_info_IR_IN.img_buf = pChar_VDMA_IMG_BUF_IR_2;
			break;
		case 3:
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_4 ;
			//video_info_IR_IN.img_buf = pChar_VDMA_IMG_BUF_IR_3;
			break;
		case 4:
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_5 ;
			//video_info_IR_IN.img_buf = pChar_VDMA_IMG_BUF_IR_3;
			break;
		case 5:
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_6 ;
			//video_info_IR_IN.img_buf = pChar_VDMA_IMG_BUF_IR_3;
			break;
		case 6:
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_7 ;
			//video_info_IR_IN.img_buf = pChar_VDMA_IMG_BUF_IR_3;
			break;
		case 7:
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_8 ;
			//video_info_IR_IN.img_buf = pChar_VDMA_IMG_BUF_IR_3;
			break;
		default:
			break;
	}
	asm("NOP");
	VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_ACITVE_ROWS>>2] = PARA_IMG_BUF_ACITVE_ROWS_IR;		asm("NOP");

//	video_info_IR_IN.ready = true;
#ifdef DEBUG_MIPI_IR_DELAY
	ir_in_latest_omp_time = omp_get_wtime();
	cur_write_id_ir = FrmWriteID_IR;
#endif
#ifdef 	TEST_TV_IN_TIME_STABLIZATION
	omp_time_ir_in_cur = omp_get_wtime();
	ir_in_time_gap = 1000.0f*(omp_time_ir_in_cur-omp_time_ir_in_pre);
	omp_time_ir_in_pre = omp_time_ir_in_cur;
	enable_print_time_gap_ir_in = true;
#endif
	ir_in_cnt++;
	video_info_IR_IN.ready = true;
}

void video_out_DISP() 	 //irq3_drv  ----v_tc_1_fsync_out		----SIGUSR1   ---- POLL_IN   ---1
{
//	video_info_TV_IN.ready = true;
	video_info_DISP_OUT.ready = true;
	if(disp_ctrl.SWITCH_HAPPENED == true)
	{
		switch(disp_ctrl.SCALING_RATE_TV)
		{
			case 1:		src_height_TV = 1080;	src_width_TV = 1920;	CURRENT_OFFSET_ADDR_TV 	= DEFAULT_OFFSET_ADDR_TV;	pip_height_TV = 360;	pip_width_TV = 640;	PIP_OFFSET_ADDR_TV = 5446560; break;
			case 2:		src_height_TV = 540;	src_width_TV = 960;		CURRENT_OFFSET_ADDR_TV 	= DEFAULT_OFFSET_ADDR_TV + 2334240;	 pip_height_TV = 180;	pip_width_TV = 320;	PIP_OFFSET_ADDR_TV = 6224640; break;	//2334240= (540 - 540/2)*8640 + (960 - 960/2)*3
			case 4:		src_height_TV = 270;	src_width_TV = 480;		CURRENT_OFFSET_ADDR_TV 	= DEFAULT_OFFSET_ADDR_TV + 3501360;	 pip_height_TV = 360;	pip_width_TV = 640;	PIP_OFFSET_ADDR_TV = 5446560; break;	//3501360= (540 - 540/4)*8640 + (960 - 960/4)*3
			default:		break;
		}
		switch(disp_ctrl.SCALING_RATE_IR)
		{
			case 1:		src_height_IR = 512;	src_width_IR = 640;		CURRENT_OFFSET_ADDR_IR 	= 0;								break;
			case 2:		src_height_IR = 256;	src_width_IR = 320;		CURRENT_OFFSET_ADDR_IR 	= 737760;							break;	//737760	= (256 - 256/2)*5760 + (320 - 320/2)*3
			case 4:		src_height_IR = 128;	src_width_IR = 160;		CURRENT_OFFSET_ADDR_IR 	= 1106640;							break;	//1106640	= (256 - 256/4)*5760 + (320 - 320/4)*3
			default:		break;
		}
	}

	if(disp_ctrl.SWITCH_HAPPENED == true)
	{
		switch(disp_ctrl.DISP_MODE)
		{
			case TV_MAIN_MODE:
						{
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= src_height_TV;	asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= src_width_TV;		asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_HEIGHT>>2] 	= DISP_HEIGHT_TV;	asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_WIDTH>>2] 	= DISP_WIDTH_TV;	asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL>>2] 	= 0x81;				asm("nop");
							switch(FrmWriteID_TV)
							{
								// case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + CURRENT_OFFSET_ADDR_TV;	break;//no pingpong
								case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3 + CURRENT_OFFSET_ADDR_TV;	break;
								case 1:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + CURRENT_OFFSET_ADDR_TV;	break;
								case 2:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2 + CURRENT_OFFSET_ADDR_TV;	break;
								default:	break;
							}
							asm("NOP");
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= src_width_TV * PARA_IMG_CHANNELS_TV;			asm("nop");
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= src_height_TV;	asm("NOP");

							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_G_ALPHA>>2] 			= 0;			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER_ENABLE>>2] 			= 0b00011011;	asm("nop");// Top reg: 		LayerEnable 0b0000_0000
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_AP_CTRL>>2] 				= 0x81;			asm("nop");
							break;
						}
			case IR_MAIN_MODE:
						{
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= src_height_IR;	asm("nop");
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= src_width_IR;		asm("nop");
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL>>2] 	= 0x81;				asm("nop");
							switch(FrmWriteID_IR)
							{
								// case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_1 + CURRENT_OFFSET_ADDR_IR;	break;//no pingpong
								case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_6 + CURRENT_OFFSET_ADDR_IR;	break;
								case 1:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_7 + CURRENT_OFFSET_ADDR_IR;	break;
								case 2:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_8 + CURRENT_OFFSET_ADDR_IR;	break;
								case 3:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_1 + CURRENT_OFFSET_ADDR_IR;	break;
								case 4:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_2 + CURRENT_OFFSET_ADDR_IR;	break;
								case 5:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_3 + CURRENT_OFFSET_ADDR_IR;	break;
								case 6:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_4 + CURRENT_OFFSET_ADDR_IR;	break;
								case 7:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_5 + CURRENT_OFFSET_ADDR_IR;	break;
								default:	break;
							}
							asm("NOP");
							VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= src_width_IR*PARA_IMG_CHANNELS_IR;						asm("nop");
							VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= src_height_IR;		asm("NOP");

							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_G_ALPHA>>2] 		= 255;			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER_ENABLE>>2] 		= 0b00011010;	asm("nop");// Top reg: 		LayerEnable 0b0000_0000
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_AP_CTRL>>2] 			= 0x81;			asm("nop");
							break;
						}
			case TV_PIP_MODE:									//TV Main & IR PIP
						{
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= src_height_TV;	asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= src_width_TV;		asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_HEIGHT>>2] 	= DISP_HEIGHT_TV;	asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_WIDTH>>2] 	= DISP_WIDTH_TV;	asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL>>2] 	= 0x81;				asm("nop");

							IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] = PIP_SRC_HEIGHT_IR;	asm("nop");
							IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= PIP_SRC_WIDTH_IR;		asm("nop");
							IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL] 		= 0x81;					asm("nop");
							switch(FrmWriteID_TV)				//pingpong for Main channel
							{
								// case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 	= VDMA_IMG_BUF_ADDR_TV_1 + CURRENT_OFFSET_ADDR_TV;	break;//no pingpong
								case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 	= VDMA_IMG_BUF_ADDR_TV_3 + CURRENT_OFFSET_ADDR_TV;  break;
								case 1:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 	= VDMA_IMG_BUF_ADDR_TV_1 + CURRENT_OFFSET_ADDR_TV;	break;
								case 2:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 	= VDMA_IMG_BUF_ADDR_TV_2 + CURRENT_OFFSET_ADDR_TV;	break;
								default:	break;
							}
							asm("NOP");
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]= src_width_TV * PARA_IMG_CHANNELS_TV;			asm("nop");
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 		= src_height_TV;								asm("NOP");
							switch(FrmWriteID_IR)				//pingpong for PIP channel
							{
								// case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_1 + PIP_OFFSET_ADDR_IR;	break;//no pingpong
								case 0:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_6 + PIP_OFFSET_ADDR_IR;	break;
								case 1:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_7 + PIP_OFFSET_ADDR_IR;	break;
								case 2:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_8 + PIP_OFFSET_ADDR_IR;	break;
								case 3:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_1 + PIP_OFFSET_ADDR_IR;	break;
								case 4:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_2 + PIP_OFFSET_ADDR_IR;	break;
								case 5:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_3 + PIP_OFFSET_ADDR_IR;	break;
								case 6:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_4 + PIP_OFFSET_ADDR_IR;	break;
								case 7:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_5 + PIP_OFFSET_ADDR_IR;	break;
								default:	break;
							}
							asm("NOP");
							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 			= PARA_IMG_BUF_STRIDE_IN_BYTE_IR; 			asm("nop");
							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]		= PIP_SRC_WIDTH_IR * PARA_IMG_CHANNELS_IR; 	asm("nop");
							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 				= PIP_SRC_HEIGHT_IR; 						asm("nop");

							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_START_COLS>>2] 	= disp_ctrl.PIP_START_X; 		asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_START_ROWS>>2] 	= disp_ctrl.PIP_START_Y; 		asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_WIDTH>>2] 		= disp_ctrl.PIP_WIDTH;			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_HEIGHT>>2] 		= disp_ctrl.PIP_HEIGHT;			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_G_ALPHA>>2] 		= 255;			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_G_ALPHA>>2] 		= 0;			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER_ENABLE>>2] 		= 0b00011111;	asm("nop");// Top reg: 		LayerEnable 0b0000_0000
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_AP_CTRL>>2] 			= 0x81;			asm("nop");
							break;
						}
			case IR_PIP_MODE:									//IR Main & TV PIP
						{
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= src_height_IR;	asm("nop");
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= src_width_IR;		asm("nop");
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL>>2] 	= 0x81;				asm("nop");

							IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] = pip_height_TV;		asm("nop");
							IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= pip_width_TV;			asm("nop");
							IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL] 		= 0x81;					asm("nop");

							switch(FrmWriteID_IR)				//pingpong for Main channel
							{
								// case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_1 + CURRENT_OFFSET_ADDR_IR;	break;//no pingpong
								case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_6 + CURRENT_OFFSET_ADDR_IR;	break;
								case 1:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_7 + CURRENT_OFFSET_ADDR_IR;	break;
								case 2:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_8 + CURRENT_OFFSET_ADDR_IR;	break;
								case 3:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_1 + CURRENT_OFFSET_ADDR_IR;	break;
								case 4:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_2 + CURRENT_OFFSET_ADDR_IR;	break;
								case 5:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_3 + CURRENT_OFFSET_ADDR_IR;	break;
								case 6:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_4 + CURRENT_OFFSET_ADDR_IR;	break;
								case 7:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_5 + CURRENT_OFFSET_ADDR_IR;	break;
								default:	break;
							}
							asm("NOP");
							VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = src_height_IR; 					asm("nop");
							switch(FrmWriteID_TV)				//pingpong for PIP channel
							{
								// case 0:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + PIP_OFFSET_ADDR_TV;		break;//no pingpong
								case 0:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3 + PIP_OFFSET_ADDR_TV;		break;
								case 1:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + PIP_OFFSET_ADDR_TV;		break;
								case 2:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2 + PIP_OFFSET_ADDR_TV ;		break;
								default:	break;
							}
							asm("NOP");
							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 			= PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880; asm("nop");
							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]		= pip_width_TV * PARA_IMG_CHANNELS_TV; asm("nop");
							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 				= pip_height_TV; 		asm("nop");

							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_START_COLS>>2] 	= disp_ctrl.PIP_START_X; 			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_START_ROWS>>2] 	= disp_ctrl.PIP_START_Y; 			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_WIDTH>>2] 		= disp_ctrl.PIP_WIDTH;				asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_HEIGHT>>2] 		= disp_ctrl.PIP_HEIGHT;				asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_G_ALPHA>>2] 		= 255;								asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER_ENABLE>>2] 		= 0b00011110;						asm("nop");// Top reg: LayerEnable 0b0000_0000
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_AP_CTRL>>2] 			= 0x81;								asm("nop");
							break;
						}
			case FUSION_MODE:
						{

							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER_ENABLE>>2] 		= 0b00000000;						asm("nop");// Top reg: LayerEnable 0b0000_0000
							switch(FrmWriteID_IR)
							{
								// case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1   + 0;		break;//no pingpong
								case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6   + 0;		break;
								case 1:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7   + 0;		break;
								case 2:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8   + 0;		break;
								case 3:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1   + 0;		break;
								case 4:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2   + 0;		break;
								case 5:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3   + 0;		break;
								case 6:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4   + 0;		break;
								case 7:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5   + 0;		break;
								default:	break;
							}
							asm("NOP");
							VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= 1920;						asm("nop");
							VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= 512;		                asm("NOP");
//#define DEBUG_FUSION_20221231
							switch(FrmWriteID_TV)
							{
								// case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1;	break;//no pingpong
#ifdef DEBUG_FUSION_20221231
								case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3+DEFAULT_OFFSET_ADDR_TV;	break;
								case 1:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1+DEFAULT_OFFSET_ADDR_TV;	break;
								case 2:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2+DEFAULT_OFFSET_ADDR_TV;	break;
								default:	break;
#endif
#ifndef DEBUG_FUSION_20221231
								case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3  + DEFAULT_OFFSET_ADDR_TV;	break;
								case 1:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1  + DEFAULT_OFFSET_ADDR_TV;	break;
								case 2:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2  + DEFAULT_OFFSET_ADDR_TV;	break;
								default:	break;
#endif
							}
							asm("NOP");
#ifndef DEBUG_FUSION_20221231
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]		= PARA_IMG_BUF_ACITVE_WIDTH_IN_BYTE_TV;		asm("nop");//PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880;		asm("nop");
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 				= 1080;		asm("nop");//PARA_IMG_BUF_ACITVE_ROWS_TV_1620;			asm("nop");

							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= 1080;						asm("nop");//1620;						asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= 1920;						asm("nop");//2880;						asm("nop");
#endif
#ifdef DEBUG_FUSION_20221231
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= 2280*3;		asm("nop");		//debug fusion
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= 1620;			asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= 1620;						asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= 2280;						asm("nop");
#endif
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_HEIGHT>>2] 	= 1080;						asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_WIDTH>>2] 	= 1920;						asm("nop");
							IMG_RESIZE_STREAM_TV_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL>>2] 	= 0x81;						asm("nop");

							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= 512;						asm("nop");
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= 640;						asm("nop");
//							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_HEIGHT>>2] 	= 1024;						asm("nop");
//							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_DST_WIDTH>>2] 	= 1280;						asm("nop");
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL>>2] 	= 0x81;						asm("nop");



							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_G_ALPHA>>2] 			= 128;								asm("nop");//set IR channel(layer) alpha
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER_ENABLE>>2] 			= 0b00011011;						asm("nop");// Top reg: LayerEnable 0b0000_0000
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_AP_CTRL>>2]					= 0x81;								asm("nop");
							break;
						}
			case IR_ALIGN_MODE:
						{
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] 	= 512;	asm("nop");
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= 640;		asm("nop");
							IMG_RESIZE_STREAM_IR_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL>>2] 	= 0x81;				asm("nop");

							IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_HEIGHT>>2] = pip_height_TV;		asm("nop");
							IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_SRC_WIDTH>>2] 	= pip_width_TV;			asm("nop");
							IMG_RESIZE_STREAM_PIP_HANDLE[IMAGE_RESIZE_REG_OFFSET_AP_CTRL] 		= 0x81;						asm("nop");

						    VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_IR_8 + 0;	asm("NOP");
							VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = 512; 						asm("nop");

							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]				= VDMA_IMG_BUF_ADDR_TV_3 + PIP_OFFSET_ADDR_TV;	asm("NOP");
							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880; asm("nop");
							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_WIDTH_IN_BYTE>>2]	= pip_width_TV * PARA_IMG_CHANNELS_TV; asm("nop");
							VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= pip_height_TV; 		asm("nop");


							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_START_COLS>>2] 	= disp_ctrl.PIP_START_X; 			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_START_ROWS>>2] 	= disp_ctrl.PIP_START_Y; 			asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_WIDTH>>2] 		= disp_ctrl.PIP_WIDTH;				asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER3_HEIGHT>>2] 		= disp_ctrl.PIP_HEIGHT;				asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER2_G_ALPHA>>2] 		= 255;								asm("nop");
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_LAYER_ENABLE>>2] 		= 0b00011110;						asm("nop");// Top reg: LayerEnable 0b0000_0000
							VIDEO_MIXER_HANDLE[MIXER_REG_OFFSET_AP_CTRL>>2] 			= 0x81;								asm("nop");
							break;
						}
			default:
				break;
		}
		disp_ctrl.SWITCH_HAPPENED = false;
	}
	else
	{
		switch(disp_ctrl.DISP_MODE)
		{
			case TV_MAIN_MODE:
							{
								switch(FrmWriteID_TV)
								{
									// case 0:	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + CURRENT_OFFSET_ADDR_TV;		break;//no pingpong
									case 0:	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3 + CURRENT_OFFSET_ADDR_TV;		break;
									case 1:	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + CURRENT_OFFSET_ADDR_TV;		break;
									case 2:	VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2 + CURRENT_OFFSET_ADDR_TV ;		break;
									default:	break;
								}
								asm("NOP");
								VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = src_height_TV;	asm("NOP");
								break;
							}
			case IR_MAIN_MODE:
							{
								switch(FrmWriteID_IR)
								{
									// case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1 + CURRENT_OFFSET_ADDR_IR;	break;//no pingpong
									case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6 + CURRENT_OFFSET_ADDR_IR;	break;
									case 1:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7 + CURRENT_OFFSET_ADDR_IR;	break;
									case 2: 	VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8 + CURRENT_OFFSET_ADDR_IR;	break;
									case 3:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1 + CURRENT_OFFSET_ADDR_IR;	break;
									case 4:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2 + CURRENT_OFFSET_ADDR_IR;	break;
									case 5:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3 + CURRENT_OFFSET_ADDR_IR;	break;
									case 6:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4 + CURRENT_OFFSET_ADDR_IR;	break;
									case 7:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5 + CURRENT_OFFSET_ADDR_IR;	break;
									default:	break;
								}
								asm("NOP");
								VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = src_height_IR;		asm("NOP");
								break;
							}
			case TV_PIP_MODE:												//TV Main & IR PIP
							{
								switch(FrmWriteID_TV)						//pingpong for Main channel
								{
									// case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + CURRENT_OFFSET_ADDR_TV;	break;//no pingpong
									case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3 + CURRENT_OFFSET_ADDR_TV;	break;
									case 1:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + CURRENT_OFFSET_ADDR_TV;	break;
									case 2:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2 + CURRENT_OFFSET_ADDR_TV ;	break;
									default:	break;
								}
								asm("NOP");
								VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = src_height_TV;	asm("NOP");

								switch(FrmWriteID_IR)						//pingpong for PIP channel
								{
									// case 0:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1 + PIP_OFFSET_ADDR_IR;		break;//no pingpong
									case 0:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6 + PIP_OFFSET_ADDR_IR;		break;
									case 1:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7 + PIP_OFFSET_ADDR_IR;		break;
									case 2: 	VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8 + PIP_OFFSET_ADDR_IR;		break;
									case 3:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1 + PIP_OFFSET_ADDR_IR;		break;
									case 4:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2 + PIP_OFFSET_ADDR_IR;		break;
									case 5:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3 + PIP_OFFSET_ADDR_IR;		break;
									case 6:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4 + PIP_OFFSET_ADDR_IR;		break;
									case 7:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5 + PIP_OFFSET_ADDR_IR;		break;
									default:	break;
								}
								asm("NOP");
								VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = PIP_SRC_HEIGHT_IR; 						asm("nop");
								break;
							}
			case IR_PIP_MODE:												//IR Main & TV PIP
							{
								switch(FrmWriteID_IR)						//pingpong for Main channel
								{
									// case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1 + CURRENT_OFFSET_ADDR_IR;	break;//no pingpong
									case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6 + CURRENT_OFFSET_ADDR_IR;	break;
									case 1:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7 + CURRENT_OFFSET_ADDR_IR;	break;
									case 2:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8 + CURRENT_OFFSET_ADDR_IR;	break;
									case 3:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1 + CURRENT_OFFSET_ADDR_IR;	break;
									case 4:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2 + CURRENT_OFFSET_ADDR_IR;	break;
									case 5:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3 + CURRENT_OFFSET_ADDR_IR;	break;
									case 6:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4 + CURRENT_OFFSET_ADDR_IR;	break;
									case 7:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5 + CURRENT_OFFSET_ADDR_IR;	break;
									default:	break;
								}
								asm("NOP");
								VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = src_height_IR; 						asm("nop");

								switch(FrmWriteID_TV)						//pingpong for PIP channel
								{
									// case 0:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + PIP_OFFSET_ADDR_TV;	break;//no pingpong
									case 0:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3 + PIP_OFFSET_ADDR_TV;	break;
									case 1:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + PIP_OFFSET_ADDR_TV;	break;
									case 2:		VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2 + PIP_OFFSET_ADDR_TV ;	break;
									default:	break;
								}
								asm("NOP");
								VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = pip_height_TV; 		asm("nop");
								break;
							}
			case FUSION_MODE:
						{
							switch(FrmWriteID_TV)
							{
#ifdef DEBUG_FUSION_20221231
								case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3+DEFAULT_OFFSET_ADDR_TV;	break;
								case 1:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1+DEFAULT_OFFSET_ADDR_TV;	break;
								case 2:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2+DEFAULT_OFFSET_ADDR_TV;	break;
								default:	break;
#endif
#ifndef DEBUG_FUSION_20221231
								case 0:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3  + DEFAULT_OFFSET_ADDR_TV;	break;
								case 1:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1  + DEFAULT_OFFSET_ADDR_TV;	break;
								case 2:		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2  + DEFAULT_OFFSET_ADDR_TV;	break;
								default:	break;
#endif
							}
							asm("NOP");

#ifndef DEBUG_FUSION_20221231
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= 1080;		asm("nop");		//PARA_IMG_BUF_ACITVE_ROWS_TV_1620;		asm("nop");
#endif
#ifdef DEBUG_FUSION_20221231
							VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= 1620;			asm("nop");
#endif

							switch(FrmWriteID_IR)
							{
								// case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1  + 0;		break;//no pingpong
								case 0:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6  + 0;		break;
								case 1:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7  + 0;		break;
								case 2:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8  + 0;		break;
								case 3:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1  + 0;		break;
								case 4:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2  + 0;		break;
								case 5:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3  + 0;		break;
								case 6:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4  + 0;		break;
								case 7:		VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5  + 0;		break;
								default:	break;
							}
							asm("NOP");
							VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = 512; 												asm("nop");
							break;
						}
			case IR_ALIGN_MODE:
							{
								VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8 + 0;	asm("NOP");
								VDMA_IR_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = 512; 									asm("nop");

								VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3 + PIP_OFFSET_ADDR_TV;		asm("NOP");
								VDMA_PIP_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = pip_height_TV; 		asm("nop");
								break;
							}
			default:
				break;
		}
	}

#ifdef BLACK_WHITE_SWITCH_IN_AVT
	if(ir_black_white_switch_happened == true)
	{
		if(ir_black_white == IR_BLACK_HOT) //IR_WHITE_HOT
		{
			for(int i = 0; i < 256; i += 2)
			{
				GAMMA_LUT_IR_HANDLE[(GAMMA_CHANNEL_RED_START_OFFSET	>> 2) + (i>>1)] = (curve_Y_ir_src[i+1] << 16) + curve_Y_ir_src[i];	asm("NOP");//R
			}
			GAMMA_LUT_IR_HANDLE[GAMMA_CTRL >> 2] 	= 0x81;	asm("NOP");		//auto restart + start
		}
		else
		{
			for(int i = 0; i < 256; i += 2)
			{
				GAMMA_LUT_IR_HANDLE[(GAMMA_CHANNEL_RED_START_OFFSET	>> 2) + (i>>1)] = (curve_Y_ir_cvt[i+1] << 16) + curve_Y_ir_cvt[i];	asm("NOP");//R
			}
			GAMMA_LUT_IR_HANDLE[GAMMA_CTRL >> 2] 	= 0x81;	asm("NOP");		//auto restart + start
		}
		ir_black_white_switch_happened = false;
	}
#endif

	if(ir_enhance_switch_happened==true)
	{
		if(ir_enhance_on)
		{
			for(int i = 0; i < 256; i += 2)
			{
				GAMMA_LUT_IR_HANDLE[(GAMMA_CHANNEL_RED_START_OFFSET	>> 2) + (i>>1)] = (curve_Y_ir_enh[i+1] << 16) + curve_Y_ir_enh[i];	asm("NOP");//R
			}
			GAMMA_LUT_IR_HANDLE[GAMMA_CTRL >> 2] 	= 0x81;	asm("NOP");//auto restart + start
		}
		else
		{
			for(int i = 0; i < 256; i += 2)
			{
				GAMMA_LUT_IR_HANDLE[(GAMMA_CHANNEL_RED_START_OFFSET	>> 2) + (i>>1)] = (curve_Y_ir_src[i+1] << 16) + curve_Y_ir_src[i];	asm("NOP");//R
			}
			GAMMA_LUT_IR_HANDLE[GAMMA_CTRL >> 2] 	= 0x81;	asm("NOP");//auto restart + start
		}
		ir_enhance_switch_happened = false;
	}


#ifdef MEASURE_LOOP_TIME
//	if(bias_ctrl_left_disp)
//	{
//		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 			= VDMA_IMG_BUF_ADDR_TV_1 +  270 * PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 500 * PARA_IMG_CHANNELS_TV;	asm("nop");
//		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880;	asm("nop");
//		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 		= PARA_IMG_BUF_ACITVE_ROWS_TV;	asm("nop");
//		bias_ctrl_left_disp = false;
//	}
//
//	if(bias_ctrl_rst_disp)
//	{
//		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] 			= VDMA_IMG_BUF_ADDR_TV_1 +  270 * PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880 + 480 * PARA_IMG_CHANNELS_TV;	asm("nop");
//		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880;	asm("nop");
//		VDMA_TV_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 		= PARA_IMG_BUF_ACITVE_ROWS_TV;	asm("nop");
//		bias_ctrl_rst_disp = false;
//	}
	time_DISP_OUT  =  std::chrono::system_clock::now();
#endif

#ifdef DEBUG_MIPI_IR_DELAY
	disp_out_cnt++;
	omp_time_disp_out_cur = omp_get_wtime();
	disp_out_time_gap = 1000.0f*(omp_time_disp_out_cur-omp_time_disp_out_pre);
	omp_time_disp_out_pre = omp_time_disp_out_cur;
#endif

#ifdef IR_CLOSE_VDMA_S2MM_20240125
	if(ir_power_on_stat==true)
	{
		if(ir_power_on_cnt>60 && cur_ir_s2mm_stat==false)
		{
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 	= 0x00000003;	asm("nop");
			VDMA_IR_HANDLE[VDMA_REG_OFFSET_S2MM_VDMACR>>2] 	= 0x00000003;	asm("nop");
			cur_ir_s2mm_stat = true;
//			printf("enable ir S2MM\n");
		}
		else
		{
			ir_power_on_cnt++;
		}
	}
#endif

}

void video_out_SDI_MIPI()	//irq1_drv  ----v_tc_0_fsync_out		----SIGIO     ---- POLL_OUT   ---2			---not used
{
	if(rst_lt9211==true)	// 	LT9211 must reset once after VTC_MIPI start generate normal 60fps BT1120 timing,  	important    2022.12.06 fyc
	{						//	change LT9211_Rst frome 0->1 will generate posedge which can rst LT9211,
		rst_lt9211_cnt++;
		if(rst_lt9211_cnt == 33)
		{
			pl_imgproc_ctrl.CTRL_CMD.LT9211_Rst 				= 1;
			GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");
			rst_lt9211 = false;
			rst_lt9211_cnt = 0;
			mipi_out_cnt = 0;
			new_mipi_stream_setup = true;
		}
	}
	video_info_MIPI_OUT.ready = true;

#ifndef MEASURE_LOOP_TIME
	if(mipi_video_ctrl.switch_channel==true)
	{
		if(mipi_video_ctrl.is_TV==0)
		{
			pl_imgproc_ctrl.CTRL_CMD.track_Channel_TV0_IR1 	= 0;//switch to TV
			GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");
			switch(FrmWriteID_TV)
			{
				case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1+  DEFAULT_OFFSET_ADDR_TV;		break;
				case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2+  DEFAULT_OFFSET_ADDR_TV;		break;
				case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3+  DEFAULT_OFFSET_ADDR_TV;		break;
				default:	break;
			}
			asm("NOP");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880;	asm("nop");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_TV;		asm("nop");

			DENOISE_MIPI_HANDLE[0x10>>2] = 0x01;asm("nop");//enable
			DENOISE_MIPI_HANDLE[0x28>>2] = 120;asm("nop");//sigma TV-120 IR-50
			DENOISE_MIPI_HANDLE[0x00>>2] = 0x81;asm("nop");//autorun
		}
		else
		{
		    pl_imgproc_ctrl.CTRL_CMD.track_Channel_TV0_IR1 	= 1;//switch to IR
			GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");
#ifdef DEBUG_MIPI_IR_DELAY
			if(switch_mipi_out_pingpong_strategy==ASYNC_MODE)
			{
				switch(FrmWriteID_IR)
				{
					case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8  + 0;		break;
					case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1  + 0;		break;
					case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2  + 0;		break;
					case 3:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3  + 0;		break;
					case 4:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4  + 0;		break;
					case 5:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5  + 0;		break;
					case 6:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6  + 0;		break;
					case 7:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7  + 0;		break;
					default:	break;
				}
			}
			else
			{
				switch(FrmWriteID_IR)
				{
					case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1  + 0;		break;
					case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2  + 0;		break;
					case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3  + 0;		break;
					case 3:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4  + 0;		break;
					case 4:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5  + 0;		break;
					case 5:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6  + 0;		break;
					case 6:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7  + 0;		break;
					case 7:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8  + 0;		break;
					default:	break;
				}
			}
#endif
#ifndef DEBUG_MIPI_IR_DELAY
			switch(FrmWriteID_IR)
			{
				case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1  + 0;		break;
				case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2  + 0;		break;
				case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3  + 0;		break;
				case 3:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4  + 0;		break;
				case 4:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5  + 0;		break;
				case 5:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6  + 0;		break;
				case 6:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7  + 0;		break;
				case 7:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8  + 0;		break;
				default:	break;
			}
#endif
			asm("NOP");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= PARA_IMG_BUF_STRIDE_IN_BYTE_IR;	asm("nop");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2]			= PARA_IMG_BUF_ACITVE_ROWS_TV; 				asm("nop");

			DENOISE_MIPI_HANDLE[0x10>>2] = 0x01;asm("nop");//enable
			DENOISE_MIPI_HANDLE[0x28>>2] = 50;asm("nop");//sigma TV-120 IR-50
			DENOISE_MIPI_HANDLE[0x00>>2] = 0x81;asm("nop");//autorun

		}
		mipi_video_ctrl.switch_channel=false;
	}
	else
	{
		if(mipi_video_ctrl.is_TV==0)
		{
			switch(FrmWriteID_TV)
			{
				case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1+  DEFAULT_OFFSET_ADDR_TV;		break;
				case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2+  DEFAULT_OFFSET_ADDR_TV;		break;
				case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3+  DEFAULT_OFFSET_ADDR_TV;		break;
				default:	break;
			}
			asm("NOP");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_TV;		asm("nop");
		}
		else
		{

#ifdef DEBUG_MIPI_IR_DELAY
			if(switch_mipi_out_pingpong_strategy == ASYNC_MODE)
			{
				switch(FrmWriteID_IR)
				{
					case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8  + 0;		break;
					case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1  + 0;		break;
					case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2  + 0;		break;
					case 3:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3  + 0;		break;
					case 4:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4  + 0;		break;
					case 5:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5  + 0;		break;
					case 6:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6  + 0;		break;
					case 7:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7  + 0;		break;
					default:	break;
				}
			}
			else
			{
				switch(FrmWriteID_IR)
				{
					case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1  + 0;		break;
					case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2  + 0;		break;
					case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3  + 0;		break;
					case 3:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4  + 0;		break;
					case 4:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5  + 0;		break;
					case 5:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6  + 0;		break;
					case 6:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7  + 0;		break;
					case 7:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8  + 0;		break;
					default:	break;
				}
			}
#endif
#ifndef DEBUG_MIPI_IR_DELAY
			switch(FrmWriteID_IR)
			{
				case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1  + 0;		break;
				case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2  + 0;		break;
				case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3  + 0;		break;
				case 3:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4  + 0;		break;
				case 4:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5  + 0;		break;
				case 5:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6  + 0;		break;
				case 6:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7  + 0;		break;
				case 7:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8  + 0;		break;
				default:	break;
			}
#endif
			asm("NOP");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2]			= PARA_IMG_BUF_ACITVE_ROWS_TV; 			asm("nop");
		}
	}
#endif

#ifdef DEBUG_MIPI_IR_DELAY
	#ifdef DEBUG_MIPI_IR_DELAY
		cur_mipi_out_omp_time = omp_get_wtime();
		time_gap_MIPI_miner_IR = 1000.0f*(cur_mipi_out_omp_time-ir_in_latest_omp_time);
		// if(abs(time_gap_MIPI_miner_IR)<time_gap_thresh_upper || abs(time_gap_MIPI_miner_IR) > time_gap_thresh_lower)
		// if(abs(time_gap_MIPI_miner_IR)<time_gap_thresh_upper || time_gap_MIPI_miner_IR < -1.0f)
		 if(time_gap_MIPI_miner_IR < 0.95f && time_gap_MIPI_miner_IR>-0.1f)
		 {
		 switch_mipi_out_pingpong_strategy = ASYNC_MODE;
		 }
		 else
		 {
		 switch_mipi_out_pingpong_strategy = SYNC_MODE;
		 }
//		switch_mipi_out_pingpong_strategy = SYNC_MODE;
	#endif
	if(switch_mipi_out_pingpong_strategy == ASYNC_MODE)
	{
		cur_read_id_mipi = (FrmWriteID_IR + 1)%8;//0--7, 1--0,  2--1,  3--2,  4---3,
		switch(FrmWriteID_IR)
		{
			case 0:		cur_read_id_mipi	= 7  + 0;		break;
			case 1:		cur_read_id_mipi	= 0  + 0;		break;
			case 2:		cur_read_id_mipi	= 1  + 0;		break;
			case 3:		cur_read_id_mipi	= 2  + 0;		break;
			case 4:		cur_read_id_mipi	= 3  + 0;		break;
			case 5:		cur_read_id_mipi	= 4  + 0;		break;
			case 6:		cur_read_id_mipi	= 5  + 0;		break;
			case 7:		cur_read_id_mipi	= 6  + 0;		break;
			default:	break;
		}
	}
	else
	{
		cur_read_id_mipi = FrmWriteID_IR;
	}

	print_diff_time = true;
	mipi_out_cnt++;
#endif
	video_info_MIPI_OUT.ready = true;

	omp_time_mipi_out_cur = cur_mipi_out_omp_time;
	mipi_out_time_gap = 1000.0f*(omp_time_mipi_out_cur-omp_time_mipi_out_pre);
	omp_time_mipi_out_pre = omp_time_mipi_out_cur;

#ifdef MEASURE_LOOP_TIME


	if(mipi_video_ctrl.switch_channel==true)
	{
		if(mipi_video_ctrl.is_TV==0)
		{
			pl_imgproc_ctrl.CTRL_CMD.track_Channel_TV0_IR1 	= 0;//switch to TV
			GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");
			switch(FrmWriteID_TV)
			{
				case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1 + DEFAULT_OFFSET_ADDR_TV;		break;
				case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2 + DEFAULT_OFFSET_ADDR_TV;		break;
				case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3 + DEFAULT_OFFSET_ADDR_TV;		break;
				default:	break;
			}
			asm("NOP");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= PARA_IMG_BUF_STRIDE_IN_BYTE_TV_2880;	asm("nop");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_TV;		asm("nop");
		}
		else
		{
			pl_imgproc_ctrl.CTRL_CMD.track_Channel_TV0_IR1 	= 1;//switch to IR
			GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");
			switch(FrmWriteID_IR)
			{
				case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1  + 0;		break;
				case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2  + 0;		break;
				case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3  + 0;		break;
				case 3:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4  + 0;		break;
				case 4:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5  + 0;		break;
				case 5:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6  + 0;		break;
				case 6:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7  + 0;		break;
				case 7:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8  + 0;		break;
				default:	break;
			}
			asm("NOP");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_STRIDE_IN_BYTE>>2] 		= PARA_IMG_BUF_STRIDE_IN_BYTE_IR;	asm("nop");
			VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2]			= PARA_IMG_BUF_FOR_MIPI_ROWS_IR; 	asm("nop");
		}

		mipi_video_ctrl.switch_channel=false;
	}
	else
	{
		if(mipi_video_ctrl.is_TV==0)
		{
			if(bias_ctrl_left_mipi)
			{
				switch(FrmWriteID_TV)
					{
						case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1+ DEFAULT_OFFSET_ADDR_TV + 20 * PARA_IMG_CHANNELS_TV;		break;
						case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2+ DEFAULT_OFFSET_ADDR_TV + 20 * PARA_IMG_CHANNELS_TV;		break;
						case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3+ DEFAULT_OFFSET_ADDR_TV + 20 * PARA_IMG_CHANNELS_TV;		break;
						default:	break;
					}
					asm("NOP");
				VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_TV;	asm("nop");
				if(bias_hapened)
					{
						time_MIPI_OUT=  std::chrono::system_clock::now();
						bias_hapened = false;
					}
			}
			else
			{
				switch(FrmWriteID_TV)
					{
						case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_1+ DEFAULT_OFFSET_ADDR_TV;		break;
						case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_2+ DEFAULT_OFFSET_ADDR_TV;		break;
						case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_TV_3+ DEFAULT_OFFSET_ADDR_TV;		break;
						default:	break;
					}
				asm("NOP");
				VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] 			= PARA_IMG_BUF_ACITVE_ROWS_TV;		asm("nop");
				if(bias_rst_hapened)
					{
						time_MIPI_OUT=  std::chrono::system_clock::now();
						bias_rst_hapened = false;
					}
			}
		}
		else
		{
			if(bias_ctrl_left_mipi)
			{
				switch(FrmWriteID_IR)
				{
					case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1 + 4 * PARA_IMG_CHANNELS_IR;		break;
					case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2 + 4 * PARA_IMG_CHANNELS_IR;		break;
					case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3 + 4 * PARA_IMG_CHANNELS_IR;		break;
					case 3:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4 + 4 * PARA_IMG_CHANNELS_IR;		break;
					case 4:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5 + 4 * PARA_IMG_CHANNELS_IR;		break;
					case 5:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6 + 4 * PARA_IMG_CHANNELS_IR;		break;
					case 6:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7 + 4 * PARA_IMG_CHANNELS_IR;		break;
					case 7:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8 + 4 * PARA_IMG_CHANNELS_IR;		break;
					default:	break;
				}
				asm("NOP");
				VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2]			= PARA_IMG_BUF_FOR_MIPI_ROWS_IR; 				asm("nop");

				if(bias_hapened)
				{
					time_MIPI_OUT=  std::chrono::system_clock::now();
					bias_hapened = false;
				}
			}
			else
			{
				switch(FrmWriteID_IR)
				{
					case 0:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_1;		break;
					case 1:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_2;		break;
					case 2:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_3;		break;
					case 3:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_4;		break;
					case 4:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_5;		break;
					case 5:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_6;		break;
					case 6:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_7;		break;
					case 7:		VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2]	= VDMA_IMG_BUF_ADDR_IR_8;		break;
					default:	break;
				}
				asm("NOP");
				VDMA_SDI_MIPI_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2]			= PARA_IMG_BUF_FOR_MIPI_ROWS_IR; 				asm("nop");

				if(bias_rst_hapened)
				{
					time_MIPI_OUT=  std::chrono::system_clock::now();
					bias_rst_hapened = false;
				}
			}
		}
	}
	#endif

}
#ifdef PHOTO_IN_DVP_OUT
static int ping_pong_num = 3;
void PHOTO_in()		//sigio ---POLL_IN
{

	photo_info_PHOTO_IN.ready = true;
	FrmWR_ID_PHOTO = (FrmWR_ID_PHOTO+1)%ping_pong_num;
	switch(FrmWR_ID_PHOTO)
	{
	case 0:
		VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_PHOTO_1 ;
		break;
	case 1:
		VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_PHOTO_2 ;
		break;
	case 2:
		VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_S2MM_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_PHOTO_3 ;
		break;
	default:
		break;
	}
	asm("NOP");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_S2MM_ACITVE_ROWS>>2] = 4096;asm("NOP");
	photo_in_cnt++;

#ifdef 	DVP_OUT_STAB_TEST
	omp_time_pt_in_cur = omp_get_wtime();
	pt_in_time_gap = 1000.0f*(omp_time_pt_in_cur-omp_time_pt_in_pre);
	omp_time_pt_in_pre = omp_time_pt_in_cur;
	enable_print_time_gap_pt_in = true;
#endif

}

void DVP_out_rk3588()	//sigio ---POLL_MSG				irq7_drv
{
	photo_info_DVP_OUT.ready = true;
	switch(FrmWR_ID_PHOTO)
	{
	case 0:
		VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_PHOTO_3 ;
		break;
	case 1:
		VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_PHOTO_1 ;
		break;
	case 2:
		VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_START_ADDR>>2] = VDMA_IMG_BUF_ADDR_PHOTO_2 ;
		break;
	default:
		break;
	}
	asm("NOP");
	VDMA_PHOTO_IN_DVP_OUT_HANDLE[VDMA_REG_OFFSET_MM2S_ACITVE_ROWS>>2] = 5462;asm("NOP");
	
#ifdef 	DVP_OUT_STAB_TEST
	omp_time_dvp_out_cur = omp_get_wtime();
	dvp_out_time_gap = 1000.0f*(omp_time_dvp_out_cur-omp_time_dvp_out_pre);
	pt_dvp_time_gap = 1000.0f*(omp_time_dvp_out_cur-omp_time_tv_in_cur);
	omp_time_dvp_out_pre = omp_time_dvp_out_cur;
	enable_print_time_gap_dvp_out = true;
#endif

}

#endif

void wait_moment()
{
	for(int i = 0; i<1000000;i++)		//important when switch video clk src must wait a moment to reset VTC
	{
		asm("nop");
	}
}

#define PI 3.1415926
void update_north_arrow(float north_angle, int color_Y, int color_U, int color_V, bool enable)
{
	static cv::Mat arrow_patch(cv::Size(ARROW_WIDTH,ARROW_HEIGHT),CV_8UC1);
	// float arrow_time_init = omp_get_wtime();
    static cv::Point pt_start(0, 0);
    static cv::Point pt_end(0, 0);
    static int line_thick = 4;
    static int line_type  = 4;

    static int center_hor =  ARROW_WIDTH/2;
    static int center_ver =  ARROW_HEIGHT/2;
    static int arrow_length = (min(ARROW_WIDTH,ARROW_HEIGHT) - 6)/2;

	if(north_angle > 180.0)
	{
		north_angle = north_angle - 360.0;
	}
    float y_var = arrow_length*cos(-PI*north_angle/180.0 - PI);//arrow_length*cos(PI*north_angle/180.0);
    float x_var = arrow_length*sin(-PI*north_angle/180.0 - PI);//arrow_length*sin(PI*north_angle/180.0);
    pt_end.x = x_var + center_hor;
    pt_end.y = y_var + center_ver;

    pt_start.x = center_hor-x_var;
    pt_start.y = center_ver-y_var;
	// float arrow_time_start = omp_get_wtime();
	memset(arrow_patch.data, 0, ARROW_WIDTH*ARROW_HEIGHT);
	// float memset_time = omp_get_wtime();
	arrowedLine(arrow_patch, pt_start, pt_end, cv::Scalar(235), line_thick, line_type, 0, 0.15);
	// float draw_time = omp_get_wtime();
	int rows_offset_mix_buf = 0;
	int cur_offset_mix = 0;
	int rows_offset_mask_buf = 0;
	int cur_offset_mask = 0;
	for(int r = 0; r<ARROW_HEIGHT; r++)
	{
		rows_offset_mix_buf  = r*ARROW_WIDTH*4;
		rows_offset_mask_buf = r*ARROW_WIDTH;
		for(int c = 0; c<ARROW_WIDTH; c++)
		{
			cur_offset_mix  = c*4 + rows_offset_mix_buf;
			cur_offset_mask = c + rows_offset_mask_buf;
			ptr_MIXER_LAYER_NORTH_ARROW_BUF[cur_offset_mix+0] = (arrow_patch.data[cur_offset_mask] && arrow_patch_center_mask.data[cur_offset_mask]) ? color_Y : 0;
			ptr_MIXER_LAYER_NORTH_ARROW_BUF[cur_offset_mix+1] = (arrow_patch.data[cur_offset_mask] && arrow_patch_center_mask.data[cur_offset_mask]) ? color_V: 0;
			ptr_MIXER_LAYER_NORTH_ARROW_BUF[cur_offset_mix+2] = (arrow_patch.data[cur_offset_mask] && arrow_patch_center_mask.data[cur_offset_mask]) ? color_U: 0;
			if(enable)
				ptr_MIXER_LAYER_NORTH_ARROW_BUF[cur_offset_mix+3] = (arrow_patch.data[cur_offset_mask] && arrow_patch_center_mask.data[cur_offset_mask]) ? 255 : 0;
			else
				ptr_MIXER_LAYER_NORTH_ARROW_BUF[cur_offset_mix+3] = 0;
		}
	}
	// float arrow_time_end = omp_get_wtime();
	// printf("memset time: %f  draw time: %f  cpy data time: %f total time: %f \n",1000.0f*(memset_time-arrow_time_start),1000.0f*(draw_time-memset_time),1000.0f*(arrow_time_end-draw_time),1000.0f*(arrow_time_end-arrow_time_init));
}

#ifdef ADDING_SDI_INFO_USING_BRAM
//
//	typedef struct{
//		union
//		{
//			unsigned int union_body;
//			struct{
//				unsigned int byte1:		8;	//corresponding bit 	30:0
//				unsigned int byte2:		8;	//corresponding bit 		30  high-level rst
//				unsigned int byte3:		8;	//corresponding bit 		31
//				unsigned int byte4:		8;	//corresponding bit 		31
//			}para;
//		}config;
//	}sdi_info;
//	sdi_info for_write;
	//#define printf_report_multi_promote
	#ifdef printf_report_multi_promote
		static int cnt_printf = 0;
	#endif
	void update_SDI_first_line_info(unsigned char * ptChar_info)
	{
		for(int c = 0; c<256; c++)
		{
			SDI_INFO_BRAM_HANDLE[c] = ptChar_info[c];asm("nop");//for_write.config.union_body;asm("nop");
		}
		#ifdef printf_report_multi_promote
			cnt_printf++;
			if(cnt_printf%15 == 0)
			{
				printf("info: ");
				for(int c = 0; c<87; c++)
				{
					printf("%c ",ptChar_info[c]);
				}
				printf("\n");
			}
		#endif
	}

	void init_SDI_info()
	{
		SDI_INFO_BRAM_HANDLE[0] = 66;asm("nop");
		for(int c = 1; c<256; c++)
		{
			SDI_INFO_BRAM_HANDLE[c] = 48;asm("nop");
		}
	}
#endif


