
#include <array>
#include <cassert>
#include <initializer_list>
// pico
#include "pico/stdlib.h"
// gui
#include "gui_page.h"
#include "gui_widget.h"


GuiPage::GuiPage(std::initializer_list<GuiWidget *> widgets,
                 void (*on_update)(intptr_t), intptr_t on_update_arg) :
    _widgets{},
    _widget_cnt(0),
    _visible(false),
    _busy(0),
    _on_update(on_update),
    _on_update_arg(on_update_arg)
{
    assert(widgets.size() <= max_widgets);
    for (GuiWidget *w : widgets)
        _widgets[_widget_cnt++] = w;
}


void GuiPage::visible(bool v)
{
    _visible = v;
    if (_visible)
        draw();
    else
        erase();
}


void GuiPage::draw() const
{
    if (_visible)
        for (size_t i = 0; i < _widget_cnt; i++)
            _widgets[i]->draw();
}


void GuiPage::erase() const
{
    for (size_t i = 0; i < _widget_cnt; i++)
        _widgets[i]->erase();
}


bool GuiPage::event(Touchscreen::Event &event)
{
    if (!_visible)
        return false;

    for (size_t i = 0; i < _widget_cnt; i++) {
        if (_widgets[i]->event(event))
            return true;
    }

    return false;
}
