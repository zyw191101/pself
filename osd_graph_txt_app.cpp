/*
 * osd_graph_txt_app.cpp
 *
 *  Created on: 2022��8��29��
 *      Author: FYC
 */

#include "osd_graph_txt_app.h"


unsigned int* GPIO_TAR1_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR1_XYWH_ADDR));
unsigned int* GPIO_TAR2_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR2_XYWH_ADDR));
unsigned int* GPIO_TAR3_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR3_XYWH_ADDR));
unsigned int* GPIO_TAR4_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR4_XYWH_ADDR));
unsigned int* GPIO_TAR5_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR5_XYWH_ADDR));
unsigned int* GPIO_TAR6_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR6_XYWH_ADDR));

unsigned int* GPIO_TAR7_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR7_XYWH_ADDR));
unsigned int* GPIO_TAR8_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR8_XYWH_ADDR));
unsigned int* GPIO_TAR9_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR9_XYWH_ADDR));
unsigned int* GPIO_TAR10_XYWH_HANDLE  = (unsigned int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)(AXI_GPIO_TAR10_XYWH_ADDR));



TarGraphInfo TarInfo_1;
TarGraphInfo TarInfo_2;
TarGraphInfo TarInfo_3;
TarGraphInfo TarInfo_4;
TarGraphInfo TarInfo_5;
TarGraphInfo TarInfo_6;

TarGraphInfo TarInfo_7;
TarGraphInfo TarInfo_8;
TarGraphInfo TarInfo_9;
TarGraphInfo TarInfo_10;

TarGraphColor targraphColor;




