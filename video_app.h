/*
 * video_app.h
 *
 *  Created on: 2022-11-01
 *      Author: FYC
 */

#ifndef SRC_VIDEO_APP_H_
#define SRC_VIDEO_APP_H_

#include "video_config.h"
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <omp.h>
#include <pthread.h>
#include <thread>

using namespace std;

#define TV_ALIGN_ON_FMQL

typedef struct {
		bool ready;
#ifdef TV_ALIGN_ON_FMQL
		bool ready_TV_ALIGN;
#endif
		unsigned char *img_buf;
} VIDEO_INFO;

extern unsigned int* VDMA_TV_HANDLE;
extern unsigned int* VDMA_IR_HANDLE;
extern unsigned int* VDMA_PIP_HANDLE;
extern unsigned int* VDMA_SDI_MIPI_HANDLE;

#ifdef ADDING_SDI_INFO_USING_BRAM
	extern unsigned int* SDI_INFO_BRAM_HANDLE;
#endif

#ifdef PHOTO_IN_DVP_OUT
	extern unsigned int* VDMA_PHOTO_IN_DVP_OUT_HANDLE;
#endif

#ifdef RST_UART_IN_PS_PROC
// 2024.03.21  to handle randomly happened restart cause of uart bus error--------------zhangzeyu
// enforece reset uart-lite IPcore in init step
	extern unsigned int* AXI_GPIO_RST_UART_HANDLE;
#endif

#ifdef RST_VID_IN_PS_PROC
//2023.01.05  to handle randomly happened gray screen--------------fengyachun & yangyifan
// enforece reset video in to axis-stream IPcore in init step
	extern unsigned int* AXI_GPIO_RST_VID_HANDLE;
#endif

#ifdef XADC		// 2023.01.31 added by fengyachun to report the temperature of FMQL-7045
	extern unsigned int* XADC_HANDLE;
	extern bool requst_temper;
#endif

//// 20250526
//#ifdef IR_COLORMAP
//	extern unsigned int* IR_COLORMAP_HANDLE;
//
//#endif

#ifdef DETECT_VIDEO_INPUT
	// 2023.01.31 added by fengyachun to detect video input & switch video output clk src
	extern unsigned int* AXI_GPIO_DETECT_VIDEO_SWITCH_CLK_HANDLE;

	typedef struct{
		union
		{
			unsigned int union_body;
			struct{
				unsigned int reserved:				27;	//corresponding bit 	26:0
				unsigned int RK3588_RST:			1;	//reset RK3588 system
				unsigned int RK3399_I2C_SDA:		1;	//corresponding bit 		28  gpio-RK3399 DVP
				unsigned int reserved_bit:			1;	//corresponding bit 		29  not uesed//20230714
				unsigned int rst_gs2972_mannul:		1;	//corresponding bit 		30  high-level rst
				unsigned int video_clk_switch:		1;	//corresponding bit 		31
			}para;
		}config;
	}CLK_INFO;

	typedef struct{
		union
		{
			unsigned int union_body;
			struct{
				unsigned int status_TV:		 	1;	//corresponding bit 		0
				unsigned int status_IR:		 	1;	//corresponding bit 		1
				unsigned int status_send_photo: 1;	//corresponding bit gpio from RK3588: RK3588_GPIO1A0, input bit for sending photo finished
				unsigned int reserved:	   		29;	//corresponding bit			31:3
			}para;
		}config;
	}VIDEO_INPUT_STATUS;

	extern CLK_INFO switch_clk;
	extern VIDEO_INPUT_STATUS video_status;


#endif

#ifdef GAMMA_LUT
	extern unsigned int* GAMMA_LUT_TV_HANDLE;
	extern unsigned int* GAMMA_LUT_IR_HANDLE;
#define ENABLE_GAMMA_LUT_PIP
#ifdef ENABLE_GAMMA_LUT_PIP
	extern unsigned int* GAMMA_LUT_PIP_HANDLE;
#endif

	extern int curve_Y_tv[256];
	extern int curve_U_tv[256];
	extern int curve_V_tv[256];

	extern int curve_Y_ir_src[256];
	extern int curve_U_ir_src[256];
	extern int curve_V_ir_src[256];

	extern int curve_Y_ir_enh[256];
	extern bool ir_enhance_switch_happened;
	extern bool ir_enhance_on;
	#ifdef BLACK_WHITE_SWITCH_IN_AVT
		extern int curve_Y_ir_cvt[256];
		extern bool ir_black_white_switch_happened;
		extern bool ir_black_white;
	#endif

