#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace gnn {

// SHA-256. Training Graph 가 소속 Design Graph 의 구조 지문을 기록하는 데 쓴다.
// 사용: update 를 여러 번 부르고 hexDigest 로 끝낸다. hexDigest 뒤에는 update 를 부를 수 없다.
class Sha256 {
public:
    Sha256();

    void update(const void* data, std::size_t size);
    void update(std::string_view text) { update(text.data(), text.size()); }

    // 소문자 16진수 64자
    std::string hexDigest();

    // 한 번에 계산하는 편의 함수
    static std::string hash(std::string_view text);

private:
    void processBlock(const std::uint8_t* block);

    std::array<std::uint32_t, 8> state_;
    std::array<std::uint8_t, 64> buffer_;
    std::size_t bufferSize_ = 0;
    std::uint64_t totalBytes_ = 0;
    bool finished_ = false;
};

}  // namespace gnn
