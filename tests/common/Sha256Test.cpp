#include <doctest/doctest.h>

#include <stdexcept>
#include <string>

#include "gnn/common/Sha256.hpp"

using namespace gnn;

TEST_CASE("Sha256: 표준 시험 값") {
    CHECK(Sha256::hash("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    CHECK(Sha256::hash("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(Sha256::hash("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
          "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");  // 두 블록에 걸친다
}

TEST_CASE("Sha256: 긴 입력과 나눠서 넣은 입력이 같은 값이다") {
    const std::string million(1000000, 'a');
    CHECK(Sha256::hash(million) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");

    Sha256 pieces;
    for (int i = 0; i < 1000; ++i) pieces.update(std::string(1000, 'a'));
    CHECK(pieces.hexDigest() == Sha256::hash(million));

    // 64바이트 경계 근처의 길이에서 패딩이 맞는지
    for (std::size_t length : {55, 56, 57, 63, 64, 65, 119, 120, 128}) {
        CAPTURE(length);
        const std::string text(length, 'x');
        Sha256 whole;
        whole.update(text);
        Sha256 split;
        split.update(text.substr(0, length / 2));
        split.update(text.substr(length / 2));
        CHECK(split.hexDigest() == whole.hexDigest());
    }
    CHECK(Sha256::hash(std::string(55, 'a')) == "9f4390f8d30c2dd92ec9f095b65e2b9ae9b0a925a5258e241c9f1e910f734318");
    CHECK(Sha256::hash(std::string(56, 'a')) == "b35439a4ac6f0948b6d6f9e3c6af0f5f590ce20f1bde7090ef7970686ec6738a");
    CHECK(Sha256::hash(std::string(64, 'a')) == "ffe054fe7ae0cb6dc65c3af9b61d5209f439851db43d0ba5997337df154668eb");
}

TEST_CASE("Sha256: 끝낸 뒤에는 쓸 수 없다") {
    Sha256 sha;
    sha.update("abc");
    sha.hexDigest();
    CHECK_THROWS_AS(sha.update("d"), std::logic_error);
    CHECK_THROWS_AS(sha.hexDigest(), std::logic_error);
}
