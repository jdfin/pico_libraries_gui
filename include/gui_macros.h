
#include "font.h"
#include "pixel_565.h"
#include "pixel_image.h"

//////////////////////////////////////////////////////////////////////////////
// GuiNumber Helpers
//////////////////////////////////////////////////////////////////////////////

// Create array of digit images 0-9 for use by GuiNumber
// Example usage:
// DIG_IMG_ARRAY(roboto_48, Color::black(), Color::white())

// Declare one digit image named (e.g.) 'roboto_48_0_img'
#define DIG_IMG_MAKE(DIG, FNT, FG, BG)                                \
    static constexpr PixelImage<Pixel565, FNT.width(#DIG), FNT.y_adv> \
        FNT##_##DIG##_img =                                             \
            label_img<Pixel565, FNT.width(#DIG), FNT.y_adv>(#DIG, FNT, FG, BG)

// Declare ten digit images and array of pointers to them
// Images are constexpr (flash), array is const (RAM)
#define DIGIT_IMAGE_ARRAY(FNT, FG, BG)                            \
    DIG_IMG_MAKE(0, FNT, FG, BG);                                 \
    DIG_IMG_MAKE(1, FNT, FG, BG);                                 \
    DIG_IMG_MAKE(2, FNT, FG, BG);                                 \
    DIG_IMG_MAKE(3, FNT, FG, BG);                                 \
    DIG_IMG_MAKE(4, FNT, FG, BG);                                 \
    DIG_IMG_MAKE(5, FNT, FG, BG);                                 \
    DIG_IMG_MAKE(6, FNT, FG, BG);                                 \
    DIG_IMG_MAKE(7, FNT, FG, BG);                                 \
    DIG_IMG_MAKE(8, FNT, FG, BG);                                 \
    DIG_IMG_MAKE(9, FNT, FG, BG);                                 \
    static const PixelImageHdr *FNT##_digit_img[10] =             \
        {                                                         \
            &FNT##_0_img.hdr, &FNT##_1_img.hdr, &FNT##_2_img.hdr, \
            &FNT##_3_img.hdr, &FNT##_4_img.hdr, &FNT##_5_img.hdr, \
            &FNT##_6_img.hdr, &FNT##_7_img.hdr, &FNT##_8_img.hdr, \
            &FNT##_9_img.hdr,                                     \
    }

//////////////////////////////////////////////////////////////////////////////
// GuiButton Helpers
//////////////////////////////////////////////////////////////////////////////

// Button with two images: one for enabled/disabled, and one for pressed

#define BUTTON_1(NAME, TXT, FB, COL, ROW, WID, HGT, BRD, FNT, FG, BG, UP_BG, \
                 DN_BG, CK_CB, CK_ARG, DN_CB, DN_ARG, UP_CB, UP_ARG, MODE,   \
                 PRESSED)                                                    \
                                                                             \
    static constexpr PixelImage<Pixel565, WID, HGT> NAME##_btn_up_img =      \
        label_img<Pixel565, WID, HGT>(TXT, FNT, FG, BRD, FG, UP_BG);         \
                                                                             \
    static constexpr PixelImage<Pixel565, WID, HGT> NAME##_btn_dn_img =      \
        label_img<Pixel565, WID, HGT>(TXT, FNT, FG, BRD, FG, DN_BG);         \
                                                                             \
    static GuiButton NAME##_btn(FB, COL, ROW, BG,                            \
                                &NAME##_btn_up_img.hdr, /* enabled */        \
                                &NAME##_btn_up_img.hdr, /* disabled */       \
                                &NAME##_btn_dn_img.hdr, /* pressed */        \
                                CK_CB, CK_ARG,          /* on_click */       \
                                DN_CB, DN_ARG,          /* on_down */        \
                                UP_CB, UP_ARG,          /* on_up */          \
                                MODE, PRESSED)
