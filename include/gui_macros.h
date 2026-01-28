
#include "font.h"
#include "pixel_565.h"
#include "pixel_image.h"

// Create array of digit images 0-9 for use by GuiNumber
// Example usage:
// DIG_IMG_LIST(roboto_48, Color::black(), Color::white())

// Declare one digit image named (e.g.) 'roboto_48_img_0'
#define DIG_IMG_MAKE(DIG, FNT, FG, BG)                                \
    static constexpr PixelImage<Pixel565, FNT.width(#DIG), FNT.y_adv> \
        FNT##_img_##DIG =                                             \
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
            &FNT##_img_0.hdr, &FNT##_img_1.hdr, &FNT##_img_2.hdr, \
            &FNT##_img_3.hdr, &FNT##_img_4.hdr, &FNT##_img_5.hdr, \
            &FNT##_img_6.hdr, &FNT##_img_7.hdr, &FNT##_img_8.hdr, \
            &FNT##_img_9.hdr,                                     \
    }
