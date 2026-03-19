/*
 * osd_graph_txt_chineseChar_app.h
 *
 *  Created on: 2023��1��8��
 *      Author: FYC
 */

#ifndef SRC_OSD_GRAPH_TXT_CHINESECHAR_APP_H_
#define SRC_OSD_GRAPH_TXT_CHINESECHAR_APP_H_

#include "osd_graph_txt_chineseChar_config.h"
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
#include <map>
#include <memory>

using namespace std;



extern unsigned int* OSD_BRAM_HANDLE ;


extern OSD_INFO osd_sys_mode_and_vcp;
extern OSD_INFO osd_pos_test;

extern OSD_INFO osd_pos_ir_power;
extern OSD_INFO osd_pos_22;//osd_pos_ldr_power;
extern OSD_INFO osd_pos_image_enhancement;
extern OSD_INFO osd_pos_moving_target;
extern OSD_INFO osd_pos_data_collect;
extern OSD_INFO osd_pos_fault_code_1;
extern OSD_INFO osd_pos_fault_code_2;
extern OSD_INFO osd_pos_yaw;
extern OSD_INFO osd_pos_pitch;
extern OSD_INFO osd_pos_laser_work_mode;
extern OSD_INFO osd_pos_deorbit_time;
extern OSD_INFO osd_pos_shine_code;
extern OSD_INFO osd_pos_shine_time;
extern OSD_INFO osd_pos_laser_prohibit;
extern OSD_INFO osd_pos_temperature_alarm;
extern OSD_INFO osd_pos_system_information;
extern OSD_INFO osd_pos_object_position;
extern OSD_INFO osd_pos_object_position_longitude;
extern OSD_INFO osd_pos_object_position_latitude;
extern OSD_INFO osd_pos_object_position_altitude;
extern OSD_INFO osd_pos_aircraft_position;
extern OSD_INFO osd_pos_aircraft_position_longitude;
extern OSD_INFO osd_pos_aircraft_position_latitude;
extern OSD_INFO osd_pos_aircraft_position_altitude;
extern OSD_INFO osd_pos_date;
extern OSD_INFO osd_pos_time;
extern OSD_INFO osd_pos_sensor_viewangle_enhance_viewstate_bricont_ir;
extern OSD_INFO osd_pos_inertia_velocitycompensation_workmode;
extern OSD_INFO osd_pos_north_angle;
extern OSD_INFO osd_pos_30;
extern OSD_INFO osd_pos_31;
extern OSD_INFO osd_pos_init_sys[6];
extern OSD_INFO osd_pos_version;

extern unsigned short sys_mode_init[32];
extern unsigned short sys_mode_init_vcp[32];

extern unsigned short sys_mode_stow[32];//collect
extern unsigned short sys_mode_stow_vcp[32];//collect

extern unsigned short sys_mode_cage1[32];//front view
extern unsigned short sys_mode_cage1_vcp[32];//front view

extern unsigned short sys_mode_mannul[32];
extern unsigned short sys_mode_mannul_vcp[32];

extern unsigned short sys_mode_scan1[32];//normal scan
extern unsigned short sys_mode_scan1_vcp[32];//normal scan

extern unsigned short sys_mode_track[32];//normal track
extern unsigned short sys_mode_track_vcp[32];//normal track

extern unsigned short sys_mode_GPCtrack[32];//geo location track
extern unsigned short sys_mode_GPCtrack_vcp[32];//geo location track

extern unsigned short sys_mode_slave1[32];//angle slave
extern unsigned short sys_mode_slave1_vcp[32];//angle slave

extern unsigned short sys_mode_service[32];//
extern unsigned short sys_mode_service_vcp[32];//

extern unsigned short sys_mode_scan2[32];//step scan
extern unsigned short sys_mode_scan2_vcp[32];//step scan

extern unsigned short sys_mode_slave2[32];//position slave
extern unsigned short sys_mode_slave2_vcp[32];//position slave

extern unsigned short sys_mode_cage2[32];//lock
extern unsigned short sys_mode_cage2_vcp[32];//lock


