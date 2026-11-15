#pragma once

#include <cstddef>
#include <filesystem>

#include "gnn/common/YamlNode.hpp"

namespace gnn {

// YAML 파일 읽기/쓰기. 쓰기는 중간에 끊겨도 원래 파일이 남도록 임시 파일에 쓴 뒤 이름을 바꾼다.
class YamlFile {
public:
    // 임시 파일의 확장자 (이 확장자의 파일만 정리 대상이다)
    static const char* tempSuffix() { return ".tmp-gnn"; }

    // path 의 YAML 파일을 읽는다 (UTF-8, BOM 은 무시). 읽기/문법 오류는 YamlError.
    static YamlNode load(const std::filesystem::path& path);

    // node 를 path 에 저장한다. 상위 폴더가 없으면 만든다.
    // `<path>.tmp-gnn` 에 쓰고 닫은 뒤 이름을 바꿔 교체한다. 실패하면 임시 파일을 지우고 YamlError.
    static void save(const std::filesystem::path& path, const YamlNode& node);

    // directory 아래(하위 폴더 포함)에 남은 임시 파일(`*.tmp-gnn`)을 지우고 지운 개수를 돌려준다.
    // 시작할 때 부르면 이전에 중단된 저장의 찌꺼기가 정리된다.
    static std::size_t removeStaleTempFiles(const std::filesystem::path& directory);
};

}  // namespace gnn
