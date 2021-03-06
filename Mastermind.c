#pragma config FOSC = INTRCIO, WDTE = OFF, PWRTE = OFF, MCLRE = OFF, CP = OFF, CPD = OFF, BOREN = OFF, IESO = OFF, FCMEN = OFF
#include "pic16f690.h"
#include <xc.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#define _XTAL_FREQ  4000000         // La freqüència del rellotge és 4 MHz
#define minipols PORTAbits.RA3      // Anomenem minipols a l'adreça PORTAbits.RA3
unsigned char Sortida1, Sortida2; // Sortida2 correspon als bits del 8 al 15 i Sortida1 als bits del 0 al 7

//Definició de les variables globals que comentarem en el moment en què s'usin
unsigned char color1, nled, cpc, cc, codi1, codi2;
bit bol;
unsigned char r1, r2, r3, r4, tirades, nivell, durada, C;
unsigned int c, r, com1, com2, seed;
int colors(char n);
unsigned int canvi = 0;
char Polsador();

// Definició de les funcions que farem servir
void EnviaR16(char Valor1, char Valor2);
void canviaColor(char led);
void TocaNota(char ValPR2, char ValCCPR1L, char ValDC1B, unsigned char durada);
unsigned char colorsABits(unsigned char color);
int rand(void);

