#include "utils.h"


static int s_argc = 0;

static char **s_argv = nullptr;

void set_args(int argc, char **argv)
{
    s_argc = argc;
    s_argv = argv;
}

std::string parse_arg(const std::string &key, std::string default_value, bool required, const std::vector<std::string> &args)
{
    auto argc = s_argc;
    auto argv = s_argv;
    std::vector<char *> args_;
    if (!args.empty())
    {
        args_.resize(args.size());
        std::transform(args.begin(), args.end(), args_.begin(), [&](const std::string &s) { return const_cast<char *>(s.c_str()); });
        argc = args.size();
        argv = &args_[0];
    }
    if (argc < 0)
    {
        throw std::runtime_error("The s_argc and s_argv should be set before calling parse_arg.");
    }

    auto key_ = "--" + key + "=";
    auto arg = std::find_if(argv, argv + argc, [&](char *arg) { 
        if (arg == nullptr) return false;
        size_t arg_len = strnlen(arg, 4096);  // 限制最大扫描长度为4096
        return (arg_len > key_.size()) && (std::equal(arg, arg + key_.size(), &key_[0])); 
    });
    if (arg < (argv + argc))
    {
        return *arg + key_.size();
    }
    else
    {
        if (required)
        {
            throw std::runtime_error("Command line argument '" + key + "' not found.");
        }
        else
        {
            return default_value;
        }
    }
}

std::string strftime(const std::string &format, time_t time_, bool is_ms)
{
    int64_t time_ms = time_;
    if (time_ == -1)
    {
        if (!is_ms)
        {
            time_ = time(0);
            time_ms = time_ * 1e3;
        }
        else
        {
            time_ms = 1e3 * std::chrono::system_clock::now().time_since_epoch().count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;
            time_ = time_ms * 1e-3;
        }
    }
    else
    {
        if (!is_ms)
        {
            time_ms = time_ * 1e3;
        }
        else
        {
            time_ms = time_;
            time_ = time_ms * 1e-3;
        }
    }

    struct tm *timeinfo = localtime(&time_);
    size_t buffer_size = format.size() * 2;
    char *buffer = new char[buffer_size];
    strftime(buffer, buffer_size, format.c_str(), timeinfo);
    std::string time_string(buffer);
    delete[] buffer;

    auto ms_index = time_string.find("%Z");
    if (ms_index != std::string::npos)
    {
        char buffer[4];
        int ms_value = (int) (time_ms - (int64_t) ((int64_t) (time_ms * 1e-3) * 1e3));
        if (ms_value >= 0 && ms_value < 1000) {
            snprintf(buffer, sizeof(buffer), "%03d", ms_value);
            time_string.replace(ms_index, ms_index + 2, std::string(buffer));
        }
    }

    return time_string;
}

std::vector<std::string> split(const std::string &str, char sep)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string str_;
    while (std::getline(ss, str_, sep))
    {
        result.push_back(str_);
    }

    return result;
}

std::string join(int n, char *strs[], const char *sep)
{
    std::stringstream ss;
    for (int i = 0; i < n; ++i)
    {
        ss << strs[i];
        if (i != (n - 1))
        {
            ss << sep;
        }
    }

    return ss.str();
}

bool startswith(const std::string &str, const std::string &start)
{
    if (str.size() < start.size())
    {
        return false;
    }

    for (auto str_iter = str.begin(), start_iter = start.begin(), start_iter_end = start.end(); start_iter != start_iter_end; ++str_iter, ++start_iter)
    {
        if ((*str_iter) != (*start_iter))
        {
            return false;
        }
    }

    return true;
}

bool endswith(const std::string &str, const std::string &end)
{
    if (str.size() < end.size())
    {
        return false;
    }

    for (auto str_iter = str.end() - 1, end_iter = end.end() - 1, end_iter_begin = end.begin(); end_iter != end_iter_begin; --str_iter, --end_iter)
    {
        if ((*str_iter) != (*end_iter))
        {
            return false;
        }
    }

    return true;
}

std::string format(const char *format, ...)
{
    if (format == nullptr) {
        return std::string();
    }
    size_t format_len = strnlen(format, 10240);  // 限制最大扫描长度为10240
    if (format_len == 0) {
        return std::string();
    }
    size_t buffer_size = format_len * 10;
    char *buffer = new char[buffer_size];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, buffer_size, format, args);
    va_end(args);
    std::string formatted_str(buffer);
    delete[]buffer;

    return formatted_str;
}

