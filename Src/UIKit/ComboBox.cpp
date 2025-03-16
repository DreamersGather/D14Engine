#include "Common/Precompile.h"

#include "UIKit/ComboBox.h"

#include "Common/CppLangUtils/PointerEquality.h"
#include "Common/DirectXError.h"
#include "Common/MathUtils/2D.h"

#include "UIKit/Application.h"
#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/PopupMenu.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    ComboBox::ComboBox(float roundRadius, const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        FlatButton(IconLabel::comboBoxLayout(), roundRadius, rect)
    {
        m_dropDownMenu = makeRootUIObject<PopupMenu>();

        m_dropDownMenu->f_onTriggerMenuItem = [this]
        (PopupMenu* menu, PopupMenu::ItemIndexParam itemIndex)
        {
            setSelected(itemIndex.index);
        };
        m_dropDownMenu->setBackgroundTriggerPanel(true);

        auto& geoSetting = m_dropDownMenu->appearance().geometry;
        geoSetting.extension = geoSetting.roundRadius = roundRadius;

        // The shadow bitmap of the drop-down menu will be loaded later
        // in onInitializeFinish when resizing the menu to fit the size.
    }

    void ComboBox::onInitializeFinish()
    {
        FlatButton::onInitializeFinish();

        loadArrowIconStrokeStyle();

        m_dropDownMenu->resize(width(), m_dropDownMenu->height());
        m_dropDownMenu->move(m_absoluteRect.left, m_absoluteRect.bottom);
    }

    void ComboBox::loadArrowIconStrokeStyle()
    {
        THROW_IF_NULL(Application::g_app);

        auto factory = Application::g_app->dx12Renderer()->d2d1Factory();

        auto prop = D2D1::StrokeStyleProperties
        (
        /* startCap */ D2D1_CAP_STYLE_ROUND,
        /* endCap   */ D2D1_CAP_STYLE_ROUND,
        /* dashCap  */ D2D1_CAP_STYLE_ROUND
        );
        auto& style = arrowIcon.strokeStyle;

        THROW_IF_FAILED(factory->CreateStrokeStyle
        (
        /* strokeStyleProperties */ prop,
        /* dashes                */ nullptr,
        /* dashesCount           */ 0,
        /* strokeStyle           */ &style
        ));
    }

    void ComboBox::onSelectedChange(IconLabel* content)
    {
        onSelectedChangeHelper(content);

        if (f_onSelectedChange) f_onSelectedChange(this, content);
    }

    void ComboBox::onSelectedChangeHelper(IconLabel* content)
    {
        // This method intentionally left blank.
    }

    const WeakPtr<MenuItem>& ComboBox::selected() const
    {
        return m_selected;
    }

    void ComboBox::setSelected(size_t indexInDropDownMenu)
    {
        WeakPtr<MenuItem> originalSelected = m_selected;

        auto& items = m_dropDownMenu->items();
        if (indexInDropDownMenu >= 0 && indexInDropDownMenu < items.size())
        {
            auto itor = std::next(items.begin(), indexInDropDownMenu);
            auto newContent = (*itor)->getContent<IconLabel>().lock();

            m_selected = (*itor);
            if (newContent != nullptr)
            {
                m_content->icon = newContent->icon;

                auto newLabel = newContent->label().get();
                m_content->label()->copyTextStyle(newLabel, newLabel->text());

                m_content->updateLayout();
                m_content->setPrivateVisible(true);
            }
        }
        else // Typically, pass SIZE_MAX to clear current selected item.
        {
            m_selected.reset();
            m_content->setPrivateVisible(false);
        }
        if (!cpp_lang_utils::isMostDerivedEqual(originalSelected.lock(), m_selected.lock()))
        {
            onSelectedChange(m_content.get());
        }
    }

    const SharedPtr<PopupMenu>& ComboBox::dropDownMenu() const
    {
        return m_dropDownMenu;
    }

    void ComboBox::setDropDownMenu(ShrdPtrRefer<PopupMenu> menu)
    {
        if (menu && !cpp_lang_utils::isMostDerivedEqual(menu, m_dropDownMenu))
        {
            setSelected(SIZE_MAX);

            m_dropDownMenu->release();
            m_dropDownMenu = menu;
        }
    }

    void ComboBox::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        FlatButton::onRendererDrawD2d1ObjectHelper(rndr);

        /////////////////////
        // Drop-down Arrow //
        /////////////////////

        auto& arrowSetting = appearance().arrow;
        auto& arrowGeometry = arrowSetting.geometry;
        auto& arrowBackground = m_enabled ? arrowSetting.background : arrowSetting.secondaryBackground;

        resource_utils::solidColorBrush()->SetColor(arrowBackground.color);
        resource_utils::solidColorBrush()->SetOpacity(arrowBackground.opacity);

        auto arrowOrigin = math_utils::rightTop(m_absoluteRect);

        rndr->d2d1DeviceContext()->DrawLine
        (
        /* point0      */ math_utils::offset(arrowOrigin, arrowGeometry.line0.point0),
        /* point1      */ math_utils::offset(arrowOrigin, arrowGeometry.line0.point1),
        /* brush       */ resource_utils::solidColorBrush(),
        /* strokeWidth */ arrowSetting.strokeWidth,
        /* strokeStyle */ arrowIcon.strokeStyle.Get()
        );
        rndr->d2d1DeviceContext()->DrawLine
        (
        /* point0      */ math_utils::offset(arrowOrigin, arrowGeometry.line1.point0),
        /* point1      */ math_utils::offset(arrowOrigin, arrowGeometry.line1.point1),
        /* brush       */ resource_utils::solidColorBrush(),
        /* strokeWidth */ arrowSetting.strokeWidth,
        /* strokeStyle */ arrowIcon.strokeStyle.Get()
        );
    }

    void ComboBox::onSizeHelper(SizeEvent& e)
    {
        FlatButton::onSizeHelper(e);

        m_dropDownMenu->resize(e.size.width, m_dropDownMenu->height());
    }

    void ComboBox::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        Button::onChangeThemeStyleHelper(style);

        appearance().changeTheme(FlatButton::appearance(), style.name);
    }

    void ComboBox::onMouseButtonReleaseHelper(ClickablePanel::Event& e)
    {
        FlatButton::onMouseButtonReleaseHelper(e);

        if (e.left())
        {
            if (!menuOffset.has_value())
            {
                auto offsetY = m_selected.expired() ? 0.0f : -m_selected.lock()->position().y;
                m_dropDownMenu->move(math_utils::offset(absolutePosition(), { 0.0f, offsetY }));
            }
            else m_dropDownMenu->move(selfCoordToAbsolute(menuOffset.value()));

            m_dropDownMenu->setActivated(true);
        }
    }
}
