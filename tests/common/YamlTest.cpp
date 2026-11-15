#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>

#include "gnn/common/Yaml.hpp"
#include "gnn/common/YamlError.hpp"
#include "gnn/common/YamlFile.hpp"
#include "gnn/common/YamlNode.hpp"

using namespace gnn;
namespace fs = std::filesystem;

namespace {

// 테스트마다 비어 있는 임시 폴더를 쓰고 끝나면 지운다.
struct TempDir {
    fs::path path;
    explicit TempDir(const std::string& name) : path(fs::temp_directory_path() / ("gnn_test_" + name)) {
        std::error_code ignored;
        fs::remove_all(path, ignored);
        fs::create_directories(path);
    }
    ~TempDir() {
        std::error_code ignored;
        fs::remove_all(path, ignored);
    }
};

std::string readAll(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
}

// 이 파일(tests/common/YamlTest.cpp)의 위치에서 저장소 루트를 구한다.
fs::path repoDir() {
    return fs::path(__FILE__).parent_path().parent_path().parent_path();
}

fs::path exampleFile(const std::string& relative) {
    return repoDir() / "examples" / "demo" / "mnist" / relative;
}

}  // namespace

TEST_CASE("YamlNode: 값을 만들고 읽는다") {
    YamlNode map = YamlNode::makeMap();
    map.set("a", YamlNode::makeInt(5));
    map.set("b", YamlNode::makeString("hello"));
    map.set("a", YamlNode::makeInt(7));  // 같은 키는 값을 바꾼다

    CHECK(map.size() == 2);
    CHECK(map.get("a").asInt() == 7);
    CHECK(map.keyAt(0) == "a");  // 넣은 순서를 유지한다
    CHECK(map.keyAt(1) == "b");
    CHECK(map.has("b"));
    CHECK(map.find("c") == nullptr);
    CHECK_THROWS_AS(map.get("c"), YamlError);
    CHECK_THROWS_AS(map.push(YamlNode()), YamlError);

    CHECK(YamlNode::makeBool(true).asBool());
    CHECK(YamlNode::makeDouble(0.1).asDouble() == 0.1);
    CHECK(YamlNode::makeScalar("12x").text() == "12x");
    CHECK_THROWS_AS(YamlNode::makeScalar("12x").asInt(), YamlError);
    CHECK_THROWS_AS(YamlNode::makeScalar("yes").asBool(), YamlError);
    CHECK_THROWS_AS(YamlNode::makeDouble(std::numeric_limits<double>::quiet_NaN()), YamlError);
}

TEST_CASE("parseYaml: 들여쓰기로 쓴 map 과 sequence") {
    const YamlNode root = parseYaml(
        "# 주석\n"
        "name: demo   # 뒤 주석\n"
        "count: 3\n"
        "list:\n"
        "  - 1\n"
        "  - two\n"
        "nested:\n"
        "  inner:\n"
        "    value: 1.5\n"
        "same:\n"
        "- x\n"
        "- y\n");
    CHECK(root.get("name").text() == "demo");
    CHECK(root.get("count").asInt() == 3);
    CHECK(root.get("list").size() == 2);
    CHECK(root.get("list").item(1).text() == "two");
    CHECK(root.get("nested").get("inner").get("value").asDouble() == 1.5);
    CHECK(root.get("same").size() == 2);  // "key:" 와 같은 들여쓰기의 목록
}

TEST_CASE("parseYaml: 목록 항목이 map 인 경우") {
    const YamlNode root = parseYaml(
        "nodes:\n"
        "  - id: 1\n"
        "    kind: layer\n"
        "    size: 784\n"
        "  - id: 2\n"
        "    kind: network\n"
        "    children: [1]\n"
        "  -\n"
        "    id: 3\n"
        "  - - a\n"
        "    - b\n");
    const YamlNode& nodes = root.get("nodes");
    REQUIRE(nodes.size() == 4);
    CHECK(nodes.item(0).get("size").asInt() == 784);
    CHECK(nodes.item(1).get("children").item(0).asInt() == 1);
    CHECK(nodes.item(2).get("id").asInt() == 3);
    CHECK(nodes.item(3).item(1).text() == "b");
}

