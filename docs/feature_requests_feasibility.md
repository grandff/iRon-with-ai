# iRon-Advanced 기능 요청사항(Feature Requests) 분석 및 구현 가능성 검토서

이 문서는 기존 iRon 프로젝트에 축적되었던 10개의 사용자 기능 요청사항(`docs/requests/`)에 대한 상세한 한국어 분석 및 iRacing SDK와 C++/DirectX 아키텍처 관점에서의 구현 가능성 검토 결과를 담고 있습니다.

---

## 📌 요약 및 총평
* **전체 구현 가능성**: **매우 높음** (10개 요청사항 중 9개 항목 즉시 구현 가능, 1개 항목은 대안 제시)
* **아키텍처 부합도**: 모든 요청사항이 iRon-Advanced의 핵심 철학인 **"가볍고, 의존성 없는(Standalone), 초고성능 60fps+ C++/DirectX 오버레이"**에 완벽히 부합하며, 시스템 리소스 추가 소모 없이 구현이 가능합니다.

---

## 🔍 상세 검토 내역 (01 ~ 10)

### 01. GPS 잡음 및 고도(Altitude) 데이터 로깅 기능 추가
* **요청 요약**: 자율주행 차량 라인 최적화 연구 및 랩타임 시뮬레이터를 개발하는 자동차 엔지니어가 실시간 트랙 맵핑을 정교하게 진행할 수 있도록, 기존 텔레메트리 로그에 iRacing의 GPS 위도(Latitude), 경도(Longitude) 및 고도(Altitude) 데이터를 포함해달라는 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Very Easy)**
* **iRacing SDK 지원 여부**: **지원함**
  * `Lat` (double, 위도 Degree)
  * `Lon` (double, 경도 Degree)
  * `Alt` (float, 고도 Meters)
* **C++ 구현 설계**:
  - `iracing.h` 및 `iracing.cpp`에 해당 변수(`ir_Lat`, `ir_Lon`, `ir_Alt`)를 `irsdkCVar` 매핑으로 등록합니다.
  - `TelemetryLogger.cpp`의 JSON 변환 루프(`loggingThread`)에서 해당 필드를 추가하여 `telemetry_debug.log` 파일에 위/경/고도 데이터를 실시간으로 인쇄하도록 가공합니다.

---

### 02. DDU 외 개별 연료 소모량 2개 평균(4랩 및 8랩) 기능 구현
* **요청 요약**: DDU(디지털 계기판) 오버레이를 켜지 않는 유저라도 연료 관리 효율을 모니터링할 수 있도록, 최근 4랩 평균 연료 소모량과 최근 8랩 평균 연료 소모량을 동시에 구해서 관리해주는 복수 평균 연료 계산 모델 도입 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Easy)**
* **iRacing SDK 지원 여부**: **연산 가능** (SDK는 현재 연료 수준 `FuelLevel`을 주며, 주행 랩 수에 맞춰 랩당 사용량을 추적해야 함)
* **C++ 구현 설계**:
  - `OverlayDDU.h` 혹은 별도 공통 연산 모듈에 최근 8랩 분량의 랩당 연료 소모량을 기록하는 원형 버퍼(Circular Buffer) 배열을 구현합니다.
  - 버퍼 데이터에 기초해 최근 4개 요소의 평균값과 8개 요소의 평균값을 실시간 연산하여, 연료 위젯이나 로그 스트림에 즉각적인 피드백을 주도록 처리합니다.

---

### 03. 상대 경로 실행 시 config.json 쓰기 권한 오류(Access Denied) 해결
* **요청 요약**: 외부 배치 스크립트나 터미널 단축 경로로 iRon을 호출하면 `config.json`이 저장되지 않고 권한 오류가 납니다. 실행 바이너리가 있는 위치나 특정 쓰기 권한 폴더에서 정상 저장되도록 상대 경로 예외를 제어해 달라는 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Already Architected)**
* **iRacing SDK 지원 여부**: 무관
* **C++ 구현 설계**:
  - 기존 코드에서는 `config.json`을 프로그램의 작업 디렉토리(Current Working Directory)에 생성 및 기록하여, 관리자 권한이 없거나 보호된 시스템 디렉토리에서 호출될 때 쓰기 에러가 납니다.
  - 앞서 해결한 `app.log` 및 `telemetry_debug.log`와 동일하게, `util.h`에 내장된 사용자 내문서 경로 도출 함수(`getRonDir()`)를 사용해 `Documents\iRon_Advanced\config.json`으로 물리적 파일 경로를 완전히 고정합니다. 이 경우 실행 디렉토리 위치와 무관하게 100% 쓰기 권한을 안전하게 확보합니다.

---

