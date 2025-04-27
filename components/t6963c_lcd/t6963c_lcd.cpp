#include "t6963c_lcd.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"

/*
https://github.com/Orabig/Rasp-T6963C/tree/master
*/

namespace esphome {
namespace t6963c {

#define TEXT_HOME_ADDR	0x40
#define TEXT_AREA	0x41
#define GRAPH_HOME_ADDR	0x42
#define GRAPH_AREA	0x43

#define LCD_MODE	0x80
    #define MODE_OR		0x00
    #define MODE_EXOR	0x01
    #define MODE_AND    0x03
    #define MODE_TEXT_ONLY  0x04
    #define MODE_CG_RAM     0x08

#define DISP_MODE	0x90
    #define DM_GRAPHICS     0x08
    #define DM_TEXT         0x04
    #define DM_CURSOR       0x02
    #define DM_CURSOR_BLINK 0x01

#define CURSOR_PTR	0x21
#define OFFSET_REG	0x22
#define ADDR_PTR	0x24

#define AUTO_WRITE_START 0xB0
#define AUTO_WRITE_STOP  0xB2
#define DATA_WRITE_UP	0xC0
#define DATA_WRITE_DN	0xC2
#define DATA_WRITE	0xC4



#define GRAPHIC_BASE    0x0400
#define GRAPHIC_SIZE    (240/8 * 128)


static const char *const TAG = "t6963c";

void T6963C::setup()
{
    ESP_LOGD("T6963C", "running setup");
    this->reset_pin_->setup();
    this->wr_pin_->setup();
    this->wr_pin_->digital_write(true);
    this->cd_pin_->setup();
    this->enable_pin_->setup();
    this->enable_pin_->digital_write(true);
    this->data_pins_[0]->setup();
    this->data_pins_[1]->setup();
    this->data_pins_[2]->setup();
    this->data_pins_[3]->setup();
    this->data_pins_[4]->setup();
    this->data_pins_[5]->setup();
    this->data_pins_[6]->setup();
    this->data_pins_[7]->setup();

    this->reset_pin_->digital_write(false);
    delay(10);
    this->reset_pin_->digital_write(true);

    this->init_internal_(GRAPHIC_SIZE );
    this->initialise_();

}


void T6963C::write_byte(uint8_t data) {
    static uint8_t prev_write = 0;
    if (prev_write ^ data & 0x01) this->data_pins_[0]->digital_write((data&0x01) > 0);
    if (prev_write ^ data & 0x02) this->data_pins_[1]->digital_write((data&0x02) > 0);
    if (prev_write ^ data & 0x04) this->data_pins_[2]->digital_write((data&0x04) > 0);
    if (prev_write ^ data & 0x08) this->data_pins_[3]->digital_write((data&0x08) > 0);
    if (prev_write ^ data & 0x10) this->data_pins_[4]->digital_write((data&0x10) > 0);
    if (prev_write ^ data & 0x20) this->data_pins_[5]->digital_write((data&0x20) > 0);
    if (prev_write ^ data & 0x40) this->data_pins_[6]->digital_write((data&0x40) > 0);
    if (prev_write ^ data & 0x80) this->data_pins_[7]->digital_write((data&0x80) > 0);
    prev_write = data;
}


void T6963C::write_cmd(uint8_t command)
{
    this->cd_pin_->digital_write(true);
    this->wr_pin_->digital_write(false);
    this->enable_pin_->digital_write(false);
    this->write_byte(command);
    delayMicroseconds(1);
    this->enable_pin_->digital_write(true);
    this->wr_pin_->digital_write(true);
    delayMicroseconds(1);
}

void T6963C::write_data(uint8_t data)
{
    this->cd_pin_->digital_write(false);
    this->wr_pin_->digital_write(false);
    this->enable_pin_->digital_write(false);
    this->write_byte(data);
    delayMicroseconds(1);
    this->enable_pin_->digital_write(true);
    this->wr_pin_->digital_write(true);
    delayMicroseconds(1);
}

void T6963C::write_command_1d(uint8_t command, uint8_t param)
{
    this->write_data(param);
    this->write_cmd(command);
}

void T6963C::write_command_2d(uint8_t command, uint16_t param)
{
    this->write_data(param & 0x00FF);
    this->write_data((param>>8) & 0x00FF);
    this->write_cmd(command);
}

void T6963C::erase_display_()
{
    int counter;
    this->write_command_2d(ADDR_PTR, GRAPHIC_BASE);
    this->write_cmd(AUTO_WRITE_START);
    for(counter=0; counter < (GRAPHIC_SIZE); counter++ )
    {
        this->write_data(0x00);
    }
    this->write_cmd(AUTO_WRITE_STOP);
    
}

void T6963C::write_display_()
{
    ESP_LOGD("T6963C", "write display");
    int counter;
    this->write_command_2d(ADDR_PTR, GRAPHIC_BASE);
    this->write_cmd(AUTO_WRITE_START);

    for(counter=0; counter < (GRAPHIC_SIZE); counter++ )
    {
        this->write_data(this->buffer_[counter]);
    }
    this->write_cmd(AUTO_WRITE_STOP);
    
}

int T6963C::get_width_internal() { 
    return this->width_; 
}
int T6963C::get_height_internal() { return this->height_; }

void T6963C::update() {
    this->do_update_();
    this->write_display_();

    
}

void HOT T6963C::draw_absolute_pixel_internal(int x, int y, Color color) {
    int pos = y * (240 >> 3) + (x >> 3);
    if (x < 0 || x >= this->width_ || y < 0 || y >= this->height_)
    {
        return;
    }
    if ( pos >= GRAPHIC_SIZE )
    {
        return;
    }
    if(color.is_on())
    {
        this->buffer_[pos] |= (0x80 >> (x & 7));
    }
    else
    {
        this->buffer_[pos] &= ~(0x80 >> (x & 7));
    }
}

void T6963C::initialise_()
{
    ESP_LOGD("T6963C", "Running initialise");
    // set text home address
    this->write_command_2d(TEXT_HOME_ADDR, 0);

    // set test area
    this->write_command_2d(TEXT_AREA, 0x001E);

    // set graphic home address
    this->write_command_2d(GRAPH_HOME_ADDR, GRAPHIC_BASE);
    
    // set graphic area
    this->write_command_2d(GRAPH_AREA, 0x001E);

    // set LCD mode
    this->write_cmd(LCD_MODE | MODE_OR);

    // set display mode
    this->write_cmd(DISP_MODE | DM_GRAPHICS);

    this->erase_display_();
    this->write_display_();
}

}  // namespace t6963c
}  // namespace esphome
