﻿#include "Common/Precompile.h"

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
        appEventReactability.focus.get = true;
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

        forceGlobalExclusiveFocusing = true;

        if (operationTarget == OperationTarget::GlobalWin32Window)
        {
            Application::g_app->isTriggerDraggingWin32Window = true;
        }
        Application::g_app->cursor()->setIcon(Cursor::Move);
    }

    void DraggablePanel::onEndDraggingHelper()
    {
        THROW_IF_NULL(Application::g_app);

        forceGlobalExclusiveFocusing = false;

        if (operationTarget == OperationTarget::GlobalWin32Window)
        {
            Application::g_app->isTriggerDraggingWin32Window = false;
        }
        Application::g_app->cursor()->setIcon(Cursor::Arrow);
    }

    bool DraggablePanel::isTriggerDraggingHelper(const Event::Point& p)
    {
        return isHit(p);
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
            m_skipDeliverNextMouseMoveEventToChildren = true;

            switch (operationTarget)
            {
            case OperationTarget::LocalSelfWindow:
            {
                auto relative = absoluteToRelative(p);
                auto& draggingPoint = m_draggingPoint.localSelfWindow;

                move(relative.x - draggingPoint.x, relative.y - draggingPoint.y);
                break;
            }
            case OperationTarget::GlobalWin32Window:
            {
                auto hwnd = Application::g_app->win32Window();
                auto& draggingPoint = m_draggingPoint.globalWin32Window;

                POINT cursorPoint = {};
                GetCursorPos(&cursorPoint);

                int X = cursorPoint.x - draggingPoint.x;
                int Y = cursorPoint.y - draggingPoint.y;

                SetWindowPos(hwnd, HWND_TOP, X, Y, 0, 0, SWP_NOSIZE);
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
                m_draggingPoint.localSelfWindow = absoluteToSelfCoord(p);

                POINT cursorPoint = {};
                GetCursorPos(&cursorPoint);
                ScreenToClient(Application::g_app->win32Window(), &cursorPoint);

                m_draggingPoint.globalWin32Window = cursorPoint;

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
