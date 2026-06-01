#pragma once

#include "Overlay.h"
#include "iracing.h"
#include <cmath>
#include <algorithm>
#include <string>

class OverlaySpotter : public Overlay
{
public:
    OverlaySpotter(bool isLeft)
        : Overlay(isLeft ? "OverlaySpotterLeft" : "OverlaySpotterRight")
        , m_isLeft(isLeft)
    {
    }

protected:
    bool m_isLeft;

    virtual float2 getDefaultSize() override
    {
        // Independent vertical spotter bar defaults
        return float2{ 80, 400 };
    }

    virtual void onUpdate() override
    {
        const int spotterState = ir_CarLeftRight.getInt();
        
        // 0: Off, 1: Clear, 2: Left, 3: Right, 4: Left+Right, 5: 2 Left, 6: 2 Right
        bool hasCar = false;
        if (m_isLeft) {
            hasCar = (spotterState == irsdk_LRCarLeft || spotterState == irsdk_LRCarLeftRight || spotterState == irsdk_LR2CarsLeft);
        } else {
            hasCar = (spotterState == irsdk_LRCarRight || spotterState == irsdk_LRCarLeftRight || spotterState == irsdk_LR2CarsRight);
        }

        // Active state if there is a car or if the user is in layout edit mode
        bool showAsActive = hasCar || m_uiEditEnabled;

        if (!showAsActive) {
            // Keep completely transparent when not in use
            return;
        }

        // Under UI layout edit mode, draw a semi-transparent black backing so the user can easily see/drag the container
        if (m_uiEditEnabled) {
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
            m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.3f), &bgBrush);
            D2D1_RECT_F bgRect = D2D1::RectF(0, 0, (float)m_width, (float)m_height);
            m_renderTarget->FillRectangle(&bgRect, bgBrush.Get());
        }

        // Default to warning yellow color
        float4 activeCol = float4(0.95f, 0.8f, 0.0f, 0.8f);

        // Calculate actual proximity distance from nearby cars to adjust alert severity color (Red/Yellow)
        if (hasCar && ir_session.driverCarIdx >= 0 && ir_session.trackLength > 0) {
            float myDistPct = ir_CarIdxLapDistPct.getFloat(ir_session.driverCarIdx);
            float minAbsDist = 9999.0f;

            for (int i = 0; i < IR_MAX_CARS; ++i) {
                if (i == ir_session.driverCarIdx) continue;
                
                const Car& car = ir_session.cars[i];
                if (car.isSpectator || ir_CarIdxTrackSurface.getInt(i) == irsdk_NotInWorld) continue;

                float otherDistPct = ir_CarIdxLapDistPct.getFloat(i);
                float deltaPct = otherDistPct - myDistPct;
                if (deltaPct > 0.5f) deltaPct -= 1.0f;
                if (deltaPct < -0.5f) deltaPct += 1.0f;

                float distMeters = deltaPct * ir_session.trackLength;
                float absDist = std::abs(distMeters);

                // Filter for cars practically alongside us
                if (absDist < 5.0f) {
                    minAbsDist = std::min(minAbsDist, absDist);
                }
            }

            // Extreme proximity threshold: <= 2.2 meters -> Red alert. Otherwise -> Yellow caution.
            if (minAbsDist <= 2.2f) {
                activeCol = float4(0.9f, 0.1f, 0.1f, 0.85f); // Red
            } else {
                activeCol = float4(0.95f, 0.8f, 0.0f, 0.8f); // Yellow
            }
        } else if (m_uiEditEnabled) {
            // Edit guide color (semi-transparent yellow)
            activeCol = float4(0.95f, 0.8f, 0.0f, 0.4f);
        }

        // Draw the full-width alert panel
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> fillBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(activeCol.x, activeCol.y, activeCol.z, activeCol.w), &fillBrush);
        D2D1_RECT_F spotterRect = D2D1::RectF(0.0f, 0.0f, (float)m_width, (float)m_height);
        m_renderTarget->FillRectangle(&spotterRect, fillBrush.Get());

        // Draw text label only during UI layout editing to guide configuration
        if (m_uiEditEnabled) {
            Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
            m_dwriteFactory->CreateTextFormat(L"Microsoft YaHei UI", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"en-us", &textFormat);
            textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textBrush;
            m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &textBrush);

            std::wstring label = m_isLeft ? L"SPOTTER L" : L"SPOTTER R";
            TextCache textCache;
            textCache.render(m_renderTarget.Get(), label.c_str(), textFormat.Get(), 0.0f, (float)m_width, (float)m_height / 2.0f, textBrush.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
        }
    }
};