void init_tar_graph()
{

//	printf("enter init tar\n");
	TarInfo_1.tar_idx 				= 0;
	TarInfo_1.gate.info.Enable 		= false;
	TarInfo_1.gate.info.IdEn		= false;
	TarInfo_1.gate.info.Id 			= 1;
	TarInfo_1.gate.info.width 		= 48;
	TarInfo_1.gate.info.height		= 48;
	TarInfo_1.cross.info.Enable		= false;
	TarInfo_1.cross.info.line_width	= 3;
	TarInfo_1.cross.info.cols_hor_x	= 960;
	TarInfo_1.cross.info.rows_ver_y = 540;


	TarInfo_2.tar_idx 				= 0;
	TarInfo_2.gate.info.Enable 		= false;
	TarInfo_2.gate.info.IdEn		= false;
	TarInfo_2.gate.info.Id 			= 2;
	TarInfo_2.gate.info.width 		= 32;
	TarInfo_2.gate.info.height		= 32;
	TarInfo_2.cross.info.Enable		= false;
	TarInfo_2.cross.info.line_width	= 1;
	TarInfo_2.cross.info.cols_hor_x	= 960;
	TarInfo_2.cross.info.rows_ver_y = 256*1;


	TarInfo_3.tar_idx 				= 0;
	TarInfo_3.gate.info.Enable 		= false;
	TarInfo_3.gate.info.IdEn		= false;
	TarInfo_3.gate.info.Id 			= 3;
	TarInfo_3.gate.info.width 		= 48;
	TarInfo_3.gate.info.height		= 48;
	TarInfo_3.cross.info.Enable		= false;
	TarInfo_3.cross.info.line_width	= 1;
	TarInfo_3.cross.info.cols_hor_x	= 960+256*2;
	TarInfo_3.cross.info.rows_ver_y = 256*1;


	TarInfo_4.tar_idx 				= 0;
	TarInfo_4.gate.info.Enable 		= false;
	TarInfo_4.gate.info.IdEn		= false;
	TarInfo_4.gate.info.Id 			= 4;
	TarInfo_4.gate.info.width 		= 48;
	TarInfo_4.gate.info.height		= 48;
	TarInfo_4.cross.info.Enable		= false;
	TarInfo_4.cross.info.line_width	= 1;
	TarInfo_4.cross.info.cols_hor_x	= 960-256*2;
	TarInfo_4.cross.info.rows_ver_y = 256*3;


	TarInfo_5.tar_idx 				= 0;
	TarInfo_5.gate.info.Enable 		= false;
	TarInfo_5.gate.info.IdEn		= false;
	TarInfo_5.gate.info.Id 			= 5;
	TarInfo_5.gate.info.width 		= 48;
	TarInfo_5.gate.info.height		= 48;
	TarInfo_5.cross.info.Enable		= false;
	TarInfo_5.cross.info.line_width	= 1;
	TarInfo_5.cross.info.cols_hor_x	= 960;
	TarInfo_5.cross.info.rows_ver_y = 256*3;


	TarInfo_6.tar_idx 				= 0;
	TarInfo_6.gate.info.Enable 		= false;//
	TarInfo_6.gate.info.IdEn		= false;
	TarInfo_6.gate.info.Id 			= 6;
	TarInfo_6.gate.info.width 		= 48;
	TarInfo_6.gate.info.height		= 48;
	TarInfo_6.cross.info.Enable		= false;
	TarInfo_6.cross.info.line_width	= 1;
	TarInfo_6.cross.info.cols_hor_x	= 960-256*2;
	TarInfo_6.cross.info.rows_ver_y = 256*3;


	TarInfo_7.tar_idx 				= 0;
	TarInfo_7.gate.info.Enable 		= false;
	TarInfo_7.gate.info.IdEn		= false;
	TarInfo_7.gate.info.Id 			= 7;
	TarInfo_7.gate.info.width 		= 48;
	TarInfo_7.gate.info.height		= 48;
	TarInfo_7.cross.info.Enable		= false;
	TarInfo_7.cross.info.line_width	= 1;
	TarInfo_7.cross.info.cols_hor_x	= 960;
	TarInfo_7.cross.info.rows_ver_y = 540;

	TarInfo_8.tar_idx 				= 0;
	TarInfo_8.gate.info.Enable 		= false;
	TarInfo_8.gate.info.IdEn		= false;
	TarInfo_8.gate.info.Id 			= 8;
	TarInfo_8.gate.info.width 		= 48;
	TarInfo_8.gate.info.height		= 48;
	TarInfo_8.cross.info.Enable		= false;
	TarInfo_8.cross.info.line_width	= 1;
	TarInfo_8.cross.info.cols_hor_x	= 960;
	TarInfo_8.cross.info.rows_ver_y = 256*3;

	TarInfo_9.tar_idx 				= 0;
	TarInfo_9.gate.info.Enable 		= false;
	TarInfo_9.gate.info.IdEn		= false;
	TarInfo_9.gate.info.Id 	= 0;
	TarInfo_9.gate.info.width 		= 48;
	TarInfo_9.gate.info.height		= 48;
	TarInfo_9.cross.info.Enable		= false;
	TarInfo_9.cross.info.line_width	= 1;
	TarInfo_9.cross.info.cols_hor_x	= 960 + 256*1;
	TarInfo_9.cross.info.rows_ver_y = 256*3;

	TarInfo_10.tar_idx 				= 0;
	TarInfo_10.gate.info.Enable 	= false;
	TarInfo_10.gate.info.IdEn		= false;
	TarInfo_10.gate.info.Id 		= 0;
	TarInfo_10.gate.info.width 		= 48;
	TarInfo_10.gate.info.height		= 48;
	TarInfo_10.cross.info.Enable	= false;
	TarInfo_10.cross.info.line_width= 1;
	TarInfo_10.cross.info.cols_hor_x= 960 + 256*2;
	TarInfo_10.cross.info.rows_ver_y= 256*3;



	GPIO_TAR1_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_1.cross.Send;
	GPIO_TAR1_XYWH_HANDLE[Tar_Gate_Addr_offset>>2] 	= TarInfo_1.gate.Send;

	GPIO_TAR2_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_2.cross.Send;
	GPIO_TAR2_XYWH_HANDLE[Tar_Gate_Addr_offset>>2] 	= TarInfo_2.gate.Send;

	GPIO_TAR3_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_3.cross.Send;
	GPIO_TAR3_XYWH_HANDLE[Tar_Gate_Addr_offset>>2] 	= TarInfo_3.gate.Send;

	GPIO_TAR4_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_4.cross.Send;
	GPIO_TAR4_XYWH_HANDLE[Tar_Gate_Addr_offset>>2] 	= TarInfo_4.gate.Send;

	GPIO_TAR5_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_5.cross.Send;
	GPIO_TAR5_XYWH_HANDLE[Tar_Gate_Addr_offset>>2] 	= TarInfo_5.gate.Send;

	GPIO_TAR6_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_6.cross.Send;
	GPIO_TAR6_XYWH_HANDLE[Tar_Gate_Addr_offset>>2] 	= TarInfo_6.gate.Send;

	GPIO_TAR7_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_7.cross.Send;
	GPIO_TAR7_XYWH_HANDLE[Tar_Gate_Addr_offset>>2] 	= TarInfo_7.gate.Send;

	GPIO_TAR8_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_8.cross.Send;
	GPIO_TAR8_XYWH_HANDLE[Tar_Gate_Addr_offset>>2] 	= TarInfo_8.gate.Send;

	GPIO_TAR9_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_9.cross.Send;
	GPIO_TAR9_XYWH_HANDLE[Tar_Gate_Addr_offset>>2] 	= TarInfo_9.gate.Send;

	GPIO_TAR10_XYWH_HANDLE[Tar_Cross_Addr_offset>>2] = TarInfo_10.cross.Send;
	GPIO_TAR10_XYWH_HANDLE[Tar_Gate_Addr_offset>>2]  = TarInfo_10.gate.Send;

	targraphColor.ColorConfig.color.reserved  = 0;
	targraphColor.ColorConfig.color.Color_R_Y = 235;
	targraphColor.ColorConfig.color.Color_G_U = 128;
	targraphColor.ColorConfig.color.Color_B_V = 128;

	GPIO_TAR_Graph_Color_HANDLE[Tar_Graph_Color_Addr_offset>>2] = targraphColor.ColorConfig.send;

	//black white hot switch init
	pl_imgproc_ctrl.CTRL_CMD.BW_Switch         = IR_HOT_WHITE;   //IR_HOT_BLACK---0              IR_HOT_WHITE---1
	pl_imgproc_ctrl.CTRL_CMD.LT9211_Rst        = 0;     //important 2022.12.06 fyc, rst signal of LT9211 generated in MIPI out interrupt, see func video_out_SDI_MIPI()
	pl_imgproc_ctrl.CTRL_CMD.ImgEnhanceChannel = COLOR_CHANNEL_ENHANCE;
    pl_imgproc_ctrl.CTRL_CMD.ImgEnhanceLevel   = ENHANCE_LEVEL_0per;
    pl_imgproc_ctrl.CTRL_CMD.track_Channel_TV0_IR1  = 0; //not used ---2022.12.04----fengyachun
	pl_imgproc_ctrl.CTRL_CMD.sync_422_speed    = 3;
	pl_imgproc_ctrl.CTRL_CMD.sync_ir_row_ver   = 825;
	pl_imgproc_ctrl.CTRL_CMD.sync_ir_col_hor   = 1;
	GPIO_TAR_Graph_Color_HANDLE[CTRL_OFFSET_PORT2>>2]	= pl_imgproc_ctrl.CTRL_CMD.ctrl_bit;	asm("nop");

	printf("tar graph init done.\n");
}

//				RESIZE_CONFIG[4] 	= READ_HEIGHT;										//0x10
//				RESIZE_CONFIG[6] 	= READ_WIDTH;										//0x18
//				RESIZE_CONFIG[8] 	= READ_HEIGHT;										//0x20
//				RESIZE_CONFIG[10] 	= READ_WIDTH;										//0x28
//				RESIZE_CONFIG[0]	= 0x81;												//AUTO_RESTART------0x81

cOSD_MAPPING::cOSD_MAPPING()
{
	ASCII_CHARS_BUF = new unsigned char[ASCII_CHAR_NUM *ASCII_CHAR_ROWS*ASCII_CHAR_COLS];
}


cOSD_MAPPING::~cOSD_MAPPING()
{
	delete[] ASCII_CHARS_BUF;
}


