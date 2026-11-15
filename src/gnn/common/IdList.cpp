#include "gnn/common/IdList.hpp"

#include <limits>
#include <stdexcept>

namespace gnn {

namespace {

struct Range {
    Id first;
    Id last;
};

std::string trim(const std::string& s) {
    std::size_t begin = 0;
    std::size_t end = s.size();
    while (begin < end && (s[begin] == ' ' || s[begin] == '\t')) ++begin;
    while (end > begin && (s[end - 1] == ' ' || s[end - 1] == '\t')) --end;
    return s.substr(begin, end - begin);
}

Id parseId(const std::string& text, const std::string& whole) {
    if (text.empty()) throw std::invalid_argument("id 가 비어 있다: '" + whole + "'");
    Id value = 0;
    for (const char c : text) {
        if (c < '0' || c > '9') throw std::invalid_argument("id 는 숫자여야 한다: '" + text + "' ('" + whole + "')");
        const int digit = c - '0';
        if (value > (std::numeric_limits<Id>::max() - digit) / 10) {
            throw std::invalid_argument("id 가 너무 크다: '" + text + "'");
        }
        value = value * 10 + digit;
    }
    if (value < 1) throw std::invalid_argument("id 는 1 이상이어야 한다: '" + text + "'");
    return value;
}

// 쉼표로 나눈 한 항목("a" 또는 "a-b")을 구간으로 바꾼다.
Range parseItem(const std::string& item, const std::string& whole) {
    const std::string text = trim(item);
    const std::size_t dash = text.find('-');
    if (dash == std::string::npos) {
        const Id id = parseId(text, whole);
        return Range{id, id};
    }
    const Id first = parseId(trim(text.substr(0, dash)), whole);
    const Id last = parseId(trim(text.substr(dash + 1)), whole);
    if (last < first) {
        throw std::invalid_argument("구간의 끝이 시작보다 작다: '" + text + "'");
    }
    return Range{first, last};
}

template <typename Visitor>
void forEachItem(const std::string& text, Visitor visitor) {
    if (trim(text).empty()) return;
    std::size_t begin = 0;
    while (true) {
        const std::size_t comma = text.find(',', begin);
        const std::string item = text.substr(begin, comma == std::string::npos ? std::string::npos : comma - begin);
        visitor(parseItem(item, text));
        if (comma == std::string::npos) break;
        begin = comma + 1;
    }
}

}  // namespace

std::vector<Id> parseIdList(const std::string& text, std::size_t maxCount) {
    std::vector<Id> ids;
    forEachItem(text, [&](const Range& range) {
        const std::size_t length = static_cast<std::size_t>(range.last - range.first) + 1;
        if (length > maxCount || ids.size() > maxCount - length) {
            throw std::invalid_argument("id 목록이 너무 길다 (최대 " + std::to_string(maxCount) + "개)");
        }
        for (Id id = range.first; id <= range.last; ++id) {
            ids.push_back(id);
            if (id == std::numeric_limits<Id>::max()) break;
        }
    });
    return ids;
}

std::size_t countIdList(const std::string& text) {
    std::size_t total = 0;
    forEachItem(text, [&](const Range& range) {
        const std::size_t length = static_cast<std::size_t>(range.last - range.first) + 1;
        if (total > std::numeric_limits<std::size_t>::max() - length) {
            throw std::invalid_argument("id 목록이 너무 길다");
        }
        total += length;
    });
    return total;
}

std::string formatIdList(const std::vector<Id>& ids) {
    std::string out;
    std::size_t i = 0;
    while (i < ids.size()) {
        if (ids[i] < 1) throw std::invalid_argument("id 는 1 이상이어야 한다: " + std::to_string(ids[i]));
        std::size_t j = i;
        while (j + 1 < ids.size() && ids[j + 1] >= 1 && ids[j] != std::numeric_limits<Id>::max() &&
               ids[j + 1] == ids[j] + 1) {
            ++j;
        }
        if (!out.empty()) out += ", ";
        out += std::to_string(ids[i]);
        if (j > i) {
            out += '-';
            out += std::to_string(ids[j]);
        }
        i = j + 1;
    }
    return out;
}

}  // namespace gnn
