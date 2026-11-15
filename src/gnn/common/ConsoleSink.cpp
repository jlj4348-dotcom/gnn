#include "gnn/common/ConsoleSink.hpp"

namespace gnn {

void ConsoleSink::report(const Issue& issue) {
    const std::string line = formatIssue(issue);
    std::lock_guard<std::mutex> lock(mutex_);
    stream_ << line << '\n';
    stream_.flush();
}

}  // namespace gnn
