# AI Framework - core cencepts

Neural network 를 graph 로 표현하는 ai framework <br>
Graph = Node + Edge

## Neural Network Graph
**DAG (Directed Acyclic Graph)** 로 표현되야 한다

### Node

Neural network 에서 perceptron 에 해당하며 INPUT/HIDDEN/OUPUT type 중에서 하나의 type 을 반드시 갖는다 <br>
INPUT node (depth = 0) 를 시작으로 OUTPUT node 까지의 경로에서 depth 가 layer 번호로 간주한다

| Type | Description |
| -- | -- |
| INPUT | 시작 node, depth = 0 |
| HIDDEN | 중간 node |
| OUTPUT | 끝 node |

### Edge

Neural network 에서 perceptron 간 연결에 해당하며 operator 를 통해 node 간의 데이터를 전달한다 <br>
Operator 로는 activation function, weight, bias, dropdown, 양자화 등을 표현한다

| 대분류 | 소분류 | Description | 예시 |
| -- | -- | -- | -- |
| Flow Operator | Single-Edge Operator | **순서**에 따라서 결과가 달라지는 operator | weight, bias, act. function |
| Flow Operator | Cross-Edge Operator | **순서**에 따라서 결과가 달라지는 operator | cross-layer normalization |
| Switch Operator | - | 순서가 영향을 주지 않는 operator (optional) | dropdown, 양자화 |


## User Interface

| 개념 | 설명 |
| --- | --- |
| CLI | 명령어 기반 학습/추론 인터페이스 제공 |
| GUI | Godot 엔진 기반 graphic user interface 제공 |

