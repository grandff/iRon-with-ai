#pragma once

#include "Overlay.h"
#include "iracing.h"
#include <deque>
#include <algorithm>

class OverlayFuel : public Overlay
{
public:
    OverlayFuel()
        : Overlay("OverlayFuel")
    {
    }

protected:
    virtual float2 getDefaultSize() override
    {
        return float2{ 250, 140 };
    }

    virtual void onConfigChanged() override
    {
        m_text.reset(m_dwriteFactory.Get());

        const std::string font = g_cfg.getString(m_name, "font", "Microsoft YaHei UI");
        const float fontSize = g_cfg.getFloat(m_name, "font_size", 14.0f);
        const int fontWeight = g_cfg.getInt(m_name, "font_weight", 600);

        HRCHECK(m_dwriteFactory->CreateTextFormat(toWide(font).c_str(), NULL, (DWRITE_FONT_WEIGHT)fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &m_textFormat));
        HRCHECK(m_dwriteFactory->CreateTextFormat(toWide(font).c_str(), NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize + 4.0f, L"en-us", &m_textFormatBold));
        HRCHECK(m_dwriteFactory->CreateTextFormat(toWide(font).c_str(), NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize - 2.0f, L"en-us", &m_textFormatSmall));
    }

    virtual void onUpdate() override
    {
        const float w = (float)m_width;
        const float h = (float)m_height;

        // Background Glassmorphism
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.02f, 0.02f, 0.02f, 0.65f), &bgBrush);
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(0, 0, w, h), 8.0f, 8.0f);
        m_renderTarget->FillRoundedRectangle(&roundedRect, bgBrush.Get());

        // Thin Border
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> borderBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.15f), &borderBrush);
        m_renderTarget->DrawRoundedRectangle(&roundedRect, borderBrush.Get(), 1.5f);

        if (ir_session.driverCarIdx < 0 || ir_session.trackLength <= 0) return;

        // Fuel calculations
        const float remainingFuel = ir_FuelLevel.getFloat();
        const float fuelPct = ir_FuelLevelPct.getFloat();
        const bool imperial = ir_DisplayUnits.getInt() == 0;

        // Check lap increment
        static int lastLap = -1;
        int currentLap = ir_Lap.getInt();
        bool lapCountUpdated = (lastLap != -1 && currentLap > lastLap);
        lastLap = currentLap;

        if (currentLap == 0) {
            m_fuelUsedLastLaps.clear();
            m_lapStartRemainingFuel = remainingFuel;
            m_isValidFuelLap = false;
        }

        float avgPerLap4 = 0;
        float avgPerLap8 = 0;
        {
            if (lapCountUpdated) {
                const float usedLastLap = std::max(0.0f, m_lapStartRemainingFuel - remainingFuel);
                m_lapStartRemainingFuel = remainingFuel;

                if (m_isValidFuelLap && usedLastLap > 0.05f) {
                    m_fuelUsedLastLaps.push_back(usedLastLap);
                }

                while (m_fuelUsedLastLaps.size() > 8) {
                    m_fuelUsedLastLaps.pop_front();
                }
                m_isValidFuelLap = true;
            }

            int carIdx = ir_session.driverCarIdx;
            if ((ir_SessionFlags.getInt() & (irsdk_yellow|irsdk_yellowWaving|irsdk_red|irsdk_checkered|irsdk_caution|irsdk_cautionWaving|irsdk_disqualify|irsdk_repair)) || ir_CarIdxOnPitRoad.getBool(carIdx)) {
                m_isValidFuelLap = false;
            }

            // 4-lap Average
            float sum4 = 0;
            int count4 = 0;
            int startIdx = std::max(0, (int)m_fuelUsedLastLaps.size() - 4);
            for (int i = startIdx; i < (int)m_fuelUsedLastLaps.size(); ++i) {
                sum4 += m_fuelUsedLastLaps[i];
                count4++;
            }
            if (count4 > 0) avgPerLap4 = sum4 / count4;

            // 8-lap Average
            float sum8 = 0;
            int count8 = 0;
            for (float v : m_fuelUsedLastLaps) {
                sum8 += v;
                count8++;
            }
            if (count8 > 0) avgPerLap8 = sum8 / count8;
        }

        // Draw top fuel bar
        const float gap = 8.0f;
        const float barH = 10.0f;
        D2D1_RECT_F barBg = D2D1::RectF(gap, gap, w - gap, gap + barH);
        m_brush->SetColor(float4(0.1f, 0.1f, 0.1f, 0.5f));
        m_renderTarget->FillRectangle(&barBg, m_brush.Get());

        if (fuelPct > 0) {
            D2D1_RECT_F barFill = D2D1::RectF(gap, gap, gap + fuelPct * (w - gap * 2), gap + barH);
            m_brush->SetColor(fuelPct < 0.15f ? float4(0.85f, 0.05f, 0.05f, 0.85f) : float4(0.1f, 0.75f, 0.15f, 0.85f));
            m_renderTarget->FillRectangle(&barFill, m_brush.Get());
        }
        m_brush->SetColor(float4(1.0f, 1.0f, 1.0f, 0.2f));
        m_renderTarget->DrawRectangle(&barBg, m_brush.Get(), 1.0f);

        // Core Text Info
        wchar_t s[256];
        float valRemaining = remainingFuel;
        if (imperial) valRemaining *= 0.264172f;

        m_brush->SetColor(float4(1.0f, 1.0f, 1.0f, 0.85f));
        swprintf(s, _countof(s), L"Fuel: %.1f %s", valRemaining, imperial ? L"gal" : L"lit");
        m_text.render(m_renderTarget.Get(), s, m_textFormatBold.Get(), gap, w - gap, gap + barH + 6.0f, m_brush.Get(), DWRITE_TEXT_ALIGNMENT_LEADING);

        // Remaining Laps (est)
        const float estimateFactor = g_cfg.getFloat("OverlayDDU", "fuel_estimate_factor", 1.1f);
        const float perLapConsEst = (avgPerLap4 > 0 ? avgPerLap4 : (remainingFuel > 0 ? 0.5f : 0.0f)) * estimateFactor;
        
        float estLaps = 0;
        if (perLapConsEst > 0) {
            estLaps = remainingFuel / perLapConsEst;
            swprintf(s, _countof(s), L"Est Laps: %.1f", estLaps);
        } else {
            swprintf(s, _countof(s), L"Est Laps: --");
        }
        m_text.render(m_renderTarget.Get(), s, m_textFormatBold.Get(), gap, w - gap, gap + barH + 6.0f, m_brush.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

        // Sub rows (Averages & Requirements)
        float textY = gap + barH + 36.0f;
        float rowH = 18.0f;

        m_brush->SetColor(float4(0.7f, 0.7f, 0.7f, 0.8f));
        m_text.render(m_renderTarget.Get(), L"Avg Per Lap (4L / 8L):", m_textFormatSmall.Get(), gap, w - gap, textY, m_brush.Get(), DWRITE_TEXT_ALIGNMENT_LEADING);
        
        float displayAvg4 = avgPerLap4;
        float displayAvg8 = avgPerLap8;
        if (imperial) {
            displayAvg4 *= 0.264172f;
            displayAvg8 *= 0.264172f;
        }
        swprintf(s, _countof(s), L"%.2f / %.2f %s", displayAvg4, displayAvg8, imperial ? L"gal" : L"lit");
        m_brush->SetColor(float4(1.0f, 1.0f, 1.0f, 0.95f));
        m_text.render(m_renderTarget.Get(), s, m_textFormat.Get(), gap, w - gap, textY, m_brush.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

        // Required To Finish
        textY += rowH;
        m_brush->SetColor(float4(0.7f, 0.7f, 0.7f, 0.8f));
        m_text.render(m_renderTarget.Get(), L"Needed to Finish:", m_textFormatSmall.Get(), gap, w - gap, textY, m_brush.Get(), DWRITE_TEXT_ALIGNMENT_LEADING);

        float remainingSessionLaps = -1.0f;
        int currentSessionLapsTotal = ir_SessionLapsTotal.getInt();
        if (currentSessionLapsTotal > 0 && currentSessionLapsTotal < 10000) {
            remainingSessionLaps = (float)currentSessionLapsTotal - (float)currentLap;
        }

        if (remainingSessionLaps >= 0 && perLapConsEst > 0) {
            float toFinish = std::max(0.0f, remainingSessionLaps * perLapConsEst - remainingFuel);
            if (imperial) toFinish *= 0.264172f;
            swprintf(s, _countof(s), L"%.1f %s", toFinish, imperial ? L"gal" : L"lit");
        } else {
            swprintf(s, _countof(s), L"--");
        }
        m_brush->SetColor(float4(1.0f, 0.3f, 0.3f, 0.95f));
        m_text.render(m_renderTarget.Get(), s, m_textFormat.Get(), gap, w - gap, textY, m_brush.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

        // Pit Stop Scheduled Add
        textY += rowH;
        m_brush->SetColor(float4(0.7f, 0.7f, 0.7f, 0.8f));
        m_text.render(m_renderTarget.Get(), L"Scheduled Pit Add:", m_textFormatSmall.Get(), gap, w - gap, textY, m_brush.Get(), DWRITE_TEXT_ALIGNMENT_LEADING);

        float addFuel = ir_PitSvFuel.getFloat();
        if (imperial) addFuel *= 0.264172f;
        if (ir_dpFuelFill.getFloat() > 0) {
            swprintf(s, _countof(s), L"+%.1f %s", addFuel, imperial ? L"gal" : L"lit");
            m_brush->SetColor(float4(1.0f, 0.85f, 0.1f, 0.95f));
        } else {
            swprintf(s, _countof(s), L"OFF");
            m_brush->SetColor(float4(0.5f, 0.5f, 0.5f, 0.8f));
        }
        m_text.render(m_renderTarget.Get(), s, m_textFormat.Get(), gap, w - gap, textY, m_brush.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);
    }

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormatBold;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormatSmall;
    TextCache m_text;

    float m_lapStartRemainingFuel = 0;
    std::deque<float> m_fuelUsedLastLaps;
    bool m_isValidFuelLap = false;
};
