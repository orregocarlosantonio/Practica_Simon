/*
*****************************************************************************
**
*****************************************************************************
*/
/* Includes */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include <stm32f4xx_tim.h>
#include "stdbool.h"
#include "stdlib.h"


#define PUL_PORT 	GPIOA 										// Seleccion del puerto (PA)
#define PUL_START 	GPIO_Pin_0									// Seleccion del pin del puerto (PA0), pulsador User de la placa
#define PUL_1 		GPIO_Pin_7									// Pulsador 1 (PA1)
#define PUL_2 		GPIO_Pin_5									// Pulsador 2 (PA3)
#define PUL_3 		GPIO_Pin_3									// Pulsador 3 (PA5)
#define PUL_4 		GPIO_Pin_1									// Pulsador 4 (PA7)

#define BUZZER_PORT GPIOB										// Seleccion del puerto (PB)
#define BUZZER GPIO_Pin_1										// Seleccion del pin del puerto (PB1), Zumbador

#define LED_PORTD 		GPIOD									// Selección del puerto (PD)
#define LED_GREENP 		GPIO_Pin_12								// LED Verde
#define LED_ORANGEP 	GPIO_Pin_13								// LED Naranja
#define LED_REDP 		GPIO_Pin_14								// LED Rojo
#define LED_BLUEP 		GPIO_Pin_15								// LED Azul
#define LED_PLA (LED_GREENP| LED_ORANGEP | LED_BLUEP) 			// Matriz de 4 LEDs PLACA| LED_REDP

#define LED_PORTE 		GPIOE									// Selección del puerto (PE)
#define LED_RED 		GPIO_Pin_15								// LED Rojo
#define LED_GREEN 		GPIO_Pin_13								// LED Verde
#define LED_YELLOW 		GPIO_Pin_11								// LED Amarillo
#define LED_BLUE 		GPIO_Pin_9								// LED Azul
#define LED_EXT (LED_RED| LED_GREEN | LED_YELLOW | LED_BLUE) 	// Matriz de 4 LEDs EXT
#define LED_RG (LED_RED| LED_GREEN) 							// Matriz de 2 LEDs EXT
#define LED_YB (LED_YELLOW | LED_BLUE) 							// Matriz de 2 LEDs EXT

#define LED_ALL (LED_RED| LED_GREEN | LED_YELLOW | LED_BLUE |LED_GREENP | LED_ORANGEP | LED_BLUEP) // Matriz de LEDs EXT y PLACA | LED_REDP

// Definicion de variables para el tono

#define TONO1 600
#define TONO2 500
#define TONO3 400
#define TONO4 300

#define F_APB1 24000000 										// Frecuencia del temporizador en Hertz
#define MAX_NIVEL 8												// Maximo nivel del juego (8)

// Variables globales
int SECUENCIA[MAX_NIVEL];										// Rutina de LEDs
int USER_SECUENCIA[MAX_NIVEL];									// Array de pulsadores
int NIVEL = 1;													// Nivel inicial
int velocity = 500;												// Velocidad de respuesta (1000)
int Timer_Pul = 2000000;										// Tiempo para que el usuario ingrese la respuesta

void GPIO_Init_Game(void);										// Funcion para activar los perifericos
void delay_ms(unsigned int delay);								// Funcion de pausa en milisegundos
void delay_us(unsigned int delay);								// Funcion de pausa en microsegundos
void BEEP(uint16_t tone, uint16_t time);						// Funcion del sonido para el zumbador
void start();													// Funcion para iniciar el juego
void GENERACION_SECUENCIA(void);								// Funcion para la secuencia de numeros aleatorios
void ENT_SECUENCIA(void);										// Funcion para mostrar la secuencia de juego
void SECUENCIA_PUL_USER(void);									// Funcion para comprobar la secuencia de los pulsadores
void SECUENCIA_CORRECTA(void);									// Funcion para la secuencia correcta
void SECUENCIA_INCORRECTA(void);								// Funcion para la secuencia incorrecta
void FIN_JUEGO(void);
void InitializeTimer(void);
void Time_User(void);

