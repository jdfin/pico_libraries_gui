
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
        _pressed = true;
        draw(); // the "pressed" image
    } else if (event.type == Event::Type::move) {
        assert(focus == this);
        assert(_pressed);
    } else if (event.type == Event::Type::up) {
        assert(_pressed);
        assert(focus == this);
        focus = nullptr;
        _pressed = false;
        draw(); // the "not pressed" image
        // if the up is within the button, it's a click
        if (contains(event.col, event.row))
            (*_on_click)(_on_click_arg);
    }
    return true;
}