void cOSD_MAPPING::init()
{
	memset(ptr_MIXER_LAYER_TXT_BUF,0,TXT_LAYER_FB_LENGTH);
	std::string ascii_lib_path = "/root/ASCII_WHITE_WRJ_16_32.txt";
	std::ifstream fstream_ascii_lib;
	fstream_ascii_lib.open(ascii_lib_path, std::ios::in);
	if (fstream_ascii_lib.fail())
	{
		printf("read file fail\n");
	}
	else
	{
		std::string tmp;
		long int bit_cnt = 0;
		while (getline(fstream_ascii_lib, tmp, '\n'))//read one row of ascii characters lib
		{
			std::istringstream is(tmp);
			std::string str_tmp;
			for (size_t i = 0; i < ASCII_CHAR_COLS; i++)
			{
				is >> str_tmp;
				ASCII_CHARS_BUF[bit_cnt] = std::stoi(str_tmp) ? 255 : 0;
				bit_cnt++;
			}
		}
		printf("charlib init done.\n");
	}
	fstream_ascii_lib.close();

	osd_info_setup();
	osd_location_setup();
#ifdef IF_DEBUG_OSD
		auto osd_start = std::chrono::system_clock::now();
#endif
	osd_sys_info.osd_enable = false;
	update_OSD_SYS_INFO();
    //**************system time ***** ----- comm
	osd_sys_time.osd_enable = false;
	update_OSD_SYS_TIME();
  	//**************menu OSD example	----	comm agreement-4.1.2.10 MENU select
   	//obj_OSD_MAPPING.osd_menu.str_OSD_MENU_1 = "FSET";
   	//obj_OSD_MAPPING.osd_menu.str_OSD_MENU_2 = {' ',ASCII_24_ARROW_RIGHT_BAR,ASCII_26_ARROW_RIGHT ,' ',' '}; //when using custom defined char in ascii-character-point-array-lib ; ref macro definition in cOSD_MAPPING.h
   	//obj_OSD_MAPPING.osd_menu.str_OSD_MENU_3 = "LSET";
   	//obj_OSD_MAPPING.osd_menu.str_OSD_MENU_4 = "MORE";
   	//obj_OSD_MAPPING.osd_menu.str_OSD_MENU_5 = "INFL";
   	//obj_OSD_MAPPING.osd_menu.osd_enable = true;					//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_menu.osd_enable = false;
	update_OSD_MENU();

	osd_menu_bd.osd_enable = false;
	update_OSD_MENU_BD();

	//**************system init info OSD example	----	comm agreement-4.1.2.1 system init info
	//obj_OSD_MAPPING.osd_sys_init_info.str_OSD = "SYSTEM INITIAL BIT FAILED";
	//obj_OSD_MAPPING.osd_sys_init_info.osd_enable = true;			//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF

	update_OSD_SYSTEM_INIT_INFO();

	//**************system work mode OSD example	----	comm agreement-4.1.2.3 system work mode
	//obj_OSD_MAPPING.osd_sys_mode.str_OSD = "MANUAL";
	//obj_OSD_MAPPING.osd_sys_mode.osd_enable = true;
	osd_sys_mode.osd_enable = false;
	update_OSD_SYSTEM_MODE();

	//**************VCP OSD example	----	comm agreement-4.1.2.8 aerial carrier velocity compensation mode
	//obj_OSD_MAPPING.osd_vcp.str_OSD = "-VCP";
	//obj_OSD_MAPPING.osd_vcp.osd_enable = true;			//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_vcp.osd_enable = false;
	update_OSD_VCP();

	//**************TV work mode OSD example	----	comm agreement-4.1.2.5 TV work mode
	//obj_OSD_MAPPING.osd_state_tv.str_OSD = "TVx*+++";
	//obj_OSD_MAPPING.osd_state_tv.osd_enable = true;		//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_state_tv.osd_enable = false;
	update_OSD_STATE_TV();

	//**************NIR work mode OSD example	----	comm agreement-4.1.2.6 IR work mode
	//obj_OSD_MAPPING.osd_state_nir.str_OSD = "IRx*++";
	//obj_OSD_MAPPING.osd_state_nir.osd_enable = true;			//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_state_nir.osd_enable = false;
	update_OSD_STATE_NIR();

	//**************TV or IR Main video work channel OSD example	----	comm agreement-4.1.1.3 TV or IR Main video work channel
	//obj_OSD_MAPPING.osd_main_video_work.osd_enable = true;				//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	//obj_OSD_MAPPING.osd_main_video_work.TV_or_NIR = 1;					//       0---TV;1---NIR
	osd_main_video_work.osd_enable = false;
	update_OSD_MAIN_VIDEO_WORK();

	//**************LAZER work mode OSD example	----	comm agreement-4.1.2.7 LAZER work mode
	//obj_OSD_MAPPING.osd_state_lazer.str_OSD = { ASCII_24_ARROW_RIGHT_BAR,ASCII_26_ARROW_RIGHT,ASCII_29_LAZER_FORBID_LAUNCH,'*',ASCII_28_PENTA_STAR };
	//obj_OSD_MAPPING.osd_state_lazer.osd_enable = false;						//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_state_lazer.osd_enable = false;
	update_OSD_STATE_LAZER();

	//**************longitude latitude altitue OSD example	----	comm agreement-4.1.2.14 longitude latitude altitue
	//obj_OSD_MAPPING.osd_tar_lon_lat_alt.str_OSD_TAR_LON = "LON: 000000";
	//obj_OSD_MAPPING.osd_tar_lon_lat_alt.str_OSD_TAR_LAT = "LAT: 000000";
	//obj_OSD_MAPPING.osd_tar_lon_lat_alt.str_OSD_TAR_ALT = "ALT: 0000.0";
	//obj_OSD_MAPPING.osd_tar_lon_lat_alt.osd_enable = true;						//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_tar_lon_lat_alt.osd_enable = false;
	update_OSD_TAR_LON_LAT_ALT();

	//**************optical aixs OSD example	----	comm agreement-4.1.2.20 optical aixs
	//obj_OSD_MAPPING.osd_optic_axis_lbr.str_axis_l = "L: 000.00000";
	//obj_OSD_MAPPING.osd_optic_axis_lbr.str_axis_b = "B: 000.00000";
	//obj_OSD_MAPPING.osd_optic_axis_lbr.str_axis_r = "R: 00000";
	//obj_OSD_MAPPING.osd_optic_axis_lbr.osd_enable = true;						//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_optic_axis_lbr.osd_enable = false;
	update_OSD_OPTIC_AXIS_LBR();

	//**************FOV info OSD example	----	comm agreement-4.1.2.19 FOV info
	//obj_OSD_MAPPING.osd_fov.str_OSD = "FOV: 00.00 x 00.00";
	//obj_OSD_MAPPING.osd_fov.osd_enable = true;					//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_fov.osd_enable = false;
	update_OSD_FOV();

	//**************STRING_2 OSD example	----	comm agreement-4.1.2.22 STRING_2
	//obj_OSD_MAPPING.osd_string_2.str_OSD = "STRING-2 test";
	//obj_OSD_MAPPING.osd_string_2.osd_enable = true;					//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF

	update_OSD_STRING_2();

	//**************length width height info OSD example	----	comm agreement-4.1.2.21 length width height info
	//obj_OSD_MAPPING.osd_lwh.str_OSD_L = "L: 0000.0";
	//obj_OSD_MAPPING.osd_lwh.str_OSD_W = "W: 0000.0";
	//obj_OSD_MAPPING.osd_lwh.str_OSD_H = "H: 0000.0";
	//obj_OSD_MAPPING.osd_lwh.osd_enable = true;								//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_lwh.osd_enable = false;
	update_OSD_LWH();

	//**************AZ EL RNG attitude angle OSD example	----	comm agreement-4.1.2.16 AZ EL RNG attitude angle
	//obj_OSD_MAPPING.osd_attitude_angle.str_OSD_AZ = "AZ : 000.0";
	//obj_OSD_MAPPING.osd_attitude_angle.str_OSD_EL = "EL : 000.0";
	//obj_OSD_MAPPING.osd_attitude_angle.str_OSD_RNG = "RNG: 000.0";
	//obj_OSD_MAPPING.osd_attitude_angle.osd_enable = false;								//true --- current OSD SHOW ON;     false --- current OSD SHOW OFF
	osd_attitude_angle.osd_enable = false;
	update_OSD_ATTITUDE_ANGEL();

#ifdef IF_DEBUG_OSD
	auto osd_end = std::chrono::system_clock::now();
	auto osd_during = (float)std::chrono::duration_cast<std::chrono::microseconds>(osd_end - osd_start).count() / 1000.0f;
	cout << "osd init time: " << osd_during << " ms" << endl;
#endif
}


