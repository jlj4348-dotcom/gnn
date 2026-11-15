#include <doctest/doctest.h>

#include <limits>
#include <stdexcept>

#include "gnn/common/IdCounter.hpp"

using namespace gnn;

TEST_CASE("IdCounter: 1 에서 시작해 하나씩 증가한다") {
    IdCounter counter;
    CHECK(counter.nextValue() == 1);
    CHECK(counter.issue() == 1);
    CHECK(counter.issue() == 2);
    CHECK(counter.nextValue() == 3);
}

TEST_CASE("IdCounter: 저장한 값에서 이어서 발급한다") {
    IdCounter counter(1801);
    CHECK(counter.issue() == 1801);
    CHECK(counter.nextValue() == 1802);
    CHECK_THROWS_AS(IdCounter(0), std::invalid_argument);
    CHECK_THROWS_AS(IdCounter(-5), std::invalid_argument);
}

TEST_CASE("IdCounter: 연속된 id 를 한 번에 발급한다") {
    IdCounter counter(10);
    CHECK(counter.issueRange(5) == 10);  // 10~14
    CHECK(counter.issue() == 15);
    CHECK(counter.issueRange(1) == 16);
    CHECK_THROWS_AS(counter.issueRange(0), std::invalid_argument);
    CHECK_THROWS_AS(counter.issueRange(-1), std::invalid_argument);
    CHECK(counter.nextValue() == 17);  // 실패한 호출은 카운터를 바꾸지 않는다
}

TEST_CASE("IdCounter: advanceTo 는 줄어들지 않는다") {
    IdCounter counter(100);
    counter.advanceTo(50);
    CHECK(counter.nextValue() == 100);
    counter.advanceTo(200);
    CHECK(counter.nextValue() == 200);
    CHECK(counter.issue() == 200);
}

TEST_CASE("IdCounter: 끝에 이르면 오류이고 값은 그대로이다") {
    const Id max = std::numeric_limits<Id>::max();
    IdCounter counter(max - 2);
    CHECK(counter.issueRange(2) == max - 2);  // max-2, max-1
    CHECK(counter.nextValue() == max);
    CHECK_THROWS_AS(counter.issue(), std::overflow_error);
    CHECK_THROWS_AS(counter.issueRange(5), std::overflow_error);
    CHECK(counter.nextValue() == max);
}
