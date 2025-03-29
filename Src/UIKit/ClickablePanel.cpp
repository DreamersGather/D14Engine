#include "Common/Precompile.h"

#include "UIKit/ClickablePanel.h"

namespace d14engine::uikit
{
    void ClickablePanel::onMouseButtonPress(Event& e)
    {
        onMouseButtonPressHelper(e);

        if (f_onMouseButtonPress) f_onMouseButtonPress(this, e);
    }

    void ClickablePanel::onMouseButtonRelease(Event& e)
    {
        onMouseButtonReleaseHelper(e);

        if (f_onMouseButtonRelease) f_onMouseButtonRelease(this, e);
    }

    void ClickablePanel::onMouseButtonPressHelper(Event& e)
    {
        // This method intentionally left blank.
    }

    void ClickablePanel::onMouseButtonReleaseHelper(Event& e)
    {
        // This method intentionally left blank.
    }

    void ClickablePanel::setEnabled(bool value)
    {
        Panel::setEnabled(value);

        setEnabledWrapper(value);
    }

    void ClickablePanel::setEnabledWrapper(bool value)
    {
        m_leftPressed = m_rightPressed = m_middlePressed = false;
    }

    void ClickablePanel::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        Panel::onMouseLeaveHelper(e);

        onMouseLeaveWrapper(e);
    }

    void ClickablePanel::onMouseLeaveWrapper(MouseMoveEvent& e)
    {
        m_leftPressed = m_rightPressed = m_middlePressed = false;
    }

    void ClickablePanel::onMouseButtonHelper(MouseButtonEvent& e)
    {
        Panel::onMouseButtonHelper(e);

        onMouseButtonWrapper(e);
    }

    void ClickablePanel::onMouseButtonWrapper(MouseButtonEvent& e)
    {
        Event be = {};
        be.cursorPoint = e.cursorPoint;

        bool leftPressed = e.state.leftDown() || (captureDoubleClick && e.state.leftDblclk());
        bool rightPressed = e.state.rightDown() || (captureDoubleClick && e.state.rightDblclk());
        bool middlePressed = e.state.middleDown() || (captureDoubleClick && e.state.middleDblclk());

        if (leftPressed || rightPressed || middlePressed)
        {
            if (leftPressed)
            {
                m_leftPressed = true;
                be.flag = Event::Flag::Left;
            }
            else if (rightPressed)
            {
                m_rightPressed = true;
                be.flag = Event::Flag::Right;
            }
            else if (middlePressed)
            {
                m_middlePressed = true;
                be.flag = Event::Flag::Middle;
            }
            else be.flag = Event::Flag::Unknown;

            onMouseButtonPress(be);
        }
        else if (m_leftPressed || m_rightPressed || m_middlePressed)
        {
            if (e.state.leftUp() || e.state.rightUp() || e.state.middleUp())
            {
                if (e.state.leftUp())
                {
                    m_leftPressed = false;
                    be.flag = Event::Flag::Left;
                }
                else if (e.state.rightUp())
                {
                    m_rightPressed = false;
                    be.flag = Event::Flag::Right;
                }
                else if (e.state.middleUp())
                {
                    m_middlePressed = false;
                    be.flag = Event::Flag::Middle;
                }
                else be.flag = Event::Flag::Unknown;

                onMouseButtonRelease(be);
            }
        }
    }
}
