//
// NekoInk Image Viewer
// Copyright 2021 Wenting Zhang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// File : disp.c
// Brief: Hardware display related functions
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "config.h"
#include "disp.h"
#include "stb_image_resize.h"
#include "stb_image.h"

#include <SDL.h>

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static Canvas *screen;

static int disp_get_bpp(PixelFormat fmt) {
    switch(fmt) {
    case PIXFMT_Y1_PACKED:
        return 1;
    case PIXFMT_Y2_PACKED:
        return 2;
    case PIXFMT_Y4_PACKED:
        return 4;
    case PIXFMT_Y8:
    case PIXFMT_Y1_LSB:
    case PIXFMT_Y2_LSB:
    case PIXFMT_Y4_LSB:
    case PIXFMT_C1_LSB:
    case PIXFMT_C2_LSB:
    case PIXFMT_C4_LSB:
    case PIXFMT_C8:
        return 8;
    case PIXFMT_RGB565:
    case PIXFMT_RGB565_BE:
        return 16;
    case PIXFMT_RGB888:
        return 24;
    case PIXFMT_ARGB8888:
    case PIXFMT_ARGB8888_BE:
    case PIXFMT_RGBA8888:
    case PIXFMT_RGBA8888_BE:
        return 32;
    }
    return 0; // Unknown
}

static uint8_t disp_get_mask(PixelFormat fmt) {
    switch(fmt) {
    case PIXFMT_Y1_PACKED:
        return 0x01;
    case PIXFMT_Y2_PACKED:
        return 0x03;
    case PIXFMT_Y4_PACKED:
        return 0x0f;
    default:
        return 0x00;
    }
    return 0x00; // Unnecessary, but gcc gives a warning if I don't do so
}

// Framebuffer operation
Canvas *disp_create(int w, int h, PixelFormat fmt) {
    size_t size = w * h;
    int bpp = disp_get_bpp(fmt);
    if (bpp >= 8) {
        size *= (bpp / 8);
    }
    else {
        size /= (8 / bpp);
    }
    Canvas *canvas = malloc(sizeof(Canvas) + size);
    canvas->width = w;
    canvas->height = h;
    canvas->pixelFormat = fmt;
    return canvas;
}

void disp_free(Canvas *canvas) {
    free(canvas);
}

uint32_t disp_conv_pix(PixelFormat dst, PixelFormat src, uint32_t color) {
    int r = 0x00, g = 0x00, b = 0x00, a = 0xff;
    uint8_t y = (uint8_t)color;

    if (src == dst)
        return color;

    switch (src) {
    case PIXFMT_Y1_LSB:
        y = (color) ? 0xff : 0x00;
        r = g = b = y;
        break;
    case PIXFMT_Y2_LSB:
        y |= y << 2; __attribute__ ((fallthrough));
    case PIXFMT_Y4_LSB:
        y |= y << 4; __attribute__ ((fallthrough));
    case PIXFMT_Y8:
        r = g = b = y;
        break;
    case PIXFMT_RGB565:
        r = (color >> 8) & 0xf8;
        g = (color >> 3) & 0xfc;
        b = (color << 3) & 0xf8;
        r |= r >> 5;
        g |= g >> 6;
        b |= b >> 5;
        break;
    case PIXFMT_ARGB8888:
        a = (color >> 24) & 0xff; __attribute__ ((fallthrough));
    case PIXFMT_RGB888:
        r = (color >> 16) & 0xff;
        g = (color >> 8) & 0xff;
        b = (color) & 0xff;
        break;
    case PIXFMT_RGBA8888:
        r = (color >> 24) & 0xff;
        g = (color >> 16) & 0xff;
        b = (color >> 8) & 0xff;
        a = (color) & 0xff;
        break;
    case PIXFMT_ARGB8888_BE:
        b = (color >> 24) & 0xff;
        g = (color >> 16) & 0xff;
        r = (color >> 8) & 0xff;
        a = (color) & 0xff;
        break;
    case PIXFMT_RGBA8888_BE:
        a = (color >> 24) & 0xff;
        b = (color >> 16) & 0xff;
        g = (color >> 8) & 0xff;
        r = (color) & 0xff;
        break;
    default:
        assert(0);
    }

    y = (uint8_t)(r * 0.312f + g * 0.563f + b * 0.125f);
    uint32_t target = 0;
    switch (dst) {
    case PIXFMT_Y1_LSB:
        target = (y >> 7) & 0x1;
        break;
    case PIXFMT_Y2_LSB:
        target = (y >> 6) & 0x3;
        break;
    case PIXFMT_Y4_LSB:
        target = (y >> 4) & 0xf;
        break;
    case PIXFMT_Y8:
        target = y;
        break;
    case PIXFMT_RGB565:
        target = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3);
        break;
    case PIXFMT_ARGB8888:
        target = (a << 24) | (r << 16) | (g << 8) | b;
        break;
    case PIXFMT_RGB888:
        target = (r << 16) | (g << 8) | b;
        break;
    case PIXFMT_RGBA8888:
        target = (r << 24) | (g << 16) | (b << 8) | a;
        break;
    default:
        assert(0);
    }
    return target;
}

