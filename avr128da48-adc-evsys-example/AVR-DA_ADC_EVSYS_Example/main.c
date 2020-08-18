/*
    \file   main.c
    
    (c) 2019 Microchip Technology Inc. and its subsidiaries.
    
    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/

#define USART1_BAUD_RATE(BAUD_RATE)     ((float)(64 * 4000000 / (16 * (float)BAUD_RATE)) + 0.5)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static void VREF_init(void);
static void ADC0_init(void);
static void PORT_init(void);
static void EVSYS_init(void);
static void USART1_init(void);
static void USART1_sendChar(char c);
static int USART1_printChar(char c, FILE *stream);

volatile uint16_t adcValue;
volatile bool adcFlag;
/* The definition of a custom stream that will be handled by the USART send function */
static FILE USART_stream = FDEV_SETUP_STREAM(USART1_printChar, NULL, _FDEV_SETUP_WRITE);

static void VREF_init(void)
{
    VREF.ADC0REF = VREF_REFSEL_VDD_gc       /* Select VDD as reference for ADC */
                 | VREF_ALWAYSON_bm;        /* Select the Always On mode */
}

static void ADC0_init(void)
{
    /* Select CLK_PER prescaled with 2 */
    ADC0.CTRLC = ADC_PRESC_DIV2_gc;
    /* Select AIN1 (PD1) as ADC input */
    ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;
    /* Enable Event System */
    ADC0.EVCTRL = ADC_STARTEI_bm;
    /* Enable Result Ready Interrupt */
    ADC0.INTCTRL = ADC_RESRDY_bm;
    ADC0.CTRLA = ADC_ENABLE_bm              /* Enable ADC */
               | ADC_RESSEL_12BIT_gc;       /* Select 12-bit Resolution */
}

static void PORT_init(void)
{ 
    /* PC0 set as as output (TX - USART1) */
    PORTC.DIRSET = PIN0_bm; 
    /* PC1 set as as input (RX - USART1) */                            
    PORTC.DIRCLR = PIN1_bm;                             
    /* PC6 set as output (LED for visualization) */
    PORTC.DIRSET = PIN6_bm;  
    /* PC7 set as input (button) */
    PORTC.DIRCLR = PIN7_bm;
    /* Use internal pull-up resistor */
    PORTC.PIN7CTRL = PORT_PULLUPEN_bm;	
    /* PD1 set as input (AIN1 - Analog Input 1) */
    PORTD.DIRCLR = PIN1_bm;
    /* Disable digital input buffer */
    PORTD.PIN1CTRL &= ~PORT_ISC_gm;
    PORTD.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    /* Disable pull-up resistor */
    PORTD.PIN1CTRL &= ~PORT_PULLUPEN_bm;
}

static void EVSYS_init(void)
{
    /* Set PC7 (button) as Event Generator on Channel 3 */
    EVSYS.CHANNEL3 = EVSYS_CHANNEL3_PORTC_PIN7_gc;
    /* Set ADC as Event User on Channel 3 */
    EVSYS.USERADC0START = EVSYS_USER_CHANNEL3_gc;
}

static void USART1_init(void)
{
    /* Set the baud rate to 9600*/
    USART1.BAUD = (uint16_t)(USART1_BAUD_RATE(9600));
    /* Set the data format to 8N1*/
    USART1.CTRLC = USART_CHSIZE0_bm
                 | USART_CHSIZE1_bm;
    /* Enable transmitter */
    USART1.CTRLB |= USART_TXEN_bm;
    /* Replace the standard output stream */
    stdout = &USART_stream;
}

static void USART1_sendChar(char c)
{
    /* Wait until data register is empty */
    while(!(USART1.STATUS & USART_DREIF_bm))
    {
        ;
    }   
    /* Send data */ 
    USART1.TXDATAL = c;
}

static int USART1_printChar(char c, FILE *stream)
{
    USART1_sendChar(c);
    return 0;
}

ISR(ADC0_RESRDY_vect)
{
    /* This flag marks the end of an ADC conversion cycle */
    adcFlag = true;
    /* Store the ADC Conversion Result and Clear Interrupt Flag */
    adcValue = ADC0.RES;
}

int main(void)
{
    VREF_init();
    ADC0_init();
    PORT_init();
    EVSYS_init();
    USART1_init();
	
    /* Enable Global Interrupts */
    sei();
	
    while (1)
    {
         if(adcFlag == true)
         {
            /* Toggle LED (for visualization) */
            PORTC.OUTTGL |= PIN6_bm;
            
            /* Printing both the raw and computed results */
            printf("ADC Conversion Raw Result = %d\r\n", adcValue);
            printf("ADC Conversion Result [V] = %.2fV\r\n", adcValue / 4096.0 * 3.3);

            /* Clearing the flag */
            adcFlag = false;
         }            
    }
}