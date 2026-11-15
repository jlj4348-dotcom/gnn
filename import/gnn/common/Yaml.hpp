#pragma once

#include <string>
#include <string_view>

#include "gnn/common/YamlNode.hpp"

namespace gnn {

// 이 프로젝트의 파일에 쓰는 YAML 부분집합을 읽고 쓴다.
//
// 지원: 들여쓰기(공백)로 만드는 map/sequence, 한 줄 또는 여러 줄 flow({}, []),
//       따옴표 문자열, plain scalar, 주석(#), 맨 앞의 `---`.
// 지원하지 않음(만나면 YamlError): 탭 들여쓰기, 여러 문서, anchor/alias/tag,
//       `|` `>` 블록 문자열, 여러 줄 plain/따옴표 문자열.
// 주석은 읽을 때 버리므로 다시 쓰면 사라진다.

// text 를 해석한다. 문법 오류는 YamlError (줄 번호 포함).
YamlNode parseYaml(std::string_view text);

// node 를 YAML 문자열로 만든다. 마지막 줄 끝에 줄바꿈이 있다.
// 한 줄(flow)에 flowLimit 글자 안으로 들어가는 map/sequence 는 한 줄로, 아니면 여러 줄로 쓴다.
// 최상위 map/sequence 는 항상 여러 줄이다.
std::string emitYaml(const YamlNode& node, std::size_t flowLimit = 100);

}  // namespace gnn
