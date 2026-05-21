/*
MIT License

Copyright (c) 2021-2022 L. E. Spalt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "Overlay.h"
#include <string>

class OverlayCover : public Overlay
{
    public:

        OverlayCover()
            : Overlay("OverlayCover")
        {}

    protected:

        virtual bool hasCustomBackground() override
        {
            return true;
        }

        virtual void onConfigChanged() override
        {
            const std::string font = g_cfg.getString(m_name, "font", "Microsoft YaHei UI");
            const float fontSize = g_cfg.getFloat(m_name, "font_size", 16.0f);
            
            // Create Bold & Italic text format
            HRCHECK(m_dwriteFactory->CreateTextFormat(
                toWide(font).c_str(), 
                NULL, 
                DWRITE_FONT_WEIGHT_BOLD, 
                DWRITE_FONT_STYLE_ITALIC, 
                DWRITE_FONT_STRETCH_NORMAL, 
                fontSize, 
                L"en-us", 
                &m_textFormat
            ));
            
            // Align Bottom-Right
            m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
            m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
        }

        virtual void onUpdate() override
        {
            const float w = (float)m_width;
            const float h = (float)m_height;
            const float cornerRadius = g_cfg.getFloat( m_name, "corner_radius", 6.0f );

            
            m_renderTarget->Clear( float4(0,0,0,0) );

            // 1. Draw solid black background
            D2D1_ROUNDED_RECT rr;
            rr.rect = { 0.5f, 0.5f, w-0.5f, h-0.5f };
            rr.radiusX = cornerRadius;
            rr.radiusY = cornerRadius;
            m_brush->SetColor( D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f) ); // Opaque Black
            m_renderTarget->FillRoundedRectangle( &rr, m_brush.Get() );

            // 2. Draw "iRon-advanced" text in the bottom right corner
            m_brush->SetColor( D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.8f) ); // White (80% opacity)
            
            // Padding from edges (8px right, 4px bottom)
            D2D1_RECT_F textRect = D2D1::RectF(0, 0, w - 8.0f, h - 4.0f);
            
            std::wstring watermark = L"iRon-advanced";
            m_renderTarget->DrawText(
                watermark.c_str(), 
                (UINT32)watermark.length(), 
                m_textFormat.Get(), 
                textRect, 
                m_brush.Get()
            );

            
        }

    private:
        Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
};