### 04. DDU 연료 위젯의 독립형 오버레이 분리 및 Inputs 그래프 바(Bar) 추가
* **요청 요약**:
  1. DDU 전체를 띄우지 않고 연료량 정보만 미니멀하게 볼 수 있도록, 연료량 막대 바 및 남은 랩 수, 랩 평균값, 최소/최대 소모량 등을 담은 독립된 **OverlayFuel**을 제작해 달라는 요청입니다.
  2. Inputs(입력값 그래프) 좌우 가장자리에 스티어링 외 클러치, 핸드브레이크 등의 물리적 바(Bar) 게이지를 부착해 달라는 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Highly Feasible)**
* **iRacing SDK 지원 여부**: 지원함 (`Clutch`, `HandbrakeRaw` 등 활용 가능)
* **C++ 구현 설계**:
  - **OverlayFuel**: `OverlayFuel.h`를 신설하여 `Overlay` 클래스를 상속받고, Direct2D를 사용해 컴팩트한 게이지 인터페이스를 구축합니다. DDU에서 이미 작동 중인 연료량 계산 공식을 재활용합니다.
  - **Inputs Bar**: `OverlayInputs.h` 내의 렌더링 영역 좌우 여백에 클러치와 핸드브레이크 실시간 게이지를 표현할 D2D rounded rectangle 바 채우기 코드를 추가합니다.

---

### 05. config.json 내 친근한 드라이버(Buddy) 리스트 구문 에러 예방
* **요청 요약**: `config.json` 파일에 친근한 동료나 주의해야 할 차량의 리스트(`buddies`, `flagged`)를 수동으로 기재할 때 문법 에러가 나거나 파싱 에러로 프로그램이 충돌하는 문제를 방지해 달라는 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Critical Bug Fix)**
* **iRacing SDK 지원 여부**: 무관
* **C++ 구현 설계**:
  - **버그 원인**: `Config::getStringVec`가 파싱 과정에서 원본 JSON 타입이 `array`인지를 검사(`value.is<picojson::array>()`)하지 않고 호출하므로, 유저가 실수로 일반 텍스트나 빈 값을 적을 시 picojson 파서가 즉시 크래시를 유발합니다. 또한 벡터의 사전 크기 지정(`std::vector<std::string> ret(arr.size())`) 후 `push_back`을 호출하여 앞쪽에 빈 문자열이 잔존하는 인덱싱 버그가 있었습니다.
  - **해결책**:
    ```cpp
    std::vector<std::string> Config::getStringVec(const std::string& component, const std::string& key, const std::vector<std::string>& defaultVal) {
        bool existed = false;
        picojson::value& value = getOrInsertValue(component, key, &existed);
        if (!existed) {
            // 초기 배열 등록 처리
        }
        if (!value.is<picojson::array>()) return defaultVal; // 크래시 철저 방어
        
        picojson::array& arr = value.get<picojson::array>();
        std::vector<std::string> ret; // 빈 형태로 초기화
        ret.reserve(arr.size());
        for(picojson::value& entry : arr) {
            if (entry.is<std::string>())
                ret.push_back(entry.get<std::string>());
        }
        return ret;
    }
    ```
    위와 같이 안전한 형태 검사 및 가변 할당 구조로 다듬어 사용자 실수에 대비한 무결성(Robustness)을 보장합니다.

---

### 06. DDU 계기판 내 앞/뒤 차량과의 상대 간격(Gap) 실시간 증감 알림
* **요청 요약**: Relative 박스의 작은 글씨를 주행 중 보기 힘들므로, DDU 화면 내에 앞차/뒷차와의 상대 간격(Gap)을 시각화하고, 거리가 좁혀지고 있는지 벌어지고 있는지 색상(예: 앞차가 가까워질 시 빨간색 경고, 뒷차가 벌어질 시 초록색 안전 등)으로 동적 피드백을 주는 미니 위젯 추가 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Easy)**
* **iRacing SDK 지원 여부**: 지원함 (`CarIdxEstTime`과 자차 랩타임 추정치를 사용해 델타 연산)
* **C++ 구현 설계**:
  - `OverlayDDU.h` 내부에서 자차 바로 앞과 뒤에 위치한 드라이버 인덱스를 지속 감지하고, 이들과의 `delta` 타임을 산출합니다.
  - 1초 전의 Gap 값을 보관해 두고 주기적으로 현재 값과 연산하여 격차가 좁혀지는지 판단하고, 좁혀지면 화살표 이모지($\Delta$ 혹은 $\nabla$)와 함께 빨간색/초록색 D2D 브러시 색상을 교체해 출력합니다.

---

