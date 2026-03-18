#ifndef IMAGEPROCESSINGSYSTEM_PROCESSOR_DEVICE_H
#define IMAGEPROCESSINGSYSTEM_PROCESSOR_DEVICE_H

#include "utils.h"
#include <chrono>
#include <queue>

extern int mismatched_crc_cnt;

class Device
{

public:
    Device(const std::string &name, const std::string &id);

    virtual ~Device();

    const std::string &get_name();

    virtual bool is_open();

    virtual bool open();

    virtual void close();

    virtual int write(const uint8_t *data, int len);

    virtual void read(const uint8_t *data, int len);

public:
    int write_print_every = 10000;
    int read_print_every = 10000;

protected:
    std::string name;
    std::string id;

};

class HostUARTDevice: public Device
{

private:
    HostUARTDevice();

public:
    static HostUARTDevice *instance();

    bool is_open();

    void parse_command(const uint8_t *data, int len, uint8_t *command_id, uint32_t *param_count, uint8_t *params, uint8_t *crc, bool with_head_tail, bool *crc_match);
    void struct_command(uint8_t *data, int *len, uint8_t command_id, uint32_t param_count, const uint8_t *params, uint8_t crc, bool with_head_tail);

    int  write(const uint8_t *data, int len);
    bool write_command(uint8_t command_id, uint32_t param_count, const uint8_t *params);
    bool write_command_new(uint8_t command_id, uint32_t param_count, const uint8_t *params, std::vector<uint8_t> &write_data, int *write_data_len);

    void read_callback();
    void read_byte(uint8_t data_byte);
    void read(const uint8_t *data, int len);
    void read_command(uint8_t command_id, uint32_t param_count, const uint8_t *params);

public:
    int write_command_print_every = 10000;
    int read_command_print_every = 10000;
    static const uint8_t read_head[1];
    // static const uint8_t read_tail[0];
    // static const uint8_t write_tail[0];
    static const uint8_t write_head[1];
    std::vector<uint8_t> write_data;
    int write_data_length;

    int fd;
    bool new_comm_recv;
    bool data_changed;
    bool use_interrupt_uart;
    bool save_split_position;
    bool report_multi_target_info;
    bool track_command_changed;
    bool capture_send;
    bool start_new_track;
    bool system_info_changed;
    bool request_temper;
    
    bool target_size_visible;
    bool fov_visible;
    bool geo_coordinates_visible;
    bool attitude_angle_visible;
    bool tv_filter;
    bool coordinate_visible;
    bool wave_gate_visible;
    uint8_t second_capture;
    bool second_capture_changed;
    bool cross_symbol_send;
    bool wave_gate_size_changed;
    bool wave_gate_position_changed;
    bool cross_main_send;
    bool yaw_view_angle_changed;

    uint8_t channel;  
    uint8_t channel_disp; 
    uint8_t track_command;
    uint8_t work_mode;
    uint8_t cross_pip_en;
    bool tv_align;
    bool ir_align;
    bool ir_cool_finish;
    bool send_ir_status;
    bool ir_mode;
    bool tv_mode;
    uint8_t photo_stat;
    uint8_t photo_time_gap;
    uint8_t photo_num;
    uint8_t photo_status;
    uint8_t detect_status;
    // uint8_t ldr_mode;
    uint8_t init_status_count;
    std::queue<std::string> init_status_cache;//20230911 fyc
    uint8_t multi_target_prompt;
    bool multi_target_prompt_send;
    bool recv_multi_prompt_tracked;

    uint8_t image_enhance_tv;
    uint8_t image_enhance_ir;
    uint8_t wave_gate_position_x;
    uint8_t wave_gate_position_y;
    // int16_t wave_gate_offset_x;
    // int16_t wave_gate_offset_y;  
    double wave_gate_width_scale;
    uint8_t wave_gate_size;  
    int16_t cross_symbol_x;
    int16_t cross_symbol_y;
    uint8_t track_ID;
    uint8_t aux_ID;
    bool aux_ID_plus;
    bool track_ID_send;
    uint8_t view_scale;
    bool view_scale_changed;
    uint8_t view_size;
    uint8_t ir_scale;
    uint8_t picture_in_picture;
    uint8_t picture_in_picture_position;
    int16_t tv_yaw_view_angle;
    int16_t ir_yaw_view_angle;
    int16_t cross_x_main;
    int16_t cross_y_main;
    int16_t cross_x_pip;
    int16_t cross_y_pip;
    int16_t cross_x_tv;
    int16_t cross_y_tv;
    int16_t cross_x_ir;
    int16_t cross_y_ir;
    uint8_t fusion;
    uint8_t aircraft_id;
    uint8_t compress_ratio;

