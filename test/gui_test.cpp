
#include <cassert>
#include <cstdio>
// pico
#include "hardware/spi.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
// misc
#include "argv.h"
#include "str_ops.h"
#include "sys_led.h"
#include "util.h"
// framebuffer
#include "color.h"
#include "font.h"
#include "pixel_565.h"
#include "pixel_image.h"
#include "roboto.h"
#include "st7796.h"
// touchscreen
#include "gt911.h"
// gui
#include "gui_button.h"
#include "gui_label.h"
#include "gui_number.h"
#include "gui_page.h"
#include "gui_slider.h"

using HAlign = Framebuffer::HAlign;
using Rotation = Framebuffer::Rotation;

// Pico:
//              +------| USB |------+
//            1 | D0       VBUS_OUT | 40
//            2 | D1        VSYS_IO | 39
//            3 | GND           GND | 38
//            4 | D2         3V3_EN | 37
//            5 | D3        3V3_OUT | 36
// (ts) SDA   6 | D4           AREF | 35
// (ts) SCL   7 | D5            D28 | 34
//            8 | GND           GND | 33
// (ts) RST   9 | D6            D27 | 32
// (ts) INT  10 | D7            D26 | 31
//           11 | D8            RUN | 30
//           12 | D9            D22 | 29  LED  (fb)
//           13 | GND           GND | 28
//           14 | D10           D21 | 27  RST  (fb)
//           15 | D11           D20 | 26  CD   (fb)
//           16 | D12           D19 | 25  MOSI (fb)
//           17 | D13           D18 | 24  SCK  (fb)
//           18 | GND           GND | 23
//           19 | D14           D17 | 22  CS   (fb)
//           20 | D15           D16 | 21  MISO (fb)
//              +-------------------+

static const int spi_miso_pin = 16;
static const int spi_mosi_pin = 19;
static const int spi_clk_pin = 18;
static const int spi_cs_pin = 17;
static const uint32_t spi_baud_request = 15'000'000;
static uint32_t spi_baud_actual = 0;
static uint32_t spi_rate_max = 0;

static const int fb_cd_pin = 20;
static const int fb_rst_pin = 21;
static const int fb_led_pin = 22;

static const int i2c_sda_pin = 4;
static const int i2c_scl_pin = 5;
static const int tp_rst_pin = 6;
static const int tp_int_pin = 7;
static const uint i2c_baud_request = 400'000;
static uint i2c_baud_actual = 0;
static const uint8_t tp_i2c_addr = 0x14; // 0x14 or 0x5d

static const int work_bytes = 128;
static uint8_t work[work_bytes];

static St7796 fb(spi0, spi_miso_pin, spi_mosi_pin, spi_clk_pin, spi_cs_pin,
                 spi_baud_request, fb_cd_pin, fb_rst_pin, fb_led_pin, 480, 320,
                 work, work_bytes);

static I2cDev i2c_dev(i2c0, i2c_scl_pin, i2c_sda_pin, i2c_baud_request);

static Gt911 ts(i2c_dev, tp_i2c_addr, tp_rst_pin, tp_int_pin);

// clang-format off
namespace Label1 { static void run(); }
namespace Button1 { static void run(); }
namespace NavGroup1 { static void run(); }
namespace Events1 { static void run(); }
// clang-format on

static struct {
    const char *name;
    void (*func)();
} tests[] = {
    {"Label1", Label1::run},
    {"Button1", Button1::run},
    {"NavGroup1", NavGroup1::run},
    {"Events1", Events1::run},
};
static const int num_tests = sizeof(tests) / sizeof(tests[0]);


static void help()
{
    printf("\n");
    printf("Usage: enter test number (0..%d)\n", num_tests - 1);
    for (int i = 0; i < num_tests; i++)
        printf("%2d: %s\n", i, tests[i].name);
    printf("\n");
}


static void reinit_screen()
{
    // landscape, connector to the left
    fb.set_rotation(Rotation::landscape);

    fb.fill_rect(0, 0, fb.width(), fb.height(), Color::white());
}


