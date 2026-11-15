#pragma once

#include <string>

#include "gnn/common/GraphKind.hpp"

namespace gnn {

// graph 이름은 폴더 이름이 되므로 Windows 에서도 쓸 수 있어야 한다.
//
// 규칙: 비어 있지 않고 UTF-8 로 100바이트 이하, 제어 문자와 `\ / : * ? " < > |` 를 쓰지 않고,
//       앞뒤에 공백이 없고 마침표로 끝나지 않으며, Windows 예약 이름(CON, PRN, AUX, NUL, COM1~9, LPT1~9.
//       확장자가 붙어도 마찬가지)이 아니다.
//       Design Graph 이름은 project 폴더 바로 아래에 놓이므로 `project.yaml`, `operators` 도 쓸 수 없다.
//
// 사용할 수 없으면 이유를 돌려주고, 사용할 수 있으면 빈 문자열을 돌려준다.
std::string checkGraphName(const std::string& name, GraphKind kind);

// 이름이 같은지 비교하는 키. 대소문자를 구분하지 않으므로 ASCII 영문자를 소문자로 바꾼다
// (한글 등 영문자가 아닌 글자는 그대로 둔다). 유일성을 확인할 때 이 키를 비교한다.
std::string graphNameKey(const std::string& name);

}  // namespace gnn
