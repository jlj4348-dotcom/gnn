#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>

#include "gnn/common/CollectingSink.hpp"
#include "gnn/common/ConsoleSink.hpp"
#include "gnn/common/GraphKind.hpp"
#include "gnn/common/Issue.hpp"
#include "gnn/common/LogFileSink.hpp"
#include "gnn/common/Reporter.hpp"

using namespace gnn;
namespace fs = std::filesystem;

TEST_CASE("Issue: 한 줄 텍스트") {
    Issue issue;
    issue.severity = Severity::Error;
    issue.code = "unknown_activation";
    issue.message = "등록되지 않은 activation";
    issue.targets = {12, 15};
    CHECK(formatIssue(issue) == "[error] unknown_activation: 등록되지 않은 activation (대상: 12, 15)");

    Issue bare;
    bare.severity = Severity::Warning;
    bare.code = "unconnected_node";
    CHECK(formatIssue(bare) == "[warning] unconnected_node");

    CHECK(std::string(toString(Severity::Info)) == "info");
}

TEST_CASE("GraphKind: 문자열과 상호 변환") {
    for (const GraphKind kind : {GraphKind::Design, GraphKind::Training, GraphKind::Inference}) {
        GraphKind back = GraphKind::Design;
        CHECK(parseGraphKind(toString(kind), back));
        CHECK((back == kind));  // doctest 가 enum 을 분해하지 않도록 괄호로 묶는다
    }
    GraphKind kind = GraphKind::Training;
    CHECK(!parseGraphKind("Design", kind));  // 대소문자를 구분한다
    CHECK((kind == GraphKind::Training));     // 실패하면 바뀌지 않는다
}

TEST_CASE("Reporter: 등록된 모든 sink 에 전달한다") {
    Reporter reporter;
    auto first = std::make_shared<CollectingSink>();
    auto second = std::make_shared<CollectingSink>();
    const auto handle = reporter.addSink(first);
    reporter.addSink(second);
    CHECK(reporter.sinkCount() == 2);

    reporter.error("a_code", "문제", {1, 2});
    reporter.warning("b_code", "의심");
    reporter.info("c_code", "알림");

    CHECK(first->issues().size() == 3);
    CHECK(second->issues().size() == 3);
    CHECK(first->count(Severity::Error) == 1);
    CHECK(first->has("b_code"));
    CHECK(!first->has("none"));
    CHECK(first->issues()[0].targets == std::vector<Id>{1, 2});

    reporter.removeSink(handle);
    reporter.error("d_code", "또 문제");
    CHECK(first->issues().size() == 3);   // 해제한 sink 는 더 받지 않는다
    CHECK(second->issues().size() == 4);

    first->clear();
    CHECK(first->issues().empty());
    CHECK(reporter.addSink(nullptr) == 0);  // nullptr 은 무시
}

TEST_CASE("Reporter: sink 가 보고 중에 다시 보고해도 교착하지 않는다") {
    class Echo : public ReportSink {
    public:
        explicit Echo(Reporter& reporter) : reporter_(reporter) {}
        void report(const Issue& issue) override {
            if (issue.code == "first") reporter_.info("echo", "다시 보고");
        }

    private:
        Reporter& reporter_;
    };

    Reporter reporter;
    auto collector = std::make_shared<CollectingSink>();
    reporter.addSink(std::make_shared<Echo>(reporter));
    reporter.addSink(collector);
    reporter.info("first", "시작");
    CHECK(collector->has("first"));
    CHECK(collector->has("echo"));
}

TEST_CASE("ConsoleSink: 스트림에 한 줄씩 쓴다") {
    std::ostringstream out;
    Reporter reporter;
    reporter.addSink(std::make_shared<ConsoleSink>(out));
    reporter.error("x_code", "메시지");
    reporter.warning("y_code", "경고", {7});
    CHECK(out.str() == "[error] x_code: 메시지\n[warning] y_code: 경고 (대상: 7)\n");

    CHECK(Reporter::makeConsole()->sinkCount() == 1);
}

TEST_CASE("LogFileSink: 시각과 함께 파일 끝에 덧붙인다") {
    const fs::path dir = fs::temp_directory_path() / "gnn_test_logfile";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const fs::path file = dir / "sub" / "run.log";

    {
        Reporter reporter;
        reporter.addSink(std::make_shared<LogFileSink>(file));
        reporter.error("log_code", "첫 줄");
    }
    {
        Reporter reporter;  // 다시 열어도 앞의 내용을 지우지 않는다
        reporter.addSink(std::make_shared<LogFileSink>(file));
        reporter.warning("log_code2", "둘째 줄", {3});
    }

    {
        std::ifstream stream(file, std::ios::binary);
        std::string line1, line2, rest;
        std::getline(stream, line1);
        std::getline(stream, line2);
        CHECK(line1.size() > 20);
        CHECK(line1.find("[error] log_code: 첫 줄") != std::string::npos);
        CHECK(line1[4] == '-');  // "YYYY-MM-DD hh:mm:ss ..." 로 시작한다
        CHECK(line2.find("[warning] log_code2: 둘째 줄 (대상: 3)") != std::string::npos);
        CHECK(!std::getline(stream, rest));
    }  // 파일을 닫은 뒤에 지운다 (Windows 는 열린 파일을 지울 수 없다)

    fs::remove_all(dir, ignored);
    CHECK(!fs::exists(dir));
}
