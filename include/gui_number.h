#pragma once

// pico
#include "pico/stdlib.h"
// framebuffer
#include "color.h"
#include "framebuffer.h"
// gui
#include "gui_widget.h"

// GuiNumber is a widget that displays a number that can be changed.
//
// Implementation uses an array of ten prebuilt digit images to avoid having
// to render text at runtime.
//
// Key differences between this and other widgets:
//   the width is variable; and
//   alignment is supported (left, center, right).


class GuiNumber : public GuiWidget
{
public:

    GuiNumber(Framebuffer &fb, int col, int row, Color bg,
              const PixelImageHdr *dig[], int num,
              Framebuffer::HAlign h_align = Framebuffer::HAlign::Left) :
        GuiWidget(fb, col, row, 0, 0, bg),
        _dig(dig),
        _num(num),
        _h_align(h_align),
        _col_ref(col)
    {
    }

    virtual void draw() override
    {
        if (_visible) {
            // fb.write() sets _wid and _hgt to the rendered size.
            _fb.write(_col_ref, _row, _num, _dig, _h_align, &_wid, &_hgt);

            // Set _col based on alignment. Rendering started at:
            //   _col_ref for left alignment,
            //   _col_ref - (_wid / 2) for center alignment, or
            //   _col_ref - _wid for right alignment.
            if (_h_align == Framebuffer::HAlign::Left) {
                _col = _col_ref;
            } else if (_h_align == Framebuffer::HAlign::Center) {
                _col = _col_ref - _wid / 2;
            } else if (_h_align == Framebuffer::HAlign::Right) {
                _col = _col_ref - _wid;
            }
        }
    }

    void set_value(int n)
    {
        if (_num != n) {
            erase();
            _num = n;
            draw();
        }
    }

    int get_value() const
    {
        return _num;
    }

protected:

    const PixelImageHdr **_dig; // array of digit images
    int _num;                   // number to display

    // _col_ref is the original col passed in constructor
    // _col and _wid are changed in draw() and erase() based on alignment
    Framebuffer::HAlign _h_align;
    int _col_ref;

}; // class GuiNumber
