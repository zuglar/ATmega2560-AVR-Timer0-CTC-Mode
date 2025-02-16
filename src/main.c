#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

int Number_Of_Times_Interrupted = 1;
uint8_t tcnt0_value = 0;

void SetUp_Timer0_CTC(void)
{
    // Set the Timer Mode to CTC
    TCCR0A |= (1 << WGM01);
    // Set the value that you want to count to 194, count or generate 40Hz interruption
    OCR0A = 0xC2;
    // Enable the compare interrupt - OCIE0A - Enable interruption of OCR0A
    TIMSK0 |= (1 << OCIE0A);
    // Start the timer - prescaler 1024 - start counting every 1024 Machine cycles
    TCCR0B |= (1 << CS02) | (1 << CS00);
}

// Function to initialize UART
void uart_init(unsigned int baud)
{
    unsigned int ubrr = F_CPU / 16 / baud - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

// Function to send a character over UART
int uart_putchar(char c, FILE *stream)
{
    if (c == '\n')
    {
        uart_putchar('\r', stream);
    }
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = c;
    return 0;
}

// Create a FILE structure to use with printf
FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

int main(void)
{
    // Initialize UART with baud rate 9600
    uart_init(9600);

    // Redirect stdout to UART
    stdout = &uart_output;

    // Now you can use printf
    printf("Hello World\n");

    // Set pin 7 of Port B as output
    DDRB |= (1 << DDB7);
    // Set pin 7 of Port B low
    PORTB &= ~(1 << PB7);
    // Set up Timer0
    SetUp_Timer0_CTC();
    sei();

    while (1)
    {
        // Read the value of TCNT0
        tcnt0_value = TCNT0;

        // Print the value of TCNT0
        printf("TCNT0: %d\n", tcnt0_value);

        if(tcnt0_value == 100)
        {
            printf("TCNT0 has reached 100\n");
        }
    }

    return 0;
}

ISR(TIMER0_COMPA_vect) // Executes at the rate of 40Hz
{
    switch (Number_Of_Times_Interrupted)
    {
    case 1:
        Number_Of_Times_Interrupted++;  // Increment the number of times interrupted., divide 40Hz by 2 and get 20Hz
        break;
    case 2:
        // Toggle the LED
        PORTB ^= (1 << PB7);
        Number_Of_Times_Interrupted = 1;
        break;
    }

    /*
        if (Number_Of_Times_Interrupted < 10)
        {
            Number_Of_Times_Interrupted++;
        }
        else
        {
            // Toggle the LED
            PORTB ^= (1 << PB7);
            Number_Of_Times_Interrupted = 1;
        }
      */
}

/*
    The code is simple, it sets up Timer0 to generate an interruption every 40Hz. The interruption toggles the state of pin 7 of Port B.
    The code is compiled using the following command:
    avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o main.o src/main.cpp
*/