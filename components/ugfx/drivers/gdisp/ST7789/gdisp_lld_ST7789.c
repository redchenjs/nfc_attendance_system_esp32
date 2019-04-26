#include "gfx.h"

#if GFX_USE_GDISP

#if defined(GDISP_SCREEN_HEIGHT) || defined(GDISP_SCREEN_HEIGHT)
    #if GFX_COMPILER_WARNING_TYPE == GFX_COMPILER_WARNING_DIRECT
        #warning "GDISP: This low level driver does not support setting a screen size. It is being ignored."
    #elif GFX_COMPILER_WARNING_TYPE == GFX_COMPILER_WARNING_MACRO
        COMPILER_WARNING("GDISP: This low level driver does not support setting a screen size. It is being ignored.")
    #endif
    #undef GDISP_SCREEN_WIDTH
    #undef GDISP_SCREEN_HEIGHT
#endif

#define GDISP_DRIVER_VMT            GDISPVMT_ST7789
#include "gdisp_lld_config.h"
#include "../../../src/gdisp/gdisp_driver.h"

#include "gdisp_lld_board.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifndef GDISP_SCREEN_HEIGHT
    #define GDISP_SCREEN_HEIGHT     ST7789_SCREEN_HEIGHT
#endif
#ifndef GDISP_SCREEN_WIDTH
    #define GDISP_SCREEN_WIDTH      ST7789_SCREEN_WIDTH
#endif
#ifndef GDISP_INITIAL_CONTRAST
    #define GDISP_INITIAL_CONTRAST  100
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
    #define GDISP_INITIAL_BACKLIGHT 100
#endif

#define GDISP_FLG_NEEDFLUSH         (GDISP_FLG_DRIVER<<0)

#include "ST7789.h"

// Some common routines and macros
#define write_reg(g, reg, data)     { write_cmd(g, reg); write_data(g, data); }

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
    g->priv = gfxAlloc(GDISP_SCREEN_HEIGHT * GDISP_SCREEN_WIDTH * 2);
    if (g->priv == NULL) {
        gfxHalt("GDISP ST7789: Failed to allocate private memory");
    }

    for (int i=0; i<GDISP_SCREEN_HEIGHT*GDISP_SCREEN_WIDTH*2; i++) {
        *((uint8_t *)g->priv + i) = 0x00;
    }

    // Initialise the board interface
    init_board(g);

    // Hardware reset
    setpin_reset(g, 0);
    gfxSleepMilliseconds(120);
    setpin_reset(g, 1);
    gfxSleepMilliseconds(120);

    write_cmd(g, ST7789_SWRESET);
    gfxSleepMilliseconds(120);
    write_cmd(g, ST7789_SLPOUT);
    gfxSleepMilliseconds(120);
    write_cmd(g, ST7789_PORCTRL);
        write_data(g, 0x0C);
        write_data(g, 0x0C);
        write_data(g, 0x00);
        write_data(g, 0x33);
        write_data(g, 0x33);
    write_cmd(g, ST7789_GCTRL);
        write_data(g, 0x35);
    write_cmd(g, ST7789_VCOMS);
        write_data(g, 0x19);
    write_cmd(g, ST7789_LCMCTRL);
        write_data(g, 0x2C);
    write_cmd(g, ST7789_VDVVRHEN);
        write_data(g, 0x01);
    write_cmd(g, ST7789_VRHS);
        write_data(g, 0x12);
    write_cmd(g, ST7789_VDVSET);
        write_data(g, 0x20);
    write_cmd(g, ST7789_FRCTR2);
        write_data(g, 0x0F);
    write_cmd(g, ST7789_PWCTRL1);
        write_data(g, 0xA4);
        write_data(g, 0xA1);
    write_cmd(g, ST7789_INVON);
    write_cmd(g, ST7789_MADCTL);
        write_data(g, 0x00);
    write_cmd(g, ST7789_COLMOD);
        write_data(g, 0x05);
    write_cmd(g, ST7789_PVGAMCTRL);
        write_data(g, 0x04);
        write_data(g, 0x0D);
        write_data(g, 0x11);
        write_data(g, 0x13);
        write_data(g, 0x2B);
        write_data(g, 0x3F);
        write_data(g, 0x54);
        write_data(g, 0x4C);
        write_data(g, 0x18);
        write_data(g, 0x0D);
        write_data(g, 0x0B);
        write_data(g, 0x1F);
        write_data(g, 0x23);
    write_cmd(g, ST7789_NVGAMCTRL);
        write_data(g, 0xD0);
        write_data(g, 0x04);
        write_data(g, 0x0C);
        write_data(g, 0x11);
        write_data(g, 0x13);
        write_data(g, 0x2C);
        write_data(g, 0x3F);
        write_data(g, 0x44);
        write_data(g, 0x51);
        write_data(g, 0x2F);
        write_data(g, 0x1F);
        write_data(g, 0x1F);
        write_data(g, 0x20);
        write_data(g, 0x23);
    write_cmd(g, ST7789_NORON);
    write_cmd(g, ST7789_DISPON);

    /* Initialise the GDISP structure */
    g->g.Width  = GDISP_SCREEN_HEIGHT;
    g->g.Height = GDISP_SCREEN_WIDTH;
    g->g.Orientation = GDISP_ROTATE_0;
    g->g.Powermode = powerOn;
    g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
    g->g.Contrast  = GDISP_INITIAL_CONTRAST;
    return TRUE;
}