void cOSD_MAPPING::clear_osd()
{
	memset(ptr_MIXER_LAYER_TXT_BUF,0,TXT_LAYER_FB_LENGTH);
	//memset(ptr_MIXER_LAYER1_BUF,0,FB_LENGTH);
}

void cOSD_MAPPING::switch_osd_color(int color_type)
{
	switch(color_type)
	{
	case OSD_COLOR_BLACK:
		color_cur_Y = 16;
		color_cur_U = 128;
		color_cur_V = 128;
		color_cur_A = 255;
		break;
	case OSD_COLOR_WHTIE:
		color_cur_Y = 235;
		color_cur_U = 128;
		color_cur_V = 128;
		color_cur_A = 255;
		break;
	case OSD_COLOR_RED:
		color_cur_Y = 76;
		color_cur_U = 84;
		color_cur_V = 255;
		color_cur_A = 255;
		break;
	case OSD_COLOR_YELLOW:
		color_cur_Y = 210;
		color_cur_U = 16;
		color_cur_V = 146;
		color_cur_A = 255;
		break;
	case OSD_COLOR_BLUE:
		color_cur_Y = 29;
		color_cur_U = 255;
		color_cur_V = 107;
		color_cur_A = 255;
		break;
	case OSD_COLOR_GREEN:
		color_cur_Y = 125;
		color_cur_U = 54;
		color_cur_V = 34;
		color_cur_A = 255;
		break;
	default: break;
	}


}
void cOSD_MAPPING::osd_info_setup()
{
	osd_menu.length1 = 5;
	osd_menu.length2 = 5;
	osd_menu.length3 = 5;
	osd_menu.length4 = 5;
	osd_menu.length5 = 5;
	osd_menu.str_OSD_MENU_1 = "FSET";				//comm agreement-4.1.2.10 MENU select
	osd_menu.str_OSD_MENU_2 = "ASET";
	osd_menu.str_OSD_MENU_3 = "LSET";
	osd_menu.str_OSD_MENU_4 = "MORE";
	osd_menu.str_OSD_MENU_5 = "INFL";
	osd_menu.osd_enable = false;

	osd_menu_bd.osd_enable = false;

	osd_sys_init_info.length = 17;
	osd_sys_init_info.str_OSD = "";		//comm agreement-4.1.2.1 system init info
	osd_sys_init_info.osd_enable = true;

	osd_sys_mode.length = 8;
	osd_sys_mode.str_OSD = "INIT";			//comm agreement-4.1.2.3 system work mode
	osd_sys_mode.osd_enable = true;

	osd_vcp.length = 4;
	osd_vcp.str_OSD = "";				//comm agreement-4.1.2.8 aerial carrier velocity compensation mode
	osd_vcp.osd_enable = true;

	osd_state_tv.length = 6;
	osd_state_tv.str_OSD = "TV   ";			//comm agreement-4.1.2.5 TV work mode
	osd_state_tv.osd_enable = true;

	osd_state_nir.length = 6;
	osd_state_nir.str_OSD = "IR   ";			//comm agreement-4.1.2.6 IR work mode
	osd_state_nir.osd_enable = true;

	osd_main_video_work.str_OSD = { ASCII_24_ARROW_RIGHT_BAR,ASCII_26_ARROW_RIGHT };	//comm agreement-4.1.1.3 TV or IR Main video work channel
	osd_main_video_work.osd_enable = true;
	osd_main_video_work.TV_or_NIR = 0;//// 0---TV;1---NIR

	osd_state_lazer.length = 7;
	osd_state_lazer.str_OSD = "";		//comm agreement-4.1.2.7 LAZER work mode
	osd_state_lazer.osd_enable = true;

	osd_tar_lon_lat_alt.length_lon = 17;
	osd_tar_lon_lat_alt.length_lat = 17;
	osd_tar_lon_lat_alt.length_alt = 17;
	osd_tar_lon_lat_alt.str_OSD_TAR_LON = "LON: 00.00000";	//comm agreement-4.1.2.14 longitude latitude altitue
	osd_tar_lon_lat_alt.str_OSD_TAR_LAT = "LAT: 00.00000";
	osd_tar_lon_lat_alt.str_OSD_TAR_ALT = "ALT: 0000.0";
	osd_tar_lon_lat_alt.osd_enable = true;

	osd_optic_axis_lbr.length_l = 20;
	osd_optic_axis_lbr.length_b = 20;
	osd_optic_axis_lbr.length_r = 20;
	osd_optic_axis_lbr.str_axis_l = "L: 00.00000";		//comm agreement-4.1.2.20 optical aixs
	osd_optic_axis_lbr.str_axis_b = "B: 00.00000";
	osd_optic_axis_lbr.str_axis_r = "R: 00000";
	osd_optic_axis_lbr.osd_enable = true;

	osd_fov.length = 18;
	osd_fov.str_OSD = "FOV: 00.00 x 00.00";				//comm agreement-4.1.2.19 FOV info
	osd_fov.osd_enable = true;

	osd_string_2.length = 20;
	osd_string_2.str_OSD = "";		//comm agreement-4.1.2.22 STRING_2
	osd_string_2.osd_enable = true;

	osd_lwh.str_OSD_L = "L: 000.0";	//comm agreement-4.1.2.21 length width height info
	osd_lwh.str_OSD_W = "W: 000.0";
	osd_lwh.str_OSD_H = "H: 000.0";
	osd_lwh.osd_enable = true;

	osd_attitude_angle.str_OSD_AZ = "AZ : 00.00";		//comm agreement-4.1.2.16 AZ EL RNG attitude angle
	osd_attitude_angle.str_OSD_EL = "EL : 00.00";
	osd_attitude_angle.str_OSD_RNG = "RNG: 00000";
	osd_attitude_angle.osd_enable = true;

	osd_sys_time.length = 24;
	osd_sys_time.osd_enable = true;
	osd_sys_time.str_OSD = "2022-11-01 00:00:00 0000";

	osd_sys_info.length = 20;
	osd_sys_info.osd_enable = true;
	osd_sys_info.str_OSD = " ";

	//add by zzy 20230109 for tracking test
	osd_test_info.length1 = 60;
	osd_test_info.length2 = 60;
	osd_test_info.osd_enable = false;
	osd_test_info.str_OSD_test_info1 = " ";
	osd_test_info.str_OSD_test_info2 = " ";
}
void cOSD_MAPPING::osd_location_setup()
{
	osd_sys_info.location_rows = 952;
	osd_sys_info.location_cols = 960 - 16 * 10;

	osd_sys_time.location_rows = 68;
	osd_sys_time.location_cols = 100;

	osd_menu.location_rows_1 = 100;  //comm agreement-4.1.2.10 MENU select
	osd_menu.location_cols_1 = 680;
	osd_menu.location_rows_2 = osd_menu.location_rows_1;
	osd_menu.location_cols_2 = osd_menu.location_cols_1 + 1 * 7 * 16 + 2 * 1;
	osd_menu.location_rows_3 = osd_menu.location_rows_1;
	osd_menu.location_cols_3 = osd_menu.location_cols_1 + 2 * 7 * 16 + 2 * 2;
	osd_menu.location_rows_4 = osd_menu.location_rows_1;
	osd_menu.location_cols_4 = osd_menu.location_cols_1 + 3 * 7 * 16 + 2 * 3;
	osd_menu.location_rows_5 = osd_menu.location_rows_1;
	osd_menu.location_cols_5 = osd_menu.location_cols_1 + 4 * 7 * 16 + 2 * 4;

	osd_sys_init_info.location_rows = 196;//comm agreement-4.1.2.1 system init info
	osd_sys_init_info.location_cols = 960 - 8* osd_sys_init_info.str_OSD.size();

	osd_sys_mode.location_rows = 132;			//comm agreement-4.1.2.3 system work mode
	osd_sys_mode.location_cols = 120;

	osd_vcp.location_rows = 132;				//comm agreement-4.1.2.8 aerial carrier velocity compensation mode
	osd_vcp.location_cols = 222;

	osd_state_tv.location_rows = 196;			//comm agreement-4.1.2.5 TV work mode
	osd_state_tv.location_cols = 120;

	osd_state_nir.location_rows = 228;			//comm agreement-4.1.2.6 IR work mode
	osd_state_nir.location_cols = 120;

	osd_main_video_work.location_rows_TV = 196;	//comm agreement-4.1.1.3 TV or IR Main video work channel
	osd_main_video_work.location_cols_TV = 88;
	osd_main_video_work.location_rows_NIR = osd_main_video_work.location_rows_TV+32;
	osd_main_video_work.location_cols_NIR = osd_main_video_work.location_cols_TV;

	osd_state_lazer.location_rows = 260;		// comm agreement-4.1.2.7 LAZER work mode
	osd_state_lazer.location_cols = 88;         // modified by zzy 20221027

	osd_tar_lon_lat_alt.location_rows_tar_lon = 196;	//comm agreement-4.1.2.14 longitude latitude altitue
	osd_tar_lon_lat_alt.location_cols_tar_lon = 1600;   
	osd_tar_lon_lat_alt.location_rows_tar_lat = osd_tar_lon_lat_alt.location_rows_tar_lon + 32;
	osd_tar_lon_lat_alt.location_cols_tar_lat = osd_tar_lon_lat_alt.location_cols_tar_lon;
	osd_tar_lon_lat_alt.location_rows_tar_alt = osd_tar_lon_lat_alt.location_rows_tar_lon + 64;
	osd_tar_lon_lat_alt.location_cols_tar_alt = osd_tar_lon_lat_alt.location_cols_tar_lon;

	osd_optic_axis_lbr.location_rows_axis_L = 68;		//comm agreement-4.1.2.20 optical aixs 68
	osd_optic_axis_lbr.location_cols_axis_L = 1600;     
	osd_optic_axis_lbr.location_rows_axis_B = osd_optic_axis_lbr.location_rows_axis_L + 32;
	osd_optic_axis_lbr.location_cols_axis_B = osd_optic_axis_lbr.location_cols_axis_L;
	osd_optic_axis_lbr.location_rows_axis_R = osd_optic_axis_lbr.location_rows_axis_L + 64;
	osd_optic_axis_lbr.location_cols_axis_R = osd_optic_axis_lbr.location_cols_axis_L;

	osd_fov.location_rows = 920;				//comm agreement-4.1.2.19 FOV info
	osd_fov.location_cols = 120;

	osd_string_2.location_rows = 920;			//comm agreement-4.1.2.22 STRING_2
	osd_string_2.location_cols = 960 - 8 * osd_string_2.str_OSD.size();

	osd_lwh.location_rows_L = 792;				//comm agreement-4.1.2.21 length width height info 792
	osd_lwh.location_cols_L = 1600;
	osd_lwh.location_rows_W = osd_lwh.location_rows_L+33;
	osd_lwh.location_cols_W = osd_lwh.location_cols_L;
	osd_lwh.location_rows_H = osd_lwh.location_rows_L+66;
	osd_lwh.location_cols_H = osd_lwh.location_cols_L;

	osd_attitude_angle.location_rows_AZ = 920;		//comm agreement-4.1.2.16 AZ EL RNG attitude angle
	osd_attitude_angle.location_cols_AZ = 1600;
	osd_attitude_angle.location_rows_EL = osd_attitude_angle.location_rows_AZ + 32;
	osd_attitude_angle.location_cols_EL = osd_attitude_angle.location_cols_AZ;
	osd_attitude_angle.location_rows_RNG = osd_attitude_angle.location_rows_AZ + 64;
	osd_attitude_angle.location_cols_RNG= osd_attitude_angle.location_cols_AZ;

	//add by zzy 20230109 for tracking test info
	osd_test_info.location_rows_1 = 1000;
	osd_test_info.location_cols_1 = 32;
	osd_test_info.location_rows_2 = 1032;
	osd_test_info.location_cols_2 = 32;
}

