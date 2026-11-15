#pragma once

#include <ostream>
#include <string>

namespace gnn {

// graph 의 종류. 파일에서는 `graph.kind` 와 폴더(design/training/inference)로 나타난다.
enum class GraphKind { Design, Training, Inference };

// "design" / "training" / "inference"
const char* toString(GraphKind kind);

// 문자열을 종류로 바꾼다. 알 수 없는 값이면 false (kind 는 바뀌지 않는다).
bool parseGraphKind(const std::string& text, GraphKind& kind);

inline std::ostream& operator<<(std::ostream& stream, GraphKind kind) { return stream << toString(kind); }

}  // namespace gnn
