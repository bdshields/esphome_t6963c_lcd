#pragma once

#include "esphome/core/hal.h"
#include "esphome/components/display/display_buffer.h"

namespace esphome {
namespace t6963c {

class T6963C;


class T6963C : public display::DisplayBuffer{
public:
    void set_data_pins(GPIOPin *d0, GPIOPin *d1, GPIOPin *d2, GPIOPin *d3, GPIOPin *d4, GPIOPin *d5, GPIOPin *d6,
                     GPIOPin *d7) {
        this->data_pins_[0] = d0;
        this->data_pins_[1] = d1;
        this->data_pins_[2] = d2;
        this->data_pins_[3] = d3;
        this->data_pins_[4] = d4;
        this->data_pins_[5] = d5;
        this->data_pins_[6] = d6;
        this->data_pins_[7] = d7;
    }
    void set_enable_pin(GPIOPin *enable) { this->enable_pin_ = enable; }
    void set_reset_pin(GPIOPin *reset) { this->reset_pin_ = reset; }
    void set_wr_pin(GPIOPin *wr) { this->wr_pin_ = wr; }
    void set_cd_pin(GPIOPin *cd) { this->cd_pin_ = cd; }

    void set_width(int16_t width) { this->width_ = width; }
    void set_height(int16_t height) { this->height_ = height; }

    display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_BINARY; }
    void setup() override;
    void update() override;
    void draw_absolute_pixel_internal(int x, int y, Color color) override;
    int get_height_internal() override;
    int get_width_internal() override;

protected:
    


    void write_byte(uint8_t data);
    void write_cmd(uint8_t cmd);
    void write_data(uint8_t data);
    void write_command_1d(uint8_t cmd, uint8_t data);
    void write_command_2d(uint8_t cmd, uint16_t data);
    void initialise_();
    void erase_display_();
    void write_display_();
    GPIOPin *reset_pin_{nullptr};
    GPIOPin *wr_pin_{nullptr};
    GPIOPin *cd_pin_{nullptr};
    GPIOPin *enable_pin_{nullptr};
    GPIOPin *data_pins_[8]{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    int16_t width_ = 240, height_ = 128;
};
}  // namespace st7920
}  // namespace esphome

