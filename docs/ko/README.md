<div align="center">

<h1>iRon-Advanced</h1>
Expanded lightweight overlays for iRacing.<br><br>

[**English**](../../README.md) | **한국어**

</div>

---

이 프로젝트는 기존 `iRon` 오버레이를 대폭 확장한 버전입니다. '가볍고 외부 프로그램 의존성이 없다'는 기존의 철학을 그대로 유지하면서, 최신 오버레이들에서 지원하는 강력한 필수 기능들(레이더, 스포터, 일자형 맵 등)을 모두 통합했습니다.

### 원본 iRon과 비교해서 무엇이 달라졌나요?
1. **글로벌 레이아웃 완벽 저장 (위치 초기화 문제 해결!)**
   * 원본 iRon은 프로그램을 껐다 켜면 창 위치가 매번 초기화되는 치명적인 단점이 있었습니다. **iRon-Advanced**는 사용자의 화면 배치 좌표, 창 크기, 그리고 오버레이를 켜고 끈 상태를 윈도우의 `내 문서\iRon_Advanced\config.json` 경로에 영구적으로 안전하게 저장합니다. 이제 게임을 켤 때마다 마지막에 설정해둔 화면 배치가 그대로 자동 복구됩니다!
2. **순위표(Standings) 및 텔레메트리 고도화**
   * 순위표 오버레이에 **실시간 예상 iRating 증감 수치 (+/-)** 가 추가되었으며, 내 주변 드라이버들이 어떤 차를 타고 있는지 알 수 있는 **차종(Car Model)** 텍스트 컬럼이 추가되었습니다.
3. **7개의 강력한 신규 오버레이 추가:**
   * **근접 레이더 (Radar), 시각적 스포터 (Spotter)**
   * **사고/황기 스마트 알림 (Incident Warning):** 전방 150m 내 사고 발생 시 거리 표시.
   * **멀티클래스 트래픽 경고 (Traffic):** 상위 클래스 차량 접근 시 랩타임 차이에 따른 동적 블루 플래그 UI.
   * **일자형 맵 (Flat Map):** 전체 트랙의 트래픽을 한눈에 보는 리본 맵.
   * **델타 바 (Delta Bar):** 최적 랩(Optimal) 대비 실시간 가로형 게이지.
   * **피트 헬퍼 (Pit Helper):** 피트레인 진입 시 속도 제한 및 정차 수리 시간 표시.

---

# 목차

