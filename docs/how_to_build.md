  🚀 Windows 빌드 및 릴리즈 가이드 (퇴근 후 집에서 하실 작업)

  이 프로젝트(iRon-with-ai)는 C++과 DirectX를 사용하므로 맥이 아닌 윈도우 환경(집
  컴퓨터)에서 빌드하셔야 합니다. 아래 절차를 그대로 따라 해주세요.

  1단계: 개발 환경 준비 (집 PC)
   1. Visual Studio 2022 설치: (무료 Community 버전이면 충분합니다)
   2. 설치 시 [C++를 사용한 데스크톱 개발] 워크로드를 반드시 체크하여 설치하세요. (여기에
      Windows SDK와 DirectX 컴파일 도구가 포함되어 있습니다.)

  2단계: 프로젝트 빌드 (컴파일)
   1. 집 컴퓨터에 Git Clone으로 최신 iRon-with-ai 코드를 받아옵니다.
   2. 프로젝트 폴더 안에 있는 iron.sln 파일을 더블클릭하여 Visual Studio 2022로 엽니다.
   3. 화면 상단(툴바)의 빌드 구성 드롭다운에서 Debug를 Release로 변경합니다. (최적화 및
      배포용)
   4. 플랫폼 드롭다운이 x64로 되어 있는지 확인합니다.
   5. 상단 메뉴에서 [빌드(Build)] -> [솔루션 빌드(Build Solution)] 또는 단축키
      Ctrl+Shift+B를 누릅니다.
   6. 빌드가 성공하면(하단 출력창에 '성공 1' 표시), 프로젝트 폴더 안의 x64/Release/iRon.exe
      파일이 생성됩니다. 이 파일이 바로 실행 가능한 완성된 오버레이 프로그램입니다!

  3단계: GitHub Release 배포 (선택)
  만든 프로그램을 다른 사람과 공유하거나 백업하고 싶다면 릴리즈로 등록하세요.
   1. GitHub의 iRon-with-ai 저장소(Repository) 페이지로 이동합니다.
   2. 우측의 [Releases] 섹션에서 Create a new release를 클릭합니다.
   3. Choose a tag에 v2.0.0-Advanced 같은 태그 이름을 입력합니다.
   4. 릴리즈 제목(Release title)과 설명(README.md에 쓴 내용 요약)을 적습니다.
   5. 가장 중요한 [Attach binaries by dropping them here] 박스에 아까 만든 iRon.exe 파일을
      드래그 앤 드롭해서 업로드합니다.
   6. Publish release를 누르면 끝입니다! 이제 누구나 설치 없이 iRon.exe만 다운받아 실행하면
      레이더와 일자형 맵을 쓸 수 있습니다.

  집에 가셔서 빌드 해보시고 iRacing에서 직접 테스트해 보세요. 수정이 필요하거나 내일 이어서
  작업할 내용이 있다면 언제든 다시 불러주세요! 조심히 들어가세요!
