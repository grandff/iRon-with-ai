#pragma once

#include "Overlay.h"
#include "iracing.h"
#include <cmath>

class OverlayRadar : public Overlay
{
public:

    OverlayRadar()
        : Overlay("OverlayRadar")
    {
    }

protected:

    virtual float2 getDefaultSize() override
    {
        return float2{ 200, 400 };
    }

    virtual void onUpdate() override
    {
        // 배경 반투명 처리
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.4f), &bgBrush);
        D2D1_RECT_F bgRect = D2D1::RectF(0, 0, (float)m_width, (float)m_height);
        m_renderTarget->FillRectangle(&bgRect, bgBrush.Get());

        float centerX = m_width / 2.0f;
        float centerY = m_height / 2.0f;

        // 레이더 스케일: 1미터 = 10픽셀
        const float scale = 10.0f;
        const float maxDistMeters = (m_height / 2.0f) / scale; // 약 20미터

        // 플레이어 차량 그리기 (가운데 고정)
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> playerBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.8f, 0.0f, 1.0f), &playerBrush);
        D2D1_ROUNDED_RECT playerRect = D2D1::RoundedRect(
            D2D1::RectF(centerX - 10, centerY - 20, centerX + 10, centerY + 20), 4.0f, 4.0f
        );
        m_renderTarget->FillRoundedRectangle(&playerRect, playerBrush.Get());

        if (ir_session.driverCarIdx < 0 || ir_session.trackLength <= 0) return;

        const float myDistPct = ir_CarIdxLapDistPct.getFloat(ir_session.driverCarIdx);
        const int spotterState = ir_CarLeftRight.getInt();

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> otherCarBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.8f, 0.1f, 0.1f, 1.0f), &otherCarBrush);

        // UI 에디트 모드 시 가상의 차량 렌더링
        if (m_uiEditEnabled) {
            D2D1_ROUNDED_RECT dummy1 = D2D1::RoundedRect(
                D2D1::RectF(centerX - 35, centerY + 10, centerX - 15, centerY + 50), 4.0f, 4.0f
            );
            m_renderTarget->FillRoundedRectangle(&dummy1, otherCarBrush.Get());
            
            D2D1_ROUNDED_RECT dummy2 = D2D1::RoundedRect(
                D2D1::RectF(centerX - 10, centerY - 80, centerX + 10, centerY - 40), 4.0f, 4.0f
            );
            m_renderTarget->FillRoundedRectangle(&dummy2, otherCarBrush.Get());
            return;
        }

        // 주변 차량 탐색 및 렌더링
        for (int i = 0; i < IR_MAX_CARS; ++i)
        {
            if (i == ir_session.driverCarIdx) continue;
            
            const Car& car = ir_session.cars[i];
            if (car.isSpectator || ir_CarIdxTrackSurface.getInt(i) == irsdk_NotInWorld) continue;

            float otherDistPct = ir_CarIdxLapDistPct.getFloat(i);
            
            // 델타 거리(비율) 계산 및 랩 넘어감(Wrap-around) 처리
            float deltaPct = otherDistPct - myDistPct;
            if (deltaPct > 0.5f) deltaPct -= 1.0f;
            if (deltaPct < -0.5f) deltaPct += 1.0f;

            // 실제 물리적 거리(미터) 계산
            float distMeters = deltaPct * ir_session.trackLength;

            // 반경(maxDistMeters) 이내의 차량만 렌더링
            if (std::abs(distMeters) < maxDistMeters)
            {
                // 가상의 가로(Lateral) 좌표 추정
                // 기본적으로 앞뒤로만 존재한다고 가정
                float lateralDistMeters = 0.0f;

                // 차량이 내 차와 종방향으로 아주 가까울 때 (옆에 있을 때)
                if (std::abs(distMeters) < 4.5f) {
                    bool left = (spotterState == irsdk_LRCarLeft || spotterState == irsdk_LRCarLeftRight || spotterState == irsdk_LR2CarsLeft);
                    bool right = (spotterState == irsdk_LRCarRight || spotterState == irsdk_LRCarLeftRight || spotterState == irsdk_LR2CarsRight);
                    
                    if (left && !right) lateralDistMeters = -2.5f;
                    else if (right && !left) lateralDistMeters = 2.5f;
                    else if (left && right) {
                        // 양쪽에 있을 경우 단순화하여 거리가 약간 앞이면 오/뒤면 왼 등 임의 배치하거나
                        // 정확히 판단 불가하므로 거리에 따라 임의로 흩어놓음
                        lateralDistMeters = (distMeters > 0) ? 2.5f : -2.5f; 
                    }
                }

                float screenX = centerX + (lateralDistMeters * scale);
                float screenY = centerY - (distMeters * scale);

                D2D1_ROUNDED_RECT rect = D2D1::RoundedRect(
                    D2D1::RectF(screenX - 10, screenY - 20, screenX + 10, screenY + 20), 4.0f, 4.0f
                );
                
                m_renderTarget->FillRoundedRectangle(&rect, otherCarBrush.Get());
            }
        }
    }
};
