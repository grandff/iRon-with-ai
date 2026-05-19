#pragma once

#include "Overlay.h"
#include "iracing.h"
#include <cmath>
#include <string>
#include <algorithm>

class OverlayTraffic : public Overlay
{
public:

    OverlayTraffic()
        : Overlay("OverlayTraffic")
    {
    }

protected:

    virtual float2 getDefaultSize() override
    {
        // 와이드한 배너 형태로 변경 (화면 가림 최소화)
        return float2{ 800, 80 };
    }

    virtual void onConfigChanged() override
    {
        m_text.reset(m_dwriteFactory.Get());

        const std::string font = g_cfg.getString(m_name, "font", "Microsoft YaHei UI");
        const float fontSize = g_cfg.getFloat(m_name, "font_size", 28.0f);
        const int fontWeight = g_cfg.getInt(m_name, "font_weight", 800); // Bold

        HRCHECK(m_dwriteFactory->CreateTextFormat(toWide(font).c_str(), NULL, (DWRITE_FONT_WEIGHT)fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &m_textFormat));
        m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        m_textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    virtual void onUpdate() override
    {
        if (ir_session.driverCarIdx < 0 || ir_session.trackLength <= 0) return;

        bool showWarning = false;
        float minDelta = -999.0f;
        float associatedLapTimeDiff = 0.0f;

        if (m_uiEditEnabled) {
            showWarning = true;
            minDelta = -1.5f; // 긴급 상황(Urgent) 테스트용
            associatedLapTimeDiff = 5.0f;
        } 
        else
        {
            if (ir_OnPitRoad.getBool() || ir_PlayerCarInPitStall.getBool()) return;

            const float L = ir_estimateLaptime();
            const float S = ir_CarIdxEstTime.getFloat(ir_session.driverCarIdx);
            const float myDistPct = ir_CarIdxLapDistPct.getFloat(ir_session.driverCarIdx);
            const Car& myCar = ir_session.cars[ir_session.driverCarIdx];

            for (int i = 0; i < IR_MAX_CARS; ++i)
            {
                if (i == ir_session.driverCarIdx) continue;
                
                const Car& car = ir_session.cars[i];
                if (car.isSpectator || car.isPaceCar || car.carNumber < 0) continue;
                
                int surface = ir_CarIdxTrackSurface.getInt(i);
                if (surface == irsdk_NotInWorld) continue;

                if (car.carClassEstLapTime <= 0.0f || myCar.carClassEstLapTime <= 0.0f) continue;
                
                // 1. 개선안: 클래스별 속도 차이 계산
                float lapTimeDiff = myCar.carClassEstLapTime - car.carClassEstLapTime;
                if (lapTimeDiff < 1.0f) continue; // 나보다 1초 이상 빠른 클래스만 대상

                // 랩타임 차이에 따른 동적 경고 시간 계산 (최소 3.0초, 최대 6.0초)
                // 예: 랩타임 차이가 10초면 3.0 + 2.0 = 5.0초 전부터 경고
                float warnLeadTime = std::min(6.0f, 3.0f + (lapTimeDiff * 0.2f));

                const float C = ir_CarIdxEstTime.getFloat(i);
                const float otherDistPct = ir_CarIdxLapDistPct.getFloat(i);

                float delta = 0;
                const bool wrap = fabsf(otherDistPct - myDistPct) > 0.5f;

                if (wrap)
                {
                    delta = S > C ? (C-S)+L : (C-S)-L;
                }
                else
                {
                    delta = C - S;
                }

                // 스포터(Spotter) 인계 거리(0.5초) 이전이며, 경고 시간 범위 내인 경우
                if (delta < -0.5f && delta > -warnLeadTime)
                {
                    if (delta > minDelta) { 
                        minDelta = delta;
                        associatedLapTimeDiff = lapTimeDiff;
                    }
                }
            }

            if (minDelta > -999.0f) {
                showWarning = true;
            }
        }

        if (showWarning)
        {
            float absDelta = fabsf(minDelta);
            
            // 2. 개선안: 거리에 따른 점진적 변화 (Progressive Warning)
            bool isUrgent = (absDelta <= 2.0f);

            D2D1_COLOR_F baseColor;
            D2D1_COLOR_F textColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f); // 기본 흰색 텍스트

            if (isUrgent) {
                // 2.0초 이내: 긴급 모드 (빠른 점멸)
                static DWORD startTick = GetTickCount();
                DWORD currentTick = GetTickCount();
                bool isBlinkOn = ((currentTick - startTick) % 250) < 125; // 0.25초 주기 매우 빠른 점멸

                if (isBlinkOn) {
                    baseColor = D2D1::ColorF(0.0f, 0.4f, 1.0f, 0.95f); // 강렬한 파란색
                } else {
                    baseColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.95f); // 하얀색 번쩍임
                    textColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f); // 바탕이 하얄 땐 검은색 텍스트
                }
            } else {
                // 2.0초 초과: 인지 모드 (은은한 하늘색 유지, 점멸 없음)
                baseColor = D2D1::ColorF(0.2f, 0.6f, 1.0f, 0.7f);
            }

            // 3. 개선안: 화면 가림 최소화 (은은한 글로우 텍스트/테두리 UI)
            D2D1_RECT_F rect = D2D1::RectF(0, 0, (float)m_width, (float)m_height);
            
            // 중앙 배경은 시야 확보를 위해 투명도를 대폭 낮춤
            D2D1_COLOR_F bgColor = baseColor;
            bgColor.a = isUrgent ? 0.4f : 0.15f; 
            
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
            m_renderTarget->CreateSolidColorBrush(bgColor, &bgBrush);
            m_renderTarget->FillRectangle(&rect, bgBrush.Get());

            // 위아래 테두리에 글로우(Glow) 효과를 위해 점점 얇고 진해지는 선을 3겹으로 그림
            for (int i = 0; i < 3; ++i) {
                float opacity = (3.0f - i) / 3.0f;
                float thickness = (i + 1) * 2.0f;
                
                D2D1_COLOR_F glowColor = baseColor;
                glowColor.a = baseColor.a * opacity;
                
                Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> glowBrush;
                m_renderTarget->CreateSolidColorBrush(glowColor, &glowBrush);
                
                // Top border
                m_renderTarget->DrawLine(D2D1::Point2F(0, 0), D2D1::Point2F((float)m_width, 0), glowBrush.Get(), thickness);
                // Bottom border
                m_renderTarget->DrawLine(D2D1::Point2F(0, (float)m_height), D2D1::Point2F((float)m_width, (float)m_height), glowBrush.Get(), thickness);
            }

            // 텍스트 그리기
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textBrush;
            m_renderTarget->CreateSolidColorBrush(textColor, &textBrush);

            wchar_t buf[128];
            if (isUrgent) {
                swprintf(buf, _countof(buf), L">> FASTER CLASS: %.1fs <<", absDelta);
            } else {
                swprintf(buf, _countof(buf), L"Faster Class Approaching (%.1fs)", absDelta);
            }

            m_text.render(m_renderTarget.Get(), buf, m_textFormat.Get(), 0, (float)m_width, (float)m_height / 2.0f, textBrush.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
        }
    }

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    TextCache m_text;
};