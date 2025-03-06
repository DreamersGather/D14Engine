#pragma once

#include "Common/Precompile.h"

#include "UIKit/Event.h"

namespace d14engine::uikit
{
    bool Event::isKeyDown(int vkey)
    {
        return GetKeyState(vkey) & 0x8000;
    }

    bool Event::isKeyToggled(int vkey)
    {
        return GetKeyState(vkey) & 0x0001;
    }

    bool Event::LALT()
    {
        return isKeyDown(VK_LMENU);
    }

    bool Event::RALT()
    {
        return isKeyDown(VK_RMENU);
    }

    bool Event::ALT()
    {
        return LALT() || RALT();
    }

    bool Event::LCTRL()
    {
        return isKeyDown(VK_LCONTROL);
    }

    bool Event::RCTRL()
    {
        return isKeyDown(VK_RCONTROL);
    }

    bool Event::CTRL()
    {
        return LCTRL() || RCTRL();
    }

    bool Event::LSHIFT()
    {
        return isKeyDown(VK_LSHIFT);
    }

    bool Event::RSHIFT()
    {
        return isKeyDown(VK_RSHIFT);
    }

    bool Event::SHIFT()
    {
        return LSHIFT() || RSHIFT();
    }

    bool Event::CAPSLOCK()
    {
        return isKeyToggled(VK_CAPITAL);
    }

#define DEF_FLAG_ITEM(Key, Value) { WM_##Key, MouseButtonEvent::State::Flag::Value }

    const MouseButtonEvent::State::FlagMap MouseButtonEvent::State::g_flagMap =
    {
        DEF_FLAG_ITEM(LBUTTONDOWN,   LeftDown),
        DEF_FLAG_ITEM(LBUTTONUP,     LeftUp),
        DEF_FLAG_ITEM(LBUTTONDBLCLK, LeftDblclk),
        DEF_FLAG_ITEM(RBUTTONDOWN,   RightDown),
        DEF_FLAG_ITEM(RBUTTONUP,     RightUp),
        DEF_FLAG_ITEM(RBUTTONDBLCLK, RightDblclk),
        DEF_FLAG_ITEM(MBUTTONDOWN,   MiddleDown),
        DEF_FLAG_ITEM(MBUTTONUP,     MiddleUp),
        DEF_FLAG_ITEM(MBUTTONDBLCLK, MiddleDblclk)
    };

#undef DEF_FLAG_ITEM

    bool MouseButtonEvent::State::leftDown() const
    {
        return flag == Flag::LeftDown;
    }

    bool MouseButtonEvent::State::leftUp() const
    {
        return flag == Flag::LeftUp;
    }

    bool MouseButtonEvent::State::leftDblclk() const
    {
        return flag == Flag::LeftDblclk;
    }

    bool MouseButtonEvent::State::rightDown() const
    {
        return flag == Flag::RightDown;
    }

    bool MouseButtonEvent::State::rightUp() const
    {
        return flag == Flag::RightUp;
    }

    bool MouseButtonEvent::State::rightDblclk() const
    {
        return flag == Flag::RightDblclk;
    }

    bool MouseButtonEvent::State::middleDown() const
    {
        return flag == Flag::MiddleDown;
    }

    bool MouseButtonEvent::State::middleUp() const
    {
        return flag == Flag::MiddleUp;
    }

    bool MouseButtonEvent::State::middleDblclk() const
    {
        return flag == Flag::MiddleDblclk;
    }

    bool KeyboardEvent::State::pressed() const
    {
        return flag == Flag::Pressed;
    }

    bool KeyboardEvent::State::released() const
    {
        return flag == Flag::Released;
    }
}
