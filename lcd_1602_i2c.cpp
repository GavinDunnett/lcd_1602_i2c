/**
 * Program to drive a LCD 1602 (5 x 8 Dots) panel via the I2C PCF8574T bridge chip.
 * 
 * Connections between Raspberry Pi Pico board and I2C chip:
 *      GPIO 4 (pin6) -> SDA on I2C chip
 *      GPIO 5 (pin7) -> SDL on I2C chip
 *      VSYS 3.3v (pin39) -> VCC on I2C chip
 *      GND (pin38) -> GND on I2C chip
 * 
 * Dependencies
 *      Quick setup for Raspberry Pi 4B or the Raspberry Pi 400
 *      wget https://raw.githubusercontent.com/raspberrypi/pico-setup/master/pico_setup.sh
 * References
 *      https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf
 *  Stack Overflow
 *      https://stackoverflow.com/questions/56668846/f3discovery-trying-to-use-an-lcd-screen-1602-with-an-i2c-module
 *  HD44780U (LCD-II) Dot Matrix Liquid Crystal Display Controller/Driver
 *      https://www.sparkfun.com/datasheets/LCD/HD44780.pdf pp. 24-25
 * 
 * Written by Gavin Dunnett 2021. MIT License https://mit-license.org
*/
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

/* This class controls a single LCD.
*/
class Lcd
{
private:
    int peripheralAddr = 0x27; // The I2C's address. 0x27 is the default for display devices.
public:
    void init()
    {
        sleep_ms(15); // Power-on initialization time for LCD

        uint8_t returnHomeInst(0x02);

        uint8_t functionSetInst = 0x20;
        uint8_t fourBitDL = 0x00;
        uint8_t eightBitDL = 0x10;
        uint8_t twoLineDisplay = 0x08;
        uint8_t oneLineDisplay = 0x00;
        uint8_t fiveByTenDots = 0x04;
        uint8_t fiveByEightDots = 0x00;

        uint8_t displayInst = 0x08;
        uint8_t displayOn = 0x04;
        uint8_t displayOff = 0x00;
        uint8_t cursorOn = 0x02;
        uint8_t cursorOff = 0x00;
        uint8_t blinkOn = 0x01;
        uint8_t blinkOff = 0x00;

        uint8_t entryModeInst = 0x04;
        uint8_t directionIncrement = 0x02;
        uint8_t directionDecrement = 0x00;
        uint8_t displayShiftOn = 0x01;
        uint8_t displayShiftOff = 0x00;

        uint8_t clearDisplayInst = 0x01;

        sendCommand(returnHomeInst);
        sendCommand(functionSetInst | fourBitDL | twoLineDisplay | fiveByEightDots);
        sendCommand(displayInst | displayOn | cursorOff | blinkOff);
        sendCommand(entryModeInst | directionIncrement | displayShiftOff);
        sendCommand(clearDisplayInst);
    }
    /*! IC2's address
    * \param addr
    */
    Lcd(uint8_t addr)
    {
        this->peripheralAddr = addr;
        i2c_init(i2c_default, 1000000);
        gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
        gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
        gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
        gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
        init();
    }
    /*! Send a command to the LCD 
    *   \param val
    */
    void sendCommand(uint8_t val)
    {
        uint8_t cmdRegister = 0x00;
        uint8_t high = (val & 0xF0) | cmdRegister;
        uint8_t low = ((val << 4) & 0xF0) | cmdRegister;
        sendByte(high);
        sendByte(low);
        sleep_ms(2);
    }
    /*! Print the given string on the LCD 
    *   \param s String
    */
    void print(const char *s)
    {
        while (*s)
            sendData(*s++);
    }
    void sendData(uint8_t val)
    {
        uint8_t dataRegister = 0x01;
        uint8_t high = (val & 0xF0) | dataRegister;
        sendByte(high);
        uint8_t low = ((val << 4) & 0xF0) | dataRegister;
        sendByte(low);
    }
    void setCursor(const int row, const int column)
    {
        const uint8_t ROW_1 = 0x08;
        const uint8_t ROW_2 = 0xC0;
        switch (row)
        {
        case 0:
            sendCommand(ROW_1 + column);
            break;
        case 1:
            sendCommand(ROW_2 + column);
        }
    }
    void sendByte(uint8_t val)
    {
        uint8_t backlight = 0x08;
        uint8_t enable = 0x04;
        bool noStop = true;
        int oneByte = 1;

        uint8_t val_enabled = val | backlight | enable;
        i2c_write_blocking(i2c_default, peripheralAddr, &val_enabled, oneByte, noStop);
        uint8_t val_disabled = (val & ~enable) | backlight;
        i2c_write_blocking(i2c_default, peripheralAddr, &val_disabled, oneByte, noStop);
        // sleep_ms(2);
    }
    /* Clear the LCD display
    */
    void clear()
    {
        const int CLEAR_DISPLAY = 0x01;
        sendCommand(CLEAR_DISPLAY);
    }
};
int main()
{
    Lcd display(0x27);
    while (true)
    {
        display.print("  Hello World!");
        sleep_ms(1000);
        display.setCursor(1, 0);
        display.print("Rasperry Pi Pico");
        sleep_ms(2000);
        display.init();
    }
}