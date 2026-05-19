#pragma once

#include "Overlay.h"
#include "iracing.h"
#include <cmath>

class OverlayFlatMap : public Overlay
{
public:

    OverlayFlatMap()
        : Overlay("OverlayFlatMap")
    {
    }

protected:

    virtual float2 getDefaultSize() override
    {
        // 넓은 가로형 리본 맵 (가로 1000, 세로 60)
        return float2{ 1000, 60 };
    }

    virtual void onUpdate() override
    {
        // 1. 전체 트랙 배경 그리기 (반투명 어두운 바)
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.5f), &bgBrush);
        D2D1_RECT_F bgRect = D2D1::RectF(0, 0, (float)m_width, (float)m_height);
        m_renderTarget->FillRectangle(&bgRect, bgBrush.Get());

        // 트랙 가이드라인 (중앙에 얇은 선 하나 그리기)
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> lineBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.2f), &lineBrush);
        D2D1_RECT_F lineRect = D2D1::RectF(0, (float)m_height / 2.0f - 1.0f, (float)m_width, (float)m_height / 2.0f + 1.0f);
        m_renderTarget->FillRectangle(&lineRect, lineBrush.Get());

        if (ir_session.driverCarIdx < 0) return;

        // UI 에디트 모드일 때 가상의 트래픽 생성
        if (m_uiEditEnabled) {
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> playerBrush;
            m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.9f, 0.0f, 1.0f), &playerBrush);
            D2D1_RECT_F pRect = D2D1::RectF(m_width * 0.5f - 4.0f, 10.0f, m_width * 0.5f + 4.0f, m_height - 10.0f);
            m_renderTarget->FillRectangle(&pRect, playerBrush.Get());

            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> otherBrush;
            m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.8f), &otherBrush);
            D2D1_RECT_F oRect = D2D1::RectF(m_width * 0.6f - 3.0f, 20.0f, m_width * 0.6f + 3.0f, m_height - 20.0f);
            m_renderTarget->FillRectangle(&oRect, otherBrush.Get());
            return;
        }

        const float myDistPct = ir_CarIdxLapDistPct.getFloat(ir_session.driverCarIdx);
        const int myLap = ir_CarIdxLap.getInt(ir_session.driverCarIdx);

        // 맵 모드: 절대 모드(0=Start, 1=End) vs 상대 모드(내 차가 항상 중앙)
        const bool isRelativeMode = g_cfg.getBool(m_name, "is_relative_mode", true);

        // 브러시 준비
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> playerBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.9f, 0.0f, 1.0f), &playerBrush); // 내 차: 녹색
        
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> sameLapBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.9f), &sameLapBrush); // 동일 랩: 흰색
        
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> lapAheadBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.2f, 0.2f, 0.9f), &lapAheadBrush); // 앞 랩(Lapper): 빨강
        
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> lapBehindBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.6f, 1.0f, 0.9f), &lapBehindBrush); // 뒤 랩(백마커): 파랑

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pitBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.8f, 0.1f, 0.6f), &pitBrush); // 피트: 노랑 투명

        // 2. 다른 차량들 그리기
        for (int i = 0; i < IR_MAX_CARS; ++i)
        {
            if (i == ir_session.driverCarIdx) continue;
            
            const Car& car = ir_session.cars[i];
            if (car.isSpectator || car.carNumber < 0 || ir_CarIdxTrackSurface.getInt(i) == irsdk_NotInWorld) continue;

            float otherDistPct = ir_CarIdxLapDistPct.getFloat(i);
            const int otherLap = ir_CarIdxLap.getInt(i);
            const int lapDelta = otherLap - myLap;

            float drawX = 0;
            if (isRelativeMode) {
                // 상대 모드: 내 차를 0.5(중앙)에 둠
                float deltaPct = otherDistPct - myDistPct;
                if (deltaPct > 0.5f) deltaPct -= 1.0f;
                if (deltaPct < -0.5f) deltaPct += 1.0f;
                drawX = (0.5f + deltaPct) * m_width;
            } else {
                // 절대 모드: 0부터 1까지 일직선
                drawX = otherDistPct * m_width;
            }

            ID2D1SolidColorBrush* currentBrush = sameLapBrush.Get();
            if (lapDelta > 0) currentBrush = lapAheadBrush.Get();
            else if (lapDelta < 0) currentBrush = lapBehindBrush.Get();

            if (ir_CarIdxOnPitRoad.getBool(i)) {
                currentBrush = pitBrush.Get();
            }

            // 트래픽 마커 그리기 (높이는 중앙 기준 위아래)
            const float markerWidth = 3.0f;
            const float markerHeight = 15.0f;
            D2D1_RECT_F carRect = D2D1::RectF(drawX - markerWidth, (m_height / 2.0f) - markerHeight, drawX + markerWidth, (m_height / 2.0f) + markerHeight);
            m_renderTarget->FillRectangle(&carRect, currentBrush);
        }

        // 3. 내 차량 그리기 (가장 나중에 그려서 맨 위에 덮도록)
        float myDrawX = 0;
        if (isRelativeMode) {
            myDrawX = m_width * 0.5f;
        } else {
            myDrawX = myDistPct * m_width;
        }

        const float myMarkerWidth = 4.0f;
        const float myMarkerHeight = 25.0f; // 다른 차들보다 위아래로 더 길게
        D2D1_RECT_F myCarRect = D2D1::RectF(myDrawX - myMarkerWidth, (m_height / 2.0f) - myMarkerHeight, myDrawX + myMarkerWidth, (m_height / 2.0f) + myMarkerHeight);
        m_renderTarget->FillRectangle(&myCarRect, playerBrush.Get());
    }
};