// Caution: really slow
void disp_conv(Canvas *dst, Canvas *src) {
    uint8_t *src_raw8 = (uint8_t *)src->buf;
    uint16_t *src_raw16 = (uint16_t *)src->buf;
    uint32_t *src_raw32 = (uint32_t *)src->buf;
    uint8_t *dst_raw8 = (uint8_t *)dst->buf;
    uint16_t *dst_raw16 = (uint16_t *)dst->buf;
    uint32_t *dst_raw32 = (uint32_t *)dst->buf;

    for (int i = 0; i < (src->width * src->height); i++) {
        int bpp = disp_get_bpp(src->pixelFormat);
        uint32_t color;
        if (bpp == 8) {
            color = *src_raw8++;
        }
        else if (bpp == 16) {
            color = *src_raw16++;
        }
        else if (bpp == 24) {
            color = *src_raw8++;
            color <<= 8;
            color |= *src_raw8++;
            color <<= 8;
            color |= *src_raw8++;
        }
        else if (bpp == 32) {
            color = *src_raw32++;
        }
        else {
            assert(0);
        }
        color = disp_conv_pix(dst->pixelFormat, src->pixelFormat, color);
        bpp = disp_get_bpp(dst->pixelFormat);
        if (bpp == 8) {
            *dst_raw8++ = color;
        }
        else if (bpp == 16) {
            *dst_raw16++ = color;
        }
        else if (bpp == 24) {
            *dst_raw8++ = (color >> 16) & 0xff;
            *dst_raw8++ = (color >> 8) & 0xff;
            *dst_raw8++ = (color) & 0xff;
        }
        else if (bpp == 32) {
            *dst_raw32++ = color;
        }
        else {
            assert(0);
        }
    }
}

void disp_scale_image_fit(Canvas *src, Canvas *dst) {
    if ((dst->height == src->height) && (dst->width == src->width)) {
        memcpy(dst->buf, src->buf,
                disp_get_bpp(dst->pixelFormat) / 8 * dst->height * dst->width);
        return;
    }

    float scalex, scaley;
    scalex = (float)dst->width / (float)src->width;
    scaley = (float)dst->height / (float)src->height;
    int channels = disp_get_bpp(src->pixelFormat);
    // Source and dest should have the same pixel format
    assert(channels == disp_get_bpp(dst->pixelFormat));
    // The pixel format should not be packed
    assert(channels >= 8);
    channels /= 8;
    // Assume byte per pixel is equal to channel count
    // Fit the image into destination size keeping aspect ratio
    int outh, outw, outstride, outoffset;
    if (scalex > scaley) {
        outh = dst->height;
        outw = src->width * scaley;
        outoffset = ((dst->width - outw) / 2) * channels;
        outstride = dst->width * channels;
    }
    else {
        outw = dst->width;
        outh = src->height * scalex;
        outoffset = ((dst->height - outh) / 2) * channels * dst->width;
        outstride = 0;
    }
    stbir_resize_uint8(src->buf, src->width, src->height, 0,
            dst->buf + outoffset, outw, outh, outstride, channels);
}

