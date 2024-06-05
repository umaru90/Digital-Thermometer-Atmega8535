#include <mega8535.h>
#include <delay.h>
#include <i2c.h>
#include <stdio.h>

// Alamat I2C dari LCD (dapat bervariasi, biasanya 0x27 atau 0x3F)
#define LCD_I2C_ADDR 0x27

// Fungsi untuk menginisialisasi LCD I2C
void lcd_i2c_init() {
    // Inisialisasi I2C
    i2c_init();

    // Inisialisasi LCD
    i2c_start();
    i2c_write(LCD_I2C_ADDR << 1); // Shift left address by 1 bit for write operation
    i2c_write(0x00); // Control byte for initialization
    i2c_write(0x38); // Function set: 2 lines, 8-bit, 5x8 dots
    i2c_write(0x0C); // Display on, cursor off, blink off
    i2c_write(0x01); // Clear display
    i2c_write(0x06); // Entry mode set: Increment mode
    i2c_stop();

    delay_ms(20); // Delay untuk memberikan waktu inisialisasi
}

// Fungsi untuk mengirim perintah ke LCD
void lcd_i2c_command(unsigned char cmd) {
    i2c_start();
    i2c_write(LCD_I2C_ADDR << 1);
    i2c_write(0x00); // Control byte for command
    i2c_write(cmd);
    i2c_stop();
    delay_ms(2); // Delay untuk memastikan perintah selesai diproses
}

// Fungsi untuk mengirim data ke LCD
void lcd_i2c_data(unsigned char data) {
    i2c_start();
    i2c_write(LCD_I2C_ADDR << 1);
    i2c_write(0x40); // Control byte for data
    i2c_write(data);
    i2c_stop();
    delay_ms(2); // Delay untuk memastikan data selesai diproses
}

// Fungsi untuk menampilkan string di LCD
void lcd_i2c_puts(char* str) {
    while (*str) {
        lcd_i2c_data(*str++);
    }
}

// Fungsi untuk mengatur posisi kursor di LCD
void lcd_i2c_gotoxy(unsigned char x, unsigned char y) {
    unsigned char pos = 0x80 + (y * 0x40) + x;
    lcd_i2c_command(pos);
}

// Fungsi inisialisasi ADC
void adc_init() {
    // AVCC with external capacitor at AREF pin
    ADMUX = (1<<REFS0);
    // ADC Enable and prescaler of 128
    // 12 MHz/128 = 93.75 KHz
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}

// Fungsi untuk membaca nilai ADC
unsigned int adc_read(unsigned char channel) {
    // Select ADC Channel ch must be 0-7
    channel &= 0b00000111;
    ADMUX = (ADMUX & 0xF8) | channel;
    // Start Single conversion
    ADCSRA |= (1<<ADSC);
    // Wait for conversion to complete
    while(ADCSRA & (1<<ADSC));
    return ADCW; // menggunakan ADCW untuk nilai ADC 10-bit
}

// Fungsi untuk mengonversi nilai ADC ke suhu dalam derajat Celsius
int get_temperature() {
    unsigned int adc_value = adc_read(0); // Membaca nilai ADC dari channel 0 (PA0)
    float voltage = adc_value * 5.0 / 1024.0; // Mengonversi nilai ADC ke tegangan
    float temperature = voltage * 100.0; // Mengonversi tegangan ke suhu (LM35: 10mV per derajat Celsius)
    return (int)(temperature * 10); // Mengembalikan suhu dalam format integer (suhu x 10)
}

void main(void) {
    // Inisialisasi ADC
    adc_init();
    // Inisialisasi LCD I2C
    lcd_i2c_init();

    while (1) {
        // Membaca suhu dari sensor LM35
        int temperature = get_temperature();
        
        // Mengonversi suhu ke string untuk ditampilkan
        char temp_str[16];
        sprintf(temp_str, "Temp: %d.%d C", temperature / 10, temperature % 10);

        // Menampilkan suhu pada LCD
        lcd_i2c_command(0x01); // Clear display
        lcd_i2c_gotoxy(0, 0);
        lcd_i2c_puts(temp_str);
        
        // Delay 1 detik sebelum pembacaan berikutnya
        delay_ms(1000);
    }
}
