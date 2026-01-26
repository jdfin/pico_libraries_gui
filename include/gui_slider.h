#pragma once

//
// A slider is a widget that can be dragged to set a value within a range.
// There is a track with a 'handle' that can be moved along the track.
// Clicking on the track outside the handle moves the handle to that position.
//
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// x                             x     x                 x
// x                             x hnd x                 x
// x                             x     x                 x
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//

#include <climits>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "color.h"
#include "framebuffer.h"
// touchscreen
#include "touchscreen.h"
// gui
#include "gui_widget.h"

class GuiSlider : public GuiWidget
{
public:

    GuiSlider(Framebuffer &fb, int col, int row, int wid, int hgt, //
              Color fg, Color bg, Color track_bg, Color handle_bg, //
              int val_min, int val_max, int val_init,              //
              void (*on_value)(intptr_t), intptr_t on_value_arg);

    virtual void draw() override;

    virtual bool event(Touchscreen::Event &event) override;

    int get_value() const
    {
        return _val;
    }

    void set_value(int v);

private:

    const int _handle_wid;

    Color _fg;
    Color _track_bg;
    Color _handle_bg;

    const int _val_min;
    const int _val_max;
    int _val;

    // We could save the position of the handle instead of the value, but it's
    // a choice here to save the value instead. This makes is so the handle
    // 'jumps' between discrete value positions instead of smoothly sliding,
    // which seems right.

    void (*_on_value)(intptr_t);
    intptr_t _on_value_arg;

    int to_column(int val);
    int to_value(int col);

    void draw_handle();
    void erase_handle();

}; // class GuiSlider
