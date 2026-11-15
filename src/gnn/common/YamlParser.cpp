#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "gnn/common/Yaml.hpp"
#include "gnn/common/YamlError.hpp"

namespace gnn {

namespace {

constexpr std::size_t kNpos = std::string::npos;

struct Line {
    int indent;
    std::string text;  // 들여쓰기와 주석을 뺀 내용 (오른쪽 공백도 뺌)
    int number;        // 원본 줄 번호 (1부터)
};

std::string trimRight(const std::string& s) {
    std::size_t end = s.size();
    while (end > 0 && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r')) --end;
    return s.substr(0, end);
}

std::string trim(const std::string& s) {
    std::size_t begin = 0;
    while (begin < s.size() && (s[begin] == ' ' || s[begin] == '\t')) ++begin;
    return trimRight(s.substr(begin));
}

// 따옴표가 "값의 시작"에 있는가 (plain scalar 안의 따옴표와 구분하려고 쓴다)
// - 앞의 공백을 건너뛴 직전 글자가 `[` `{` `,` 이거나 줄의 시작이면 시작이다.
// - 직전 글자가 `:` 또는 줄 맨 앞의 `-` 이면, 둘 사이에 공백이 있을 때만 시작이다.
bool atValueStart(const std::string& s, std::size_t i) {
    std::size_t j = i;
    while (j > 0 && s[j - 1] == ' ') --j;
    if (j == 0) return true;
    const char previous = s[j - 1];
    if (previous == '[' || previous == '{' || previous == ',') return true;
    if (j == i) return false;  // 공백 없이 붙어 있음
    if (previous == ':') return true;
    if (previous == '-') {
        std::size_t k = j - 1;
        while (k > 0 && s[k - 1] == ' ') --k;
        return k == 0;
    }
    return false;
}

// 따옴표 문자열 안의 한 글자를 지나가며 끝을 찾는다. 닫히지 않으면 YamlError.
// start 는 여는 따옴표의 위치이며, 돌려주는 값은 닫는 따옴표 다음 위치이다.
std::size_t skipQuoted(const std::string& s, std::size_t start, int line) {
    const char quote = s[start];
    for (std::size_t i = start + 1; i < s.size(); ++i) {
        if (quote == '"' && s[i] == '\\') {
            ++i;
        } else if (s[i] == quote) {
            if (quote == '\'' && i + 1 < s.size() && s[i + 1] == '\'') {
                ++i;
                continue;
            }
            return i + 1;
        }
    }
    throw YamlError("따옴표가 닫히지 않았다", line);
}

void appendUtf8(std::string& out, std::uint32_t cp) {
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

std::uint32_t parseHex4(const std::string& s, std::size_t pos, int line) {
    if (pos + 4 > s.size()) throw YamlError("\\u 뒤에 16진수 4자리가 필요하다", line);
    std::uint32_t value = 0;
    for (std::size_t i = pos; i < pos + 4; ++i) {
        const char c = s[i];
        value <<= 4;
        if (c >= '0' && c <= '9') value |= static_cast<std::uint32_t>(c - '0');
        else if (c >= 'a' && c <= 'f') value |= static_cast<std::uint32_t>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') value |= static_cast<std::uint32_t>(c - 'A' + 10);
        else throw YamlError("\\u 뒤에 16진수 4자리가 필요하다", line);
    }
    return value;
}

// start 위치의 따옴표 문자열을 읽어 내용을 돌려주고 pos 를 닫는 따옴표 다음으로 옮긴다.
std::string readQuoted(const std::string& s, std::size_t& pos, int line) {
    const char quote = s[pos];
    const std::size_t end = skipQuoted(s, pos, line);
    std::string out;
    for (std::size_t i = pos + 1; i + 1 < end; ++i) {
        const char c = s[i];
        if (quote == '\'') {
            if (c == '\'' && i + 1 < end - 1 && s[i + 1] == '\'') {
                out += '\'';
                ++i;
            } else {
                out += c;
            }
            continue;
        }
        if (c != '\\') {
            out += c;
            continue;
        }
        ++i;
        if (i >= end - 1) throw YamlError("잘못된 이스케이프", line);
        switch (s[i]) {
            case 'n': out += '\n'; break;
            case 't': out += '\t'; break;
            case 'r': out += '\r'; break;
            case '0': out += '\0'; break;
            case '\\': out += '\\'; break;
            case '"': out += '"'; break;
            case '/': out += '/'; break;
            case 'u': {
                std::uint32_t cp = parseHex4(s, i + 1, line);
                i += 4;
                if (cp >= 0xD800 && cp <= 0xDBFF) {
                    if (i + 6 < end - 1 && s[i + 1] == '\\' && s[i + 2] == 'u') {
                        const std::uint32_t low = parseHex4(s, i + 3, line);
                        if (low < 0xDC00 || low > 0xDFFF) throw YamlError("잘못된 surrogate 쌍", line);
                        cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                        i += 6;
                    } else {
                        throw YamlError("잘못된 surrogate 쌍", line);
                    }
                }
                appendUtf8(out, cp);
                break;
            }
            default:
                throw YamlError(std::string("지원하지 않는 이스케이프: \\") + s[i], line);
        }
    }
    pos = end;
    return out;
}

// 줄 끝의 주석(` #...`)을 뗀다. 따옴표 안의 #은 건드리지 않는다.
std::string stripComment(const std::string& s, int line) {
    for (std::size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if ((c == '"' || c == '\'') && atValueStart(s, i)) {
            i = skipQuoted(s, i, line) - 1;
        } else if (c == '#' && (i == 0 || s[i - 1] == ' ' || s[i - 1] == '\t')) {
            return s.substr(0, i);
        }
    }
    return s;
}

bool isSeqItem(const std::string& text) {
    return text == "-" || (text.size() >= 2 && text[0] == '-' && text[1] == ' ');
}

// "키: 값" 에서 키 뒤의 ':' 위치를 찾는다. map 항목이 아니면 npos.
std::size_t findKeyEnd(const std::string& text, int line) {
    if (text.empty()) return kNpos;
    if (text[0] == '"' || text[0] == '\'') {
        std::size_t i = skipQuoted(text, 0, line);
        while (i < text.size() && text[i] == ' ') ++i;
        if (i < text.size() && text[i] == ':' && (i + 1 == text.size() || text[i + 1] == ' ')) return i;
        return kNpos;
    }
    if (text[0] == '{' || text[0] == '[') return kNpos;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == ':' && (i + 1 == text.size() || text[i + 1] == ' ')) return i;
    }
    return kNpos;
}

// plain scalar 를 값으로 바꾼다. 비어 있거나 null/~ 이면 Null.
YamlNode plainToNode(const std::string& text, int line) {
    if (text.empty() || text == "~" || text == "null" || text == "Null" || text == "NULL") {
        return YamlNode();
    }
    const char first = text[0];
    if (first == '&' || first == '*' || first == '!' || first == '|' || first == '>' ||
        first == '%' || first == '@' || first == '`') {
        throw YamlError(std::string("지원하지 않는 YAML 기능(anchor/alias/tag/블록 문자열 등): '") + first +
                            "'", line);
    }
    return YamlNode::makeScalar(text);
}

// flow(`{}`, `[]`) 문자열을 해석한다.
class FlowParser {
public:
    FlowParser(const std::string& text, int line) : s_(text), line_(line) {}

