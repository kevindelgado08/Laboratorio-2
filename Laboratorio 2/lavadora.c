#include <avr/io.h>
#include <avr/interrupt.h>


unsigned char t_suministro, t_lavar, t_enjuagar, t_centrigar; //tiempos de cada estado
//unsigned char Estado, Asignar_Carga, Inicio, Suministro_Agua, Lavar, Enjuagar, Centrifugar, Pausa;
unsigned char S_Inicio, S_suministro, S_Lavar, S_Enjuagar, S_Centrifugar;
unsigned char Estado;

unsigned char Sb = 0b00000001, Lb = 0b00000011, Eb = 0b00000010, Cb = 0b00000011;
unsigned char Sm = 0b00000010, Lm = 0b00000111, Em = 0b00000100, Cm = 0b00000110;
unsigned char Sa = 0b00000011, La = 0b00001010, Ea = 0b00000101, Ca = 0b00001001;
void estados();
void interrupciones();
int main(void)
{
  
  DDRD = 0b00110000;
 
  GIMSK = 0b01111000;
  PCMSK = 0b00000001;
  PCMSK1 = 0b00000001;
  PCMSK2 = 0b00000001; //Habilitación de Pines
  while(1){
    interrupciones();
    estados();
  }
}

void interrupciones()
{
  sei(); //habilita las interrupciones globales
}
ISR(PCINT0_vect) //
      {
        t_suministro = Sb;
        t_lavar = Lb;
        t_enjuagar = Eb; 
        t_centrigar = Cb;
        GIMSK &= ~(1<<PCIE1);//Se deshabilitan los pines para evitar un error
        GIMSK &= ~(1<<PCIE2);
        Estado = 'b';
        PORTD = 0b00110000; 
      }

ISR(PCINT1_vect) //
    {
      t_suministro = Sm;
      t_lavar = Lm;
      t_enjuagar = Em; 
      t_centrigar = Cm;
      GIMSK &= ~(1<<PCIE0); //Se deshabilitan los pines para evitar un error
      GIMSK &= ~(1<<PCIE2);
      Estado = 'b';
    }
ISR (PCINT2_vect) //
    {
      t_suministro = Sa;
      t_lavar = La;
      t_enjuagar = Ea; 
      t_centrigar = Ca;
      GIMSK &= ~(1<<PCIE0); //Se deshabilitan los pines para evitar un error
      GIMSK &= ~(1<<PCIE1);
      Estado = 'b';
    }


ISR (INT0_vect)
  {
    Estado = 'g';
  }

ISR (TIMER0_COMPA_vect)
{
  S_suministro = 0;
  Estado = 'd';
}
 

void estados(void) //procede a llamar los estados de la lavadora
{

switch(Estado)
{
  Estado = 'a';
  case 'a': //Estado inicial
    

  case 'b':
    S_Inicio = 1;
    

  case 'c':
      S_suministro = 1;  
      
  case 'd':
    S_Lavar = 1;
      
  case 'e':
    TCNT0 = t_enjuagar;
    S_Enjuagar = 1;
     
  case 'f':
    S_Centrifugar = 1;
    
  case 'g':
     break;
}
}

