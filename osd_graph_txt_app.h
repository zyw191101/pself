/*
 * osd_graph.h
 *
 *  Created on: 2022-11-01
 *      Author: FYC
 */

#ifndef SRC_OSD_GRAPH_TXT_APP_H_
#define SRC_OSD_GRAPH_TXT_APP_H_
#include "osd_graph_txt_config.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream>
#include <sys/mman.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <chrono>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>

#include "video_app.h"

#define IF_DEBUG_OSD

using namespace std;

//add by zzy 20230109 for tracking test info
typedef struct
{
	int location_rows_1;
	int location_cols_1;
	int location_rows_2;
	int location_cols_2;	
	int length1;
	int length2;
	string str_OSD_test_info1;
	string str_OSD_test_info2;
	bool osd_enable;
}OSD_TEST_INFO;

//comm agreement-4.1.2.10 MENU select
typedef struct
{
	int location_rows_1;
	int location_cols_1;
	int location_rows_2;
	int location_cols_2;
	int location_rows_3;
	int location_cols_3;
	int location_rows_4;
	int location_cols_4;
	int location_rows_5;
	int location_cols_5;
	int length1;
	int length2;
	int length3;
	int length4;
	int length5;
	string str_OSD_MENU_1;
	string str_OSD_MENU_2;
	string str_OSD_MENU_3;
	string str_OSD_MENU_4;
	string str_OSD_MENU_5;
	bool osd_enable;
}OSD_MENU;

//comm agreement-4.1.2.1 system init info
typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_SYSTEM_INIT_INFO;

//comm agreement-4.1.2.3 system work mode
typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_SYSTEM_MODE;

//comm agreement-4.1.2.8 aerial carrier velocity compensation mode
typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_VCP;

//comm agreement-4.1.2.5 TV work mode
typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_STATE_TV;

//comm agreement-4.1.2.6 IR work mode
typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_STATE_NIR;

//comm agreement-4.1.1.3 TV or IR Main video work channel
typedef struct
{
	int location_rows_TV;
	int location_cols_TV;
	int location_rows_NIR;
	int location_cols_NIR;
	string str_OSD;
	bool TV_or_NIR;// 0---TV;1---NIR
	bool osd_enable;
}OSD_MAIN_VIDEO_WORK;

//comm agreement-4.1.2.7 LAZER work mode
typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_STATE_LAZER;

//comm agreement-4.1.2.14 longitude latitude altitue
typedef struct
{
	int location_rows_tar_lon;
	int location_cols_tar_lon;
	int location_rows_tar_lat;
	int location_cols_tar_lat;
	int location_rows_tar_alt;
	int location_cols_tar_alt;
	int length_lon;
	int length_lat;
	int length_alt;
	string str_OSD_TAR_LON;
	string str_OSD_TAR_LAT;
	string str_OSD_TAR_ALT;
	bool osd_enable;
}OSD_TAR_LON_LAT_ALT;

//comm agreement-4.1.2.20 optical aixs
typedef struct
{
	int location_rows_axis_L;
	int location_cols_axis_L;
	int location_rows_axis_B;
	int location_cols_axis_B;
	int location_rows_axis_R;
	int location_cols_axis_R;
	int length_l;
	int length_b;
	int length_r;
	string str_axis_l;
	string str_axis_b;
	string str_axis_r;
	bool osd_enable;
}OSD_OPTIC_AXIS_LBR;

//comm agreement-4.1.2.19 FOV info
typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_FOV;

//comm agreement-4.1.2.22 STRING_2
typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_STRING_2;

//comm agreement-4.1.2.21 length width height info
typedef struct
{
	int location_rows_L;
	int location_cols_L;
	int location_rows_W;
	int location_cols_W;
	int location_rows_H;
	int location_cols_H;
	string str_axis_l;
	string str_axis_w;
	string str_axis_h;
	string str_OSD_L;
	string str_OSD_W;
	string str_OSD_H;
	bool osd_enable;
}OSD_LWH;

//comm agreement-4.1.2.16 AZ EL RNG attitude angle
typedef struct
{
	int location_rows_AZ;
	int location_cols_AZ;
	int location_rows_EL;
	int location_cols_EL;
	int location_rows_RNG;
	int location_cols_RNG;
	string str_axis_az;
	string str_axis_el;
	string str_axis_rng;
	string str_OSD_AZ;
	string str_OSD_EL;
	string str_OSD_RNG;
	bool osd_enable;
}OSD_ATTITUDE_ANGEL;

typedef struct
{
	bool osd_enable;
}OSD_MENU_BD;

typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_SYS_TIME;

typedef struct
{
	int location_rows;
	int location_cols;
	int length;
	string str_OSD;
	bool osd_enable;
}OSD_SYS_INFO;

