#include "device.h"
#include <opencv2/opencv.hpp>
#include "osd_graph_txt_app.h"
#include "utils.h"
#ifdef OS_UNIX
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#endif

int mismatched_crc_cnt = 0;

// std::string pad_str(const std::string &str, int len, int align_direction = 0)
// {
//     if (str.size() >= len)
//     {
//         return str;
//     }

//     int left = (len - str.size()) / 2;
//     int right = len - str.size() - left;
//     if (align_direction < 0)
//     {
//         left = 0;
//         right = len - str.size();
//     }
//     else if (align_direction > 0)
//     {
//         left = len - str.size();
//         right = 0;
//     }

//     return std::string(left, ' ') + str + std::string(right, ' ');
// }

#ifdef OS_UNIX

extern volatile unsigned int *Uart_HOST_RX_HANDLE;

extern volatile unsigned int *Uart_HOST_TX_HANDLE;

extern volatile unsigned int *Uart_XJ3_RX_HANDLE;

extern volatile unsigned int *Uart_XJ3_TX_HANDLE;

extern std::map<std::string, int> interrupt_uart_fds;

extern bool init_interrupt_XJ3_uart();
extern bool init_interrupt_HOST_uart();
#endif

Device::Device(const std::string &name, const std::string &id)
    : name(name), id(id)
{
    // open();
}

Device::~Device()
{
    // close();
}

const std::string &Device::get_name()
{
    return name;
}

bool Device::open()
{
    static auto disabled_devices = split(parse_arg("disabled_devices", ""), ';');
    if (contains(disabled_devices, name) || contains(disabled_devices, "all"))
    {
        log_warning("Device %s disabled.", name.c_str());
        return false;
    }

    close();

    return true;
}

bool Device::is_open()
{
    static auto disabled_devices = split(parse_arg("disabled_devices", ""), ';');
    if (contains(disabled_devices, name) || contains(disabled_devices, "all"))
    {
        return false;
    }

    return true;
}

void Device::close()
{
    if (!is_open())
    {
        return;
    }
}

int Device::write(const uint8_t *data, int len)
{
    if (!is_open())
    {
        return -1;
    }

    return 0;
}

void Device::read(const uint8_t *data, int len)
{
}

const uint8_t HostUARTDevice::read_head[1] = {0xCC};

// const uint8_t HostUARTDevice::read_tail[1] = {};

const uint8_t HostUARTDevice::write_head[1] = {0xCC};

// const uint8_t HostUARTDevice::write_tail[0] = {};

