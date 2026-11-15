#include <doctest/doctest.h>

#include <string>

#include "gnn/common/GraphName.hpp"

using namespace gnn;

TEST_CASE("checkGraphName: 쓸 수 있는 이름") {
    for (const char* name : {"mnist", "mnist_run1", "한글 이름", "a.b", "v1.2", "My Net", "console", "com10", "x"}) {
        CAPTURE(name);
        CHECK(checkGraphName(name, GraphKind::Design).empty());
        CHECK(checkGraphName(name, GraphKind::Training).empty());
    }
}

TEST_CASE("checkGraphName: 쓸 수 없는 이름") {
    for (const char* name : {"", " lead", "trail ", "dot.", "..", "a/b", "a\\b", "a:b", "a*b", "a?b", "a\"b", "a<b",
                             "a>b", "a|b", "ctl\tchar", "CON", "con", "Nul", "aux.txt", "COM1", "lpt9", "Com3.log"}) {
        CAPTURE(name);
        CHECK(!checkGraphName(name, GraphKind::Training).empty());
    }
    CHECK(!checkGraphName(std::string(101, 'a'), GraphKind::Design).empty());  // 100바이트 초과
    CHECK(checkGraphName(std::string(100, 'a'), GraphKind::Design).empty());
}

TEST_CASE("checkGraphName: Design Graph 이름은 project 폴더 안의 예약어를 쓸 수 없다") {
    CHECK(!checkGraphName("operators", GraphKind::Design).empty());
    CHECK(!checkGraphName("Operators", GraphKind::Design).empty());
    CHECK(!checkGraphName("project.yaml", GraphKind::Design).empty());
    CHECK(!checkGraphName("PROJECT.YAML", GraphKind::Design).empty());
    // Training/Inference 이름은 design 과 다른 폴더에 놓이므로 괜찮다
    CHECK(checkGraphName("operators", GraphKind::Training).empty());
    CHECK(checkGraphName("project.yaml", GraphKind::Inference).empty());
}

TEST_CASE("graphNameKey: 대소문자를 구분하지 않는 비교 키") {
    CHECK(graphNameKey("MNIST") == graphNameKey("mnist"));
    CHECK(graphNameKey("Mnist_Run1") == "mnist_run1");
    CHECK(graphNameKey("한글") == "한글");  // 영문자가 아닌 글자는 그대로 둔다
    CHECK(graphNameKey("abc") != graphNameKey("abd"));
}