void cOSD_MAPPING::update_OSD_TEST_INFO()
{
	osd_test_info.length1 = 30;
	osd_test_info.length2 = 30;
	addTxtToLayer(osd_test_info.osd_enable, osd_test_info.str_OSD_test_info1, osd_test_info.location_rows_1, osd_test_info.location_cols_1);
	addTxtToLayer(osd_test_info.osd_enable, osd_test_info.str_OSD_test_info2, osd_test_info.location_rows_2, osd_test_info.location_cols_2);
}

void cOSD_MAPPING::update_OSD_SYS_INFO()
{
	osd_sys_info.length = 20;
	osd_sys_info.location_cols = 960 - 8 * osd_sys_info.str_OSD.size();
	addTxtToLayer(osd_sys_info.osd_enable, osd_sys_info.str_OSD, osd_sys_info.location_rows, osd_sys_info.location_cols);
}

void cOSD_MAPPING::update_OSD_SYS_TIME()
{
	osd_sys_time.length = 24;
	addTxtToLayer(osd_sys_time.osd_enable, osd_sys_time.str_OSD, osd_sys_time.location_rows, osd_sys_time.location_cols);
}

void cOSD_MAPPING::update_OSD_MENU()
{
	osd_menu.length1 = 5;
	osd_menu.length2 = 5;
	osd_menu.length3 = 5;
	osd_menu.length4 = 5;
	osd_menu.length5 = 5;

	addTxtToLayer(osd_menu.osd_enable, osd_menu.str_OSD_MENU_1, osd_menu.location_rows_1, osd_menu.location_cols_1);
	addTxtToLayer(osd_menu.osd_enable, osd_menu.str_OSD_MENU_2, osd_menu.location_rows_2, osd_menu.location_cols_2);
	addTxtToLayer(osd_menu.osd_enable, osd_menu.str_OSD_MENU_3, osd_menu.location_rows_3, osd_menu.location_cols_3);
	addTxtToLayer(osd_menu.osd_enable, osd_menu.str_OSD_MENU_4, osd_menu.location_rows_4, osd_menu.location_cols_4);
	addTxtToLayer(osd_menu.osd_enable, osd_menu.str_OSD_MENU_5, osd_menu.location_rows_5, osd_menu.location_cols_5);
}


