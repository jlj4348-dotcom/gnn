#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "gnn/common/Issue.hpp"
#include "gnn/common/ReportSink.hpp"

namespace gnn {

// 문제를 보고하는 길을 하나로 모은 객체. 받은 Issue 를 등록된 sink 들에 전달한다.
//
// - 전역 객체가 아니다. 쓰는 쪽에 생성할 때 넘겨서 쓰고, 테스트에서는 독립으로 만든다.
// - 보고한다고 호출 흐름이 바뀌지 않는다. 실패는 반환값(또는 예외)으로도 알려야 한다.
// - 스레드 안전하다. sink 를 부르는 동안에는 잠금을 잡지 않는다.
class Reporter {
public:
    using SinkHandle = std::size_t;

    Reporter() = default;
    Reporter(const Reporter&) = delete;
    Reporter& operator=(const Reporter&) = delete;

    // 콘솔(std::cerr)에 출력하는 sink 하나를 가진 기본 reporter
    static std::shared_ptr<Reporter> makeConsole();

    // sink 를 등록하고 해제용 handle 을 돌려준다. nullptr 은 무시하고 0 을 돌려준다.
    SinkHandle addSink(std::shared_ptr<ReportSink> sink);
    void removeSink(SinkHandle handle);
    std::size_t sinkCount() const;

    void report(const Issue& issue) const;
    void report(Severity severity, std::string code, std::string message,
                std::vector<Id> targets = {}) const;

    void info(std::string code, std::string message, std::vector<Id> targets = {}) const {
        report(Severity::Info, std::move(code), std::move(message), std::move(targets));
    }
    void warning(std::string code, std::string message, std::vector<Id> targets = {}) const {
        report(Severity::Warning, std::move(code), std::move(message), std::move(targets));
    }
    void error(std::string code, std::string message, std::vector<Id> targets = {}) const {
        report(Severity::Error, std::move(code), std::move(message), std::move(targets));
    }

private:
    struct Entry {
        SinkHandle handle;
        std::shared_ptr<ReportSink> sink;
    };

    mutable std::mutex mutex_;
    std::vector<Entry> sinks_;
    SinkHandle nextHandle_ = 1;
};

}  // namespace gnn