int main(void)
	{
	GPIO_Init_Game();											// Se inician los perifericos

while(1)
		{
		if (NIVEL==1)
			{
			GENERACION_SECUENCIA();								// Genera secuencia inicial
			}
			ENT_SECUENCIA();									// Reproduce la array con la secuencia en los LEDs
			SECUENCIA_PUL_USER();								// Toma la secuencia de los puldadores
		}
	}

void ENT_SECUENCIA()											// Nueva secuencia de juego
	{
	uint_fast8_t led_lights[] = {LED_RED, LED_GREEN, LED_YELLOW, LED_BLUE}; 	// Array de LEDs
	uint16_t beep_tone[] = {TONO1,TONO2,TONO3,TONO4};			// Matriz de tonos
	GPIO_ResetBits (LED_PORTD, LED_PLA);						// Reset de los LEDs
	GPIO_ResetBits (LED_PORTE, LED_EXT);

	int i;
	for (i=0; i<NIVEL; i++)										// Bucle hasta completar el nivel maximo
		{
		GPIO_SetBits(LED_PORTE, led_lights[SECUENCIA[i]]);		// Secuencia de LEDs
		BEEP(beep_tone[SECUENCIA[i]],100);						// Emite sonido
		delay_ms(velocity);										// Tiempo que el LED dura encendido
		GPIO_ResetBits(LED_PORTE, led_lights[SECUENCIA[i]]);		// Reset de los LEDs
		delay_ms(1000);											// Retraso hasta la siguiente secuencia (200)
		}
	}

// Muestreo de los pulsadores
void SECUENCIA_PUL_USER()
{
	bool flag;													// Bandera para informar que un pulsador fue oprimido
	int i;
	int cont;
	cont = 0;
	for (i = 0; i < NIVEL; i++) 								// Contador de nivel
		{
		flag=false;												// Reestablece el estado de la bandera
		while(flag==false)										// Esperar hasta que la flag deje de ser falsa
			{
			GPIO_SetBits(LED_PORTD,  LED_REDP);
			if (GPIO_ReadInputDataBit(PUL_PORT,PUL_1)==SET) 	//Comprueba si oprimio el primer pulsador
				{
				GPIO_SetBits(LED_PORTE, LED_RED);				// LED correspondiente al primer pulsador
				BEEP(TONO1,100);								// Tono del primer pulsador
				USER_SECUENCIA[i]= 0;							// Guarda en la array de pulsadores
				flag=true;
				delay_ms(1000);									//tiempo para mantener el pulso ***
				if (USER_SECUENCIA[i] != SECUENCIA[i])			// Comprueba si la secuencia pulsada es correcta
					{
					SECUENCIA_INCORRECTA();						// Salta a la funcion de secuencia incorrecta
					return;
					}
				GPIO_ResetBits(LED_PORTE, LED_RED);				// Reset del LED
				}

			if (GPIO_ReadInputDataBit(PUL_PORT,PUL_2)==SET)
				{
				GPIO_SetBits(LED_PORTE,LED_GREEN);
				BEEP(TONO2,100);
				USER_SECUENCIA[i]=1;
				flag=true;
				delay_ms(1000);
				if (USER_SECUENCIA[i] != SECUENCIA[i])
					{
					SECUENCIA_INCORRECTA();
					return;
					}
				GPIO_ResetBits(LED_PORTE, LED_GREEN);
				}

			if (GPIO_ReadInputDataBit(PUL_PORT,PUL_3)==SET)
				{
				GPIO_SetBits(LED_PORTE, LED_YELLOW);
				BEEP(TONO3,100);
				USER_SECUENCIA[i]=2;
				flag=true;
				delay_ms(1000);
				if (USER_SECUENCIA[i] != SECUENCIA[i])
					{
					SECUENCIA_INCORRECTA();
					return;
					}
				GPIO_ResetBits(LED_PORTE, LED_YELLOW);
				}

			if (GPIO_ReadInputDataBit(PUL_PORT,PUL_4)==SET)
				{
				GPIO_SetBits(LED_PORTE, LED_BLUE);
				BEEP(TONO4,100);
				USER_SECUENCIA[i]=3;
				flag=true;
				delay_ms(1000);
				if (USER_SECUENCIA[i] != SECUENCIA[i])
					{
					SECUENCIA_INCORRECTA();
					return;
					}
				GPIO_ResetBits(LED_PORTE,LED_BLUE);
				}

			if (cont== Timer_Pul)								// Comprueba si la secuencia pulsada es correcta
				{
				cont=0;
				GPIO_ResetBits(LED_PORTD,  LED_REDP);
				SECUENCIA_INCORRECTA();
				return;
				}
			cont++;
			}
		}
	SECUENCIA_CORRECTA();
}

