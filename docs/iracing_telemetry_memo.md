# iRacing 텔레메트리 데이터 분석 (Phase 1) - 메모

iRon-with-ai 프로젝트의 기능 확장을 위해 iRacing SDK에서 제공하는 주요
 텔레메트리 변수들의 상태와 활용 방안을 분석했습니다. 현재 `iracing.cpp` 파일
 내에 대부분의 필요 변수들이 `irsdkCVar` 형태로 래핑되어 있어 즉시 활용
 가능합니다.
 
 ## 분석 완료된 핵심 SDK 변수
 
 ### 1. CarIdxEstTime (상대 거리/멀티클래스)
 - **타입:** `float[64]`
 - **iRon 상태:** `ir_CarIdxEstTime` 로 매핑 완료 (`iracing.cpp` 91라인)
 - **활용:** 각 차량의 랩 예상 도달 시간. 내 차량과의 델타 타임을 계산하여
 멀티클래스 접근 경고(블루 플래그 대응) 및 정밀한 상대 거리 렌더링에 사용됩니다.

 ### 2. CarIdxLapDistPct (일자형 맵)
 - **타입:** `float[64]`
 - **iRon 상태:** `ir_CarIdxLapDistPct` 로 매핑 완료 (`iracing.cpp` 83라인)
 - **활용:** 트랙 전체 길이를 기준으로 차량의 위치를 0.0 ~ 1.0 비율로
         나타냅니다. 띠 형태의 일자형 맵(Flat Map) 구현 시 필수 데이터입니다.

 ### 3. SessionFlags (사고/황기 알림)
 - **타입:** `bitfield`
 - **iRon 상태:** `ir_SessionFlags` 로 매핑 완료 (현재 DDU 오버레이에서 일부
         활용 중)
 - **활용:** 비트 마스킹을 통해 `irsdk_yellow`, `irsdk_blue` 등을 감별합니다.
         상단 배너 형태의 사고 알림 경고창에 사용합니다.

 ### 4. CarLeftRight (스포터/레이더)
 - **타입:** `int` (Enum)
 - **iRon 상태:** `ir_CarLeftRight` 로 매핑 완료 (`iracing.cpp` 162라인)
 - **활용:** iRacing 내장 스포터 데이터로, 양옆 차량 유무(0=없음, 1=Clear,
         2=왼쪽, 3=오른쪽, 4=양쪽)를 직관적으로 알려줍니다. 시각적 스포터 구현에 가장
         이상적인 데이터입니다.
      26
      27 ### 5. OnPitRoad & PitRepairLeft (피트 헬퍼)
      28 - **타입:** `bool`, `float`
      29 - **iRon 상태:** `ir_OnPitRoad`, `ir_PitRepairLeft` 로 매핑 완료
      30 - **활용:** 피트레인 진입 여부와 수리 남은 시간을 감지하여 피트 헬퍼 오버레이를
         자동으로 띄우고 카운트다운을 표시하는 데 사용합니다.
      31
      32 ## 공식 레퍼런스 및 관련 자료
      33 * [iRacing SDK C++ 통합 헤더/상수 참조
         (GitHub)](https://github.com/bencevans/iracing-telemetry/blob/master/irsdk_defi
         nes.h)
      34 * [iRacing Telemetry 변수 스키마 문서 (Go.dev 문서
         참조)](https://pkg.go.dev/github.com/sjeandeaux/iracing-telemetry)
      35 * [iRacing 커스텀 오버레이 제작 가이드 (SDK
         Gaming)](https://www.sdk-gaming.co.uk/)