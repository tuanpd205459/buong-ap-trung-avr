#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "DHT.h"
#include "lcdpcf8574/lcdpcf8574.h"
#include "avr/interrupt.h"

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0

#define leftButton PC0
#define rightButton PC1
#define middleButton PC2
#define lightHatcher PB4		//chan D12 
#define buzzerPin PB1				//chan D9

static uint8_t dayTimeToHatch;
static uint8_t hourTimeToHatch;
static uint8_t minTimeToHatch;
static uint8_t tempHatch;
static float currentTemp;
static uint8_t prState = 0;

#define millisWaiting   1000
#define millisHatching  1000
#define millisBuzzer    1000*5
#define millisAlarm     1000*25

void setTimeToHatch();
void setTempHatch();
void printPoint(uint8_t y, uint8_t x);
void updateDisplayTimeToHatch();
void updateDisplayTemp();
void updateDisplay();
void tempAdjusting();
int debounce(uint8_t buttonPin);
void myDigitalWrite(uint8_t pin, uint8_t value);
int8_t myDigitalRead(uint8_t pin);

void setup() {
	sei();
	lcd_init(LCD_DISP_ON_BLINK);
	lcd_home();
	uint8_t led = 0;
	lcd_led(led);
	// Thiết lập các chân PC0, PC1, và PC2 là đầu ra
	DDRC |= (1 << leftButton) | (1 << middleButton) | (1 << rightButton);

	// Đặt các chân PC0, PC1, và PC2 lên mức logic HIGH
	PORTC |= (1 << leftButton) | (1 << middleButton) | (1 << rightButton);
	
	//Set Led output
	DDRB |= (1 << lightHatcher);
	
	//set coi output
	DDRB |= (1<<buzzerPin);
}
int8_t temperature_int = 0;
int8_t humidity_int = 0;
int main(void) {
	setup();

	

	while (1) {
		if (dht_GetTempUtil(&temperature_int, &humidity_int) != -1) {
			char buffer[50];
			sprintf(buffer, "Temp: %d C, Hum: %d%%", temperature_int, humidity_int);
			lcd_gotoxy(0, 0);
			lcd_puts(buffer);
			} else {
			lcd_gotoxy(1, 0);
			lcd_puts("ERROR");
		}
		_delay_ms(1500);
		
		
		unsigned long currentMillis;

		// Configuring State
		prState = 3;

		updateDisplay();
		setTimeToHatch();
		setTempHatch();
		
		// Waiting state
		prState = 0;

		
		
		
	}
	
	
	
	
	
	
	
	
	return 0;
}

void updateDisplayTimeToHatch() {
	lcd_gotoxy(6, 2);
	lcd_puts("Done");

	lcd_gotoxy(6, 3);
	lcd_puts(dayTimeToHatch <= 9 ? "0" : "");
	lcd_puts(utoa(dayTimeToHatch, NULL, 10));
	lcd_puts(":");

	lcd_gotoxy(9, 3);
	lcd_puts(hourTimeToHatch <= 9 ? "0" : "");
	lcd_puts(utoa(hourTimeToHatch, NULL, 10));
	lcd_puts(":");

	lcd_gotoxy(12, 3);
	lcd_puts(minTimeToHatch <= 9 ? "0" : "");
	lcd_puts(utoa(minTimeToHatch, NULL, 10));
}

void updateDisplayTemp() {
	lcd_gotoxy(16, 2);
	lcd_puts("Temp");

	lcd_gotoxy(17, 3);
	lcd_puts(prState == 3 ? utoa(tempHatch, NULL, 10) : utoa((uint8_t)currentTemp, NULL, 10));
}

void updateDisplay() {
	if (prState < 2) {
		lcd_gotoxy(3, 0);
		lcd_puts("BUONG AP TRUNG");

		lcd_gotoxy(0, 1);
		lcd_puts("====================");

		lcd_gotoxy(5, 2);
		lcd_puts("|");

		lcd_gotoxy(5, 3);
		lcd_puts("|");

		lcd_gotoxy(14, 2);
		lcd_puts("|");

		lcd_gotoxy(14, 3);
		lcd_puts("|");

		updateDisplayTimeToHatch();
		updateDisplayTemp();
		} else if (prState < 3) {
		//lcd_clear();
		lcd_gotoxy(3, 0);
		lcd_puts("BUONG AP TRUNG");

		lcd_gotoxy(4, 2);
		lcd_puts("TRUNG DA NO!");
		} else {
		while (myDigitalRead(middleButton)) {
			lcd_gotoxy(3, 0);
			lcd_puts("BUONG AP TRUNG");

			lcd_gotoxy(5, 2);
			lcd_puts("WELLCOME!!!");
		}
		//lcd_clear();
		lcd_gotoxy(3, 0);
		lcd_puts("BUONG AP TRUNG");

		lcd_gotoxy(0, 1);
		lcd_puts("====================");

		lcd_gotoxy(5, 2);
		lcd_puts("|");

		lcd_gotoxy(5, 3);
		lcd_puts("|");

		lcd_gotoxy(14, 2);
		lcd_puts("|");

		lcd_gotoxy(14, 3);
		lcd_puts("|");
	}
}

