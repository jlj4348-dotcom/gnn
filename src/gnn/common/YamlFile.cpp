#include "gnn/common/YamlFile.hpp"

#include <fstream>
#include <iterator>
#include <string>
#include <system_error>

#include "gnn/common/Yaml.hpp"
#include "gnn/common/YamlError.hpp"

namespace gnn {

namespace fs = std::filesystem;

namespace {

std::string pathText(const fs::path& path) { return path.u8string(); }

}  // namespace

YamlNode YamlFile::load(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw YamlError("파일을 열 수 없다: " + pathText(path));
    std::string text((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    if (stream.bad()) throw YamlError("파일을 읽는 중 오류가 났다: " + pathText(path));
    try {
        return parseYaml(text);
    } catch (const YamlError& error) {
        throw YamlError(pathText(path) + ": " + error.what());
    }
}

void YamlFile::save(const fs::path& path, const YamlNode& node) {
    const std::string text = emitYaml(node);

    std::error_code code;
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path(), code);
        if (code) throw YamlError("폴더를 만들 수 없다: " + pathText(path.parent_path()) + " (" + code.message() + ")");
    }

    fs::path temp = path;
    temp += tempSuffix();

    {
        std::ofstream stream(temp, std::ios::binary | std::ios::trunc);
        if (!stream) throw YamlError("임시 파일을 만들 수 없다: " + pathText(temp));
        stream.write(text.data(), static_cast<std::streamsize>(text.size()));
        stream.flush();
        if (!stream) {
            stream.close();
            fs::remove(temp, code);
            throw YamlError("임시 파일에 쓰지 못했다: " + pathText(temp));
        }
    }

    fs::rename(temp, path, code);
    if (code) {
        std::error_code ignored;
        fs::remove(temp, ignored);
        throw YamlError("파일을 교체하지 못했다: " + pathText(path) + " (" + code.message() + ")");
    }
}

std::size_t YamlFile::removeStaleTempFiles(const fs::path& directory) {
    std::error_code code;
    std::size_t removed = 0;
    fs::recursive_directory_iterator it(directory, fs::directory_options::skip_permission_denied, code);
    if (code) return 0;
    const fs::recursive_directory_iterator end;
    for (; it != end; it.increment(code)) {
        if (code) break;
        if (!it->is_regular_file(code)) continue;
        if (it->path().extension() == tempSuffix()) {
            std::error_code removeCode;
            if (fs::remove(it->path(), removeCode)) ++removed;
        }
    }
    return removed;
}

}  // namespace gnn
