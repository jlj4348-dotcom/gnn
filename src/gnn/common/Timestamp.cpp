#include "gnn/common/Timestamp.hpp"

#include <ctime>
#include <stdexcept>

namespace gnn {

namespace {

// 스레드에 안전한 gmtime/localtime. 플랫폼 차이는 이 함수 안에만 둔다.
std::tm toCalendar(std::time_t seconds, bool utc) {
    std::tm result{};
#if defined(_WIN32)
    const errno_t error = utc ? gmtime_s(&result, &seconds) : localtime_s(&result, &seconds);
    if (error != 0) throw std::runtime_error("시각을 변환하지 못했다");
#else
    const std::tm* converted = utc ? gmtime_r(&seconds, &result) : localtime_r(&seconds, &result);
    if (converted == nullptr) throw std::runtime_error("시각을 변환하지 못했다");
#endif
    return result;
}

}  // namespace

std::string formatTimestamp(std::chrono::system_clock::time_point time, TimestampStyle style, bool utc) {
    const std::tm calendar = toCalendar(std::chrono::system_clock::to_time_t(time), utc);

    const char* format = "%Y-%m-%d %H:%M:%S";
    switch (style) {
        case TimestampStyle::Compact: format = "%Y%m%d-%H%M%S"; break;
        case TimestampStyle::Iso: format = "%Y-%m-%dT%H:%M:%S"; break;
        case TimestampStyle::Log: format = "%Y-%m-%d %H:%M:%S"; break;
    }
    char buffer[64];
    const std::size_t length = std::strftime(buffer, sizeof(buffer), format, &calendar);
    if (length == 0) throw std::runtime_error("시각 문자열을 만들지 못했다");
    return std::string(buffer, length);
}

std::string currentTimestamp(TimestampStyle style, bool utc) {
    return formatTimestamp(std::chrono::system_clock::now(), style, utc);
}

}  // namespace gnn