void cOSD_MAPPING::update_OSD_MENU_BD()
{
	int color_Y = osd_menu_bd.osd_enable?color_cur_Y:0;
	int color_U = osd_menu_bd.osd_enable?color_cur_U:0;
	int color_V = osd_menu_bd.osd_enable?color_cur_V:0;
	int color_A = osd_menu_bd.osd_enable?color_cur_A:0;

	for(int r = 98 ; r < 100 ; r++)
	{
		int off_set_1 = r*TXT_LAYER_STRIDE;
		for(int c = 664 ; c < 1234 ; c++)
		{
			int off_set_2 = off_set_1+c*TXT_LAYER_CHANNEL;
			ptr_MIXER_LAYER_TXT_BUF[off_set_2 + Y_CHANNEL] = color_Y;
			ptr_MIXER_LAYER_TXT_BUF[off_set_2 + U_CHANNEL] = color_U;
			ptr_MIXER_LAYER_TXT_BUF[off_set_2 + V_CHANNEL] = color_V;
			ptr_MIXER_LAYER_TXT_BUF[off_set_2 + A_CHANNEL] = color_A;//A alpha,255---totally not opaque    0---fully transparent

		}
	}
	for(int r = 132 ; r < 134 ; r++)
	{
		int off_set_1 = r*TXT_LAYER_STRIDE;
		for(int c = 664 ; c < 1234 ; c++)
		{
			int off_set_2 = off_set_1+c*TXT_LAYER_CHANNEL;
			ptr_MIXER_LAYER_TXT_BUF[off_set_2 + Y_CHANNEL] = color_Y;
			ptr_MIXER_LAYER_TXT_BUF[off_set_2 + U_CHANNEL] = color_U;
			ptr_MIXER_LAYER_TXT_BUF[off_set_2 + V_CHANNEL] = color_V;
			ptr_MIXER_LAYER_TXT_BUF[off_set_2 + A_CHANNEL] = color_A;//A alpha,255---totally not opaque    0---fully transparent

		}
	}
	for(int r = 98 ; r < 134 ; r++)
	{
		int off_set_1 = r*TXT_LAYER_STRIDE;
		for(int c = 664 ; c <= 1234 ; c += ((1234 - 664) / 5))
		{
			for (int c2 = 0; c2 < 2; ++c2)
			{
				int off_set_2 = off_set_1+(c+c2)*TXT_LAYER_CHANNEL;
				ptr_MIXER_LAYER_TXT_BUF[off_set_2 + Y_CHANNEL] = color_Y;
				ptr_MIXER_LAYER_TXT_BUF[off_set_2 + U_CHANNEL] = color_U;
				ptr_MIXER_LAYER_TXT_BUF[off_set_2 + V_CHANNEL] = color_V;
				ptr_MIXER_LAYER_TXT_BUF[off_set_2 + A_CHANNEL] = color_A;//A alpha,255---totally not opaque    0---fully transparent
			}
		}
	}
	// int menu_block_stride = 16 * 7 * TXT_LAYER_CHANNEL;
	// int menu_block_start_col_stride = 664*TXT_LAYER_CHANNEL;
	// for (int c = 0; c < 6; c++)
	// {
	// 	int offset_1  = menu_block_stride * c + 2 * c +menu_block_start_col_stride;
	// 	for (int r = 0; r < 32; r++)
	// 	{
	// 		int offset_2 = TXT_LAYER_STRIDE * (100 + r) + offset_1;
	// 		for(int linewidth = 0;linewidth<2;linewidth++ )
	// 		{
	// 			int offset_3 = linewidth*TXT_LAYER_CHANNEL+offset_2;
	// 			ptr_MIXER_LAYER_TXT_BUF[offset_3+ Y_CHANNEL] = color_Y;
	// 			ptr_MIXER_LAYER_TXT_BUF[offset_3+ U_CHANNEL] = color_U;
	// 			ptr_MIXER_LAYER_TXT_BUF[offset_3+ V_CHANNEL] = color_V;
	// 			ptr_MIXER_LAYER_TXT_BUF[offset_3+ A_CHANNEL] = color_A;//A alpha,255---totally not opaque    0---fully transparent
	// 		}
	// 	}
	// }
}

