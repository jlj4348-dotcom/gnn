#include "gnn/common/GraphName.hpp"

namespace gnn {

namespace {

constexpr std::size_t kMaxNameBytes = 100;

char toLowerAscii(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; }

bool isWindowsReservedStem(const std::string& stemKey) {
    if (stemKey == "con" || stemKey == "prn" || stemKey == "aux" || stemKey == "nul") return true;
    if (stemKey.size() == 4 && (stemKey.compare(0, 3, "com") == 0 || stemKey.compare(0, 3, "lpt") == 0) &&
        stemKey[3] >= '1' && stemKey[3] <= '9') {
        return true;
    }
    return false;
}

}  // namespace

std::string graphNameKey(const std::string& name) {
    std::string key = name;
    for (char& c : key) c = toLowerAscii(c);
    return key;
}

std::string checkGraphName(const std::string& name, GraphKind kind) {
    if (name.empty()) return "이름이 비어 있다";
    if (name.size() > kMaxNameBytes) return "이름이 너무 길다 (최대 100바이트)";

    for (const char c : name) {
        const unsigned char byte = static_cast<unsigned char>(c);
        if (byte < 0x20 || byte == 0x7F) return "이름에 제어 문자를 쓸 수 없다";
        if (std::string("\\/:*?\"<>|").find(c) != std::string::npos) {
            return std::string("이름에 '") + c + "' 를 쓸 수 없다";
        }
    }
    if (name.front() == ' ') return "이름이 공백으로 시작할 수 없다";
    if (name.back() == ' ') return "이름이 공백으로 끝날 수 없다";
    if (name.back() == '.') return "이름이 마침표로 끝날 수 없다";

    const std::string key = graphNameKey(name);
    const std::string stem = key.substr(0, key.find('.'));
    if (isWindowsReservedStem(stem)) return "Windows 예약 이름은 쓸 수 없다: " + name;

    if (kind == GraphKind::Design && (key == "project.yaml" || key == "operators")) {
        return "Design Graph 이름으로 쓸 수 없는 예약어이다: " + name;
    }
    return std::string();
}

}  // namespace gnn