    YamlNode parse() {
        YamlNode value = parseValue();
        skipSpaces();
        if (i_ < s_.size()) throw YamlError("flow 값 뒤에 해석할 수 없는 내용이 있다: '" + s_.substr(i_) + "'", line_);
        return value;
    }

private:
    void skipSpaces() {
        while (i_ < s_.size() && (s_[i_] == ' ' || s_[i_] == '\t')) ++i_;
    }

    YamlNode parseValue() {
        skipSpaces();
        if (i_ >= s_.size()) throw YamlError("값이 필요하다", line_);
        const char c = s_[i_];
        if (c == '{') return parseMap();
        if (c == '[') return parseSequence();
        if (c == '"' || c == '\'') return YamlNode::makeString(readQuoted(s_, i_, line_));
        return parsePlain();
    }

    YamlNode parsePlain() {
        const std::size_t begin = i_;
        while (i_ < s_.size() && s_[i_] != ',' && s_[i_] != '}' && s_[i_] != ']') ++i_;
        return plainToNode(trim(s_.substr(begin, i_ - begin)), line_);
    }

    std::string parseKey() {
        skipSpaces();
        if (i_ >= s_.size()) throw YamlError("키가 필요하다", line_);
        if (s_[i_] == '"' || s_[i_] == '\'') {
            std::string key = readQuoted(s_, i_, line_);
            skipSpaces();
            if (i_ >= s_.size() || s_[i_] != ':') throw YamlError("키 뒤에 ':' 가 필요하다", line_);
            ++i_;
            return key;
        }
        const std::size_t begin = i_;
        while (i_ < s_.size()) {
            const char c = s_[i_];
            if (c == ':' && (i_ + 1 == s_.size() || s_[i_ + 1] == ' ' || s_[i_ + 1] == ',' ||
                             s_[i_ + 1] == '}')) {
                break;
            }
            if (c == ',' || c == '}') throw YamlError("키 뒤에 ':' 가 필요하다", line_);
            ++i_;
        }
        if (i_ >= s_.size()) throw YamlError("키 뒤에 ':' 가 필요하다", line_);
        std::string key = trim(s_.substr(begin, i_ - begin));
        ++i_;  // ':'
        if (key.empty()) throw YamlError("빈 키는 쓸 수 없다", line_);
        return key;
    }