void main(void) {
    TRISB = 0;
    TRISA = 0xFF;
    ANSEL = 0b00000101;
    ANSELH = 0;
    PORTB = 0;
    PORTC = 0;
    ADCON1 = 0b00010000;
    ADCON0 = 0b00001001;
    TRISC = 0b00100000;
    CCP1CON = 0b00001100;
    CCPR1L = 49;
    PIR1bits.TMR2IF = 0;
    T2CON = 0b00000011;
    Sortida1 = 0b00000000;
    Sortida2 = 0b00000000;
    EnviaR16(Sortida1, Sortida2); //Enviem leds apagats
    unsigned int partides1[10];
    unsigned int partides2[10];

    while (1) {
        if (canvi == 0) { //S'escull el nivell i es fa la combinació aleatòria
            PORTC = 0;
            seed = 0; //És la llavor que farà que cada partida tingui una combinació diferent de l'anterior
            tirades = 0; //Tirada que està fent el jugador
            nivell = 0; // Nombre de tirades màximes que tindrà el jugador
            // bol es 1 en els nivells dificils (hi ha repeticions de colors) i és 0 en els nivells fàcils (no hi ha repeticions)
            while (Polsador() != 5) {
                seed++;
                if (seed == 256) {
                    seed = 0;
                }
                if (Polsador() == 4) {
                    nivell = 5;
                    Sortida1 = 0b00000001;
                    bol = 1;
                }
                if (Polsador() == 3) {
                    nivell = 10;
                    Sortida1 = 0b00000010;
                    bol = 1;
                }
                if (Polsador() == 2) {
                    nivell = 5;
                    Sortida1 = 0b00000100;
                    bol = 0;
                }
                if (Polsador() == 1) {
                    nivell = 10;
                    Sortida1 = 0b00001000;
                    bol = 0;
                }
                EnviaR16(Sortida1, Sortida2);

            }
            if (nivell < 4) { //Fins que no s'esculli el nivell no s'avançarà al seguent pas i sonarà una nota
                TocaNota(189, 95, 0, 1);
                TocaNota(189, 95, 0, 1);
            }
            srand(seed);
            if (Polsador() == 5 && nivell != 0) {
                if (bol == 0) { // Es crea la combinació aleatòria per als nivells fàcils
                    TocaNota(238, 119, 2, 1);
                    TocaNota(158, 79, 2, 2);
                    r1 = (rand() % 8);
                    r2 = (rand() % 8);
                    while (r2 == r1) {
                        r2 = (rand() % 8);
                    }
                    r3 = (rand() % 8);
                    while (r3 == r1 || r3 == r2) {
                        r3 = (rand() % 8);
                    }
                    r4 = (rand() % 8);
                    while (r4 == r1 || r4 == r2 || r4 == r3) {
                        r4 = (rand() % 8);
                    }
                }
                if (bol == 1) { // Es crea la combinació aleatòria per als nivells difícils
                    TocaNota(238, 119, 2, 1);
                    TocaNota(158, 79, 2, 2);
                    r1 = (rand() % 8);
                    r2 = (rand() % 8);
                    r3 = (rand() % 8);
                    r4 = (rand() % 8);
                }
                r = ((r1 << 9) | (r2 << 6) | (r3 << 3) | r4) << 4;

                Sortida1 = 0b00000000;
                Sortida2 = 0b00000000;
                EnviaR16(Sortida1, Sortida2);
                canvi++;
                _delay(500000);
            }
            PORTC = 1;
        }
        if (canvi == 1) { //El jugador posa la combinació que vol provar
            tirades++;
            while (canvi == 1) {
                if (Polsador() == 5) {
                    _delay(400000);
                    c = (256 * Sortida2)+(Sortida1);
                    if (bol == 0 && (c >> 13 == (c << 3) >> 13 || c >> 13 == (c << 6) >> 13 || c >> 13 == (c << 9) >> 13 || (c << 3) >> 13 == (c << 6) >> 13 || (c << 3) >> 13 == (c << 9) >> 13 || (c << 6) >> 13 == (c << 9) >> 13)) {
                        //No es permet enviar colors repetits si s'està en el nivell fàcil
                        TocaNota(189, 95, 0, 1);
                        TocaNota(189, 95, 0, 1);
                    } else {
                        canvi++;
                    }
                    _delay(200000);
                }
                if (Polsador() != 5 | Polsador() != 0) { //Mostra la combinació que està fent el jugador
                    canviaColor(Polsador());
                    EnviaR16(Sortida1, Sortida2);
                    _delay(100000);
                }
            }
        }
        if (canvi == 2) { //Comprovació
            unsigned char n = tirades % 4; //S'augmenta la tirada en la que s'està
            C = 1;
            while (n > 0) {
                C = C * 2;
                n = n - 1;
            }
            PORTC = C;
            cpc = 0;
            cc = 0;
            codi1 = 0;
            codi2 = 0;
            unsigned char ccor[4];
            unsigned char rcor[4];
            unsigned char i = 0;
            while (i < 4) {
                ccor[i] = 0;
                rcor[i] = 0;
                i++;
            }
            i = 0;
            while (i < 4) { // Es comproven els colors i posicions correcte
                unsigned char cu = (c << i * 3) >> 13;
                unsigned char ru = (r << i * 3) >> 13;
                if (cu == ru) {
                    cpc++;
                    ccor[i] = 1;
                    rcor[i] = 1;
                }
                i++;
            }
            i = 0;
            while (i < 4) { // Es comprova que el color sigui correcte i la posició no
                unsigned char cu = (c << i * 3) >> 13;
                unsigned char j = 0;
                if (ccor[i] == 0) {
                    while (j < 4) {
                        unsigned char ru = (r << j * 3) >> 13;
                        if (i != j && rcor[j] == 0 && cu == ru) {
                            cc++;
                            rcor[j] = 1;
                            j = 5;
                        }
                        j++;
                    }
                }
                i++;
            }
            // S'encenen els leds adients a la comprovació
            if (cpc == 1) {
                codi1 = codi1 + 8;
                codi2 = codi2 + 8;
            }
            if (cpc == 2) {
                codi1 = codi1 + 12;
                codi2 = codi2 + 12;
            }
            if (cpc == 3) {
                codi1 = codi1 + 14;
                codi2 = codi2 + 14;
            }
            if (cpc == 4) {
                codi1 = 15;
                codi2 = 15;
            }
            if (cc == 4) {
                codi1 = codi1 + 15;
            }
            if (cc == 3) {
                codi1 = codi1 + 7;
            }
            if (cc == 2) {
                codi1 = codi1 + 3;
            }
            if (cc == 1) {
                codi1 = codi1 + 1;
            }
            com1 = c + codi1;
            com2 = c + codi2;
            partides1[tirades - 1] = com1;
            partides2[tirades - 1] = com2;
            while (Polsador() != 5 && cpc != 4 && tirades < nivell) { // Si es clica el polsador 6 (minipols) es poden consultar les partides anteriors
                if (minipols == 0) {
                    PORTC = 0;
                    _delay(520000);
                    unsigned char posicio = tirades - 1;
                    while (1) {
                        if (Polsador() == 2) { // Amb el polsador 2 s'accedeix a la combinació posterior
                            if (posicio == (tirades - 1)) {
                                TocaNota(189, 95, 0, 1);
                                TocaNota(189, 95, 0, 1);
                            } else {
                                posicio = posicio + 1;
                            }
                        }
                        if (Polsador() == 1) { // Amb el polsador 1 s'accedeix a la combinació anterior
                            if (posicio == 0) {
                                TocaNota(189, 95, 0, 1);
                                TocaNota(189, 95, 0, 1);
                            } else {
                                posicio = posicio - 1;
                            }
                        }
                        EnviaR16(partides1[posicio], partides1[posicio] >> 8);
                        _delay(100000);
                        EnviaR16(partides2[posicio], partides2[posicio] >> 8);
                        _delay(100000);
                        unsigned char D = 1;
                        unsigned char n = (posicio + 1) % 4;
                        while (n > 0) { // S'indica la tirada que s'està consultant en els leds vermells
                            D = D * 2;
                            n = n - 1;
                        }
                        if (D == 1) {
                            D = 16;
                        }
                        PORTC = D / 2;
                        if (minipols == 0) { // Si es clica el polsador 6 es surt d'aquest mode
                            break;
                        }
                    }
                }
                PORTC = C;
                EnviaR16(com1, com1 >> 8);
                _delay(120000);
                EnviaR16(com2, com2 >> 8);
                _delay(120000);
                canvi = 1;
            }
            if (cpc == 4) { // Si s'han encertat tots els colors en posició correcte es passa a la victòria
                EnviaR16(com1, com1 >> 8);
                canvi = 3;
            }
            if (tirades >= nivell) { // Si s'han superat el nombre de tirades màxim es passa a la derrota
                Sortida1 = 0b00000000;
                Sortida2 = 0b00000000;
                EnviaR16(Sortida1, Sortida2);
                canvi = 4;
            }
            _delay(220000);
        }

        if (canvi == 3) { //VICTORIA!
            //Sona música victoria i s'ensenya la combinació amb la que s'ha guanyat
            PORTC = 15;
            TocaNota(189, 95, 0, 2);
            TocaNota(189, 95, 0, 1);
            TocaNota(189, 95, 0, 1);
            TocaNota(141, 71, 0, 3);
            TocaNota(158, 79, 2, 1);
            TocaNota(189, 95, 0, 2);
            TocaNota(189, 95, 0, 1);
            TocaNota(189, 95, 0, 1);
            TocaNota(141, 71, 0, 3);
            TocaNota(126, 63, 2, 1);
            TocaNota(118, 59, 2, 2);
            TocaNota(189, 95, 0, 1);
            TocaNota(189, 95, 0, 1);
            TocaNota(141, 71, 0, 3);
            TocaNota(126, 63, 2, 1);
            TocaNota(141, 71, 0, 2);
            TocaNota(126, 63, 2, 2);
            TocaNota(141, 71, 0, 4);
            __delay_ms(400);
            canvi = 5;
        }
        if (canvi == 4) { //DERROTA : LÍMIT TIRADES
            //Sona música i s'ensenya combinació guanyadora
            Sortida1 = 0b00000000;
            Sortida2 = 0b00000000;
            EnviaR16(Sortida1, Sortida2);
            __delay_ms(200);
            Sortida2 = (r >> 8) & 0xFF;
            Sortida1 = (r) & 0xFF;
            EnviaR16(Sortida1, Sortida2);
            TocaNota(158, 79, 2, 3); //sol3
            Sortida1 = 0b00000000;
            Sortida2 = 0b00000000;
            EnviaR16(Sortida1, Sortida2);
            __delay_ms(200);
            Sortida2 = (r >> 8) & 0xFF;
            Sortida1 = (r) & 0xFF;
            EnviaR16(Sortida1, Sortida2);
            TocaNota(168, 84, 2, 3); //fa3#
            Sortida1 = 0b00000000;
            Sortida2 = 0b00000000;
            EnviaR16(Sortida1, Sortida2);
            __delay_ms(200);
            Sortida2 = (r >> 8) & 0xFF;
            Sortida1 = (r) & 0xFF;
            EnviaR16(Sortida1, Sortida2);
            TocaNota(178, 89, 2, 3); //fa3
            Sortida1 = 0b00000000;
            Sortida2 = 0b00000000;
            EnviaR16(Sortida1, Sortida2);
            __delay_ms(200);
            Sortida2 = (r >> 8) & 0xFF;
            Sortida1 = (r) & 0xFF;
            EnviaR16(Sortida1, Sortida2);
            TocaNota(189, 95, 0, 8); //mi3
            canvi++;
        }
        if (canvi == 5) { // Pulsa el polsador 5 per a tornar a jugar
            PORTC = 0;
            Sortida1 = 0b00000000;
            Sortida2 = 0b00000000;
            EnviaR16(Sortida1, Sortida2);

            if (Polsador() == 5) {

                canvi = 0;
                _delay(9000);
            }
        }
    }
}

