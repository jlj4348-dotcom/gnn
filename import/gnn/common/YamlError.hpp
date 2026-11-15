#pragma once

#include <stdexcept>
#include <string>

namespace gnn {

// YAML 읽기/쓰기/파일 처리에서 생기는 오류. 줄 번호를 알면 메시지에 덧붙인다.
class YamlError : public std::runtime_error {
public:
    explicit YamlError(const std::string& message, int line = 0)
        : std::runtime_error(line > 0 ? message + " (줄 " + std::to_string(line) + ")" : message),
          line_(line) {}

    // 오류가 난 줄 (1부터, 모르면 0)
    int line() const { return line_; }

private:
    int line_;
};

}  // namespace gnn
