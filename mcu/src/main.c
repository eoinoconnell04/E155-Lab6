/*
File: main.c
Author Eoin O'Connell
Email: eoconnell@hmc.edu
Date: Nov. 4, 2025

Modified from E155 Source Code:
File: Lab_6_JHB.c
Author: Josh Brake
Email: jbrake@hmc.edu
Date: 9/14/19
*/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

int _write(int file, char *ptr, int len);

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>E155 Web Server Demo Webpage</h1>";
char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";
char* tempPrecisionStr = "<p>Temperature Precision Control:</p>"
    "<form action=\"8bit\"><input type=\"submit\" value=\"8-Bit Precision\"></form>"
    "<form action=\"9bit\"><input type=\"submit\" value=\"9-Bit Precision\"></form>"
    "<form action=\"10bit\"><input type=\"submit\" value=\"10-Bit Precision\"></form>"
    "<form action=\"11bit\"><input type=\"submit\" value=\"11-Bit Precision\"></form>"
    "<form action=\"12bit\"><input type=\"submit\" value=\"12-Bit Precision\"></form>";
char* webpageEnd   = "</body></html>";

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

int updateLEDStatus(char request[])
{
	int led_status = 0;
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}

int updatePrecision(char request[]) {
  int precision = 0;
	// The request has been received. now process which is user selected precision
	if (inString(request, "8bit")==1) {
		setPrecision(8);
    precision = 8;
	}
	else if (inString(request, "9bit")==1) {
		setPrecision(9);
    precision = 9;
	}
  else if (inString(request, "10bit")==1) {
		setPrecision(10);
    precision = 10;
	}
  else if (inString(request, "11bit")==1) {
		setPrecision(11);
    precision = 11;
	}
  else if (inString(request, "12bit")==1) {
		setPrecision(12);
    precision = 12;
	}

	return precision;
}

/////////////////////////////////////////////////////////////////
// Solution Functions
/////////////////////////////////////////////////////////////////

int main(void) {
  configureFlash();
  configureClock();

  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  pinMode(PB3, GPIO_OUTPUT);
  
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);
  
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  // TODO: Add SPI initialization code
  initDS1722();  // this function initializes SPI for the specific configuration of the temperature sensor

  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */

    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
      // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      request[charIndex++] = readChar(USART);
    }

    // Update the temperature precision
    int newPrecision = updatePrecision(request);

    // Convert the current precision to a string for HTML
    char currentPrecisionStr[30];  
    sprintf(currentPrecisionStr, "Current Precision: %d bit", newPrecision);

        // Read temperature from sensor
    float temp = readTemp();
    //float temp = 12.02323;
    char currentTempStr[100];

    if (newPrecision == 8)
        sprintf(currentTempStr, "Current temperature: %.0f &deg;C", temp);
    else if (newPrecision == 9)
        sprintf(currentTempStr, "Current temperature: %.1f &deg;C", temp);
    else if (newPrecision == 10)
        sprintf(currentTempStr, "Current temperature: %.2f &deg;C", temp);
    else if (newPrecision == 11)
        sprintf(currentTempStr, "Current temperature: %.3f &deg;C", temp);
    else if (newPrecision == 12)
        sprintf(currentTempStr, "Current temperature: %.4f &deg;C", temp);
    else
        sprintf(currentTempStr, "Current temperature: %.1f &deg;C", temp);  // default

    // Update string with current LED state
  
    int led_status = updateLEDStatus(request);

    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED is on!");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED is off!");

    // finally, transmit the webpage over UART
    sendString(USART, webpageStart); // webpage header code
    sendString(USART, ledStr); // button for controlling LED

    sendString(USART, "<h2>LED Status</h2>");


    sendString(USART, "<p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");

    sendString(USART, "<h2>Temperature</h2>");

    sendString(USART, currentTempStr);
    sendString(USART, "<br><br>");
    sendString(USART, currentPrecisionStr);
    sendString(USART, tempPrecisionStr);
  
    sendString(USART, webpageEnd);
  }
}
/*
// Function used by printf to send characters to the laptop (taken from E155 website)
int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}
*/