void EnviaR16(char Valor1, char Valor2) {
    char Port = 0; // Variable on guardem l'estat del port B
    char Temp; // Variable temporal
    for (int k = 1; k < 9; k++) { // Primer enviem Valor2
        Temp = Valor2 & 0b10000000; // Agafa el bit de mÈs a l'esquerra
        // Temp nomÈs podr? valer 0 o 128
        if (Temp == 0) { // Si val 0
            Port = Port & 0b11101111; // Desactiva Data (bit 4)
        } else { // Si val 128
            Port = Port | 0b00010000; // Activa Data (bit 4)
        }
        Valor2 = Valor2 << 1; // Rodem els bits per situar el seg¸ent
        PORTB = Port; // Ho posa al port B
        Port = Port | 0b00100000; // Activa Clock (bit 5) i forÁa lectura
        PORTB = Port; // Ho posa al port B
        Port = Port & 0b11011111; // Desactiva Clock (bit 5)
        PORTB = Port; // Ho posa al port B
    }
    for (int k = 1; k < 9; k++) { // DesprÈs enviem Valor1
        Temp = Valor1 & 0b10000000; // Agafa el bit de mÈs a l'esquerra
        // Temp nomÈs podr? valer 0 o 128
        if (Temp == 0) { // Si val 0
            Port = Port & 0b11101111; // Desactiva Data (bit 4)
        } else { // Si val 128

            Port = Port | 0b00010000; // Activa Data (bit 4)
        }
        Valor1 = Valor1 << 1; // Rodem els bits per situar el seg¸ent
        PORTB = Port; // Ho posa al port B
        Port = Port | 0b00100000; // Activa Clock (bit 5) i forÁa lectura
        PORTB = Port; // Ho posa al port B
        Port = Port & 0b11011111; // Desactiva Clock (bit 5)
        PORTB = Port; // Ho posa al port B
    }
    Port = Port | 0b01000000; // Activa Latch (bit 6) per copiar a les sortides
    PORTB = Port; // Ho posa al port B
}

