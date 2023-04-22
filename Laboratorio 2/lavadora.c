#include <avr/io.h>
#include <avr/interrupt.h>

//Variables Importantes
unsigned char t_suministro, t_lavar, t_enjuagar, t_centrigar, Tiempo_Actual; //tiempos de cada estado
unsigned char S_Inicio, S_suministro, S_Lavar, S_Enjuagar, S_Centrifugar; //Indicadores de Estado
unsigned char Estado, Accion, Volver_Estado, Prox_Estado, Estado_Actual;
unsigned char Sb = 0b00000001, Lb = 0b00000011, Eb = 0b00000010, Cb = 0b00000011; //Tiempo en Carga Baja
unsigned char Sm = 0b00000010, Lm = 0b00000111, Em = 0b00000100, Cm = 0b00000110; //Tiempo en Carga Media
unsigned char Sa = 0b00000011, La = 0b00001010, Ea = 0b00000101, Ca = 0b00001001; //Tiempo en Carga Alta

//Números a mostrar en el display
unsigned const char numuni[]= {0b00000110, 
0b00010110, 
0b00100110, 
0b00110110, 
0b01000110, 
0b01010110, 
0b01100110, 
0b01110110, 
0b10000110, 
0b10010110};
unsigned const char numdeci[] = {0b00000110, 
0b00010110, 
0b00100110, 
0b00110110, 
0b01000110, 
0b01010110, 
0b01100110, 
0b01110110, 
0b10000110, 
0b10010110};
int timer, tiempo_estado;
//Llamado a Funciones
void estados();

//Funciones a utilizar
int main(void)
{
  
  //Habilitaci\'on de GPIO'S
  DDRD = 0b01111000;
  PORTD = 0b00000000;  
  DDRB = 0b11110110;
  PORTB = 0b00000110;
  DDRA = 0b00000010;
  PORTA = 0b00000000;

  //Habilitación de interrupciones
  MCUCR = 0b00000001; // Permite que INTO se active cada vez que haya un cambio en su entrada
  GIMSK = 0b00111000; 
  PCMSK = 0b00000001;
  PCMSK1 = 0b00000001;
  PCMSK2 = 0b00000001;

  
  // Configuración Timer Counter 1 en modo CTC con prescaler de 1024

  TCCR1A |= (1 << WGM13) | (1 << WGM12) | (0 << WGM10) | (0 << WGM11); 
  TCCR1B |= (1 << CS12) | (1 << CS10) | (0 << CS11); 
  ICR1 = (Tiempo_Actual * 1000000-1024)/2*1024; // valor de comparación deberia cambiar conforme cambia tiempo actual
  
  sei(); //habilita las interrupciones globales
    
  //Valores iniciales
  Estado = 'b';
  Accion = 0;
  Volver_Estado = 'a';
  Tiempo_Actual = 0;
  while(1){
    
    estados();
  }
}
  
//Función para mostrar en el display
int num_display(tiempo_estado)
{
    int unid;
    int dec;
    int i;
    

    //Para multiplexar el numero en decena y unidad
    for (i = tiempo_estado; i<=0; i--)
    {
      //Para multiplexar el numero en decena y unidad
      dec=tiempo_estado/10;
      unid=tiempo_estado%10;
      //PORTB &= ~(1<<2);
      PORTB &= ~(0<<1);
      PORTB |= numuni[unid];   
      TIMSK |= (1 << TOIE1);           
      PORTB |= numdeci[dec];
      PORTB &= ~(0<<2);
      //PORTB &= ~(1<<1);
    }
}