int main()
{
    stdio_init_all();

    SysLed::init();
    SysLed::pattern(50, 950);

    while (!stdio_usb_connected()) {
        tight_loop_contents();
        SysLed::loop();
    }

    sleep_ms(10);

    SysLed::off();

    printf("\n");
    printf("gui_test\n");
    printf("\n");

    Argv argv(1); // verbosity == 1 means echo

    // initialize framebuffer

    spi_baud_actual = fb.spi_freq();
    spi_rate_max = spi_baud_actual / 8;
    printf("spi: requested %lu Hz, got %lu Hz (max %lu bytes/sec)\n", //
           spi_baud_request, spi_baud_actual, spi_rate_max);

    fb.init();
    printf("Framebuffer ready\n");

    // Turning on the backlight here shows whatever happens to be in RAM
    // (previously displayed or random junk), so we turn it on after filling
    // the screen with something.

    reinit_screen(); // set rotation, fill background

    // Now turn on backlight
    fb.brightness(100);

    // initialize touchscreen

    i2c_baud_actual = i2c_dev.baud();
    printf("i2c: requested %u Hz, got %u Hz\n", //
           i2c_baud_request, i2c_baud_actual);

    assert(ts.init());
    ts.set_rotation(Touchscreen::Rotation::landscape);
    printf("Touchscreen ready\n");

    help();
    printf("> ");

    while (true) {
        int c = stdio_getchar_timeout_us(0);
        if (0 <= c && c <= 255) {
            if (argv.add_char(char(c))) {
                int test_num = -1;
                if (argv.argc() != 1) {
                    printf("\n");
                    printf("One integer only (got %d)\n", argv.argc());
                    help();
                } else if (!str_to_int(argv[0], &test_num)) {
                    printf("\n");
                    printf("Invalid test number: \"%s\"\n", argv[0]);
                    help();
                } else if (test_num < 0 || test_num >= num_tests) {
                    printf("\n");
                    printf("Test number out of range: %d\n", test_num);
                    help();
                } else {
                    printf("\n");
                    printf("Running \"%s\"\n", tests[test_num].name);
                    printf("\n");
                    reinit_screen();
                    tests[test_num].func();
                    printf("> ");
                }
                argv.reset();
            }
        }
    }

    sleep_ms(100);
    return 0;
}


namespace Label1 {

static constexpr Font font = roboto_32;

static constexpr char txt_en[] = "Enabled";
static constexpr char txt_dis[] = "Disabled";

static constexpr int wid_en = font.width(txt_en);
static constexpr int wid_dis = font.width(txt_dis);
static constexpr int wid = (wid_en > wid_dis) ? wid_en : wid_dis;

static constexpr int hgt_en = font.y_adv;
static constexpr int hgt_dis = font.y_adv;
static constexpr int hgt = (hgt_en > hgt_dis) ? hgt_en : hgt_dis;

static constexpr Color fg_en = Color::black();
static constexpr Color fg_dis = Color::gray(75);

static constexpr Color bg_en = Color::white();
static constexpr Color bg_dis = Color::gray(25);

static constexpr PixelImage<Pixel565, wid, hgt> img_en =
    label_img<Pixel565, wid, hgt>(txt_en, font, fg_en, 0, fg_en, bg_en);

static constexpr PixelImage<Pixel565, wid, hgt> img_dis =
    label_img<Pixel565, wid, hgt>(txt_dis, font, fg_dis, 0, fg_dis, bg_dis);

static void run()
{
    int col = (fb.width() - wid) / 2;
    int row = (fb.height() - hgt) / 2;

    GuiLabel label(fb, col, row, bg_en,
                   reinterpret_cast<const PixelImageInfo *>(&img_en),
                   reinterpret_cast<const PixelImageInfo *>(&img_dis));
    label.draw();
    for (int i = 0; i < 5; i++) {
        sleep_ms(1000);
        label.enabled(false);
        //label.draw();
        sleep_ms(1000);
        label.enabled(true);
        //label.draw();
    }
}

} // namespace Label1


