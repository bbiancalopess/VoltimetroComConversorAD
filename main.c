// Programa básico para teste das configurações do ADC12

// Variáveis globais
float Tensao = 0;
char Tecla = 0;        // Contem o código ASCII da varredura do teclado

#include "io430.h"
#include "MLCD_lib.h"
#include "kb_lib.h"
#include "stdlib.h"

#define Nsamples 10

void itoa();
void itoa_min(float minimo);
void itoa_max(float maximo);

// Configura ADC12 de acordo com as orientações da Prática 20
void config_AD()
{
  // Configura P6.0 para atuar como entrada CHO do ADC12
  P6SEL |= BIT0;
  P6DIR &= ~BIT0;
  
  // Configura o ADC12
  ADC12CTL0 = ADC12ON + REFON + REF2_5V + MSC + SHT0_6; 
  /*
  - ADC12ON Liga o ADC12
  - REFON Liga o gerador de referência
  - REF2_5V Coloca a referência como 2,5V
  - MSC configura para que o trigger do tempo de amsotragem seja ativado na primeira borda de subida do sinal, 
  - no entanto, as próximas conversões são realizadas automaticamente o mais rápido possível
  - SHT0_6 Define o número de clocks do período de amostragem do ADC12CLK, nesse caso, 128
 */
  
  ADC12CTL1 = CONSEQ_2 + ADC12SSEL_1 + ADC12DIV_7 + SHP + CSTARTADD_0;
  /*
  - CONSEQ_2 Seleciona o modo da sequência de conversão, nesse caso foi selecionado para ficar repetindo a conversão de um único sinal
  - ADC12SSEL_1 seleciona a fonte do clock, nesse caso, o ACLK
  - ADC12DIV_7 seleciona a divisão do clock, nesse caso, divide por 8
  - SHP Seleciona a fonte do sinal de amostragem, nesse caso, a fonte vem do timer de amostragem, interno do CI
  - CSTARTADD_0 Esse bit seleciona qual registrador será usado para uma conversão única ou para a primeira conversão, nesse caso, o registrador 0 foi escolhido
  */
  ADC12IE |=  BIT0;    
  // ADC12IE0 Desabilita a interrupção 
  
  ADC12MCTL0 = SREF_1 + INCH_0 + EOS; 
  /*
  - SREF_1 Seleciona a referência, nesse caso, VR+ = VREF e VR- = AVSS
  - INCH_0 Seleciona a porta de entrada, nesse caso, a porta A0
  - EOS Indica a última conversão de uma sequência
  */
}

void main( void )
{  
  float Anterior = 2.7;
  float minimo = 3;
  float maximo = 0;
  
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  // Configurações dos periféricos
  Init_Display();
  Config_Display();
  tec_config_rti();
  
  // Configurando o ADC12
  config_AD();
  //habilita a interrupção
  __enable_interrupt();
  
  /*
  // Disparando as conversões
  ADC12CTL0 |= ENC; //Habilita a conversão
  ADC12CTL0 |= ADC12SC; //Inicia a conversão
  */
  
  //Configuração de amostragem inicial no display 
  EnviaString("Tensao = "); 
  EnviaComando(0x89);  // Muda para início da 2a linha
  itoa();
  
  
  EnviaComando(0x87);  // Muda para início da 2a linha
  EnviaString("Min:"); 
  EnviaComando(0xC7);  // Muda para a posição que os valores serão exibidos
  EnviaString("Max:"); 
  
  
  while(1)
  {
    if (Tecla)
    {
      switch(Tecla)
      {
        case 'A': //se a tecla A for pressionada, os valores de mínimo e máximo serão o valor da tensão atual
          {
          minimo = Tensao;
          maximo = Tensao;
          itoa_min(minimo);
          itoa_max(maximo);
          }
          break;
      
        case 'E': //se a tecla E for pressionada
          {
            if(ADC12CTL0 & ENC) //se a conversão estiver ativada
            {
              ADC12CTL0 = ~ENC; //faz um clear bit no bit que habilita a conversão, desativando-a
            }
            else //se a conversão não estiver ativada
            {
              ADC12CTL0 |= ENC; //habilita a conversão
              ADC12CTL0 |= ADC12SC; //ativa a conversão
            }
          }
          break;
      }
      Tecla = 0; //zera a tecla para que a interrupção não identifique a tecla como sempre pressionada
    }
    
    if (Anterior != Tensao) //Atualiza o valor da tensão anterior para que a tensão atual no display permaneça atualizada
    {
      Anterior = Tensao;
      itoa();
        if (minimo > Tensao) //Faz com que o valor mínimo fique sempre atualizado se a tensão mínima mudar
      {
        minimo = Tensao;
      }
      else if (maximo < Tensao) //Faz com que o valor máximo fique sempre atualizado se a tensão máxima mudar
      {
        maximo = Tensao;
      }
      else if (minimo == 0) //Faz com que, quando o valor de mínimo é zero, atualize esse valor
      {
        minimo = Tensao;
      }
      else if (maximo == 0) //Faz com que, quando o valor de máximo é zero, atualize esse valor
      {
        maximo = Tensao;
      }
      itoa_min(minimo);
      itoa_max(maximo);
    }
  }
  
}

