
#define DDSFREQ 16000L
#define DDSTMRCOUNT (F_CPU / (8 * DDSFREQ) - 1)
#define PIN_DDS_OUT 9

#define DTMF_CW_697 2855
#define DTMF_CW_770 3154
#define DTMF_CW_852 3490
#define DTMF_CW_941 3855
#define DTMF_CW_1209 4953
#define DTMF_CW_1336 5473
#define DTMF_CW_1477 6051
#define DTMF_CW_1633 6690

uint16_t dtmfLo[] = {
                      DTMF_CW_941,  // 0
                      DTMF_CW_697,  // 1
                      DTMF_CW_697,  // 2
                      DTMF_CW_697,  // 3
                      DTMF_CW_770,  // 4
                      DTMF_CW_770,  // 5
                      DTMF_CW_770,  // 6
                      DTMF_CW_852,  // 7 
                      DTMF_CW_852,  // 8 
                      DTMF_CW_852,  // 9 
                      };
                      
uint16_t dtmfHi[] = { DTMF_CW_1336, // 0
                      DTMF_CW_1209, // 1 
                      DTMF_CW_1336, // 2
                      DTMF_CW_1477, // 3
                      DTMF_CW_1209, // 4
                      DTMF_CW_1336, // 5
                      DTMF_CW_1477, // 6
                      DTMF_CW_1209, // 7
                      DTMF_CW_1336, // 8
                      DTMF_CW_1477  // 9
                      };

volatile uint16_t freqControlA = 0;     // DDS Generator A control word.
volatile uint16_t freqControlB = 0;     // DDS Generator B control word.
uint16_t phaseAccumulatorA = 0;         // DDS Generator A Phase 
uint16_t phaseAccumulatorB = 0;         // DDS Generator B Phase 
uint8_t pwmAmplitude = 0;               // PWM generator amplitude control.

uint8_t sineLUT[] = {128, 152, 176, 199, 218, 234, 246, 253,
                     254, 253, 246, 234, 218, 199, 176, 152,
                     128, 103, 79, 56, 37, 21, 9, 2,
                     0, 2, 9, 21, 37, 56, 79, 103};

void setup()
{
    cli();

    // TIMER1: Drives the PWM at 62.5KHz
    TCCR1A = (1 << WGM10) | (1 << COM1A1);
    TCCR1B = (1 << WGM12) | (1 << CS10); // Prescaler /1
    TIMSK1 = (1 << TOIE1);               // Enable overflow

    // TIMER2 drives the DDS at 16KHz
    TCCR2A = 0;
    TCCR2B = (1 << CS21);   // Prescaler /8
    OCR2A = DDSTMRCOUNT;    // 16KHz DDS Clock
    TIMSK2 = (1 << OCIE2A); // Enable Timer2 OCR2A interrupt

    sei();

    pinMode(PIN_DDS_OUT, OUTPUT);

    Serial.begin(115400);
}

ISR(TIMER2_COMPA_vect)
{
    // NCO
    phaseAccumulatorA += freqControlA;
    phaseAccumulatorB += freqControlB;

    // PAC
    pwmAmplitude = (sineLUT[phaseAccumulatorA >> 11] >> 1) + (sineLUT[phaseAccumulatorB >> 11] >> 1);
}

ISR(TIMER1_OVF_vect)
{
    OCR1A = pwmAmplitude; // Output to PWM
}

void loop()
{
    uint8_t keyPressed = Serial.read();
    
    if (keyPressed >= '0' && keyPressed <= '9')
    {
        keyPressed = keyPressed - '0';
        
        freqControlA = 2 * dtmfHi[keyPressed];
        freqControlB = 2 * dtmfLo[keyPressed];
        
        delay(200);
        
        freqControlA = 0;
        freqControlB = 0;       
    }
}
