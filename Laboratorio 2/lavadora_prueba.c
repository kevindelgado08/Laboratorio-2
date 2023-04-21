// Configuración del oscilador interno a 128 kHz
    OSCCAL |= (1 << CAL1) | (1 << CAL0);
    
    // Configuración del registro CLKPR
    CLKPR |= (1 << CLKPCE);  // Habilita la escritura en el registro CLKPR
    CLKPR |= (1 << CLKPS0);  // Establece el preescalador del reloj a 2 (128 kHz / 2 = 64 kHz)
    
    // Restablece el valor del registro OSCCAL a su valor predeterminado
    OSCCAL = 0x00;



  // Configurar Timer Counter 0 en modo CTC con prescaler de 1024
  TCCR0A |= (1 << WGM01); // modo CTC
  TCCR0B |= (1 << CS02) | (1 << CS00) | (0 << CS01); // prescaler de 1024
  OCR0A = 255; // valor de comparación
  
  TCNT0 = 131;
  //TIMSK = (1 << OCIE0A);