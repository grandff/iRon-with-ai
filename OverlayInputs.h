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
#include "Config.h"
#include "OverlayDebug.h"

class OverlayInputs : public Overlay
{
    public:

        OverlayInputs()
            : Overlay("OverlayInputs")
        {}

    protected:

        virtual float2 getDefaultSize()
        {
            return float2(400,100);
        }

        virtual void onConfigChanged()
        {
            // Width might have changed, reset tracker values
            m_throttleVtx.resize( m_width );
            m_brakeVtx.resize( m_width );
            m_steerVtx.resize( m_width );
            for( int i=0; i<m_width; ++i )
            {
                m_throttleVtx[i].x = float(i);
                m_brakeVtx[i].x = float(i);
                m_steerVtx[i].x = float(i);
            }
        }

        virtual void onUpdate()
        {
            const float w = (float)m_width;
            const float h = (float)m_height;

            // Make code below safe against indexing into size-1 when sizes are zero
            if( m_throttleVtx.empty() )
                m_throttleVtx.resize( 1 );
            if( m_brakeVtx.empty() )
                m_brakeVtx.resize( 1 );
            if( m_steerVtx.empty() )
                m_steerVtx.resize( 1 );

            // Advance input vertices
            {
                for( int i=0; i<(int)m_throttleVtx.size()-1; ++i )
                    m_throttleVtx[i].y = m_throttleVtx[i+1].y;
                m_throttleVtx[(int)m_throttleVtx.size()-1].y = ir_Throttle.getFloat();

                for( int i=0; i<(int)m_brakeVtx.size()-1; ++i )
                    m_brakeVtx[i].y = m_brakeVtx[i+1].y;
                m_brakeVtx[(int)m_brakeVtx.size()-1].y = ir_Brake.getFloat();

                for( int i=0; i<(int)m_steerVtx.size()-1; ++i )
                    m_steerVtx[i].y = m_steerVtx[i+1].y;
                m_steerVtx[(int)m_steerVtx.size()-1].y = std::min( 1.0f, std::max( 0.0f, (ir_SteeringWheelAngle.getFloat() / ir_SteeringWheelAngleMax.getFloat()) * -0.5f + 0.5f) );
            }

            const float thickness = g_cfg.getFloat( m_name, "line_thickness", 2.0f );
            
            const float barWidth = 12.0f;
            const float gap = 6.0f;
            const float graphX0 = gap * 2 + barWidth;
            const float graphX1 = w - gap * 2 - barWidth;
            const float graphW = graphX1 - graphX0;

            auto vtx2coord = [&]( const float2& v )->float2 {
                float xPct = v.x / (w - 1.0f);
                float posX = graphX0 + xPct * graphW;
                return float2( posX, h-0.5f*thickness - v.y*(h-thickness) );
            };

            // Throttle (fill)
            Microsoft::WRL::ComPtr<ID2D1PathGeometry1> throttleFillPath;
            Microsoft::WRL::ComPtr<ID2D1GeometrySink>  throttleFillSink;
            m_d2dFactory->CreatePathGeometry( &throttleFillPath );
            throttleFillPath->Open( &throttleFillSink );
            throttleFillSink->BeginFigure( float2(graphX0,h), D2D1_FIGURE_BEGIN_FILLED );
            for( int i=0; i<(int)m_throttleVtx.size(); ++i )
                throttleFillSink->AddLine( vtx2coord(m_throttleVtx[i]) );
            throttleFillSink->AddLine( float2(graphX1,h) );
            throttleFillSink->EndFigure( D2D1_FIGURE_END_OPEN );
            throttleFillSink->Close();

            // Brake (fill)
            Microsoft::WRL::ComPtr<ID2D1PathGeometry1> brakeFillPath;
            Microsoft::WRL::ComPtr<ID2D1GeometrySink>  brakeFillSink;
            m_d2dFactory->CreatePathGeometry( &brakeFillPath );
            brakeFillPath->Open( &brakeFillSink );
            brakeFillSink->BeginFigure( float2(graphX0,h), D2D1_FIGURE_BEGIN_FILLED );
            for( int i=0; i<(int)m_brakeVtx.size(); ++i )
                brakeFillSink->AddLine( vtx2coord(m_brakeVtx[i]) );
            brakeFillSink->AddLine( float2(graphX1,h) );
            brakeFillSink->EndFigure( D2D1_FIGURE_END_OPEN );
            brakeFillSink->Close();

            // Throttle (line)
            Microsoft::WRL::ComPtr<ID2D1PathGeometry1> throttleLinePath;
            Microsoft::WRL::ComPtr<ID2D1GeometrySink>  throttleLineSink;
            m_d2dFactory->CreatePathGeometry( &throttleLinePath );
            throttleLinePath->Open( &throttleLineSink );
            throttleLineSink->BeginFigure( vtx2coord(m_throttleVtx[0]), D2D1_FIGURE_BEGIN_HOLLOW );
            for( int i=1; i<(int)m_throttleVtx.size(); ++i )
                throttleLineSink->AddLine( vtx2coord(m_throttleVtx[i]) );
            throttleLineSink->EndFigure( D2D1_FIGURE_END_OPEN );
            throttleLineSink->Close();

            // Brake (line)
            Microsoft::WRL::ComPtr<ID2D1PathGeometry1> brakeLinePath;
            Microsoft::WRL::ComPtr<ID2D1GeometrySink>  brakeLineSink;
            m_d2dFactory->CreatePathGeometry( &brakeLinePath );
            brakeLinePath->Open( &brakeLineSink );
            brakeLineSink->BeginFigure( vtx2coord(m_brakeVtx[0]), D2D1_FIGURE_BEGIN_HOLLOW );
            for( int i=1; i<(int)m_brakeVtx.size(); ++i )
                brakeLineSink->AddLine( vtx2coord(m_brakeVtx[i]) );
            brakeLineSink->EndFigure( D2D1_FIGURE_END_OPEN );
            brakeLineSink->Close();

            // Steering
            Microsoft::WRL::ComPtr<ID2D1PathGeometry1> steeringLinePath;
            Microsoft::WRL::ComPtr<ID2D1GeometrySink>  steeringLineSink;
            m_d2dFactory->CreatePathGeometry( &steeringLinePath );
            steeringLinePath->Open( &steeringLineSink );
            steeringLineSink->BeginFigure( vtx2coord(m_steerVtx[0]), D2D1_FIGURE_BEGIN_HOLLOW );
            for( int i=1; i<(int)m_steerVtx.size(); ++i )
                steeringLineSink->AddLine( vtx2coord(m_steerVtx[i]) );
            steeringLineSink->EndFigure( D2D1_FIGURE_END_OPEN );
            steeringLineSink->Close();

            
            m_brush->SetColor( g_cfg.getFloat4( m_name, "throttle_fill_col", float4(0.2f,0.45f,0.15f,0.6f) ) );
            m_renderTarget->FillGeometry( throttleFillPath.Get(), m_brush.Get() );
            m_brush->SetColor( g_cfg.getFloat4( m_name, "brake_fill_col", float4(0.46f,0.01f,0.06f,0.6f) ) );
            m_renderTarget->FillGeometry( brakeFillPath.Get(), m_brush.Get() );
            m_brush->SetColor( g_cfg.getFloat4( m_name, "throttle_col", float4(0.38f,0.91f,0.31f,0.8f) ) );
            m_renderTarget->DrawGeometry( throttleLinePath.Get(), m_brush.Get(), thickness );
            m_brush->SetColor( g_cfg.getFloat4( m_name, "brake_col", float4(0.93f,0.03f,0.13f,0.8f) ) );
            m_renderTarget->DrawGeometry( brakeLinePath.Get(), m_brush.Get(), thickness );
            m_brush->SetColor( g_cfg.getFloat4( m_name, "steering_col", float4(1,1,1,0.3f) ) );
            m_renderTarget->DrawGeometry( steeringLinePath.Get(), m_brush.Get(), thickness );

            // --- Draw Clutch & Handbrake Bars ---
            float clutchVal = ir_Clutch.isValid() ? ir_Clutch.getFloat() : 0.0f;
            float handbrakeVal = ir_HandbrakeRaw.isValid() ? ir_HandbrakeRaw.getFloat() : 0.0f;

            // Clutch (Left Side)
            D2D1_ROUNDED_RECT clutchBg = D2D1::RoundedRect(D2D1::RectF(gap, gap, gap + barWidth, h - gap), 2.0f, 2.0f);
            m_brush->SetColor(float4(0.15f, 0.15f, 0.15f, 0.5f));
            m_renderTarget->FillRoundedRectangle(&clutchBg, m_brush.Get());
            
            if (clutchVal > 0.01f) {
                float fillY = (h - gap * 2) * clutchVal;
                D2D1_ROUNDED_RECT clutchFill = D2D1::RoundedRect(D2D1::RectF(gap, h - gap - fillY, gap + barWidth, h - gap), 2.0f, 2.0f);
                m_brush->SetColor(float4(0.1f, 0.45f, 0.85f, 0.8f));
                m_renderTarget->FillRoundedRectangle(&clutchFill, m_brush.Get());
            }
            m_brush->SetColor(float4(1.0f, 1.0f, 1.0f, 0.2f));
            m_renderTarget->DrawRoundedRectangle(&clutchBg, m_brush.Get(), 1.0f);

            // Handbrake (Right Side)
            D2D1_ROUNDED_RECT hbBg = D2D1::RoundedRect(D2D1::RectF(w - gap - barWidth, gap, w - gap, h - gap), 2.0f, 2.0f);
            m_brush->SetColor(float4(0.15f, 0.15f, 0.15f, 0.5f));
            m_renderTarget->FillRoundedRectangle(&hbBg, m_brush.Get());

            if (handbrakeVal > 0.01f) {
                float fillY = (h - gap * 2) * handbrakeVal;
                D2D1_ROUNDED_RECT hbFill = D2D1::RoundedRect(D2D1::RectF(w - gap - barWidth, h - gap - fillY, w - gap, h - gap), 2.0f, 2.0f);
                m_brush->SetColor(float4(0.9f, 0.5f, 0.1f, 0.8f));
                m_renderTarget->FillRoundedRectangle(&hbFill, m_brush.Get());
            }
            m_brush->SetColor(float4(1.0f, 1.0f, 1.0f, 0.2f));
            m_renderTarget->DrawRoundedRectangle(&hbBg, m_brush.Get(), 1.0f);
            
        }

    protected:

        std::vector<float2> m_throttleVtx;
        std::vector<float2> m_brakeVtx;
        std::vector<float2> m_steerVtx;
};
