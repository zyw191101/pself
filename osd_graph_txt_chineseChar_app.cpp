/*
 * osd_graph_txt_chineseChar_app.cpp
 *
 *  Created on: 2023闁跨喐鏋婚幏锟�1闁跨喐鏋婚幏锟�8闁跨喐鏋婚幏锟�
 *      Author: FYC
 */

#include "osd_graph_txt_chineseChar_app.h"



unsigned int* OSD_BRAM_HANDLE = (unsigned int*)mmap(NULL, OSD_BRAM_SPACE_BYTE, PROT_READ | PROT_WRITE, MAP_SHARED, open("/dev/mem", O_RDWR | O_SYNC), (off_t)OSD_BRAM_CTRL_ADDR);


OSD_INFO osd_sys_mode_and_vcp;
OSD_INFO osd_pos_test;
OSD_INFO osd_zyw_info;
//zyw add
OSD_INFO osd_pos_fov;//FOV
OSD_INFO osd_pos_ir;
OSD_INFO osd_pos_tv;
OSD_INFO osd_pos_ldr;

OSD_INFO osd_pos_vcp;

OSD_INFO osd_pos_enhance_avt;
OSD_INFO osd_pos_enhance_tv;
OSD_INFO osd_pos_enhance_ir;

OSD_INFO osd_pos_z;
OSD_INFO osd_pos_s;
OSD_INFO osd_pos_n;
OSD_INFO osd_pos_m;
OSD_INFO osd_pos_w;

OSD_INFO osd_pos_arrow;
OSD_INFO osd_pos_arrow_tail;
OSD_INFO osd_pos_arrow_blank;
OSD_INFO osd_pos_az;
OSD_INFO osd_pos_el;
OSD_INFO osd_pos_rng;
OSD_INFO osd_pos_top_char;



//zyw add end

OSD_INFO osd_pos_ir_power;
OSD_INFO osd_pos_22;//osd_pos_ldr_power
OSD_INFO osd_pos_image_enhancement;
OSD_INFO osd_pos_moving_target;
OSD_INFO osd_pos_data_collect;
OSD_INFO osd_pos_fault_code_1;
OSD_INFO osd_pos_fault_code_2;
OSD_INFO osd_pos_yaw;
OSD_INFO osd_pos_pitch;
OSD_INFO osd_pos_laser_work_mode;
OSD_INFO osd_pos_deorbit_time;
OSD_INFO osd_pos_shine_code;
OSD_INFO osd_pos_shine_time;
OSD_INFO osd_pos_laser_prohibit;
OSD_INFO osd_pos_temperature_alarm;
OSD_INFO osd_pos_system_information;
OSD_INFO osd_pos_object_position;
OSD_INFO osd_pos_object_position_longitude;
OSD_INFO osd_pos_object_position_latitude;
OSD_INFO osd_pos_object_position_altitude;
OSD_INFO osd_pos_aircraft_position;
OSD_INFO osd_pos_aircraft_position_longitude;
OSD_INFO osd_pos_aircraft_position_latitude;
OSD_INFO osd_pos_aircraft_position_altitude;
OSD_INFO osd_pos_date;
OSD_INFO osd_pos_time;
OSD_INFO osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir;
OSD_INFO osd_pos_inertia_velocitycompensation_workmode;
OSD_INFO osd_pos_north_angle;
OSD_INFO osd_pos_30;
OSD_INFO osd_pos_31;
OSD_INFO osd_pos_init_sys[6];
OSD_INFO osd_pos_version;
#ifdef CHINESE_CHAR_INDEX

