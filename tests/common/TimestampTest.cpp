#include <doctest/doctest.h>

#include <chrono>
#include <string>

#include "gnn/common/Timestamp.hpp"

using namespace gnn;

namespace {
// 2026-11-14 15:30:45 UTC
std::chrono::system_clock::time_point sample() {
    return std::chrono::system_clock::from_time_t(1794670245);
}
}  // namespace

TEST_CASE("formatTimestamp: 세 가지 형식 (UTC)") {
    CHECK(formatTimestamp(sample(), TimestampStyle::Compact, true) == "20261114-153045");
    CHECK(formatTimestamp(sample(), TimestampStyle::Iso, true) == "2026-11-14T15:30:45");
    CHECK(formatTimestamp(sample(), TimestampStyle::Log, true) == "2026-11-14 15:30:45");
}

TEST_CASE("formatTimestamp: 문자열 순서가 시간 순서와 같다 (checkpoint 폴더 정렬)") {
    const auto earlier = sample();
    const auto later = earlier + std::chrono::hours(30);
    CHECK(formatTimestamp(earlier, TimestampStyle::Compact, true) <
          formatTimestamp(later, TimestampStyle::Compact, true));
}

TEST_CASE("currentTimestamp: 형식이 맞다") {
    const std::string compact = currentTimestamp(TimestampStyle::Compact);
    CHECK(compact.size() == 15);
    CHECK(compact[8] == '-');
    const std::string iso = currentTimestamp(TimestampStyle::Iso, true);
    CHECK(iso.size() == 19);
    CHECK(iso[10] == 'T');
}
