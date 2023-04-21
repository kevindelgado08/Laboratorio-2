#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//Variables Importantes
unsigned char t_suministro, t_lavar, t_enjuagar, t_centrigar, Tiempo_Actual; //tiempos de cada estado
unsigned char S_Inicio, S_suministro, S_Lavar, S_Enjuagar, S_Centrifugar; //Indicadores de Estado
unsigned char Estado, Accion, Volver_Estado, Prox_Estado, prox_TCNT0, unid, dec, i, tiempo_estado;
unsigned char Sb = 0b00000001, Lb = 0b00000011, Eb = 0b00000010, Cb = 0b00000011; //Tiempo en Carga Baja
unsigned char Sm = 0b00000010, Lm = 0b00000111, Em = 0b00000100, Cm = 0b00000110; //Tiempo en Carga Media
unsigned char Sa = 0b00000011, La = 0b00001010, Ea = 0b00000101, Ca = 0b00001001; //Tiempo en Carga Alta
int timer;
 
//Llamado a Funciones
void estados();

//Funciones a utilizar
int main(void)
{
  
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

  
  // Configurar Timer Counter 0 en modo CTC con prescaler de 1024

  
  TCCR1A |= (1 << WGM13) | (1 << WGM12) | (0 << WGM10) | (0 << WGM11); // modo CTC
  TCCR1B |= (1 << CS12) | (1 << CS10) | (0 << CS11); // prescaler de 1024
  ICR1 = 30000; // valor de comparación
  
  sei(); //habilita las interrupciones globales
    
  //Valores iniciales
  Estado = 'b';
  Accion = 0;
  Volver_Estado = 'a';
  while(1){
    
    estados();
  }
}
  
//Función para mostrar en el display
void num_display(tiempo_estado)
{
   
    

    //Para multiplexar el numero en decena y unidad
    for (i = tiempo_estado; i<=0; i--)
    {
      dec =i/10;
      unid =i%10;
      unid = unid << 4;
      dec = dec << 4;

      PORTB &= ~(0<<1);
      PORTB = PORTB | (unid); 
      TIMSK = (1 << OCIE1A);  
      PORTB &= ~(0<<1);
      TIMSK = (0 << OCIE1A);
      PORTD &= ~(0<<6);           
      PORTB = PORTB | (dec);
      TIMSK = (1 << OCIE1A);
      PORTD &= ~(0<<6);
      TIMSK = (0 << OCIE1A);

    }
}/*
ISR (TIMER1_COMPA_vect)
{
  if(timer == 0)
  {
    Estado = Prox_Estado;
  }
  timer = Tiempo_Actual;
  
  Tiempo_Actual--;
  timer--;

  
}
*/
//Version Original
ISR (TIMER1_COMPA_vect)
{

  num_display(Tiempo_Actual);
  
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
    Tiempo_Actual = t_suministro; 
    
    S_suministro = 0, S_Lavar = 1, S_Enjuagar = 0, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<5);
    num_display(Tiempo_Actual);
    Estado = 'd';
    ;  
    
    
      break;

  case 'd': //Lavar
    Tiempo_Actual = t_lavar;  
    Prox_Estado = 'e';
    S_suministro = 0, S_Lavar = 1, S_Enjuagar = 0, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<4);
    PORTD &= ~(1<<5);
    

    break;

  case 'e': //Enjuagar
    Tiempo_Actual = t_enjuagar;  
    Prox_Estado = 'f';
    S_suministro = 0, S_Lavar = 0, S_Enjuagar = 1, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<3);
    PORTD &= ~(1<<4);
    
    break;

  case 'f': //Centrifugar
    Tiempo_Actual = t_enjuagar;  
    Prox_Estado = 'b';
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





