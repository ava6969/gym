
#include "atari_ntsc_rgb_palette.h"
#include "ale/ale_interface.hpp"
#include "ale_c_wrapper.h"

void getScreen(ale::ALEInterface *ale, unsigned char *screen_data) {
     auto w = ale->getScreen().width();
     auto h = ale->getScreen().height();
     auto ale_screen_data = ale->getScreen().getArray();
     memcpy(screen_data, ale_screen_data, w * h * sizeof(ale_screen_data));
 }

void getRAM(ale::ALEInterface *ale, uint8_t *ram) {
    auto ale_ram = reinterpret_cast<const uint8_t *>(ale->getRAM().array());
    auto size = ale->getRAM().size();
    memcpy(ram, ale_ram, size * sizeof(uint8_t));
}

void getScreenRGB(ale::ALEInterface *ale, unsigned char *output_buffer) {
    size_t w = ale->getScreen().width();
    size_t h = ale->getScreen().height();
    size_t screen_size = w * h;
    auto *ale_screen_data = ale->getScreen().getArray();

    ale->theOSystem->colourPalette().applyPaletteRGB(output_buffer, ale_screen_data, screen_size);
}

void getScreenRGB2(ale::ALEInterface *ale, unsigned char *output_buffer) {
    size_t w = ale->getScreen().width();
    size_t h = ale->getScreen().height();
    size_t screen_size = w * h;
    auto ale_screen_data = ale->getScreen().getArray();

    int j = 0;
    for (auto i = 0UL; i < screen_size; i++) {
        unsigned int zrgb = rgb_palette[ale_screen_data[i]];
        output_buffer[j++] = (zrgb >> 16) & 0xff;
        output_buffer[j++] = (zrgb >> 8) & 0xff;
        output_buffer[j++] = (zrgb >> 0) & 0xff;
    }
}

void setLoggerMode(int mode) {
    ale::Logger::setMode(ale::Logger::mode(mode));
}
