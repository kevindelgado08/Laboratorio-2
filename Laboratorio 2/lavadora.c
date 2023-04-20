#include <avr/io.h>
#include <avr/interrupt.h>

//Variables Importantes
unsigned char t_suministro, t_lavar, t_enjuagar, t_centrigar, prox_TCNT0; //tiempos de cada estado
unsigned char S_Inicio, S_suministro, S_Lavar, S_Enjuagar, S_Centrifugar; //Indicadores de Estado
unsigned char Estado, Accion, Volver_Estado, Prox_Estado;
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
  PORTB = 0b11110010;
  DDRA = 0b00000010;
  PORTA = 0b00000000;

  //Habilitación de interrupciones
  MCUCR = 0b00000001; // Permite que INTO se active cada vez que haya un cambio en su entrada
  GIMSK = 0b00111000; 
  PCMSK = 0b00000001;
  PCMSK1 = 0b00000001;
  PCMSK2 = 0b00000001;

  //Valores iniciales
  Estado = 'b';
  Accion = 0;
  Volver_Estado = 'a';
  while(1){
    
    estados();
  }
}

void estados(void) //procede a llamar los estados de la lavadora
{

switch(Estado)
{ 
  case 'b': //Inicio
    Accion = 0;
    TCN0 = t_suministro;
    break; 

    
  case 'c': //Suministro de Agua

    S_suministro = 0, S_Lavar = 1, S_Enjuagar = 0, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<5);
    prox_TCNT0 = t_lavar;  
    Prox_Estado = 'd';  
      break;

  case 'd': //Lavar
    S_suministro = 0, S_Lavar = 1, S_Enjuagar = 0, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<4);
    PORTD &= ~(1<<5);
    prox_TCNT0 = t_enjuagar;  
    Prox_Estado = 'e';

    break;

  case 'e': //Enjuagar
    S_suministro = 0, S_Lavar = 0, S_Enjuagar = 1, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<3);
    PORTD &= ~(1<<4);
    break;

  case 'f': //Centrifugar
    S_suministro = 0, S_Lavar = 0, S_Enjuagar = 0, S_Centrifugar = 1, Accion = 1;
    PORTA = PORTA | (1<<1);
    PORTD &= ~(0<<3);
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


switch(Tiempos)
  {
    case 'x': //Tiempo de Suministro de Agua

      break;
    case 'y': //Tiempo de Lavar
      break;
    case 'w': //Tiempo de Enjuagar
      break;
    case 'z': //Tiempo de Centrifugar
      break;
  } 




//Interrupción de Timeout de los estados:




ISR (TIMER0_COMPA_vect)
{
  Estado = Prox_Estado;
  TCN0 = prox_TCNT0;
}
 


