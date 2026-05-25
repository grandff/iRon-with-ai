#!/bin/bash

# 가장 최근의 태그(버전)를 가져옴 (태그가 하나도 없으면 v1.0.0으로 초기화)
LATEST_TAG=$(git describe --tags --abbrev=0 2>/dev/null)

if [ -z "$LATEST_TAG" ]; then
    LATEST_TAG="v1.0.0"
    echo "아직 생성된 태그가 없습니다. 기준 버전: $LATEST_TAG"
else
    echo "현재 가장 최신 릴리즈 버전: $LATEST_TAG"
fi

# 버전 파싱 (v1.0.0 -> major=1, minor=0, patch=0)
# v를 제거하고 .을 기준으로 분리
VERSION_WITHOUT_V="${LATEST_TAG#v}"
IFS='.' read -ra VERSION_PARTS <<< "$VERSION_WITHOUT_V"

# 버전 숫자가 3자리가 아닐 경우를 대비한 안전 장치
if [ ${#VERSION_PARTS[@]} -eq 3 ]; then
    MAJOR=${VERSION_PARTS[0]}
    MINOR=${VERSION_PARTS[1]}
    PATCH=${VERSION_PARTS[2]}
else
    MAJOR=1
    MINOR=0
    PATCH=0
fi

# 자동으로 Patch 버전을 1 올린 버전 계산
NEXT_PATCH=$((PATCH + 1))
AUTO_NEXT_TAG="v${MAJOR}.${MINOR}.${NEXT_PATCH}"

echo ""
echo "어떤 버전을 릴리즈하시겠습니까?"
echo "1) 자동으로 패치 버전 올리기 ($AUTO_NEXT_TAG)"
echo "2) 직접 버전 이름 입력하기 (예: v2.0.0)"
echo "3) 취소"
read -p "번호를 선택하세요 (1/2/3): " CHOICE

if [ "$CHOICE" == "1" ]; then
    NEW_TAG=$AUTO_NEXT_TAG
elif [ "$CHOICE" == "2" ]; then
    read -p "새로운 태그 이름을 입력하세요 (반드시 'v'로 시작해야 합니다, 예: v1.1.0): " CUSTOM_TAG
    
    # 'v'로 시작하는지 검사
    if [[ $CUSTOM_TAG != v* ]]; then
        echo "오류: 태그는 반드시 'v'로 시작해야 합니다. 취소합니다."
        exit 1
    fi
    NEW_TAG=$CUSTOM_TAG
elif [ "$CHOICE" == "3" ]; then
    echo "릴리즈를 취소했습니다."
    exit 0
else
    echo "잘못된 입력입니다. 취소합니다."
    exit 1
fi

echo ""
echo "🚀 새로운 릴리즈 태그($NEW_TAG)를 생성하고 원격 저장소에 푸시합니다..."

# 메인 브랜치의 최신 커밋들 먼저 푸시
echo "[1/3] 메인 브랜치 코드 푸시 중..."
git push origin main

# 로컬에 새 태그 생성 (기존 태그 강제 덮어쓰기 허용)
echo "[2/3] 로컬 태그($NEW_TAG) 생성 중..."
git tag -f $NEW_TAG

# 생성된 태그를 원격 저장소(GitHub)로 강제 푸시 (기존 원격 태그 덮어쓰기 및 Actions 트리거)
echo "[3/3] 태그 푸시 및 GitHub Actions 트리거 가동..."
git push -f origin $NEW_TAG

echo ""
echo "✅ 완료되었습니다! GitHub Actions에서 자동으로 빌드 및 릴리즈를 진행하고 있습니다."
echo "웹 브라우저에서 레포지토리의 [Actions] 탭을 확인해 보세요."
