#pragma once

#include <mutex>
#include <ostream>

#include "gnn/common/ReportSink.hpp"

namespace gnn {

// 보고를 출력 스트림(예: std::cerr)에 한 줄로 쓴다. 스트림은 이 sink 보다 오래 살아야 한다.
class ConsoleSink : public ReportSink {
public:
    explicit ConsoleSink(std::ostream& stream) : stream_(stream) {}

    void report(const Issue& issue) override;

private:
    std::ostream& stream_;
    std::mutex mutex_;
};

}  // namespace gnn
