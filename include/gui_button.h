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
// When pressed, the "pressed" image is shown on a "down" event,
// then the image reverts to the "enabled" image on an "up" event.
//
// If the down and up events are both within the button bounds,
// the on_click() handler is called.


class GuiButton : public GuiLabel
{
public:

    GuiButton(Framebuffer &fb, int col, int row, Color bg, //
              const PixelImageHdr *img_enabled,            //
              const PixelImageHdr *img_disabled,           //
              const PixelImageHdr *img_pressed,            //
              void (*on_click)(intptr_t), intptr_t on_click_arg) :
        GuiLabel(fb, col, row, bg, img_enabled, img_disabled),
        _img_pressed(img_pressed),
        _pressed(false),
        _on_click(on_click),
        _on_click_arg(on_click_arg)
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

protected:

    const PixelImageHdr *_img_pressed;

private:

    bool _pressed;

    void (*_on_click)(intptr_t);
    intptr_t _on_click_arg;
};
