# UI 렌더링 프레임워크 검토 결과 (Phase 1)

## 현재 iRon 렌더링 아키텍처 분석
`Overlay.h` 및 `Overlay.cpp` 코드를 분석한 결과, iRon 프로젝트는 현재 다음과 같은 윈도우 네이티브 하드웨어 가속 그래픽 API를 사용하고 있습니다:
1.  **Direct2D (`ID2D1RenderTarget`, `ID2D1SolidColorBrush`)**: 2D 도형 렌더링 및 색상/투명도 처리.
2.  **DirectWrite (`IDWriteFactory`)**: 고품질 텍스트 렌더링.
3.  **DirectComposition (`IDCompositionDevice`)**: 투명한 오버레이 창 구현(배경 투명화 및 게임 위에 띄우기).

## 검토 및 결론: Dear ImGui 도입 여부
초기에는 복잡한 렌더링을 위해 ImGui와 같은 외부 GUI 라이브러리 도입을 고려했으나, 현재의 **Direct2D 구조를 그대로 유지하는 것이 최적**이라고 판단됩니다.

### 1. 외부 의존성 제거 (프로젝트 철학 유지)
iRon의 `README.md`에 명시된 핵심 철학은 **"free of external dependencies(외부 의존성 없음)"**와 **"lightweight(경량화)"**입니다. ImGui를 연동하기 위해서는 라이브러리를 프로젝트에 포함하고 렌더링 파이프라인을 수정해야 하므로 이 철학에 위배됩니다.

### 2. Direct2D의 렌더링 성능
우리가 추가하려는 핵심 기능(근접 레이더의 원형 마커, 일자형 맵의 사각형 게이지, 델타 바 등)은 모두 기본적인 2D 기하학 도형입니다. Direct2D는 이러한 2D 도형 렌더링에 대해 최상급의 하드웨어 가속 성능을 제공합니다. 

### 3. 상호작용(Interaction)의 부재
ImGui는 체크박스, 스크롤바, 드롭다운 메뉴 등 "상호작용이 가능한 복잡한 위젯"을 만들 때 빛을 발합니다. 하지만 심레이싱 오버레이는 주행 중 단순히 데이터를 "보여주는(Display)" 역할만 하므로 복잡한 위젯이 필요하지 않습니다. 

## 향후 개발 방향 (Action Item)
기존 `Overlay` 클래스나 유틸리티 함수에 레이더 구현을 위한 **원/타원 그리기 헬퍼 함수 (`FillEllipse`, `DrawEllipse`)**가 없다면, `m_renderTarget`을 활용하여 간단히 추가한 뒤 오버레이 렌더링을 진행합니다. 외부 라이브러리 추가 없이 기존 DirectX 파이프라인 안에서 모든 Phase 2, 3 기능을 충분히 가볍고 빠르게 구현할 수 있습니다.