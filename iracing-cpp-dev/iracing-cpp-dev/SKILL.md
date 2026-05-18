---
name: iracing-cpp-dev
description: iRacing C++ 오버레이 개발 스킬. iRon-with-ai 프로젝트에서 iRacing 텔레메트리 연동, DirectX UI 렌더링, 오버레이 기능(레이더, 순위표 등) 추가 시 사용합니다.
---

# iRacing C++ 오버레이 개발 스킬 (iracing-cpp-dev)

이 스킬은 `iRon-with-ai` 프로젝트(C++ 및 DirectX 기반)에 새로운 iRacing 오버레이 기능(레이더, 피트 헬퍼, 멀티클래스 경고 등)을 추가하고 텔레메트리 데이터를 연동할 때 사용하는 지침입니다.

## 🏁 워크플로우

새로운 오버레이 기능(예: 레이더)을 추가할 때는 다음 순서로 접근하세요.

1. **텔레메트리 데이터 식별 (Telemetry):**
   - 구현하고자 하는 기능에 필요한 iRacing SDK 변수(예: `CarIdxPosition`, `CarIdxEstTime`)를 파악합니다.
   - [references/iracing_telemetry.md](references/iracing_telemetry.md)를 참조하여 필요한 데이터가 현재 `iracing.cpp`에 래핑되어 있는지 확인합니다.
   - 래핑되어 있지 않다면 `iracing.h`와 `iracing.cpp`를 수정하여 메모리 매핑 변수를 추가합니다.

2. **데이터 처리 (Data Processing):**
   - UI 스레드와 분리된 로직(예: 예상 iRating 계산, 델타 타임 연산)은 별도의 유틸리티 함수나 클래스로 분리하여 계산 후 렌더링 스레드로 전달합니다.

3. **UI 렌더링 (DirectX / GDI):**
   - 새로운 오버레이 클래스(예: `OverlayRadar.h`, `OverlayRadar.cpp`)를 생성합니다.
   - 기존의 `Overlay.h` 인터페이스를 상속받아 `render()` 메서드를 구현합니다.
   - 그래픽 렌더링 기법은 [references/directx_ui.md](references/directx_ui.md)를 참조하세요.

4. **설정 연동 (Config):**
   - `Config.h`, `Config.cpp`에 새 오버레이의 On/Off 토글 단축키, 크기, 위치 저장 로직을 추가합니다.
   - `main.cpp`에 새 오버레이 인스턴스를 등록합니다.

## 📚 참조 문서

복잡한 구현이나 세부 API는 다음 레퍼런스를 참조하세요.

- **[references/iracing_telemetry.md](references/iracing_telemetry.md)**: iRacing 공유 메모리 SDK 변수 목록 및 데이터 획득 방법. (레이더, 멀티클래스, 피트 헬퍼 구현 시 필수 확인)
- **[references/directx_ui.md](references/directx_ui.md)**: iRon 프로젝트의 현재 DirectX/GDI 렌더링 구조 및 새로운 도형/텍스트를 그리는 방법.