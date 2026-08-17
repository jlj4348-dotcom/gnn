# AI Framework - core cencepts

Block -> Network -> Model/Trainer 로 조합된다.
Block, network 안에서 연결은 connectionSpec 으로 정의하며 model, trainer 에서는 checkpoint 로 가중치를 관리한다.

| 개념 | 설명 | Runnable | 문서 |
| --- | --- | --- | --- |
| ConnectionSpec | Layer 간 연결 방식 (Fully Connected, One2One, Custom) | ❌ | [ConnectionSpec.md] |
| Block | 하나 이상의 layer 간의 linear 조합으로 framework 의 기본 단위 | ❌ | [Block.md] |
| Network | Block 들의 연결 graph 로 복잡한 연결을 표현 | ❌ | [Network.md] |
| Checkpoint | Weight/Bias 의 snapshot (.ckpt) | ❌ | [Checkpoint.md] |
| Model | run() 함수를 위한 전용 경량 추론 객체 (Network, Checkpoint, DeviceManager 필요) | ✔️ | [Model.md] |
| Trainer | Network + Checkpoint + TrainingConfig + DeviceManager 로 checkpoint 관리 | ✔️ | [Trainer.md] |
| DeviceManager | Device 관리 객체로 tensor 연산 위임 | ❌ | [DeviceManager.md] |


## User Interface

| 개념 | 설명 |
| --- | --- |
| CLI | 명령어 기반 학습/추론 인터페이스 제공 |
| GUI | Godot 엔진 기반 graphic user interface 제공 |


## Directory Hierarchy

```
GNN
├─ docs
|	├─ AGENTS.md
|	├─ 
├─ src
|	├─ core
|	|
|	├─ cli
|	└─ gui
|
├─ include
|
├─ tests
|
├─ CMake
|
├─ config
├─ 
```