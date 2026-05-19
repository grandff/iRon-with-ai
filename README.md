# iRon-Advanced - expanded lightweight overlays for iRacing

## 🇬🇧 English (About & What's New)

This project is an expanded version of the original `iRon` overlays, aiming to provide additional advanced features while maintaining the original philosophy. It incorporates highly requested features found in modern overlay suites (like Radar, Spotter, Flat Map, etc.). It remains extremely lightweight, easy to modify, and free of external runtime dependencies.

### What's different from the original iRon?
1. **Global Layout Persistence (No more resetting positions!)**
   * The original iRon lost window positions if the app was restarted. In **iRon-Advanced**, your overlay positions, sizes, and toggle states (on/off) are safely and globally saved to `Documents\iRon_Advanced\config.json`. Every time you launch the `.exe`, your exact last layout is restored automatically.
2. **Enhanced Telemetry & Standings**
   * The Standings overlay now calculates **real-time Expected iRating changes** based on the Elo system and displays the **Car Model/Manufacturer** column.
3. **7 Brand New Overlays Added:**
   * **Radar:** Proximity top-down radar.
   * **Spotter:** Visual blind-spot warnings.
   * **Incident Warning:** Smart forward-crash detection.
   * **Traffic (Multiclass):** Dynamic blue-flag warnings for faster approaching classes.
   * **Flat Map:** Linear traffic ribbon map.
   * **Delta Bar:** Real-time optimal/best lap delta gauge.
   * **Pit Helper:** Speed limit and pit stop repair timer.

---

## 🇰🇷 한국어 (소개 및 달라진 점)

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

# Contents