char Polsador(void) {
    char Pols = 0;
    ADCON0bits.GO = 1; // Posa en marxa el conversor
    while (ADCON0bits.GO == 1) // Mentre no acabi
        ; // ens esperem
    if (ADRESH < 220 && ADRESH > 200) {
        Pols = 5; // Comprova polsador 1
    }
    if (ADRESH < 194 && ADRESH > 174) {
        Pols = 4; // Comprova polsador 2
    }
    if (ADRESH < 163 && ADRESH > 143) {
        Pols = 3; // Comprova polsador 3
    }
    if (ADRESH < 90 && ADRESH > 70) {
        Pols = 2; // Comprova polsador 4
    }
    if (ADRESH < 55 && ADRESH > 35) {

        Pols = 1; // Comprova polsador 5
    }

    return Pols;
}

void canviaColor(char led) {

    unsigned int temp = Sortida2;
    temp <<= 8;
    temp |= Sortida1;
    unsigned int mask = 0b0111 << 3 * (4 - led) + 4;
    unsigned char bitsInicials = (temp & (mask)) >> 3 * (4 - led) + 4;
    temp = (temp & (~mask)) | (colors(colorsABits(bitsInicials) + 1) << 3 * (4 - led) + 4);
    Sortida2 = (temp >> 8) & 0xFF;
    Sortida1 = (temp) & 0xFF;
}