// For a certain pixel, get its color component on the EPD screen,
// return in the RSH amount to get the component
static uint32_t get_panel_color_shift(int x, int y) {
    int c = (x + y % 2) % 3;
    if (c == 0)
        return 16; // r
    else if (c == 1)
        return 8; // g
    else
        return 0; // b
}

static uint32_t get_panel_color_component(int x, int y) {
    int c = (x + y % 2) % 3;
    if (c == 0)
        return 0; // r
    else if (c == 1)
        return 1; // g
    else
        return 2; // b
}

uint8_t add_saturate(uint8_t a, int8_t b) {
    int32_t val = (int32_t)a + (int32_t)b;
    if (val > 255) return 255;
    if (val < 0) return 0;
    return val;
}

// R G B R G B
//  B R G B R
// R G B R G B

// Process image to be displayed on EPD
void disp_filtering_image(Canvas *src, Rect src_rect, Rect dst_rect) {
    uint8_t *src_raw = (uint8_t *)src->buf;
    uint32_t *dst_raw = (uint32_t *)screen->buf;
    uint32_t dst_w = screen->width;
    uint32_t src_x = src_rect.x;
    uint32_t src_y = src_rect.y;
    uint32_t w = src_rect.w;
    uint32_t h = src_rect.h;
    uint32_t dst_x = dst_rect.x;
    uint32_t dst_y = dst_rect.y;
    if ((w == 0) && (h == 0)) {
        w = src->width;
        h = src->height;
    }

    assert(src->pixelFormat == PIXFMT_RGB888);
#define SRC_PIX(x, y, comp) src_raw[((src_y + y) * w + src_x + x) * 3 + comp]
#define DST_PIX(x, y) dst_raw[(dst_y + y) * dst_w + dst_x + x]

    printf("const uint8_t image[] = {\n   ");
    int output_counter = 0;

    // Convert to 8bpp in target buffer
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint32_t pix;
            uint32_t comp = get_panel_color_component(dst_x + x, dst_y + y);
            pix = SRC_PIX(x, y, comp);

            if (y % 2 == 0) {
                // Input is skewed by 0.5 px
                uint32_t pix_l = pix;
                uint32_t pix_r = (x == w - 1) ? (pix_l) : (SRC_PIX(x + 1, y, comp));
                pix = (pix_l + pix_r) / 2;
            }

    #ifdef ENABLE_LPF
            // Low pass filtering to reduce the color/ jagged egdes
            uint32_t pix_ul = (y == 0) ? pix : SRC_PIX(x, y - 1, comp);
            uint32_t pix_ur = (y == 0) ? pix : ((x == (w - 1)) ? pix : SRC_PIX(x + 1, y - 1, comp));
            uint32_t pix_dl = (y == (h - 1)) ? pix : SRC_PIX(x, y + 1, comp);
            uint32_t pix_dr = (y == (h - 1)) ? pix : ((x == (w - 1)) ? pix : SRC_PIX(x + 1, y + 1, comp));
            uint32_t pix_l = (x == 0) ? pix : SRC_PIX(x - 1, y, comp);
            uint32_t pix_r = (x == (w - 1)) ? pix : SRC_PIX(x + 1, y, comp);
            pix = pix * 10; // 10/16
            pix_ul = pix_ul; // 1/16
            pix_ur = pix_ur; // 1/16
            pix_dl = pix_dl;
            pix_dr = pix_dr;
            pix_l = pix_l;
            pix_r = pix_r;
            pix = (pix + pix_ul + pix_ur + pix_dl + pix_dr + pix_l + pix_r) / 16;
    #endif

            printf(" 0x%02x,", pix);
            output_counter++;
            if (output_counter == 16) {
                output_counter = 0;
                printf("\n   ");
            }

            uint32_t shift = get_panel_color_shift(dst_x + x, dst_y + y);
            pix <<= shift;
            pix |= 0xff000000;
            int xx = x * 2, yy = y * 2;
            if (y % 2 == 0) xx++;
            DST_PIX(xx, yy) = pix;
            DST_PIX(xx + 1, yy) = pix;
            DST_PIX(xx, yy + 1) = pix;
            DST_PIX(xx + 1, yy + 1) = pix;
        }
    }

    printf("\n};\n");

    uint32_t *texture_pixels;
    int texture_pitch;
    SDL_LockTexture(texture, NULL, (void **)&texture_pixels, &texture_pitch);
    assert(texture_pitch == (screen->width * 4));
    memcpy(texture_pixels, screen->buf, screen->height * texture_pitch);
    SDL_UnlockTexture(texture);

}

