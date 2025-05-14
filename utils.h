#ifndef IMAGEPROCESSINGSYSTEM_UTILS_UTILS_H
#define IMAGEPROCESSINGSYSTEM_UTILS_UTILS_H

#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>
#include <time.h>
#include <sys/time.h>


#if defined(WIN32) || defined(_WIN32)
#define OS_WIN
#else
#define OS_UNIX
#if defined(__CYGWIN__)
#define OS_CYGWIN
#endif
#endif


#define SIZEOF(x) (sizeof(x) / sizeof((x)[0]))


#define DO_FIRST_N(n, func, ...) do { \
    static int visit_time = 0; \
    ++visit_time; \
    if (visit_time <= (n)) \
    { \
        (func)(__VA_ARGS__); \
    } \
} while (false)

#define DO_EVERY_N(n, func, ...) do { \
    static int visit_time = 0; \
    static int do_time = 0; \
    ++visit_time; \
    if (((n) > 0) && (((n) == 1) || (visit_time % (n) == 1))) \
    { \
        ++do_time; \
        (func)(__VA_ARGS__); \
    } \
} while (false)

#define DO_EVERY_N_MS(n, func, ...) do { \
    static int visit_time = 0; \
    static int do_time = 0; \
    static int last_time = 0; \
    ++visit_time; \
    static const auto begin_time = std::chrono::system_clock::now(); \
    int current_time = static_cast<double>((std::chrono::system_clock::now() - begin_time).count()) * std::chrono::system_clock::duration::period::num / std::chrono::system_clock::duration::period::den * 1e3; \
    if (((n) >= 0) && ((last_time == 0) || (current_time - last_time >= (n)) || (current_time < last_time))) \
    { \
        ++do_time; \
        (func)(__VA_ARGS__); \
        last_time = current_time; \
    } \
} while (false)


static void parse_data(const void *data, void *value, int byte_count, bool is_little_endian)
{
    static bool is_little_endian_system = true;
    static bool is_little_endian_system_calculated = false;
    if (!is_little_endian_system_calculated)
    {
        uint16_t value = 0x00FF;
        uint8_t data[2] = {0x00};
        memcpy(data, &value, 2);
        is_little_endian_system = (data[0] == 0xFF);
        is_little_endian_system_calculated = true;
    }

    assert((byte_count == 1) || (byte_count == 2) || (byte_count == 4) || (byte_count == 8));

    memcpy(value, data, byte_count);

    if (is_little_endian != is_little_endian_system)
    {
        uint8_t temp;
        uint8_t *value_ = (uint8_t *) value;
        for (int byte_index = 0; byte_index < (byte_count / 2); ++byte_index)
        {
            temp = value_[byte_index];
            value_[byte_index] = value_[byte_count - 1 - byte_index];
            value_[byte_count - 1 - byte_index] = temp;
        }
    }
}

static void struct_data(void *data, const void *value, int byte_count, bool is_little_endian)
{
    parse_data(value, data, byte_count, is_little_endian);
}

#define PARSE_DATA(data, value, is_little_endian) parse_data((data) + byte_index, value, sizeof(*(value)), is_little_endian); byte_index += sizeof(*(value))
#define PARSE_DATA_LE(data, value) PARSE_DATA(data, value, true)
#define PARSE_DATA_BE(data, value) PARSE_DATA(data, value, false)

#define STRUCT_DATA(data, value, is_little_endian) struct_data((data) + byte_index, value, sizeof(*(value)), is_little_endian); byte_index += sizeof(*(value))
#define STRUCT_DATA_LE(data, value) STRUCT_DATA(data, value, true)
#define STRUCT_DATA_BE(data, value) STRUCT_DATA(data, value, false)

#define PARSE_DATA_COPY(data, value, len) memcpy(value, (data) + byte_index, len); byte_index += (len)
#define STRUCT_DATA_COPY(data, value, len) memcpy((data) + byte_index, value, len); byte_index += (len)


void set_args(int argc, char **argv);

std::string parse_arg(const std::string &key, std::string default_value = "", bool required = false, const std::vector<std::string> &args = {});

std::string strftime(const std::string &format = "%Y-%m-%dT%H:%M:%S.%%Z", time_t time_ = -1, bool is_ms = true);

std::vector<std::string> split(const std::string &str, char sep);

std::string join(int n, char *strs[], const char *sep);

template<typename Container=std::vector<std::string> >
std::string join(const Container &strs, const std::string &sep)
{
    std::stringstream ss;
    for (int i = 0; i < strs.size(); ++i)
    {
        ss << strs[i];
        if (i != (strs.size() - 1))
        {
            ss << sep;
        }
    }

    return ss.str();
}

bool startswith(const std::string &str, const std::string &start);

bool endswith(const std::string &str, const std::string &end);

std::string format(const char *format, ...);

std::string pad_str(const std::string &str, int len, int align_direction);

std::map<std::string, std::string> parse_string_map(const std::string &str, char sep1 = ';', char sep2 = '=');

template<typename Map>
const typename Map::mapped_type &map_at(const Map &map, const typename Map::key_type &key, const typename Map::mapped_type &default_value = (typename Map::mapped_type()))
{
    return (map.find(key) != map.end()) ? map.at(key) : default_value;
}

template<typename Container=std::vector<std::string>, typename T=std::string>
bool contains(const Container &container, const T &value)
{
    return std::find(container.begin(), container.end(), value) != container.end();
}

std::string encode_bytes(const std::string &v, char sep = ' ', bool with_header = true);
std::string decode_bytes(const std::string &v);

enum LogLevel
{
    Debug,
    Info,
    Warning,
    Critical,
    Fatal
};

void logger_init(const std::string &log_level = "info");

void log_debug(const char *format, ...);

void log_info(const char *format, ...);

void log_warning(const char *format, ...);

void log_critical(const char *format, ...);

void log_fatal(const char *format, ...);

double get_wall_time();

void intToAscii(uint8_t* params, int offset, int x, int len);

#endif