TEST_CASE("parseYaml: flow 는 한 줄과 여러 줄 모두 읽는다") {
    const YamlNode root = parseYaml(
        "a: {id: 1, name: x, list: [1, 2, 3], empty: {}, none: []}\n"
        "b:\n"
        "  - {id: 11, kind: layer,\n"
        "     activation: {name: relu},   # 여러 줄\n"
        "     exceptions: {1790: {name: tanh}}}\n"
        "c: [ ]\n"
        "d: {k: , j: 2}\n");
    CHECK(root.get("a").get("list").size() == 3);
    CHECK(root.get("a").get("empty").isMap());
    CHECK(root.get("a").get("none").isSequence());
    const YamlNode& item = root.get("b").item(0);
    CHECK(item.get("activation").get("name").text() == "relu");
    CHECK(item.get("exceptions").get("1790").get("name").text() == "tanh");
    CHECK(root.get("c").size() == 0);
    CHECK(root.get("d").get("k").isNull());
    CHECK(root.get("d").get("j").asInt() == 2);
}

TEST_CASE("parseYaml: 문자열") {
    const YamlNode root = parseYaml(
        "a: 입력 784 -> encoder(128 -> 64) -> 출력 10, 입력에서 출력으로\n"
        "b: \"x: y # z\"\n"
        "c: 'it''s'\n"
        "d: \"tab\\there\\nnew \\u00e9 \\\"q\\\"\"\n"
        "e: \"123\"\n"
        "f: 123\n"
        "g: range 1001-1784, 1790\n"
        "\"quoted key\": 1\n"
        "h: ~\n"
        "i:\n");
    CHECK(root.get("a").text() == "입력 784 -> encoder(128 -> 64) -> 출력 10, 입력에서 출력으로");
    CHECK(root.get("b").text() == "x: y # z");
    CHECK(root.get("c").text() == "it's");
    CHECK(root.get("d").text() == "tab\there\nnew \xC3\xA9 \"q\"");
    CHECK(root.get("e").text() == "123");
    CHECK(root.get("e").isQuoted());
    CHECK(!root.get("f").isQuoted());
    CHECK(root.get("g").text() == "range 1001-1784, 1790");
    CHECK(root.get("quoted key").asInt() == 1);
    CHECK(root.get("h").isNull());
    CHECK(root.get("i").isNull());
}

TEST_CASE("parseYaml: 빈 문서와 문서 시작 표시") {
    CHECK(parseYaml("").isNull());
    CHECK(parseYaml("# 주석만\n\n").isNull());
    CHECK(parseYaml("---\na: 1\n").get("a").asInt() == 1);
    CHECK(parseYaml("\xEF\xBB\xBF" "a: 1\n").get("a").asInt() == 1);  // BOM
    CHECK(parseYaml("a: 1\r\nb: 2\r\n").get("b").asInt() == 2);        // CRLF
    CHECK(parseYaml("just text").text() == "just text");
}

TEST_CASE("parseYaml: 오류는 YamlError 이고 줄 번호를 담는다") {
    CHECK_THROWS_AS(parseYaml("a: 1\na: 2\n"), YamlError);                // 같은 키
    CHECK_THROWS_AS(parseYaml("a:\n\tb: 1\n"), YamlError);               // 탭 들여쓰기
    CHECK_THROWS_AS(parseYaml("a: {x: 1\n"), YamlError);                 // 닫히지 않은 flow
    CHECK_THROWS_AS(parseYaml("a: \"abc\n"), YamlError);                 // 닫히지 않은 따옴표
    CHECK_THROWS_AS(parseYaml("a: &anchor 1\n"), YamlError);             // anchor
    CHECK_THROWS_AS(parseYaml("a: |\n  text\n"), YamlError);             // 블록 문자열
    CHECK_THROWS_AS(parseYaml("a: 1\n---\nb: 2\n"), YamlError);          // 여러 문서
    CHECK_THROWS_AS(parseYaml("a: 1\n   b: 2\n"), YamlError);            // 들여쓰기 어긋남
    CHECK_THROWS_AS(parseYaml("a: 1\nnot a map\n"), YamlError);          // 키가 없는 줄
    CHECK_THROWS_AS(parseYaml("a: {x: 1, x: 2}\n"), YamlError);          // flow 의 같은 키
    CHECK_THROWS_AS(parseYaml("a: [1, 2]]\n"), YamlError);               // 닫는 괄호가 더 많음

    try {
        parseYaml("a: 1\nb: 2\nb: 3\n");
        FAIL("예외가 나야 한다");
    } catch (const YamlError& error) {
        CHECK(error.line() == 3);
    }
}

