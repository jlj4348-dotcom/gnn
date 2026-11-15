#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>

#include "gnn/common/ReportSink.hpp"

namespace gnn {

// 보고를 로그 파일 끝에 한 줄씩 덧붙인다: "2026-11-14 15:30:00 [error] code: 메시지 (대상: 1)".
// 파일을 열 수 없으면 생성자가 std::runtime_error 를 던진다. 상위 폴더가 없으면 만든다.
// (로그 파일의 위치와 보관 개수는 정하지 않았다 — 이 sink 는 받은 경로에 쓰기만 한다.)
class LogFileSink : public ReportSink {
public:
    explicit LogFileSink(const std::filesystem::path& path);

    void report(const Issue& issue) override;

private:
    std::ofstream stream_;
    std::mutex mutex_;
};

}  // namespace gnn
