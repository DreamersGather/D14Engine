#include "Common/Precompile.h"

#include "UIKit/ToggleButton.h"

#include "UIKit/IconLabel.h"
#include "UIKit/ResourceUtils.h"

namespace d14engine::uikit
{
    ToggleButton::ToggleButton(
        ShrdPtrRefer<IconLabel> content,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        FilledButton(content, roundRadius, rect)
    {
        StatefulObject::m_state = { Deactivated, StatefulObject::State::ButtonFlag::Idle };
        StatefulObject::m_stateDetail.flag = Deactivated;
    }

    ToggleButton::ToggleButton(
        WstrRefer text,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        ToggleButton(
            IconLabel::uniformLayout(text),
            roundRadius,
            rect) { }

    void ToggleButton::setActivated(StatefulObject::State::ActiveFlag flag)
    {
        StatefulObject::m_state.activeFlag = flag;

        StatefulObject::Event soe = {};
        soe.flag = StatefulObject::m_state.activeFlag;

        if (StatefulObject::m_stateDetail != soe)
        {
            StatefulObject::m_stateDetail = soe;
            onStateChange(StatefulObject::m_stateDetail);
        }
    }

    void ToggleButton::setActivatedState(StatefulObject::State::ActiveFlag flag)
    {
        StatefulObject::m_state.activeFlag = flag;
        StatefulObject::m_stateDetail.flag = StatefulObject::m_state.activeFlag;
    }

    void ToggleButton::onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr)
    {
        if (StatefulObject::m_state.activeFlag == Activated)
        {
            auto& dstSetting = Button::appearance();
            auto& srcSetting = appearance().main[(size_t)Button::m_state];

            dstSetting.foreground = srcSetting.foreground;
            dstSetting.background = srcSetting.background;
            dstSetting.stroke = srcSetting.stroke;

            Button::onRendererDrawD2d1ObjectHelper(rndr);
        }
        else FilledButton::onRendererDrawD2d1ObjectHelper(rndr);
    }

    void ToggleButton::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        FilledButton::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void ToggleButton::onMouseButtonReleaseHelper(Button::Event& e)
    {
        FilledButton::onMouseButtonReleaseHelper(e);

        if (e.left()) setActivated(StatefulObject::m_stateDetail.activated() ? Deactivated : Activated);
    }
}