TEST_CASE("emitYaml: 짧으면 한 줄, 길면 여러 줄") {
    YamlNode root = YamlNode::makeMap();
    root.set("small", [] {
        YamlNode m = YamlNode::makeMap();
        m.set("id", YamlNode::makeInt(1));
        m.set("name", YamlNode::makeString("x"));
        return m;
    }());
    YamlNode list = YamlNode::makeSequence();
    for (int i = 0; i < 3; ++i) {
        YamlNode m = YamlNode::makeMap();
        m.set("id", YamlNode::makeInt(i));
        m.set("kind", YamlNode::makeString("layer"));
        list.push(m);
    }
    root.set("list", list);

    const std::string text = emitYaml(root);
    CHECK(text ==
          "small: {id: 1, name: x}\n"
          "list:\n"
          "  - {id: 0, kind: layer}\n"
          "  - {id: 1, kind: layer}\n"
          "  - {id: 2, kind: layer}\n");

    // 한 줄 제한을 줄이면 항목이 여러 줄로 나뉜다
    const std::string wide = emitYaml(root, 10);
    CHECK(wide.find("- id: 0\n    kind: layer\n") != std::string::npos);
    CHECK(parseYaml(wide) == root);
}

TEST_CASE("emitYaml: 읽을 때 달라질 수 있는 문자열은 따옴표를 붙인다") {
    const char* texts[] = {"", " lead", "trail ", "123", "true", "null", "~", "a: b", "a #b", "#x", "- x",
                           "-", "{x}", "[x]", "a,b", "x:", "line\nbreak", "tab\there", "quote\"inside",
                           "back\\slash", "한글 이름", "885-950, 960-1021", "-5", "it's"};
    for (const char* text : texts) {
        CAPTURE(text);
        YamlNode root = YamlNode::makeMap();
        root.set("k", YamlNode::makeString(text));
        YamlNode flowHolder = YamlNode::makeSequence();
        flowHolder.push(YamlNode::makeString(text));
        root.set("seq", flowHolder);  // flow 안에서도 확인

        const YamlNode back = parseYaml(emitYaml(root));
        CHECK(back.get("k").text() == text);
        CHECK(back.get("seq").item(0).text() == text);
    }
}

TEST_CASE("emitYaml: 구조를 유지한다 (파싱 → 출력 → 파싱)") {
    const YamlNode original = parseYaml(
        "a: 1\n"
        "b:\n"
        "  - {x: 1, y: [1, 2]}\n"
        "  - name: long\n"
        "    values: [1, 2, 3]\n"
        "  - - 1\n"
        "    - 2\n"
        "c: {}\n"
        "d: []\n"
        "e: ~\n");
    CHECK(parseYaml(emitYaml(original)) == original);
    CHECK(emitYaml(YamlNode()) == "null\n");
    CHECK(emitYaml(YamlNode::makeMap()) == "{}\n");
}

TEST_CASE("YamlFile: 저장하고 다시 읽는다") {
    TempDir dir("yamlfile_roundtrip");
    const fs::path file = dir.path / "sub" / "graph.yaml";  // 상위 폴더가 없어도 만든다

    YamlNode root = YamlNode::makeMap();
    root.set("format_version", YamlNode::makeInt(1));
    root.set("name", YamlNode::makeString("한글 이름"));
    YamlFile::save(file, root);

    CHECK(fs::exists(file));
    CHECK(!fs::exists(file.string() + YamlFile::tempSuffix()));  // 임시 파일은 남지 않는다
    CHECK(YamlFile::load(file) == root);

    // 덮어쓰기
    root.set("format_version", YamlNode::makeInt(2));
    YamlFile::save(file, root);
    CHECK(YamlFile::load(file).get("format_version").asInt() == 2);
}

