#include <LPC17xx.h>
#include <stdio.h>

// ---- LCD PIN DEFINITIONS ----
#define RS (1<<9)
#define RW (1<<10)
#define EN (1<<11)

#define D4 (1<<19)
#define D5 (1<<20)
#define D6 (1<<21)
#define D7 (1<<22)

#define DATA_PINS (D4 | D5 | D6 | D7)

// ---- DELAY FUNCTION ----
void delay(int time) {
    for(int i=0;i<time;i++)
        for(int j=0;j<3000;j++);
}

// ---- LCD FUNCTIONS (UNCHANGED) ----
void lcd_nibble(unsigned char nibble) {
    LPC_GPIO0->FIOCLR = DATA_PINS;

    if(nibble & 0x01) LPC_GPIO0->FIOSET = D4;
    if(nibble & 0x02) LPC_GPIO0->FIOSET = D5;
    if(nibble & 0x04) LPC_GPIO0->FIOSET = D6;
    if(nibble & 0x08) LPC_GPIO0->FIOSET = D7;
}

void lcd_enable() {
    LPC_GPIO0->FIOSET = EN;
    delay(5);
    LPC_GPIO0->FIOCLR = EN;
}

void lcd_cmd(unsigned char cmd) {
    LPC_GPIO0->FIOCLR = RS;
    LPC_GPIO0->FIOCLR = RW;

    lcd_nibble(cmd >> 4);
    lcd_enable();

    lcd_nibble(cmd & 0x0F);
    lcd_enable();

    delay(5);
}

void lcd_data(unsigned char data) {
    LPC_GPIO0->FIOSET = RS;
    LPC_GPIO0->FIOCLR = RW;

    lcd_nibble(data >> 4);
    lcd_enable();

    lcd_nibble(data & 0x0F);
    lcd_enable();

    delay(5);
}

void lcd_string(char *str) {
    while(*str) {
        lcd_data(*str++);
    }
}

// ---- ADC INITIALIZATION ----
void adc_init() {
    LPC_PINCON->PINSEL1 &= ~(3<<14);  // Clear P0.23
    LPC_PINCON->PINSEL1 |=  (1<<14);  // Set as AD0.0

    LPC_SC->PCONP |= (1<<12);         // Power ON ADC

    LPC_ADC->ADCR = (1<<0) | (4<<8) | (1<<21);
}

// ---- READ ADC ----
unsigned int adc_read() {
    LPC_ADC->ADCR |= (1<<24);                 // Start conversion

    while(!(LPC_ADC->ADGDR & (1UL<<31)));     // Wait DONE

    return (LPC_ADC->ADGDR >> 4) & 0xFFF;     // 12-bit result
}

// ---- LCD INIT ----
void lcd_init() {
    delay(50);

    lcd_cmd(0x33);
    lcd_cmd(0x32);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);

    delay(10);
}

// ---- MAIN ----
int main() {

    unsigned int adc_val, percentage;
    char buffer[16];

    LPC_GPIO0->FIODIR |= RS | RW | EN | DATA_PINS;

    lcd_init();
    adc_init();

    while(1) {

        adc_val = adc_read();

        // ?? Calibrate (adjust later if needed)
        if(adc_val > 4000) adc_val = 4000;
        if(adc_val < 1000) adc_val = 1000;

        percentage = 100 - ((adc_val - 1000) * 100 / 3000);

        // ---- DISPLAY ----
        lcd_cmd(0x80);
        lcd_string("SOIL MOISTURE");

        lcd_cmd(0xC0);
        sprintf(buffer, "Value: %d%% ", percentage);
        lcd_string(buffer);

        delay(200);
    }
}
