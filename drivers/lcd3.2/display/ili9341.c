#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include "ili9341.h"
#include "ili9341_config.h"

#define DATA_SIZE	90

static struct spi_device *lcd_spi_device;

static void lcd_ili9341_reset(void)
{
	gpio_set_value(LCD_PIN_RESET, 0);
	mdelay(5);
	gpio_set_value(LCD_PIN_RESET, 1);
}

static int lcd_ili9341_write_command(u8 cmd)
{
	gpio_set_value(LCD_PIN_DC, 0);
	return spi_write(lcd_spi_device, &cmd, sizeof(cmd));
}

int lcd_ili9341_write_data(u8 *buff, size_t buff_size)
{
	size_t i = 0;
	
	gpio_set_value(LCD_PIN_DC, 1);
	while (buff_size > DATA_SIZE) {
		spi_write(lcd_spi_device, buff + i, DATA_SIZE);
		i += DATA_SIZE;
		buff_size -= DATA_SIZE;
	}
	return spi_write(lcd_spi_device, buff + i, buff_size);
}

static void lcd_ili9341_set_address_window(u16 x0, u16 y0, u16 x1, u16 y1)
{

	lcd_ili9341_write_command(LCD_CASET);
	{
		uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF,
				 (x1 >> 8) & 0xFF, x1 & 0xFF };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	lcd_ili9341_write_command(LCD_RASET);
	{
		uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF,
				(y1 >> 8) & 0xFF, y1 & 0xFF };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	lcd_ili9341_write_command(LCD_RAMWR);
}


void lcd_ili9341_init(struct spi_device *lcd_spi_dev)
{
	lcd_spi_device = lcd_spi_dev;
    gpio_request(LCD_PIN_RESET, "LCD_PIN_RESET");
    gpio_direction_output(LCD_PIN_RESET, 0);
    gpio_request(LCD_PIN_DC, "LCD_PIN_DC");
    gpio_direction_output(LCD_PIN_DC, 0);
    lcd_ili9341_reset();

	// SOFTWARE RESET
	lcd_ili9341_write_command(0x01);
	mdelay(1000);

	// POWER CONTROL A
	lcd_ili9341_write_command(0xCB);
	{
		u8 data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// POWER CONTROL B
	lcd_ili9341_write_command(0xCF);
	{
		u8 data[] = { 0x00, 0xC1, 0x30 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// DRIVER TIMING CONTROL A
	lcd_ili9341_write_command(0xE8);
	{
		u8 data[] = { 0x85, 0x00, 0x78 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// DRIVER TIMING CONTROL B
	lcd_ili9341_write_command(0xEA);
	{
		u8 data[] = { 0x00, 0x00 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// POWER ON SEQUENCE CONTROL
	lcd_ili9341_write_command(0xED);
	{
		u8 data[] = { 0x64, 0x03, 0x12, 0x81 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// PUMP RATIO CONTROL
	lcd_ili9341_write_command(0xF7);
	{
		u8 data[] = { 0x20 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// POWER CONTROL,VRH[5:0]
	lcd_ili9341_write_command(0xC0);
	{
		u8 data[] = { 0x23 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// POWER CONTROL,SAP[2:0];BT[3:0]
	lcd_ili9341_write_command(0xC1);
	{
		u8 data[] = { 0x10 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// VCM CONTROL
	lcd_ili9341_write_command(0xC5);
	{
		u8 data[] = { 0x3E, 0x28 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// VCM CONTROL 2
	lcd_ili9341_write_command(0xC7);
	{
		u8 data[] = { 0x86 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// PIXEL FORMAT
	lcd_ili9341_write_command(0x3A);
	{
		u8 data[] = { 0x55 };
		lcd_ili9341_write_data(data, sizeof(data));
	}
	
	// FRAME RATIO CONTROL, STANDARD RGB COLOR
	lcd_ili9341_write_command(0xB1);
	{
		u8 data[] = { 0x00, 0x18 };
		lcd_ili9341_write_data(data, sizeof(data));
	}
	
	// DISPLAY FUNCTION CONTROL
	lcd_ili9341_write_command(0xB6);
	{
		u8 data[] = { 0x08, 0x82, 0x27 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// 3GAMMA FUNCTION DISABLE
	lcd_ili9341_write_command(0xF2);
	{
		u8 data[] = { 0x00 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// GAMMA CURVE SELECTED
	lcd_ili9341_write_command(0x26);
	{
		u8 data[] = { 0x01 };
		lcd_ili9341_write_data(data, sizeof(data));
	}
	
	// POSITIVE GAMMA CORRECTION
	lcd_ili9341_write_command(0xE0);
	{
		u8 data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
		lcd_ili9341_write_data(data, sizeof(data));
	}
	
	// NEGATIVE GAMMA CORRECTION
	lcd_ili9341_write_command(0xE1);
	{
		u8 data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// EXIT SLEEP
	lcd_ili9341_write_command(0x11);
	mdelay(120);
    
	// TURN ON DISPLAY
	lcd_ili9341_write_command(0x29);

	// MEMORY ACCESS CONTROL
	lcd_ili9341_write_command(0x36);
	{
		u8 data[] = { 0x28 };
		lcd_ili9341_write_data(data, sizeof(data));
	}

	// INVERSION
//	lcd_ili9341_write_command(0x21);


    lcd_ili9341_set_address_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

}
