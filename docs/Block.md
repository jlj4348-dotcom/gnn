# Block

Framework 의 기본 단위, layer 들의 조합이지만 linear 조합만 가능 <br>
사용자가 activation 함수 및 runTime/Training option 지정 가능하고 yaml 파일로 save/load 한다

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| id | string | 고유 식별자 (block, network, model, trainer 포함) |
| name | string | Block 의 이름 |
| size | integer | Block 내 layer 개수 |
| layers | integer array | Block 내 layer 들의 크기 |
| connections | [connectionSpec.md] array | Block 내 layer 간 연결 정보 array (i번째가 i, i+1 layer 들의 연결 정보를 가짐) |
| activations | ActivationFunc array | Layer 마다 활성화 함수 지정 (default = Relu) |
| options | map <string, variant ?> | Layer 별 runTime/Training 옵션 (dropout, batch normalization 등) |


## Functions

| Function | Parameter | Return | Description
| --- | --- | --- | --- |
| getId | - | string | 고유 식별자 조회 |
| getName | - | string | Block 이름 조회 |
| setName | string | int | Block 이름 설정 |
| getSize | int | int | 지정 인덱스 Block 의 크기 조회 |
| setSize | int, int | int | 지정 인덱스 Block 의 크기 설정 |
| getNumOfLayers | - | int | Block 내 layer 들의 개수 조회 |
| getConnection | int | [connectionSpec.md] | 지정한 layer 의 연결 정보 조회 |
| setConnection | int, [connectionSpec.md] | [connectionSpec.md] | 지정한 layer 의 연결 정보 변경 |
| getActivation | int | ActivationFunc | 지정한 layer 의 활성함수 조회 |
| setActivation | int, ActivationFunc | int | 지정한 layer 의 활성함수 변경 |
| getOption | ? | ? | option 조회 |
| addOption | ? | ? | option 추가 |
| delOption | ? | ? | option 삭제 |