#endif

#ifdef only_median
		extern unsigned int* MEDIAN_MIPI_HANDLE;
#endif

#ifdef only_denoise
		extern unsigned int* DENOISE_MIPI_HANDLE;
#endif

#ifdef median_denoise
		extern unsigned int* DENOISE_MIPI_HANDLE;
		extern unsigned int* MEDIAN_MIPI_HANDLE;
#endif

extern unsigned int* IMG_RESIZE_STREAM_TV_HANDLE;
extern unsigned int* IMG_RESIZE_STREAM_IR_HANDLE;
extern unsigned int* IMG_RESIZE_STREAM_PIP_HANDLE;
extern unsigned int* VIDEO_MIXER_HANDLE;

#ifdef VTC_RESET
		extern unsigned int* VTC_DISP_HANDLE;
	#ifdef VTC_MIPI_RESET
			extern unsigned int* VTC_MIPI_HANDLE;
	#endif
	#ifdef VTC_DVP_RESET
		extern unsigned int* VTC_DVP_HANDLE;
	#endif
#endif

extern unsigned int* GPIO_CROSS_CTRL_HANDLE;
extern unsigned int* GPIO_TAR_Graph_Color_HANDLE;
extern unsigned int* GPIO_VDMA_FRAME_INDEX_HANDLE;

extern unsigned char* pChar_VDMA_IMG_BUF_TV_1;
extern unsigned char* pChar_VDMA_IMG_BUF_TV_2;
extern unsigned char* pChar_VDMA_IMG_BUF_TV_3;

extern unsigned char* pChar_VDMA_IMG_BUF_IR_1;
extern unsigned char* pChar_VDMA_IMG_BUF_IR_2;
extern unsigned char* pChar_VDMA_IMG_BUF_IR_3;
extern unsigned char* pChar_VDMA_IMG_BUF_IR_4;
extern unsigned char* pChar_VDMA_IMG_BUF_IR_5;
extern unsigned char* pChar_VDMA_IMG_BUF_IR_6;
extern unsigned char* pChar_VDMA_IMG_BUF_IR_7;
extern unsigned char* pChar_VDMA_IMG_BUF_IR_8;


#ifdef ADD_dot_line_mixer
	extern unsigned char* ptr_MIXER_LAYER_TXT_BUF;
#endif

extern unsigned char* ptr_MIXER_LAYER_NORTH_ARROW_BUF;

extern int zoom_rate ;

#define TV_MAIN_MODE 		0x01
#define IR_MAIN_MODE		0x02
#define TV_PIP_MODE			0x03
#define IR_PIP_MODE			0x04
#define FUSION_MODE			0x05
#define IR_ALIGN_MODE		0x06

#define SCALING_X1		0x01
#define SCALING_X2		0x02
#define SCALING_X4		0x04


typedef struct {
	bool 			SWITCH_HAPPENED;					//0-------hold state		1----Switch happened
	bool			IR_ALIGN_KEEP;
	unsigned char 	DISP_MODE;				//0x01----TV_MAIN;			0x02----IR_MAIN			0x03-----TV_PIP		0x04----IR_PIP  0x05--------FUSION_MODE     0x06-----IR_ALIGN_MODE
	unsigned char 	SCALING_RATE_TV;			//0x01----x1; 				0x02----x2;				0x04-----x4;
	unsigned char 	SCALING_RATE_IR;			//0x01----x1; 				0x02----x2;				0x04-----x4;
	int 			PIP_START_X;
	int 			PIP_START_Y;
	int 			PIP_WIDTH;
	int 			PIP_HEIGHT;
	union layer_ctrl
	{
		struct {
			unsigned char layer_TV_EN: 	1;	//1  bit ----- 	0 bit ---TV layer
			unsigned char layer_IR_EN:	1;	//1  bit ----- 	1 bit ---IR layer
			unsigned char layer_PIP_EN: 1;	//1  bit ----- 	2 bit ---PIP layer
			unsigned char layer_OSD_EN: 1;	//1  bit ----- 	3 bit ---OSD layer
			unsigned char reserved : 	4;	//4  bit -----7:4 bit ---reserved
		};
		unsigned char layer_en;
	}layer_ctrl;
}DISP_CTRL;