- [다운로드 위치](#다운로드-위치)
- [오버레이 종류](#오버레이-종류)
  - [*Relative (상대 거리)*](#relative-상대-거리)
  - [*DDU (대시보드)*](#ddu-대시보드)
  - [*Inputs (입력 그래프)*](#inputs-입력-그래프)
  - [*Standings (순위표)*](#standings-순위표)
  - [*Cover (가림막)*](#cover-가림막)
  - [*Spotter (스포터) (NEW)*](#spotter-스포터-new)
  - [*Radar (레이더) (NEW)*](#radar-레이더-new)
  - [*Incident Warning (사고 경고) (NEW)*](#incident-warning-사고-경고-new)
  - [*Multiclass Traffic (멀티클래스) (NEW)*](#multiclass-traffic-멀티클래스-new)
  - [*Flat Map (일자형 맵) (NEW)*](#flat-map-일자형-맵-new)
  - [*Delta Bar (델타 바) (NEW)*](#delta-bar-델타-바-new)
  - [*Pit Helper (피트 헬퍼) (NEW)*](#pit-helper-피트-헬퍼-new)
- [설치 및 실행](#설치-및-실행)
- [설정 변경 (Configuration)](#설정-변경-configuration)
- [소스 코드에서 빌드하기](#소스-코드에서-빌드하기)
- [의존성](#의존성)

---

## 다운로드 위치

최신 실행 파일(Binary)은 [이곳 (Releases)](https://github.com/lespalt/iRon/releases/latest)에서 다운로드할 수 있습니다.

## 오버레이 종류

### *Relative (상대 거리)*
iRacing의 기본 Relative 박스와 유사하지만 라이선스, iRating, 마지막 피트스탑 이후 주행 랩 수 등의 추가 정보를 제공합니다. 친구의 이름을 등록하여 초록색으로 강조할 수도 있습니다.
상단에는 미니맵이 포함되어 있으며, 내 차를 중앙에 고정(Relative)하거나 결승선을 중앙에 고정(Absolute)하도록 설정할 수 있습니다.

![relative](https://github.com/lespalt/iRon/blob/main/relative.png?raw=true)

### *DDU (대시보드)*
iRacing에서 여러 블랙박스를 넘겨봐야 알 수 있는 중요한 정보들을 한곳에 모아둔 대시보드입니다.
연료 계산기는 예상 남은 랩, 남은 연료량, 랩당 예상 연료 소모량, 완주를 위해 필요한 추가 연료량, 그리고 다음 피트스탑에서 주입될 예정인 연료량을 보여줍니다.

![ddu](https://github.com/lespalt/iRon/blob/main/ddu.png?raw=true)

### *Inputs (입력 그래프)*
스로틀, 브레이크, 스티어링의 입력값을 실시간으로 움직이는 그래프로 보여줍니다. 일관된 트레일 브레이킹을 연습하는 데 유용합니다.

![inputs](https://github.com/lespalt/iRon/blob/main/inputs.png?raw=true)

### *Standings (순위표)*
안전 등급(SR), iRating, 마지막 피트스탑 이후 주행 랩 수, **실시간 예상 iRating 증감 수치**, 그리고 **차종(Car Model)**을 포함하여 필드 전체의 순위를 보여줍니다. 주로 코션 상황이나 레이스 전에 상대방의 실력을 파악할 때 유용합니다.

![standings](https://github.com/lespalt/iRon/blob/main/standings.png?raw=true)

### *Cover (가림막)*
스크린샷은 생략합니다. 넥스트 젠 NASCAR처럼 시야를 방해하는 인게임 대시보드를 가릴 수 있는 단순한 빈 사각형입니다.

### *Spotter (스포터) (NEW)*
차량이 사각지대에 있을 때 화면 좌우에 투명한 색상의 사각형을 렌더링하는 시각적 스포터입니다. iRacing의 기본 스포터 텔레메트리와 직접 연동됩니다.

![spotter](../../screenshot_placeholder_spotter.png)

### *Radar (레이더) (NEW)*
내 차를 중심으로 주변 차량을 탑다운 뷰로 보여주는 근접 레이더입니다. 내 차는 중앙에 초록색으로 표시되며, 앞뒤 또는 옆에 있는 차량은 실제 거리에 비례하여 빨간색 사각형으로 그려집니다.

![radar](../../screenshot_placeholder_radar.png)

### *Incident Warning (사고 경고) (NEW)*
단순한 황기 배너가 아닌 스마트 사고 경고 시스템입니다. 코션 상황 중 내 앞쪽 150미터 이내에서 차가 트랙을 벗어나거나 충돌한 것이 감지되면, 사고 지점까지의 정확한 거리와 함께 깜박이는 경고 배너를 띄워 대처를 돕습니다.

![incident](../../screenshot_placeholder_incident.png)

### *Multiclass Traffic (멀티클래스) (NEW)*
멀티클래스 레이싱을 위한 동적 블루 플래그 경고 시스템입니다. 주변 차량들의 랩타임 페이스를 비교하여, 나보다 상위 클래스의 빠른 차량이 뒤에서 접근할 때 시야를 방해하지 않는 은은한 글로우 UI와 텍스트로 미리 양보를 준비할 수 있게 알려줍니다.

![traffic](../../screenshot_placeholder_traffic.png)

### *Flat Map (일자형 맵) (NEW)*
전체 트랙을 일직선으로 펼쳐 보여주는 리본 맵입니다. 멀티클래스나 내구 레이스에서 전체 트래픽 팩(Pack)의 흐름을 한눈에 파악하는 데 필수적입니다. 내 차는 긴 초록색 마커로 표시되며, 다른 차들은 랩 수에 따라(동일 랩=흰색, 한 랩 앞=빨간색, 한 랩 뒤=파란색) 다르게 표시됩니다.

![flatmap](../../screenshot_placeholder_flatmap.png)

### *Delta Bar (델타 바) (NEW)*
현재 랩타임 페이스를 세션의 옵티멀(Optimal) 또는 베스트 랩과 비교하는 실시간 가로형 게이지입니다. 내가 더 빠르면 왼쪽으로 초록색 바가 차오르고, 느리면 오른쪽으로 빨간색 바가 차오르며, 수치 단위의 델타 타임도 함께 표시됩니다.

![deltabar](../../screenshot_placeholder_deltabar.png)

### *Pit Helper (피트 헬퍼) (NEW)*
피트레인에 진입할 때만 나타나는 전용 오버레이입니다. 현재 속도와 트랙의 피트 속도 제한을 보여주며, 리미터가 꺼져 있을 경우 붉은색으로 점멸하여 페널티를 방지합니다. 피트 박스에 정차하면 필수 수리 시간 타이머로 자동 전환됩니다.

![pithelper](../../screenshot_placeholder_pithelper.png)

---

## 설치 및 실행

이 앱은 설치가 필요 없습니다. 원하는 폴더에 실행 파일(`.exe`)을 복사하기만 하면 됩니다. 

사용하려면 그냥 실행 파일을 더블클릭하세요. iRacing을 켜기 전이나 후 언제 실행하든 상관없습니다. iRon이 실행 중임을 알리는 콘솔 창이 나타나며, iRacing에서 차를 타고 트랙에 들어가면 오버레이가 나타납니다. iRacing을 **테두리 없는 창 모드(Borderless Window)** 로 실행하는 것을 권장합니다.

---

## 설정 변경 (Configuration)

오버레이의 위치를 옮기거나 크기를 조절하려면 **ALT-J**를 누르세요. 마우스로 드래그하여 창을 옮기고, 우측 하단을 잡아 크기를 조절할 수 있습니다. 완료 후 다시 **ALT-J**를 누르면 정상 모드로 돌아갑니다.

프로그램이 켜질 때 콘솔 창에 안내되는 단축키를 눌러서 런타임 중에 원하는 오버레이만 끄거나 켤 수 있습니다.

사용자의 레이아웃 및 설정 내역은 다음 경로에 안전하게 영구 저장됩니다:
`내 문서\iRon_Advanced\config.json`

글꼴, 텍스트 크기, 색상 등 세부 사항을 조절하고 싶다면, 프로그램이 켜져 있는 상태에서 저 `config.json` 파일을 열고 원하는 값으로 수정한 뒤 저장(Ctrl+S)해 보세요. 변경 사항이 화면에 즉시 실시간으로 반영됩니다!

---

## 소스 코드에서 빌드하기

이 앱은 Visual Studio 2022로 빌드되었습니다. 기본 솔루션 파일들이 바로 작동하며, DirectX 응용 프로그램을 빌드하는 데 필요한 몇 가지 추가 정적 라이브러리(Windows SDK 등)만 설치되어 있다면 손쉽게 직접 빌드할 수 있습니다.

---

## 의존성

DirectX와 같은 표준 Windows 구성 요소 외에는 실행을 위한 별도의 런타임 의존성이 없습니다. (iRacing을 구동할 수 있는 PC라면 이미 모두 갖춰져 있습니다.)
빌드를 위한 iRacing SDK 및 picojson과 같은 최소한의 의존성은 이미 리포지토리 내에 포함되어 있습니다.