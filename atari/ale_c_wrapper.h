#ifndef __ALE_C_WRAPPER_H__
#define __ALE_C_WRAPPER_H__

#include "ale/ale_interface.hpp"

extern "C" {
  // Declares int rgb_palette[256]
  #include "atari_ntsc_rgb_palette.h"

static
  void getScreen(ale::ALEInterface *ale,unsigned char *screen_data){
    int w = ale->getScreen().width();
    int h = ale->getScreen().height();
    auto ale_screen_data = ale->getScreen().getArray();
    memcpy(screen_data,ale_screen_data,w*h*sizeof(ale_screen_data));
  }

static
  void getRAM(ale::ALEInterface *ale,unsigned char *ram){
    auto ale_ram = reinterpret_cast<const unsigned char *>(ale->getRAM().array());
    int size = ale->getRAM().size();
    memcpy(ram,ale_ram,size*sizeof(unsigned char));
  }

static
  void getScreenRGB(ale::ALEInterface *ale, unsigned char *output_buffer){
    size_t w = ale->getScreen().width();
    size_t h = ale->getScreen().height();
    size_t screen_size = w*h;
    auto *ale_screen_data = ale->getScreen().getArray();

    ale->theOSystem->colourPalette().applyPaletteRGB(output_buffer, ale_screen_data, screen_size );
  }

static
  void getScreenRGB2(ale::ALEInterface *ale, unsigned char *output_buffer){
    size_t w = ale->getScreen().width();
    size_t h = ale->getScreen().height();
    size_t screen_size = w*h;
    auto ale_screen_data = ale->getScreen().getArray();

    int j = 0;
    for(auto i = 0UL;i < screen_size;i++){
        unsigned int zrgb = rgb_palette[ale_screen_data[i]];
        output_buffer[j++] = (zrgb>>16)&0xff;
        output_buffer[j++] = (zrgb>>8)&0xff;
        output_buffer[j++] = (zrgb>>0)&0xff;
    }
  }
 static void setLoggerMode(int mode) { ale::Logger::setMode(ale::Logger::mode(mode)); }
}

#endif