// Generación de secuencia aleatoria
void GENERACION_SECUENCIA()
	{
	start();
	int i;
	for (i = 0; i < MAX_NIVEL; i++)
		{
		SECUENCIA[i]=rand()%4; 									//Randon de 0 a 3
		}
	}

// Secuencia incorrecta
void SECUENCIA_INCORRECTA()
	{
	int i;
	for (i = 0; i < 2; i++) 									// Parpadean los LEDs tres veces indicando que la secuencia fue incorrecta
		{
		GPIO_SetBits(LED_PORTD, LED_PLA);						// Coloca a 1 los LEDs
		GPIO_SetBits(LED_PORTE, LED_EXT);
		BEEP(800,100);											// Generacion del tono para el fallo
		delay_ms(50);											// Duración del estado en 1
		BEEP(1000,200);											// Generacion del tono de fallo
		delay_ms(50);											// Duración del estado 0
		}
	velocity=1000;												// Velocidad de respuesta
	NIVEL = 1;
	return;
	}

// Secuencia correcta
void SECUENCIA_CORRECTA()
	{
	delay_ms(500);												// Tiempo de cambio entre una secuencia a otra
	if (NIVEL<=MAX_NIVEL) 										// Si no alcanza el nivel maximo, lo aumenta
		{
		NIVEL++;
		}
	if (NIVEL > MAX_NIVEL) 										// final
		{
		NIVEL = 1;
		FIN_JUEGO();
		}
	}

void FIN_JUEGO()
	{
	GPIO_SetBits(LED_PORTD,LED_REDP);
	GPIO_SetBits(LED_PORTE,LED_RED);
	BEEP(TONO1,100);
	GPIO_ResetBits(LED_PORTE,LED_RED);
	GPIO_SetBits(LED_PORTE,LED_GREEN);
	GPIO_ResetBits(LED_PORTD,LED_REDP);
	GPIO_SetBits(LED_PORTD,LED_GREENP);
	BEEP(TONO4,100);
	GPIO_ResetBits(LED_PORTE,LED_GREEN);
	GPIO_SetBits(LED_PORTE,LED_YELLOW);
	GPIO_ResetBits(LED_PORTD,LED_GREENP);
	GPIO_SetBits(LED_PORTD,LED_ORANGEP);
	BEEP(TONO2,100);
	delay_ms(300);
	GPIO_ResetBits(LED_PORTE, LED_YELLOW);
	GPIO_SetBits(LED_PORTE,LED_BLUE);
	GPIO_ResetBits(LED_PORTD, LED_ORANGEP);
	GPIO_SetBits(LED_PORTD,LED_BLUEP);
	BEEP(TONO3,100);
	GPIO_ResetBits(LED_PORTE, LED_BLUE);
	GPIO_ResetBits(LED_PORTD, LED_BLUEP);
	delay_ms(500);
	GPIO_SetBits(LED_PORTE, LED_BLUE);
	GPIO_SetBits(LED_PORTD, LED_BLUEP);
	BEEP(TONO4,100);
	GPIO_ResetBits(LED_PORTE, LED_BLUE);
	GPIO_SetBits(LED_PORTE,LED_YELLOW);
	GPIO_ResetBits(LED_PORTD, LED_BLUEP);
	GPIO_SetBits(LED_PORTD,LED_ORANGEP);
	BEEP(TONO2,100);
	GPIO_ResetBits(LED_PORTE, LED_YELLOW);
	GPIO_SetBits(LED_PORTE, LED_GREEN);
	GPIO_ResetBits(LED_PORTD, LED_ORANGEP);
	GPIO_SetBits(LED_PORTD, LED_GREENP);
	BEEP(TONO3,100);
	delay_ms(300);
	GPIO_ResetBits(LED_PORTE, LED_GREEN);
	GPIO_SetBits(LED_PORTE,LED_RED);
	GPIO_ResetBits(LED_PORTD, LED_GREENP);
	GPIO_SetBits(LED_PORTD,LED_REDP);
	BEEP(TONO1,100);
	GPIO_ResetBits(LED_PORTE,LED_RED);
	GPIO_ResetBits(LED_PORTD,LED_REDP);
	NIVEL = 1;
	delay_ms(1000);
	return;
	}