int colors(char n) {
    unsigned char ne;
    if (n % 8 == 6) ne = 0b00000111;
    if (n % 8 == 5) ne = 0b00000110;
    if (n % 8 == 4) ne = 0b00000101;
    if (n % 8 == 3) ne = 0b00000011;
    if (n % 8 == 2) ne = 0b00000100;
    if (n % 8 == 1) ne = 0b00000010;
    if (n % 8 == 0) ne = 0b00000001;
    if (n % 8 == 7) ne = 0b00000000;

    return ne;
}

unsigned char colorsABits(unsigned char color) {
    unsigned char ne = 0;
    if (color == 0b00000111) ne = 6;
    if (color == 0b00000110) ne = 5;
    if (color == 0b00000101) ne = 4;
    if (color == 0b00000011) ne = 3;
    if (color == 0b00000100) ne = 2;
    if (color == 0b00000010) ne = 1;
    if (color == 0b00000001) ne = 0;
    if (color == 0b00000000) ne = 7;

    return ne;
}

void TocaNota(char ValPR2, char ValCCPR1L, char ValDC1B, unsigned char durada) {
    TRISC = 0b00100000; // Definim com volem les E/S del port C
    // RC5 (sortida del PWM), de moment, com a entrada
    PR2 = ValPR2; // Carrega PR2
    CCP1CON = CCP1CON & 0b11001111; // Posa a zero els bits que corresponen a DC1B
    ValDC1B = ValDC1B % 4; // DC1B va de 0 a 3
    ValDC1B = ValDC1B * 16; // Desplaça els bits a la posició que els correspon a CCP1CON
    CCP1CON = CCP1CON + ValDC1B; // Coloca DC1B al seu lloc
    CCPR1L = ValCCPR1L; // Carrega CCPR1L, registre que ens dóna l'amplada de tON
    PIR1bits.TMR2IF = 0; // Desactiva el bit d'interrupció del Timer 2
    T2CON = 0b00000111; // Configura el Timer 2
    // bits T2KCPS (bits 1-0) a 11 prescalat de 16
    // bit 2 (TMR2ON) a 1, Timer activat
    // Postscaler TOUTPS (bits 6-3) no afecten al PWM

    while (PIR1bits.TMR2IF == 0) // Espera l'activació del bit d'interrupció del Timer 2
        ; // Esperem
    TRISC = 0b00000000; // Posem RC5 (sortida del PWM) com a sortida
    if (durada == 1) {
        __delay_ms(150);
    } // CORXERA
    if (durada == 2) {
        __delay_ms(350);
    } // NEGRA
    if (durada == 3) {
        __delay_ms(550);
    } // NEGRA AMB PUNT
    if (durada == 4) {
        __delay_ms(750);
    } // BLANCA
    if (durada == 8) {
        __delay_ms(1550);
    } // RODONA
    TRISC = 0b00100000; // Posem RC5 (sortida del PWM) com a entrada
    // O sigui, silenci
    __delay_ms(50); // Retard de 0,05 s
}