### 07. 하이브리드(LMDh, F1, LMP1) 차량 ERS 배터리 잔량 및 디플로이 모드 출력
* **요청 요약**: 하이브리드 차량을 주행할 때 필수적인 배터리 충전량(SOC %) 및 전기 모터 배치(Deployment) 모드 정보를 DDU 화면에 통합하거나, 하이브리드 차량 탑승 시에만 자동으로 게이지가 표시되도록 구현해달라는 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Highly Feasible)**
* **iRacing SDK 지원 여부**: **지원함**
  * `EnergyPercent` (float, 배터리 충전 잔량 %)
  * `dcHybridEnergyLimit` (하이브리드 에너지 전개 제한 범위)
  * `dcHybridPacemode` 및 배터리 전개 옵션 값
* **C++ 구현 설계**:
  - `iracing.h`/`iracing.cpp`에 해당 ERS 변수들을 바인딩합니다.
  - `OverlayDDU.h`에서 현재 차종이 ERS를 사용하는지 확인하기 위해 `EnergyPercent` 변수값이 음수나 비정상 값이 아닌지(유효 여부) 체크합니다.
  - 유효한 경우 DDU 화면 중앙 혹은 하단에 배터리 잔량을 막대 그래프로 렌더링하고 모드 텍스트를 오버레이해 줍니다.

---

### 08. Input 오버레이 및 타 오버레이의 리플레이(Replay) 모드 렌더링 제어 옵션
* **요청 요약**: 주행 중에는 화면을 깨끗이 하고 오직 리플레이를 볼 때에만 내 입력값을 모니터링하고 싶으므로, 오버레이별로 '레이스 중에만 표시(Race Only)', '리플레이 시에만 표시(Replay Only)', '모두 표시(Both)'를 `config.json`에서 제어할 수 있게 기능을 일반화해달라는 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Easy)**
* **iRacing SDK 지원 여부**: **지원함**
  * `IsReplayPlaying` (bool, 현재 리플레이 재생 여부)
* **C++ 구현 설계**:
  - 각 오버레이의 Config에 `"display_mode"` 설정을 추가합니다 (값: `"race"`, `"replay"`, `"both"`).
  - `main.cpp`에서 오버레이 활성화 조건 검사 시 `ir_IsReplayPlaying.getBool()`의 참/거짓 값과 개별 오버레이의 설정값을 논리 연산(AND)하여 그리기를 스킵하거나 활성화 상태를 온오프 제어합니다.

---

### 09. Standings 또는 Relative 하단에 현재 랩(Lap) 정보 카운터 추가
* **요청 요약**: Standings나 Relative 오버레이의 상단 또는 하단 여백 공간에 현재 달리고 있는 세션의 랩 정보(예: `Lap: 12/25` 혹은 남은 시간)를 명확히 출력해 달라는 직관적인 편의성 개선 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Very Easy)**
* **iRacing SDK 지원 여부**: **지원함**
  * `Lap` (현재 진행 중인 랩)
  * `SessionLapsTotal` (전체 세션 목표 랩 수)
* **C++ 구현 설계**:
  - `OverlayStandings.h` 및 `OverlayRelative.h` 하단의 푸터 그리기 영역에 `SoF` 데이터 출력 옆 혹은 위 공간에 `Lap: %d / %d` 포맷 문자열을 작성하고, DirectWrite를 통해 화면에 렌더링해 줍니다.

---

### 10. 멀티클래스(Multi-class) 주행 시 후방에서 접근 중인 빠른 클래스 차량 강조 표시
* **요청 요약**: 여러 클래스가 동시에 달리는 멀티클래스 내구 레이스 주행 시, 내 뒤에서 빠른 다른 클래스 차량이 추월을 시도하거나 접근해 오고 있을 때, Relative 상에 해당 차량 행(Row)을 강력하게 강조/색상 표시하여 사고와 블루 플래그 대응을 수월하게 도와달라는 요청입니다.
* **구현 가능성 (Feasibility)**: **상 (Highly Feasible / High Utility)**
* **iRacing SDK 지원 여부**: **지원함**
  * `CarIdxClass` (각 차량의 클래스 고유 ID)
  * `CarClassEstLapTime` (클래스별 예상 기본 랩타임)
* **C++ 구현 설계**:
  - `OverlayRelative.h` 렌더링 루프 시, 대상 드라이버의 `CarIdxClass`가 자차의 클래스 ID와 다르고, 대상 클래스의 `CarClassEstLapTime`이 자차 클래스보다 현저히 작다면(더 빠른 클래스라면) 특수 하이라이트 트리거를 작동시킵니다.
  - 해당 차량이 후방에서 5초 이내(`delta < 0 && delta > -5.0f`)로 바짝 추적하고 있다면, 해당 드라이버 행의 배경에 반투명한 노란색/주황색 하이라이트 사각형을 Fill하고 깜빡임 효과를 가미해 드라이버의 측면/후면 주의집중도를 최고조로 올려 줍니다.