#define COLOR_CHANNEL_ENHANCE 	0
#define MONO_CHANNEL_ENHANCE	1

#define ENHANCE_LEVEL_0per		0
#define ENHANCE_LEVEL_25per		1
#define ENHANCE_LEVEL_50per		2
#define ENHANCE_LEVEL_100per	3
typedef struct{
 union CTRL_CMD
 {
  unsigned int ctrl_bit;
  struct{
   unsigned int ImgEnhanceChannel:   1;  //bit 0    Color----0   Mono----1
   unsigned int ImgEnhanceLevel:   2;  //bit 2~1  bit 00 -----direct connection  bit 01 -----25%      bit 10 ------50%       bit 11 -----100%
   unsigned int sync_ir_row_ver:   12;  //bit 14:3   sync_ir_row_ver
   unsigned int sync_ir_col_hor:   12;  //bit 26:15  sync_ir_col_hor
   unsigned int sync_422_speed:    2;  //bit 28:27   sync_422_speed -------bit 01 0960KHz   bit: 10/11 7680KHz
   unsigned int track_Channel_TV0_IR1:  1;  //bit 29:   track channel switch 0----TV   1----IR    //not used ---2022.12.04----fengyachun
   unsigned int LT9211_Rst:    1;  //bit 30  posedge trigger initial given 1 and keep at 1
   unsigned int BW_Switch:     1;  //bit 31    black hot----0   white hot----1
  };
 }CTRL_CMD;
}PL_IMG_PROC_CTRL;

typedef struct{
	union cross_info
	{
		unsigned int send;
		struct{
			unsigned int cross_ver:		12;		//bit 11:0  ver y
			unsigned int cross_hor:		12;		//bit 23:12 hor x
			unsigned int reserved:		7;		//bit 30:24 reserved
			unsigned int crossEn:		1;		//bit 31  ON----1   OFF----0
		}info;
	}cross_info;
}CROSS_INFO;

typedef struct{
	bool is_TV;//		TV---0;			IR---1
	bool switch_channel; // switch -- true;   maintain --- false;
}MIPI_VIDEO_CTRL;


#define DEBUG_MIPI_IR_DELAY

#ifdef DEBUG_MIPI_IR_DELAY
	extern double 	omp_start_time;
	extern double 	ir_in_latest_omp_time;
	extern double 	cur_mipi_out_omp_time;
	extern int 		cur_write_id_ir;
	extern int 		cur_read_id_mipi;
	extern bool 	print_diff_time;
	extern int 		mipi_ir_cnt;
	extern double	time_gap_MIPI_miner_IR;
	extern bool 	switch_mipi_out_pingpong_strategy;
	extern double 	omp_time_disp_out_pre;
	extern double 	omp_time_disp_out_cur;
	extern double	disp_out_time_gap;
	extern double 	omp_time_mipi_out_pre;
	extern double 	omp_time_mipi_out_cur;
	extern double	mipi_out_time_gap;

#define SYNC_MODE	true
#define ASYNC_MODE	false
#endif

#define TEST_TV_IN_TIME_STABLIZATION
#ifdef 	TEST_TV_IN_TIME_STABLIZATION
	extern double 	omp_time_tv_in_pre;
	extern double 	omp_time_tv_in_cur;
	extern double	tv_in_time_gap;

	extern double 	omp_time_ir_in_pre;
	extern double 	omp_time_ir_in_cur;
	extern double	ir_in_time_gap;

	extern bool 	enable_print_time_gap_tv_in;
	extern bool 	enable_print_time_gap_ir_in;
#endif

#ifdef	IR_CLOSE_VDMA_S2MM_20240125
	extern bool ir_power_on_stat;
	extern int ir_power_on_cnt;
	extern bool cur_ir_s2mm_stat;
#endif

extern int 		ir_in_cnt;
extern int 		tv_in_cnt;
extern int 		disp_out_cnt;
extern int 		mipi_out_cnt;

extern VIDEO_INFO video_info_TV_IN;
extern VIDEO_INFO video_info_IR_IN;
extern VIDEO_INFO video_info_MIPI_OUT;
extern VIDEO_INFO video_info_DISP_OUT;