namespace Button1 {

static constexpr Font font = roboto_32;

static constexpr char txt_en[] = "Enabled";
static constexpr char txt_dis[] = "Disabled";

static constexpr int wid_en = font.width(txt_en);
static constexpr int wid_dis = font.width(txt_dis);
static constexpr int wid = ((wid_en > wid_dis) ? wid_en : wid_dis) * 2;

static constexpr int hgt_en = font.y_adv;
static constexpr int hgt_dis = font.y_adv;
static constexpr int hgt = ((hgt_en > hgt_dis) ? hgt_en : hgt_dis) * 2;

static constexpr Color fg = Color::black();

static constexpr Color bg_en = Color::white();
static constexpr Color bg_dis = Color::gray(80);

static constexpr int brd_thk = 4;
static constexpr Color brd_clr = fg;

static constexpr PixelImage<Pixel565, wid, hgt> img_en =
    label_img<Pixel565, wid, hgt>(txt_en, font, fg, brd_thk, brd_clr, bg_en);

static constexpr PixelImage<Pixel565, wid, hgt> img_dis =
    label_img<Pixel565, wid, hgt>(txt_dis, font, fg, brd_thk, brd_clr, bg_dis);

static void run()
{
    int col = (fb.width() - wid) / 2;
    int row = (fb.height() - hgt) / 2;

    GuiLabel label(fb, col, row, bg_en,
                   reinterpret_cast<const PixelImageInfo *>(&img_en),
                   reinterpret_cast<const PixelImageInfo *>(&img_dis));
    label.draw();
    for (int i = 0; i < 5; i++) {
        sleep_ms(1000);
        label.enabled(false);
        //label.draw();
        sleep_ms(1000);
        label.enabled(true);
        //label.draw();
    }
}

} // namespace Button1


