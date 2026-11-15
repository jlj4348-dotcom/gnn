#include "gnn/common/CollectingSink.hpp"

#include <algorithm>

namespace gnn {

void CollectingSink::report(const Issue& issue) {
    std::lock_guard<std::mutex> lock(mutex_);
    issues_.push_back(issue);
}

std::vector<Issue> CollectingSink::issues() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return issues_;
}

std::size_t CollectingSink::count(Severity severity) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<std::size_t>(std::count_if(
        issues_.begin(), issues_.end(), [severity](const Issue& issue) { return issue.severity == severity; }));
}

bool CollectingSink::has(const std::string& code) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::any_of(issues_.begin(), issues_.end(),
                       [&code](const Issue& issue) { return issue.code == code; });
}

void CollectingSink::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    issues_.clear();
}

}  // namespace gnn
