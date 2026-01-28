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

    GuiPage(std::initializer_list<GuiWidget *> widgets);

    void visible(bool v);

    void draw() const;

    void erase() const;

    // System calls this to see if anything on the page wants to claim event
    bool event(Touchscreen::Event &event);

private:

    static const size_t max_widgets = 20;
    std::array<GuiWidget *, max_widgets> _widgets;
    unsigned _widget_cnt;
    bool _visible;
};