void dst_pix_l(uint32_t *buf, int w, int x, int y, uint32_t p) {
    buf[y * w + x] = p;
    buf[y * w + x + 1] = p;
    buf[(y + 1) * w + x] = p;
    buf[(y + 1) * w + x + 1] = p;
}

void disp_filtering_image_b(Canvas *src) {
    uint8_t *src_raw = (uint8_t *)src->buf;
    uint32_t *dst_raw = (uint32_t *)screen->buf;
    uint32_t dst_w = screen->width;
    uint32_t src_x = 0;
    uint32_t src_y = 0;
    uint32_t w = src->width;
    uint32_t h = src->height;
    uint32_t dst_x = 0;
    uint32_t dst_y = 0;

    assert(src->pixelFormat == PIXFMT_RGB888);
#define SRC_PIX(x, y, comp) src_raw[((src_y + y) * w + src_x + x) * 3 + comp]
    int output_counter = 0;

    // Convert to 8bpp in target buffer
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint32_t r, g, b;
            r = SRC_PIX(x, y, 0);
            g = SRC_PIX(x, y, 1);
            b = SRC_PIX(x, y, 2);

            r = (r << 16) | 0xff000000;
            g = (g << 8) | 0xff000000;
            b = (b << 0) | 0xff000000;

            int xx = x * 3;
            int yy = y * 4;
            if (x % 2 == 0) {
                dst_pix_l(dst_raw, dst_w, xx + 1, yy, r);
                dst_pix_l(dst_raw, dst_w, xx, yy + 2, g);
                dst_pix_l(dst_raw, dst_w, xx + 2, yy + 2, b);
            }
            else {
                dst_pix_l(dst_raw, dst_w, xx + 1, yy + 2, r);
                dst_pix_l(dst_raw, dst_w, xx, yy, g);
                dst_pix_l(dst_raw, dst_w, xx + 2, yy, b);
            }
        }
    }

    uint32_t *texture_pixels;
    int texture_pitch;
    SDL_LockTexture(texture, NULL, (void **)&texture_pixels, &texture_pitch);
    assert(texture_pitch == (screen->width * 4));
    memcpy(texture_pixels, screen->buf, screen->height * texture_pitch);
    SDL_UnlockTexture(texture);

}

void disp_init(void) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "SDL initialization error %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    // Detect 2X HiDPI screen, if high dpi, set to 1X size
    if (w == WINDOW_WIDTH * 2) {
        SDL_SetWindowSize(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        SDL_GL_GetDrawableSize(window, &w, &h);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, w, h);

    screen = disp_create(w, h, PIXFMT_ARGB8888);
}

void disp_deinit(void) {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void disp_present(Rect dest_rect, WaveformMode mode, bool partial, bool wait) {
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

Canvas *disp_load_image(char *filename) {
    int x, y, n;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    PixelFormat fmt;
    if (n == 1)
        fmt = PIXFMT_Y8;
    else if (n == 3)
        fmt = PIXFMT_RGB888;
    else if (n == 4)
        fmt = PIXFMT_RGBA8888_BE;
    else
        return NULL; // YA88 not supported
    Canvas *canvas = disp_create(x, y, fmt);
    memcpy(canvas->buf, data, x * y * n);
    return canvas;
}
