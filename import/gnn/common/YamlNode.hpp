#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace gnn {

// YAML 문서의 값 하나 (Null / Scalar / Sequence / Map).
//
// - Scalar 는 문자열로 보관하고 asInt/asDouble/asBool 로 해석한다.
// - Map 은 넣은 순서를 유지한다 (저장한 파일이 매번 같은 순서로 나오도록).
// - 값 타입이므로 복사하면 전체가 복사된다.
class YamlNode {
public:
    enum class Kind { Null, Scalar, Sequence, Map };

    YamlNode() = default;

    // --- 만들기 ---
    // 그대로 출력하는 scalar (숫자, 구간 표기 등).
    static YamlNode makeScalar(std::string text);
    // 문자열. 숫자/true/false/null 처럼 읽힐 수 있으면 따옴표를 붙여 저장한다.
    static YamlNode makeString(std::string text);
    static YamlNode makeInt(std::int64_t value);
    static YamlNode makeDouble(double value);  // NaN/Inf 는 YamlError
    static YamlNode makeBool(bool value);
    static YamlNode makeSequence();
    static YamlNode makeMap();

    // 숫자/true/false/null 로 읽히는 문자열인가
    static bool looksTyped(const std::string& text);

    // --- 종류 ---
    Kind kind() const { return kind_; }
    bool isNull() const { return kind_ == Kind::Null; }
    bool isScalar() const { return kind_ == Kind::Scalar; }
    bool isSequence() const { return kind_ == Kind::Sequence; }
    bool isMap() const { return kind_ == Kind::Map; }

    // --- Scalar ---
    const std::string& text() const;  // Scalar 가 아니면 YamlError
    std::string asString() const { return text(); }
    std::int64_t asInt() const;       // 정수가 아니면 YamlError
    double asDouble() const;          // 실수가 아니면 YamlError
    bool asBool() const;              // true / false 만 허용
    // 파일에서 따옴표로 쓴 값인가 (출력 때 따옴표를 유지하는 데 쓴다)
    bool isQuoted() const { return quoted_; }

    // --- Sequence / Map 공통 ---
    // Sequence 는 항목 수, Map 은 키 수, 나머지는 0
    std::size_t size() const;

    // --- Sequence ---
    void push(YamlNode value);                 // Sequence 가 아니면 YamlError
    const YamlNode& item(std::size_t index) const;
    const std::vector<YamlNode>& items() const;

    // --- Map ---
    // 같은 키가 있으면 값을 바꾸고, 없으면 끝에 추가한다. Map 이 아니면 YamlError
    void set(const std::string& key, YamlNode value);
    bool has(const std::string& key) const;
    const YamlNode* find(const std::string& key) const;  // 없으면 nullptr
    const YamlNode& get(const std::string& key) const;   // 없으면 YamlError
    const std::string& keyAt(std::size_t index) const;
    const YamlNode& valueAt(std::size_t index) const;

    // 구조와 값이 같은가 (따옴표 여부와 주석은 비교하지 않는다)
    bool operator==(const YamlNode& other) const;
    bool operator!=(const YamlNode& other) const { return !(*this == other); }

private:
    void requireKind(Kind expected, const char* what) const;

    Kind kind_ = Kind::Null;
    std::string text_;
    bool quoted_ = false;
    std::vector<YamlNode> items_;                      // Sequence 의 항목, Map 의 값
    std::vector<std::string> keys_;                    // Map 의 키 (items_ 와 같은 순서)
    std::unordered_map<std::string, std::size_t> index_;  // 키 -> 위치
};

}  // namespace gnn
