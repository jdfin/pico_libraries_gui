#pragma once

#include <array>
#include <cassert>
#include <initializer_list>
// pico
#include "pico/stdlib.h"
// touchscreen
#include "touchscreen.h"
// gui
#include "gui_widget.h"


class GuiPage
{
public:

    GuiPage(std::initializer_list<GuiWidget *> widgets,
            void (*on_update)(intptr_t) = nullptr, intptr_t on_update_arg = 0);

    void visible(bool v);

    int busy() const
    {
        return _busy;
    }

    void busy(int b)
    {
        _busy = b;
    }

    void draw() const;

    void erase() const;

    // System calls this to see if anything on the page wants to claim event
    bool event(Touchscreen::Event &event);

    // System calls this to see if the page wants to update itself
    void update()
    {
        if (_visible && _on_update != nullptr)
            _on_update(_on_update_arg);
    }

private:

    static const size_t max_widgets = 30;
    std::array<GuiWidget *, max_widgets> _widgets;
    unsigned _widget_cnt;
    bool _visible;
    int _busy;
    void (*_on_update)(intptr_t);
    intptr_t _on_update_arg;
};
