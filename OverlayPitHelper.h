#pragma once

#include "Overlay.h"
#include "iracing.h"
#include <string>

class OverlayPitHelper : public Overlay
{
public:

    OverlayPitHelper()
        : Overlay("OverlayPitHelper")
    {
    }

protected:

    virtual float2 getDefaultSize() override
    {
        return float2{ 400, 160 };
    }

    virtual void onConfigChanged() override
    {
        m_text.reset(m_dwriteFactory.Get());

        const std::string font = g_cfg.getString(m_name, "font", "Microsoft YaHei UI");
        
        // Fonts for different sections
        const float titleFontSize = g_cfg.getFloat(m_name, "title_font_size", 32.0f);
        const float valueFontSize = g_cfg.getFloat(m_name, "value_font_size", 48.0f);
        const float smallFontSize = g_cfg.getFloat(m_name, "small_font_size", 24.0f);
        
        const int fontWeight = g_cfg.getInt(m_name, "font_weight", 800); // Bold

        HRCHECK(m_dwriteFactory->CreateTextFormat(toWide(font).c_str(), NULL, (DWRITE_FONT_WEIGHT)fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, titleFontSize, L"en-us", &m_titleFormat));
        m_titleFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        m_titleFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        m_titleFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

        HRCHECK(m_dwriteFactory->CreateTextFormat(toWide(font).c_str(), NULL, (DWRITE_FONT_WEIGHT)fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, valueFontSize, L"en-us", &m_valueFormat));
        m_valueFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        m_valueFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        m_valueFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

        HRCHECK(m_dwriteFactory->CreateTextFormat(toWide(font).c_str(), NULL, (DWRITE_FONT_WEIGHT)fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, smallFontSize, L"en-us", &m_smallFormat));
        m_smallFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        m_smallFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        m_smallFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    virtual void onUpdate() override
    {
        bool showOverlay = false;
        
        if (m_uiEditEnabled) {
            showOverlay = true;
        } else {
            // Show if player is on pit road or in pit stall
            showOverlay = ir_OnPitRoad.getBool() || ir_PlayerCarInPitStall.getBool();
        }

        if (!showOverlay) return;

        // Colors
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.1f, 0.1f, 0.1f, 0.9f), &bgBrush);
        
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textWhiteBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &textWhiteBrush);

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textYellowBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.8f, 0.0f, 1.0f), &textYellowBrush);

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> borderBrush;
        m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.6f, 1.0f, 1.0f), &borderBrush);

        // Background
        D2D1_ROUNDED_RECT bgRect = D2D1::RoundedRect(D2D1::RectF(0, 0, (float)m_width, (float)m_height), 8.0f, 8.0f);
        m_renderTarget->FillRoundedRectangle(&bgRect, bgBrush.Get());
        m_renderTarget->DrawRoundedRectangle(&bgRect, borderBrush.Get(), 3.0f);

        // State detection
        bool inStall = ir_PlayerCarInPitStall.getBool();
        float repairLeft = ir_PitRepairLeft.getFloat();
        int engineWarnings = ir_EngineWarnings.getInt();
        bool limiterActive = (engineWarnings & irsdk_pitSpeedLimiter) != 0;

        float speed = ir_Speed.getFloat(); // m/s
        float speedKmh = speed * 3.6f;
        float speedMph = speed * 2.23694f;

        std::wstring titleText = L"PIT LIMIT";
        std::wstring valueText;
        std::wstring subText;

        if (m_uiEditEnabled) {
            inStall = false;
            limiterActive = true;
            ir_session.trackPitSpeedLimitStr = "60.00 km/h";
            speedKmh = 59.5f;
            repairLeft = 0.0f;
        }

        if (inStall) {
            titleText = L"PIT STOP";
            if (repairLeft > 0.0f) {
                wchar_t buf[32];
                swprintf(buf, _countof(buf), L"REPAIR: %.1fs", repairLeft);
                valueText = buf;
            } else {
                valueText = L"SERVICING...";
            }
            subText = L"WAIT FOR CREW";
        } else {
            // Approaching or leaving
            titleText = L"PIT LANE";
            
            // Speed and Limiter
            wchar_t buf[64];
            bool isImperial = ir_DisplayUnits.getInt() == 0;
            
            if (isImperial) {
                swprintf(buf, _countof(buf), L"%.0f MPH", speedMph);
            } else {
                swprintf(buf, _countof(buf), L"%.0f KM/H", speedKmh);
            }
            valueText = buf;

            // Speed Limit string from YAML
            if (!ir_session.trackPitSpeedLimitStr.empty()) {
                std::string limitStr = ir_session.trackPitSpeedLimitStr;
                subText = L"LIMIT: " + toWide(limitStr);
            } else {
                subText = L"LIMIT: UNKNOWN";
            }
        }

        // Title 
        m_text.render(m_renderTarget.Get(), titleText.c_str(), m_titleFormat.Get(), 0, (float)m_width, 35.0f, textYellowBrush.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);

        // Value (Speed or Repair Time)
        ID2D1SolidColorBrush* valueBrush = textWhiteBrush.Get();
        if (!inStall && !limiterActive) {
            // Flash red if limiter is NOT active in pit lane
            static DWORD startTick = GetTickCount();
            DWORD currentTick = GetTickCount();
            bool isBlinkOn = ((currentTick - startTick) % 500) < 250;
            if (isBlinkOn) {
                m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.2f, 0.2f, 1.0f), &valueBrush);
            }
        } else if (inStall && repairLeft > 0.0f) {
             m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.5f, 0.0f, 1.0f), &valueBrush); // Orange for repair
        }

        m_text.render(m_renderTarget.Get(), valueText.c_str(), m_valueFormat.Get(), 0, (float)m_width, 85.0f, valueBrush, DWRITE_TEXT_ALIGNMENT_CENTER);

        // Subtext (Limit)
        m_text.render(m_renderTarget.Get(), subText.c_str(), m_smallFormat.Get(), 0, (float)m_width, 135.0f, textWhiteBrush.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
    }

private:
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_titleFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_valueFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_smallFormat;
    TextCache m_text;
};