// Inicia el juego
void start()
	{
	uint16_t start_rand = 0;									// Declaración de la secuencia randon = 0
	uint_fast8_t splash[] = {LED_RED, LED_GREEN, LED_YELLOW, LED_BLUE, LED_GREENP, LED_ORANGEP, LED_REDP, LED_BLUE};// Declaración de la matriz de led (splash)
	uint8_t i=0;												// Valor inicial
	while (GPIO_ReadInputDataBit(PUL_PORT,PUL_START)==Bit_RESET)// Espera hasta que se pulse User
		{
		LED_PORTD ->BSRRL = splash[i];							// Set de la matriz de LEDs
		LED_PORTE ->BSRRL = splash[i];
		delay_ms(70);
		LED_PORTD ->BSRRH = splash[i];							// Puesta a 0 de la matris de LEDs
		LED_PORTE ->BSRRH = splash[i];
		start_rand++;											// Generación de secuencia aleatoria
		if ((i<(sizeof(splash)/sizeof(int))-1))
			{
			i++;
			}
		else
			{
			i=0;
			}
		}

// Cuando se orpime el pulsador de inicio
// Inicia la secuencia randon, sonido de bienvenida y luces iniciales
	srand(start_rand);
		GPIO_SetBits(LED_PORTD,LED_REDP);
		GPIO_SetBits(LED_PORTE,LED_RED);
		BEEP(TONO1,100);
		GPIO_ResetBits(LED_PORTE,LED_RED);
		GPIO_SetBits(LED_PORTE,LED_GREEN);
		GPIO_ResetBits(LED_PORTD,LED_REDP);
		GPIO_SetBits(LED_PORTD,LED_GREENP);
		BEEP(TONO2,100);
		GPIO_ResetBits(LED_PORTE,LED_GREEN);
		GPIO_SetBits(LED_PORTE,LED_YELLOW);
		GPIO_ResetBits(LED_PORTD,LED_GREENP);
		GPIO_SetBits(LED_PORTD,LED_ORANGEP);
		BEEP(TONO3,100);
		GPIO_ResetBits(LED_PORTE, LED_YELLOW);
		GPIO_SetBits(LED_PORTE,LED_BLUE);
		GPIO_ResetBits(LED_PORTD, LED_ORANGEP);
		GPIO_SetBits(LED_PORTD,LED_BLUEP);
		BEEP(TONO4,100);
		GPIO_ResetBits(LED_PORTE, LED_BLUE);
		GPIO_ResetBits(LED_PORTD, LED_BLUEP);
		delay_ms(500);
		GPIO_SetBits(LED_PORTE, LED_BLUE);
		GPIO_SetBits(LED_PORTD, LED_BLUEP);
		BEEP(TONO4,100);
		GPIO_ResetBits(LED_PORTE, LED_BLUE);
		GPIO_SetBits(LED_PORTE,LED_YELLOW);
		GPIO_ResetBits(LED_PORTD, LED_BLUEP);
		GPIO_SetBits(LED_PORTD,LED_ORANGEP);
		BEEP(TONO3,100);
		GPIO_ResetBits(LED_PORTE, LED_YELLOW);
		GPIO_SetBits(LED_PORTE, LED_GREEN);
		GPIO_ResetBits(LED_PORTD, LED_ORANGEP);
		GPIO_SetBits(LED_PORTD, LED_GREENP);
		BEEP(TONO2,100);
		GPIO_ResetBits(LED_PORTE, LED_GREEN);
		GPIO_SetBits(LED_PORTE,LED_RED);
		GPIO_ResetBits(LED_PORTD, LED_GREENP);
		GPIO_SetBits(LED_PORTD,LED_REDP);
		BEEP(TONO1,100);
		GPIO_ResetBits(LED_PORTE,LED_RED);
		GPIO_ResetBits(LED_PORTD,LED_REDP);
		delay_ms(1000);
		GPIO_ResetBits(LED_PORTE,LED_EXT);
		delay_ms(1000);
	}

