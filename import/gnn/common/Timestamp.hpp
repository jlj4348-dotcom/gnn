#pragma once

#include <chrono>
#include <string>

namespace gnn {

enum class TimestampStyle {
    Compact,  // 20261114-153000  (checkpoint 폴더 이름. 문자열 순서가 시간 순서와 같다)
    Iso,      // 2026-11-14T15:30:00  (파일 안의 시각, 예: Inference Graph 의 taken_at)
    Log       // 2026-11-14 15:30:00  (로그 줄)
};

// 시각을 문자열로 만든다. utc 가 false 이면 이 컴퓨터의 지역 시간대로 바꾼다.
std::string formatTimestamp(std::chrono::system_clock::time_point time, TimestampStyle style,
                            bool utc = false);

// 지금 시각
std::string currentTimestamp(TimestampStyle style, bool utc = false);

}  // namespace gnn