extern unsigned int* GPIO_TAR1_XYWH_HANDLE;
extern unsigned int* GPIO_TAR2_XYWH_HANDLE;
extern unsigned int* GPIO_TAR3_XYWH_HANDLE;
extern unsigned int* GPIO_TAR4_XYWH_HANDLE;
extern unsigned int* GPIO_TAR5_XYWH_HANDLE;
extern unsigned int* GPIO_TAR6_XYWH_HANDLE;
extern unsigned int* GPIO_TAR7_XYWH_HANDLE;
extern unsigned int* GPIO_TAR8_XYWH_HANDLE;
extern unsigned int* GPIO_TAR9_XYWH_HANDLE;
extern unsigned int* GPIO_TAR10_XYWH_HANDLE;


extern TarGraphInfo TarInfo_1;
extern TarGraphInfo TarInfo_2;
extern TarGraphInfo TarInfo_3;
extern TarGraphInfo TarInfo_4;
extern TarGraphInfo TarInfo_5;
extern TarGraphInfo TarInfo_6;
extern TarGraphInfo TarInfo_7;
extern TarGraphInfo TarInfo_8;
extern TarGraphInfo TarInfo_9;
extern TarGraphInfo TarInfo_10;

extern TarGraphColor targraphColor;




void init_tar_graph();



class cOSD_MAPPING
{
public:

	cOSD_MAPPING();
	~cOSD_MAPPING();

private:
	unsigned char *ASCII_CHARS_BUF;

	unsigned char* ptr_MIXER_LAYER2_BUF;
public:
	unsigned char *LAYER_TXT_VIS;	// osd img layer

	OSD_MENU				osd_menu;				//comm agreement-4.1.2.10 MENU select
	OSD_MENU_BD				osd_menu_bd;				//comm agreement-4.1.2.10 MENU select
	OSD_SYSTEM_INIT_INFO	osd_sys_init_info;		//comm agreement-4.1.2.1 system init info
	OSD_SYSTEM_MODE			osd_sys_mode;			//comm agreement-4.1.2.3 system work mode
	OSD_VCP					osd_vcp;				//comm agreement-4.1.2.8 aerial carrier velocity compensation mode
	OSD_STATE_TV			osd_state_tv;			//comm agreement-4.1.2.5 TV work mode
	OSD_STATE_NIR			osd_state_nir;			//comm agreement-4.1.2.6 IR work mode
	OSD_MAIN_VIDEO_WORK		osd_main_video_work;	//comm agreement-4.1.1.3 TV or IR Main video work channel
	OSD_STATE_LAZER			osd_state_lazer;		//comm agreement-4.1.2.7 LAZER work mode
	OSD_TAR_LON_LAT_ALT		osd_tar_lon_lat_alt;	//comm agreement-4.1.2.14 longitude latitude altitue
	OSD_OPTIC_AXIS_LBR		osd_optic_axis_lbr;		//comm agreement-4.1.2.20 optical aixs
	OSD_FOV					osd_fov;				//comm agreement-4.1.2.19 FOV info
	OSD_STRING_2			osd_string_2;			//comm agreement-4.1.2.22 STRING_2
	OSD_LWH					osd_lwh;				//comm agreement-4.1.2.21 length width height info
	OSD_ATTITUDE_ANGEL		osd_attitude_angle;		//comm agreement-4.1.2.16 AZ EL RNG attitude angle
	OSD_SYS_TIME			osd_sys_time;
	OSD_SYS_INFO            osd_sys_info;        //comm agreement-ID 0x22 AVT VER: 1.00
	//add by zzy 20230109 for tracking test info
	OSD_TEST_INFO           osd_test_info;


	void init();
	void switch_osd_color(int color_type);
	void clear_osd();
	void update_OSD_MENU();
	void update_OSD_MENU_BD();
	void update_OSD_SYSTEM_INIT_INFO();
	void update_OSD_SYSTEM_MODE();
	void update_OSD_VCP();
	void update_OSD_STATE_TV();
	void update_OSD_STATE_NIR();
	void update_OSD_MAIN_VIDEO_WORK();
	void update_OSD_STATE_LAZER();
	void update_OSD_TAR_LON_LAT_ALT();
	void update_OSD_OPTIC_AXIS_LBR();
	void update_OSD_FOV();
	void update_OSD_STRING_2();
	void update_OSD_LWH();
	void update_OSD_ATTITUDE_ANGEL();
	void update_OSD_SYS_TIME();
	void update_OSD_SYS_INFO();
	//add by zzy 20230109 for tracking test info
	void update_OSD_TEST_INFO();

	unsigned char color_cur_Y;
	unsigned char color_cur_U;
	unsigned char color_cur_V;
	unsigned char color_cur_A;

private:

	void osd_location_setup();
	void osd_info_setup();
	void addTxtToLayer(bool show_ON_OFF, unsigned char *pTxt, int lenthTxt, int location_rows, int location_cols);
	void addTxtToLayer(bool show_ON_OFF, string str, int location_rows, int location_cols);
	unsigned char * getCharPointArray(unsigned char cAscii_idx);
	void addCharToLayer(bool show_ON_OFF,unsigned char *pchar_layer_txt, unsigned char *ascii_char_arr, int location_rows, int location_cols);


};




#endif /* SRC_OSD_GRAPH_TXT_APP_H_ */
