#pragma once

#include "Overlay.h"
#include "iracing.h"
#include <cmath>
#include <string>

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
        return float2{ 600, 100 };
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
        float minDelta = -999.0f; 

        if (m_uiEditEnabled) {
            showWarning = true;
            warningText = L"🟦 FASTER CLASS: 1.5s 🟦";
        } 
        else
        {
            // Do not warn if player is in pits
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

                // Check if they are a faster class
                // (at least 1 second faster estimated lap time)
                if (car.carClassEstLapTime <= 0.0f || myCar.carClassEstLapTime <= 0.0f) continue;
                if (car.carClassEstLapTime >= myCar.carClassEstLapTime - 1.0f) continue;

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

                // If behind by 0.5 to 3.5 seconds
                if (delta < -0.5f && delta > -3.5f)
                {
                    if (delta > minDelta) { 
                        minDelta = delta;
                    }
                }
            }

            if (minDelta > -999.0f) {
                showWarning = true;
                wchar_t buf[128];
                swprintf(buf, _countof(buf), L"🟦 FASTER CLASS: %.1fs 🟦", fabsf(minDelta));
                warningText = buf;
            }
        }

        if (showWarning)
        {
            // Blink effect: 0.5s period, 70% ON
            static DWORD startTick = GetTickCount();
            DWORD currentTick = GetTickCount();
            bool isBlinkOn = ((currentTick - startTick) % 500) < 350;

            if (isBlinkOn || m_uiEditEnabled)
            {
                // Background: Blue
                Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
                m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.4f, 0.9f, 0.85f), &bgBrush);
                
                D2D1_ROUNDED_RECT bgRect = D2D1::RoundedRect(
                    D2D1::RectF(0, 0, (float)m_width, (float)m_height), 10.0f, 10.0f
                );
                m_renderTarget->FillRoundedRectangle(&bgRect, bgBrush.Get());

                // Border: White
                Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> borderBrush;
                m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &borderBrush);
                m_renderTarget->DrawRoundedRectangle(&bgRect, borderBrush.Get(), 4.0f);

                // Text: White
                Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textBrush;
                m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &textBrush);

                m_text.render(m_renderTarget.Get(), warningText.c_str(), m_textFormat.Get(), 0, (float)m_width, (float)m_height / 2.0f, textBrush.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
            }
        }
    }

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    TextCache m_text;
};
