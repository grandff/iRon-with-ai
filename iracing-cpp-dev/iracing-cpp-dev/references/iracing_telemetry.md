# iRacing 텔레메트리 (SDK) 레퍼런스

iRacing은 메모리 매핑 파일(Memory Mapped File)을 통해 60Hz(또는 그 이상)의 속도로 텔레메트리 데이터를 뿜어냅니다. `iRon` 프로젝트는 `irsdk` 폴더 내의 헤더들을 사용하여 이 데이터를 읽어옵니다.

## 주요 데이터 분류

### 1. 세션 정보 (Session String / YAML)
세션 초기에 한 번(또는 드물게 업데이트) 제공되는 정적 데이터입니다. YAML 형식 문자열로 제공됩니다.
*   **활용:** 플레이어 차량 번호, 트랙 길이, 참가자 목록(이름, iRating, 클래스 색상, 차량 모델).
*   **추가 기능 시:** 순위표에 차량 브랜드 로고나 멀티클래스 색상을 넣으려면 `DriverInfo` 섹션 파싱 로직을 확장해야 합니다.

### 2. 실시간 텔레메트리 (Telemetry Variables)
프레임마다 갱신되는 동적 데이터입니다. `iracing.cpp`의 `update()` 함수 등에서 읽어옵니다.

#### 안전 및 레이더 (우선순위 1) 관련 변수
*   `CarIdxEstTime` (float array): 각 차량의 랩 기준 예상 시간(초). 앞/뒤 차량과의 간격(Delta) 계산 및 멀티클래스 경고에 사용.
*   `CarIdxLapDistPct` (float array): 각 차량이 트랙의 어느 위치에 있는지(0.0 ~ 1.0). 일자형 맵(Flat Map) 구현에 필수.
*   `SessionFlags` (bitfield): 현재 발생 중인 깃발(Green, Yellow, Blue, White 등). 사고 경고 오버레이에 사용.
*   `CarLeftRight` (int): 0(없음), 1(왼쪽), 2(오른쪽), 3(양쪽). iRacing 내장 스포터 데이터로 시각적 스포터 구현 시 가장 쉽게 사용 가능.

#### 피트 헬퍼 관련 변수
*   `OnPitRoad` (bool): 플레이어가 피트레인에 있는지 여부. 피트 헬퍼 오버레이 활성화 트리거.
*   `PitRepairLeft` (float): 수리 남은 시간.

## `iRon` 에서의 변수 읽기
새로운 변수를 읽으려면 `iracing.cpp` 내부에 다음과 같이 정의합니다.

```cpp
// 예: SessionFlags 읽기
int flags = irsdk_getVarInt("SessionFlags");
if (flags & irsdk_yellow) {
    // 황기 발생 처리
}
```
*배열 데이터(CarIdx...)는 `irsdk_getVarFloatArray` 등의 유틸 함수를 활용해야 할 수 있습니다.*