    /*
 * File:   PWM.c
 * Author: Cristian Catú
 * LAB 9
 *
 * Created on 27 april 2022, 19:04
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdint.h>
/*------------------------------------------------------------------------------
 * LIBRERIAS
 ------------------------------------------------------------------------------*/
#include "oscilador.h"
#include "adc.h"
#include "tmr0.h"
#include "pwm.h"
#include "LCD.h"

#define _XTAL_FREQ 4000000
#define RS RD2
#define EN RD3
#define D0 RC0
#define D1 RC1
#define D2 RC2
#define D3 RC3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

/*------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------*/
uint8_t contador = 0;
uint8_t canal_ADC = 0;
uint8_t VAL_POT_0 = 0;
uint8_t VAL_POT_1 = 0;
uint8_t init_POT_0 = 0;
uint8_t dec_POT_0 = 0;
uint8_t init_POT_1 = 0;
uint8_t dec_POT_1 = 0;
unsigned short VOLTAJE_0 = 0;
unsigned short VOLTAJE_1 = 0;
char s[];

uint8_t bandera = 0;
uint8_t ciclo_trabajo = 0;
void setup(void);
unsigned short map(uint8_t val, uint8_t in_min, uint8_t in_max, //Función del mapeo
            unsigned short out_min, unsigned short out_max);


void __interrupt() isr (void){
    if(INTCONbits.T0IF){ //Interrupción TMR0?
        contador++;
        if(contador == 100){ //cuenta hasta 5s
            if(bandera == 1){ //primer estado
                bandera = 0;
            }
            else if(bandera == 0){ //segundo estado
                bandera = 1;
            }
            contador = 0;
        }
        tmr0_reload();
    }
    if(PIR1bits.ADIF){ //verificamos canal y leemos con el ADC
        if(canal_ADC == 0){
            VAL_POT_0 = adc_read();
        }
        else if(canal_ADC == 1){
            VAL_POT_1 = adc_read();
        }
    }
    return;
}
/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();
    unsigned int a;
    Lcd_Init(); //inicialización
    Lcd_Clear(); //limpiamos LCD
    Lcd_Set_Cursor(1,1); //escojo la fila
    Lcd_Write_String(" CRISTIAN CATU "); //identifico con nombre
    Lcd_Set_Cursor(2,1); //escojo la fila
    Lcd_Write_String(" 20295"); //identifico con carné
    __delay_ms(2000); //esperamos 2 segundos
    while(1){
        if (canal_ADC == 0){ //primer canal
            adc_start(canal_ADC);
            canal_ADC = 1;
            VOLTAJE_0 = map(VAL_POT_0, 0, 255, 0, 500); //mapeamos el voltaje de 0 a 500
            init_POT_0 = VOLTAJE_0/100; //guardo el entero
            dec_POT_0 = VOLTAJE_0-init_POT_0*100; //guardo el decimal
        }
        else if (canal_ADC == 1){//segundo canal
            adc_start(canal_ADC);
            canal_ADC = 0;
            VOLTAJE_1 = map(VAL_POT_1, 0, 255, 0, 500);
            init_POT_1 = VOLTAJE_1/100;
            dec_POT_1 = VOLTAJE_1-init_POT_1*100;
        }
        if(bandera == 1){
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("      POT1        ");
            Lcd_Set_Cursor(2,1);
            sprintf(s, "      %d.%dV     ", init_POT_0, dec_POT_0); //guardamos el string que vamos a mostrar
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String(s);
        }
        else if(bandera == 0){
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("      POT2      ");
            Lcd_Set_Cursor(2,1);
            sprintf(s, "      %d.%dV      ", init_POT_1, dec_POT_1);
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String(s);
        }
        
    }   
    return;
}

/*------------------------------------------------------------------------------
 * CONFIGURACION
 ------------------------------------------------------------------------------*/
void setup(void){
    TRISD = 0X00;
    TRISC = 0X00;
    ANSELH = 0;
    TRISB = 0;
    PORTB = 0;
    init_osc_MHz(2); //freq            0 ---> 1MHz, 1 ---> 2MHz, 2 ---> 4MHz, 3 ---> 8MHz, 4 ---> 500kHz, default ---> 4MHz
    adc_init(1,0,0);
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;        // Habilitamos int. de perifericos
    INTCONbits.GIE = 1;         // Habilitamos int. globales
    PIE1bits.RCIE = 1;
    INTCONbits.T0IE = 1; //Habiliatamos int. TMR0
    tmr0_reload();   //función de TMR0
    tmr0_init(256);  //configuración prescaler 256 TMR0 
}

unsigned short map(uint8_t x, uint8_t x0, uint8_t x1,
            unsigned short y0, unsigned short y1){
    return (unsigned short)(y0+((float)(y1-y0)/(x1-x0))*(x-x0));
}