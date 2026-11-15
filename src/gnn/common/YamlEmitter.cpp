#include <cstddef>
#include <cstdio>
#include <string>

#include "gnn/common/Yaml.hpp"
#include "gnn/common/YamlError.hpp"

namespace gnn {

namespace {

// 따옴표 없이 쓰면 다시 읽을 때 다른 값이 되거나 해석이 안 되는 문자열인가
bool needsQuote(const std::string& text, bool inFlow) {
    if (text.empty()) return true;
    if (text.front() == ' ' || text.back() == ' ') return true;
    for (const char c : text) {
        if (c == '\n' || c == '\r' || c == '\t' || static_cast<unsigned char>(c) < 0x20) return true;
    }
    const char first = text.front();
    const std::string reservedFirst = "{}[]&*!|>'\"%@`#,";
    if (reservedFirst.find(first) != std::string::npos) return true;
    if ((first == '-' || first == '?' || first == ':') && (text.size() == 1 || text[1] == ' ')) return true;
    if (text.find(": ") != std::string::npos || text.find(" #") != std::string::npos) return true;
    if (text.back() == ':') return true;
    if (inFlow && text.find_first_of(",[]{}") != std::string::npos) return true;
    return false;
}

std::string quote(const std::string& text) {
    std::string out = "\"";
    for (const char c : text) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buffer[8];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(c));
                    out += buffer;
                } else {
                    out += c;
                }
        }
    }
    out += '"';
    return out;
}

std::string scalarText(const YamlNode& node, bool inFlow) {
    if (node.isNull()) return "null";
    const std::string& text = node.text();
    if (node.isQuoted() || needsQuote(text, inFlow) || (!node.isQuoted() && text == "null")) {
        return quote(text);
    }
    return text;
}

std::string keyText(const std::string& key) {
    return needsQuote(key, true) ? quote(key) : key;
}

// node 를 한 줄(flow)로 쓴다. 길이가 budget 을 넘으면 false.
bool tryFlow(const YamlNode& node, std::string& out, std::size_t budget) {
    if (node.isNull() || node.isScalar()) {
        out += scalarText(node, true);
        return out.size() <= budget;
    }
    if (node.isSequence()) {
        out += '[';
        for (std::size_t i = 0; i < node.size(); ++i) {
            if (i > 0) out += ", ";
            if (!tryFlow(node.item(i), out, budget)) return false;
            if (out.size() > budget) return false;
        }
        out += ']';
        return out.size() <= budget;
    }
    out += '{';
    for (std::size_t i = 0; i < node.size(); ++i) {
        if (i > 0) out += ", ";
        out += keyText(node.keyAt(i));
        out += ": ";
        if (!tryFlow(node.valueAt(i), out, budget)) return false;
        if (out.size() > budget) return false;
    }
    out += '}';
    return out.size() <= budget;
}

// 안쪽에 "map 이나 목록을 항목으로 가진 목록"이 있는가.
// 이런 목록은 한 줄에 들어가도 항목마다 한 줄씩 쓴다 (nodes, edges 처럼 읽기 쉽게).
bool hasSequenceOfCollections(const YamlNode& node) {
    if (node.isSequence()) {
        for (std::size_t i = 0; i < node.size(); ++i) {
            const YamlNode& item = node.item(i);
            if ((item.isMap() || item.isSequence()) && item.size() > 0) return true;
            if (hasSequenceOfCollections(item)) return true;
        }
    } else if (node.isMap()) {
        for (std::size_t i = 0; i < node.size(); ++i) {
            if (hasSequenceOfCollections(node.valueAt(i))) return true;
        }
    }
    return false;
}

class Emitter {
public:
    explicit Emitter(std::size_t flowLimit) : flowLimit_(flowLimit) {}

    std::string emit(const YamlNode& root) {
        std::string out;
        if (root.isMap() && root.size() > 0) {
            emitMap(root, 0, out);
        } else if (root.isSequence() && root.size() > 0) {
            emitSequence(root, 0, out);
        } else {
            std::string flow;
            tryFlow(root, flow, static_cast<std::size_t>(-1));
            out = flow + "\n";
        }
        return out;
    }

private:
    // 앞에 이미 prefixWidth 글자가 있을 때 node 가 한 줄에 들어가면 flow 문자열을 채우고 true.
    bool fitsInLine(const YamlNode& node, std::size_t prefixWidth, std::string& flow) const {
        if (prefixWidth >= flowLimit_) return false;
        if (hasSequenceOfCollections(node)) return false;
        flow.clear();
        return tryFlow(node, flow, flowLimit_ - prefixWidth);
    }

    void emitMap(const YamlNode& node, std::size_t indent, std::string& out) const {
        const std::string pad(indent, ' ');
        for (std::size_t i = 0; i < node.size(); ++i) {
            const std::string key = keyText(node.keyAt(i));
            const YamlNode& value = node.valueAt(i);
            std::string flow;
            if (value.isNull() || value.isScalar()) {
                out += pad + key + ": " + scalarText(value, false) + "\n";
            } else if (fitsInLine(value, indent + key.size() + 2, flow)) {
                out += pad + key + ": " + flow + "\n";
            } else {
                out += pad + key + ":\n";
                if (value.isSequence()) {
                    emitSequence(value, indent + 2, out);
                } else {
                    emitMap(value, indent + 2, out);
                }
            }
        }
    }

    void emitSequence(const YamlNode& node, std::size_t indent, std::string& out) const {
        const std::string pad(indent, ' ');
        for (std::size_t i = 0; i < node.size(); ++i) {
            const YamlNode& item = node.item(i);
            std::string flow;
            if (item.isNull() || item.isScalar()) {
                out += pad + "- " + scalarText(item, false) + "\n";
            } else if (fitsInLine(item, indent + 2, flow)) {
                out += pad + "- " + flow + "\n";
            } else if (item.isMap()) {
                // "- 첫키: 값" 처럼 첫 항목을 '-' 줄에 붙인다.
                std::string block;
                emitMap(item, indent + 2, block);
                out += pad + "- " + block.substr(indent + 2);
            } else {
                out += pad + "-\n";
                emitSequence(item, indent + 2, out);
            }
        }
    }

    std::size_t flowLimit_;
};

}  // namespace

std::string emitYaml(const YamlNode& node, std::size_t flowLimit) {
    return Emitter(flowLimit).emit(node);
}

}  // namespace gnn