TEST_CASE("YamlFile: 읽기 오류와 임시 파일 정리") {
    TempDir dir("yamlfile_errors");
    CHECK_THROWS_AS(YamlFile::load(dir.path / "missing.yaml"), YamlError);

    const fs::path bad = dir.path / "bad.yaml";
    {
        std::ofstream(bad, std::ios::binary) << "a: 1\na: 2\n";
    }
    try {
        YamlFile::load(bad);
        FAIL("예외가 나야 한다");
    } catch (const YamlError& error) {
        CHECK(std::string(error.what()).find("bad.yaml") != std::string::npos);  // 파일 이름이 들어 있다
    }

    // 중단된 저장이 남긴 임시 파일만 지운다
    fs::create_directories(dir.path / "d");
    const fs::path stale1 = dir.path / "x.yaml.tmp-gnn";
    const fs::path stale2 = dir.path / "d" / "y.yaml.tmp-gnn";
    const fs::path keep = dir.path / "d" / "keep.yaml";
    for (const fs::path& path : {stale1, stale2, keep}) {
        std::ofstream(path, std::ios::binary) << "a: 1\n";
    }
    CHECK(YamlFile::removeStaleTempFiles(dir.path) == 2);
    CHECK(!fs::exists(stale1));
    CHECK(!fs::exists(stale2));
    CHECK(fs::exists(keep));
    CHECK(YamlFile::removeStaleTempFiles(dir.path / "no_such_dir") == 0);
}

TEST_CASE("예시 파일(examples/)을 읽을 수 있다") {
    SUBCASE("Design Graph") {
        const YamlNode root = YamlFile::load(exampleFile("design/graph.yaml"));
        CHECK(root.get("format_version").asInt() == 1);
        CHECK(root.get("graph").get("kind").text() == "design");
        CHECK(root.get("nodes").size() == 8);
        CHECK(root.get("edges").size() == 7);

        const YamlNode& hidden1 = root.get("nodes").item(1);
        CHECK(hidden1.get("size").asInt() == 128);
        CHECK(hidden1.get("perceptrons").text() == "885-950, 960-1021");
        CHECK(hidden1.get("exceptions").get("960").get("activation").get("name").text() == "tanh");

        const YamlNode& network = root.get("nodes").item(4);
        CHECK(network.get("ports").size() == 2);
        CHECK(network.get("ports").item(0).get("dir").text() == "input");

        const YamlNode& edge = root.get("edges").item(1);
        CHECK(edge.get("to").get("node").asInt() == 3);
        CHECK(!edge.get("to").has("port"));
        CHECK(root.get("edges").item(0).get("to").get("port").text() == "in");

        const YamlNode& origin = root.get("origin").item(0);
        CHECK(origin.get("node_id").asInt() == 1098);
        CHECK(origin.get("source").get("kind").text() == "training");
        CHECK(origin.get("source").get("node_id").asInt() == 12);
        CHECK(origin.get("id_map").size() == 6);
        CHECK(origin.get("id_map").get("1102-1229").text() == "301-428");

        CHECK(parseYaml(emitYaml(root)) == root);
    }

    SUBCASE("Training Graph") {
        const YamlNode root = YamlFile::load(exampleFile("training/mnist_run1/graph.yaml"));
        CHECK(root.get("graph").get("kind").text() == "training");
        CHECK(root.get("graph").get("clone_of").isNull());
        CHECK(root.get("graph").get("design").get("edge_group").asInt() == 1097);
        CHECK(root.get("nodes").size() == 12);
        CHECK(root.get("edges").size() == 12);

        const YamlNode& weight = root.get("nodes").item(1).get("params").get("weight");
        CHECK(weight.get("shape").item(0).asInt() == 128);
        CHECK(weight.get("offset").asInt() == 0);
        CHECK(weight.get("rows").text() == "885-950, 960-1021");

        CHECK(parseYaml(emitYaml(root)) == root);
    }

    SUBCASE("Inference Graph") {
        const YamlNode root = YamlFile::load(exampleFile("inference/mnist_run1_final/graph.yaml"));
        CHECK(root.get("graph").get("kind").text() == "inference");
        CHECK(root.get("graph").get("training").get("graph").text() == "mnist_run1");
        CHECK(root.get("graph").get("training").get("taken_at").text() == "2026-11-15T14:20:00");
        CHECK(root.get("nodes").size() == 12);
        CHECK(root.get("edges").size() == 12);
        CHECK(root.get("nodes").item(4).get("rows").text() == "66");

        CHECK(parseYaml(emitYaml(root)) == root);
    }
}
