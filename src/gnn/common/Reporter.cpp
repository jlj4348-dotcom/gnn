#include "gnn/common/Reporter.hpp"

#include <algorithm>
#include <iostream>

#include "gnn/common/ConsoleSink.hpp"

namespace gnn {

std::shared_ptr<Reporter> Reporter::makeConsole() {
    auto reporter = std::make_shared<Reporter>();
    reporter->addSink(std::make_shared<ConsoleSink>(std::cerr));
    return reporter;
}

Reporter::SinkHandle Reporter::addSink(std::shared_ptr<ReportSink> sink) {
    if (!sink) return 0;
    std::lock_guard<std::mutex> lock(mutex_);
    const SinkHandle handle = nextHandle_++;
    sinks_.push_back(Entry{handle, std::move(sink)});
    return handle;
}

void Reporter::removeSink(SinkHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.erase(std::remove_if(sinks_.begin(), sinks_.end(),
                                [handle](const Entry& entry) { return entry.handle == handle; }),
                 sinks_.end());
}

std::size_t Reporter::sinkCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sinks_.size();
}

void Reporter::report(const Issue& issue) const {
    // sink 가 다시 report 하거나 sink 를 추가/제거해도 안전하도록 목록을 복사해서 잠금 밖에서 부른다.
    std::vector<std::shared_ptr<ReportSink>> targets;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        targets.reserve(sinks_.size());
        for (const Entry& entry : sinks_) targets.push_back(entry.sink);
    }
    for (const auto& sink : targets) sink->report(issue);
}

void Reporter::report(Severity severity, std::string code, std::string message,
                      std::vector<Id> targets) const {
    Issue issue;
    issue.severity = severity;
    issue.code = std::move(code);
    issue.message = std::move(message);
    issue.targets = std::move(targets);
    report(issue);
}

}  // namespace gnn