    uint8_t show_level;
    uint8_t show_color;
    std::string show_str_system_init;
    std::string show_str_version;
    std::string show_str_work_mode;
    std::string show_str_view_size;
    std::string show_str_tv_status;
    std::string show_str_ir_status;
    std::string show_str_ldr_status;
    std::string show_str_vcp;
    std::vector<std::string> show_str_menus;
    std::map<uint8_t, int> init_status;
    std::string show_str_target_longitude;
    std::string show_str_target_latitude;
    std::string show_str_target_altitude;
    std::string show_str_system_info;
    std::string show_str_optical_axis_longitude;
    std::string show_str_optical_axis_latitude;
    std::string show_str_optical_axis_elevation;
    std::string show_str_attitude_angle_az;
    std::string show_str_attitude_angle_el;
    std::string show_str_attitude_angle_rng;
    std::string show_str_fov;
    std::string show_str_string_2;
    std::string show_str_target_length;
    std::string show_str_target_width;
    std::string show_str_target_height;

    int16_t yaw;
    std::string yaw_str;
	int16_t pitch;
	std::string pitch_str;
	int16_t yaw_v;
	std::string yaw_v_str;
	int16_t pitch_v;
	std::string pitch_v_str;
    bool yaw_pitch_v_send;
    int16_t eastward_vel;
    int16_t northward_vel;
    int16_t zenith_vel;

    int16_t flight_yaw;
    int16_t flight_pitch;
    int16_t flight_roll;

	uint8_t enhance_level;
	// std::string day;
	// std::string month;
	// std::string year;
	std::string date_str;
	std::string time_str;
	uint8_t brigheness_contrast_modify;
	uint8_t inertial_state;
	uint8_t image_enhancement;
	uint8_t target_style;
    uint8_t target_type;
    uint8_t multi_type;
	uint8_t para_collect;
	std::string shine_code_str;
	std::string shine_time_str;
	std::string yaw_view_angle_str;
	uint8_t sensor_view_size;
    bool sensor_view_changed;
	bool ir_power;
	bool ir_work;
	uint8_t ir_pola;   // 5:black, 6:white, 0:else
	uint8_t vel_comp;
	std::string fault_code_str;
	std::string deorbiting_time_str;
    int16_t distance;
	std::string laser_ranging_distance_str;
	// std::string laser_ranging_time;
	bool laser_power;
	bool laser_work;
	uint8_t laser_mode;
	bool laser_forbid;
	bool laser_alarm;
	int laser_work_time;
	std::string laser_work_time_str;
	double laser_work_start;
	bool weapon_activate;
	int weapon_work_time;
	std::string weapon_work_time_str;
	double weapon_work_start;
	std::string object_longitude_str;
    int object_longitude;
	std::string object_latitude_str;
    int object_latitude;
	std::string object_altitude_str;
    uint32_t object_altitude;
	std::string aircraft_longitude_str;
    int aircraft_longitude;
	std::string aircraft_latitude_str;
    int aircraft_latitude;
	std::string aircraft_altitude_str;
    uint32_t aircraft_altitude;
    uint16_t show_type;
	std::string system_info_str;
    std::string position_22_str;
    // std::string position_31_str;
	double system_info_start;
	bool system_info_enable;
    double system_version_start;
    bool system_version_enable;
	std::string north_angle_str;
};

class XJ3UARTDevice: public Device
{

private:
    XJ3UARTDevice();

public:
    static XJ3UARTDevice *instance();

    bool is_open();

    void parse_command(const uint8_t *data, int len, uint8_t *command_id, uint32_t *param_count, uint8_t *params, uint8_t *crc, bool with_head_tail, bool *crc_match);
    void struct_command(uint8_t *data, int *len, uint8_t command_id, uint32_t param_count, const uint8_t *params, uint8_t crc, bool with_head_tail);

    int write(const uint8_t *data, int len);
    bool write_command(uint8_t command_id, uint32_t param_count, const uint8_t *params);
    bool write_command_control(uint8_t track_command, uint8_t multi_target_prompt, uint8_t wave_gate_position, uint8_t wave_gate_width, uint8_t second_capture, uint16_t cross_symbol_x, uint16_t cross_symbol_y, uint8_t view);

    void read_callback();
    void read_byte(uint8_t data_byte);
    void read(const uint8_t *data, int len);
    void read_command(uint8_t command_id, uint32_t param_count, const uint8_t *params);

public:
    int write_command_print_every = 10000;
    int read_command_print_every = 10000;
    static const uint8_t read_head[1];
    static const uint8_t read_tail[1];
    static const uint8_t write_head[2];
    static const uint8_t write_tail[1];

    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    int fd;
    bool data_changed;
    bool self_check_ok;
    bool start_ok;
    bool mipi_reset;
    bool use_interrupt_uart;

    uint8_t track_status;
    uint8_t channel;
    std::vector<std::pair<int, int> > targets;
    std::vector<std::pair<int, int> > targets_sz;
    std::vector<std::pair<int, int> > targets_att;

    //add for tracking debug
    uint8_t nano_score;
    uint8_t prev_peak;
    uint8_t prev_apce;
    uint8_t cur_peak;
    uint8_t cur_apce;
    uint8_t nano_width;
    uint8_t nano_height;
    uint8_t csk_width;
    uint8_t csk_height;
    uint8_t strategy_ID;
    uint8_t pre_cur;
    uint8_t strategy_status;
    uint8_t continue_lost;
};


#endif
