#include "gnn/common/YamlNode.hpp"

#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <utility>

#include "gnn/common/YamlError.hpp"

namespace gnn {

namespace {

// 앞뒤에 공백이 없고 문자열 전체가 숫자로 읽히는가
bool isNumberText(const std::string& text) {
    if (text.empty()) return false;
    const unsigned char first = static_cast<unsigned char>(text[0]);
    if (!(std::isdigit(first) || first == '+' || first == '-' || first == '.')) return false;
    char* end = nullptr;
    errno = 0;
    std::strtod(text.c_str(), &end);
    return end == text.c_str() + text.size();
}

std::string formatDouble(double value) {
    if (std::isnan(value) || std::isinf(value)) {
        throw YamlError("NaN/Inf 는 YAML 로 저장할 수 없다");
    }
    char buffer[64];
    // 다시 읽었을 때 같은 값이 되는 가장 짧은 자릿수를 찾는다.
    for (int precision = 1; precision <= 17; ++precision) {
        std::snprintf(buffer, sizeof(buffer), "%.*g", precision, value);
        if (std::strtod(buffer, nullptr) == value) break;
    }
    return buffer;
}

}  // namespace

YamlNode YamlNode::makeScalar(std::string text) {
    YamlNode node;
    node.kind_ = Kind::Scalar;
    node.text_ = std::move(text);
    return node;
}

bool YamlNode::looksTyped(const std::string& text) {
    return text == "null" || text == "Null" || text == "NULL" || text == "~" ||
           text == "true" || text == "false" || isNumberText(text);
}

YamlNode YamlNode::makeString(std::string text) {
    const bool ambiguous = looksTyped(text);
    YamlNode node = makeScalar(std::move(text));
    node.quoted_ = ambiguous;
    return node;
}

YamlNode YamlNode::makeInt(std::int64_t value) { return makeScalar(std::to_string(value)); }

YamlNode YamlNode::makeDouble(double value) { return makeScalar(formatDouble(value)); }

YamlNode YamlNode::makeBool(bool value) { return makeScalar(value ? "true" : "false"); }

YamlNode YamlNode::makeSequence() {
    YamlNode node;
    node.kind_ = Kind::Sequence;
    return node;
}

YamlNode YamlNode::makeMap() {
    YamlNode node;
    node.kind_ = Kind::Map;
    return node;
}

void YamlNode::requireKind(Kind expected, const char* what) const {
    if (kind_ != expected) {
        throw YamlError(std::string(what) + " 이어야 하는 값이 다른 종류이다");
    }
}

const std::string& YamlNode::text() const {
    requireKind(Kind::Scalar, "문자열/숫자");
    return text_;
}

std::int64_t YamlNode::asInt() const {
    const std::string& value = text();
    if (value.empty() || std::isspace(static_cast<unsigned char>(value[0]))) {
        throw YamlError("정수가 아니다: '" + value + "'");
    }
    char* end = nullptr;
    errno = 0;
    const long long parsed = std::strtoll(value.c_str(), &end, 10);
    if (end != value.c_str() + value.size() || errno == ERANGE) {
        throw YamlError("정수가 아니거나 범위를 벗어났다: '" + value + "'");
    }
    return static_cast<std::int64_t>(parsed);
}

double YamlNode::asDouble() const {
    const std::string& value = text();
    if (value.empty() || std::isspace(static_cast<unsigned char>(value[0]))) {
        throw YamlError("실수가 아니다: '" + value + "'");
    }
    char* end = nullptr;
    errno = 0;
    const double parsed = std::strtod(value.c_str(), &end);
    if (end != value.c_str() + value.size() || errno == ERANGE) {
        throw YamlError("실수가 아니거나 범위를 벗어났다: '" + value + "'");
    }
    return parsed;
}

bool YamlNode::asBool() const {
    const std::string& value = text();
    if (value == "true") return true;
    if (value == "false") return false;
    throw YamlError("true/false 가 아니다: '" + value + "'");
}

std::size_t YamlNode::size() const {
    if (kind_ == Kind::Sequence || kind_ == Kind::Map) return items_.size();
    return 0;
}

void YamlNode::push(YamlNode value) {
    requireKind(Kind::Sequence, "목록");
    items_.push_back(std::move(value));
}

const YamlNode& YamlNode::item(std::size_t index) const {
    requireKind(Kind::Sequence, "목록");
    if (index >= items_.size()) throw YamlError("목록의 범위를 벗어났다: " + std::to_string(index));
    return items_[index];
}

const std::vector<YamlNode>& YamlNode::items() const {
    requireKind(Kind::Sequence, "목록");
    return items_;
}

void YamlNode::set(const std::string& key, YamlNode value) {
    requireKind(Kind::Map, "map");
    const auto found = index_.find(key);
    if (found != index_.end()) {
        items_[found->second] = std::move(value);
        return;
    }
    index_.emplace(key, items_.size());
    keys_.push_back(key);
    items_.push_back(std::move(value));
}

bool YamlNode::has(const std::string& key) const { return find(key) != nullptr; }

const YamlNode* YamlNode::find(const std::string& key) const {
    if (kind_ != Kind::Map) return nullptr;
    const auto found = index_.find(key);
    return found == index_.end() ? nullptr : &items_[found->second];
}

const YamlNode& YamlNode::get(const std::string& key) const {
    requireKind(Kind::Map, "map");
    const YamlNode* value = find(key);
    if (value == nullptr) throw YamlError("키가 없다: '" + key + "'");
    return *value;
}

const std::string& YamlNode::keyAt(std::size_t index) const {
    requireKind(Kind::Map, "map");
    if (index >= keys_.size()) throw YamlError("map 의 범위를 벗어났다: " + std::to_string(index));
    return keys_[index];
}

const YamlNode& YamlNode::valueAt(std::size_t index) const {
    requireKind(Kind::Map, "map");
    if (index >= items_.size()) throw YamlError("map 의 범위를 벗어났다: " + std::to_string(index));
    return items_[index];
}

bool YamlNode::operator==(const YamlNode& other) const {
    if (kind_ != other.kind_) return false;
    switch (kind_) {
        case Kind::Null:
            return true;
        case Kind::Scalar:
            return text_ == other.text_;
        case Kind::Sequence:
            return items_ == other.items_;
        case Kind::Map:
            return keys_ == other.keys_ && items_ == other.items_;
    }
    return false;
}

}  // namespace gnn
