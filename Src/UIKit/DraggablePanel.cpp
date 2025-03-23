#include "Common/Precompile.h"

#include "UIKit/DraggablePanel.h"

#include "Common/RuntimeError.h"

#include "UIKit/Application.h"
#include "UIKit/Cursor.h"

namespace d14engine::uikit
{
    DraggablePanel::DraggablePanel(
        const D2D1_RECT_F& rect,
        ComPtrParam<ID2D1Brush> brush,
        ComPtrParam<ID2D1Bitmap1> bitmap)
        :
        Panel(rect, brush, bitmap)
    {
        // Here left blank intentionally.
    }

    void DraggablePanel::onStartDragging()
    {
        onStartDraggingHelper();

        if (f_onStartDragging) f_onStartDragging(this);
    }

    void DraggablePanel::onEndDragging()
    {
        onEndDraggingHelper();

        if (f_onEndDragging) f_onEndDragging(this);
    }

    bool DraggablePanel::isTriggerDragging(const Event::Point& p)
    {
        if (!isDraggable) return false;

        if (f_isTriggerDragging)
        {
            return f_isTriggerDragging(this, p);
        }
        else return isTriggerDraggingHelper(p);
    }

    void DraggablePanel::onStartDraggingHelper()
    {
        THROW_IF_NULL(Application::g_app);

        auto& app = Application::g_app;

        enableChildrenMouseMoveEvent = false;

        app->focusUIObject
        (
            Application::FocusType::Mouse, shared_from_this()
        );
        if (draggingTarget == RootWindow)
        {
            app->isTriggerDraggingWin32Window = true;
        }
        app->cursor()->setIcon(Cursor::Move);
    }

    void DraggablePanel::onEndDraggingHelper()
    {
        THROW_IF_NULL(Application::g_app);

        auto& app = Application::g_app;

        enableChildrenMouseMoveEvent = true;

        app->focusUIObject
        (
            Application::FocusType::Mouse, nullptr
        );
        if (draggingTarget == RootWindow)
        {
            app->isTriggerDraggingWin32Window = false;
        }
        app->cursor()->setIcon(Cursor::Arrow);
    }

    bool DraggablePanel::isTriggerDraggingHelper(const Event::Point& p)
    {
        return isHit(p);
    }

    bool DraggablePanel::isDragging() const
    {
        return m_isDragging;
    }

    const DraggablePanel::DraggingPoint& DraggablePanel::draggingPoint() const
    {
        return m_draggingPoint;
    }

    void DraggablePanel::onMouseMoveHelper(MouseMoveEvent& e)
    {
        Panel::onMouseMoveHelper(e);

        onMouseMoveWrapper(e);
    }

    void DraggablePanel::onMouseMoveWrapper(MouseMoveEvent& e)
    {
        THROW_IF_NULL(Application::g_app);

        auto& p = e.cursorPoint;

        if (m_isDragging)
        {
            switch (draggingTarget)
            {
            case SelfObject:
            {
                if (std::holds_alternative<SelfPoint>(m_draggingPoint))
                {
                    auto& point = std::get<SelfPoint>(m_draggingPoint);

                    auto relative = absoluteToRelative(p);
                    setPosition(relative.x - point.x, relative.y - point.y);
                }
                break;
            }
            case RootWindow:
            {
                if (std::holds_alternative<RootPoint>(m_draggingPoint))
                {
                    auto& point = std::get<RootPoint>(m_draggingPoint);

                    POINT cursorPoint = {};
                    GetCursorPos(&cursorPoint);

                    int X = cursorPoint.x - point.x;
                    int Y = cursorPoint.y - point.y;

                    auto hwnd = Application::g_app->win32Window();
                    SetWindowPos(hwnd, HWND_TOP, X, Y, 0, 0, SWP_NOSIZE);
                }
                break;
            }
            default: break;
            }
            Application::g_app->cursor()->setIcon(Cursor::Move);
        }
    }

    void DraggablePanel::onMouseButtonHelper(MouseButtonEvent& e)
    {
        Panel::onMouseButtonHelper(e);

        onMouseButtonWrapper(e);
    }

    void DraggablePanel::onMouseButtonWrapper(MouseButtonEvent& e)
    {
        THROW_IF_NULL(Application::g_app);

        auto& p = e.cursorPoint;

        if (e.state.leftDown() || e.state.leftDblclk())
        {
            if (m_isDragging = isTriggerDragging(p))
            {
                switch (draggingTarget)
                {
                case SelfObject:
                {
                    m_draggingPoint = absoluteToSelfCoord(p);
                    break;
                }
                case RootWindow:
                {
                    POINT cursorPoint = {};
                    GetCursorPos(&cursorPoint);
                    ScreenToClient(Application::g_app->win32Window(), &cursorPoint);

                    m_draggingPoint = cursorPoint;
                    break;
                }
                default: m_draggingPoint = {};
                }
                onStartDragging();
            }
        }
        else if (e.state.leftUp())
        {
            if (m_isDragging)
            {
                m_isDragging = false;

                onEndDragging();
            }
        }
    }
}
