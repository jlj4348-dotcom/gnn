#include "gnn/common/IdCounter.hpp"

#include <limits>
#include <stdexcept>
#include <string>

namespace gnn {

namespace {
constexpr Id kMaxId = std::numeric_limits<Id>::max();
}

IdCounter::IdCounter(Id nextValue) : next_(nextValue) {
    if (nextValue < 1) {
        throw std::invalid_argument("id 카운터의 시작 값은 1 이상이어야 한다: " + std::to_string(nextValue));
    }
}

Id IdCounter::issue() { return issueRange(1); }

Id IdCounter::issueRange(std::int64_t count) {
    if (count < 1) {
        throw std::invalid_argument("발급할 id 개수는 1 이상이어야 한다: " + std::to_string(count));
    }
    // 발급하는 마지막 id(next_ + count - 1)는 kMaxId 보다 작아야 한다.
    // 그래야 다음 값(next_)이 항상 표현 가능하다.
    if (count > kMaxId - next_) {
        throw std::overflow_error("더 발급할 id 가 없다");
    }
    const Id first = next_;
    next_ = first + count;
    return first;
}

void IdCounter::advanceTo(Id value) {
    if (value > next_) next_ = value;
}

}  // namespace gnn