std::string pad_str(const std::string &str, int len, int align_direction = 0)
{
    if (str.size() >= len)
    {
        return str;
    }

    int left = (len - str.size()) / 2;
    int right = len - str.size() - left;
    if (align_direction < 0)
    {
        left = 0;
        right = len - str.size();
    }
    else if (align_direction > 0)
    {
        left = len - str.size();
        right = 0;
    }

    return std::string(left, ' ') + str + std::string(right, ' ');
}

std::map<std::string, std::string> parse_string_map(const std::string &str, char sep1, char sep2)
{
    std::map<std::string, std::string> map;
    auto str_split = split(str, sep1);
    for (auto &item : str_split)
    {
        auto item_split = split(item, sep2);
        if (item_split.size() != 2)
        {
            continue;
        }
        map[item_split[0]] = item_split[1];
    }

    return map;
}

std::string encode_bytes(const std::string &v, char sep, bool with_header)
{
    std::stringstream ss;

    if (with_header)
    {
        ss << format("[bytes-hex][len=%d]", v.size());
        if (sep)
        {
            ss << sep;
        }
    }
    for (int i = 0; i < v.size(); ++i)
    {
        if (i)
        {
            ss << sep;
        }
        int8_t c = v[i];
        ss << format("%02X", reinterpret_cast<uint8_t &>(c));
    }

    return ss.str();
}

std::string decode_bytes(const std::string &v)
{
    auto decode_bytes_ = [](std::string &&v) {
        v.resize(std::remove_if(v.begin(), v.end(), [](char c) { return !(((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))); }) - v.begin());
        if (v.size() % 2 != 0)
        {
            v.push_back(v.back());
            v[v.size() - 2] = 0;
        }
        std::stringstream ss;
        for (int i = 0; i < v.size(); i += 2)
        {
            int8_t c = std::stoi(v.substr(i, 2), nullptr, 16);
            ss << reinterpret_cast<uint8_t &>(c);
        }

        return ss.str();
    };

    if (startswith(v, "[bytes-hex][len="))
    {
        int index = v.find(']', 16);  // std::string("[bytes-hex][len=").size() == 16
        if (index < 0)
        {
            return std::string();
        }
        else
        {
            return decode_bytes_(v.substr(index + 1));
        }
    }
    else if (startswith(v, "[bytes-hex]"))
    {
        return decode_bytes_(v.substr(11));  // std::string("[bytes-hex]").size() == 11
    }
    else
    {
        return v;
    }
}

static LogLevel s_log_level = LogLevel::Info;

void logger_init(const std::string &log_level)
{
    const std::map<std::string, LogLevel> level_map{{"debug", LogLevel::Debug}, {"info", LogLevel::Info}, {"warning", LogLevel::Warning}, {"critical", LogLevel::Critical}, {"fatal", LogLevel::Fatal}};

    s_log_level = level_map.at(log_level);
}

void log_debug(const char *format, ...)
{
    if (s_log_level > LogLevel::Debug)
    {
        return;
    }

    printf("[%s] [D] ", strftime().c_str());
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void log_info(const char *format, ...)
{
    if (s_log_level > LogLevel::Info)
    {
        return;
    }

    printf("[%s] [I] ", strftime().c_str());
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void log_warning(const char *format, ...)
{
    if (s_log_level > LogLevel::Warning)
    {
        return;
    }

    printf("[%s] [W] ", strftime().c_str());
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void log_critical(const char *format, ...)
{
    if (s_log_level > LogLevel::Critical)
    {
        return;
    }

    printf("[%s] [C] ", strftime().c_str());
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void log_fatal(const char *format, ...)
{
    if (s_log_level > LogLevel::Fatal)
    {
        return;
    }

    printf("[%s] [F] ", strftime().c_str());
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");

    exit(1);
}

double get_wall_time()
{
	struct timeval time;
	if (gettimeofday(&time, NULL))
	{
		return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void intToAscii(uint8_t* params, int offset, int x, int len)
{
    std::string str = std::to_string(x);
    if (str.length() > len)
    {
        printf("The number is too large.\n");
        return;
    }
    while (str.length() < len)
    {
        if(str[0] == '-')
            str.insert(1, "0");
        else
            str = "0" + str;
    }
    for (int i = 0; i < len; i++)
    {
        params[offset+i] = str[i];
    }
}
