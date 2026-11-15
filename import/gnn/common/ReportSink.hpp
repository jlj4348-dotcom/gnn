#pragma once

#include "gnn/common/Issue.hpp"

namespace gnn {

// Reporter 가 보고를 전달하는 곳 (로그, UI 알림, 테스트 수집 등).
// report 는 여러 스레드에서 불릴 수 있으므로 구현이 스스로 안전해야 한다.
// report 안에서 같은 Reporter 에 다시 보고해도 된다 (교착이 생기지 않는다).
class ReportSink {
public:
    virtual ~ReportSink() = default;
    virtual void report(const Issue& issue) = 0;
};

}  // namespace gnn