void cOSD_MAPPING::update_OSD_SYSTEM_INIT_INFO()
{
	osd_sys_init_info.length = 17;
	osd_sys_init_info.location_cols = 960 - 8 * osd_sys_init_info.str_OSD.size();
	addTxtToLayer(osd_sys_init_info.osd_enable, osd_sys_init_info.str_OSD, osd_sys_init_info.location_rows, osd_sys_init_info.location_cols);

}

void cOSD_MAPPING::update_OSD_SYSTEM_MODE()
{
	osd_sys_mode.length = 9;
	addTxtToLayer(osd_sys_mode.osd_enable, osd_sys_mode.str_OSD, osd_sys_mode.location_rows, osd_sys_mode.location_cols);
}

void cOSD_MAPPING::update_OSD_VCP()
{
	osd_vcp.length = 4;
	addTxtToLayer(osd_vcp.osd_enable, osd_vcp.str_OSD, osd_vcp.location_rows, osd_vcp.location_cols);
}

void cOSD_MAPPING::update_OSD_STATE_TV()
{
	osd_state_tv.length = 6;
	addTxtToLayer(osd_state_tv.osd_enable, osd_state_tv.str_OSD, osd_state_tv.location_rows, osd_state_tv.location_cols);
}

void cOSD_MAPPING::update_OSD_STATE_NIR()
{
	osd_state_nir.length = 6;
	addTxtToLayer(osd_state_nir.osd_enable, osd_state_nir.str_OSD, osd_state_nir.location_rows, osd_state_nir.location_cols);
}

void cOSD_MAPPING::update_OSD_MAIN_VIDEO_WORK()
{
	if (osd_main_video_work.TV_or_NIR==0)//TV---0;    NIR------1
	{
		addTxtToLayer(osd_main_video_work.osd_enable, osd_main_video_work.str_OSD,  osd_main_video_work.location_rows_TV, osd_main_video_work.location_cols_TV);
		addTxtToLayer(osd_main_video_work.TV_or_NIR, osd_main_video_work.str_OSD,  osd_main_video_work.location_rows_NIR, osd_main_video_work.location_cols_NIR);
	}
	else
	{
		addTxtToLayer(!osd_main_video_work.TV_or_NIR, osd_main_video_work.str_OSD,  osd_main_video_work.location_rows_TV, osd_main_video_work.location_cols_TV);
		addTxtToLayer(osd_main_video_work.osd_enable, osd_main_video_work.str_OSD, osd_main_video_work.location_rows_NIR, osd_main_video_work.location_cols_NIR);
	}
}
void cOSD_MAPPING::update_OSD_STATE_LAZER()
{
	osd_state_lazer.length = 7;
	addTxtToLayer(osd_state_lazer.osd_enable, osd_state_lazer.str_OSD, osd_state_lazer.location_rows, osd_state_lazer.location_cols);
}

void cOSD_MAPPING::update_OSD_TAR_LON_LAT_ALT()
{
	osd_tar_lon_lat_alt.length_lon = 13;
	osd_tar_lon_lat_alt.length_lat = 13;
	osd_tar_lon_lat_alt.length_alt = 13;
	addTxtToLayer(osd_tar_lon_lat_alt.osd_enable, osd_tar_lon_lat_alt.str_OSD_TAR_LON, osd_tar_lon_lat_alt.location_rows_tar_lon, osd_tar_lon_lat_alt.location_cols_tar_lon);
	addTxtToLayer(osd_tar_lon_lat_alt.osd_enable, osd_tar_lon_lat_alt.str_OSD_TAR_LAT, osd_tar_lon_lat_alt.location_rows_tar_lat, osd_tar_lon_lat_alt.location_cols_tar_lat);
	addTxtToLayer(osd_tar_lon_lat_alt.osd_enable, osd_tar_lon_lat_alt.str_OSD_TAR_ALT, osd_tar_lon_lat_alt.location_rows_tar_alt, osd_tar_lon_lat_alt.location_cols_tar_alt);
}

void cOSD_MAPPING::update_OSD_OPTIC_AXIS_LBR()
{
	addTxtToLayer(osd_optic_axis_lbr.osd_enable, osd_optic_axis_lbr.str_axis_l, osd_optic_axis_lbr.location_rows_axis_L, osd_optic_axis_lbr.location_cols_axis_L);
	addTxtToLayer(osd_optic_axis_lbr.osd_enable, osd_optic_axis_lbr.str_axis_b, osd_optic_axis_lbr.location_rows_axis_B, osd_optic_axis_lbr.location_cols_axis_B);
	addTxtToLayer(osd_optic_axis_lbr.osd_enable, osd_optic_axis_lbr.str_axis_r, osd_optic_axis_lbr.location_rows_axis_R, osd_optic_axis_lbr.location_cols_axis_R);
}