// Funcion para retardos en milisegundos
void delay_ms(unsigned int delay)
	{
    TIM7->PSC = F_APB1/1000+1;
    TIM7->ARR = delay;
    TIM7->EGR |= TIM_EGR_UG;
    TIM7->CR1 |= TIM_CR1_CEN|TIM_CR1_OPM;
    while ((TIM7->CR1) & (TIM_CR1_CEN!=0));
	}

// Funcion para retardos en microsegundos
void delay_us(unsigned int delay)
	{
    TIM7->PSC = F_APB1/1000000+1;
    TIM7->ARR = delay;
    TIM7->EGR |= TIM_EGR_UG;
    TIM7->CR1 |= TIM_CR1_CEN|TIM_CR1_OPM;
    while ((TIM7->CR1) & (TIM_CR1_CEN!=0));
	}

// Función del zumbador
void BEEP(uint16_t tone, uint16_t time)								//Tono, Tiempo
	{
	uint16_t j;
	for (j = 0; j < time; ++j)
		{
		BUZZER_PORT ->BSRRL = BUZZER;
		delay_us(tone);
		BUZZER_PORT ->BSRRH = BUZZER;
		delay_us(tone);
		}
	}

// Configuración y activación de los perifericos
void GPIO_Init_Game(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;								// Definición de type

	  // Habilita los perifericos necesarios en el puerto D
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

	  // Establece el pin de los pulsadores en el GPIO PA
	  GPIO_InitStruct.GPIO_Pin = PUL_START|PUL_1|PUL_2|PUL_3|PUL_4;
	  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	  GPIO_Init(PUL_PORT, &GPIO_InitStruct);

	  // Establece el pin del Zumbador en el GPIO PB
	  GPIO_InitStruct.GPIO_Pin = BUZZER;
	  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	  GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
	  GPIO_ResetBits(GPIOB, BUZZER);


	  // Establece el pin los LEDs en el GPIOD
	  GPIO_InitStruct.GPIO_Pin = LED_PLA | LED_REDP;
	  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	  //GPIO_Init(GPIOD, &GPIO_InitStruct); // Esta o la siguiente linea es valida
	  GPIO_Init(LED_PORTD, &GPIO_InitStruct);
	  GPIO_ResetBits(GPIOD, LED_PLA | LED_REDP);

	  // Establece el pin los LEDs en el GPIOE
	  GPIO_InitStruct.GPIO_Pin = LED_EXT;
	  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	  //GPIO_Init(GPIOD, &GPIO_InitStruct); // Esta o la siguiente linea es valida
	  GPIO_Init(LED_PORTE, &GPIO_InitStruct);
	  GPIO_ResetBits(GPIOE, LED_EXT);



  // Habilita temporizadores
  RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;

  GPIO_ResetBits(LED_PORTD,LED_PLA);
  GPIO_ResetBits(LED_PORTE,LED_EXT);
}

/*
 * Callback used by stm32f4_discovery_audio_codec.c.
 * Refer to stm32f4_discovery_audio_codec.h for more info.
 */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){
  /* TODO, implement your code here */
  return;
}

/*
 * Callback used by stm324xg_eval_audio_codec.c.
 * Refer to stm324xg_eval_audio_codec.h for more info.
 */
uint16_t EVAL_AUDIO_GetSampleCallBack(void){
  /* TODO, implement your code here */
  return -1;
}
