#pragma once

#include <cassert>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer.h"
#include "pixel_image.h"
// touchscreen
#include "touchscreen.h"
// gui
#include "gui_label.h"


// A button is a label that can be clicked.
//
// When enabled, the "enabled" image is shown.
// When disabled, the "disabled" image is shown.
// When pressed, the "pressed" image is shown on a "down" event, then the
// image reverts to the "enabled" image on an "up" event.
//
// Momentary (default)
// Down: on_down(), "pressed"
// Up:   on_up(), "enabled"
//       on_click() if the up event is inside the button.
//
// Toggle
// Down: on_down(), "pressed"
// Move: (nop)
// Up:   (nop)
// Down: on_down(), "enabled"
// Move: (nop)
// Up:   (nop)
// Down: on_down(), "pressed"
// Move: (nop)
// Up:   (nop)
//
// Sticky
// Down: on_down(), "pressed"
// Move: (nop)
// Up:   (nop)
// Down: (nop)
// Move: (nop)
// Up:   (nop)
// ...call pressed(false) to un-press it
// Down: on_down(), "pressed"
// Move: (nop)
// Up:   (nop)


class GuiButton : public GuiLabel
{
public:

    enum class Mode {
        Momentary,
        Check,
        Radio,
    };

    GuiButton(Framebuffer &fb, int col, int row, Color bg,       //
              const PixelImageHdr *img_enabled,                  //
              const PixelImageHdr *img_disabled,                 //
              const PixelImageHdr *img_pressed,                  //
              void (*on_click)(intptr_t), intptr_t on_click_arg, //
              void (*on_down)(intptr_t), intptr_t on_down_arg,   //
              void (*on_up)(intptr_t), intptr_t on_up_arg,       //
              Mode mode = Mode::Momentary, bool pressed = false) :
        GuiLabel(fb, col, row, bg, img_enabled, img_disabled),
        _img_pressed(img_pressed),
        _pressed(pressed),
        _mode(mode),
        _on_click(on_click),
        _on_click_arg(on_click_arg),
        _on_down(on_down),
        _on_down_arg(on_down_arg),
        _on_up(on_up),
        _on_up_arg(on_up_arg)
    {
    }

    virtual void draw() override
    {
        if (_visible)
            _fb.write(_col, _row,
                      _enabled ? (_pressed ? _img_pressed : _img_enabled)
                               : _img_disabled);
    }

    // System calls this to see if button wants to claim event
    virtual bool event(Touchscreen::Event &event) override;

    // Return true if button is pressed (useful for toggle or sticky buttons)
    bool pressed() const
    {
        return _pressed;
    }

    void pressed(bool p)
    {
        if (_pressed != p) {
            _pressed = p;
            draw();
        }
    }

protected:

    const PixelImageHdr *_img_pressed;

private:

    bool _pressed;
    const Mode _mode;

    void (*_on_click)(intptr_t);
    intptr_t _on_click_arg;

    void (*_on_down)(intptr_t);
    intptr_t _on_down_arg;

    void (*_on_up)(intptr_t);
    intptr_t _on_up_arg;
};
