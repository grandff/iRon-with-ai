# DirectX / GDI UI 렌더링 레퍼런스

현재 `iRon` 프로젝트는 가벼운 구동을 위해 DirectX 디바이스를 초기화한 후, GDI(Graphics Device Interface) 기반의 텍스트 렌더링이나 매우 단순한 도형 그리기를 수행하는 구조를 띄고 있습니다.

## 오버레이 추가 방법

모든 오버레이는 `Overlay` 베이스 클래스를 상속받습니다.

```cpp
class OverlayMyNewFeature : public Overlay {
public:
    OverlayMyNewFeature(Config& config, iRacing& iracing);
    virtual void render() override;
};
```

### 도형 그리기 (레이더, 일자형 맵용)
기존 프로젝트에서 사각형(`FillRect`)이나 선(`LineTo`)을 그리는 방식은 투명도를 다루거나 둥근 사각형(차량 마커)을 그리는 데 한계가 있을 수 있습니다.
*   **주의사항:** 레이더의 차량 점(Dot)이나 일자형 맵의 트래픽 박스를 그릴 때, 너무 많은 렌더링 호출이 발생하면 프레임 드랍이 일어납니다.
*   **개선 고려:** ImGui(Dear ImGui) 라이브러리를 DirectX 백엔드와 연결하여 추가하는 것이 레이더나 복잡한 대시보드 렌더링에 훨씬 유리할 수 있습니다. (구조 변경 필요)

### 텍스트 그리기
*   폰트는 윈도우 기본 GDI 폰트를 로드하여 사용합니다.
*   순위표에 iRating 예상치 컬럼을 추가하려면, 텍스트의 X 오프셋 간격을 조정하는 `Config.json` 파라미터를 추가해야 글자가 겹치지 않습니다.

### 레이아웃 자동 전환 (세션별)
*   세션 타입(`SessionType`)이 변경될 때 이벤트를 캐치하여, `Config` 객체 안의 `visible` 속성을 오버라이드(override) 하는 방식이 필요합니다.