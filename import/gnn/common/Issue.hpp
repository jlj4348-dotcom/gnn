#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "gnn/common/IdCounter.hpp"

namespace gnn {

enum class Severity {
    Info,     // 알림 (로그에만 남는다)
    Warning,  // 만들 수는 있으나 의심스러움
    Error     // 진행할 수 없음 (예: Training Graph 를 만들 수 없음)
};

// "info" / "warning" / "error"
const char* toString(Severity severity);

inline std::ostream& operator<<(std::ostream& stream, Severity severity) { return stream << toString(severity); }

// 문제 하나. command 결과의 안내, 점검 결과, 실행 중 오류를 모두 이 형식으로 보고한다.
struct Issue {
    Severity severity = Severity::Info;
    // 안정된 이름 (소문자 snake_case, 예: unknown_activation). 한 번 정하면 바꾸지 않는다.
    // 테스트는 message 가 아니라 code 를 기준으로 한다.
    std::string code;
    // 문제가 있는 node/edge id (없으면 비어 있다)
    std::vector<Id> targets;
    std::string message;
};

// 한 줄 텍스트: "[error] unknown_activation: 메시지 (대상: 12, 15)"
std::string formatIssue(const Issue& issue);

}  // namespace gnn
