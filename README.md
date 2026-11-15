# gnn

그래프 기반으로 neural network 를 **설계하고, 훈련하고, 분석**하는 C++ framework.
사용자는 UI 로 쉽게 network 를 만들고 훈련할 수 있고, 훈련된 결과는 C++ 코드로 내보내 다른 프로젝트에서 사용할 수 있다.

> **상태: 설계 단계.** 핵심 개념과 규칙을 정리하는 중이며, 빌드 설정(CMakeLists.txt)만 있고 구현 코드는 아직 없다.

## 핵심 아이디어

network 를 **세 종류의 graph** 로 나누어 다룬다.

```
Design Graph ──(최적화)──▶ Training Graph ──(추론 변환)──▶ Inference Graph
  설계 (구조만)              훈련 (tensor 연산)               추론 (스냅샷)
```

| Graph | 역할 | 특징 |
|---|---|---|
| **Design Graph** | 사용자가 설계하는 구조 | node/edge 단위로 세밀하게 표현. weight 를 갖지 않는다 |
| **Training Graph** | 실제 훈련용 | Design Graph 를 tensor 연산 단위로 최적화. 하나의 Design Graph 에서 여러 개를 만들 수 있고 각각 독립적으로 훈련한다 |
| **Inference Graph** | 추론용 | 사용자가 요청할 때만 만든다. 만든 시점의 Training Graph 스냅샷이며 C++ export 의 기준이 된다 |

### 구성 요소

- **Node**: perceptron, layer, network 중 하나. perceptron 은 반드시 layer 에 속하고, layer 는 network 로 묶거나 해체할 수 있으며, network 안에 network 를 넣을 수 있다. network 는 이름 붙은 **port**(입구/출구)로만 바깥과 연결되므로 내부가 바뀌어도 외부 연결이 유지된다.
- **Edge**: node 사이의 연결. weight 는 edge 에 붙는다.
- **Edge group**: Training Graph 로 최적화하는 단위. group 에 속한 edge 와 연결된 node 가 하나의 sub network 를 이룬다. group 끼리 겹쳐도 된다.
- **Trainer**: 훈련 방식(supervised / unsupervised / reinforcement)을 추상화한다. 데이터 소스, loss, optimizer, 훈련 루프의 조합이며 preset 을 골라 간단하게 훈련할 수 있다.

## 주요 기능

### network 재사용
- network 일부를 먼저 독립적으로 훈련한 뒤, 훈련된 값을 유지한 채 다른 graph 에 붙여 쓸 수 있다.
- 붙일 때는 weight 를 복사하고 원본(Design Graph 또는 Training Graph)을 기억한다. 사용자가 **load** 를 요청할 때만 원본 → 붙인 쪽 방향으로 가져온다.
- 붙인 network 는 구조를 바꿀 수 없다. 바꾸려면 해체해서 직접 만든 것으로 취급한다.

### 훈련
- **freeze**: 훈련 설정에서 node 를 지정하면 그 weight 는 훈련해도 바뀌지 않는다 (node 종류와 무관).
- **dropout**: Training Graph 마다 대상 node(Design Graph 의 id)와 확률 p 를 지정한다. 훈련 step 마다 일부 node 를 비활성화한 instance 로 훈련하고 결과를 가장 큰 graph 에 반영한다. 비활성화 단위는 node 이며, node 가 perceptron 인지 layer 인지 network 인지는 구분하지 않는다. layer 를 지정할 때는 layer 전체를 끌지, layer 안의 perceptron 을 각각 끌지 선택한다.
- **loss**: 사용자가 선택하며 Trainer 가 붙인다. Training Graph 는 순전파만 가진다.
- **양자화**: 선택 옵션이며 Trainer 가 수행한다. 훈련 중에도 적용하거나 바꿀 수 있다.
- **통계**: 훈련별로 heatmap 등 통계를 제공한다.

### clone 모델 (dropout, checkpoint, 실험)
dropout instance, checkpoint, 실험용 복제본은 모두 **부모 Training Graph 의 clone** 으로 설명한다. 차이는 mask 와 **merge**(clone 의 값을 부모에 반영, mask 가 1 인 부분만)를 언제 하느냐뿐이다.

| 종류 | mask | merge | 수명 |
|---|---|---|---|
| dropout instance | 확률 p 로 일부 비활성 | 매 step 자동 | 일시적 (이름 없음, 저장하지 않음) |
| checkpoint | 전부 활성 | 되돌리기를 요청할 때 | 부모에 종속, 부모 id + 날짜/시간으로 식별 |
| 실험용 clone | 전부 활성 | 사용자가 요청할 때만 | 독립 Training Graph (새 이름) |

- 사용자가 요청하거나 설정한 시점에 checkpoint(snapshot)를 저장하고, 목록에서 골라 되돌릴 수 있다. 되돌리기는 checkpoint 를 부모에 merge 하는 것이며, 그 전에 현재 상태를 남길지 묻는다.
- checkpoint 는 특정 Training Graph 에 종속되며 **Training Graph id + 날짜/시간** 으로 식별한다.
- Training Graph 를 **clone** 하여 learning rate 등을 달리한 실험을 비교할 수 있다. clone 의 원본은 현재 상태 또는 선택한 checkpoint 의 상태이다. 결과가 좋으면 사용자가 요청해서 원본에 merge 할 수 있다.
- 부모의 구조가 clone 시점과 다르면 merge 할 수 없다.
- Training Graph 의 구조는 만들어진 뒤 바뀌지 않는다. 그래서 훈련 결과(weight, checkpoint)는 Design Graph 편집으로 사라지지 않는다.

