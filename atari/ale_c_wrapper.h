#pragma once

#include <cstdint>

namespace ale{
    class ALEInterface;
}

void getScreen(ale::ALEInterface *ale, unsigned char *screen_data);
void getRAM(ale::ALEInterface *ale, uint8_t *ram);
void getScreenRGB(ale::ALEInterface *ale, unsigned char *output_buffer);
void getScreenRGB2(ale::ALEInterface *ale, unsigned char *output_buffer);
void setLoggerMode(int mode);
