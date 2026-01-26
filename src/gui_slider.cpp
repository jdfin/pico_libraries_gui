
#include <cassert>
#include <cstdio>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "color.h"
#include "framebuffer.h"
// touchscreen
#include "touchscreen.h"
// gui
#include "gui_slider.h"
#include "gui_widget.h"

using Event = Touchscreen::Event;


GuiSlider::GuiSlider(Framebuffer &fb, int col, int row, int wid, int hgt, //
                     Color fg, Color bg, Color track_bg, Color handle_bg, //
                     int val_min, int val_max, int val_init,              //
                     void (*on_value)(intptr_t), intptr_t on_value_arg) :
    GuiWidget(fb, col, row, wid, hgt, bg),
    _handle_wid(hgt / 2 * 2 + 1), // square, but make sure it's odd
    _fg(fg),
    _track_bg(track_bg),
    _handle_bg(handle_bg),
    _val_min(val_min),
    _val_max(val_max),
    _val(val_init),
    _on_value(on_value),
    _on_value_arg(on_value_arg)
{
}


// Mapping column to value and vice versa.
// Assume the left edge of the slider is at zero (it's really at _col).
//
// The leftmost column of the handle is handle_wid/2
// The rightmost column of the slider is (wid - 1) - handle_wid/2
// The total number of columns available for the handle center is
//    (wid - 1) - handle_wid/2 - handle_wid/2
//    wid - 1 - handle_wid
//
// There are val_max - val_min + 1 possible values, e.g. for min=0 and max=10
// there are 11 values. Usually there are more horizontal pixels than values,
// so multiple pixel map to the same value. On average there should be:
//    (wid - 1 - handle_wid) / (val_max - val_min + 1)
// columns per value.
//
// We convert between value and column such that the range of columns mapping
// to val_min is centered at the leftmost position of the handle, and the
// range of columns mapping to val_max is centered at the rightmost position.
// That does mean that there are half as many column positions mapping to
// val_min and val_max as there are for the other values, e.g. for min=1 and
// max=3, if there are 100 columns of handle range, then the left 25 map to
// val=1, the middle 50 map to val=2, and the right 25 map to val=3. The
// touch point can be slid past the left and right limits to get the min and
// max values, so this seems like a good mapping. It is important that min
// maps exactly to the leftmost position so the handle is against the left
// edge, and similar for the rightmost position and max.


// Find handle position for a given value
int GuiSlider::to_column(int v)
{
    // Forcing the handle width always be odd makes it easier to ensure we get
    // all the way left and right instead of being a pixel off sometimes
    assert(_handle_wid % 2 == 1);

    // Ignore slider's actual position (_col) and alignment at first and just
    // find the offset within the track.

    const int off_min = _handle_wid / 2;
    const int off_max = _wid - 1 - _handle_wid / 2;
    int off;
    if (v <= _val_min) {
        off = off_min;
    } else if (v >= _val_max) {
        off = off_max;
    } else {
        const int off_rng = off_max - off_min;
        const int val_rng = _val_max - _val_min;
        off = off_min + ((v - _val_min) * off_rng + val_rng / 2) / val_rng;
    }

    return _col + off;
}


// find value for given handle position
int GuiSlider::to_value(int c)
{
    assert(_handle_wid % 2 == 1); // must be odd

    // convert absolute column to offset within track
    int off = c - _col;

    const int off_min = _handle_wid / 2;
    if (off <= off_min)
        return _val_min;

    const int off_max = _wid - 1 - _handle_wid / 2;
    if (off >= off_max)
        return _val_max;

    const int off_rng = off_max - off_min;
    const int val_rng = _val_max - _val_min;
    return _val_min + ((off - off_min) * val_rng + off_rng / 2) / off_rng;
}


void GuiSlider::draw_handle()
{
    int handle_ctr = to_column(_val);
    const int left = handle_ctr - _handle_wid / 2;
    const int right = handle_ctr + _handle_wid / 2;
    _fb.line(left, _row + 1, left, _row + _hgt - 2, _fg);
    _fb.line(right, _row + 1, right, _row + _hgt - 2, _fg);
    _fb.fill_rect(left + 1, _row + 1, _handle_wid - 2, _hgt - 2, _handle_bg);
}


void GuiSlider::erase_handle()
{
    int handle_ctr = to_column(_val);

    // fill to cover the left and right edges of the handle
    int handle_left = handle_ctr - _handle_wid / 2;
    int width = _handle_wid;
    if (handle_left == _col) {
        // at left end, don't fill left edge
        handle_left++;
        width--;
    } else if ((handle_left + _handle_wid) == (_col + _wid)) {
        // at right end, don't fill right edge
        width--;
    }
    _fb.fill_rect(handle_left, _row + 1, width, _hgt - 2, _track_bg);
}


// draw track, then handle
void GuiSlider::draw()
{
    if (_visible) {
        _fb.draw_rect(_col, _row, _wid, _hgt, _fg);
        _fb.fill_rect(_col + 1, _row + 1, _wid - 2, _hgt - 2, _track_bg);
        draw_handle();
    }
}


bool GuiSlider::event(Event &event)
{
    if (!_visible || !_enabled)
        return false;

    // this widget has focus or no one has focus
    assert(focus == this || focus == nullptr);

    // handle event if:
    // 1. this widget has focus, or
    // 2. event (col, row) is in this widget's bounds

    if (GuiWidget::focus == this || contains(event.col, event.row)) {

        // on 'down' or 'move', update value and redraw handle
        // on 'up', just release focus

        if (event.type == Event::Type::down ||
            event.type == Event::Type::move) {
            int new_val = to_value(event.col);
            //printf("GuiSlider::on_event: event type %s at (%d,%d), new_val=%d\n",
            //       event.type_name(), event.col, event.row, new_val);
            if (new_val < _val_min)
                new_val = _val_min;
            if (new_val > _val_max)
                new_val = _val_max;
            if (new_val != _val) {
                set_value(new_val); // sets _val and updates handle
                (*_on_value)(_on_value_arg);
            }
            // take (or keep) focus
            focus = this;
        } else {
            focus = nullptr;
        }
        return true;
    }
    return false;
}


void GuiSlider::set_value(int v)
{
    if (v < _val_min)
        v = _val_min;
    if (v > _val_max)
        v = _val_max;

    if (v != _val) {
        erase_handle();
        _val = v;
        draw_handle();
    }
}