void tempAdjusting() {
	dht_GetTempUtil(&temperature_int, &humidity_int);
		currentTemp = temperature_int;

	if (currentTemp >= tempHatch + 1) {
		myDigitalWrite(lightHatcher, LOW);
		} else if (currentTemp <= tempHatch - 1) {
		myDigitalWrite(lightHatcher, HIGH);
	}
}
#define DEBOUNCE_DELAY 10
int debounce(uint8_t buttonPin) {
	if (!(PINC & (1 << buttonPin))) {
		_delay_ms(DEBOUNCE_DELAY);
		if (!(PINC & (1 << buttonPin))) {
			return 0;
		}
	}
	return 1;
}

void setTimeToHatch() {
	dayTimeToHatch = 0;
	hourTimeToHatch = 0;
	minTimeToHatch = 0;

	while (myDigitalRead(middleButton)) {
		updateDisplayTimeToHatch();
		if (!myDigitalRead(leftButton)) {
			if (dayTimeToHatch >= 49) {
				dayTimeToHatch = 49;
				} else {
				dayTimeToHatch++;
			}
			updateDisplayTimeToHatch();
			_delay_ms(350);
		}
		if (!myDigitalRead(rightButton)) {
			if (dayTimeToHatch == 0) {
				dayTimeToHatch = 0;
				} else {
				dayTimeToHatch--;
			}
			updateDisplayTimeToHatch();
			_delay_ms(350);
		}
		printPoint(6, 3);
	}
	updateDisplayTimeToHatch();
	_delay_ms(350);

	while (myDigitalRead(middleButton)) {
		updateDisplayTimeToHatch();
		if (!myDigitalRead(leftButton)) {
			if (hourTimeToHatch >= 23) {
				hourTimeToHatch = 23;
				} else {
				hourTimeToHatch++;
			}
			updateDisplayTimeToHatch();
			_delay_ms(350);
		}
		if (!myDigitalRead(rightButton)) {
			if (hourTimeToHatch == 0) {
				hourTimeToHatch = 0;
				} else {
				hourTimeToHatch--;
			}
			updateDisplayTimeToHatch();
			_delay_ms(350);
		}
		printPoint(9, 3);
	}
	updateDisplayTimeToHatch();
	_delay_ms(350);

	while (myDigitalRead(middleButton)) {
		updateDisplayTimeToHatch();
		if (!myDigitalRead(leftButton)) {
			if (minTimeToHatch >= 59) {
				minTimeToHatch = 59;
				} else {
				minTimeToHatch++;
			}
			updateDisplayTimeToHatch();
			_delay_ms(350);
		}
		if (! myDigitalRead(rightButton)) {
			if (minTimeToHatch == 0) {
				minTimeToHatch = 0;
				} else {
				minTimeToHatch--;
			}
			updateDisplayTimeToHatch();
			_delay_ms(350);
		}
		printPoint(12, 3);
	}
	updateDisplayTimeToHatch();
	_delay_ms(350);
}

void setTempHatch() {
	tempHatch = 34;
	while (myDigitalRead(middleButton)) {
		updateDisplayTemp();
		if (!myDigitalRead(leftButton)) {
			if (tempHatch >= 60) {
				tempHatch = 60;
				} else {
				tempHatch++;
			}
			updateDisplayTemp();
			_delay_ms(200);
		}
		if (!myDigitalRead(rightButton)) {
			if (tempHatch == 25) {
				tempHatch = 25;
				} else {
				tempHatch--;
			}
			updateDisplayTemp();
			_delay_ms(200);
		}
		printPoint(17, 3);
	}
	updateDisplayTemp();
	_delay_ms(350);
}

void printPoint(uint8_t y, uint8_t x) {
	lcd_gotoxy(y, x);
	lcd_puts("__");
}
int8_t myDigitalRead( uint8_t pin){
	if(PINB & (1<<pin)) return HIGH;
	else return LOW;
	
}
void myDigitalWrite(uint8_t pin, uint8_t value){
	 // Kiểm tra giá trị truyền vào
	 if (value == LOW) {
		 // Thiết lập chân là đầu ra và đặt nó về mức thấp
		 DDRB &= ~(1 << pin);
		 } else if (value == HIGH) {
		 // Thiết lập chân là đầu ra và đặt nó về mức cao
		 DDRB |= (1 << pin);
	 }
}
