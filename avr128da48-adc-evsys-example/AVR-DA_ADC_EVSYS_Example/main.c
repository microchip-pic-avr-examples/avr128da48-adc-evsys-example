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

#include <avr/io.h>
#include <avr/interrupt.h>

static void VREF_init(void);
static void ADC0_init(void);
static void PORT_init(void);
static void EVSYS_init(void);

volatile uint16_t adc_value;

static void VREF_init(void)
{
    VREF.ADC0REF = VREF_REFSEL_2V048_gc     /* Select 2.048V Reference for ADC */
                 | VREF_ALWAYSON_bm;        /* Select the Always On mode */
}

static void ADC0_init(void)
{
    /* Select CLK_PER prescaled with 2 */
    ADC0.CTRLC = ADC_PRESC_DIV2_gc;
    /* MUX selection for Positive ADC input */
    ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;
    /* Enable Event System */
    ADC0.EVCTRL = ADC_STARTEI_bm;
    /* Enable Result Ready Interrupt */
    ADC0.INTCTRL = ADC_RESRDY_bm;
    ADC0.CTRLA = ADC_ENABLE_bm              /* Enable ADC */
               | ADC_RESSEL_12BIT_gc;       /* Select 12-bit Resolution */
}

static void PORT_init(void)
{
    /* PC7 set as input (button) */
    PORTC.DIRCLR |= PIN7_bm;
    /* Use internal pull-up resistor */
    PORTC.PIN7CTRL |= PORT_PULLUPEN_bm;	
    /* PC6 set as output (LED for visualization) */
    PORTC.DIRSET |= PIN6_bm;
}

static void EVSYS_init(void)
{
    /* Set PC7 (button) as Event Generator on Channel 3 */
    EVSYS.CHANNEL3 = EVSYS_CHANNEL3_PORTC_PIN7_gc;
    /* Set ADC as Event User on Channel 3 */
    EVSYS.USERADC0START = EVSYS_USER_CHANNEL3_gc;
}

ISR(ADC0_RESRDY_vect)
{
    /* Toggle LED (for visualization) */
    PORTC.OUTTGL |= PIN6_bm;
    /* Store the ADC Conversion Result and Clear Interrupt Flag */
    adc_value = ADC0.RES;
}

int main(void)
{
    VREF_init();
    ADC0_init();
    PORT_init();
    EVSYS_init();
	
    /* Enable Global Interrupts */
    sei();
	
    while (1)
    {
        ;
    }
}