void cOSD_MAPPING::update_OSD_FOV()
{
	osd_fov.length = 18;
	addTxtToLayer(osd_fov.osd_enable, osd_fov.str_OSD, osd_fov.location_rows, osd_fov.location_cols);
}

void cOSD_MAPPING::update_OSD_STRING_2()
{
	osd_string_2.length = 20;
	osd_string_2.location_cols = 960 - 8 * osd_string_2.str_OSD.size();
	addTxtToLayer(osd_string_2.osd_enable, osd_string_2.str_OSD, osd_string_2.location_rows, osd_string_2.location_cols);
}

void cOSD_MAPPING::update_OSD_LWH()
{

	addTxtToLayer(osd_lwh.osd_enable, osd_lwh.str_OSD_L, osd_lwh.location_rows_L, osd_lwh.location_cols_L);
	addTxtToLayer(osd_lwh.osd_enable, osd_lwh.str_OSD_W, osd_lwh.location_rows_W, osd_lwh.location_cols_W);
	addTxtToLayer(osd_lwh.osd_enable, osd_lwh.str_OSD_H, osd_lwh.location_rows_H, osd_lwh.location_cols_H);
}

void cOSD_MAPPING::update_OSD_ATTITUDE_ANGEL()
{
	addTxtToLayer(osd_attitude_angle.osd_enable, osd_attitude_angle.str_OSD_AZ, osd_attitude_angle.location_rows_AZ, osd_attitude_angle.location_cols_AZ);
	addTxtToLayer(osd_attitude_angle.osd_enable, osd_attitude_angle.str_OSD_EL, osd_attitude_angle.location_rows_EL, osd_attitude_angle.location_cols_EL);
	addTxtToLayer(osd_attitude_angle.osd_enable, osd_attitude_angle.str_OSD_RNG, osd_attitude_angle.location_rows_RNG, osd_attitude_angle.location_cols_RNG);
}


//adding a txt line to TXT layer
void cOSD_MAPPING::addTxtToLayer(bool show_ON_OFF, unsigned char *pTxt, int lenthTxt, int location_rows, int location_cols)
{
	for (int i = 0; i < lenthTxt; i++)
	{
		int tmp_rows = location_rows;
		int tmp_cols = location_cols + i * ASCII_CHAR_COLS;
		if (pTxt[i] > 127)
			continue;
		addCharToLayer(show_ON_OFF,ptr_MIXER_LAYER_TXT_BUF, ASCII_CHARS_BUF + pTxt[i] * ASCII_CHAR_ROWS * ASCII_CHAR_COLS, tmp_rows, tmp_cols);
	}
}
void cOSD_MAPPING::addTxtToLayer(bool show_ON_OFF, string str, int location_rows, int location_cols)
{
	const char *pTxt = str.data();
	int lenthTxt = str.size();
	for (int i = 0; i < lenthTxt; i++)
	{
		int tmp_rows = location_rows;
		int tmp_cols = location_cols + i * ASCII_CHAR_COLS;
		if (pTxt[i] > 127)
			continue;
		addCharToLayer(show_ON_OFF,ptr_MIXER_LAYER_TXT_BUF, ASCII_CHARS_BUF + pTxt[i] * ASCII_CHAR_ROWS * ASCII_CHAR_COLS, tmp_rows, tmp_cols);
	}
}
unsigned char * cOSD_MAPPING::getCharPointArray(unsigned char cAscii_idx)
{

	return ASCII_CHARS_BUF + cAscii_idx * ASCII_CHAR_ROWS * ASCII_CHAR_COLS;
}
//adding single character to TXT layer



void cOSD_MAPPING::addCharToLayer(bool show_ON_OFF,unsigned char *pchar_layer_txt, unsigned char *ascii_char_arr, int location_rows, int location_cols)
{

	if (location_rows>=TXT_LAYER_HEIGHT- ASCII_CHAR_ROWS || location_cols>=TXT_LAYER_WIDTH- ASCII_CHAR_COLS)
	{
		return;
	}
	int init_offset = location_rows*TXT_LAYER_STRIDE + location_cols*TXT_LAYER_CHANNEL;
	for (size_t r = 0; r < ASCII_CHAR_ROWS; r++)
	{
		int offset_l1 = init_offset + r*TXT_LAYER_STRIDE;
		int offset_a1 = r*ASCII_CHAR_COLS;

		for (size_t c = 0; c < ASCII_CHAR_COLS; c++)
		{
			int offset_l2 = offset_l1 + c*TXT_LAYER_CHANNEL;
			int offset_a2 = offset_a1 + c;
			pchar_layer_txt[offset_l2 + U_CHANNEL] = (show_ON_OFF && ascii_char_arr[offset_a2]) ? color_cur_U:0;//R --- U or V
			pchar_layer_txt[offset_l2 + V_CHANNEL] = (show_ON_OFF && ascii_char_arr[offset_a2]) ? color_cur_V:0;//G --- U or V
			pchar_layer_txt[offset_l2 + Y_CHANNEL] = (show_ON_OFF && ascii_char_arr[offset_a2]) ? color_cur_Y:0;//B --- Y
			pchar_layer_txt[offset_l2 + A_CHANNEL] = (show_ON_OFF && ascii_char_arr[offset_a2]) ? color_cur_A:0;//A alpha,255---totally not opaque    0---fully transparent

//			pchar_layer_txt[init_offset + r*TXT_LAYER_STRIDE + c*TXT_LAYER_CHANNEL + U_CHANNEL] = (show_ON_OFF && ascii_char_arr[r*ASCII_CHAR_COLS + c]) ? 128 : 0;
//			pchar_layer_txt[init_offset + r*TXT_LAYER_STRIDE + c*TXT_LAYER_CHANNEL + V_CHANNEL] = (show_ON_OFF && ascii_char_arr[r*ASCII_CHAR_COLS + c]) ? 128 : 0;
//			pchar_layer_txt[init_offset + r*TXT_LAYER_STRIDE + c*TXT_LAYER_CHANNEL + Y_CHANNEL] = (show_ON_OFF && ascii_char_arr[r*ASCII_CHAR_COLS + c]) ? 235 : 0;
//			pchar_layer_txt[init_offset + r*TXT_LAYER_STRIDE + c*TXT_LAYER_CHANNEL + A_CHANNEL] = (show_ON_OFF && ascii_char_arr[r*ASCII_CHAR_COLS + c]) ? 255 : 0;
		}

	}
}
