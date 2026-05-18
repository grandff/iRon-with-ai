#pragma once

#include "Overlay.h"
#include "iracing.h"

class OverlaySpotter : public Overlay
{
public:

    OverlaySpotter()
        : Overlay("OverlaySpotter")
    {
    }

protected:

    virtual float2 getDefaultSize() override
    {
        return float2{ 600, 150 };
    }

    virtual void onUpdate() override
    {
        const int spotterState = ir_CarLeftRight.getInt();
        
        // 0: Off, 1: Clear, 2: Left, 3: Right, 4: Left+Right, 5: 2 Left, 6: 2 Right
        bool carLeft = (spotterState == irsdk_LRCarLeft || spotterState == irsdk_LRCarLeftRight || spotterState == irsdk_LR2CarsLeft);
        bool carRight = (spotterState == irsdk_LRCarRight || spotterState == irsdk_LRCarLeftRight || spotterState == irsdk_LR2CarsRight);

        // UI Edit Mode logic to always show them for positioning
        if (m_uiEditEnabled) {
            carLeft = true;
            carRight = true;
        }

        m_d2dFactory->CreateSolidColorBrush( D2D1::ColorF(1.0f, 0.2f, 0.0f, 0.8f), &m_brush );
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> clearBrush;
        m_d2dFactory->CreateSolidColorBrush( D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f), &clearBrush );

        float rectWidth = m_width * 0.2f;

        if (carLeft) {
            D2D1_RECT_F leftRect = D2D1::RectF(0.0f, 0.0f, rectWidth, (float)m_height);
            m_renderTarget->FillRectangle(&leftRect, m_brush.Get());
        }

        if (carRight) {
            D2D1_RECT_F rightRect = D2D1::RectF((float)m_width - rectWidth, 0.0f, (float)m_width, (float)m_height);
            m_renderTarget->FillRectangle(&rightRect, m_brush.Get());
        }
    }
};
