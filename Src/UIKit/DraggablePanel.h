#pragma once

#include "Common/Precompile.h"

#include "UIKit/Panel.h"

namespace d14engine::uikit
{
    struct DraggablePanel : virtual Panel
    {
        DraggablePanel(
            const D2D1_RECT_F& rect = {},
            ComPtrParam<ID2D1Brush> brush = nullptr,
            ComPtrParam<ID2D1Bitmap1> bitmap = nullptr);

    public:
        void onStartDragging();

        Function<void(DraggablePanel*)> f_onStartDragging = {};

        void onEndDragging();

        Function<void(DraggablePanel*)> f_onEndDragging = {};

        bool isTriggerDragging(const Event::Point& p);

        Function<bool(DraggablePanel*, const Event::Point&)> f_isTriggerDragging = {};

    protected:
        virtual void onStartDraggingHelper();
        virtual void onEndDraggingHelper();

        virtual bool isTriggerDraggingHelper(const Event::Point& p);

    public:
        bool isDraggable = true;

        enum class DraggingTarget
        {
            SelfObject, RootWindow
        };
        constexpr static auto SelfObject = DraggingTarget::SelfObject;
        constexpr static auto RootWindow = DraggingTarget::RootWindow;

        DraggingTarget draggingTarget = SelfObject;

    protected:
        bool m_isDragging = false;

        using SelfPoint = D2D1_POINT_2F;
        using RootPoint = POINT;

        using DraggingPoint = Variant<std::monostate, SelfPoint, RootPoint>;

        DraggingPoint m_draggingPoint = {};

    public:
        bool isDragging() const;

        const DraggingPoint& draggingPoint() const;

    protected:
        // Panel
        void onMouseMoveHelper(MouseMoveEvent& e) override;
        void onMouseMoveWrapper(MouseMoveEvent& e);

        void onMouseButtonHelper(MouseButtonEvent& e) override;
        void onMouseButtonWrapper(MouseButtonEvent& e);
    };
}