- [Where to Download](#where-to-download)
- [Overlays](#overlays)
  - [*Relative*](#relative)
  - [*DDU*](#ddu)
  - [*Inputs*](#inputs)
  - [*Standings*](#standings)
  - [*Cover*](#cover)
  - [*Spotter (NEW)*](#spotter-new)
  - [*Radar (NEW)*](#radar-new)
  - [*Incident Warning (NEW)*](#incident-warning-new)
  - [*Multiclass Traffic (NEW)*](#multiclass-traffic-new)
  - [*Flat Map (NEW)*](#flat-map-new)
  - [*Delta Bar (NEW)*](#delta-bar-new)
  - [*Pit Helper (NEW)*](#pit-helper-new)
- [Installing & Running](#installing--running)
- [Configuration](#configuration)
- [Building from source](#building-from-source)
- [Dependencies](#dependencies)

---

## Where to Download

The latest binary release can be found [here](https://github.com/lespalt/iRon/releases/latest).

## Overlays

### *Relative*
Like the *Relative* box in iRacing, but with additional information such as license, iRating, and laps driven since the last pit stop. You can also highlight your friends by adding their names to a buddy list.
At the top is an optional minimap. It can be set to either relative mode (own car fixed in the center) or absolute mode (start/finish line fixed in the center).

![relative](https://github.com/lespalt/iRon/blob/main/relative.png?raw=true)

### *DDU*
A dashboard that concentrates important pieces of information for which you would otherwise have to flip through various boxes in iRacing.
The fuel calculator shows the estimated remaining laps, remaining amount of fuel, estimated fuel used per lap, estimated _additional_ fuel required to finish the race, and the fuel amount that is scheduled to be added on the next pit stop. To compute the estimated fuel consumption, the last 4 laps under green and without pit stops are taken into account, and a 5% safety margin is added. These parameters can be customized.

![ddu](https://github.com/lespalt/iRon/blob/main/ddu.png?raw=true)

### *Inputs*
Shows throttle/brake/steering in a moving graph. I find it useful to practice consistent braking.

![inputs](https://github.com/lespalt/iRon/blob/main/inputs.png?raw=true)

### *Standings*
Shows the standings of the entire field, including safety rating, iRating, number of laps since the last pit stop ("pit age"), **real-time Expected iRating changes**, and **Car Model**. I usually leave this off by default and switch it on during cautions. Or glimpse at it pre-race to get a sense of the competition level.

![standings](https://github.com/lespalt/iRon/blob/main/standings.png?raw=true)

### *Cover*
No screenshot for this one, because all it is is a blank rectangle. Can be useful to cover up distracting in-game dashboards, like the one in the next-gen NASCAR.

### *Spotter (NEW)*
A visual spotter that draws transparent colored rectangles on the left and right sides of your screen when a car is in your blind spot. This directly taps into iRacing's built-in spotter telemetry.

![spotter](screenshot_placeholder_spotter.png)

### *Radar (NEW)*
A proximity radar rendering a top-down view of cars around you. Your car is positioned at the center (green). Cars ahead or behind are drawn as red rectangles based on their physical distance.

![radar](screenshot_placeholder_radar.png)

### *Incident Warning (NEW)*
A smart incident warning system. Instead of just showing a generic yellow flag banner, this overlay calculates if a car ahead of you has gone off-track during a caution. If a crashed or off-track car is detected within 150 meters ahead, it renders a prominent blinking warning banner specifying the distance to the crash.

![incident](screenshot_placeholder_incident.png)

### *Multiclass Traffic (NEW)*
A dynamic blue-flag warning system for multiclass racing. It compares the estimated lap times of surrounding cars to yours. If a faster class car approaches within a critical time gap from behind, it triggers a progressive glow UI that flashes intensely blue/white to warn you without blocking your vision.

![traffic](screenshot_placeholder_traffic.png)

### *Flat Map (NEW)*
A linear track map (ribbon map) that visualizes the entire track horizontally. It provides a global overview of traffic density, crucial for multiclass and endurance racing. Your car is a tall green marker; other cars are colored based on their lap delta (lap ahead, same lap, lap behind).

![flatmap](screenshot_placeholder_flatmap.png)

### *Delta Bar (NEW)*
A real-time horizontal gauge comparing your current lap to the session's optimal or best lap. The bar fills green to the left if you are faster, or red to the right if you are slower, accompanied by a precise numerical delta.

![deltabar](screenshot_placeholder_deltabar.png)

### *Pit Helper (NEW)*
A dedicated overlay that only appears when entering the pit lane. It displays your current speed and the track's pit speed limit, flashing a red warning if your speed limiter is disabled. Once stopped in the pit stall, it automatically transitions to a countdown timer for mandatory repairs.

![pithelper](screenshot_placeholder_pithelper.png)

---

## Installing & Running

The app does not require installation. Just copy the executable to a folder of your choice.

To use it, simply run the executable. It doesn't matter whether you do this before or after launching iRacing. A console window will pop up, indicating that iRon is running. Once you're in the car in iRacing, the overlays should show up, and you can configure things to your liking. I recommend running iRacing in borderless window mode. Overlays *might* work in other modes as well, but I haven't tested it.

---

## Configuration

To place and resize the overlays, press **ALT-J**. This will enter a mode in which you can move overlays around with the mouse and resize them by dragging their bottom-right corner. Press **ALT-J** again to go back to normal mode.

Overlays can be switched on and off at runtime using the hotkeys displayed during startup. All hotkeys are configurable.

Your layout and settings are automatically and globally saved to:
`Documents\iRon_Advanced\config.json`

Certain aspects of the overlays, such as colors, font types, sizes etc. can be customized. To do that, open `config.json` and experiment by editing the parameters. You can do that while the app is running -- the changes will take effect immediately whenever the file is saved.

---

## Building from source

This app is built with Visual Studio 2022. The free version should suffice, though I haven't verified it. The project/solution files should work out of the box. Depending on your Visual Studio setup, you may need to install additional prerequisites (static libs) needed to build DirectX applications.

---

## Dependencies

There are no runtime dependencies other than standard Windows components like DirectX. Those should already be present on most if not all systems that can run iRacing. Build dependencies (most notably the iRacing SDK and picojson) are kept to a minimum and are included in the repository.