    YamlNode parseMap() {
        ++i_;  // '{'
        YamlNode node = YamlNode::makeMap();
        while (true) {
            skipSpaces();
            if (i_ >= s_.size()) throw YamlError("'}' 가 필요하다", line_);
            if (s_[i_] == '}') {
                ++i_;
                break;
            }
            std::string key = parseKey();
            if (node.has(key)) throw YamlError("같은 키가 두 번 나왔다: '" + key + "'", line_);
            skipSpaces();
            if (i_ < s_.size() && (s_[i_] == ',' || s_[i_] == '}')) {
                node.set(key, YamlNode());
            } else {
                node.set(key, parseValue());
            }
            skipSpaces();
            if (i_ < s_.size() && s_[i_] == ',') ++i_;
        }
        return node;
    }

    YamlNode parseSequence() {
        ++i_;  // '['
        YamlNode node = YamlNode::makeSequence();
        while (true) {
            skipSpaces();
            if (i_ >= s_.size()) throw YamlError("']' 가 필요하다", line_);
            if (s_[i_] == ']') {
                ++i_;
                break;
            }
            node.push(parseValue());
            skipSpaces();
            if (i_ < s_.size() && s_[i_] == ',') ++i_;
        }
        return node;
    }

    const std::string& s_;
    int line_;
    std::size_t i_ = 0;
};

// flow 의 괄호 깊이를 줄 단위로 이어서 센다.
struct FlowScan {
    int depth = 0;
};

void scanFlow(FlowScan& state, const std::string& segment, int line) {
    for (std::size_t i = 0; i < segment.size(); ++i) {
        const char c = segment[i];
        if ((c == '"' || c == '\'') && atValueStart(segment, i)) {
            i = skipQuoted(segment, i, line) - 1;
        } else if (c == '{' || c == '[') {
            ++state.depth;
        } else if (c == '}' || c == ']') {
            --state.depth;
        }
    }
}

class Parser {
public:
    explicit Parser(std::string_view text) { splitLines(text); }

    YamlNode parseDocument() {
        if (lines_.empty()) return YamlNode();
        YamlNode root = parseBlock(lines_[0].indent);
        if (pos_ < lines_.size()) {
            throw YamlError("들여쓰기가 맞지 않거나 해석할 수 없는 줄이다", lines_[pos_].number);
        }
        return root;
    }

private:
    void splitLines(std::string_view text) {
        if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF &&
            static_cast<unsigned char>(text[1]) == 0xBB && static_cast<unsigned char>(text[2]) == 0xBF) {
            text.remove_prefix(3);
        }
        int number = 0;
        std::size_t begin = 0;
        while (begin <= text.size()) {
            std::size_t end = text.find('\n', begin);
            if (end == std::string_view::npos) end = text.size();
            ++number;
            addLine(std::string(text.substr(begin, end - begin)), number);
            begin = end + 1;
        }
    }

    void addLine(const std::string& raw, int number) {
        std::size_t indent = 0;
        while (indent < raw.size() && raw[indent] == ' ') ++indent;
        if (indent < raw.size() && raw[indent] == '\t') {
            throw YamlError("들여쓰기에 탭을 쓸 수 없다", number);
        }
        std::string content = trimRight(stripComment(raw.substr(indent), number));
        if (content.empty()) return;
        if (lines_.empty() && content == "---") return;
        if (content == "---" || content == "...") {
            throw YamlError("문서 구분자는 지원하지 않는다 (YAML 문서는 하나만 쓴다)", number);
        }
        lines_.push_back(Line{static_cast<int>(indent), std::move(content), number});
    }

    // lines_[pos_] 에서 시작하는 값(들여쓰기가 indent)을 읽는다.
    YamlNode parseBlock(int indent) {
        const Line& line = lines_[pos_];
        if (isSeqItem(line.text)) return parseSequence(indent);
        if (findKeyEnd(line.text, line.number) != kNpos) return parseMap(indent);
        const std::string text = line.text;
        return parseInline(text);
    }

