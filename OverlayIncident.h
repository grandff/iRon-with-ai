#pragma once

#include "Overlay.h"
#include "iracing.h"
#include <cmath>
#include <string>

class OverlayIncident : public Overlay
{
public:

    OverlayIncident()
        : Overlay("OverlayIncident")
    {
    }

protected:

    virtual float2 getDefaultSize() override
    {
        // 넓고 눈에 띄는 배너 형태
        return float2{ 800, 100 };
    }

    virtual void onConfigChanged() override
    {
        m_text.reset(m_dwriteFactory.Get());

        const std::string font = g_cfg.getString(m_name, "font", "Microsoft YaHei UI");
        const float fontSize = g_cfg.getFloat(m_name, "font_size", 36.0f);
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
        std::wstring warningText = L"";
        float minIncidentDistMeters = 99999.0f;

        // UI Edit Mode
        if (m_uiEditEnabled) {
            showWarning = true;
            warningText = L"⚠️ CRASH 150m AHEAD ⚠️";
        } 
        else if (ir_session.sessionType == SessionType::RACE) // 주로 레이스 세션에서만 활성화
        {
            const float myDistPct = ir_CarIdxLapDistPct.getFloat(ir_session.driverCarIdx);
            const int flags = ir_SessionFlags.getInt();
            const bool isGlobalYellow = (flags & (irsdk_yellow | irsdk_yellowWaving | irsdk_caution | irsdk_cautionWaving));

            // 전방 차량 탐색
            for (int i = 0; i < IR_MAX_CARS; ++i)
            {
                if (i == ir_session.driverCarIdx) continue;
                
                const Car& car = ir_session.cars[i];
                if (car.isSpectator || car.isPaceCar) continue;
                
                int surface = ir_CarIdxTrackSurface.getInt(i);
                if (surface == irsdk_NotInWorld) continue;

                float otherDistPct = ir_CarIdxLapDistPct.getFloat(i);
                float deltaPct = otherDistPct - myDistPct;
                
                // Wrap-around 처리
                if (deltaPct > 0.5f) deltaPct -= 1.0f;
                if (deltaPct < -0.5f) deltaPct += 1.0f;

                // 내 앞쪽인지 확인 (예: 최대 15% 트랙 앞쪽까지만, 대략 10~15초 이내)
                if (deltaPct > 0.0f && deltaPct < 0.15f)
                {
                    // 1. 차가 트랙 밖으로 나갔거나 (OffTrack)
                    // 2. 전체 황기 상태인데 이 차가 사고에 연루된 것으로 추정될 때 (속도 저하 등 - 여기서는 OffTrack을 주요 트리거로 사용)
                    if (surface == irsdk_OffTrack)
                    {
                        float distMeters = deltaPct * ir_session.trackLength;
                        if (distMeters < minIncidentDistMeters) {
                            minIncidentDistMeters = distMeters;
                        }
                    }
                }
            }

            if (minIncidentDistMeters < 99999.0f) {
                showWarning = true;
                wchar_t buf[128];
                swprintf(buf, _countof(buf), L"⚠️ CRASH %.0fm AHEAD ⚠️", minIncidentDistMeters);
                warningText = buf;
            }
        }

        if (showWarning)
        {
            // 점멸 효과 (Blink) - 0.5초 주기
            static DWORD startTick = GetTickCount();
            DWORD currentTick = GetTickCount();
            bool isBlinkOn = ((currentTick - startTick) % 500) < 250;

            if (isBlinkOn || m_uiEditEnabled)
            {
                // 배경: 반투명 빨간색
                Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
                m_d2dFactory->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.1f, 0.1f, 0.85f), &bgBrush);
                
                D2D1_ROUNDED_RECT bgRect = D2D1::RoundedRect(
                    D2D1::RectF(0, 0, (float)m_width, (float)m_height), 10.0f, 10.0f
                );
                m_renderTarget->FillRoundedRectangle(&bgRect, bgBrush.Get());

                // 테두리: 노란색
                Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> borderBrush;
                m_d2dFactory->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.9f, 0.0f, 1.0f), &borderBrush);
                m_renderTarget->DrawRoundedRectangle(&bgRect, borderBrush.Get(), 4.0f);

                // 텍스트: 하얀색
                Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textBrush;
                m_d2dFactory->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &textBrush);

                m_text.render(m_renderTarget.Get(), warningText.c_str(), m_textFormat.Get(), 0, (float)m_width, (float)m_height / 2.0f, textBrush.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
            }
        }
    }

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    TextCache m_text;
};
