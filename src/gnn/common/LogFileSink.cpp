#include "gnn/common/LogFileSink.hpp"

#include <stdexcept>
#include <system_error>

#include "gnn/common/Timestamp.hpp"

namespace gnn {

LogFileSink::LogFileSink(const std::filesystem::path& path) {
    if (path.has_parent_path()) {
        std::error_code code;
        std::filesystem::create_directories(path.parent_path(), code);
    }
    stream_.open(path, std::ios::binary | std::ios::app);
    if (!stream_) {
        throw std::runtime_error("로그 파일을 열 수 없다: " + path.u8string());
    }
}

void LogFileSink::report(const Issue& issue) {
    const std::string line = currentTimestamp(TimestampStyle::Log) + " " + formatIssue(issue);
    std::lock_guard<std::mutex> lock(mutex_);
    stream_ << line << '\n';
    stream_.flush();
}

}  // namespace gnn