void estados(void) //procede a llamar los estados de la lavadora
{
num_display();
switch(Estado)
{ 
  case 'b': //Inicio
    TIMSK = (0 << OCIE1A);
    Accion = 0; //Permite que se de inicio al programa al pulsar el botón de inicio.
    PORTD &= ~(1<<5);
    PORTD &= ~(1<<4);
    PORTD &= ~(1<<3);
    PORTA &= ~(1<<1);
    Tiempo_Actual = t_suministro;
    break; 

    
  case 'c': //Suministro de Agua
    Tiempo_Actual = t_suministro;
    ICR1 = (Tiempo_Actual * 1000000-1024)/2*1024;
    TIMSK = (1 << OCIE1A); //habilita la interrupción
    Prox_Estado = 'd';
    S_suministro = 0, S_Lavar = 1, S_Enjuagar = 0, S_Centrifugar = 0, Accion = 1; // Estados que permiten volver de pausa
    PORTD = PORTD | (1<<5); //enciende el led correspondiente
    
    num_display(Tiempo_Actual);
    break;

  case 'd': //Lavar
    
    Tiempo_Actual = t_lavar; 
    ICR1 = (Tiempo_Actual * 1000000-1024)/2*1024;
    Prox_Estado = 'e';
    S_suministro = 0, S_Lavar = 1, S_Enjuagar = 0, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<4);
    PORTD &= ~(1<<5);
    num_display(Tiempo_Actual);
    break;

  case 'e': //Enjuagar
    Tiempo_Actual = t_enjuagar; 
    ICR1 = (Tiempo_Actual * 1000000-1024)/2*1024;
    Prox_Estado = 'f';
    S_suministro = 0, S_Lavar = 0, S_Enjuagar = 1, S_Centrifugar = 0, Accion = 1;
    PORTD = PORTD | (1<<3);
    PORTD &= ~(1<<4);
    num_display(Tiempo_Actual);
    break;

  case 'f': //Centrifugar
      
    Prox_Estado = 'b';
    S_suministro = 0, S_Lavar = 0, S_Enjuagar = 0, S_Centrifugar = 1, Accion = 1;
    Tiempo_Actual = t_centrigar;
    ICR1 = (Tiempo_Actual * 1000000-1024)/2*1024;
    PORTD &= ~(1<<3);
    PORTA = PORTA | (1<<1);
    num_display(Tiempo_Actual);
    break;

  case 'g': //Pausa 
  //Al pulsar pausa de nuevo debería volver al estado correspondiente.
    
    
    PORTD &= ~(1<<5);
    PORTD &= ~(1<<4);
    PORTD &= ~(1<<3);
    PORTA &= ~(1<<1);
    switch(Volver_Estado)
    {
      case 'a': //Volver a Suminitro de Agua
        switch(S_suministro)
        {
          case 0:
            Volver_Estado = 'b'; // pasa a revisar si se encuentra en estado Lavar
            break;
          case 1:
            Accion = 2; //Vuelve a Suministro de Agua 
            Estado_Actual = 'c';
            break;
        }
      case 'b': // Volver a Lavar
        switch(S_Lavar)
        {
          case 0:
            Volver_Estado = 'c';
            break;
          case 1:
            Accion = 2;
            Estado_Actual = 'd';
            break;
        }
      case 'c': // Volver a Enjuagar
        switch(S_Enjuagar)
        {
          case 0:
            Volver_Estado = 'd';
            
            break;
          case 1:
            Accion = 2;
            Estado_Actual = 'e';
            //asignar el valor de TCN0 de tal forma que al volver de la pausa, siga la cuenta en el mismo numero que se tiene
            break;
        }
      case 'd': // Volver a Centrifugar
        switch(S_Centrifugar)
        {
          case 1:
            Accion = 2;
            Estado_Actual = 'f';
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
      TIMSK = (0 << OCIE1A);
      break;
      
      case 2:
      Estado = Estado_Actual;
      Accion = 1;
      TIMSK = (1 << OCIE1A);
      
      
  
    }
    
  }

//Interrupción de Timeout de los estados:

ISR (TIMER1_COMPA_vect)
{
  
  Estado = Prox_Estado;
  num_display(Tiempo_Actual);
  
}

/*ISR (TIMER1_OVF_vect) //Conmutar displays
{
   
}*/




