#pragma once

#include <cassert>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "color.h"
#include "framebuffer.h"
#include "pixel_image.h"
// gui
#include "gui_widget.h"


// A label can be:
// * visible or not; if not visible, it is not drawn
// * enabled or disabled, which just selects one of two images to draw
// A label does not handle input events.

class GuiLabel : public GuiWidget
{
public:

    GuiLabel(Framebuffer &fb, int col, int row, Color bg,
             const PixelImageHdr *img_enabled,
             const PixelImageHdr *img_disabled, bool visible = true) :
        GuiWidget(fb, col, row, img_enabled->wid, img_enabled->hgt, bg,
                  visible),
        _img_enabled(img_enabled),
        _img_disabled(img_disabled)
    {
        assert(_img_enabled != nullptr);
        assert(_img_disabled != nullptr);
        assert(_img_disabled->wid == _wid);
        assert(_img_disabled->hgt == _hgt);
    }

    virtual void draw() override
    {
        if (_visible)
            _fb.write(_col, _row, _enabled ? _img_enabled : _img_disabled);
    }

protected:

    const PixelImageHdr *_img_enabled;
    const PixelImageHdr *_img_disabled;
};
