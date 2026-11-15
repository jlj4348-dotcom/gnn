#pragma once

#include <cstdint>

namespace gnn {

// node/edge/graph 의 id. 64비트 정수이며 1 이상이다 (0 은 "없음").
// 부호 있는 정수를 쓰는 이유: YAML 에서 asInt 로 손실 없이 읽고 쓰기 위해서이다.
using Id = std::int64_t;
constexpr Id kInvalidId = 0;

// id 를 발급하는 카운터. 항상 증가만 하며 한 번 발급한 id 는 다시 발급하지 않는다.
// (undo 로 지워도 카운터는 되돌리지 않는다.)
//
// 쓰임새
// - graph 마다 하나씩 가지며 그 graph 안의 node/edge id 를 발급한다. 다음 값은 graph 의 YAML 에 저장한다.
// - 실행(세션) 하나에 하나를 두고 메모리에 올린 graph 의 graph id 를 발급한다. 저장하지 않는다.
//
// 스레드 안전하지 않다 (Design Graph 는 단일 스레드로 다룬다).
class IdCounter {
public:
    // nextValue: 다음에 발급할 값 (1 이상). 새 카운터는 1 에서 시작한다.
    explicit IdCounter(Id nextValue = 1);

    // id 하나를 발급한다. 더 발급할 수 없으면 std::overflow_error.
    Id issue();

    // 연속된 count 개(1 이상)를 발급하고 첫 id 를 돌려준다 (복사/붙여넣기가 연속 id 를 쓰도록).
    Id issueRange(std::int64_t count);

    // 다음에 발급될 값. 저장할 때 쓴다.
    Id nextValue() const { return next_; }

    // 다음 발급 값이 value 보다 작지 않게 올린다 (이미 크면 그대로). 파일을 읽을 때
    // 카운터 값이 파일 안의 가장 큰 id 보다 작은 경우를 바로잡는 데 쓴다.
    void advanceTo(Id value);

private:
    Id next_;
};

}  // namespace gnn
