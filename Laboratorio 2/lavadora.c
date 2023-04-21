#include <avr/io.h>
#include <avr/interrupt.h>

//Variables Importantes
unsigned char t_suministro, t_lavar, t_enjuagar, t_centrigar, Tiempo_Actual; //tiempos de cada estado
unsigned char S_Inicio, S_suministro, S_Lavar, S_Enjuagar, S_Centrifugar; //Indicadores de Estado
unsigned char Estado, Accion, Volver_Estado, Prox_Estado, tiempo_estado;
unsigned char Sb = 0b00000001, Lb = 0b00000011, Eb = 0b00000010, Cb = 0b00000011; //Tiempo en Carga Baja
unsigned char Sm = 0b00000010, Lm = 0b00000111, Em = 0b00000100, Cm = 0b00000110; //Tiempo en Carga Media
unsigned char Sa = 0b00000011, La = 0b00001010, Ea = 0b00000101, Ca = 0b00001001; //Tiempo en Carga Alta

//Llamado a Funciones
void estados();

//Funciones a utilizar
int main(void)
{
  sei(); //habilita las interrupciones globales
  //Habilitaci\'on de GPIO'S
  DDRD = 0b01111000;
  PORTD = 0b01000000;  
  DDRB = 0b11110010;
  PORTB = 0b00000010;
  DDRA = 0b00000010;
  PORTA = 0b00000000;

  //Habilitación de interrupciones
  MCUCR = 0b00000001; // Permite que INTO se active cada vez que haya un cambio en su entrada
  GIMSK = 0b00111000; 
  PCMSK = 0b00000001;
  PCMSK1 = 0b00000001;
  PCMSK2 = 0b00000001;

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
  //Valores iniciales
  Estado = 'b';
  Accion = 0;
  Volver_Estado = 'a';
  while(1){
    
    estados();
  }
}
void display(tiempo_estado)
{
  
  for( decenas = tiempo_estado; decenas < 0; decenas --)
      for( unidades= tiempo_estado; unidades < 0; unidades --)
          for(int i=0;i<10; i++)
          {
              GPIO = (numdeci[decenas]); 
              GPIO = (numuni[unidades]);   
              
                              
          }
            
}
void estados(void) //procede a llamar los estados de la lavadora
{

switch(Estado)
{ 
  case 'b': //Inicio
    Accion = 0;
    PORTD &= ~(1<<5);
    PORTD &= ~(1<<4);
    PORTD &= ~(1<<3);
    PORTA &= ~(1<<1);
    break; 

    
  case 'c': //Suministro de Agua
    TIMSK = (1 << OCIE0A);
    S_suministro = 0, S_Lavar = 1, S_Enjuagar = 0, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<5);
    Tiempo_Actual = t_lavar;  
    Prox_Estado = 'd';
    
      break;

  case 'd': //Lavar
    S_suministro = 0, S_Lavar = 1, S_Enjuagar = 0, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<4);
    PORTD &= ~(1<<5);
    Tiempo_Actual = t_enjuagar;  
    Prox_Estado = 'e';

    break;

  case 'e': //Enjuagar
    S_suministro = 0, S_Lavar = 0, S_Enjuagar = 1, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<3);
    PORTD &= ~(1<<4);
    Tiempo_Actual = t_centrigar;  
    Prox_Estado = 'f';
    break;

  case 'f': //Centrifugar
    S_suministro = 0, S_Lavar = 0, S_Enjuagar = 0, S_Centrifugar = 1, Accion = 1;
    PORTA = PORTA | (1<<1);
    PORTD &= ~(1<<3);
    break;

  case 'g': //Pausa
    
    switch(Volver_Estado)
    {
      case 'a': //Volver a Suminitro de Agua
        switch(S_suministro)
        {
          case 0:
            Volver_Estado = 'b'; // pasa a revisar si se encuentra en estado Lavar
            break;
          case 1:
            Estado = 'c'; //Vuelve a Suministro de Agua 
            break;
        }
      case 'b': // Volver a Lavar
        switch(S_Lavar)
        {
          case 0:
            Volver_Estado = 'c';
            break;
          case 1:
            Estado = 'd';
            break;
        }
      case 'c': // Volver a Enjuagar
        switch(S_Enjuagar)
        {
          case 0:
            Volver_Estado = 'd';
            break;
          case 1:
            Estado = 'e'; 
            //asignar el valor de TCN0 de tal forma que al volver de la pausa, siga la cuenta en el mismo numero que se tiene
            break;
        }
      case 'd': // Volver a Centrifugar
        switch(S_Centrifugar)
        {
          case 1:
            Estado = 'f';
            break;
        }
    } 
    
    break;
  
}
}

//Interrupciones utilizadas:
//Interrupción para habilitar carga baja:
ISR(PCINT0_vect) //
      {
        t_suministro = Sb, t_lavar = Lb, t_enjuagar = Eb, t_centrigar = Cb;
        GIMSK &= ~(1<<PCIE1);//Se deshabilitan los pines para evitar un error
        GIMSK &= ~(1<<PCIE2);
        GIMSK &= ~(1<<PCIE0);
        GIMSK = GIMSK | 1<<INT0; // Se habilita el botón de Inicio para evitar una interrupción no deseada
        DDRB = DDRB | (1<<0); //Cambia el bit 0 de entrada a salida 
        PORTB = PORTB | (1<<0); //Pone la salida en 1 para que no se apague el bit luego de pulsar el botón
        
      }

//Interrupción para habilitar carga media:
ISR(PCINT1_vect) //
    {
      t_suministro = Sm, t_lavar = Lm, t_enjuagar = Em, t_centrigar = Cm;
      GIMSK &= ~(1<<PCIE0); //Se deshabilitan los pines para evitar un error
      GIMSK &= ~(1<<PCIE2);
      GIMSK &= ~(1<<PCIE1);
      GIMSK = GIMSK | 1<<INT0; // Se habilita el botón de Inicio para evitar una interrupción no deseada
      DDRA = DDRA | (1<<0); //Cambia el bit 0 de entrada a salida
      PORTA = PORTA | (1<<0); //Pone la salida en 1 para que no se apague el bit luego de pulsar el botón
    }

//Interrupción para habilitar carga alta:
ISR (PCINT2_vect) //
    {
      t_suministro = Sa, t_lavar = La, t_enjuagar = Ea, t_centrigar = Ca;
      GIMSK &= ~(1<<PCIE0); //Se deshabilitan los pines para evitar un error
      GIMSK &= ~(1<<PCIE1);
      GIMSK &= ~(1<<PCIE2);
      GIMSK = GIMSK | 1<<INT0; // Se habilita el botón de Inicio para evitar una interrupción no deseada
      DDRD = DDRD | (1<<0);//Cambia el bit 0 de entrada a salida
      PORTD = PORTD | (1<<0); //Pone la salida en 1 para que no se apague el bit luego de pulsar el botón
    }

//Interrupción para habilitar botón inicio/pausa:

ISR (INT0_vect)
  {
    switch(Accion)
    {
      
      case 0: //Inicio
      Estado = 'c';
      break;
      
      case 1: //Pausa
      Estado = 'g';
      break;
      
  
    }
    
  }

//Interrupción de Timeout de los estados:


ISR (TIMER0_COMPA_vect)
{
  
  Estado = Prox_Estado;
  TCNT0 = prox_TCNT0;
}
 