void itoa()
{
  // Define valores de centena, dezena e unidade
  int unidade = (int) Tensao;
  int decimo = (int)((Tensao - unidade)*10);
  int centesimo = (int)((Tensao - unidade - (decimo/10.))*100);
  
   
  // Aloca cada grandeza em sua respectiva posição
  EnviaComando(0xC0);
  EnviaDado(unidade+0x30);
  EnviaComando(0xC1);
  EnviaDado(',');
  EnviaComando(0xC2);
  EnviaDado(decimo+0x30);
  EnviaComando(0xC3);
  EnviaDado(centesimo+0x30);
  EnviaComando(0xC4);
  EnviaDado('V');
}

void itoa_min(float minimo)
{
  // Define valores de centena, dezena e unidade
  int unidade = (int) minimo;
  int decimo = (int)((minimo - unidade)*10);
  int centesimo = (int)((minimo - unidade - (decimo/10.))*100);
  
   
  // Aloca cada grandeza em sua respectiva posição
  EnviaComando(0x8B);
  EnviaDado(unidade+0x30);
  EnviaComando(0x8C);
  EnviaDado(',');
  EnviaComando(0x8D);
  EnviaDado(decimo+0x30);
  EnviaComando(0x8E);
  EnviaDado(centesimo+0x30);
  EnviaComando(0x8F);
  EnviaDado('V');
}

void itoa_max(float maximo)
{
  // Define valores de centena, dezena e unidade
  int unidade = (int) maximo;
  int decimo = (int)((maximo - unidade)*10);
  int centesimo = (int)((maximo - unidade - (decimo/10.))*100);
  
   
  // Aloca cada grandeza em sua respectiva posição
  EnviaComando(0xCB);
  EnviaDado(unidade+0x30);
  EnviaComando(0xCC);
  EnviaDado(',');
  EnviaComando(0xCD);
  EnviaDado(decimo+0x30);
  EnviaComando(0xCE);
  EnviaDado(centesimo+0x30);
  EnviaComando(0xCF);
  EnviaDado('V');
}


//-------------Convert--------------------
//Interrupção para se obter uma média das conversões e obter um valor mais constante para a exibição no display
#pragma vector= ADC12_VECTOR
__interrupt void ADC12(void)
{
  static int i = 0;
  static unsigned long int acumulado = 0;
  float Media;
  
  // Faz a totalização dos valores convertidos para, em seguida, fazer a média aritmética
  acumulado += ADC12MEM0;
  
  // Faz a contagem das interrupções
  i++;
  
  if(i == Nsamples)
  {
    // Canculando a média de N amostras
    Media = acumulado / Nsamples;
    acumulado = 0; //zera a totalização dos valores convertidos
    i = 0; //zera a contagem das interrupções
    
    // Fazendo a conversão da palavra binária para tensão
    Tensao = Media*(2.5/4095); // multiplica o numero binario obtido pela minima variacao de tensao
  }
  ADC12IFG &= BIT0;    //Desabilita o flag de interrupção
}

// Interrupção da varredura do teclado
#pragma vector=PORT2_VECTOR
 __interrupt void Teclado_rti()
 {
   // Desabilita as INTs de P2
   P2IE &= ~(BIT0+BIT1+BIT2);
   // Periféricos com múltiplas fontes ressetado ao fonal da RTI (em inglês ISR = rotina de tratamento de INT)
   P2IFG &= ~(BIT0+BIT1+BIT2);
   
   __enable_interrupt(); // Reabilitação geral das INTS
   
 // Aguarda a passagem do tempo correspondente à trepidação da tecla
 // Normalmente, um tempo de delay de 5ms é suficiente.
 __delay_cycles((long)(Fclk*5e-3));


 // Passada a trepidação, só faz a varredura se alguma coluna estiver ativa.
 // Isto quer dizer que a INT ocorreu no fechamento da tecla
 if (P2IN &(BIT0+BIT1+BIT2))
 {
 Tecla = tec_rd_rti();
 }

 // Periféricos com múltiplas fontes de INT devem ter o flag correspondente
 // Resetado ao final da RTI (em inglês ISR = rotina de tratamento de INT)
 P2IFG &= ~(BIT0+BIT1+BIT2);
 
 // Reabilita as INTS de P2
 P2IE |= (BIT0+BIT1+BIT2);
 }

  