#if GDISP_HARDWARE_FLUSH
    LLDSPEC void gdisp_lld_flush(GDisplay *g) {
        if (!(g->flags & GDISP_FLG_NEEDFLUSH)) {
            return;
        }
        refresh_gram(g, (uint8_t *)g->priv);
        g->flags &= ~GDISP_FLG_NEEDFLUSH;
    }
#endif

#if GDISP_HARDWARE_STREAM_WRITE
    static int16_t stream_write_x  = 0;
    static int16_t stream_write_cx = 0;
    static int16_t stream_write_y  = 0;
    static int16_t stream_write_cy = 0;
    LLDSPEC    void gdisp_lld_write_start(GDisplay *g) {
        stream_write_x  = g->p.x;
        stream_write_cx = g->p.cx;
        stream_write_y  = g->p.y;
        stream_write_cy = g->p.cy;
    }
    LLDSPEC    void gdisp_lld_write_color(GDisplay *g) {
        LLDCOLOR_TYPE c = gdispColor2Native(g->p.color);
        *((uint8_t *)g->priv + (stream_write_x + stream_write_y * g->g.Width) * 2 + 0) = c >> 8;
        *((uint8_t *)g->priv + (stream_write_x + stream_write_y * g->g.Width) * 2 + 1) = c;
        stream_write_x++;
        if (--stream_write_cx <= 0) {
            stream_write_x  = g->p.x;
            stream_write_cx = g->p.cx;
            stream_write_y++;
            if (--stream_write_cy <= 0) {
                stream_write_y  = g->p.y;
                stream_write_cy = g->p.cy;
            }
        }
    }
    LLDSPEC    void gdisp_lld_write_stop(GDisplay *g) {
        stream_write_x  = 0;
        stream_write_cx = 0;
        stream_write_y  = 0;
        stream_write_cy = 0;
        g->flags |= GDISP_FLG_NEEDFLUSH;
    }
#endif

#if GDISP_HARDWARE_STREAM_READ
    static int16_t stream_read_x  = 0;
    static int16_t stream_read_cx = 0;
    static int16_t stream_read_y  = 0;
    static int16_t stream_read_cy = 0;
    LLDSPEC    void gdisp_lld_read_start(GDisplay *g) {
        stream_read_x  = g->p.x;
        stream_read_cx = g->p.cx;
        stream_read_y  = g->p.y;
        stream_read_cy = g->p.cy;
    }
    LLDSPEC    color_t gdisp_lld_read_color(GDisplay *g) {
        LLDCOLOR_TYPE c = (*((uint8_t *)g->priv + (stream_read_x + stream_read_y * g->g.Width) * 2 + 0) << 8)
                        | (*((uint8_t *)g->priv + (stream_read_x + stream_read_y * g->g.Width) * 2 + 1));
        stream_read_x++;
        if (--stream_read_cx <= 0) {
            stream_read_x  = g->p.x;
            stream_read_cx = g->p.cx;
            stream_read_y++;
            if (--stream_read_cy <= 0) {
                stream_read_y  = g->p.y;
                stream_read_cy = g->p.cy;
            }
        }
        return c;
    }
    LLDSPEC    void gdisp_lld_read_stop(GDisplay *g) {
        stream_read_x  = 0;
        stream_read_cx = 0;
        stream_read_y  = 0;
        stream_read_cy = 0;
    }
#endif

#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
LLDSPEC void gdisp_lld_control(GDisplay *g) {
    switch(g->p.x) {
    case GDISP_CONTROL_POWER:
        if (g->g.Powermode == (powermode_t)g->p.ptr)
            return;
        switch((powermode_t)g->p.ptr) {
            case powerOff:
            case powerSleep:
            case powerDeepSleep:
            case powerOn:
            default:
                return;
        }
        g->g.Powermode = (powermode_t)g->p.ptr;
        return;

    case GDISP_CONTROL_ORIENTATION:
        if (g->g.Orientation == (orientation_t)g->p.ptr)
            return;
        switch((orientation_t)g->p.ptr) {
            case GDISP_ROTATE_0:
                g->g.Height = GDISP_SCREEN_HEIGHT;
                g->g.Width  = GDISP_SCREEN_WIDTH;
                break;
            case GDISP_ROTATE_90:
                g->g.Height = GDISP_SCREEN_WIDTH;
                g->g.Width  = GDISP_SCREEN_HEIGHT;
                break;
            case GDISP_ROTATE_180:
                g->g.Height = GDISP_SCREEN_HEIGHT;
                g->g.Width  = GDISP_SCREEN_WIDTH;
                break;
            case GDISP_ROTATE_270:
                g->g.Height = GDISP_SCREEN_WIDTH;
                g->g.Width  = GDISP_SCREEN_HEIGHT;
                break;
            default:
                return;
        }
        g->g.Orientation = (orientation_t)g->p.ptr;
        return;
    case GDISP_CONTROL_BACKLIGHT:
        if ((unsigned)g->p.ptr > 100)
            g->p.ptr = (void *)100;
        g->g.Backlight = (unsigned)g->p.ptr;
        return;
    default:
        return;
    }
}
#endif

#endif /* GFX_USE_GDISP */
