#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "gnn/common/IdCounter.hpp"

namespace gnn {

// id 목록의 압축 표기. 구간은 "a-b", 쉼표로 이어 쓴다. 예: "1001-1010, 1020, 1030-1040".
// 적힌 순서가 곧 목록의 순서이다 (layer 의 perceptron 순서 = tensor 의 행 순서).
// 이 함수들은 형식이 맞는지만 확인한다. 같은 id 가 두 번 나오는지는 호출하는 쪽이 확인한다.
// 형식이 잘못되면 std::invalid_argument 를 던진다.

// 표기를 id 목록으로 펼친다. 빈 문자열(공백만 포함)은 빈 목록이다.
// 펼친 개수가 maxCount 를 넘으면 오류이다 (잘못된 큰 구간으로 메모리를 다 쓰지 않도록).
std::vector<Id> parseIdList(const std::string& text, std::size_t maxCount = 50000000);

// 펼치지 않고 개수만 센다 (큰 layer 의 크기를 확인할 때).
std::size_t countIdList(const std::string& text);

// id 목록을 표기로 만든다. 1 씩 커지며 이어지는 id 는 구간으로 묶고 순서는 바꾸지 않는다.
// 1 미만의 id 는 std::invalid_argument.
std::string formatIdList(const std::vector<Id>& ids);

}  // namespace gnn
