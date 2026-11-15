#include <doctest/doctest.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "gnn/common/IdList.hpp"

using namespace gnn;

TEST_CASE("parseIdList: 구간과 단일 id 를 펼친다") {
    CHECK(parseIdList("1001-1003") == std::vector<Id>{1001, 1002, 1003});
    CHECK(parseIdList("5") == std::vector<Id>{5});
    CHECK(parseIdList("5-5") == std::vector<Id>{5});
    CHECK(parseIdList("  ").empty());
    CHECK(parseIdList("").empty());

    // 적힌 순서를 그대로 유지한다 (분할/병합 뒤 id 가 연속이 아닐 수 있다)
    CHECK(parseIdList("885-887, 960, 1000-1001") == std::vector<Id>{885, 886, 887, 960, 1000, 1001});
    CHECK(parseIdList("30, 10-11, 20") == std::vector<Id>{30, 10, 11, 20});
    CHECK(parseIdList(" 7 - 9 ,12 ") == std::vector<Id>{7, 8, 9, 12});  // 공백은 무시

    CHECK(parseIdList("1001-1010, 1020, 1030-1040").size() == 22);
}

TEST_CASE("parseIdList: 잘못된 표기는 오류") {
    for (const char* text : {"a", "1-", "-5", "1,,2", "1,", ",1", "5-3", "0", "1-0", "1.5", "12abc", "1--3",
                             "9999999999999999999999"}) {
        CAPTURE(text);
        CHECK_THROWS_AS(parseIdList(text), std::invalid_argument);
    }
}

TEST_CASE("parseIdList: 너무 큰 구간은 펼치지 않고 오류") {
    CHECK_THROWS_AS(parseIdList("1-1000000000000"), std::invalid_argument);
    CHECK_THROWS_AS(parseIdList("1-5, 10-20", 10), std::invalid_argument);
    CHECK(parseIdList("1-5, 10-14", 10).size() == 10);
}

TEST_CASE("countIdList: 펼치지 않고 개수를 센다") {
    CHECK(countIdList("") == 0);
    CHECK(countIdList("101-884") == 784);
    CHECK(countIdList("885-950, 960-1021") == 128);
    CHECK(countIdList("1-1000000000000") == 1000000000000ULL);  // 펼치면 메모리를 다 쓸 크기
    CHECK_THROWS_AS(countIdList("3-1"), std::invalid_argument);
}

TEST_CASE("formatIdList: 이어지는 id 는 구간으로 묶는다") {
    CHECK(formatIdList({}) == "");
    CHECK(formatIdList({7}) == "7");
    CHECK(formatIdList({1, 2, 3, 5, 7, 8}) == "1-3, 5, 7-8");
    CHECK(formatIdList({5, 4}) == "5, 4");  // 순서는 바꾸지 않는다
    CHECK(formatIdList({10, 11, 20, 21, 22, 40}) == "10-11, 20-22, 40");
    CHECK_THROWS_AS(formatIdList({3, 0}), std::invalid_argument);
}

TEST_CASE("IdList: 만들고 읽으면 원래 목록이 된다") {
    const std::vector<Id> original = {101, 102, 103, 200, 300, 301, 5, 6, 7, 8};
    CHECK(parseIdList(formatIdList(original)) == original);
    CHECK(formatIdList(parseIdList("885-950, 960-1021")) == "885-950, 960-1021");
}
