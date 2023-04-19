#include <avr/io.h>
#include <avr/interrupt.h>


int t_suministro, t_lavar, t_enjuagar, t_centrigar; //tiempos de cada estado
int Sb, Lb, Eb, Cb = 0b00000001, 0b00000011, 0b00000010, 0b00000011;
int Sm, Lm, Em, Cm = 0b00000010, 0b00000111, 0b00000100, 0b00000110;
int Sa, La, Ea, Ca = 0b00000011, 0b00001010, 0b00000101, 0b00001001;
sei(); //habilita las interrupciones globales
interrupciones();
int main(void)
{
  GIMSK, PCMSK0 , PCMSK1 , PCMSK2  = 0b01111000, 0b00000001, 0b00000001, 0b00000001; //Habilitación de Pines
  while(1){
     
     estados();
  }
}


  

int estados() //procede a llamar los estados de la lavadora
{

switch(Estado)
{
  Estado = Asignar_Carga;
  case Asignar_Carga: //Estado inicial

    ISR_BAJA(PCIF0) //al habilitarse cambia el valor de Estado a Suministro de agua
        {
          t_suministro = Sb;
          t_lavar = Lb;
          t_enjuagar = Eb; 
          t_centrigar = Cb;
          GIMSK &= ~(1<<PCIE1);//Se deshabilitan los pines para evitar un error
          GIMSK &= ~(1<<PCIE2);
          Estado = Inicio;

        }

    ISR_MEDIA(PCIF1) //al habilitarse cambia el valor de Estado a Suministro de agua
        {
          t_suministro = Sm;
          t_lavar = Lm;
          t_enjuagar = Em; 
          t_centrigar = Cm;
          GIMSK &= ~(1<<PCIE0); //Se deshabilitan los pines para evitar un error
          GIMSK &= ~(1<<PCIE2);
          Estado = Inicio;
        }
    ISR_ALTA(PCIF2) //al habilitarse cambia el valor de Estado a Suministro de agua
        {
          t_suministro = Sa;
          t_lavar = La;
          t_enjuagar = Ea; 
          t_centrigar = Ca;
          GIMSK &= ~(1<<PCIE0); //Se deshabilitan los pines para evitar un error
          GIMSK &= ~(1<<PCIE1);
          Estado = Inicio;
        }

  case Inicio:
    S_Inicio = 1;
    
      ISR_SUMINISTRO(INTF0) //al habilitarse cambia el valor de Estado a Suministro de agua
      {
        S_Inicio = 0;
        Estado = Suministro_Agua;

      }

    //break;

  case Suministro_Agua:
      S_suministro = 1;
      
      
      ISR_PAUSA(PCIF0)
      {
        Estado = Pausa;
      }

      ISR_LAVAR(OCF0A)
      {
        S_suministro = 0;
        Estado = Lavar;
      }

  case Lavar:
    S_Lavar = 1;
    
    ISR_PAUSA(PCIF0)
    {
      Estado = Pausa;
    }
    ISR_ENJUAGAR(OCF0A)
    {
      S_Lavar = 0;
      Estado = Enjuagar;
    }
    
  case Enjuagar:
    S_Enjuagar = 1;
    
    ISR_PAUSA(PCIF0)
    {
      Estado = Pausa;
    }

    ISR_CENTRIFUGAR(OCF0A)
    {
      S_Enjuagar = 0;
      Estado = Centrifugar;
    }
    
  case Centrifugar:
    S_Centrifugar = 1;
    
    ISR_PAUSA(PCIF0)
    {
      Estado = Pausa;
    }
  
    ISR_TERMINAR(OCF0A)
    {
      S_Centrifugar = 0;
      Estado = Asignar_Carga;
    }

  case Pausa:
  
    ISR_PAUSA(PCIF0)
    {
      Estado = Pausa;
    }
}
}