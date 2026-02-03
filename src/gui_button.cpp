
#include <cassert>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer.h"
// touchscreen
#include "touchscreen.h"
// gui
#include "gui_button.h"
#include "gui_widget.h"

using Event = Touchscreen::Event;


bool GuiButton::event(Event &event)
{
    if (!_visible || !_enabled)
        return false;

    // this widget has focus or no one has focus
    assert(focus == this || focus == nullptr);

    // handle event if:
    // 1. this widget has focus, or
    // 2. event (col, row) is in this widget's bounds

    if (GuiWidget::focus != this && !contains(event.col, event.row))
        return false;

    if (event.type == Event::Type::down) {
        focus = this;
        bool was_pressed = _pressed;
        if (_mode == Mode::Check)
            _pressed = !_pressed;
        else // Momentary or Radio
            _pressed = true;
        if (_pressed != was_pressed) {
            draw();
            if (_on_down != nullptr)
                (*_on_down)(_on_down_arg);
        }
    } else if (focus != this) {
        // If a touch starts outside a widget and slides into it, the widget
        // does not get the down, but will get a move (and up) even though it
        // does not have focus. In this case, we ignore the move and up.
    } else if (event.type == Event::Type::move) {
        // ignore
    } else if (event.type == Event::Type::up) {
        focus = nullptr;
        bool was_pressed = _pressed;
        if (_mode == Mode::Momentary)
            _pressed = false;
        if (_pressed != was_pressed) {
            draw();
            if (_on_up != nullptr)
                (*_on_up)(_on_up_arg);
            // if the up is within the button, it's a click
            if (_on_click != nullptr && contains(event.col, event.row))
                (*_on_click)(_on_click_arg);
        }
    }

    return true;
}
