#pragma once

#include <mutex>
#include <vector>

#include "gnn/common/ReportSink.hpp"

namespace gnn {

// 받은 보고를 모아 둔다 (테스트와, 점검 결과를 모으는 용도).
class CollectingSink : public ReportSink {
public:
    void report(const Issue& issue) override;

    // 지금까지 모은 보고의 복사본
    std::vector<Issue> issues() const;
    // severity 가 같은 것의 개수
    std::size_t count(Severity severity) const;
    // code 가 같은 것이 있는가
    bool has(const std::string& code) const;
    void clear();

private:
    mutable std::mutex mutex_;
    std::vector<Issue> issues_;
};

}  // namespace gnn