//#define SUM_DELAY
#ifdef SUM_DELAY
extern VIDEO_INFO video_info_SUM_delay;
#endif


extern DISP_CTRL disp_ctrl;
extern MIPI_VIDEO_CTRL mipi_video_ctrl;

extern PL_IMG_PROC_CTRL pl_imgproc_ctrl;

extern CROSS_INFO cross_Main;
extern CROSS_INFO cross_PIP;


extern bool rst_lt9211;
extern int rst_lt9211_cnt;
extern bool new_mipi_stream_setup;

extern int FrmWriteID_IR;
extern int FrmWriteID_TV;
extern int FrmWriteID_TV_RK3399;

// #define MEASURE_LOOP_TIME
#ifdef MEASURE_LOOP_TIME
	extern std::chrono::system_clock::time_point time_TV_IN;
	extern std::chrono::system_clock::time_point time_DISP_OUT;
	extern std::chrono::system_clock::time_point time_MIPI_OUT;

	extern bool bias_ctrl_left_mipi;
	extern bool bias_hapened;
	extern bool bias_rst_hapened;
#endif


void update_north_arrow(float north_angle, int color_Y, int color_U, int color_V, bool enable);

void wait_moment();
void wait_moment(int nop_nums);
void video_init();
void reset_VTC_IN_SYNC_MODE();
void reset_VTC_IN_ASYNC_MODE();
//bool video_intr_init();
//void video_intr_callback(int signum,siginfo_t *info,void *myact);

void video_in_TV();
void video_out_SDI_MIPI();
void video_in_IR();
void video_out_DISP();
#ifdef PHOTO_IN_DVP_OUT
	void PHOTO_in();		//sigio ---POLL_IN
	void DVP_out_rk3588();	//sigio ---POLL_MSG
	extern bool lock_dvp_start;
	extern int photo_in_cnt;
	extern unsigned int FrmWR_ID_PHOTO;
	extern unsigned int FrmWR_SUB_ID_PHOTO_DVP_OUT;
	extern VIDEO_INFO photo_info_PHOTO_IN;
	extern VIDEO_INFO photo_info_DVP_OUT;
	extern unsigned char* pChar_VDMA_IMG_BUF_PHOTO_1;
	extern unsigned char* pChar_VDMA_IMG_BUF_PHOTO_2;
	extern unsigned char* pChar_VDMA_IMG_BUF_PHOTO_3;
	extern unsigned char* send_pt_cmd_stat_shared;

	typedef struct {
			unsigned char cmd_init_state	=0;		//init_state
			unsigned char cmd_photo_id		=3;		//1,2,3,default = 3;
			unsigned char cmd_send_enable	=0;		//0 ---send disable;  1----send enable
			unsigned char work_end_init 	=0;
			unsigned char work_photo_id		=3;		//1,2,3,default = 3;
			unsigned char work_stat			=0; 	//0 ---finished;  1----sending
	} PHOTO_CMD_STAT_SHARED;
	extern PHOTO_CMD_STAT_SHARED *photo_cmd_stat_shared;
#endif

#ifdef	SDI_ADD_INFO
	extern unsigned char* ptr_MIXER_HOST_INFO_BUF_ADDR;
#endif

//#define DVP_OUT_STAB_TEST
#ifdef 	DVP_OUT_STAB_TEST
	extern double 	omp_time_pt_in_pre;
	extern double 	omp_time_pt_in_cur;
	extern double	pt_in_time_gap;
	extern bool 	enable_print_time_gap_pt_in;

	extern double 	omp_time_dvp_out_pre;
	extern double 	omp_time_dvp_out_cur;
	extern double	dvp_out_time_gap;
	extern bool 	enable_print_time_gap_dvp_out;

	extern double	pt_dvp_time_gap;


#endif
//#define DVP_CNT_reset_debug
#ifdef DVP_CNT_reset_debug
	extern unsigned char *pCh_sub_idx;
#endif


#ifdef ADDING_SDI_INFO_USING_BRAM
	void init_SDI_info();
	void update_SDI_first_line_info(unsigned char * ptChar_info);
#endif

#endif /* SRC_VIDEO_APP_H_ */