HostUARTDevice::HostUARTDevice()
    : Device("Host", parse_arg("host_uart_file", "/dev/irq2_drv")), fd(-1), new_comm_recv(false), write_data_length(0), channel(1), work_mode(0x01), ir_cool_finish(false), send_ir_status(false), ir_power(false), ir_mode(false), tv_mode(false), laser_mode(0x09), tv_align(false),ir_align(false), init_status_count(0), picture_in_picture(2), track_command(1), track_command_changed(false), multi_target_prompt(2), report_multi_target_info(false), image_enhance_tv(2), image_enhance_ir(4), coordinate_visible(false), wave_gate_visible(true), photo_status('0'),request_temper(false), wave_gate_position_x(0), wave_gate_position_y(0), wave_gate_position_changed(false), wave_gate_size_changed(false), wave_gate_size(2), wave_gate_width_scale(1.0), second_capture(2), second_capture_changed(false), tv_filter(1), photo_stat(0x03),system_info_changed(false), cross_symbol_x(-1), cross_symbol_y(-1), track_ID_send(false), track_ID(1), aux_ID(0), aux_ID_plus(true), view_size(1), view_scale(2), view_scale_changed(false), target_size_visible(false), fov_visible(true), geo_coordinates_visible(false), fusion(2), detect_status('0'),attitude_angle_visible(true), aircraft_id(0xa1), show_color(0), show_level(1), cross_symbol_send(false), cross_main_send(false), cross_x_main(960), cross_y_main(540), cross_x_pip(960), cross_y_pip(540), cross_x_pip_independent(960), cross_y_pip_independent(540),cross_x_tv(960), cross_y_tv(540), cross_x_ir(960), cross_y_ir(540), cross_pip_en(0), save_split_position(false), laser_power(false), channel_disp(1),laser_forbid(false), target_style(1), target_type(1), sensor_view_size(1), sensor_view_changed(false), ir_work(true), laser_work(true), ir_pola(0x06), vel_comp(2), yaw_pitch_v_send(false), tv_yaw_view_angle(0), ir_yaw_view_angle(0), yaw_view_angle_changed(false), compress_ratio(3)
{
    use_interrupt_uart = std::stoi(parse_arg("use_interrupt_uart", "1"));

#ifdef OS_UNIX
    if (use_interrupt_uart)
    {
        init_interrupt_HOST_uart();
        fd = map_at(interrupt_uart_fds, id, -1);
    }
    else
    {
        fd = ::open(id.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (fd < 0)
        {
            log_critical("Failed to open: '%s'", id.c_str());
            return;
        }

        termios attr;
        tcgetattr(fd, &attr);
        attr.c_cflag |= CLOCAL | CREAD;
        attr.c_cflag &= ~CSIZE;
        cfsetispeed(&attr, B115200);  // baud rate: 115200
        cfsetospeed(&attr, B115200);  // baud rate: 115200
        attr.c_cflag |= CS8;  // data bit: 8
        attr.c_cflag &= ~PARENB;  // no check
        attr.c_iflag &= ~INPCK;  // no check
        attr.c_cflag &= ~CSTOPB;  // stop bit: 1
        attr.c_cc[VTIME] = 1;  // timeout: 0.1s
        attr.c_cc[VMIN] = 0;  // nonblock
        tcflush(fd, TCIOFLUSH);
        tcsetattr(fd, TCSANOW, &attr);
    }
#endif

    // init show strings
	date_str = "00/00/0000";
	time_str = "00:00:00";
    show_str_system_init = "......";
    show_str_system_init = pad_str(show_str_system_init, 17, -1);
    show_str_version = format("SSDS SW V%d.%d.%02d.%02d 20%02d %02d %02d", 0, 0, 0, 1, 23, 6, 6);
    show_str_version = pad_str(show_str_version, 30, 0);
    show_str_work_mode = "INIT";
    show_str_work_mode = pad_str(show_str_work_mode, 9, -1);
    show_str_view_size = "L";
    show_str_view_size = pad_str(show_str_view_size, 6, -1);
    show_str_tv_status = "";
    show_str_tv_status = pad_str(show_str_tv_status, 6, -1);
    show_str_ir_status = "";
    show_str_ir_status = pad_str(show_str_ir_status, 7, -1);
    show_str_ldr_status = "";
    show_str_ldr_status = pad_str(show_str_ldr_status, 7, -1);
    show_str_vcp = "    ";
    show_str_vcp = pad_str(show_str_vcp, 4, -1);
    show_str_menus = std::vector<std::string>{"     ", "     ", "     ", "     ", "     "};
    init_status = {{0x11, -1}, {0x12, -1}, {0x13, -1}, {0x14, -1}, {0x15, -1}, {0x16, -1}, {0x17, -1}, {0x21, -1}, {0x22, -1}, {0x23, -1}, {0x24, -1}, {0x25, -1}, {0x31, -1}, {0x41, -1}, {0x42, -1}, {0x43, -1}};
    show_str_target_longitude = format("LON: %.5lf", 0 * 1e-5);
    show_str_target_longitude = pad_str(show_str_target_longitude, 14, -1);
    show_str_target_latitude = format("LAT: %.5lf", 0 * 1e-5);
    show_str_target_latitude = pad_str(show_str_target_latitude, 14, -1);
    show_str_target_altitude = format("ALT: %d", 0);
    show_str_target_altitude = pad_str(show_str_target_altitude, 14, -1);
    show_str_system_info = format("AVT VER: %.2lf", 0 * 1e-2);
    show_str_system_info = pad_str(show_str_system_info, 20, 0);
    show_str_fov = format("FOV: %.2lf x %.2lf", 0 * 1e-2, 0 * 1e-2);
    show_str_fov = pad_str(show_str_fov, 18, -1);
    show_str_optical_axis_longitude = format("L: %.5lf", 0 * 1e-5);
    show_str_optical_axis_longitude = pad_str(show_str_optical_axis_longitude, 14, -1);
    show_str_optical_axis_latitude  = format("B: %.5lf", 0 * 1e-5);
    show_str_optical_axis_latitude  = pad_str(show_str_optical_axis_latitude, 14, -1);
    show_str_optical_axis_elevation = format("R: %d", 0);
    show_str_optical_axis_elevation = pad_str(show_str_optical_axis_elevation, 14, -1);
    show_str_attitude_angle_az = format("AZ: %.2lf", 0 * 1e-2);
    show_str_attitude_angle_az = pad_str(show_str_attitude_angle_az, 14, -1);
    show_str_attitude_angle_el = format("EL: %.2lf", 0 * 1e-2);
    show_str_attitude_angle_el = pad_str(show_str_attitude_angle_el, 14, -1);
    show_str_attitude_angle_rng = format("RNG: %d", 0);
    show_str_attitude_angle_rng = pad_str(show_str_attitude_angle_rng, 14, -1);

    show_str_target_length = format("L: %.1lf", 0 * 1e-1);
    show_str_target_length = pad_str(show_str_target_length, 14, -1);
    show_str_target_width = format("W: %.1lf", 0 * 1e-1);
    show_str_target_width = pad_str(show_str_target_width, 14, -1);
    show_str_target_height = format("H: %.1lf", 0 * 1e-1);
    show_str_target_height = pad_str(show_str_target_height, 14, -1);
    show_str_string_2 = " ";
    show_str_string_2 = pad_str(show_str_string_2, 20, 0);

    north_angle_str = "000.00";
    north_angle_str = pad_str(north_angle_str,7,1);
    shine_code_str = " ";
    shine_time_str = " ";

}

HostUARTDevice *HostUARTDevice::instance()
{
    static HostUARTDevice instance;
    return &instance;
}

bool HostUARTDevice::is_open()
{
    return fd >= 0;
}

void HostUARTDevice::parse_command(const uint8_t *data, int len, uint8_t *command_id, uint32_t *param_count, uint8_t *params, uint8_t *crc, bool with_head_tail, bool *crc_match)
{
    int byte_index = 0;
    if (with_head_tail)
    {
        byte_index += SIZEOF(read_head);
    }
    // uint8_t remote_address;
    // PARSE_DATA_LE(data, &remote_address);
    uint8_t this_address;
    PARSE_DATA_LE(data, &this_address);
    if(this_address == 0x06)
    {
        uint8_t param_count_;
        PARSE_DATA_LE(data, &param_count_);
        *param_count = param_count_ - SIZEOF(read_head) - sizeof(this_address) - sizeof(param_count_) - sizeof(*crc) - 1;
        int crc_begin_index = byte_index;
        PARSE_DATA_LE(data, command_id);
        PARSE_DATA_COPY(data, params, *param_count);
        int crc_end_index = byte_index;
        PARSE_DATA_LE(data, crc);
        if (with_head_tail)
        {
            // byte_index += SIZEOF(read_tail);
        }
        // check crc
        if (crc_match)
        {
            uint8_t crc_ = 0;
            for (int i = crc_begin_index; i < crc_end_index; ++i)
            {
                crc_ += data[i];
            }
            *crc_match = (*crc == crc_);
            if (!(*crc_match))
            {
            	mismatched_crc_cnt++;
//                log_warning("Mismatched CRC in %s command id 0x%02X: 0x%02X (actual) != 0x%02X (calculated)", name.c_str(), *command_id, *crc, crc_);
//                log_warning("Data for CRC: %s", encode_bytes(std::string(reinterpret_cast<const char *>(data) + crc_begin_index, crc_end_index - crc_begin_index)).c_str());
            }
        }
    }
    // else
    // {
    //     uint8_t param_count_;
    //     PARSE_DATA_LE(data, &param_count_);
    //     *param_count = param_count_ - SIZEOF(read_head) - sizeof(this_address) - sizeof(param_count_) - sizeof(*crc);
    //     *command_id  = 0x80;
    //     int crc_begin_index = byte_index;
    //     PARSE_DATA_COPY(data, params, *param_count);   
    //     int crc_end_index = byte_index;
    //     PARSE_DATA_LE(data, crc);
    //     // check crc
    //     if (crc_match)
    //     {
    //         uint8_t crc_ = 0;
    //         for (int i = crc_begin_index; i < crc_end_index; ++i)
    //         {
    //             crc_ += data[i];
    //         }
    //         *crc_match = (*crc == crc_);
    //         if (!(*crc_match))
    //         {
    //             log_warning("Mismatched CRC in %s command: 0x%02X (actual) != 0x%02X (calculated)", name.c_str(), *crc, crc_);
    //             log_warning("Data for CRC: %s", encode_bytes(std::string(reinterpret_cast<const char *>(data) + crc_begin_index, crc_end_index - crc_begin_index)).c_str());
    //         }
    //     }    
    // }
}

void HostUARTDevice::struct_command(uint8_t *data, int *len, uint8_t command_id, uint32_t param_count, const uint8_t *params, uint8_t crc, bool with_head_tail)
{
    int byte_index = 0;
    if (with_head_tail)
    {
        STRUCT_DATA_COPY(data, write_head, SIZEOF(write_head));
    }
    // uint8_t remote_address = 0x00;
    // STRUCT_DATA_LE(data, &remote_address);
    uint8_t this_address = 0x06;
    STRUCT_DATA_LE(data, &this_address);
    uint8_t param_count_ = param_count + SIZEOF(read_head) + sizeof(this_address) + sizeof(param_count_) + sizeof(crc) + 1;
    STRUCT_DATA_LE(data, &param_count_);
    int crc_begin_index = byte_index;
    STRUCT_DATA_LE(data, &command_id);
    STRUCT_DATA_COPY(data, params, param_count);
    int crc_end_index = byte_index;
    if (crc == 0)
    {
        // calculate crc
        crc = 0;
        for (int i = crc_begin_index; i < crc_end_index; ++i)
        {
            crc += data[i];
        }
    }
    STRUCT_DATA_LE(data, &crc);
    if (with_head_tail)
    {
        // STRUCT_DATA_COPY(data, write_tail, SIZEOF(write_tail));
    }
    *len = byte_index;
}

int HostUARTDevice::write(const uint8_t *data, int len)
{
    int write_len = 0;
//    std::cout << "HostUARTDevice write: ";
//    for (int i = 0; i < len; i++) {
//        std::cout << std::hex << (int) data[i] << " ";
//    }
//    std::cout << std::endl;

#ifdef OS_UNIX
    if (use_interrupt_uart)
    {
        for (int i = 0; i < len; ++i)
        {
            while (Uart_HOST_TX_HANDLE[0x08 >> 2] & 0x08)
            {
                asm("nop");
                //std::this_thread::sleep_for(std::chrono::microseconds(5)); //changed zzy 2022/12/17
            }
			//zyw compare add 
            Uart_HOST_TX_HANDLE[1] = data[i];
            asm("nop");
        }
        write_len = len;
    }
    else
    {
        write_len = ::write(fd, data, len);
    }
#endif

    //DO_EVERY_N_MS(read_print_every, log_info, "[%ds/p] %s write (index=%d,len=%d/%d): %s ...", read_print_every / 1000, name.c_str(), visit_time, len, write_len, encode_bytes(std::string(reinterpret_cast<const char *>(data), std::min<int>(64, len))).c_str());

    return write_len;
}

bool HostUARTDevice::write_command(uint8_t command_id, uint32_t param_count, const uint8_t *params)
{
    static uint8_t command_data[64] = {0};
    static int command_data_len = 0;

    struct_command(command_data, &command_data_len, command_id, param_count, params, 0, true);
    int len = write(command_data, command_data_len);

    return len == command_data_len;
}

bool HostUARTDevice::write_command_new(uint8_t command_id, uint32_t param_count, const uint8_t *params, std::vector<uint8_t> &write_data, int *write_data_len)
{
    static uint8_t command_data[64] = {0};
    static int command_data_len = 0;
    struct_command(command_data, &command_data_len, command_id, param_count, params, 0, true);

    for(int i=0; i<command_data_len; ++i)
        write_data.emplace_back(command_data[i]);
    *write_data_len += command_data_len;
//    printf("data_changed %d\n",data_changed);
    if(!data_changed)
        data_changed = true;
    return true;
}

void HostUARTDevice::read_callback()
{

#ifdef OS_UNIX
    if (!use_interrupt_uart)
    {
        uint8_t data_byte = 0;
        while (true)
        {
            int len = ::read(fd, &data_byte, 1);
            if (len <= 0)
            {
                break;
            }
            read_byte(data_byte);
        }
    }
#endif
}

void HostUARTDevice::read_byte(uint8_t data_byte)
{
    static bool print_read_byte = std::stoi(parse_arg("print_read_byte", "0"));
    if (print_read_byte)
    {
        DO_EVERY_N(1, log_info, "%s read byte (index=%d): 0x%02X", name.c_str(), visit_time, data_byte);
    }

    static std::string buffer(1024, 0);
    static uint8_t *data = reinterpret_cast<uint8_t *>(&buffer[0]);
    static int len = 0;
    static int frame_len = 0;

    if ((len == 0) && (data_byte != read_head[0]))
    {
        return;
    }

    data[len] = data_byte;
    ++len;

    if (len < 3)
    {
        return;
    }
    if (frame_len == 0)
    {
        frame_len = data[2];
    }
    if (len < frame_len)
    {
        return;
    }

    read(data, len);
    len = 0;
    frame_len = 0;
}

void HostUARTDevice::read(const uint8_t *data, int len)
{
    //DO_EVERY_N_MS(read_print_every, log_info, "[%ds/p] %s read (index=%d,len=%d): %s ...", read_print_every / 1000, name.c_str(), visit_time, len, encode_bytes(std::string(reinterpret_cast<const char *>(data), std::min<int>(64, len))).c_str());
    //zyw 闂傚倷娴囧▔鏇㈠窗閹版澘鍑犲┑鐘宠壘缁狀垶鏌ｉ幋锝呅撻柡鍛倐閺岋繝宕掑Ο鐚存嫹閺嶎偓鎷峰鐐 闂傚倷娴囧▔鏇㈠窗閹版澘鍑犲┑鐘宠壘缁狀垳锟界懓瀚妯肩矈椤忓牊鈷戦悹鎭掑妼閺嬫垿鏌＄�ｎ亶鐓兼鐐茬箻閺屻劎锟斤絺鏅濈粈锟�
//	std::cout << "Value of data[3]: " << std::hex << (int)data[3] << std::endl;
//    if (data[3] == 0x23 || data[3] == 0x25) {
//
//    } else {
//        std::cout << "HostUARTDevice read : ";
//        for (int i = 0; i < len; i++) {
//            std::cout << std::hex << (int) data[i] << " ";
//        }
//        std::cout << std::endl;
//    }
    const uint8_t *data_end = data + len;
    while (data < data_end)
    {
        const uint8_t *data_head = std::search(data, data_end, read_head, read_head + SIZEOF(read_head));
        if (!data_head || (data_head >= data_end))
        {
            data = data_end;
            break;
        }
        int command_len = data_head[SIZEOF(read_head) + 1];
        const uint8_t *data_tail = data_head + command_len;
        if (data_tail > data_end)
        {
            data = data_end;
            break;
        }

        do
        {
            const uint8_t *data = data_head + SIZEOF(read_head);
            int len = data_tail - data;

            uint8_t command_id = 0;
            uint32_t param_count = 0;
            static uint8_t params[1024] = {0};
            uint8_t crc = 0;
            bool crc_match = true;
            parse_command(data, len, &command_id, &param_count, params, &crc, false, &crc_match);

            if (!crc_match)
            {
                break;
            }

            read_command(command_id, param_count, params);
        }
        while (false);

        data = data_tail;
    }
}

void HostUARTDevice::read_command(uint8_t command_id, uint32_t param_count, const uint8_t *params)
{
    // write_data.clear()
    // write_data_length = 0;
    new_comm_recv = true;
//    if(command_id==0x23)
//    {
//
//    }else
//    {
//    printf("command_id %x\n",command_id);
//    if(command_id==0x16)
//    {
//    	        printf("params: ");
//    	        for (uint32_t i = 0; i < param_count; ++i)
//    	        {
//    	            printf("%02X ", params[i]);  // 濞寸姰鍎卞畷鍕礂椤擄紕绠婚柛鎺曟硾閼告澘顕ｈ箛鏂库叺闁告澧楅惁鈩冪▔椤忓嫮鎽熼柤鐚存嫹
//    	        }
//    	        printf("\n");
//
//    }
//         闁瑰灚鎸稿畵锟� params 闁汇劌瀚敓鏂ゆ嫹
//        printf("params: ");
//        for (uint32_t i = 0; i < param_count; ++i)
//        {
//            printf("%02X ", params[i]);  // 濞寸姰鍎卞畷鍕礂椤擄紕绠婚柛鎺曟硾閼告澘顕ｈ箛鏂库叺闁告澧楅惁鈩冪▔椤忓嫮鎽熼柤鐚存嫹
//        }
//        printf("\n");
//    }
    switch (command_id)
    {
    case 0x00:
    {
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): empty", read_command_print_every / 1000, name.c_str(), visit_time);
    }
        break;
    case 0xAA:
    {
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): shake_hand", read_command_print_every / 1000, name.c_str(), visit_time);
        track_command = 1;
        track_command_changed = true;
        second_capture = 2;
        second_capture_changed = true;
        multi_target_prompt = 2;
        image_enhance_tv = 2;
        image_enhance_ir = 4;
        picture_in_picture = 2;
        photo_stat = 3;
        shine_code_str = " ";
        shine_time_str = " ";

//        this->view_scale = 2;

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): self_checking (flag=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, flag);
        {
            uint8_t params[2] = {0x01, 0x00};
            // write_command(0x02, SIZEOF(params), params);
            write_command_new(0x01, SIZEOF(params), params, write_data, &write_data_length);
            //electronic zoom callback
            uint8_t params_view[1] = {2};
            //write_command(0x10, SIZEOF(params), params);
            write_command_new(0x10, SIZEOF(params_view), params_view, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): self_checking_response", write_command_print_every / 1000, name.c_str(), visit_time);
            XJ3UARTDevice::instance()->write_command_control((0xF0 | track_command), (0xF0 | multi_target_prompt), 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
        }
    }
        break;
    case 0x01:
    {
        int byte_index = 0;
        uint8_t flag;
        PARSE_DATA_LE(params, &flag);
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): self_checking (flag=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, flag);
        {
            uint8_t params[2] = {0x01, 0x00};
            // write_command(0x02, SIZEOF(params), params);
            write_command_new(0x02, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): self_checking_response", write_command_print_every / 1000, name.c_str(), visit_time);
            // XJ3UARTDevice::instance()->write_command_control((0xF0 | track_command), (0xF0 | multi_target_prompt), 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
        }
    }
        break;
    case 0x02:
    {
        int byte_index = 0;
        uint8_t channel;
        PARSE_DATA_LE(params, &channel);
        this->channel = channel;

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): select_channel (channel=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, channel);
        
        {
            uint8_t params[1] = {channel};
            //write_command(0x06, SIZEOF(params), params);
            write_command_new(0x06, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): select_channel_response", write_command_print_every / 1000, name.c_str(), visit_time, channel);
        }
    }
        break;
    case 0x03:
    {
        int byte_index = 0;
        uint8_t track_command;
        PARSE_DATA_LE(params, &track_command);
        if((this->track_command != track_command) || (track_command == 4))
        {
            track_command_changed = true;  
            uint8_t params[1];
            if(track_command == 1)
                params[0] = {0x01};
            else
                params[0] = {0x02};

            if(track_command == 4)
            {
                // printf("send XJ3 second track.\n");
                XJ3UARTDevice::instance()->write_command_control(0xF4, 0xFF, 0xFF, 0xFF, 0xFF, cross_symbol_x, cross_symbol_y, 0x01); //20230316
                write_command_new(0x07, SIZEOF(params), params, write_data, &write_data_length);
            }
            else if(track_command == 3)
            {
                track_ID_send = true;  
                track_command_changed = false;             
            }
            else if(track_command == 2)
            {
                XJ3UARTDevice::instance()->write_command_control(0xF2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
                write_command_new(0x07, SIZEOF(params), params, write_data, &write_data_length);
            }
            else
            {
                capture_send = true;
                write_command_new(0x07, SIZEOF(params), params, write_data, &write_data_length);
            }
        }
        if((this->track_command == 2) && (track_command == 2))
            start_new_track = true;

        this->track_command = track_command;
    
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): track_command (track_command=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, track_command);
        // XJ3UARTDevice::instance()->write_command_control((0x0F << 4) | track_command, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF);
        {
            // if(track_command != this->track_command)
            // {        
            // uint8_t params[1] = {(uint8_t) (track_command)};
            // HostUARTDevice::instance()->write_command(0x07, SIZEOF(params), params);                            
            // }
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): track_command_response_status", write_command_print_every / 1000, name.c_str(), visit_time);
        }
            // {
            // uint8_t params[1] = {(uint8_t) (XJ3UARTDevice::instance()->track_status)};
            // write_command(0x08, SIZEOF(params), params);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): track_command_response_type", write_command_print_every / 1000, name.c_str(), visit_time);
            // }
        {
            // uint8_t params[4] = {(uint8_t) x,  (uint8_t) (x >> 8), (uint8_t) y, (uint8_t) (y >> 8)};
            // write_command(0x09, SIZEOF(params), params);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): track_command_response_variance", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x04:
    {
        int byte_index = 0;
        uint8_t multi_target_prompt;
        PARSE_DATA_LE(params, &multi_target_prompt);

        if(track_command != 1 && multi_target_prompt == 1) //&& this->multi_target_prompt == 2
            recv_multi_prompt_tracked = true;    //闂傚倷绶氬鑽ゆ嫻閻旂厧绀夐煫鍥ㄦ礈閻滃鏌ㄩ悢鍝勑ラ柛娆愬笚閵囧嫯绠涢幘鎼缂佹鍨垮铏规嫚閳ュ啿骞愰梺缁樺釜缁犳挸顕ｉ銉ｄ汗闁圭儤鍨归ˇ顐ｄ繆椤愶富鏆掗柤鍐茬埣瀹曨垱绻濋崒銈囧數闂佽偐顭堥悘姘舵儍閹寸偟绠鹃棅顐幖瀹撳棛锟芥鍠曠划娆忕暦閸洖惟闁靛／鍐挎嫹閹烘梻纾藉〒姘攻鐎氬綊姊洪崨濠勭畵閻庢凹鍠氭竟鏇熺節濮橆厾鍘介梺闈涱焾閸庨亶顢旈鍫熷�甸柛顭戝亞椤ｈ尙绱掗崒娑樻诞濠碘剝鍎肩粻娑㈠即閻欙拷濡茬兘姊婚崒娆戣窗闁稿鎹囬獮蹇涙倻閼恒儳鍘遍梺纭呮彧婵″洨妲愰幍顔剧＜闁规澘澧庣弧锟介梺缁樻惄娴滎亪銆侀弮鍫濈妞ゆ帪鎷烽柨鐔烘櫕閺佸寮婚妶鍡欓檮濠㈣泛顦遍惄搴㈢節濞堝灝鏋撻柡鍛█閻涱喖顓兼径濠勵啋閻庤娲栧ú銊╂晬濞戞瑧绡�闁靛骏绲介悡鎰叏濡濡奸悡銈吤归崗鍏肩稇闁告劧鎷烽梻浣规偠閸庮垶宕濆畝锟藉濠囧捶椤撶姷锛滈梺璺ㄥ枍缁瑩寮幘缁樻櫢闁跨噦鎷�
        else 
        {
            if(multi_target_prompt == 1 || multi_target_prompt == 2)
                XJ3UARTDevice::instance()->write_command_control(0xFF, (0xF0 | multi_target_prompt), 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF); //20230106
        }

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): multi_target_prompt (multi_target_prompt=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, multi_target_prompt);
        {
            // if(multi_target_prompt == 0x03 || multi_target_prompt == 0x04 || multi_target_prompt == 0x08 || multi_target_prompt == 0x09 || this->multi_target_prompt == multi_target_prompt)
            // {
            //     uint8_t params[1] = {multi_target_prompt};
            //     write_command_new(0x0A, SIZEOF(params), params, write_data, &write_data_length);
            // }
            // else
            //     this->multi_target_prompt = multi_target_prompt;
            if(multi_target_prompt == 0x01 || multi_target_prompt == 0x02)
            {
                this->multi_target_prompt = multi_target_prompt;
                this->multi_target_prompt_send = true;
            }
            else 
            {
                if(multi_target_prompt == 0x03)
                {
                    if(aux_ID == 4)
                        aux_ID = 0;
                    else 
                        aux_ID++;
                    aux_ID = (aux_ID >= 0) ? ((aux_ID <= 4) ? aux_ID : 4) : 0;
                    aux_ID_plus = true;
                }    
                else if(multi_target_prompt == 0x04)
                {
                    if(aux_ID == 0)
                        aux_ID = 4;
                    else 
                        aux_ID--;
                    aux_ID = (aux_ID >= 0) ? ((aux_ID <= 4) ? aux_ID : 4) : 0; 
                    aux_ID_plus = false;
                }
                else if(multi_target_prompt == 0x08 || multi_target_prompt == 0x09)
                {
                    this->multi_type = multi_target_prompt;
                }
                uint8_t params[1] = {multi_target_prompt};
                write_command_new(0x0A, SIZEOF(params), params, write_data, &write_data_length);                
            }
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): multi_target_prompt_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x05:
    {
		this->weapon_work_start = get_wall_time();
		this->weapon_work_time = -1;
		this->weapon_activate = true;

		//printf("%d\n", this->weapon_activate);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): weapon_activate", read_command_print_every / 1000, name.c_str(), visit_time);

        {
            //write_command(0x0C, 0, 0);
            write_command_new(0x0C, 0, 0, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): weapon_activate_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
        XJ3UARTDevice::instance()->write_command_control(0xAF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF); //20230106
    }
        break;
    case 0x06:
    {
        int byte_index = 0;
        uint8_t wave_gate_visible;
        PARSE_DATA_LE(params, &wave_gate_visible);
        this->wave_gate_visible = (wave_gate_visible==1);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): wave_gate_visible (wave_gate_visible=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, wave_gate_visible);

        {
            uint8_t params[1] = {wave_gate_visible};
            //write_command(0x0D, SIZEOF(params), params);
            write_command_new(0x0D, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): wave_gate_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x07:
    {
        int byte_index = 0;
        uint8_t wave_gate_position_x;
        PARSE_DATA_LE(params, &wave_gate_position_x);
        this->wave_gate_position_x = wave_gate_position_x;
        // if (wave_gate_position_x == 1)
        // {
        //     ++wave_gate_offset_x;
        // }
        // else if (wave_gate_position_x == 2)
        // {
        //     --wave_gate_offset_x;
        // }
        uint8_t wave_gate_position_y;
        PARSE_DATA_LE(params, &wave_gate_position_y);
        this->wave_gate_position_y = wave_gate_position_y;
        // if (wave_gate_position_y == 1)
        // {
        //     ++wave_gate_offset_y;
        // }
        // else if (wave_gate_position_y == 2)
        // {
        //     --wave_gate_offset_y;
        // }
        this->wave_gate_position_changed = true;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): wave_gate_position (x=%d, y=%d)", read_command_print_every / 1000, name.c_str(), visit_time, (int) wave_gate_position_x, (int) wave_gate_position_y);

        {
            uint8_t params[1] = {(uint8_t) (((uint8_t) ((bool) wave_gate_position_x)) | ((uint8_t) ((bool) wave_gate_position_y) << 1))};
            //write_command(0x16, SIZEOF(params), params);
            write_command_new(0x16, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): wave_gate_position_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
        {
            //if( (wave_gate_offset_x & 0x0010) || (wave_gate_offset_y & 0x0010) )
            // XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, (wave_gate_position_y << 4) | wave_gate_position_x, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF); //20230106
        }
    }
        break;
    case 0x08:
    {
        int byte_index = 0;
        uint8_t image_enhance;
        PARSE_DATA_LE(params, &image_enhance);
        if(image_enhance <3)
            image_enhance_tv = image_enhance;
        else
            image_enhance_ir = image_enhance;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): image_enhance (image_enhance=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, image_enhance);
        {
            uint8_t params[1] = {image_enhance};
            //write_command(0x0E, SIZEOF(params), params);
            write_command_new(0x0E, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): image_enhance_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x09:
    {
        int byte_index = 0;
        uint8_t picture_in_picture;
        PARSE_DATA_LE(params, &picture_in_picture);
        this->picture_in_picture = picture_in_picture;
        //闁汇垼顔婇懙鎴︽偨鐠佸磭鐟濋柛娆愵殘閸庡綊宕撹箛鎾虫珯闁烩晜娼欐總鏍传瀹勫攨cture-in-picture is not affected by thermal image collimation
        switch(picture_in_picture)
        {
            case 0x01:
            case 0x02:
            case 0x04:
            {
                this->picture_in_picture = picture_in_picture;
            }
                break;
            case 0x03:
            {
                this->picture_in_picture = picture_in_picture;
                picture_in_picture_position = (picture_in_picture_position + 1) % 4;
            }
                break;
            case 0x10:
            {
                cross_pip_en = 0;
            }
                break;
            case 0x11:
            {
                cross_pip_en = 1;
            }
                break;
            default:
                break;
        } 
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): picture_in_picture (picture_in_picture=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, picture_in_picture);

        {
            uint8_t params[1] = {picture_in_picture};
            //write_command(0x0F, SIZEOF(params), params);
            write_command_new(0x0F, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): picture_in_picture_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x0A:
    {
        int byte_index = 0;
        uint8_t view_scale;
        PARSE_DATA_LE(params, &view_scale);
        if(this->view_scale != view_scale)
        {
            view_scale_changed = true;
            this->view_scale = view_scale;   
        }    
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): view_scale (view_scale=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, view_scale);

        {
            uint8_t params[1] = {view_scale};
            //write_command(0x10, SIZEOF(params), params);
            write_command_new(0x10, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): view_scale_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x0B:
    {
        int byte_index = 0;
        uint8_t ir_align;
        PARSE_DATA_LE(params, &ir_align);
        int16_t x;
        PARSE_DATA_LE(params, &x);
        int16_t y;
        PARSE_DATA_LE(params, &y);
        this->ir_align = (ir_align == 0x01);

        cross_x_pip = x;
        cross_y_pip = y;
//        printf("x = %d, y = %d\n", x, y);
//        if(x==0&&y==0)
//        {
//            cross_x_pip = 960;
//            cross_y_pip = 540;
//        }
        cross_x_pip = (cross_x_pip < 1010) ? ((cross_x_pip > 910) ? cross_x_pip : 910) : 1010;  
        cross_y_pip = (cross_y_pip < 590)  ? ((cross_y_pip > 490) ? cross_y_pip : 490) : 590; 
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): infrared_image_collimation_mode (enter=0x%02X, x=%d, y=%d)", read_command_print_every / 1000, name.c_str(), visit_time, enter, (int) x, (int) y);

        {
            uint8_t params[1] = {ir_align};
            //write_command(0x11, SIZEOF(params), params);
            write_command_new(0x11, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): infrared_image_collimation_mode_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x0C:
    {
        int byte_index = 0;
        uint8_t tv_align;
        PARSE_DATA_LE(params, &tv_align);
        this->tv_align = (tv_align == 0x01);
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): tv_image_collimation_mode (enter=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, enter);

        {
            uint8_t params[1] = {tv_align};
            // write_command(0x12, SIZEOF(params), params);
            write_command_new(0x12, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): tv_image_collimation_mode_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x0D:
    {
        int byte_index = 0;
        uint8_t wave_gate_width;
        PARSE_DATA_LE(params, &wave_gate_width);
        switch(wave_gate_width)
        {
            case 0x01:
                if(wave_gate_size < 6)
                    ++wave_gate_size;
                // wave_gate_width_scale = wave_gate_width_scale * 1.075;
                break;
            case 0x02:
                if(wave_gate_size > 0)
                    --wave_gate_size;
                // wave_gate_width_scale = wave_gate_width_scale * 0.903;
                break;
            case 0x03:
                wave_gate_width_scale = 2.0;
                wave_gate_size_changed = true;
                break;
            case 0x04:
                wave_gate_width_scale = 1.0;
                wave_gate_size_changed = true;
                break;
            case 0x05:
                wave_gate_width_scale = 0.5;
                wave_gate_size_changed = true;
                break;
            default:
                break;
        }
        if(!wave_gate_size_changed)
        {
            switch(wave_gate_size)
            {
                case 0:
                    wave_gate_width_scale = 0.5;
                    break;
                case 1:
                    wave_gate_width_scale = 0.75;
                    break;
                case 2:
                    wave_gate_width_scale = 1.0;
                    break;
                case 3:
                    wave_gate_width_scale = 1.25;
                    break;            
                case 4:
                    wave_gate_width_scale = 1.5;
                    break;            
                case 5:
                    wave_gate_width_scale = 1.75;
                    break; 
                case 6: 
                    wave_gate_width_scale = 2.0;
                    break;
                default:
                    wave_gate_width_scale = wave_gate_width_scale;
                    break;          
            }
            // wave_gate_width_scale = (wave_gate_width_scale > 3.00) ? 3.00 : ((wave_gate_width_scale < 0.40) ?  0.40 : wave_gate_width_scale);        
            wave_gate_size_changed = true;
        }
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): wave_gate_width (wave_gate_width=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, wave_gate_width);

        {
            uint8_t params[1] = {wave_gate_width};
            //write_command(0x17, SIZEOF(params), params);
            write_command_new(0x17, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): wave_gate_width_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x0E:
    {
        int byte_index = 0;
        uint8_t target_type;
        PARSE_DATA_LE(params, &target_type);
        this->target_type = target_type;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): target_polarity (target_polarity=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, target_polarity);

        {
            uint8_t params[1] = {target_type};
            //write_command(0x13, SIZEOF(params), params);
            write_command_new(0x13, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): target_polarity_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x10:
    {
        int byte_index = 0;
        uint8_t read_product_flag;
        PARSE_DATA_LE(params, &read_product_flag);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): read_product_flag (read_product_flag=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, read_product_flag);

        {
            if(read_product_flag == 0x01)
            {
                uint8_t version_a = 0;	//1		20230906 zzy  20241021 zzy
                uint8_t version_b = 1;	//1		20230906 zzy  20241021 zzy
                uint8_t version_c = 0;	//0		20230906 zzy  20241021 zzy
                uint8_t version_d = 0;	//0		20230906 zzy  20241021 zzy
                uint8_t version_year = 24;      // 23  24
                uint8_t version_month = 11;     // 09  08
                uint8_t version_day = 27;       // 03  16
                uint8_t params[5] = {(uint8_t) ((version_a << 2) | (version_b >> 4)), ((version_b << 4) | (version_c >> 2)), ( (version_c << 6) | version_d), (uint8_t) ((version_year << 1) | (version_month >> 3)), (uint8_t) ((version_month << 5) | version_day)};
                //write_command(0x15, SIZEOF(params), params);
                write_command_new(0x15, SIZEOF(params), params, write_data, &write_data_length);
//                printf("read_product_flag:%d.\n", read_product_flag);
                //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): read_product_flag_response", write_command_print_every / 1000, name.c_str(), visit_time);
            }
        }
    }
        break;
    case 0x11:
    {
        int byte_index = 0;
        uint8_t view_size;
        PARSE_DATA_LE(params, &view_size);
        this->view_size = view_size;
        switch(view_size)
        {
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
                break;
            case 0x06:
                this->ir_mode = true;
                break;
            case 0x07:
                this->ir_mode = false;
                break;
            case 0x08:
                this->tv_mode = true;
                break;
            case 0x09:
                this->tv_mode = false;
                break;
            default:
                break;
        }
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): view_size (view_size=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, view_size);

        {
            // if( (view_size != 6) && (view_size != 7) )
            // {
            uint8_t params[1] = {view_size};
            // write_command(0x18, SIZEOF(params), params);
            write_command_new(0x18, SIZEOF(params), params, write_data, &write_data_length);
            // printf("report 0x18\n");
            // }
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): view_size_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }

    }
        break;
    case 0x12:
    {
        int byte_index = 0;
        uint8_t second_capture;
        PARSE_DATA_LE(params, &second_capture);
        if(this->second_capture != second_capture)
            second_capture_changed = true;
        this->second_capture = second_capture;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): second_capture (second_capture=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, second_capture);

        {
            // uint8_t params[1] = {second_capture};
            // write_command(0x14, SIZEOF(params), params);
            // write_command_new(0x14, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): second_capture_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x13:
    {
        int byte_index = 0;
        int16_t cross_symbol_x;
        PARSE_DATA_LE(params, &cross_symbol_x);
        int16_t cross_symbol_y;
        PARSE_DATA_LE(params, &cross_symbol_y);
        if(this->channel!=1)
        {
            cross_symbol_x = (cross_symbol_x < 360) ? 360 : ((cross_symbol_x > 1560) ?  1560 : cross_symbol_x);
            cross_symbol_y = (cross_symbol_y <  68) ?  68 : ((cross_symbol_y > 1012) ?  1012 : cross_symbol_y);
            cross_symbol_x = (cross_symbol_x-320)/2;
            cross_symbol_y = (cross_symbol_y-28)/2;
            // if(cross_symbol_x < 625 && cross_symbol_x > 15 && cross_symbol_y < 497 && cross_symbol_y > 15)
            // cross_valid = true;
        }
        else
        {
            if(view_scale == 1)
            {
                cross_symbol_x = (cross_symbol_x + cross_x_main)/2;
                cross_symbol_y = (cross_symbol_y + cross_y_main)/2;
            }
            // if(cross_symbol_x < 1880 && cross_symbol_x > 40 && cross_symbol_y < 1050 && cross_symbol_y > 30)
            cross_symbol_x = (cross_symbol_x < 40) ? 40 : ((cross_symbol_x > 1880) ?  1879 : cross_symbol_x);	
            cross_symbol_y = (cross_symbol_y < 40) ? 40 : ((cross_symbol_y > 1040) ?  1039 : cross_symbol_y);
            // cross_valid = true;  
        }
        this->cross_symbol_x = cross_symbol_x;
        this->cross_symbol_y = cross_symbol_y;
        cross_symbol_send = true;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): cross_symbol_position (x=%d, y=%d)", read_command_print_every / 1000, name.c_str(), visit_time, (int) cross_symbol_x, (int) cross_symbol_y);
        //XJ3UARTDevice::instance()->write_command_control(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, cross_symbol_x, cross_symbol_y, 0x01); //20230106
    }
        break;
    case 0x30:
    {
        int byte_index = 0;
        int16_t yaw;
        PARSE_DATA_LE(params, &yaw);
        int16_t pitch;
        PARSE_DATA_LE(params, &pitch);
        int16_t roll;
        PARSE_DATA_LE(params, &roll);
        int16_t east_vel;
        PARSE_DATA_LE(params, &east_vel);
        int16_t north_vel;
        PARSE_DATA_LE(params, &north_vel);
        int16_t zenith_vel;
        PARSE_DATA_LE(params, &zenith_vel);

//        flight_yaw = (float)yaw / 32767.0 * 18000.0;//fang wei pian hang
        flight_yaw = (yaw<=32767)? ((float) yaw/65535.0 * 36000.0):((float)(yaw-360)/65535.0 * 36000.0);
        flight_pitch = (float)pitch / 32767.0 * 18000.0;//fu yang
        flight_roll = (float)roll / 32767.0 * 18000.0;//gun zhuan
        eastward_vel = east_vel;
        northward_vel = north_vel;
        this->zenith_vel = zenith_vel;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): open_target_preview (open_target_preview=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, open_target_preview);
    }
        break;
    // case 0x31:
    // {
    //     int byte_index = 0;
    //     int16_t east_vel;
    //     PARSE_DATA_LE(params, &east_vel);
    //     int16_t north_vel;
    //     PARSE_DATA_LE(params, &north_vel);
    //     int16_t zenith_vel;
    //     PARSE_DATA_LE(params, &zenith_vel);
    //     //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): image_control (image_control=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, image_control);
    //     eastward_vel = east_vel;
    //     northward_vel = north_vel;
    //     this->zenith_vel = zenith_vel;
    //     {
    //         // uint8_t params[1] = {image_control};
    //         //write_command(0x31, SIZEOF(params), params);
    //         // write_command_new(0x31, SIZEOF(params), params, write_data, &write_data_length);
    //         //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): image_control_response", write_command_print_every / 1000, name.c_str(), visit_time);
    //     }
    // }
    //     break;
    case 0x32:
    {
        int byte_index = 0;
        uint8_t video_control;
        PARSE_DATA_LE(params, &video_control);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): video_control (video_control=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, video_control);

        {
            uint8_t params[1] = {video_control};
            //write_command(0x32, SIZEOF(params), params);
            write_command_new(0x32, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): video_control_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x33:
    {
        int byte_index = 0;
        uint8_t full_screen;
        PARSE_DATA_LE(params, &full_screen);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): full_screen (full_screen=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, full_screen);

        {
            uint8_t params[1] = {full_screen};
            //write_command(0x1D, SIZEOF(params), params);
            write_command_new(0x1D, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): full_screen_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x34:
    {
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): multi_target_crop", read_command_print_every / 1000, name.c_str(), visit_time);

        const auto targets = XJ3UARTDevice::instance()->targets;

        {
            uint8_t params[21] = {0};
            int byte_index = 0;
            uint8_t target_count = targets.size();
            STRUCT_DATA_LE(params, &target_count);
            for (int i = 0; i < std::min<int>(5, targets.size()); ++i)
            {
                uint16_t x = targets[i].first;
                STRUCT_DATA_LE(params, &x);
                uint16_t y = targets[i].second;
                STRUCT_DATA_LE(params, &y);
            }
            //write_command(0x1E, SIZEOF(params), params);
            write_command_new(0x1E, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): multi_target_crop_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x35:
    {
        int byte_index = 0;
        uint8_t geo_coordinates_visible;
        PARSE_DATA_LE(params, &geo_coordinates_visible);
        this->geo_coordinates_visible = (geo_coordinates_visible==1);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): geo_coordinates_visible (geo_coordinates_visible=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, geo_coordinates_visible);

        {
            uint8_t params[1] = {geo_coordinates_visible};
            //write_command(0x1F, SIZEOF(params), params);
            write_command_new(0x1F, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): optical_axis_geo_coordinates_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x36:
    {
        int byte_index = 0;
        uint8_t report_multi_target_info;
        PARSE_DATA_LE(params, &report_multi_target_info);
        this->report_multi_target_info = (report_multi_target_info == 1);
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): report_multi_target_info (report_multi_target_info=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, report_multi_target_info);

        {
            uint8_t params[1] = {report_multi_target_info};
            //write_command(0x21, SIZEOF(params), params);
            write_command_new(0x21, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): report_multi_target_info_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x37:
    {
        int byte_index = 0;
        uint8_t target_size_visible;
        PARSE_DATA_LE(params, &target_size_visible);
        this->target_size_visible = (target_size_visible==1);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): target_size_visible (target_size_visible=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, target_size_visible);

        {
            uint8_t params[1] = {target_size_visible};
            // write_command(0x22, SIZEOF(params), params);
            write_command_new(0x22, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): target_size_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x38:
    {
        int byte_index = 0;
        uint8_t tv_filter;
        PARSE_DATA_LE(params, &tv_filter);
        this->tv_filter = tv_filter;

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): tv_filter (tv_filter=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, tv_filter);

        {
            uint8_t params[1] = {tv_filter};
            // write_command(0x24, SIZEOF(params), params);
            write_command_new(0x24, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): tv_filter_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x39:
    {
        int byte_index = 0;
        uint8_t attitude_angle_visible;
        PARSE_DATA_LE(params, &attitude_angle_visible);
        this->attitude_angle_visible = (attitude_angle_visible==1);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): attitude_angle_visible (attitude_angle_visible=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, attitude_angle_visible);

        {
            uint8_t params[1] = {attitude_angle_visible};
            // write_command(0x39, SIZEOF(params), params);
            write_command_new(0x39, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): attitude_angle_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x40:
    {
        int byte_index = 0; 
        uint8_t channel_disp;
        PARSE_DATA_LE(params, &channel_disp);
        this->channel_disp = channel_disp;

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x49:
    {
        int byte_index = 0;
        uint8_t ir_scale;
        PARSE_DATA_LE(params, &ir_scale);
        this->ir_scale = ir_scale;

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): ir_scale (ir_scale=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, ir_scale);

        {
            uint8_t params[1] = {ir_scale};
            // write_command(0x49, SIZEOF(params), params);
            write_command_new(0x49, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ir_scale_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x50:
    {
        // DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): read_temperature", read_command_print_every / 1000, name.c_str(), visit_time);
        // int byte_index = 0;
        request_temper = true;
        // {
            // uint8_t params[1] = {23};
            // write_command(0x50, SIZEOF(params), params);
            // write_command_new(0x50, SIZEOF(params), params, write_data, &write_data_length);
            // DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): read_temperature_response", write_command_print_every / 1000, name.c_str(), visit_time);
        // }
    }
        break;
    case 0x51:
    {
        int byte_index = 0;
        uint8_t fov_visible;
        PARSE_DATA_LE(params, &fov_visible);
        this->fov_visible = (fov_visible==1);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): fov_visible (show_view_angle=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, fov_visible);

        {
            uint8_t params[1] = {fov_visible};
            // write_command(0x51, SIZEOF(params), params);
            write_command_new(0x51, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): fov_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }
        break;
    case 0x52:
    {
        int byte_index = 0;
        uint8_t photo_stat;
        PARSE_DATA_LE(params, &photo_stat);
        uint8_t photo_time;
        PARSE_DATA_LE(params, &photo_time_gap);
        uint8_t photo_num;
        PARSE_DATA_LE(params, &photo_num);
        this->photo_stat = photo_stat;
        this->photo_time_gap = photo_time_gap;
        this->photo_num = photo_num;
        {
            // if(photo_stat != 0x03)
            // {
            uint8_t params[1] = {photo_stat};
            // write_command(0x51, SIZEOF(params), params);
            write_command_new(0x52, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): fov_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
            // }
        }
    }
        break;
    case 0x53:
    {
        int byte_index = 0;
        uint8_t track_ID;
        PARSE_DATA_LE(params, &track_ID);        
        track_ID = (track_ID >= 1) ? ((track_ID <= 5) ? track_ID : 5) : 1;  
        this->track_ID = track_ID;
        track_ID_send = true;    
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): reset_fpga", read_command_print_every / 1000, name.c_str(), visit_time, track_ID);
        {
            // uint8_t params[1] = {(uint8_t) (track_command)};
            // HostUARTDevice::instance()->write_command(0x07, SIZEOF(params), params);   
            // DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): fov_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
        // XJ3UARTDevice::instance()->write_command_control(0xFF, (track_ID<<4 | 0x0F), 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF); //20230106
    }    
        break;
    case 0x54:
    {
        int byte_index = 0;
        uint8_t compress_ratio;
        PARSE_DATA_LE(params, &compress_ratio);        

        if(compress_ratio > 0 && compress_ratio < 4)
            this->compress_ratio = compress_ratio;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): reset_fpga", read_command_print_every / 1000, name.c_str(), visit_time, track_ID);
        {
            uint8_t params[1] = {(uint8_t) (compress_ratio)};
            write_command_new(0x54, SIZEOF(params), params, write_data, &write_data_length);   
            // DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): fov_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
    }    
        break;
    // case 0x55:
    // {
    //     int byte_index = 0;
    //     uint8_t take_photo;
    //     PARSE_DATA_LE(params, &take_photo);
    //     this->take_photo = (take_photo == 0x01);
    //     {
    //         uint8_t params[1] = {take_photo};
    //         // write_command(0x51, SIZEOF(params), params);
    //         write_command_new(0x55, SIZEOF(params), params, write_data, &write_data_length);
    //         //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): fov_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
    //     }        
    // }
    case 0x60:
    {
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): reset_fpga", read_command_print_every / 1000, name.c_str(), visit_time);
    }
        break;
    case 0x70:
    {
        int byte_index = 0;
        uint8_t speed;
        PARSE_DATA_LE(params, &speed);
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): set_speed (speed=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, speed);
    }
        break;

    case 0x71:
    {
        int byte_index = 0;
        uint8_t fusion;
        PARSE_DATA_LE(params, &fusion);
        this->fusion = fusion;
        {
            uint8_t params[1] = {fusion};
            // write_command(0x71, SIZEOF(params), params);
            write_command_new(0x71, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): fov_visible_response", write_command_print_every / 1000, name.c_str(), visit_time);
        }
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): set_speed (speed=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, speed);
    }
        break;

    case 0x14:
    {
        int byte_index = 0;
        uint8_t item;
        PARSE_DATA_LE(params, &item);
        uint8_t status;
        PARSE_DATA_LE(params, &status);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): system_init (item=0x%02X, status=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, item, status);

        static const std::map<uint8_t, std::string> item_2_str{{0x11, "TVS"}, {0x12, "IR"}, {0x13, "LDR"}, {0x14, "LDTV"}, {0x15, "LSC"}, {0x16, "LST"}, {0x17, "SERVO"}, {0x21, "MAINSBC"}, {0x22, "ITC"}, {0x23, "AFDX"}, {0x24, "AVT"}, {0x25, "PSB"}, {0x31, "OCU"}, {0x32, "OCU2"}, {0x41, "......"}, {0x42, "PRESS K5 TO DEBUG"}, {0x43, "WAITING FOR DEBUG"}, {0x44, "DATA"}};

        auto has_shown = map_at(init_status, item, 1);   
        if( has_shown < 1){ //
            init_status[item]++;
            auto item_str = map_at(item_2_str, item, "Unknown item");
            static const std::map<uint8_t, std::string> status_2_str{{0x01, "BIT OK"}, {0x02, "BIT FAIL"}, {0x03, "COM OK"}, {0x04, "COM FAIL"}};
            auto status_str = map_at(status_2_str, status, "Unknown status");
            show_str_system_init = (item < 0x40) ? (item_str + " " + status_str) : item_str;
            show_str_system_init = pad_str(show_str_system_init, 17, -1); 
            init_status_cache.push(show_str_system_init);
            this->init_status_count++;
        }
        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_system_init (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_system_init.c_str(), (int) show_str_system_init.size());

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x15:
    {
        int byte_index = 0;
        uint8_t Byte2;
        PARSE_DATA_LE(params, &Byte2);
        uint8_t version_a = (Byte2 >> 2);
        uint8_t Byte_3;
        PARSE_DATA_LE(params, &Byte_3);
        uint8_t version_b = ((Byte2 & 0x03) << 4) | (Byte_3 >> 4);
        uint8_t Byte_4;
        PARSE_DATA_LE(params, &Byte_4);
        uint8_t version_c = ((Byte_3 & 0x0F)<< 2) | (Byte_4 >> 6);
        uint8_t version_d = (Byte_4 & 0x3F);
        uint16_t version_ymd;
        PARSE_DATA_BE(params, &version_ymd);
        uint8_t version_year = (version_ymd & 0xFE00)>>9;
        uint8_t version_month =(version_ymd & 0x01E0)>>5;
        uint8_t version_day = (version_ymd & 0x001F);
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): version (version_major=0x%02X, version_minor=0x%02X, version_year=0x%02X, version_month=0x%02X, version_day=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, version_major, version_minor, version_year, version_month, version_day);

        show_str_version = format("SSDS SW V%d.%d.%d.%d 20%02d %02d %02d", version_a, version_b, version_c, version_d, version_year, version_month, version_day);
        show_str_version = pad_str(show_str_version, 30, 0);
        system_version_start = get_wall_time();
    	system_version_enable = true;

        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_version (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_version.c_str(), (int) show_str_version.size());

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x16:
    {
        int byte_index = 0;
        uint8_t work_mode;
        PARSE_DATA_LE(params, &work_mode);
        this->work_mode = work_mode;
//        std::cout << "device work_mode: " << work_mode  << std::endl;

        if(work_mode != 0x01)
        {
            std::map<uint8_t, int>::iterator iter = init_status.begin();
            while(iter != init_status.end())
            {
                iter->second = -1;
                iter++;
            }
            this->init_status_count = 0;
        }
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): work_mode (work_mode=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, work_mode);
        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_work_mode (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_work_mode.c_str(), (int) show_str_work_mode.size());

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x17:
    {
        int byte_index = 0;
        uint8_t view_size;
        PARSE_DATA_LE(params, &view_size);
        this->sensor_view_size = view_size;
        sensor_view_changed = true;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): view_size (view_size=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, view_size);

        static const std::map<uint8_t, std::string> view_size_2_str{{0x01, "L"}, {0x02, "M"}, {0x03, "S"}, {0x04, "XS"}, {0x05, "Z"}};  // TODO
        show_str_view_size = map_at(view_size_2_str, view_size, "Unknown view size");
        show_str_view_size = pad_str(show_str_view_size, 6, 0);
        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_view_size (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_view_size.c_str(), (int) show_str_view_size.size());

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x18:
    {
        three_update=true;
        int byte_index = 0;
        uint8_t tv_status;
        static bool tv_power_on;
        static bool tv_work_ok = true;
        static uint8_t tv_mode;
        PARSE_DATA_LE(params, &tv_status);
        switch(tv_status)
        {
            case 1:
                tv_power_on = true;
                break;
            case 2:
                tv_power_on = false;
                break;
            case 3:
                tv_work_ok = true;
                break;
            case 4:
                tv_work_ok = false;
                break;
            case 5:
            case 6:
            case 7:
                tv_mode = tv_status;
                break;
            default:
            {
                tv_power_on = true;
                tv_work_ok = true;
                tv_mode = 5;
            }
        }      
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): tv_status (tv_status=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, tv_status);
		if(tv_power_on)
        {
            if(tv_work_ok)
            {
                switch(tv_mode)
                {
                case 0x05:
                    show_str_tv_status = pad_str("TV", 6, -1);
                    break; 
                case 0x06:
                    show_str_tv_status = pad_str("TV  B", 6, -1);
                    break; 
                case 0x07:
                    show_str_tv_status = pad_str("TV  I", 6, -1);
                    break;
                default:
                    show_str_tv_status = pad_str("TV", 6, -1);
                }
            }
            else
            {
                show_str_tv_status= pad_str ("TV x", 6, -1);
            }

        }
        else
            show_str_tv_status = pad_str("", 6, -1);
        // static const std::map<uint8_t, std::string> tv_status_2_str{{0x01, "TV"}, {0x02, "  "}, {0x03, "TV"}, {0x04, "TV x"}, {0x05, "TV"}, {0x06, "TV   B"}, {0x07, "TV   I"}};
        // show_str_tv_status = map_at(tv_status_2_str, tv_status, "Unknown TV status");
        // show_str_tv_status = pad_str(show_str_tv_status, 6, -1);
        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_tv_status (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_tv_status.c_str(), (int) show_str_tv_status.size());

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x19:
    {
        three_update=true;
		int byte_index = 0;
		uint8_t ir_status;
		static bool ir_power_on = true;
		static bool ir_work_ok = true;
		this->ir_work = true;
		static uint8_t ir_mode;
		PARSE_DATA_LE(params, &ir_status);
		switch(ir_status)
		{
			case 1:
            {
				ir_power_on = true;
				this->ir_power = true;
                send_ir_status = true;
            }
				break;
			case 2:
            {
				ir_power_on = false;
				this->ir_power = false;
            }
				break;
			case 3:
            {
				ir_work_ok = true;
				this->ir_work = true;
            }
				break;
			case 4:
            {
				ir_work_ok = false;
				this->ir_work = false;
            }
				break;
			case 5:
			case 6:
				ir_mode = ir_status;
				break;
            case 7:
                break;
			case 8:
				this->ir_cool_finish = true;		//COOL_FINISH
				break;
			default:
				break;
		}
		//DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): ir_status (ir_status=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, ir_status);
		if(ir_power_on)
		{
			if(ir_work_ok)
			{
				switch(ir_mode)
				{
				case 0x05:
					show_str_ir_status = pad_str("IR  B", 6, -1);
					this->ir_pola = 0x05;
					break;
				case 0x06:
					show_str_ir_status = pad_str("IR  W", 6, -1);
					this->ir_pola = 0x06;
					break;
				default:
					show_str_ir_status = pad_str("IR", 6, -1);
					this->ir_pola = 0;
				}
			}
			else
				show_str_ir_status = pad_str("IR x", 6, -1);
		}
		else
		{
			this->ir_cool_finish = false;
			this->ir_power = false;
			this->ir_mode = false;
			show_str_ir_status = pad_str("", 6, -1);
		}
		// static const std::map<uint8_t, std::string> ir_status_2_str{{0x01, "IR"}, {0x02, ""}, {0x03, "IR"}, {0x04, "IR x"}, {0x05, "IR   B"}, {0x06, "IR   W"}, {0x07, ""}, {0x08, ""}};
		// show_str_ir_status = map_at(ir_status_2_str, ir_status, "Unknown IR status");
		// show_str_ir_status = pad_str(show_str_ir_status, 6, -1);
		//DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_ir_status (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_ir_status.c_str(), (int) show_str_ir_status.size());
		{
			uint8_t params[1] = {command_id};
			// write_command(0x04, SIZEOF(params), params);
			write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
			// params[1] = {ir_mode-4};
			// write_command(0x13, SIZEOF(params), params);
			//DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
		}
	}
        break;
    case 0x1A:
    {
        three_update=true;
		int byte_index = 0;
		uint8_t ldr_status;
		static bool ldr_power_on;
		static bool ldr_work_ok = true;
		this->laser_work = true;
		static uint8_t ldr_mode;
		static bool ldr_forbid = true;
//		this->laser_forbid = true;
		PARSE_DATA_LE(params, &ldr_status);
		bool skip_retiming = false;
//		printf("ldr_status %x\n",ldr_status);
		switch(ldr_status)
		{
			case 0x01:
				if(ldr_power_on == true)
					skip_retiming = true;
				ldr_power_on = true;
				this->laser_power = true;
                ldr_mode = ldr_status;
				break;
			case 0x02:
				ldr_power_on = false;
				this->laser_power = false;
				// this->laser_mode = 0x09;
                ldr_mode = ldr_status;
				break;
            case 0x03:
                if (ldr_work_ok == true)
                    skip_retiming = true;
                ldr_work_ok = true;
                this->laser_work = true;
                break;
			case 0x04:
				ldr_work_ok = false;
				this->laser_work = false;
				// this->laser_mode = 0x09;
                ldr_mode = ldr_status;
				break;
            case 0x05:
                ldr_mode = ldr_status;
                break;
            case 0x06:
                ldr_mode = ldr_status;
                break;
			case 0x07:
                ldr_mode = ldr_status;
                break;
			case 0x08:
                ldr_mode = ldr_status;
                break;
			case 0x09:
				ldr_mode = ldr_status;
				break;
			case 0x10:
				if(ldr_forbid == true)
					skip_retiming = true;
				ldr_forbid = true;
				this->laser_forbid = false;
                ldr_mode = ldr_status;
				break;
			case 0x11:
				ldr_forbid = false;
				this->laser_forbid = true;
                ldr_mode = ldr_status;
				break;
			case 0x0a:
				this->laser_alarm = false;
                ldr_mode = ldr_status;
				break;
			case 0x0b:
				this->laser_alarm = true;
                ldr_mode = ldr_status;
				break;
			default:
                break;
		}
		// this->ldr_mode = ldr_mode;
		// DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): ldr_status (ldr_status=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, ldr_status);
        if(ldr_power_on)
        {
//            printf("ldr_mode %x\n",ldr_mode);
            switch(ldr_mode)
            {
                case 1:
                    show_str_ldr_status = pad_str({' ',' ','L'}, 7, -1);
                    this->laser_mode = 0x01;
                    break;
                case 2:
                    show_str_ldr_status = pad_str("", 7, -1);
                    this->laser_mode = 0x02;
                    break;
//                case 3:
//                    show_str_ldr_status = pad_str({' ',' ',' ', ' ','F',}, 7, -1);
//                    this->laser_mode = 0x03;
//                    break;
                case 4:
                    show_str_ldr_status = pad_str({' ',' ',' ', ' ', 'x',}, 7, -1);
                    this->laser_mode = 0x04;
                    break;
                case 5:
                    show_str_ldr_status = pad_str({ASCII_24_ARROW_RIGHT_BAR, ASCII_26_ARROW_RIGHT}, 7, -1);
                    this->laser_mode = 0x05;
                    break;
                case 6:
                    show_str_ldr_status = pad_str({'s',' '}, 7, -1);
                    this->laser_mode = 0x06;
                    break;
                case 7:
                    show_str_ldr_status = pad_str({' ', ' ',' ', '*',}, 7, -1);
                    this->laser_mode = 0x07;
                    if(skip_retiming == false)
                    {
                        this->laser_work_start = get_wall_time();
                        this->laser_work_time = -1;
                    }
                    sjuo=true;
                    break;
                case 8:
                    show_str_ldr_status = pad_str({' ', ' ', ' ', '\x1C'}, 7, -1);
                    this->laser_mode = 0x08;
                    sjuo=true;
                    if(skip_retiming == false)
                    {
                        this->laser_work_start = get_wall_time();
                        this->laser_work_time = -1;
                    }
                    break;
                case 9:
                    show_str_ldr_status = pad_str({ ' ', ' ', ' ', 'n',}, 7, -1);//ASCII_24_ARROW_RIGHT_BAR, ASCII_26_ARROW_RIGHT,
                    this->laser_mode = 0x09;
                    sjuo= false;
                    break;
                case 0x10:
                    show_str_ldr_status = pad_str({ ' ', ' ','t',}, 7, -1);
                    this->laser_mode = 0x10;
                    break;
                case 0x11:
                    show_str_ldr_status = pad_str({' ',' ',ASCII_29_LAZER_FORBID_LAUNCH,}, 7, -1);
                    this->laser_mode = 0x11;
                    break;
                case 0x0a:
                    show_str_ldr_status = pad_str({' ',' ',' ',' ',' ','E',}, 7, -1);
                    this->laser_mode = 0x0a;
                    break;
                case 0x0b:
                    show_str_ldr_status = pad_str({ ' ',' ',' ',' ',' ','H',}, 7, -1);
                    this->laser_mode = 0x0b;
                    break;
                default:
                    show_str_ldr_status = pad_str({ASCII_24_ARROW_RIGHT_BAR, ASCII_26_ARROW_RIGHT,'L',}, 7, -1);
                    this->laser_mode = 0x01;
            }
        }
        else
        {
            show_str_ldr_status = pad_str("", 7, -1);
            ldr_mode = 9;
            // this->ldr_mode = 9;
            this->laser_mode = 0x09;
            this->laser_alarm = false;
            this->laser_forbid = false;
        }

        // static const std::map<uint8_t, std::string> ldr_status_2_str{{0x01, "L"}, {0x02, ""}, {0x03, "L"}, {0x04, "L x"}, {0x05, "\x26\x22L"}, {0x06, "L"}, {0x07, "L *"}, {0x08, "L \x1C"}, {0x09, "L"}, {0x10, "L"}, {0x11, "\x1D"}};  // 0x160x1A: arrow right, 0x1C: five point star, 0x1D: slash on 'L'  // TODO
        // show_str_ldr_status = map_at(ldr_status_2_str, ldr_status, "Unknown LDR status");
        // show_str_ldr_status = pad_str(show_str_ldr_status, 6, -1);
        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_ldr_status (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_ldr_status.c_str(), (int) show_str_ldr_status.size());

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
	}
        break;
    case 0x1B:
    {
        int byte_index = 0;
        uint8_t vcp;
        PARSE_DATA_LE(params, &vcp);
		this->vel_comp = vcp;

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): vcp (vcp=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, vcp);

        static const std::map<uint8_t, std::string> vcp_2_str{{0x01, "+VCP"}, {0x02, "    "}};
        show_str_vcp = map_at(vcp_2_str, vcp, "Unknown VCP");
        show_str_vcp = pad_str(show_str_vcp, 4, 0);
        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_vcp (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_vcp.c_str(), (int) show_str_vcp.size());

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x1C:
    {
        int byte_index = 0;
        uint8_t save_split_position;
        PARSE_DATA_LE(params, &save_split_position);
        this->save_split_position = (save_split_position == 1);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): save_split_position (save_split_position=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, save_split_position);

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x1D:
    {
        int byte_index = 0;
        uint8_t menu;
        PARSE_DATA_LE(params, &menu);
        is_case_0x1d_triggered=true;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): menu (menu=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, menu);

         static std::map<uint8_t, std::vector<std::string> > menu_2_str{
             {0x00, {"FSET", "", "LSET", "", "INFL"}},
             {0xC0, {"POWR", "NUC", "AT\\MA", "MORE", "QUIT"}},
             {0xC3, {"GN+", "GN-", "BR+", "BR-", "QUIT"}},
             {0x4A, {"POWR", "ENAB", "SIMU", "SAFE", "QUIT"}},
             {0x60, {"FSET", "ASET", "LSET", "MORE", "INFL"}},
             {0x6C, {"IMEN1", "IMEN2", "COLR", "PINP", "QUIT"}},
             {0x20, {"VER", "VCPT", "JS-S", "DRFT", "QUIT"}},
             {0x21, {"VER", "VCPT", "JS-N", "DRFT", "QUIT"}},
             {0x6A, {"\x19", "\x18", "", "\x16\x1A", "QUIT"}},  // 0x18: arrow up, 0x19: arrow down, 0x160x1A: arrow right, 0x170x1B: arrow left
             {0x10, {"", "", "", "", "PAUSE"}},
             {0x70, {"FSET", "SSET", "LSET", "MORE", "INFL"}},
             {0x74, {"-SPD", "+SPD", "MRGN", "SAVE", "QUIT"}},
             {0x75, {"LEFT", "", "RGHT", "", "QUIT"}},
             {0x78, {"FSET", "", "LSET", "SYMB", "INFL"}},
             {0x88, {"FSET", "", "LSET", "", "INFL"}},
             {0x38, {"FSET", "", "LSET", "", "INFL"}},
             {0x08, {"FSET", "MAIN", "LSET", "PARM", "CALIB"}},
             {0x0A, {"CFG1", "CFG2", "", "", "QUIT"}},
             {0x09, {"*", "*", "*", "*", "*"}},
             {0x13, {"MORE", "", "ADJ", "MBIT", "QUIT"}},
             {0x0C, {"ENG+", "ENG-", "ENGT", "JOYS", "QUIT"}},
             {0x02, {"*", "*", "*", "*", "*"}},
             {0x1A, {"AXIS", "", "SAVE", "READ", "QUIT"}},
             {0x0E, {"\x19", "\x18","", "\x16\x1A", "QUIT"}},
             {0x18, {"*", "*", "*", "*", "*"}},
             {0x19, {"TVS", "FLIR", "SAVE", "AUTO", "QUIT"}},
             {0x0F, {"\x19", "\x18", "", "\x16\x1A", "QUIT"}},
             {0x11, {"FSET", "", "LSET", "PARM", "CALIB"}},
             {0x03, {"*", "*", "*", "*", "*"}},
             {0x04, {"BKWD", "", "SAVE", "", "QUIT"}},
             {0x05, {"\x19", "\x18", "", "\x16\x1A", "QUIT"}},
             {0x06, {"*", "*", "*", "*", "*"}},
             {0x07, {"AUTO", "", "SAVE", "", "QUIT"}}
         };
         static bool menu_2_str_padded = false;
         if (!menu_2_str_padded)
         {
             for (auto &kv : menu_2_str)
             {
                 std::transform(kv.second.begin(), kv.second.end(), kv.second.begin(), [](const std::string &s) { return pad_str(s, 6, 0); });
             }
             menu_2_str_padded = true;
         }
         show_str_menus = (menu_2_str.find(menu) != menu_2_str.end()) ? menu_2_str.at(menu) : std::vector<std::string>{"Unknow", "Unknow", "Unknow", "Unknow", "Unknow"};
        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_menus (str=['%s','%s','%s','%s','%s'], len=[%d,%d,%d,%d,%d])", write_command_print_every / 1000, name.c_str(), visit_time, show_str_menus[0].c_str(), show_str_menus[1].c_str(), show_str_menus[2].c_str(), show_str_menus[3].c_str(), show_str_menus[4].c_str(), (int) show_str_menus[0].size(), (int) show_str_menus[1].size(), (int) show_str_menus[2].size(), (int) show_str_menus[3].size(), (int) show_str_menus[4].size());

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x1E:
    {
        int byte_index = 0;
        uint8_t show_level;
        PARSE_DATA_LE(params, &show_level);
        this->show_level = show_level;

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): show_level (show_level=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, show_level);

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x1F:
    {
        int byte_index = 0;
        uint8_t show_color;
        PARSE_DATA_LE(params, &show_color);
        this->show_color = show_color;

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): show_color (show_color=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, show_color);

        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x20:
    {
        int byte_index = 0;
        int16_t x1;
        PARSE_DATA_LE(params, &x1);
        int16_t y1;
        PARSE_DATA_LE(params, &y1);
        int16_t x2;
        PARSE_DATA_LE(params, &x2);
        int16_t y2;
        PARSE_DATA_LE(params, &y2);
        cross_x_main = x1;
        cross_y_main = y1;
        cross_x_pip = x2;
        cross_y_pip = y2;

        cross_main_send = true;

        cross_x_main = (cross_x_main < 1010) ? ((cross_x_main > 910) ? cross_x_main : 910) : 1010;  
        cross_y_main = (cross_y_main < 590)  ? ((cross_y_main > 490) ? cross_y_main : 490) : 590; 
        cross_x_pip = (cross_x_pip < 1010) ? ((cross_x_pip > 910) ? cross_x_pip : 910) : 1010;  
        cross_y_pip = (cross_y_pip < 590)  ? ((cross_y_pip > 490) ? cross_y_pip : 490) : 590;  
        if(this->channel == 1)
        {
            this->cross_x_tv = cross_x_main;
            this->cross_y_tv = cross_y_main;
            this->cross_x_ir = cross_x_pip;
            this->cross_y_ir = cross_y_pip;
        }
        else
        {
            this->cross_x_tv = cross_x_pip;
            this->cross_y_tv = cross_y_pip;
            this->cross_x_ir = cross_x_main;
            this->cross_y_ir = cross_y_main;
        }
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): update_split_position (x1=%d, y1=%d, x2=%d, y2=%d)", read_command_print_every / 1000, name.c_str(), visit_time, (int) x1, (int) y1, (int) x2, (int) y2);
        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
            //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): ack (command_id=0x%02X)", write_command_print_every / 1000, name.c_str(), visit_time, command_id);
        }
    }
        break;
    case 0x21:
    {
    	int byte_index = 0;
		uint8_t location_id;
		PARSE_DATA_LE(params, &location_id);
		//-------------------------------------------------------------
//        printf("location_id %x\n",location_id);
//		const uint8_t* raw_data_ptr = params;
//		printf("Raw params bytes (hex): ");
//		for (int i = 0; i < 10; i++) {
//			printf("%02X ", raw_data_ptr[i]); // 按字节打印原始数据
//		}
//		printf("\n");
		//-----------------------------------------------------------
		switch (location_id)
		{
		case 0x01:
		{
//		    // 打印原始数据（16进制格式）
//			const uint8_t* raw_data_ptr = params;
//		    printf("Raw params bytes (hex): ");
//		    for (int i = 0; i < 10; i++) {
//		        printf("%02X ", raw_data_ptr[i]); // 按字节打印原始数据
//		    }
//		    printf("\n");
		    //---------------------------------------------------------------
//			std::string str(8, 0);
//			uint8_t num_tmp;
//			for(int i=0; i<8; i++)
//			{
//				PARSE_DATA_LE(params, &num_tmp);
//				str[i] = format("%d", num_tmp)[0];
//			}
//			this->date_str = std::string(str.c_str());
			std::string str(10, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());
			this->date_str = std::string(str.c_str());

//			// 打印date_str的值
//			std::cout << "Date String: " << this->date_str << std::endl;
//			// 或使用C风格打印
//			printf("Date String: %s\n", this->date_str.c_str());
		}
		break;
		case 0x02:
		{
		    // 打印原始数据（16进制格式）
//			const uint8_t* raw_data_ptr = params;
//		    printf("Raw params bytes (hex): ");
//		    for (int i = 0; i < 10; i++) {
//		        printf("%02X ", raw_data_ptr[i]); // 按字节打印原始数据
//		    }
//		    printf("\n");
		    //---------------------------------------------------------------
//			std::string str(6, 0);
//			uint8_t num_tmp;
//			for(int i=0; i<6; i++)
//			{
//				PARSE_DATA_LE(params, &num_tmp);
//				str[i] = format("%d", num_tmp)[0];
//			}
//			this->time_str = std::string(str.c_str());s
			std::string str(8, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());
			this->time_str = std::string(str.c_str());
//			// 打印date_str的值
//			std::cout << "time_str String: " << this->time_str << std::endl;
//			// 或使用C风格打印
//			printf("time_str String: %s\n", this->time_str.c_str());

		}
		break;
		case 0x03:
		{
//			uint8_t enhance_level;
//			PARSE_DATA_LE(params, &enhance_level);
//			this->enhance_level = enhance_level;
            std::string str(8, 0);
            PARSE_DATA_COPY(params, &str[0], str.size());
            this->pos_eng_3 = std::string(str.c_str());
//            std::cout << "pos_eng_3 str: " << str << std::endl;
		}
		break;
		case 0x04:
		{
//			uint8_t image_enhancement;
//			PARSE_DATA_LE(params, &image_enhancement);
//			this->image_enhancement = image_enhancement;
			// printf("image: %d\n ", image_enhancement);
            std::string str(8, 0);
            PARSE_DATA_COPY(params, &str[0], str.size());
            this->pos_eng_4 = std::string(str.c_str());
//            std::cout << "pos_eng_4 str: " << str << std::endl;
		}
		break;
		case 0x05:
		{
//			uint8_t brigheness_contrast_modify;
//			PARSE_DATA_LE(params, &brigheness_contrast_modify);
//			this->brigheness_contrast_modify = brigheness_contrast_modify;
            std::string str(8, 0);
            // 婵炴挸鎳愰埞锟� str 缁绢収鍠曠换姘柦閳╁啯绠掗柛鎾崇У椤愬ジ寮悧鍫濈ウ鐟滄澘宕幖锟�
            str.clear();
            str.resize(8, 0);  // 闂佹彃绉甸弻濠勬媼閸撗呮瀭濠㈠爢鍐瘓濞戞搫鎷� 8 妤犵偠娉涢敐鐐哄礂閿燂拷 0
            PARSE_DATA_COPY(params, &str[0], str.size());
            this->pos_eng_5 = std::string(str.c_str());
//            std::cout << "pos_eng_5 str: " << str << std::endl;
		}
		break;
		case 0x06:
		{
//			uint8_t inertial_state;
//			PARSE_DATA_LE(params, &inertial_state);
//			this->inertial_state = inertial_state;
            std::string str(8, 0);
            PARSE_DATA_COPY(params, &str[0], str.size());
            this->pos_eng_6 = std::string(str.c_str());
//            std::cout << "pos_eng_6 str: " << str << std::endl;
		}
		break;
		case 0x07:
		{
			std::string str(2, '0');
			PARSE_DATA_COPY(params, &str[0], str.size());
			this->fault_code_str = std::string(str.c_str());
		}
		break;
        case 0x08:
        {
            // std::string str(param_count-1, 0);
            // PARSE_DATA_COPY(params, &str[0], str.size());
            // position_30_str = pad_str(std::string(str.c_str()), 20, -1);
			uint8_t detect_status;
			PARSE_DATA_LE(params, &detect_status);
			this->detect_status = detect_status;            
        }
        break;
		case 0x10:
		{
			uint8_t target_style;
			PARSE_DATA_LE(params, &target_style);
			this->target_style = target_style;
		}
		break;
		case 0x11:
		{
			uint8_t para_collect;
			PARSE_DATA_LE(params, &para_collect);
			this->para_collect = para_collect;
		}
		break;
		case 0x12:
		{
			std::string str(param_count-1, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());
			// this->shine_code_str = std::string(str.c_str());
			// this->shine_code_str = pad_str(this->shine_code_str, 4, -1);
            shine_code_str = pad_str(std::string(str.c_str()), 4, -1);
		}
		break;
		case 0x13:
		{
			std::string str(param_count-1, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());
			// this->shine_time_str = std::string(str.c_str());
			// this->shine_time_str = pad_str(this->shine_time_str, 5, -1);
            shine_time_str = pad_str(std::string(str.c_str()), 5, -1);
		}
		break;
        case 0x14:
        {
            std::string str(param_count-1, 0);
            PARSE_DATA_COPY(params, &str[0], str.size());
            position_22_str = pad_str(std::string(str.c_str()), 20, -1);
        }
        break;
		case 0x09:
		{
			std::string str(param_count-1, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());

            // aircraft_longitude = std::stof(str) / 8.3819032e-8;
			aircraft_longitude_str = pad_str(std::string(str.c_str()), 12, -1);
//			std::cout << "Aircraft Longitude String: '" << aircraft_longitude_str << "'" << std::endl;
			size_t colon_pos = str.find(':');
			std::string number_str;
			if (colon_pos != std::string::npos && colon_pos + 1 < str.size()) {
			    number_str = str.substr(colon_pos + 1);
			} else {
			    number_str = str;  // 容错处理
			}
			// 3. 转换浮点数
			try {
			    aircraft_longitude = std::stof(number_str) / 8.3819032e-8;
			} catch (const std::invalid_argument& e) {
//			    printf("转换失败: 非数字内容 -> %s\n", str.c_str());
			    aircraft_longitude = 0.0;  // 默认值
			}
		}
		break;
		case 0x0a:
		{
//			float latitude;
//			PARSE_DATA_LE(params, &latitude);
//			this->aircraft_latitude_str = format("%.7lf", latitude);
//			this->aircraft_latitude_str = pad_str(this->aircraft_latitude_str, 12, -1);
			std::string str(param_count-1, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());
			// 新增字符串分割逻辑
			    size_t colon_pos = str.find(':');
			    std::string number_str;
			    if (colon_pos != std::string::npos && colon_pos + 1 < str.size()) {
			        number_str = str.substr(colon_pos + 1);
			    } else {
			        number_str = str;  // 容错处理
			    }

			    // 转换逻辑
			    try {
			        aircraft_latitude = std::stof(number_str) / 8.3819032e-8;
			    } catch (const std::invalid_argument& e) {
//			        printf("转换失败: 非数字内容 -> %s\n", str.c_str());
			        aircraft_latitude = 0.0;  // 默认值
			    }
            // aircraft_latitude = std::stof(str) / 8.3819032e-8;
			aircraft_latitude_str = pad_str(std::string(str.c_str()), 12, -1);
		}
		break;
		case 0x0b:
		{
//			uint16_t altitude;
//			PARSE_DATA_LE(params, &altitude);
//			this->aircraft_altitude_str = format("%dm", altitude);
//			this->aircraft_altitude_str = pad_str(this->aircraft_altitude_str, 6, -1);
			std::string str(param_count-1, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());
			// 新增字符串分割逻辑
			    size_t colon_pos = str.find(':');
			    std::string number_str;
			    if (colon_pos != std::string::npos && colon_pos + 1 < str.size()) {
			        number_str = str.substr(colon_pos + 1);
			    } else {
			        number_str = str;  // 容错处理
			    }

			    // 转换逻辑
			    try {
			        aircraft_altitude = (std::stoi(number_str) + 500) * 50;
			    } catch (const std::invalid_argument& e) {
//			        printf("转换失败: 非数字内容 -> %s\n", str.c_str());
			        aircraft_altitude = 0;  // 默认值
			    }
            // aircraft_altitude = (std::stoi(str) + 500) * 50;
			aircraft_altitude_str = pad_str(std::string(str.c_str()), 10, -1);

		}
		break;
        case 0x0c:
        {
            // std::string str(param_count-1, 0);
            // PARSE_DATA_COPY(params, &str[0], str.size());
            // position_31_str = pad_str(std::string(str.c_str()), 20, -1);
		    // 打印原始数据（16进制格式）
//			const uint8_t* raw_data_ptr = params;
//		    printf("Raw params bytes (hex): ");
//		    for (int i = 0; i < 10; i++) {
//		        printf("%02X ", raw_data_ptr[i]); // 按字节打印原始数据
//		    }
//		    printf("\n");
		    //---------------------------------------------------------------
            uint8_t photo_status;
			PARSE_DATA_LE(params, &photo_status);
			this->photo_status = photo_status;
        }
        break;
		case 0x0d:
		{
			std::string str(param_count-1, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());
			// 新增字符串分割逻辑
			    size_t colon_pos = str.find(':');
			    std::string number_str;
			    if (colon_pos != std::string::npos && colon_pos + 1 < str.size()) {
			        number_str = str.substr(colon_pos + 1);
			    } else {
			        number_str = str;  // 容错处理
			    }

			    // 转换逻辑
			    try {
			        object_longitude = std::stof(number_str) / 8.3819032e-8;
			    } catch (const std::invalid_argument& e) {
//			        printf("转换失败: 非数字内容 -> %s\n", str.c_str());
			        object_longitude = 0.0;  // 默认值
			    }
            // object_longitude = std::stof(str) / 8.3819032e-8;
			object_longitude_str = pad_str(std::string(str.c_str()), 12, -1);

		}
		break;
		case 0x0e:
		{
			std::string str(param_count-1, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());
			// 新增字符串分割逻辑
			    size_t colon_pos = str.find(':');
			    std::string number_str;
			    if (colon_pos != std::string::npos && colon_pos + 1 < str.size()) {
			        number_str = str.substr(colon_pos + 1);
			    } else {
			        number_str = str;  // 容错处理
			    }

			    // 转换逻辑
			    try {
			        object_latitude = std::stof(number_str) / 8.3819032e-8;
			    } catch (const std::invalid_argument& e) {
//			        printf("转换失败: 非数字内容 -> %s\n", str.c_str());
			        object_latitude = 0.0;  // 默认值
			    }
            // object_latitude = std::stof(str) / 8.3819032e-8;
			object_latitude_str = pad_str(std::string(str.c_str()), 12, -1);


		}
		break;
		case 0x0f:
		{
			std::string str(param_count-1, 0);
			PARSE_DATA_COPY(params, &str[0], str.size());
			 // 新增字符串分割逻辑
			    size_t colon_pos = str.find(':');
			    std::string number_str;
			    if (colon_pos != std::string::npos && colon_pos + 1 < str.size()) {
			        number_str = str.substr(colon_pos + 1);
			    } else {
			        number_str = str;  // 容错处理
			    }

			    // 转换逻辑
			    try {
			        object_altitude = (std::stoi(number_str) + 500) * 50;
			    } catch (const std::invalid_argument& e) {
//			        printf("转换失败: 非数字内容 -> %s\n", str.c_str());
			        object_altitude = 0;  // 默认值
			    }
            // object_altitude = (std::stoi(str) + 500) * 50;
			object_altitude_str = pad_str(std::string(str.c_str()), 10, -1);

		}
		break;
		default:
			break;
		}
    }
        break;
    case 0x22:
    {
        int byte_index = 0;     
    	this->system_info_start = get_wall_time();
    	this->system_info_enable = true;
    	uint8_t show_type;
    	PARSE_DATA_LE(params, &show_type);
        std::string system_info_str;
    	switch(show_type)
    	{
            case 0:
            {
                uint16_t focus;
                PARSE_DATA_LE(params, &focus);
                system_info_str = format("TV FOCUS: %d", focus);
            }
                break;
            case 1:
            {
                uint16_t focus;
                PARSE_DATA_LE(params, &focus);
                system_info_str = format("IR FOCUS: %d", focus);
            }
                break;
            case 2:
            {
//                uint8_t byte3;
//                PARSE_DATA_LE(params, &byte3);
//                uint8_t byte4;
//                PARSE_DATA_LE(params, &byte4);
//                uint8_t byte3_high = (byte3 & 0xf0) / 16;
//                uint8_t byte3_low = byte3 & 0x0f;
//                uint8_t byte4_high = (byte4 & 0xf0) / 16;
//                uint8_t byte4_low = byte4 & 0x0f;
//                std::string version_str = format("%d.%d%d", byte3_low, byte4_high, byte4_low);
                //---------------------------------------------------------------------------------------------------
                uint8_t byte2;
                PARSE_DATA_LE(params, &byte2);  // 閻熸瑱绲鹃悗锟� byte2


            	uint8_t byte3;
            	PARSE_DATA_LE(params, &byte3);  // 閻熸瑱绲鹃悗锟� byte3
            	uint8_t byte4;
            	PARSE_DATA_LE(params, &byte4);  // 閻熸瑱绲鹃悗锟� byte4
            	uint8_t byte5;
            	PARSE_DATA_LE(params, &byte5);  // 閻熸瑱绲鹃悗锟� byte5
            	uint8_t byte6;
            	PARSE_DATA_LE(params, &byte6);  // 閻熸瑱绲鹃悗锟� byte6

            	// 闁圭粯鍔曡ぐ锟� byte3 闁汇劌瀚伴悵锟�4濞达絽绉撮幏鐗堟媴閿燂拷4濞达綇鎷�
            	uint8_t byte3_high = (byte3 & 0xf0) / 16;
            	uint8_t byte3_low = byte3 & 0x0f;

            	// 闁圭粯鍔曡ぐ锟� byte4 闁汇劌瀚伴悵锟�4濞达絽绉撮幏鐗堟媴閿燂拷4濞达綇鎷�
            	uint8_t byte4_high = (byte4 & 0xf0) / 16;
            	uint8_t byte4_low = byte4 & 0x0f;

            	// 闁圭粯鍔曡ぐ锟� byte5 闁汇劌瀚伴悵锟�7濞达絽绉撮幏鐗堟媴閿燂拷8濞达綇鎷�
            	uint8_t byte5_high = (byte5 & 0x7f); // 濡ゅ偊鎷�7濞达綇鎷�
            	uint8_t byte5_low = (byte5 & 0x7f); // byte5闁汇劌瀚紞锟�7濞达絽绋勭槐婵堬拷鍦仱濡绢垶骞嗛崨顓炴瀸濞戞挸顑勭紞鍡樻媴瀹ュ懐瀹夐悹鍥ュ劙缁嚱yte5濞达絽绨肩紞鍛存焾閵娿儱鐎婚柤鎯у槻缁讹拷

            	uint8_t byte6_low = byte6 & 0x1f;  // 闁圭粯鍔曡ぐ锟� byte6 濞达綇鎷�5濞达綇鎷�

            	// 闁圭粯鍔曡ぐ鍥偋閸喐鎷卞ǎ鍥ｅ墲娴硷拷 a, b, c, d
            	uint8_t a = byte3_high;  // 闁绘鐗婂﹢锟� a 闁哄嫸鎷� byte3 闁汇劌瀚伴悵锟�4濞达綇鎷�
            	uint8_t b = ((byte3_low << 4) | (byte4_high));  // 闁绘鐗婂﹢锟� b 闁哄嫸鎷� byte3 濞达綇鎷�4濞达絽绉崇粭锟� byte4 濡ゅ偊鎷�4濞达絽绉撮幃搴ㄧ嵁閿燂拷
            	uint8_t c = ((byte4_low << 2) | (byte5_high >> 5));  // 闁绘鐗婂﹢锟� c 闁哄嫸鎷� byte4 濞达綇鎷�4濞达絽绉崇粭锟� byte5 濡ゅ偊鎷�3濞达絽绉撮幃搴ㄧ嵁閿燂拷
            	uint8_t d = byte5_low;  // 闁绘鐗婂﹢锟� d 闁哄嫸鎷� byte5 闁汇劌瀚紞锟�7濞达綇鎷�

                // 闁哄秶鍘х槐锟犲礌閺嵮勭＝婵炲牓娼х槐锟犳偋閸喐鎷遍悗娑欘殘椤戜焦绋夐敓锟�
                std::string version_str = format("%d.%d.%d.%d", a, b, c, d);
                //-----------------------------------------------------------------------------------------------------
//                std::string version_str = "aaaaa";
//                printf("byte3_high: %u\n", byte3);
//                switch(byte3_high)
                switch(byte2)
                {
                    case 0x01:
                    {
                        system_info_str = std::string("OCU VER: ") + version_str;
                    }
                        break;
                    case 0x02:
                    {
                        system_info_str = std::string("IR VER: ") + version_str;
                    }
                        break;
                    case 0x03:
                    {
                        system_info_str = std::string("TV VER: ") + version_str;
                    }
                        break;
                    case 0x04:
                    {
                        system_info_str = std::string("AVT VER: ") + version_str;
                    }
                        break;
                    case 0x05:
                    {
                        system_info_str = std::string("LD VER: ") + version_str;
                    }
                        break;
                    case 0x0A:
                    {
                        system_info_str = std::string("SER VER: ") + version_str;
                    }
                        break;
                    default:
                        break;
                }
            }
                break;
            case 3:
            {
                std::string str(param_count-1, 0);
                PARSE_DATA_COPY(params, &str[0], str.size());
                system_info_str = std::string(str.c_str());
            }
                break;
            default:
            {
                std::string str(param_count-1, 0);
                PARSE_DATA_COPY(params, &str[0], str.size());
                system_info_str = std::string(str.c_str());                
            }
                break;
        }
        this->show_type = show_type;
        this->system_info_str.clear();
        this->system_info_str = system_info_str;//pad_str(system_info_str, 20, 0);
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): system_info_id (system_info_id=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, system_info_id);
    }
        break;
    case 0x23:
    {
        int byte_index = 0;
        int16_t pitch;
        PARSE_DATA_LE(params, &pitch);
        int16_t yaw;
        PARSE_DATA_LE(params, &yaw);
        //fhww
        this->pitch = pitch;
        this->pitch_str = format("%.2lf", pitch * 1e-2);
        this->pitch_str = pad_str(this->pitch_str, 7, 1);
//        std::cout << "pitch_str"<<this->pitch_str << std::endl;s
        //fuyh
        this->yaw = yaw;
        this->yaw_str = format("%.2lf", yaw * 1e-2);
        this->yaw_str = pad_str(this->yaw_str, 7, 1);
//        std::cout << "yaw_str"<<this->yaw_str << std::endl;


        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): update_yaw_pitch (AZ=%d, EL=%d)", read_command_print_every / 1000, name.c_str(), visit_time, (int) yaw, (int) pitch);
    }
        break;
    case 0x24:
    {
        int byte_index = 0;
        uint16_t distance;
        PARSE_DATA_LE(params, &distance);
        this->distance = distance;
        show_str_attitude_angle_rng = format("RNG: %u", distance);
        show_str_attitude_angle_rng = pad_str(show_str_attitude_angle_rng, 14, -1);

        this->laser_ranging_distance_str = format("%um", distance);
		this->laser_ranging_distance_str = pad_str(this->laser_ranging_distance_str, 7, -1);
//        printf("show_str_attitude_angle_rng: %s\n", show_str_attitude_angle_rng.c_str());
//        printf("laser_ranging_distance_str:%s\n", laser_ranging_distance_str.c_str());
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): update_distance (RNG=%d)", read_command_print_every / 1000, name.c_str(), visit_time, (int) distance);
    }
        break;
    case 0x25:
    {
        int byte_index = 0;
        int16_t yaw_v;
        PARSE_DATA_LE(params, &yaw_v);
        int16_t pitch_v;
        PARSE_DATA_LE(params, &pitch_v);
        
        this->yaw_v = yaw_v;
        this->pitch_v = pitch_v;

        this->yaw_v_str = format("%.2lf", yaw_v * 1e-2);
		this->yaw_v_str = pad_str(this->yaw_v_str, 7, 1);
		this->pitch_v_str = format("%.2lf", pitch_v * 1e-2);
		this->pitch_v_str = pad_str(this->pitch_v_str, 7, 1);
        yaw_pitch_v_send = true;
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): update_yaw_pitch_velocity (yaw_v=%d, pitch_v=%d)", read_command_print_every / 1000, name.c_str(), visit_time, (int) yaw_v, (int) pitch_v);
    }
        break;
    case 0x26:
    {
        int byte_index = 0;
        // int16_t tv_yaw_view_angle;
        // PARSE_DATA_LE(params, &tv_yaw_view_angle);
        // int16_t ir_yaw_view_angle;
        // PARSE_DATA_LE(params, &ir_yaw_view_angle);
		// this->tv_yaw_view_angle = tv_yaw_view_angle;
		// this->ir_yaw_view_angle = ir_yaw_view_angle;
        // if(this->channel == 1)
        //     show_str_fov = format("FOV: %.2lf x %.2lf", tv_yaw_view_angle * 1e-2, tv_yaw_view_angle * 1e-2 * 0.5625);
        // else
        //     show_str_fov = format("FOV: %.2lf x %.2lf", ir_yaw_view_angle * 1e-2, ir_yaw_view_angle * 1e-2 * 0.8);
        // show_str_fov = pad_str(show_str_fov, 18, -1);
        uint8_t channel;
        PARSE_DATA_LE(params, &channel);

        int16_t yaw_view_angle;
        PARSE_DATA_LE(params, &yaw_view_angle);
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): update_yaw_pitch_view_angle (tv_yaw_view_angle=%.2lf, ir_yaw_view_angle=%.2lf)", read_command_print_every / 1000, name.c_str(), visit_time, tv_yaw_view_angle * 1e-2, ir_yaw_view_angle * 1e-2);
        if(channel == 0)
        {
            if(this->tv_yaw_view_angle != yaw_view_angle)
            {
                this->tv_yaw_view_angle = yaw_view_angle;
                yaw_view_angle_changed = true;
            }
            show_str_fov = format("FOV: %.2lf x %.2lf", yaw_view_angle * 1e-2, yaw_view_angle * 1e-2 * 0.5625);
            this->yaw_view_angle_str = format("%.2lf", yaw_view_angle * 1e-2);
            this->yaw_view_angle_str = pad_str(this->yaw_view_angle_str, 6, 1);
        }
        else
        {
            if(this->ir_yaw_view_angle != yaw_view_angle)
            {
                this->ir_yaw_view_angle = yaw_view_angle;
                yaw_view_angle_changed = true;                
            }
            show_str_fov = format("FOV: %.2lf x %.2lf", yaw_view_angle * 1e-2, yaw_view_angle * 1e-2 * 0.8);
            this->yaw_view_angle_str = format("%.2lf", yaw_view_angle * 1e-2);
            this->yaw_view_angle_str = pad_str(this->yaw_view_angle_str, 6, 1);
        }
        show_str_fov = pad_str(show_str_fov, 18, -1);

        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_fov (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_fov.c_str(), (int) show_str_fov.size()); 

        //        std::string str(param_count-1, 0);
        //        PARSE_DATA_COPY(params, &str[0], str.size());
        //        this->yaw_view_angle_str = std::string(str.c_str());
        //        this->yaw_view_angle_str = pad_str(this->yaw_view_angle_str, 6, -1);

    }
        break;
    case 0x27:
    {
    	int byte_index = 0;
		uint16_t north_angle;
		PARSE_DATA_LE(params, &north_angle);
		this->north_angle_str = format("%.2lf", north_angle * 1e-2);
		this->north_angle_str = pad_str(this->north_angle_str, 7, 1);
    }
        break;
    case 0x28:
    {
        int byte_index = 0;
        int16_t length;
        PARSE_DATA_LE(params, &length);
        int16_t width;
        PARSE_DATA_LE(params, &width);
        int16_t height;
        PARSE_DATA_LE(params, &height);

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): target_size (length=%.1lf, width=%.1lf, height=%.1lf)", read_command_print_every / 1000, name.c_str(), visit_time, length * 1e-1, width * 1e-1, height * 1e-1);

        show_str_target_length = format("L: %.1lf", length * 1e-1);
        show_str_target_length = pad_str(show_str_target_length, 14, -1);
        show_str_target_width = format("W: %.1lf", width * 1e-1);
        show_str_target_width = pad_str(show_str_target_width, 14, -1);
        show_str_target_height = format("H: %.1lf", height * 1e-1);
        show_str_target_height = pad_str(show_str_target_height, 14, -1);
        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_target_size (str=['%s','%s','%s'], len=[%d,%d,%d])", write_command_print_every / 1000, name.c_str(), visit_time, show_str_target_length.c_str(), show_str_target_width.c_str(), show_str_target_height.c_str(), (int) show_str_target_length.size(), (int) show_str_target_width.size(), (int) show_str_target_height.size());
    }
        break;
    case 0x29:
    {
        int byte_index = 0;
        // uint8_t head;
        // PARSE_DATA_LE(params, &head);
        std::string str(20, 0);
        PARSE_DATA_COPY(params, &str[0], str.size());

        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): string_2 (str='%s')", read_command_print_every / 1000, name.c_str(), visit_time, str.c_str());

        show_str_string_2 = std::string(str.c_str());
        show_str_string_2 = pad_str(show_str_string_2, 20, 0);
        //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s change show str (index=%d): show_str_string_2 (str='%s', len=%d)", write_command_print_every / 1000, name.c_str(), visit_time, show_str_string_2.c_str(), (int) show_str_string_2.size());
    }
        break;
    case 0x2A:
    {
        int byte_index = 0;
        std::vector<std::tuple<uint8_t, int32_t, int32_t, int32_t> > targets;
        for (int i = 0; i < 5; ++i)
        {
            uint8_t id;
            PARSE_DATA_LE(params, &id);
            int32_t longitude;
            PARSE_DATA_LE(params, &longitude);
            int32_t latitude;
            PARSE_DATA_LE(params, &latitude);
            int32_t distance;
            PARSE_DATA_LE(params, &distance);
            distance &= 0xFFFFFF;
            --byte_index;
            if (id != 0)
            {
                targets.push_back(std::make_tuple(id, longitude, latitude, distance));
            }
        }
        //DO_EVERY_N_MS(read_command_print_every, log_info, "[%ds/p] %s read command (index=%d): show_targets_info (target_size=%d)", read_command_print_every / 1000, name.c_str(), visit_time, (int) targets.size());
    }
        break;
    case 0xEA:
    {
        int byte_index = 0;  
        uint8_t aircraft_id;
        PARSE_DATA_LE(params, &aircraft_id);
        this->aircraft_id = aircraft_id;
        {
            uint8_t params[1] = {command_id};
            // write_command(0x04, SIZEOF(params), params);  
            write_command_new(0x04, SIZEOF(params), params, write_data, &write_data_length);
        }
    }
        break;
    default:
        break;
    }
}

const uint8_t XJ3UARTDevice::read_head[1] = {0xFE};

const uint8_t XJ3UARTDevice::read_tail[1] = {0xC9};

const uint8_t XJ3UARTDevice::write_head[2] = {0xF3, 0xD9};

const uint8_t XJ3UARTDevice::write_tail[1] = {0xEC};

XJ3UARTDevice::XJ3UARTDevice()
    : Device("XJ3", parse_arg("xj3_uart_file", "/dev/irq5_drv")), fd(-1), track_status(1), channel(0), mipi_reset(false), self_check_ok(false), start_ok(false), nano_score(0), 
    prev_peak(0), prev_apce(0), cur_peak(0), cur_apce(0), nano_width(0), nano_height(0), csk_width(0), csk_height(0), strategy_ID(0), pre_cur(0), strategy_status(0), continue_lost(0)
{
    use_interrupt_uart = std::stoi(parse_arg("use_interrupt_uart", "1"));

#ifdef OS_UNIX
    if (use_interrupt_uart)
    {
        init_interrupt_XJ3_uart();
        fd = map_at(interrupt_uart_fds, id, -1);
    }
    else
    {
        fd = ::open(id.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (fd < 0)
        {
            log_critical("Failed to open: '%s'", id.c_str());
            return;
        }

        termios attr;
        tcgetattr(fd, &attr);
        attr.c_cflag |= CLOCAL | CREAD;
        attr.c_cflag &= ~CSIZE;
        cfsetispeed(&attr, B115200);  // baud rate: 115200
        cfsetospeed(&attr, B115200);  // baud rate: 115200
        attr.c_cflag |= CS8;  // data bit: 8
        attr.c_cflag &= ~PARENB;  // no check
        attr.c_iflag &= ~INPCK;  // no check
        attr.c_cflag &= ~CSTOPB;  // stop bit: 1
        attr.c_cc[VTIME] = 1;  // timeout: 0.1s
        attr.c_cc[VMIN] = 0;  // nonblock
        tcflush(fd, TCIOFLUSH);
        tcsetattr(fd, TCSANOW, &attr);
    }
#endif
}

XJ3UARTDevice *XJ3UARTDevice::instance()
{
    static XJ3UARTDevice instance;
    return &instance;
}

bool XJ3UARTDevice::is_open()
{
    return fd >= 0;
}

void XJ3UARTDevice::parse_command(const uint8_t *data, int len, uint8_t *command_id, uint32_t *param_count, uint8_t *params, uint8_t *crc, bool with_head_tail, bool *crc_match)
{
    int byte_index = 0;
    if (with_head_tail)
    {
        byte_index += SIZEOF(read_head);
    }
    PARSE_DATA_BE(data, command_id);
    *param_count = len - 1;
    PARSE_DATA_COPY(data, params, *param_count);
    if (with_head_tail)
    {
        byte_index += SIZEOF(read_tail);
    }
}

void XJ3UARTDevice::struct_command(uint8_t *data, int *len, uint8_t command_id, uint32_t param_count, const uint8_t *params, uint8_t crc, bool with_head_tail)
{
    int byte_index = 0;
    if (with_head_tail)
    {
        STRUCT_DATA_COPY(data, write_head, SIZEOF(write_head));
    }
    // int crc_begin_index = byte_index;
    // STRUCT_DATA_BE(data, &command_id);
    STRUCT_DATA_COPY(data, params, param_count);
    // int crc_end_index = byte_index;
    // STRUCT_DATA_BE(data, &crc);
    if (with_head_tail)
    {
        STRUCT_DATA_COPY(data, write_tail, SIZEOF(write_tail));
    }
    *len = byte_index;
}

int XJ3UARTDevice::write(const uint8_t *data, int len)
{
    int write_len = 0;

#ifdef OS_UNIX
    if (use_interrupt_uart)
    {
        for (int i = 0; i < len; ++i)
        {
            while (Uart_XJ3_TX_HANDLE[0x08 >> 2] & 0x08)
            {
                asm("nop");
                //std::this_thread::sleep_for(std::chrono::microseconds(5)); // //changed zzy 2022/12/17
            }

            Uart_XJ3_TX_HANDLE[1] = data[i];
            asm("nop");
        }

        write_len = len;
    }
    else
    {
        write_len = ::write(fd, data, len);
    }
#endif

    //DO_EVERY_N_MS(read_print_every, log_info, "[%ds/p] %s write (index=%d,len=%d/%d): %s ...", read_print_every / 1000, name.c_str(), visit_time, len, write_len, encode_bytes(std::string(reinterpret_cast<const char *>(data), std::min<int>(64, len))).c_str());

    return write_len;
}

bool XJ3UARTDevice::write_command(uint8_t command_id, uint32_t param_count, const uint8_t *params)
{
    static uint8_t command_data[1024] = {0};
    static int command_data_len = 0;

    struct_command(command_data, &command_data_len, command_id, param_count, params, 0, true);

    int len = write(command_data, command_data_len);

    return len == command_data_len;
}

bool XJ3UARTDevice::write_command_control(uint8_t track_command, uint8_t multi_target_prompt, uint8_t wave_gate_position, uint8_t wave_gate_width, uint8_t second_capture, uint16_t cross_symbol_x, uint16_t cross_symbol_y, uint8_t view)
{
    uint8_t params[10] = {0};
    int byte_index = 0;
    STRUCT_DATA_BE(params, &track_command);
    STRUCT_DATA_BE(params, &multi_target_prompt);
    STRUCT_DATA_BE(params, &wave_gate_position);
    STRUCT_DATA_BE(params, &wave_gate_width);
    STRUCT_DATA_BE(params, &second_capture);
    STRUCT_DATA_BE(params, &cross_symbol_x);
    STRUCT_DATA_BE(params, &cross_symbol_y);
    STRUCT_DATA_BE(params, &view);

    bool ok = write_command(0x00, SIZEOF(params), params);
    //DO_EVERY_N_MS(write_command_print_every, log_info, "[%ds/p] %s write command (index=%d): control (track_command=0x%02X, multi_target_prompt=0x%02X, wave_gate_position=0x%02X, wave_gate_width=0x%02X, second_capture=0x%02X, cross_symbol_x=%d, cross_symbol_y=%d, view=0x%02X)", read_command_print_every / 1000, name.c_str(), visit_time, track_command, multi_target_prompt, wave_gate_position, wave_gate_width, second_capture, cross_symbol_x, cross_symbol_y, view);

    return ok;
}

void XJ3UARTDevice::read_callback()
{
    //data_changed = false;
#ifdef OS_UNIX
    if (!use_interrupt_uart)
    {
        uint8_t data_byte = 0;
        while (true)
        {
            int len = ::read(fd, &data_byte, 1);
            if (len <= 0)
            {
                break;
            }
            read_byte(data_byte);
        }
    }
#endif
}

void XJ3UARTDevice::read_byte(uint8_t data_byte)
{
    static bool print_read_byte = std::stoi(parse_arg("print_read_byte", "0"));
    if (print_read_byte)
    {
        DO_EVERY_N(1, log_info, "%s read byte (index=%d): 0x%02X", name.c_str(), visit_time, data_byte);
    }

    static std::string buffer(1024, 0);
    static uint8_t *data = reinterpret_cast<uint8_t *>(&buffer[0]);
    static int len = 0;
    static int frame_len = 0;

    if ((len == 0) && (data_byte != read_head[0]))
    {
        return;
    }

    data[len] = data_byte;
    ++len;

    if (len < 2) 
    {
        return;
    }
    if (frame_len == 0)
    {
        if(data[1] == 0xD4)
            frame_len = 15; //13
        else if(data[1] == 0xD3)
            frame_len = 45; //37
        else
            frame_len = 32; //37
    }
    if (len < frame_len)
    {
        return;
    }
    read(data, len);
    len = 0;
    frame_len = 0;
}

void XJ3UARTDevice::read(const uint8_t *data, int len)
{
    // DO_EVERY_N_MS(read_print_every, log_info, "[%ds/p] %s read (index=%d,len=%d): %s ...", read_print_every / 1000, name.c_str(), visit_time, len, encode_bytes(std::string(reinterpret_cast<const char *>(data), std::min<int>(64, len))).c_str());
//    std::cout << "XJ3UARTDevice read : ";
//    for (int i = 0; i < len; i++) {
//        std::cout << std::hex << (int) data[i] << " ";
//    }
//    std::cout << std::endl;

    const uint8_t *data_end = data + len;
    while (data < data_end)
    {
        const uint8_t *data_head = std::search(data, data_end, read_head, read_head + SIZEOF(read_head));
        if (!data_head || (data_head >= data_end))
        {
            data = data_end;
            break;
        }
        int command_len = len; 
        const uint8_t *data_tail = std::search(data_head + command_len - SIZEOF(read_tail), data_end, read_tail, read_tail + SIZEOF(read_tail));
        if (!data_tail || (data_tail >= data_end))
        {
            data = data_end;
            break;
        }

        do
        {
            const uint8_t *data = data_head + SIZEOF(read_head);
            int len = data_tail - data;
            // printf("data length:%d \n", len);
            uint8_t command_id = 0;
            uint32_t param_count = 0;
            static uint8_t params[1024] = {0};
            uint8_t crc = 0;
            bool crc_match = true;
            parse_command(data, len, &command_id, &param_count, params, &crc, false, &crc_match);
            // printf("param_count:%d, command_id:%d.\n", param_count, command_id);
            if (!crc_match)
            {
                break;
            }
            read_command(command_id, param_count, params);
        }
        while (false);
        data = data_tail;
    }
}

void XJ3UARTDevice::read_command(uint8_t command_id, uint32_t param_count, const uint8_t *params) {
    std::vector <std::pair<int, int>> targets; // <x, y>
    std::vector <std::pair<int, int>> targets_att; // <score, class_id>
    std::vector <std::pair<int, int>> targets_sz; // <w, h>
    int byte_index = 0;
    uint8_t byte_3;
    PARSE_DATA_BE(params, &byte_3);
    track_status = (byte_3 & 0x0F);
    channel = (byte_3 & 0xF0) >> 4;

    uint8_t byte_4;
    PARSE_DATA_BE(params, &byte_4);
    switch((byte_4 & 0xF0)>>4)
    {
        case 0:
            break;
        case 1:
            start_ok = true; 
            break;
        case 2:
            self_check_ok = true;
            break;
        case 3:
            mipi_reset = true;
            break;
        default:
//            printf("error self_check info!");
            break;
    } 

    targets.resize(8);
    targets_att.resize(8);
    targets_sz.resize(8);
    switch(command_id)
    {
        case 0xD3:
        {        
            for (int i = 0; i < 8; ++i)
            {
                uint16_t x;
                PARSE_DATA_BE(params, &x);
                targets[i].first = (x & 0x07ff);               
                targets_sz[i].first  = ((x & 0xf800) >> 11)*5;
                uint16_t y;
                PARSE_DATA_BE(params, &y);
                targets[i].second = (y & 0x07ff);
                targets_sz[i].second = ((y & 0xf800) >> 11)*5;
                uint8_t z;
                PARSE_DATA_BE(params, &z);
                targets_att[i].first = (z & 0x1f);
                targets_att[i].second = (z & 0xe0) >> 5;
            } 
        }
            break;
        case 0xD5:
        {
            for (int i = 0; i < 8; ++i)
            {
                if(i<3)
                {
                    uint16_t x;
                    PARSE_DATA_BE(params, &x);
                    targets[i].first = x;
                    uint16_t y;
                    PARSE_DATA_BE(params, &y);
                    targets[i].second = y;
                    uint8_t  z;
                    PARSE_DATA_BE(params, &z);
                    targets_att[i].first  = (z & 0x1f);
                    targets_att[i].second = (z & 0xe0) >> 5;
                }
                else
                {
                    targets[i].first = 0; 
                    targets[i].second = 0; 
                }
            } 
            //For tracking debug
            uint8_t score;
            PARSE_DATA_BE(params, &score);
            this->nano_score = score;       
            uint8_t prev_peak;
            PARSE_DATA_BE(params, &prev_peak);
            this->prev_peak = prev_peak;
            uint8_t prev_apce;
            PARSE_DATA_BE(params, &prev_apce);
            this->prev_apce = prev_apce;
            uint8_t cur_peak;
            PARSE_DATA_BE(params, &cur_peak);
            this->cur_peak = cur_peak;
            uint8_t cur_apce;
            PARSE_DATA_BE(params, &cur_apce);
            this->cur_apce = cur_apce;
            uint8_t box_width;
            PARSE_DATA_BE(params, &box_width);
            this->nano_width = box_width;        
            uint8_t box_height;
            PARSE_DATA_BE(params, &box_height);
            this->nano_height = box_height; 
            uint8_t csk_width;
            PARSE_DATA_BE(params, &csk_width);
            this->csk_width = csk_width;        
            uint8_t csk_height;
            PARSE_DATA_BE(params, &csk_height);
            this->csk_height = csk_height; 
            uint8_t strategy_ID;
            PARSE_DATA_BE(params, &strategy_ID);
            this->strategy_ID = strategy_ID; 
            uint8_t byte35;
            PARSE_DATA_BE(params, &byte35);
            this->pre_cur = (byte35 & 0x0F);
            this->strategy_status = (byte35 & 0xF0) >> 4; 
            uint8_t lost_count;
            PARSE_DATA_BE(params, &lost_count);
            this->continue_lost = lost_count; 
        } 
            break;  
        case 0xD4:
        {
            for (int i = 0; i < 8; ++i)
            {
                if(i<2)
                {
                    uint16_t x;
                    PARSE_DATA_BE(params, &x);
                    targets[i].first = x;
                    uint16_t y;
                    PARSE_DATA_BE(params, &y);
                    targets[i].second = y;
                    uint8_t z;
                    PARSE_DATA_BE(params, &z);
                    targets_att[i].first  = (z & 0x0f);
                    targets_att[i].second = (z & 0xf0) >> 4;
                }
                else
                {
                    targets[i].first = 0; 
                    targets[i].second = 0; 
                }
            } 
        }
            break;  
        default:
            break;
    }
    this->targets = targets;
    this->targets_att = targets_att;
    this->targets_sz = targets_sz;  
    if(!(self_check_ok || start_ok || mipi_reset))     
        this->data_changed = true;  
}