### Design Graph 편집
- 모든 변경은 command 로 처리하며 **undo/redo** 를 지원한다 (Design Graph 편집에만 적용. 훈련을 되돌리는 것은 checkpoint 의 역할이다).
- **분기**: Training Graph 가 있는 Design Graph 의 구조를 편집하면 새 Design Graph 로 분기해서 편집한다. 원본과 그 Training Graph 는 그대로 남는다. name, description, 화면 위치 변경은 분기하지 않는다.
- 분기한 Design Graph 에서 새 Training Graph 를 만들 때 부모의 Training Graph 를 지정하면, edge id 와 shape 가 일치하는 weight 를 이어받는다.
- **텍스트 모드**와 **그래픽 모드**로 Design Graph 를 본다. 텍스트 모드는 perceptron 까지 모두 보고 편집하며 CLI/YAML 의 UI 버전이다. 그래픽 모드는 layer 와 subnetwork 만 그려서 큰 graph 도 다룰 수 있다. network 는 폴더처럼 들어가서 본다.

### 식별과 저장
- graph 는 파일에서는 **이름**으로 식별하고(폴더 구조 `<project 이름>/<graph 이름>/design`, `training/<이름>`, `inference/<이름>` — Design Graph 폴더 안에 그것에서 만든 Training/Inference Graph 가 모인다. 같은 이름에 저장하면 덮어쓸지 묻는다), 메모리에 올릴 때 **graph id** 를 발급받아 그 뒤로는 id 로 동작한다. graph id 는 저장하지 않으며 실행마다 달라질 수 있다.
- node 와 edge 는 id 를 가지며 graph 마다의 카운터가 발급하고 삭제된 id 는 재사용하지 않는다. 이 id 는 파일에 저장된다. **(graph, node/edge id)** 쌍이 전역에서 유일하다.
- 저장 형식은 **구조 = YAML, 값 = binary** 이다.
- 훈련이 끝나면 Inference Graph 를 만들어 C++ 코드로 export 한다.

## 기술 스택

| 항목 | 내용 |
|---|---|
| 언어 | C++17 |
| 빌드 | CMake + MSVC (Visual Studio 2026) |
| 테스트 | doctest |
| 실행 장치 | CPU 먼저, CUDA 는 나중. backend 로 추상화해 Apple/AMD/Intel 확장을 염두에 둔다 |
| tensor | 외부 수치 라이브러리 없이 직접 구현. 정확성을 먼저 확보하고 이후 최적화(블록 처리, SIMD, 멀티스레드) |
| UI | Godot 4.x (GDExtension 으로 C++ 코어 연결) |
| 개발 순서 | **CLI 를 먼저 완성**하고 그 뒤에 Godot UI 를 개발한다 |

## 디렉터리 구조

```
import/gnn/<module>/   공개 헤더 (.hpp)
src/gnn/<module>/      구현 (.cpp)
tests/                 테스트
docs/                  문서
```

- 헤더와 구현은 같은 이름의 module 디렉터리에 둔다.
- include 경로의 루트는 `import/` 이므로 `#include "gnn/<module>/<File>.hpp"` 형태로 쓴다.
- 파일명은 PascalCase 를 따른다 (예: `Graph.hpp`).

## 빌드

필요한 것: Windows, Visual Studio 2026 (MSVC), CMake 3.20 이상. (개발 환경: Visual Studio Community 2026 MSVC 14.52, CMake 4.5.0, CUDA 13.3)

```
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Debug
```

- 각 module 은 `import/gnn/<module>/` 과 `src/gnn/<module>/` 에서 자동으로 구성된다. `.cpp` 가 있으면 정적 라이브러리, 없으면 헤더 전용이다.
- `src/gnn/cli/` 에 소스가 생기면 실행 파일 `gnn` 이 만들어진다.
- 테스트는 doctest(`third_party/doctest/`)로 작성하고, `tests/` 에 테스트가 생기면 `ctest --test-dir build -C Debug` 로 실행한다.

## 로드맵

1. **CLI 코어**: Design Graph, Training Graph, tensor 와 연산(순전파/역전파), supervised Trainer
2. **unsupervised Trainer**: Autoencoder 부터 시작해 필요에 따라 확장
3. **Inference Graph 와 C++ export**
4. **CUDA backend**
5. **Godot UI** (CLI 완성 후)
6. **reinforcement Trainer** (환경 인터페이스는 그때 결정)

## 아직 정하지 않은 것

- 첫 버전에서 지원할 연산의 범위
- tensor 세부(shape, layout, stride, view), backend 인터페이스의 구체적 모양, batch/shape 규칙, RNG 와 seed
- 저장 세부(YAML 라이브러리, binary 레이아웃, project 단위 파일)
- checkpoint 보관 개수와 디스크 용량 관리
- 양자화의 구체적 방식과 binary 저장 형식
- export 시 weight 은닉, reinforcement Trainer 의 환경 인터페이스