namespace NavGroup1 {

static constexpr Font font = roboto_32;

static constexpr Color screen_fg = Color::black();
static constexpr Color screen_bg = Color::white();

static constexpr int page_cnt = 3;

static void nav_click(int page_num);

///// Use preprocessor to create an image for each digit 0-9.

#define IMG_MAKE(N, F, FG, BG)                                                \
    static constexpr PixelImage<Pixel565, F.width(#N), F.y_adv> F##_img_##N = \
        label_img<Pixel565, F.width(#N), F.y_adv>(#N, F, FG, BG);

// clang-format off
#define IMG_LIST(F, FG, BG) IMG_MAKE(0, F, FG, BG) IMG_MAKE(1, F, FG, BG) \
                            IMG_MAKE(2, F, FG, BG) IMG_MAKE(3, F, FG, BG) \
                            IMG_MAKE(4, F, FG, BG) IMG_MAKE(5, F, FG, BG) \
                            IMG_MAKE(6, F, FG, BG) IMG_MAKE(7, F, FG, BG) \
                            IMG_MAKE(8, F, FG, BG) IMG_MAKE(9, F, FG, BG)
// clang-format on

// Expand to create all ten images
IMG_LIST(roboto_48, screen_fg, screen_bg)

// Clean up macros
#undef IMG_MAKE
#undef IMG_LIST

static const PixelImageInfo *roboto_48_digit_img[10] = {
    (PixelImageInfo *)&roboto_48_img_0, (PixelImageInfo *)&roboto_48_img_1,
    (PixelImageInfo *)&roboto_48_img_2, (PixelImageInfo *)&roboto_48_img_3,
    (PixelImageInfo *)&roboto_48_img_4, (PixelImageInfo *)&roboto_48_img_5,
    (PixelImageInfo *)&roboto_48_img_6, (PixelImageInfo *)&roboto_48_img_7,
    (PixelImageInfo *)&roboto_48_img_8, (PixelImageInfo *)&roboto_48_img_9,
};

/////

#define GUI_LABEL(FB, VAR, TXT, FNT, WID, HGT, COL, ROW, FG, BG)              \
    static constexpr int VAR##_wid = (WID != 0 ? WID : FNT.width(TXT));       \
    static constexpr int VAR##_hgt = (HGT != 0 ? HGT : FNT.y_adv);            \
    static constexpr PixelImage<Pixel565, VAR##_wid, VAR##_hgt> VAR##_img =   \
        label_img<Pixel565, VAR##_wid, VAR##_hgt>(TXT, FNT, FG, 0,            \
                                                  Color::none(), BG);         \
    static GuiLabel VAR(FB, (COL), (ROW), BG,                                 \
                        reinterpret_cast<const PixelImageInfo *>(&VAR##_img), \
                        reinterpret_cast<const PixelImageInfo *>(&VAR##_img))

/////

// page 0

GUI_LABEL(fb, l0a, "Label 0A", font, 0, 0, 100, 100, screen_fg, screen_bg);
GUI_LABEL(fb, l0b, "Label 0B", font, 0, 0, 100, (100 + l0a_hgt + 10), screen_fg,
          screen_bg);

static GuiPage page_0({&l0a, &l0b});

// page 1

GUI_LABEL(fb, l1a, "Label 1A", font, 0, 0, 200, 100, screen_fg, screen_bg);
GUI_LABEL(fb, l1b, "Label 1B", font, 0, 0, 200, (100 + l1a_hgt + 10), screen_fg,
          screen_bg);

static GuiPage page_1({&l1a, &l1b});

// page 2

// numbers to the left of this, sliders to the right
static int s_align = fb.width() / 2;

static const int s_marg = 1;
static const int s_wid = fb.width() - s_align - s_marg;
static const int s_hgt = 40;

static const Color s_fill = Color::gray(90);

static const int row_a = 40;
static const int row_b = row_a + s_hgt + 20;
static const int row_c = row_b + s_hgt + 20;
static const int row_d = row_c + s_hgt + 20;

static GuiNumber n2a(fb, s_align, row_a, screen_bg,                        //
                     reinterpret_cast<const void **>(roboto_48_digit_img), //
                     0, HAlign::Right);

static void s2a_value(intptr_t);

static GuiSlider s2a(fb, s_align, row_a, s_wid, s_hgt,             //
                     screen_fg, screen_bg, s_fill, Color::white(), //
                     0, 10, 0, s2a_value, 0);

static void s2a_value(intptr_t)
{
    int val = s2a.get_value();
    n2a.set_value(val);
    n2a.draw();
}

static GuiNumber n2b(fb, s_align, row_b, screen_bg,                        //
                     reinterpret_cast<const void **>(roboto_48_digit_img), //
                     0, HAlign::Left);

static void s2b_value(intptr_t);

static GuiSlider s2b(fb, s_align - s_wid, row_b, s_wid, s_hgt,     //
                     screen_fg, screen_bg, s_fill, Color::white(), //
                     1000, 5000, 0, s2b_value, 0);

static void s2b_value(intptr_t)
{
    int val = s2b.get_value();
    n2b.set_value(val);
}

static GuiNumber n2c(fb, s_align, row_c, screen_bg,                        //
                     reinterpret_cast<const void **>(roboto_48_digit_img), //
                     0, HAlign::Center);

static void s2c_value(intptr_t);

static GuiSlider s2c(fb, s_align - s_wid / 2, row_d, s_wid, s_hgt, //
                     screen_fg, screen_bg, s_fill, Color::white(), //
                     0, 127, 0, s2c_value, 0);

static void s2c_value(intptr_t)
{
    int val = s2c.get_value();
    n2c.set_value(val);
    n2c.draw();
}

static GuiPage page_2({&n2a, &s2a, &n2b, &s2b, &n2c, &s2c});

/////

static GuiPage *pages[] = {&page_0, &page_1, &page_2};

static int active_page = -1;

///// nav bar

// fb.width() is not constexpr
static constexpr int fb_width = 480;

static constexpr int nav_cnt = 3;
static constexpr Font nav_font = roboto_24;

static constexpr Color nav_bg_ena = Color::gray(80);
static constexpr Color nav_bg_dis = screen_bg;
static constexpr Color nav_bg_prs = Color::red();

static constexpr int nav_brd_thk_ena = 4;
static constexpr int nav_brd_thk_dis = 1;
static constexpr int nav_brd_thk_prs = 6;
static constexpr int nav_brd_thk_max = 6;

static constexpr int nav_hgt = nav_font.y_adv + 2 * nav_brd_thk_max;
static constexpr int nav_wid = fb_width / nav_cnt;

// clang-format off
#define NAV_BUTTON(N, TXT) \
    static constexpr PixelImage<Pixel565, nav_wid, nav_hgt> b##N##_img_ena = \
        label_img<Pixel565, nav_wid, nav_hgt>(TXT, nav_font, screen_fg, \
                                              nav_brd_thk_ena, screen_fg, \
                                              nav_bg_ena); \
    \
    static constexpr PixelImage<Pixel565, nav_wid, nav_hgt> b##N##_img_dis = \
        label_img<Pixel565, nav_wid, nav_hgt>(TXT, nav_font, screen_fg, \
                                              nav_brd_thk_dis, screen_fg, \
                                              nav_bg_dis); \
    \
    static constexpr PixelImage<Pixel565, nav_wid, nav_hgt> b##N##_img_prs = \
        label_img<Pixel565, nav_wid, nav_hgt>(TXT, nav_font, screen_fg, \
                                              nav_brd_thk_prs, screen_fg, \
                                              nav_bg_prs); \
    \
    static GuiButton nav_##N(fb, N * fb_width / nav_cnt, 0, screen_bg, \
        reinterpret_cast<const PixelImageInfo *>(&b##N##_img_ena), \
        reinterpret_cast<const PixelImageInfo *>(&b##N##_img_dis), \
        reinterpret_cast<const PixelImageInfo *>(&b##N##_img_prs), \
        nav_click, N);
// clang-format on

NAV_BUTTON(0, "PAGE 0")
NAV_BUTTON(1, "PAGE 1")
NAV_BUTTON(2, "PAGE 2")

static GuiButton *navs[] = {&nav_0, &nav_1, &nav_2};

/////

static void show_page(int page_num)
{
    navs[page_num]->enabled(false);
    pages[page_num]->visible(true);
}

static void hide_page(int page_num, bool force_draw = false)
{
    navs[page_num]->enabled(true, force_draw);
    pages[page_num]->visible(false);
}

static void nav_click(int page_num)
{
    assert(0 <= page_num && page_num < page_cnt);

    if (active_page == -1) {
        // first call only, hide them all and force redraw
        for (int p = 0; p < page_cnt; p++)
            hide_page(p, true);
    } else {
        // old 'active_page' is becoming inactive - hide it
        hide_page(active_page);
    }

    // switch pages
    active_page = page_num;

    // new 'active_page' is becoming active - show it
    show_page(active_page);
}

/////

static void run()
{
    printf("(press any key to stop)\n");

    nav_click(0); // start out on page 0

    while (true) {

        int c = stdio_getchar_timeout_us(0);
        if (0 <= c && c <= 255)
            break;

        Touchscreen::Event event(ts.get_event());
        if (event.type == Touchscreen::Event::Type::none)
            continue;

        //printf("Event: %s at (%d, %d)\n", //
        //event.type_name(), event.col, event.row);

        // anyone have focus?
        if (GuiWidget::focus != nullptr) {
            // yes, send event there
            GuiWidget::focus->event(event);
        } else {
            // no, see if anyone wants it
            bool handled = false;
            // nav buttons?
            for (int p = 0; p < page_cnt && !handled; p++)
                handled = navs[p]->event(event);
            // anyone on current page want it?
            if (!handled)
                pages[active_page]->event(event);
        }
    }

    printf("\n");
}

} // namespace NavGroup1


namespace Events1 {

static void run()
{
    printf("(press any key to stop)\n");

    while (true) {
        Touchscreen::Event event(ts.get_event());
        if (event.type != Touchscreen::Event::Type::none) {
            printf("Event: %s at (%d, %d)\n", //
                   event.type_name(), event.col, event.row);
        }
        int c = stdio_getchar_timeout_us(0);
        if (0 <= c && c <= 255)
            break;
    }

    printf("\n");
}

} // namespace Events1
