#if !defined(MS_LOGGER_UTILITIES_H)
#define MS_LOGGER_UTILITIES_H

#include <string>
#include <chrono>
#include <unordered_map>

typedef enum LEVELS {TRACE,DEBUG,INFO,WARN,ERROR} LEVELS;

struct enum_hasher { template <typename T> std::size_t operator()(T t) const { return static_cast<std::size_t>(t); } };
const std::unordered_map<LEVELS, std::string, enum_hasher> uncolored
{
    {ERROR, "[ERROR]\t"}, {WARN, "[WARN]\t"}, {INFO, "[INFO]\t"},
    {DEBUG, "[DEBUG]\t"}, {TRACE, "[TRACE]t"}
};
const std::unordered_map<LEVELS, std::string, enum_hasher> colored
{
    {ERROR, "\x1b[31;1m[ERROR]\x1b[0m\t"}, {WARN, "\x1b[33;1m[WARN]\x1b[0m\t"},
    {INFO, "\x1b[32;1m[INFO]\x1b[0m\t"}, {DEBUG, "\x1b[34;1m[DEBUG]\x1b[0m\t"},
    {TRACE, "\x1b[37;1m[TRACE]\x1b[0m\t"}
};


inline const std::string generate_timestamp() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[23];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X : ", &tstruct);

    return buf;
}

inline const std::string generate_date()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[11];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return buf;
}

#endif // MS_LOGGER_UTILITIES_H