extern unsigned short osd_blank[32];    //xiao yin
extern unsigned short osd_sensor_viewangle_enhance_viewstate_bricont_ir[32];
extern unsigned short osd_inertia_velocitycompensation_workmode[32];
extern unsigned short osd_tv[32];    //dian shi
extern unsigned short osd_ir[32];    //hong wai
// unsigned short osd_light[32];    //hong wai
extern unsigned short osd_medium[32];    //zhong
extern unsigned short osd_heavy[32];    //zhong
extern unsigned short osd_big[32];    //da
extern unsigned short osd_small[32];    //xiao
extern unsigned short osd_super[32];    //chao
extern unsigned short osd_manual[32];    //shou dong
extern unsigned short osd_auto[32];    //zi dong
extern unsigned short osd_blackheat[32];    //hei re
extern unsigned short osd_whiteheat[32];    //bai re
extern unsigned short osd_valid[32];    //you xiao
extern unsigned short osd_invalid[32];    //wu xiao
extern unsigned short osd_close[32];    //guan bi
extern unsigned short osd_LMC[32];    //guan bi
extern unsigned short osd_start[32];    //qi dong
extern unsigned short osd_inertia[32];    //guan xing
extern unsigned short osd_airsurvey[32];    //hang ce
extern unsigned short osd_callback[32];    //hui shou
extern unsigned short osd_scan[32];    //sao miao
extern unsigned short osd_lock[32];    //suo ding
extern unsigned short osd_track[32];    //gen zong
// unsigned short osd_tracksearch[32];    //gen sou
// unsigned short osd_search[32];    //sou suo
extern unsigned short osd_lockcurrent[32];    //suo ding dang qian
extern unsigned short osd_geotrack[32];    //di li gen zong
extern unsigned short osd_yaw[32];    //fang wei
extern unsigned short osd_pitch[32];    //fu yang
extern unsigned short osd_laser_work_mode[32];    //ce ju
extern unsigned short osd_laser_work_five_seconds_stop[32];
extern unsigned short osd_laser_work_mode_stop[32];
extern unsigned short osd_shine[32];    //zhao she
extern unsigned short osd_stop[32];    //tingzhi
extern unsigned short osd_date[32];    //ri yue nian
extern unsigned short osd_time[32];    //shi fen miao
extern unsigned short osd_irclose[32];   //hong wai guan
extern unsigned short osd_iropen[32];    //hong wai kai
extern unsigned short osd_laserclose[32];    //ji guang guan
extern unsigned short osd_laseropen[32];      //ji guang kai
extern unsigned short osd_imageenhance[32];      //tu xiang zeng qiang
extern unsigned short osd_movingtarget[32];      //dong mu biao
extern unsigned short osd_datacollect[32];      //shu ju cai ji
extern unsigned short osd_shinecode[32];      //zhao she bian ma
extern unsigned short osd_shinetime[32];      //zhao she shi jian
extern unsigned short osd_laserprohibit[32];      //ji guang jin zhi
extern unsigned short osd_temperaturealarm[32];      //wen du gao jing
extern unsigned short osd_deorbitingtime[32];      //li gui shi jian
extern unsigned short osd_faultcode[32];      //gu zhang ma HEX
extern unsigned short osd_faultcode_HEX[32];      //gu zhang ma HEX
extern unsigned short osd_object_location[32];      //mu biao wei zhi
extern unsigned short osd_object_longitude[32];      //gao du
extern unsigned short osd_object_latitude[32];      //wei du
extern unsigned short osd_object_altitude[32];      //gao du
extern unsigned short osd_aircraft_location[32];      //zai ji wei zhi
extern unsigned short osd_aircraft_longitude[32];      //gao du
extern unsigned short osd_aircraft_latitude[32];      //wei du
extern unsigned short osd_aircraft_altitude[32];      //gao du
extern unsigned short osd_system_info[32];
extern unsigned short osd_system_init_info[32];
extern unsigned short osd_north_angle[32];
extern unsigned short osd_ai_detect[32];
extern unsigned short osd_moving_detect[32];
extern unsigned short osd_single_photo[32];
extern unsigned short osd_multi_photo[32];
extern unsigned short osd_pos_22_info[32];
// extern unsigned short osd_pos_31_info[32];
extern unsigned short osd_version_info[32];
extern unsigned short osd_pos_39_info[32];


extern std::map<unsigned char, unsigned short int*> map_sys_mode;
extern std::map<unsigned char, unsigned short int*> map_sys_mode_VCP;

void osd_init_chinese();
void update_OSD_chinese(OSD_INFO osd_info, unsigned int *BRAM_HANDLE);

#endif /* SRC_OSD_GRAPH_TXT_CHINESECHAR_APP_H_ */