//ASCII_24_ARROW_LEFT_BAR,ASCII_24_ARROW_RIGHT_BAR
unsigned short osd_arrow[32]        =      {ASCII_27_ARROW_LEFT, ASCII_26_ARROW_RIGHT,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_arrow_tail[32]        =      {0, 0, 0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_blank[32]        =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_sensor_viewangle_enhance_viewstate_bricont_ir[32]     =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_fov[32]     =      {ASCII_F, ASCII_O, ASCII_V, ':', 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_inertia_velocitycompensation_workmode[32]              =      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','L','M','C',' ',' ',' ',CH_idx_shou_848_1,CH_idx_shou_849_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_zyw[32] = {ASCII_Z, ASCII_Y,ASCII_W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//zyw add 20250210
unsigned short osd_tv[32] = {ASCII_24_ARROW_RIGHT_BAR, ASCII_26_ARROW_RIGHT,ASCII_T, ASCII_V,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_ir[32] = {0, 0,0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_ldr[32] = { 0, 0,ASCII_L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned short osd_vcp[32] = { ASCII_I, ASCII_N,ASCII_I, ASCII_T, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned short osd_enhance_avt[32] = {0, 0,0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_enhance_tv[32] = {0, 0,0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_enhance_ir[32] = {0, 0,0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};



unsigned short osd_z[32] = { ASCII_Z,0, 0 , 0, 0, 0, 0, 0, 0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_s[32] = { ASCII_S, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_n[32] = { ASCII_N, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_m[32] = { ASCII_M, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_w[32] = { ASCII_W, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned short osd_OPERATIONAL_MESSAGE[32] = { ASCII_O,ASCII_P, ASCII_E , ASCII_R, ASCII_T, ASCII_I, ASCII_O, ASCII_N, ASCII_A , ' ', ASCII_M, ASCII_E, ASCII_S, ASCII_S, ASCII_A, ASCII_G, ASCII_E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_az[32] = { ASCII_A, ASCII_Z,':', '0', '0', '.', '0', '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_el[32] = { ASCII_E, ASCII_L, ':', '0', '0', '.', '0', '0',  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_rng[32] = { ASCII_R, ASCII_N,ASCII_G,':', 0, '0',  0,0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_top_char[32] = { 0,  ASCII_F, ASCII_S,ASCII_E, ASCII_T, 0, 0,ASCII_A, ASCII_S,ASCII_E, ASCII_T, 0, 0,  ASCII_L, ASCII_S,ASCII_E, ASCII_T, 0, 0,  ASCII_M, ASCII_O,ASCII_R, ASCII_E,0,0, ASCII_I, ASCII_N,ASCII_F, ASCII_L,0,0,0};


//------------------------------------------------------------------------------------------------------------------------------
unsigned short osd_light[32] = {ASCII_L, ASCII_I, ASCII_G, ASCII_H, ASCII_T, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_medium[32] = {ASCII_M, ASCII_E, ASCII_D, ASCII_I, ASCII_U, ASCII_M, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_heavy[32] = {ASCII_H, ASCII_E, ASCII_A, ASCII_V, ASCII_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_big[32] = {ASCII_B, ASCII_I, ASCII_G, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_small[32] = {ASCII_S, ASCII_M, ASCII_A, ASCII_L, ASCII_L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_super[32] = {ASCII_S, ASCII_U, ASCII_P, ASCII_E, ASCII_R, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_manual[32] = {ASCII_M, ASCII_A, ASCII_N, ASCII_U, ASCII_A, ASCII_L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_auto[32] = {ASCII_A, ASCII_U, ASCII_T, ASCII_O, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_blackheat[32] = {ASCII_B, ASCII_L, ASCII_A, ASCII_C, ASCII_K, ASCII_H, ASCII_E, ASCII_A, ASCII_T, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_whiteheat[32] = {ASCII_W, ASCII_H, ASCII_I, ASCII_T, ASCII_E, ASCII_H, ASCII_E, ASCII_A, ASCII_T, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_valid[32] = {ASCII_V, ASCII_A, ASCII_L, ASCII_I, ASCII_D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_invalid[32] = {ASCII_I, ASCII_N, ASCII_V, ASCII_A, ASCII_L, ASCII_I, ASCII_D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_close[32] = {ASCII_C, ASCII_L, ASCII_O, ASCII_S, ASCII_E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//unsigned short osd_LMC[32] = {ASCII_L, ASCII_M, ASCII_C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_LMC[32] = {'L', 'M', 'C', 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_start[32] = {ASCII_S, ASCII_T, ASCII_A, ASCII_R, ASCII_T, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_inertia[32] = {ASCII_I, ASCII_N, ASCII_E, ASCII_R, ASCII_T, ASCII_I, ASCII_A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_airsurvey[32] = {ASCII_A, ASCII_I, ASCII_R, ASCII_S, ASCII_U, ASCII_R, ASCII_V, ASCII_E, ASCII_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_callback[32] = {ASCII_C, ASCII_A, ASCII_L, ASCII_L, ASCII_B, ASCII_A, ASCII_C, ASCII_K, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_scan[32] = {ASCII_S, ASCII_C, ASCII_A, ASCII_N, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_lock[32] = {ASCII_L, ASCII_O, ASCII_C, ASCII_K, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_track[32] = {ASCII_T, ASCII_R, ASCII_A, ASCII_C, ASCII_K, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned short osd_tracksearch[32]  =       {ASCII_T, ASCII_R, ASCII_A, ASCII_C, ASCII_K, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_search[32]       =       {ASCII_T, ASCII_R, ASCII_A, ASCII_C, ASCII_K, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_lockcurrent[32]  =       {ASCII_L, ASCII_O, ASCII_C, ASCII_K, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_geotrack[32]     =       {ASCII_G, ASCII_E, ASCII_O, ASCII_T, ASCII_R, ASCII_A, ASCII_C, ASCII_K,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

unsigned short osd_yaw[32] = {ASCII_Y, ASCII_A, ASCII_W, ':', ' ', '0', '0', '0', '.', '0', '0', DEGREE_SYMBOL_ASCII, ' ', ' ', '0', '0', '0', '.', '0', '0', DEGREE_SYMBOL_ASCII, '/', 's', 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_pitch[32] = {ASCII_P, ASCII_I, ASCII_T, ASCII_C, ASCII_H, ':', ' ', '0', '0', '0', '.', '0', '0', DEGREE_SYMBOL_ASCII, ' ', ' ', '0', '0', '0', '.', '0', '0', DEGREE_SYMBOL_ASCII, '/', 's', 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_shine[32] = {ASCII_S, ASCII_H, ASCII_I, ASCII_N, ASCII_E, ':', ' ', '0', '0', '0', 's', ' ', ' ', '0', '0', '0', '0', '0', 'm', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_stop[32] = {ASCII_S, ASCII_T, ASCII_O, ASCII_P, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_irclose[32] = {ASCII_I, ASCII_R, ASCII_C, ASCII_L, ASCII_O, ASCII_S, ASCII_E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_iropen[32] = {ASCII_I, ASCII_R, ASCII_O, ASCII_P, ASCII_E, ASCII_N, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_laserclose[32] = {ASCII_L, ASCII_A, ASCII_S, ASCII_E, ASCII_R, ASCII_C, ASCII_L, ASCII_O, ASCII_S, ASCII_E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_laseropen[32] = {ASCII_L, ASCII_A, ASCII_S, ASCII_E, ASCII_R, ASCII_O, ASCII_P, ASCII_E, ASCII_N, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_imageenhance[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_movingtarget[32] = {ASCII_M, ASCII_O, ASCII_V, ASCII_I, ASCII_N, ASCII_G, ASCII_T, ASCII_A, ASCII_R, ASCII_G, ASCII_E, ASCII_T, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_datacollect[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_shinecode[32] = {ASCII_S, ASCII_H, ASCII_I, ASCII_N, ASCII_E, ASCII_C, ASCII_O, ASCII_D, ASCII_E, ':', '0', '0', '0', '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_shinetime[32] = {ASCII_S, ASCII_H, ASCII_I, ASCII_N, ASCII_E, ASCII_T, ASCII_I, ASCII_M, ASCII_E, ':', '0', '0', 's', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_laserprohibit[32] = {ASCII_L, ASCII_A, ASCII_S, ASCII_E, ASCII_R, ASCII_P, ASCII_R, ASCII_O, ASCII_H, ASCII_I, ASCII_B, ASCII_I, ASCII_T, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_temperaturealarm[32] = {ASCII_T, ASCII_E, ASCII_M, ASCII_P, ASCII_E, ASCII_R, ASCII_A, ASCII_T, ASCII_U, ASCII_R, ASCII_E, ASCII_A, ASCII_L, ASCII_A, ASCII_R, ASCII_M, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_deorbitingtime[32] = {ASCII_D, ASCII_E, ASCII_O, ASCII_R, ASCII_B, ASCII_I, ASCII_T, ASCII_I, ASCII_N, ASCII_G, ASCII_T, ASCII_I, ASCII_M, ASCII_E, ':', ' ', ' ', ' ', ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned short osd_faultcode[32] = {ASCII_F, ASCII_A, ASCII_U, ASCII_L, ASCII_T, ASCII_C, ASCII_O, ASCII_D, ASCII_E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


unsigned short osd_laser_work_mode[32]       =      {CH_idx_ce_210_1, CH_idx_ce_211_2, CH_idx_ju_584_1, CH_idx_ju_585_2, ':', ' ', '0','0','0','s',' ',' ','0','0','0','0','0','m',0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_laser_work_mode_stop[32]  =      {CH_idx_ji_506_1,CH_idx_ji_507_2,CH_idx_guang_432_1,CH_idx_guang_433_2,CH_idx_zhun_1216_1,CH_idx_zhun_1217_2,CH_idx_bei_148_1,CH_idx_bei_149_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_date[32]         =       {'0', '0', '-', '0', '0','-','0','0','0','0',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_time[32]         =       {'0', '0', ':', '0', '0',':','0','0',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

unsigned short osd_faultcode_HEX[32]         =      {'0', '0', 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
//閻╊噣鏁撻弬銈嗗娴ｅ秹鏁撻惌顐ゅ皑閹风兘鏁撻幋顏冪串閹疯渹缍呴柨鐔告灮閹凤拷
unsigned short osd_object_location[32]       =      {ASCII_T, ASCII_A, ASCII_R, ASCII_G, ASCII_E, ASCII_T, ':', 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_object_longitude[32]      =      {ASCII_L, ASCII_O, ASCII_N,':', '0', '0', '0', '.', '0', '0', '0', '0', 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_object_latitude[32]       =      {ASCII_L, ASCII_A, ASCII_T,':',  '0',  '0', '0', '.', '0', '0', '0', '0', 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_object_altitude[32]       =      {ASCII_A, ASCII_L, ASCII_T,':', '0', '0', '0', '.', '0', 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_aircraft_location[32]     =      {ASCII_P, ASCII_L, ASCII_A, ASCII_N, ASCII_E,':',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_aircraft_longitude[32]    =      {ASCII_L, ASCII_O, ASCII_N,':', '0', '0', '0', '.', '0', '0', '0', '0', 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_aircraft_latitude[32]     =      {ASCII_L, ASCII_A, ASCII_T,':',  '0',  '0', '0', '.', '0', '0', '0', '0', 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_aircraft_altitude[32]     =      {ASCII_A, ASCII_L, ASCII_T,':', '0', '0', '0', '.', '0', 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

unsigned short osd_system_info[32]           =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_system_init_info[32]      =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_north_angle[32]           =      {'N', ' ', 0, '0', '0','0','.','0','0',DEGREE_SYMBOL_ASCII,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
//闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻敓锟�
unsigned short osd_ai_detect[32]     		 =      {CH_idx_duo_342_1, CH_idx_duo_343_2, CH_idx_mu_702_1, CH_idx_mu_703_2, CH_idx_biao_170_1, CH_idx_biao_171_2, CH_idx_jian_532_1, CH_idx_jian_533_2, CH_idx_ce_210_1, CH_idx_ce_211_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_moving_detect[32] 		 =      {CH_idx_dong_326_1, CH_idx_dong_327_2, CH_idx_mu_702_1, CH_idx_mu_703_2, CH_idx_biao_170_1, CH_idx_biao_171_2, CH_idx_jian_532_1, CH_idx_jian_533_2, CH_idx_ce_210_1, CH_idx_ce_211_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_single_photo[32]          =      {CH_idx_dan_282_1, CH_idx_dan_283_2, CH_idx_ci_262_1, CH_idx_ci_263_2, CH_idx_pai_1284_1, CH_idx_pai_1285_2, CH_idx_zhao_1154_1, CH_idx_zhao_1155_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_multi_photo[32]           =      {CH_idx_lian_638_1, CH_idx_lian_639_2, CH_idx_xu_1036_1, CH_idx_xu_1037_2, CH_idx_pai_1284_1, CH_idx_pai_1285_2, CH_idx_zhao_1154_1, CH_idx_zhao_1155_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_pos_22_info[32]     		 =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

// unsigned short osd_pos_31_info[32]     		 =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_version_info[32]          =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_pos_39_info[32]           =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

unsigned short sys_mode_init[32] 		= {CH_idx_chu_244_1,CH_idx_chu_245_2,CH_idx_shi_832_1,CH_idx_shi_833_2,CH_idx_hua_476_1,CH_idx_hua_477_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//front view
unsigned short sys_mode_init_vcp[32] 	= {CH_idx_chu_244_1,CH_idx_chu_245_2,CH_idx_shi_832_1,CH_idx_shi_833_2,CH_idx_hua_476_1,CH_idx_hua_477_2,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//front view

unsigned short sys_mode_stow[32]		= {CH_idx_shou_846_1,CH_idx_shou_847_2,CH_idx_cang_202_1,CH_idx_cang_203_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};;//collect
unsigned short sys_mode_stow_vcp[32]	= {CH_idx_shou_846_1,CH_idx_shou_847_2,CH_idx_cang_202_1,CH_idx_cang_203_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};;//collect
//
unsigned short sys_mode_cage1[32] 		= {CH_idx_qian_750_1,CH_idx_qian_751_2,CH_idx_shi_840_1,CH_idx_shi_841_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//front view
unsigned short sys_mode_cage1_vcp[32] 	= {CH_idx_qian_750_1,CH_idx_qian_751_2,CH_idx_shi_840_1,CH_idx_shi_841_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//front view

unsigned short sys_mode_mannul[32] 		= {CH_idx_shou_848_1,CH_idx_shou_849_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_mannul_vcp[32] 	= {CH_idx_shou_848_1,CH_idx_shou_849_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_scan1[32] 		= {CH_idx_sao_802_1,CH_idx_sao_803_2,CH_idx_miao_686_1,CH_idx_miao_687_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_scan1_vcp[32] 	= {CH_idx_sao_802_1,CH_idx_sao_803_2,CH_idx_miao_686_1,CH_idx_miao_687_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_track[32] 		= {CH_idx_gen_406_1,CH_idx_gen_407_2,CH_idx_zong_1224_1,CH_idx_zong_1225_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_track_vcp[32] 	= {CH_idx_gen_406_1,CH_idx_gen_407_2,CH_idx_zong_1224_1,CH_idx_zong_1225_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//geo location track
unsigned short sys_mode_GPCtrack[32] 		= {CH_idx_di_298_1,CH_idx_di_299_2,CH_idx_li_632_1,CH_idx_li_633_2,CH_idx_gen_406_1,CH_idx_gen_407_2,CH_idx_zong_1224_1,CH_idx_zong_1225_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_GPCtrack_vcp[32] 	= {CH_idx_di_298_1,CH_idx_di_299_2,CH_idx_li_632_1,CH_idx_li_633_2,CH_idx_gen_406_1,CH_idx_gen_407_2,CH_idx_zong_1224_1,CH_idx_zong_1225_2,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//angle slave
unsigned short sys_mode_slave1[32]			= {CH_idx_jiao_552_1,CH_idx_jiao_553_2,CH_idx_du_332_1,CH_idx_du_333_2,CH_idx_sui_882_1,CH_idx_sui_883_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_slave1_vcp[32]		= {CH_idx_jiao_552_1,CH_idx_jiao_553_2,CH_idx_du_332_1,CH_idx_du_333_2,CH_idx_sui_882_1,CH_idx_sui_883_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_service[32] 		= {CH_idx_wei_934_1,CH_idx_wei_935_2,CH_idx_hu_472_1,CH_idx_hu_473_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_service_vcp[32] 	= {CH_idx_wei_934_1,CH_idx_wei_935_2,CH_idx_hu_472_1,CH_idx_hu_473_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_scan2[32]			= {CH_idx_bu_190_1,CH_idx_bu_191_2,CH_idx_jin_568_1,CH_idx_jin_569_2,CH_idx_sao_802_1,CH_idx_sao_803_2,CH_idx_miao_686_1,CH_idx_miao_687_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_scan2_vcp[32]		= {CH_idx_bu_190_1,CH_idx_bu_191_2,CH_idx_jin_568_1,CH_idx_jin_569_2,CH_idx_sao_802_1,CH_idx_sao_803_2,CH_idx_miao_686_1,CH_idx_miao_687_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_slave2[32]			= {CH_idx_wei_946_1,CH_idx_wei_947_2,CH_idx_zhi_1190_1,CH_idx_zhi_1191_2,CH_idx_sui_882_1,CH_idx_sui_883_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_slave2_vcp[32]		= {CH_idx_wei_946_1,CH_idx_wei_947_2,CH_idx_zhi_1190_1,CH_idx_zhi_1191_2,CH_idx_sui_882_1,CH_idx_sui_883_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_cage2[32] 		= {CH_idx_suo_886_1,CH_idx_suo_887_2,CH_idx_ding_320_1,CH_idx_ding_321_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_cage2_vcp[32] 	= {CH_idx_suo_886_1,CH_idx_suo_887_2,CH_idx_ding_320_1,CH_idx_ding_321_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#else
unsigned short osd_blank[32]        =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_sensor_viewangle_enhance_viewstate_bricont_ir[32]     =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_inertia_velocitycompensation_workmode[32]             =      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','L','M','C',' ',' ',' ',CH_idx_shou_848_1,CH_idx_shou_849_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_tv[32]           =       {CH_idx_dian_312_1, CH_idx_dian_313_2, CH_idx_shi_840_1, CH_idx_shi_841_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_ir[32]           =       {CH_idx_hong_468_1, CH_idx_hong_469_2, CH_idx_wai_926_1, CH_idx_wai_927_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_light[32]        =       {CH_idx_qing_1250_1, CH_idx_qing_1251_2, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_medium[32]       =       {CH_idx_zhong_1192_1, CH_idx_zhong_1193_2, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_heavy[32]        =       {CH_idx_zhong_1198_1, CH_idx_zhong_1199_2, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_big[32]          =       {CH_idx_da_276_1, CH_idx_da_277_2, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_small[32]        =       {CH_idx_xiao_1002_1, CH_idx_xiao_1003_2, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_super[32]        =       {CH_idx_chao_228_1, CH_idx_chao_229_2, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_manual[32]       =       {CH_idx_shou_848_1, CH_idx_shou_849_2, CH_idx_dong_326_1, CH_idx_dong_327_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_auto[32]         =       {CH_idx_zi_1220_1, CH_idx_zi_1221_2, CH_idx_dong_326_1, CH_idx_dong_327_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_blackheat[32]    =       {CH_idx_hei_464_1, CH_idx_hei_465_2, CH_idx_re_778_1, CH_idx_re_779_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_whiteheat[32]    =       {CH_idx_bai_132_1, CH_idx_bai_133_2, CH_idx_re_778_1, CH_idx_re_779_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_valid[32]        =       {CH_idx_you_1100_1, CH_idx_you_1101_2, CH_idx_xiao_1006_1, CH_idx_xiao_1007_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_invalid[32]      =       {CH_idx_wu_954_1, CH_idx_wu_955_2, CH_idx_xiao_1006_1, CH_idx_xiao_1007_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_close[32]        =       {CH_idx_guan_426_1, CH_idx_guan_427_2, CH_idx_bi_158_1, CH_idx_bi_159_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_LMC[32]          =       {'L', 'M', 'C', 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };    //guan bi
unsigned short osd_start[32]        =       {CH_idx_qi_742_1, CH_idx_qi_743_2, CH_idx_dong_326_1, CH_idx_dong_327_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_inertia[32]      =       {CH_idx_guan_430_1, CH_idx_guan_431_2, CH_idx_xing_1026_1, CH_idx_xing_1027_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_airsurvey[32]    =       {CH_idx_hang_450_1, CH_idx_hang_451_2, CH_idx_ce_210_1, CH_idx_ce_211_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_callback[32]     =       {CH_idx_hui_490_1, CH_idx_hui_491_2, CH_idx_shou_846_1, CH_idx_shou_847_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_scan[32]         =       {CH_idx_sao_802_1, CH_idx_sao_803_2, CH_idx_miao_686_1, CH_idx_miao_687_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_lock[32]         =       {CH_idx_suo_886_1, CH_idx_suo_887_2, CH_idx_ding_320_1, CH_idx_ding_321_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_track[32]        =       {CH_idx_gen_406_1, CH_idx_gen_407_2, CH_idx_zong_1224_1, CH_idx_zong_1225_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_tracksearch[32]  =       {CH_idx_gen_406_1, CH_idx_gen_407_2, CH_idx_sou_1252_1, CH_idx_sou_1253_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_search[32]       =       {CH_idx_gen_406_1, CH_idx_gen_407_2, CH_idx_zong_1224_1, CH_idx_zong_1225_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_lockcurrent[32]  =       {CH_idx_suo_886_1, CH_idx_suo_887_2, CH_idx_ding_320_1, CH_idx_ding_321_2, CH_idx_dang_286_1, CH_idx_dang_287_2, CH_idx_qian_750_1, CH_idx_qian_751_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_geotrack[32]     =       {CH_idx_di_298_1, CH_idx_di_299_2, CH_idx_li_632_1, CH_idx_li_633_2, CH_idx_gen_406_1, CH_idx_gen_407_2, CH_idx_zong_1224_1, CH_idx_zong_1225_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_yaw[32]          =       {CH_idx_fang_352_1, CH_idx_fang_353_2, CH_idx_wei_946_1, CH_idx_wei_947_2, ':',' ', '0', '0','0','.','0','0',DEGREE_SYMBOL_ASCII,' ', ' ', '0','0','0','.','0','0', DEGREE_SYMBOL_ASCII, '/','s',0,0,0,0,0,0,0,0 };
unsigned short osd_pitch[32]        =       {CH_idx_fu_370_1, CH_idx_fu_371_2, CH_idx_yang_1054_1, CH_idx_yang_1055_2, ':',' ', '0', '0','0','.','0','0',DEGREE_SYMBOL_ASCII,' ', ' ', '0','0','0','.','0','0', DEGREE_SYMBOL_ASCII, '/','s',0,0,0,0,0,0,0,0 };
unsigned short osd_laser_work_mode[32]       =      {CH_idx_ce_210_1, CH_idx_ce_211_2, CH_idx_ju_584_1, CH_idx_ju_585_2, ':', ' ', '0','0','0','s',' ',' ','0','0','0','0','0','m',0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
// unsigned short osd_laser_work_mode_stop[32]  =      {CH_idx_ting_908_1,CH_idx_ting_909_2,CH_idx_zhi_1178_1,CH_idx_zhi_1179_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_laser_work_mode_stop[32]  =      {CH_idx_ji_506_1,CH_idx_ji_507_2,CH_idx_guang_432_1,CH_idx_guang_433_2,CH_idx_zhun_1216_1,CH_idx_zhun_1217_2,CH_idx_bei_148_1,CH_idx_bei_149_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_shine[32]        =       {CH_idx_zhao_1154_1, CH_idx_zhao_1155_2, CH_idx_she_814_1, CH_idx_she_815_2, ':', ' ', '0','0','0','s',' ',' ','0','0','0','0','0','m',0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_stop[32]         =       {CH_idx_ting_908_1, CH_idx_ting_909_2, CH_idx_zhi_1178_1, CH_idx_zhi_1179_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_date[32]         =       {'0', '0', '-', '0', '0','-','0','0','0','0',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_time[32]         =       {'0', '0', ':', '0', '0',':','0','0',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_irclose[32]      =       {CH_idx_hong_468_1, CH_idx_hong_469_2, CH_idx_wai_926_1, CH_idx_wai_927_2, CH_idx_guan_426_1, CH_idx_guan_427_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_iropen[32]       =       {CH_idx_hong_468_1, CH_idx_hong_469_2, CH_idx_wai_926_1, CH_idx_wai_927_2, CH_idx_kai_588_1, CH_idx_kai_589_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_laserclose[32]   =       {CH_idx_ji_506_1, CH_idx_ji_507_2, CH_idx_guang_432_1, CH_idx_guang_433_2, CH_idx_guan_426_1, CH_idx_guan_427_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_laseropen[32]    =       {CH_idx_ji_506_1, CH_idx_ji_507_2, CH_idx_guang_432_1, CH_idx_guang_433_2, CH_idx_kai_588_1, CH_idx_kai_589_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_imageenhance[32] =       {CH_idx_tu_918_1, CH_idx_tu_919_2, CH_idx_xiang_998_1, CH_idx_xiang_999_2, CH_idx_ban_136_1, CH_idx_ban_137_2, CH_idx_zeng_1144_1, CH_idx_zeng_1145_2, CH_idx_qiang_754_1, CH_idx_qiang_755_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_movingtarget[32] =       {CH_idx_dong_326_1, CH_idx_dong_327_2, CH_idx_mu_702_1, CH_idx_mu_703_2, CH_idx_biao_170_1, CH_idx_biao_171_2, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_datacollect[32]  =       {CH_idx_shu_860_1, CH_idx_shu_861_2, CH_idx_ju_582_1, CH_idx_ju_583_2, CH_idx_cai_194_1, CH_idx_cai_195_2, CH_idx_ji_1242_1, CH_idx_ji_1243_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_shinecode[32]    =       {CH_idx_zhao_1154_1, CH_idx_zhao_1155_2, CH_idx_she_814_1, CH_idx_she_815_2, CH_idx_bian_164_1, CH_idx_bian_165_2, CH_idx_ma_674_1, CH_idx_ma_675_2, ':', '0','0','0','0',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_shinetime[32]    =       {CH_idx_zhao_1154_1, CH_idx_zhao_1155_2, CH_idx_she_814_1, CH_idx_she_815_2, CH_idx_shi_824_1, CH_idx_shi_825_2, CH_idx_jian_530_1, CH_idx_jian_531_2, ':', '0','0','s',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_laserprohibit[32]         =      {CH_idx_ji_506_1, CH_idx_ji_507_2, CH_idx_guang_432_1, CH_idx_guang_433_2, CH_idx_jin_572_1, CH_idx_jin_573_2, CH_idx_zhi_1178_1, CH_idx_zhi_1179_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short osd_temperaturealarm[32]      =      {CH_idx_wen_948_1, CH_idx_wen_949_2, CH_idx_du_332_1, CH_idx_du_333_2, CH_idx_gao_392_1, CH_idx_gao_393_2, CH_idx_jing_1244_1, CH_idx_jing_1245_2, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short osd_deorbitingtime[32]        =      {CH_idx_li_630_1, CH_idx_li_631_2, CH_idx_gui_1240_1, CH_idx_gui_1241_2, CH_idx_shi_824_1, CH_idx_shi_825_2, CH_idx_jian_530_1, CH_idx_jian_531_2, ':',' ',' ',' ',' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_faultcode[32]             =      {CH_idx_gu_424_1, CH_idx_gu_425_2, CH_idx_zhang_1152_1, CH_idx_zhang_1153_2, CH_idx_ma_674_1, CH_idx_ma_675_2, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_faultcode_HEX[32]         =      {'0', '0', 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_object_location[32]       =      {CH_idx_mu_702_1, CH_idx_mu_703_2, CH_idx_biao_170_1, CH_idx_biao_171_2, CH_idx_wei_946_1, CH_idx_wei_947_2, CH_idx_zhi_1190_1, CH_idx_zhi_1191_2, ':',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_object_longitude[32]      =      {CH_idx_jing_574_1, CH_idx_jing_575_2, CH_idx_du_332_1, CH_idx_du_333_2, ':', '0', '0', '0', '.', '0', '0', '0', '0', '0', '0', '0',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_object_latitude[32]       =      {CH_idx_wei_938_1, CH_idx_wei_939_2, CH_idx_du_332_1, CH_idx_du_333_2, ':',  '0',  '0', '0', '.', '0', '0', '0', '0', '0', '0', '0',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_object_altitude[32]       =      {CH_idx_gao_390_1, CH_idx_gao_391_2, CH_idx_du_332_1, CH_idx_du_333_2, ':', '0', '0', '0', '0', '0', '0', 'm', 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_aircraft_location[32]     =      {CH_idx_zai_1136_1, CH_idx_zai_1137_2, CH_idx_ji_502_1, CH_idx_ji_503_2, CH_idx_wei_946_1, CH_idx_wei_947_2, CH_idx_zhi_1190_1, CH_idx_zhi_1191_2, ':',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_aircraft_longitude[32]    =      {CH_idx_jing_574_1, CH_idx_jing_575_2, CH_idx_du_332_1, CH_idx_du_333_2, ':', '0', '0', '0', '.', '0', '0', '0', '0', '0', '0', '0',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_aircraft_latitude[32]     =      {CH_idx_wei_938_1, CH_idx_wei_939_2, CH_idx_du_332_1, CH_idx_du_333_2, ':', '0',  '0', '0', '.', '0', '0', '0', '0', '0', '0', '0',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_aircraft_altitude[32]     =      {CH_idx_gao_390_1, CH_idx_gao_391_2, CH_idx_du_332_1, CH_idx_du_333_2, ':', '0', '0', '0', '0', '0', '0', 'm', 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_system_info[32]           =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_system_init_info[32]      =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_north_angle[32]           =      {'N', ' ', 0, '0', '0','0','.','0','0',DEGREE_SYMBOL_ASCII,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_ai_detect[32]     		 =      {CH_idx_duo_342_1, CH_idx_duo_343_2, CH_idx_mu_702_1, CH_idx_mu_703_2, CH_idx_biao_170_1, CH_idx_biao_171_2, CH_idx_jian_532_1, CH_idx_jian_533_2, CH_idx_ce_210_1, CH_idx_ce_211_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_moving_detect[32] 		 =      {CH_idx_dong_326_1, CH_idx_dong_327_2, CH_idx_mu_702_1, CH_idx_mu_703_2, CH_idx_biao_170_1, CH_idx_biao_171_2, CH_idx_jian_532_1, CH_idx_jian_533_2, CH_idx_ce_210_1, CH_idx_ce_211_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_single_photo[32]          =      {CH_idx_dan_282_1, CH_idx_dan_283_2, CH_idx_ci_262_1, CH_idx_ci_263_2, CH_idx_pai_1284_1, CH_idx_pai_1285_2, CH_idx_zhao_1154_1, CH_idx_zhao_1155_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_multi_photo[32]           =      {CH_idx_lian_638_1, CH_idx_lian_639_2, CH_idx_xu_1036_1, CH_idx_xu_1037_2, CH_idx_pai_1284_1, CH_idx_pai_1285_2, CH_idx_zhao_1154_1, CH_idx_zhao_1155_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_pos_22_info[32]     		 =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
// unsigned short osd_pos_31_info[32]     		 =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_version_info[32]          =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned short osd_pos_39_info[32]           =      {0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

unsigned short sys_mode_init[32] 		= {CH_idx_chu_244_1,CH_idx_chu_245_2,CH_idx_shi_832_1,CH_idx_shi_833_2,CH_idx_hua_476_1,CH_idx_hua_477_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//front view
unsigned short sys_mode_init_vcp[32] 	= {CH_idx_chu_244_1,CH_idx_chu_245_2,CH_idx_shi_832_1,CH_idx_shi_833_2,CH_idx_hua_476_1,CH_idx_hua_477_2,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//front view

unsigned short sys_mode_stow[32]		= {CH_idx_shou_846_1,CH_idx_shou_847_2,CH_idx_cang_202_1,CH_idx_cang_203_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};;//collect
unsigned short sys_mode_stow_vcp[32]	= {CH_idx_shou_846_1,CH_idx_shou_847_2,CH_idx_cang_202_1,CH_idx_cang_203_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};;//collect
//
unsigned short sys_mode_cage1[32] 		= {CH_idx_qian_750_1,CH_idx_qian_751_2,CH_idx_shi_840_1,CH_idx_shi_841_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//front view
unsigned short sys_mode_cage1_vcp[32] 	= {CH_idx_qian_750_1,CH_idx_qian_751_2,CH_idx_shi_840_1,CH_idx_shi_841_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//front view

unsigned short sys_mode_mannul[32] 		= {CH_idx_shou_848_1,CH_idx_shou_849_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_mannul_vcp[32] 	= {CH_idx_shou_848_1,CH_idx_shou_849_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_scan1[32] 		= {CH_idx_sao_802_1,CH_idx_sao_803_2,CH_idx_miao_686_1,CH_idx_miao_687_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_scan1_vcp[32] 	= {CH_idx_sao_802_1,CH_idx_sao_803_2,CH_idx_miao_686_1,CH_idx_miao_687_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_track[32] 		= {CH_idx_gen_406_1,CH_idx_gen_407_2,CH_idx_zong_1224_1,CH_idx_zong_1225_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_track_vcp[32] 	= {CH_idx_gen_406_1,CH_idx_gen_407_2,CH_idx_zong_1224_1,CH_idx_zong_1225_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//geo location track
unsigned short sys_mode_GPCtrack[32] 		= {CH_idx_di_298_1,CH_idx_di_299_2,CH_idx_li_632_1,CH_idx_li_633_2,CH_idx_gen_406_1,CH_idx_gen_407_2,CH_idx_zong_1224_1,CH_idx_zong_1225_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_GPCtrack_vcp[32] 	= {CH_idx_di_298_1,CH_idx_di_299_2,CH_idx_li_632_1,CH_idx_li_633_2,CH_idx_gen_406_1,CH_idx_gen_407_2,CH_idx_zong_1224_1,CH_idx_zong_1225_2,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//angle slave
unsigned short sys_mode_slave1[32]			= {CH_idx_jiao_552_1,CH_idx_jiao_553_2,CH_idx_du_332_1,CH_idx_du_333_2,CH_idx_sui_882_1,CH_idx_sui_883_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_slave1_vcp[32]		= {CH_idx_jiao_552_1,CH_idx_jiao_553_2,CH_idx_du_332_1,CH_idx_du_333_2,CH_idx_sui_882_1,CH_idx_sui_883_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_service[32] 		= {CH_idx_wei_934_1,CH_idx_wei_935_2,CH_idx_hu_472_1,CH_idx_hu_473_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_service_vcp[32] 	= {CH_idx_wei_934_1,CH_idx_wei_935_2,CH_idx_hu_472_1,CH_idx_hu_473_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_scan2[32]			= {CH_idx_bu_190_1,CH_idx_bu_191_2,CH_idx_jin_568_1,CH_idx_jin_569_2,CH_idx_sao_802_1,CH_idx_sao_803_2,CH_idx_miao_686_1,CH_idx_miao_687_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_scan2_vcp[32]		= {CH_idx_bu_190_1,CH_idx_bu_191_2,CH_idx_jin_568_1,CH_idx_jin_569_2,CH_idx_sao_802_1,CH_idx_sao_803_2,CH_idx_miao_686_1,CH_idx_miao_687_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_slave2[32]			= {CH_idx_wei_946_1,CH_idx_wei_947_2,CH_idx_zhi_1190_1,CH_idx_zhi_1191_2,CH_idx_sui_882_1,CH_idx_sui_883_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_slave2_vcp[32]		= {CH_idx_wei_946_1,CH_idx_wei_947_2,CH_idx_zhi_1190_1,CH_idx_zhi_1191_2,CH_idx_sui_882_1,CH_idx_sui_883_2,CH_idx_dong_326_1,CH_idx_dong_327_2,0,0,0,0,0  ,0  ,0  ,0  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short sys_mode_cage2[32] 		= {CH_idx_suo_886_1,CH_idx_suo_887_2,CH_idx_ding_320_1,CH_idx_ding_321_2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short sys_mode_cage2_vcp[32] 	= {CH_idx_suo_886_1,CH_idx_suo_887_2,CH_idx_ding_320_1,CH_idx_ding_321_2,0,0,0,0,0,0,0,0,'+','V','C','P',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif
std::map<unsigned char, unsigned short int*> map_sys_mode ;
std::map<unsigned char, unsigned short int*> map_sys_mode_VCP ;

void osd_init_chinese()
{
	//闁跨喕绶濈喊澶嬪闁跨喓娈曠涵閿嬪闁跨喐鏋婚幏椋庛仛
	map_sys_mode[0x01] = sys_mode_init;
	map_sys_mode[0x02] = sys_mode_stow;
	map_sys_mode[0x03] = sys_mode_cage1;
	map_sys_mode[0x04] = sys_mode_mannul;
	map_sys_mode[0x05] = sys_mode_scan1;
	map_sys_mode[0x06] = sys_mode_track;
	map_sys_mode[0x07] = sys_mode_GPCtrack;
	map_sys_mode[0x08] = sys_mode_slave1;
	map_sys_mode[0x09] = sys_mode_service;
	map_sys_mode[0x0a] = sys_mode_scan2;
	map_sys_mode[0x0b] = sys_mode_slave2;
	map_sys_mode[0x0c] = sys_mode_cage2;

	map_sys_mode_VCP[0x01] = sys_mode_init_vcp;
	map_sys_mode_VCP[0x02] = sys_mode_stow_vcp;
	map_sys_mode_VCP[0x03] = sys_mode_cage1_vcp;
	map_sys_mode_VCP[0x04] = sys_mode_mannul_vcp;
	map_sys_mode_VCP[0x05] = sys_mode_scan1_vcp;
	map_sys_mode_VCP[0x06] = sys_mode_track_vcp;
	map_sys_mode_VCP[0x07] = sys_mode_GPCtrack_vcp;
	map_sys_mode_VCP[0x08] = sys_mode_slave1_vcp;
	map_sys_mode_VCP[0x09] = sys_mode_service_vcp;
	map_sys_mode_VCP[0x0a] = sys_mode_scan2_vcp;
	map_sys_mode_VCP[0x0b] = sys_mode_slave2_vcp;
	map_sys_mode_VCP[0x0c] = sys_mode_cage2_vcp;

	memset( (unsigned char*)OSD_BRAM_HANDLE , 0,  OSD_BRAM_SPACE_BYTE);

	//zyw add
	// 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
	osd_pos_date.osd_idx = 1;
	osd_pos_date.config.para.Enable = 1;
	osd_pos_date.config.para.valid_length = 0;
	osd_pos_date.config.para.ver_y = 32;
	osd_pos_date.config.para.hor_x = 96;
	osd_pos_date.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_date, OSD_BRAM_HANDLE);

	// 閺冨爼鏁撻弬銈嗗
	osd_pos_time.osd_idx = 2;
	osd_pos_time.config.para.Enable = 1;
	osd_pos_time.config.para.valid_length = 0;
	osd_pos_time.config.para.ver_y = 64;
	osd_pos_time.config.para.hor_x = 96;
	osd_pos_time.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_time, OSD_BRAM_HANDLE);

    osd_pos_vcp.osd_idx = 3;
    osd_pos_vcp.config.para.Enable = 1;
    osd_pos_vcp.config.para.valid_length = 0;
    osd_pos_vcp.config.para.ver_y = 160;
    osd_pos_vcp.config.para.hor_x = 96;
    osd_pos_vcp.str_arr = osd_blank;//chu shi yc bu yc xian shi osd_vcp
    update_OSD_chinese(osd_pos_vcp, OSD_BRAM_HANDLE);

	// TV 閻樿埖锟斤拷
	osd_pos_tv.osd_idx = 4;
	osd_pos_tv.config.para.Enable = 1;
	osd_pos_tv.config.para.valid_length = 0;
	osd_pos_tv.config.para.ver_y = 224;
	osd_pos_tv.config.para.hor_x = 96;
	osd_pos_tv.str_arr = osd_tv;//osd_tv
	update_OSD_chinese(osd_pos_tv, OSD_BRAM_HANDLE);

	// IR 閻樿埖锟斤拷
	osd_pos_ir.osd_idx = 5;
	osd_pos_ir.config.para.Enable = 1;
	osd_pos_ir.config.para.valid_length = 0;
	osd_pos_ir.config.para.ver_y = 224+32;
	osd_pos_ir.config.para.hor_x = 96;
	osd_pos_ir.str_arr = osd_ir;//osd_ldr
	update_OSD_chinese(osd_pos_ir, OSD_BRAM_HANDLE);

	//LDR閻樿埖锟斤拷
	osd_pos_ldr.osd_idx = 6;
	osd_pos_ldr.config.para.Enable = 1;
	osd_pos_ldr.config.para.valid_length = 0;
	osd_pos_ldr.config.para.ver_y = 224+64;
	osd_pos_ldr.config.para.hor_x = 96;
	osd_pos_ldr.str_arr = osd_ldr;//osd_ldr
	update_OSD_chinese(osd_pos_ldr, OSD_BRAM_HANDLE);

	// PIC EN(闁跨喐鏋婚幏鐑芥晸閺傘倖瀚� )
	osd_pos_image_enhancement.osd_idx = 7;
	osd_pos_image_enhancement.config.para.Enable = 1;
	osd_pos_image_enhancement.config.para.valid_length = 0;
	osd_pos_image_enhancement.config.para.ver_y = 352;
	osd_pos_image_enhancement.config.para.hor_x = 96;
	osd_pos_image_enhancement.str_arr = osd_blank;//osd_imageenhance
	update_OSD_chinese(osd_pos_image_enhancement, OSD_BRAM_HANDLE);

	// AVT
	osd_pos_enhance_avt.osd_idx = 8;
	osd_pos_enhance_avt.config.para.Enable = 1;
	osd_pos_enhance_avt.config.para.valid_length = 0;
	osd_pos_enhance_avt.config.para.ver_y = 352+32;
	osd_pos_enhance_avt.config.para.hor_x = 96;
	osd_pos_enhance_avt.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_enhance_avt, OSD_BRAM_HANDLE);

	//ehhance tv
	osd_pos_enhance_tv.osd_idx = 9;
	osd_pos_enhance_tv.config.para.Enable = 1;
	osd_pos_enhance_tv.config.para.valid_length = 0;
	osd_pos_enhance_tv.config.para.ver_y = 352+64;
	osd_pos_enhance_tv.config.para.hor_x = 96;
	osd_pos_enhance_tv.str_arr = osd_blank;//osd_imageenhance
	update_OSD_chinese(osd_pos_enhance_tv, OSD_BRAM_HANDLE);

	//ehhance ir
	osd_pos_enhance_ir.osd_idx = 10;
	osd_pos_enhance_ir.config.para.Enable = 1;
	osd_pos_enhance_ir.config.para.valid_length = 0;
	osd_pos_enhance_ir.config.para.ver_y = 352+96;//
	osd_pos_enhance_ir.config.para.hor_x = 96;
	osd_pos_enhance_ir.str_arr = osd_blank;//osd_imageenhance
	update_OSD_chinese(osd_pos_enhance_ir, OSD_BRAM_HANDLE);


	// FOV
	osd_pos_fov.osd_idx = 11;
	osd_pos_fov.config.para.Enable = 1;
	osd_pos_fov.config.para.valid_length = 0;
	osd_pos_fov.config.para.ver_y = 760;
	osd_pos_fov.config.para.hor_x = 96;
	osd_pos_fov.str_arr = osd_fov;
	update_OSD_chinese(osd_pos_fov, OSD_BRAM_HANDLE);

    //zsnmw
    osd_pos_z.osd_idx = 12;
    osd_pos_z.config.para.Enable = 1;
    osd_pos_z.config.para.valid_length = 0;
    osd_pos_z.config.para.ver_y = 800;
    osd_pos_z.config.para.hor_x = 96;
    osd_pos_z.str_arr = osd_z;
    update_OSD_chinese(osd_pos_z, OSD_BRAM_HANDLE);

    osd_pos_s.osd_idx =13;
    osd_pos_s.config.para.Enable = 1;
    osd_pos_s.config.para.valid_length = 0;
    osd_pos_s.config.para.ver_y = 800+32;
    osd_pos_s.config.para.hor_x = 96;
    osd_pos_s.str_arr = osd_s;
    update_OSD_chinese(osd_pos_s, OSD_BRAM_HANDLE);

    osd_pos_n.osd_idx =14;
    osd_pos_n.config.para.Enable = 1;
    osd_pos_n.config.para.valid_length = 0;
    osd_pos_n.config.para.ver_y = 800+32*2;
    osd_pos_n.config.para.hor_x = 96;
    osd_pos_n.str_arr = osd_n;
    update_OSD_chinese(osd_pos_n, OSD_BRAM_HANDLE);

    osd_pos_m.osd_idx = 15;
    osd_pos_m.config.para.Enable = 1;
    osd_pos_m.config.para.valid_length = 0;
    osd_pos_m.config.para.ver_y = 800+32*3;
    osd_pos_m.config.para.hor_x = 96;
    osd_pos_m.str_arr = osd_m;
    update_OSD_chinese(osd_pos_m, OSD_BRAM_HANDLE);

    osd_pos_w.osd_idx = 16;
    osd_pos_w.config.para.Enable = 1;
    osd_pos_w.config.para.valid_length = 0;
    osd_pos_w.config.para.ver_y = 800+32*4;
    osd_pos_w.config.para.hor_x = 96;
    osd_pos_w.str_arr = osd_w;
    update_OSD_chinese(osd_pos_w, OSD_BRAM_HANDLE);

	// target
	// 19: 1
	osd_pos_object_position.osd_idx = 17;//16
	osd_pos_object_position.config.para.Enable = 1;
	osd_pos_object_position.config.para.valid_length = 0;
	osd_pos_object_position.config.para.ver_y = 50+92+40;
	osd_pos_object_position.config.para.hor_x = 1712;//fyc
	osd_pos_object_position.str_arr = osd_object_location;
	update_OSD_chinese(osd_pos_object_position, OSD_BRAM_HANDLE);

	// 19: 2
	osd_pos_object_position_longitude.osd_idx = 18;
	osd_pos_object_position_longitude.config.para.Enable = 1;
	osd_pos_object_position_longitude.config.para.valid_length = 0;
	osd_pos_object_position_longitude.config.para.ver_y = 50+92+40+32;
	osd_pos_object_position_longitude.config.para.hor_x = 1680;
	osd_pos_object_position_longitude.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_object_position_longitude, OSD_BRAM_HANDLE);

	// 19: 3
	osd_pos_object_position_latitude.osd_idx = 19;
	osd_pos_object_position_latitude.config.para.Enable = 1;
	osd_pos_object_position_latitude.config.para.valid_length = 0;
	osd_pos_object_position_latitude.config.para.ver_y = 50+92+40+64;
	osd_pos_object_position_latitude.config.para.hor_x = 1680;
	osd_pos_object_position_latitude.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_object_position_latitude, OSD_BRAM_HANDLE);

	// 19: 4
	osd_pos_object_position_altitude.osd_idx = 20;
	osd_pos_object_position_altitude.config.para.Enable = 1;
	osd_pos_object_position_altitude.config.para.valid_length = 0;
	osd_pos_object_position_altitude.config.para.ver_y = 50+92+40+96;
	osd_pos_object_position_altitude.config.para.hor_x = 1680;
	osd_pos_object_position_altitude.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_object_position_altitude, OSD_BRAM_HANDLE);

	//plane
	// 20: 1
	osd_pos_aircraft_position.osd_idx = 21;
	osd_pos_aircraft_position.config.para.Enable = 1;
	osd_pos_aircraft_position.config.para.valid_length = 0;
	osd_pos_aircraft_position.config.para.ver_y = 50;
	osd_pos_aircraft_position.config.para.hor_x = 1712;
	osd_pos_aircraft_position.str_arr = osd_aircraft_location;
	update_OSD_chinese(osd_pos_aircraft_position, OSD_BRAM_HANDLE);

	// 20: 2
	osd_pos_aircraft_position_longitude.osd_idx = 22;
	osd_pos_aircraft_position_longitude.config.para.Enable = 1;
	osd_pos_aircraft_position_longitude.config.para.valid_length = 0;
	osd_pos_aircraft_position_longitude.config.para.ver_y = 50+32;
	osd_pos_aircraft_position_longitude.config.para.hor_x = 1680;
	osd_pos_aircraft_position_longitude.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_aircraft_position_longitude, OSD_BRAM_HANDLE);

	// 20: 3
	osd_pos_aircraft_position_latitude.osd_idx = 23;
	osd_pos_aircraft_position_latitude.config.para.Enable = 1;
	osd_pos_aircraft_position_latitude.config.para.valid_length = 0;
	osd_pos_aircraft_position_latitude.config.para.ver_y = 50+64;
	osd_pos_aircraft_position_latitude.config.para.hor_x = 1680;
	osd_pos_aircraft_position_latitude.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_aircraft_position_latitude, OSD_BRAM_HANDLE);

	// 20: 4
	osd_pos_aircraft_position_altitude.osd_idx = 24;
	osd_pos_aircraft_position_altitude.config.para.Enable = 1;
	osd_pos_aircraft_position_altitude.config.para.valid_length = 0;
	osd_pos_aircraft_position_altitude.config.para.ver_y = 50+92;
	osd_pos_aircraft_position_altitude.config.para.hor_x = 1680;
	osd_pos_aircraft_position_altitude.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_aircraft_position_altitude, OSD_BRAM_HANDLE);

	//origin message
//	osd_pos_arrow.osd_idx = 25;
//	osd_pos_arrow.config.para.Enable = 1;
//	osd_pos_arrow.config.para.valid_length = 0;
//	osd_pos_arrow.config.para.ver_y = 800+32*4;
//	osd_pos_arrow.config.para.hor_x = 96+16*3+1;//
//	osd_pos_arrow.str_arr = osd_arrow;//
//	update_OSD_chinese(osd_pos_arrow, OSD_BRAM_HANDLE);
//	osd_pos_arrow_tail.osd_idx = 29;
//	osd_pos_arrow_tail.config.para.Enable = 1;
//	osd_pos_arrow_tail.config.para.valid_length = 0;
//	osd_pos_arrow_tail.config.para.ver_y = 1000;
//	osd_pos_arrow_tail.config.para.hor_x = 600;
//	osd_pos_arrow_tail.str_arr = osd_blank;
//	update_OSD_chinese(osd_pos_arrow_tail, OSD_BRAM_HANDLE);



	//az
	osd_pos_az.osd_idx = 26;
	osd_pos_az.config.para.Enable = 1;
	osd_pos_az.config.para.valid_length = 0;
	osd_pos_az.config.para.ver_y = 800+32*2;
	osd_pos_az.config.para.hor_x = 1680;
	osd_pos_az.str_arr = osd_az;
	update_OSD_chinese(osd_pos_az, OSD_BRAM_HANDLE);

	//el
	osd_pos_el.osd_idx = 27;
	osd_pos_el.config.para.Enable = 1;
	osd_pos_el.config.para.valid_length = 0;
	osd_pos_el.config.para.ver_y = 800+32*3;
	osd_pos_el.config.para.hor_x = 1680;
	osd_pos_el.str_arr = osd_el;
	update_OSD_chinese(osd_pos_el, OSD_BRAM_HANDLE);

	//rng
	osd_pos_rng.osd_idx = 28;
	osd_pos_rng.config.para.Enable = 1;
	osd_pos_rng.config.para.valid_length = 0;
	osd_pos_rng.config.para.ver_y = 800+32*4;
	osd_pos_rng.config.para.hor_x = 1680;
	osd_pos_rng.str_arr = osd_rng;
	update_OSD_chinese(osd_pos_rng, OSD_BRAM_HANDLE);

	//top char
	osd_pos_top_char.osd_idx = 30;
	osd_pos_top_char.config.para.Enable = 1;
	osd_pos_top_char.config.para.valid_length = 0;
	osd_pos_top_char.config.para.ver_y = 120;
	osd_pos_top_char.config.para.hor_x = 680;
	osd_pos_top_char.str_arr = osd_top_char;
	update_OSD_chinese(osd_pos_top_char, OSD_BRAM_HANDLE);
//    osd_pos_arrow_blank.osd_idx = 0;
//    osd_pos_arrow_blank.config.para.Enable = 1;
//    osd_pos_arrow_blank.config.para.valid_length = 0;
//    osd_pos_arrow_blank.config.para.ver_y = 120+10;
//    osd_pos_arrow_blank.config.para.hor_x = 680+300;
//    osd_pos_arrow_blank.str_arr = osd_north_angle;
//    update_OSD_chinese(osd_pos_arrow_blank, OSD_BRAM_HANDLE);



	//zyw add end-----------------------------------------------------------------------------------------------


	// osd_sys_mode_and_vcp.osd_idx = 0;
	// osd_sys_mode_and_vcp.config.para.Enable = 1;
	// osd_sys_mode_and_vcp.config.para.valid_length = 0;
	// osd_sys_mode_and_vcp.config.para.ver_y = 64;
	// osd_sys_mode_and_vcp.config.para.hor_x = 64;
	// osd_sys_mode_and_vcp.str_arr = sys_mode_GPCtrack;
	// update_OSD_chinese(osd_sys_mode_and_vcp, OSD_BRAM_HANDLE);

	// osd_test2.osd_idx = 2;
	// osd_test2.config.para.Enable = 1;
	// osd_test2.config.para.valid_length = 0;
	// osd_test2.config.para.ver_y = 64;
	// osd_test2.config.para.hor_x = 160;
	// osd_test2.str_arr = sys_mode_slave1;
	// update_OSD_chinese(osd_test2, OSD_BRAM_HANDLE);





//	// 21 闁跨喐鏋婚幏鐑芥晸鐟欙絽绱�
	osd_pos_ir_power.osd_idx = 110;
	osd_pos_ir_power.config.para.Enable = 1;
	osd_pos_ir_power.config.para.valid_length = 0;
	osd_pos_ir_power.config.para.ver_y = 200;
	osd_pos_ir_power.config.para.hor_x = 54;
	osd_pos_ir_power.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_ir_power, OSD_BRAM_HANDLE);

	// 22
	// osd_pos_ldr_power.osd_idx = 1;
	// osd_pos_ldr_power.config.para.Enable = 1;
	// osd_pos_ldr_power.config.para.valid_length = 0;
	// osd_pos_ldr_power.config.para.ver_y = 247;
	// osd_pos_ldr_power.config.para.hor_x = 54;
	// osd_pos_ldr_power.str_arr = osd_blank;
	// update_OSD_chinese(osd_pos_ldr_power, OSD_BRAM_HANDLE);
	osd_pos_22.osd_idx = 111;
	osd_pos_22.config.para.Enable = 1;
	osd_pos_22.config.para.valid_length = 0;
	osd_pos_22.config.para.ver_y = 247;
	osd_pos_22.config.para.hor_x = 54;
	osd_pos_22.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_22, OSD_BRAM_HANDLE);




	// 24
	osd_pos_moving_target.osd_idx = 113;
	osd_pos_moving_target.config.para.Enable = 1;
	osd_pos_moving_target.config.para.valid_length = 0;
	osd_pos_moving_target.config.para.ver_y = 377;
	osd_pos_moving_target.config.para.hor_x = 54;
	osd_pos_moving_target.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_moving_target, OSD_BRAM_HANDLE);

	// 25
	osd_pos_data_collect.osd_idx = 114;
	osd_pos_data_collect.config.para.Enable = 1;
	osd_pos_data_collect.config.para.valid_length = 0;
	osd_pos_data_collect.config.para.ver_y = 424;
	osd_pos_data_collect.config.para.hor_x = 54;
	osd_pos_data_collect.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_data_collect, OSD_BRAM_HANDLE);

	// 15: 1
	osd_pos_fault_code_1.osd_idx = 115;
	osd_pos_fault_code_1.config.para.Enable = 1;
	osd_pos_fault_code_1.config.para.valid_length = 0;
	osd_pos_fault_code_1.config.para.ver_y = 530;
	osd_pos_fault_code_1.config.para.hor_x = 54;
	osd_pos_fault_code_1.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_fault_code_1, OSD_BRAM_HANDLE);

	// 15: 2
	osd_pos_fault_code_2.osd_idx = 116;
	osd_pos_fault_code_2.config.para.Enable = 1;
	osd_pos_fault_code_2.config.para.valid_length = 0;
	osd_pos_fault_code_2.config.para.ver_y = 577;
	osd_pos_fault_code_2.config.para.hor_x = 115;
	osd_pos_fault_code_2.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_fault_code_2, OSD_BRAM_HANDLE);

	// 10: 1
	osd_pos_yaw.osd_idx = 119;
	osd_pos_yaw.config.para.Enable = 1;
	osd_pos_yaw.config.para.valid_length = 0;
	osd_pos_yaw.config.para.ver_y = 20;
	osd_pos_yaw.config.para.hor_x = 990;
	osd_pos_yaw.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_yaw, OSD_BRAM_HANDLE);

	// 10: 2
	osd_pos_pitch.osd_idx = 118;
	osd_pos_pitch.config.para.Enable = 1;
	osd_pos_pitch.config.para.valid_length = 0;
	osd_pos_pitch.config.para.ver_y = 67;
	osd_pos_pitch.config.para.hor_x = 990;
	osd_pos_pitch.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_pitch, OSD_BRAM_HANDLE);

	// 11
	osd_pos_laser_work_mode.osd_idx = 127;
	osd_pos_laser_work_mode.config.para.Enable = 1;
	osd_pos_laser_work_mode.config.para.valid_length = 0;
	osd_pos_laser_work_mode.config.para.ver_y = 20;
	osd_pos_laser_work_mode.config.para.hor_x = 1485;
	osd_pos_laser_work_mode.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_laser_work_mode, OSD_BRAM_HANDLE);

	// 14
	osd_pos_deorbit_time.osd_idx = 110;
	osd_pos_deorbit_time.config.para.Enable = 1;
	osd_pos_deorbit_time.config.para.valid_length = 0;
	osd_pos_deorbit_time.config.para.ver_y = 80;
	osd_pos_deorbit_time.config.para.hor_x = 1660;
	osd_pos_deorbit_time.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_deorbit_time, OSD_BRAM_HANDLE);

	// 26
	osd_pos_shine_code.osd_idx = 211;
	osd_pos_shine_code.config.para.Enable = 1;
	osd_pos_shine_code.config.para.valid_length = 0;
	osd_pos_shine_code.config.para.ver_y = 200;
	osd_pos_shine_code.config.para.hor_x = 1660;
	osd_pos_shine_code.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_shine_code, OSD_BRAM_HANDLE);

	// 27
	osd_pos_shine_time.osd_idx = 212;
	osd_pos_shine_time.config.para.Enable = 1;
	osd_pos_shine_time.config.para.valid_length = 0;
	osd_pos_shine_time.config.para.ver_y = 247;
	osd_pos_shine_time.config.para.hor_x = 1660;
	osd_pos_shine_time.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_shine_time, OSD_BRAM_HANDLE);

	// 28
	osd_pos_laser_prohibit.osd_idx = 213;
	osd_pos_laser_prohibit.config.para.Enable = 1;
	osd_pos_laser_prohibit.config.para.valid_length = 0;
	osd_pos_laser_prohibit.config.para.ver_y = 360;
	osd_pos_laser_prohibit.config.para.hor_x = 1720;
	osd_pos_laser_prohibit.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_laser_prohibit, OSD_BRAM_HANDLE);

	// 29
	osd_pos_temperature_alarm.osd_idx = 214;
	osd_pos_temperature_alarm.config.para.Enable = 1;
	osd_pos_temperature_alarm.config.para.valid_length = 0;
	osd_pos_temperature_alarm.config.para.ver_y = 410;
	osd_pos_temperature_alarm.config.para.hor_x = 1720;
	osd_pos_temperature_alarm.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_temperature_alarm, OSD_BRAM_HANDLE);

	// 18
	osd_pos_system_information.osd_idx = 31;//15
	osd_pos_system_information.config.para.Enable = 1;
	osd_pos_system_information.config.para.valid_length = 0;
	osd_pos_system_information.config.para.ver_y = 960;
	osd_pos_system_information.config.para.hor_x = 704;
	osd_pos_system_information.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_system_information, OSD_BRAM_HANDLE);



	// 1, 2, 3, 4, 5, 6
	osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir.osd_idx = 226;
	osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir.config.para.Enable = 1;
	osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir.config.para.valid_length = 0;
	osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir.config.para.ver_y = 20;
	osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir.config.para.hor_x = 54;
	osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir, OSD_BRAM_HANDLE);

	// 7, 8, 9
	osd_pos_inertia_velocitycompensation_workmode.osd_idx = 247;
	osd_pos_inertia_velocitycompensation_workmode.config.para.Enable = 1;
	osd_pos_inertia_velocitycompensation_workmode.config.para.valid_length = 0;
	osd_pos_inertia_velocitycompensation_workmode.config.para.ver_y = 20;
	osd_pos_inertia_velocitycompensation_workmode.config.para.hor_x = 610;
	osd_pos_inertia_velocitycompensation_workmode.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_inertia_velocitycompensation_workmode, OSD_BRAM_HANDLE);

	// 28
	osd_pos_north_angle.osd_idx = 228;
	osd_pos_north_angle.config.para.Enable = 1;  //
	osd_pos_north_angle.config.para.valid_length = 0;
	osd_pos_north_angle.config.para.ver_y = 106;
	osd_pos_north_angle.config.para.hor_x = 630;
	osd_pos_north_angle.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_north_angle, OSD_BRAM_HANDLE);

	/* 0, 1, 2, ..., 26, 27, 28  HAVE been used  */

	// 30 position_30_str_info
	osd_pos_30.osd_idx = 230;
	osd_pos_30.config.para.Enable = 0;
	osd_pos_30.config.para.valid_length = 0;
	osd_pos_30.config.para.ver_y = 460;
	osd_pos_30.config.para.hor_x = 1720;
	osd_pos_30.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_30, OSD_BRAM_HANDLE);

	// 31 position_31_str_info
	osd_pos_31.osd_idx = 131;
	osd_pos_31.config.para.Enable = 0;
	osd_pos_31.config.para.valid_length = 0;
	osd_pos_31.config.para.ver_y = 510;
	osd_pos_31.config.para.hor_x = 1720;
	osd_pos_31.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_31, OSD_BRAM_HANDLE);


	// 32-37 sys_init_pos
	osd_pos_init_sys[0].osd_idx = 32;
	osd_pos_init_sys[0].config.para.Enable = 0;
	osd_pos_init_sys[0].config.para.valid_length = 0;
	osd_pos_init_sys[0].config.para.ver_y = 192+48;//350;
	osd_pos_init_sys[0].config.para.hor_x = 824;
	osd_pos_init_sys[0].str_arr = osd_blank;
	update_OSD_chinese(osd_pos_init_sys[0], OSD_BRAM_HANDLE);
	osd_pos_init_sys[1].osd_idx = 33;
	osd_pos_init_sys[1].config.para.Enable = 0;
	osd_pos_init_sys[1].config.para.valid_length = 0;
	osd_pos_init_sys[1].config.para.ver_y = 232+48;//400;
	osd_pos_init_sys[1].config.para.hor_x = 824;
	osd_pos_init_sys[1].str_arr = osd_blank;
	update_OSD_chinese(osd_pos_init_sys[1], OSD_BRAM_HANDLE);
	osd_pos_init_sys[2].osd_idx = 34;
	osd_pos_init_sys[2].config.para.Enable = 0;
	osd_pos_init_sys[2].config.para.valid_length = 0;
	osd_pos_init_sys[2].config.para.ver_y = 272+48;//450;
	osd_pos_init_sys[2].config.para.hor_x = 824;
	osd_pos_init_sys[2].str_arr = osd_blank;
	update_OSD_chinese(osd_pos_init_sys[2], OSD_BRAM_HANDLE);
	osd_pos_init_sys[3].osd_idx = 35;
	osd_pos_init_sys[3].config.para.Enable = 0;
	osd_pos_init_sys[3].config.para.valid_length = 0;
	osd_pos_init_sys[3].config.para.ver_y = 312+48;//500;
	osd_pos_init_sys[3].config.para.hor_x = 824;
	osd_pos_init_sys[3].str_arr = osd_blank;
	update_OSD_chinese(osd_pos_init_sys[3], OSD_BRAM_HANDLE);
	osd_pos_init_sys[4].osd_idx = 36;
	osd_pos_init_sys[4].config.para.Enable = 0;
	osd_pos_init_sys[4].config.para.valid_length = 0;
	osd_pos_init_sys[4].config.para.ver_y = 352+48;//550;
	osd_pos_init_sys[4].config.para.hor_x = 824;
	osd_pos_init_sys[4].str_arr = osd_blank;
	update_OSD_chinese(osd_pos_init_sys[4], OSD_BRAM_HANDLE);
	osd_pos_init_sys[5].osd_idx = 37;
	osd_pos_init_sys[5].config.para.Enable = 0;
	osd_pos_init_sys[5].config.para.valid_length = 0;
	osd_pos_init_sys[5].config.para.ver_y = 392+48;//600;
	osd_pos_init_sys[5].config.para.hor_x = 824;
	osd_pos_init_sys[5].str_arr = osd_blank;
	update_OSD_chinese(osd_pos_init_sys[5], OSD_BRAM_HANDLE);

	//38 sys_version_pos
	osd_pos_version.osd_idx = 38;
	osd_pos_version.config.para.Enable = 0;
	osd_pos_version.config.para.valid_length = 0;
	osd_pos_version.config.para.ver_y = 144+48;//300;
	osd_pos_version.config.para.hor_x = 704;
	osd_pos_version.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_version, OSD_BRAM_HANDLE);

//	39 sys_test_string_pos
	osd_pos_test.osd_idx = 139;
	osd_pos_test.config.para.Enable = 1;
	osd_pos_test.config.para.valid_length = 0;
	osd_pos_test.config.para.ver_y = 1027;
	osd_pos_test.config.para.hor_x = 54;
	osd_pos_test.str_arr = osd_blank;
	update_OSD_chinese(osd_pos_test, OSD_BRAM_HANDLE);
}

void update_OSD_chinese(OSD_INFO osd_info, unsigned int *BRAM_HANDLE)
{
	if(osd_info.osd_idx>39)
//	if(osd_info.osd_idx>-1)
	{
//		printf("update_OSD_chinese >18\n");

	}
	else
	{
		BRAM_HANDLE[(START_OFFSET + OSD_INFO_STRIDE_BRAM_BYTE * osd_info.osd_idx + 4 * 0) >> 2] = osd_info.config.union_body; asm("nop");
		for(int i = 0; i < MAX_OSD_STRING_LENGTH_IN_CHINESE; i++)
		{
			BRAM_HANDLE[(START_OFFSET + OSD_INFO_STRIDE_BRAM_BYTE * osd_info.osd_idx + 4 * (i + 1)) >> 2] = ((osd_info.str_arr[2 * i + 1] )<<16) + osd_info.str_arr[2 * i]; asm("nop");
		}
	}

}

