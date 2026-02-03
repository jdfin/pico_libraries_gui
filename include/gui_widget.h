#pragma once

// framebuffer
#include "color.h"
#include "framebuffer.h"
// touchscreen
#include "touchscreen.h"


class GuiWidget
{
public:

    GuiWidget(Framebuffer &fb, int col, int row, int wid, int hgt, Color bg,
              bool visible = true, bool enabled = true) :
        _fb(fb),
        _col(col),
        _row(row),
        _wid(wid),
        _hgt(hgt),
        _bg(bg),
        _visible(visible),
        _enabled(enabled)
    {
    }

    virtual ~GuiWidget() = default;

    void visible(bool v)
    {
        _visible = v;
    }

    bool visible() const
    {
        return _visible;
    }

    // On the first call, force_draw can be used to force drawing.
    // It shouldn't be needed after that; the widget will be drawn if the
    // enable state changes.
    void enabled(bool e, bool force_draw=false)
    {
        // all widgets (so far) appear different when enabled/disabled
        if (_enabled != e) {
            _enabled = e;
            force_draw = true;
        }
        if (force_draw)
            draw();
    }

    bool contains(int c, int r) const
    {
        return c >= _col && c < (_col + _wid) && r >= _row && r < (_row + _hgt);
    }

    virtual void draw()
    {
    }

    virtual void erase()
    {
        if (_visible)
            _fb.fill_rect(_col, _row, _wid, _hgt, _bg);
    }

    // This is called for all widgets when there is an event until one returns
    // true. The one returning true often calls a user handler.
    virtual bool event(Touchscreen::Event &)
    {
        return false;
    }

    static GuiWidget *focus;

protected:

    Framebuffer &_fb;

    int _col;
    int _row;
    int _wid;
    int _hgt;

    Color _bg;

    bool _visible;
    bool _enabled;
};