    YamlNode parseSequence(int indent) {
        YamlNode node = YamlNode::makeSequence();
        while (pos_ < lines_.size() && lines_[pos_].indent == indent && isSeqItem(lines_[pos_].text)) {
            Line& line = lines_[pos_];
            const std::string rest = line.text == "-" ? std::string() : trim(line.text.substr(2));
            if (rest.empty()) {
                ++pos_;
                if (pos_ < lines_.size() && lines_[pos_].indent > indent) {
                    node.push(parseBlock(lines_[pos_].indent));
                } else {
                    node.push(YamlNode());
                }
            } else if (isSeqItem(rest) || findKeyEnd(rest, line.number) != kNpos) {
                // "- key: value" 는 줄을 "key: value" 로 바꾸고 들여쓰기를 키 위치로 옮겨서 map 으로 읽는다.
                const int offset = static_cast<int>(line.text.size() - rest.size());
                line.indent = indent + offset;
                line.text = rest;
                node.push(parseBlock(line.indent));
            } else {
                node.push(parseInline(rest));
            }
        }
        return node;
    }

    YamlNode parseMap(int indent) {
        YamlNode node = YamlNode::makeMap();
        while (pos_ < lines_.size() && lines_[pos_].indent == indent) {
            const Line& line = lines_[pos_];
            if (isSeqItem(line.text)) break;
            const std::size_t keyEnd = findKeyEnd(line.text, line.number);
            if (keyEnd == kNpos) {
                throw YamlError("'키: 값' 형식이 아니다: '" + line.text + "'", line.number);
            }
            const int number = line.number;
            std::string key = readKey(line.text.substr(0, keyEnd), number);
            const std::string rest = trim(line.text.substr(keyEnd + 1));
            if (node.has(key)) throw YamlError("같은 키가 두 번 나왔다: '" + key + "'", number);

            if (rest.empty()) {
                ++pos_;
                if (pos_ < lines_.size() && lines_[pos_].indent > indent) {
                    node.set(key, parseBlock(lines_[pos_].indent));
                } else if (pos_ < lines_.size() && lines_[pos_].indent == indent &&
                           isSeqItem(lines_[pos_].text)) {
                    node.set(key, parseSequence(indent));  // "key:" 와 같은 들여쓰기의 "- 항목"
                } else {
                    node.set(key, YamlNode());
                }
            } else {
                node.set(key, parseInline(rest));
            }
        }
        return node;
    }

    static std::string readKey(const std::string& raw, int line) {
        const std::string key = trim(raw);
        if (key.empty()) throw YamlError("빈 키는 쓸 수 없다", line);
        if (key[0] == '"' || key[0] == '\'') {
            std::size_t pos = 0;
            return readQuoted(key, pos, line);
        }
        return key;
    }

    // 현재 줄(lines_[pos_])에서 시작하는 한 줄 값을 읽고, 쓴 줄만큼 pos_ 를 넘긴다.
    YamlNode parseInline(const std::string& text) {
        const int number = lines_[pos_].number;
        if (text[0] == '{' || text[0] == '[') {
            std::string buffer = text;
            FlowScan state;
            scanFlow(state, buffer, number);
            while (state.depth > 0) {
                ++pos_;
                if (pos_ >= lines_.size()) throw YamlError("{ 또는 [ 가 닫히지 않았다", number);
                const std::string& next = lines_[pos_].text;
                buffer += ' ';
                buffer += next;
                scanFlow(state, next, lines_[pos_].number);
            }
            if (state.depth < 0) throw YamlError("닫는 괄호가 더 많다", number);
            ++pos_;
            return FlowParser(buffer, number).parse();
        }
        ++pos_;
        if (text[0] == '"' || text[0] == '\'') {
            std::size_t pos = 0;
            std::string content = readQuoted(text, pos, number);
            if (pos != text.size()) {
                throw YamlError("따옴표 문자열 뒤에 해석할 수 없는 내용이 있다: '" + text.substr(pos) + "'",
                                number);
            }
            return YamlNode::makeString(std::move(content));
        }
        return plainToNode(text, number);
    }

    std::vector<Line> lines_;
    std::size_t pos_ = 0;
};

}  // namespace

YamlNode parseYaml(std::string_view text) {
    Parser parser(text);
    return parser.parseDocument();
}

}  // namespace gnn
