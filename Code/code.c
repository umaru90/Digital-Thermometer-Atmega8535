#include <mega8535.h>
#include <alcd.h>
#include <delay.h>
#include <stdio.h>

// Inisialisasi ADC
void adc_init() {
    ADMUX = (1 << REFS0); // Referensi tegangan pada AVcc
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC dan prescaler 128
}

// Membaca nilai ADC dari channel yang dipilih
unsigned int adc_read(unsigned char channel) {
    channel &= 0b00000111; // Masking untuk memastikan channel berada di antara 0-7
    ADMUX = (ADMUX & 0xF8) | channel; // Pilih channel input
    ADCSRA |= (1 << ADSC); // Mulai konversi
    while (ADCSRA & (1 << ADSC)); // Tunggu sampai konversi selesai
    return ADCW; // Kembalikan nilai ADC
}

// Mendapatkan suhu dari sensor LM35
float get_temperature() {
    unsigned int adc_value = adc_read(0);
    float temperature = adc_value * 0.48828125; // (5 / 1024) * 100
    float calibration_offset = -0.2; // Penyesuaian kalibrasi
    return temperature + calibration_offset; // Kalibrasi suhu
}

// Inisialisasi Timer1
void timer1_init() {
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // Mode CTC dan prescaler 64
    OCR1A = 187; // Nilai pembanding untuk menghasilkan interrupt setiap 1ms
    TCNT1 = 0; // Reset counter
}

// Inisialisasi pin dan port
void pin_init() {
    DDRA = 0x00; // Port A sebagai input untuk sensor LM35
    DDRB = 0x07; // Port B sebagai output untuk LED (PIN.B0 - PIN.B2)
    DDRC = 0xFF; // Port C sebagai output untuk LCD
}

// Mengatur LED sesuai dengan suhu
void set_led(float temperature) {
    PORTB &= ~(1 << PORTB0); // Matikan LED Biru
    PORTB &= ~(1 << PORTB1); // Matikan LED Hijau
    PORTB &= ~(1 << PORTB2); // Matikan LED Merah
    
    if (temperature < 25.0) {
        PORTB |= (1 << PORTB0); // Nyalakan LED Biru
    } else if (temperature <= 35.0) {
        PORTB |= (1 << PORTB1); // Nyalakan LED Hijau
    } else {
        PORTB |= (1 << PORTB2); // Nyalakan LED Merah
    }
}

// Mendapatkan status ruangan berdasarkan suhu
flash char status_dingin[] = "Dingin";
flash char status_hangat[] = "Hangat";
flash char status_panas[] = "Panas";

flash char* get_room_status(float temperature) {
    if (temperature < 25.0) {
        return status_dingin;
    } else if (temperature <= 35.0) {
        return status_hangat;
    } else {
        return status_panas;
    }
}

// Fungsi utama
void main(void) {
    float temperature;
    char temp_str[16];
    char status_str[16];
    
    adc_init();
    lcd_init(16);
    timer1_init();
    pin_init();
    
    while (1) {
        temperature = get_temperature();
        set_led(temperature);
        
        sprintf(temp_str, "Temp: %.1f C", temperature);
        sprintf(status_str, "Status: %s", get_room_status(temperature));
        
        lcd_clear();
        lcd_gotoxy(0, 0);
        lcd_puts(temp_str);
        
        lcd_gotoxy(0, 1);
        lcd_puts(status_str);
        
        delay_ms(1000);
    }
}
