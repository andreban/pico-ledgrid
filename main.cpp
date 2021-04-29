#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "generated/ws2811.pio.h"

const int PANEL_WIDTH = 16;
const int PANEL_HEIGHT = 16;
const uint32_t CHROME_LOGO[256] {
        0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
        0x000000, 0x000000, 0x000000, 0x011500, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x011500, 0x000000, 0x000000, 0x000000,
        0x000000, 0x000000, 0x001100, 0x001300, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x000000, 0x000000,
        0x000000, 0x000F00, 0x001000, 0x001200, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x001400, 0x011500, 0x000000,
        0x000000, 0x080001, 0x000F00, 0x001100, 0x001300, 0x001400, 0x001400, 0x1B1B1B, 0x1B1B1B, 0x081500, 0x081700, 0x091800, 0x0A1900, 0x0B1B00, 0x0C1D00, 0x000000,
        0x080001, 0x080001, 0x000B00, 0x001000, 0x001200, 0x1B1B1B, 0x0F091B, 0x05001C, 0x05001C, 0x0F091B, 0x1B1B1B, 0x0D1E00, 0x0E1E00, 0x0E1E00, 0x0E1F00, 0x0F1F00,
        0x080001, 0x080001, 0x080001, 0x000C00, 0x001100, 0x0F091B, 0x05001C, 0x05001C, 0x05001C, 0x05001C, 0x0F091B, 0x0F1F00, 0x0F1F00, 0x101F00, 0x101F00, 0x111F00,
        0x080001, 0x080001, 0x080001, 0x030200, 0x1B1B1B, 0x05001C, 0x05001C, 0x05001C, 0x05001C, 0x05001C, 0x05001C, 0x1B1B1B, 0x111F00, 0x111F00, 0x111F00, 0x111F00,
        0x080001, 0x080001, 0x080001, 0x080001, 0x1B1B1B, 0x05001C, 0x05001C, 0x05001C, 0x05001C, 0x05001C, 0x05001C, 0x1B1B1B, 0x111F00, 0x111F00, 0x111F00, 0x111F00,
        0x080001, 0x080001, 0x080001, 0x080001, 0x080001, 0x0F091B, 0x05001C, 0x05001C, 0x05001C, 0x05001C, 0x0F091B, 0x111F00, 0x111F00, 0x111F00, 0x111F00, 0x111F00,
        0x080001, 0x080001, 0x080001, 0x080001, 0x080001, 0x1B1B1B, 0x0F091B, 0x05001C, 0x05001C, 0x0F091B, 0x1B1B1B, 0x111F00, 0x111F00, 0x111F00, 0x111F00, 0x111F00,
        0x000000, 0x080001, 0x080001, 0x080001, 0x080001, 0x080001, 0x080001, 0x1B1B1B, 0x1B1B1B, 0x050001, 0x111F00, 0x111F00, 0x111F00, 0x111F00, 0x111F00, 0x000000,
        0x000000, 0x070001, 0x080001, 0x080001, 0x080001, 0x080001, 0x080001, 0x080001, 0x070001, 0x040001, 0x111F00, 0x111F00, 0x111F00, 0x111F00, 0x101E00, 0x000000,
        0x000000, 0x000000, 0x080001, 0x080001, 0x080001, 0x080001, 0x080001, 0x080001, 0x050001, 0x111F00, 0x111F00, 0x111F00, 0x111F00, 0x111F00, 0x000000, 0x000000,
        0x000000, 0x000000, 0x000000, 0x070001, 0x080001, 0x080001, 0x080001, 0x070001, 0x111F00, 0x111F00, 0x111F00, 0x111F00, 0x0F1B00, 0x000000, 0x000000, 0x000000,
        0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x070001, 0x070001, 0x060001, 0x111F00, 0x111F00, 0x0E1B00, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
};

class LedStrip {
public:
    PIO pio;
    uint32_t *buffer;
    int pin_tx;
    int length;
    int sm = 0;
    LedStrip(PIO pio, int pin_tx, uint32_t buffer[], int length):
            pio(pio), pin_tx(pin_tx), buffer(buffer), length(length) {
        uint offset = pio_add_program(pio, &ws2812_program);
        ws2812_program_init(pio, sm, offset, pin_tx, 800000, false);
    }

    void clear() {
        for (int i = 0; i < length; i++) {
            buffer[i] = 0;
        }
    }

    void update() {
        for (int i = 0; i < length; i++) {
            pio_sm_put_blocking(pio, 0, buffer[i] << 8u);
        }
    }
};

class LedDisplay: public LedStrip {
public:
    int width;
    int height;
    LedDisplay(PIO pio, int pin_tx, uint32_t buffer[], int width, int height):
            LedStrip(pio, pin_tx, buffer, width * height), width(width), height(height) {
    }

    void set_pixel(int x, int y, uint32_t color) {
        if (x % 2 == 0) {
            buffer[x * height + y] = color;
        }  else {
            buffer[(x + 1) * height - y - 1] = color;
        }
    }
};

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}


const int PIN_TX = 7;
const int NUM_LEDS = 256;

int main() {
    stdio_init_all();
    puts("Led Strip!");

    uint8_t read_buffer[NUM_LEDS * 3];
    uint32_t buffer[NUM_LEDS];
    auto ledStrip = LedDisplay(pio0, PIN_TX, buffer, PANEL_WIDTH, PANEL_HEIGHT);
    ledStrip.clear();
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            uint32_t color = CHROME_LOGO[y * PANEL_WIDTH + x];
            ledStrip.set_pixel(x, y, color);
        }
    }
    ledStrip.update();
    while (true) {
        printf("Waiting for data\n");
        fread(read_buffer, 1, NUM_LEDS * 3, stdin);
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                int start_index = (y * PANEL_WIDTH + x) * 3;
                uint8_t red = read_buffer[start_index];
                uint8_t green = read_buffer[start_index + 1];
                uint8_t blue = read_buffer[start_index + 2];
                ledStrip.set_pixel(x, y, urgb_u32(red, green, blue));
            }
        }
        ledStrip.update();
        printf("Received data\n");
    }
}
