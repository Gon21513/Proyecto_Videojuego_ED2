//Luis Pedro Gonzalez 21513
//Gabriel Carrera 21216
//Proyecto 2, Frog Smashers

//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include <ili9341_8_bits_fast.h> //Librería para la pantalla con chip ILI9341
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
//#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"
#include <SPI.h> //Librería para la comunicacion SPI
#include <SD.h> //Librería para comunicación con la memoria microSD

File myFile; //Definimos objeto de la librería


#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};  

//--------------generales-----------------
#define frogswidth 25
#define frogsheight 22

#define MENU 29
#define BATTLE 9
#define WINNER 10
#define SONGSELECT 28




//---------------------------------------posiciones --------------------------------------


int posXf1 = 25; //posicion de inicio en x de frog1
int posX2f1;
int posYf1 = 193; // posición vertical inicial para el salto 
int posXf2 = 230; //posicion de inicio en x de frog2
int posX2f2;
int posYf2 = 193; // posición vertical inicial para el salto 


int8_t  movimiento =  0; // = quieto, 1 = derecha, -1 = izquierda
int8_t  movimientof2 =  0; // = quieto, 1 = derecha, -1 = izquierda

uint8_t alturaActual = 193;  // mapae la altura del persnaje
uint8_t alturaActualf2 = 193;  // mapae la altura del persnaje

  

// Medidas de plataformas
  // altura de la plataforma
#define platformHeight 169
#define platformStartX 74
#define platformEndX 250
#define platform2Height 129
#define platform2StartX 114
#define platform2EndX 210





//--------------------------------------colores-----------------------------------------

#define fillmovecolor 0xFDF1 //color con el que se rellena cuando el perosnje borra 
extern uint8_t platlow[]; //tile de la primera plataforma
extern uint8_t vigap1low[]; //tile de madera en la primera plataforma 
extern uint8_t viga2p1low[]; //tile madera 2 en primera plataforma
extern uint8_t plat2high[]; // tile de la segunda plataforma
extern uint8_t viga1p2high[]; //tile madera 1 en plat high 
extern uint8_t viga2p2high[];// madera 2 plat high 
extern uint8_t jumpfrog1[];//animacion de salto frog 1
extern uint8_t frogcol1[];//anamacion de colision frog1
extern uint8_t frogcol2[];//anamicaion de colision frog2


//-------------FROG1
//antirrebote para el salto
bool isJumping = false; //chequea si se esta saltando 
uint16_t  lastJumpTime = 0; //banddera para saber cuando salto
const uint16_t  jumpDebounceTime = 2000; // Tiempo de anti-rebote en milisegundos

//antirrebote para el bateo
bool isBatting = false; // Variable para revisar si se está bateando
uint16_t  lastBatTime = 0; // Tiempo del último bateo
const uint16_t  batDebounceTime = 1050; // Tiempo de anti-rebote para el bateo

//------------------FROG2
//antirrebote para el salto
bool isJumpingf2 = false; //chequea si se esta saltando 
uint16_t  lastJumpTimef2 = 0; //banddera para saber cuando salto
const uint16_t  jumpDebounceTimef2 = 2000; // Tiempo de anti-rebote en milisegundos

//antirrebote para el bateo
bool isBattingf2 = false; // Variable para revisar si se está bateando
uint16_t  lastBatTimef2 = 0; // Tiempo del último bateo
const uint16_t  batDebounceTimef2 = 1050; // Tiempo de anti-rebote para el bateo


//antirrebote de caida 
//---------------FROG1----------------
uint16_t lastFrameUpdate = 0;
const uint16_t frameDuration = 5;  // Duración de cada frame en milisegundos
bool enElAire = false;

//--------------FROG2----------------------------------------
uint16_t lastFrameUpdatef2 = 0;
const uint16_t frameDurationf2 = 5;  // Duración de cada frame en milisegundos
bool enElAiref2 = false;


//-----------------------VIDAS DE LA FROGS------------------

int vidasFrog1 = 10;
int vidasFrog2 = 10;

int vidasAnterioresFrog1 = vidasFrog1;
int vidasAnterioresFrog2 = vidasFrog2;

//-------------colisiones
bool animacionColisionActiva = false; // Indica si la animación de colisión está activa
uint16_t tiempoInicioAnimacion;       // Almacena el momento en que comenzó la animación
uint8_t frameActualAnimacion = 0;     // Almacena el frame actual de la animació

bool animacionColisionActivaFrog1 = false;
uint16_t tiempoInicioAnimacionFrog1 = 0;
uint8_t frameActualAnimacionFrog1 = 0;


// bandera para revisar si el menu debe estar mostrandose o no
int menuflag = 0;

char gamemode; //Variable para validar cambio de modos

//**************************************************************

//************************************************************

//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************

int asciitohex (int a); //Función para pasar ASCII a valor hexadecimal
void mapeoSD(); //Función para mostrar imagen de la memoria microSD


//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {


  pinMode(PA_3, OUTPUT); //Se define PA_3 como salida y se convierte en slave select

  pinMode(MENU, OUTPUT); //Se define PA_5 como salida para selector de musica
  pinMode(BATTLE, OUTPUT); //Se define PA_6 como salida para selector de musica
  pinMode(WINNER, OUTPUT); //Se define PA_7 como salida para selector de musica
  pinMode(SONGSELECT, OUTPUT); //Se define PA_7 como salida para selector de musica

  digitalWrite(MENU, LOW); // INICIAR CON TODAS LAS CANCIONES APAGADAS - es mejor iniciar la cancion del menu antes de dibujarlo
  digitalWrite(BATTLE, LOW);
  digitalWrite(WINNER, LOW);
  digitalWrite(SONGSELECT, LOW);
  digitalWrite(SONGSELECT, HIGH);
  
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(115200);//comunicacion con la pantalla lcd

  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
   Serial.println("Iniciando pantalla...");
  SPI.setModule(0); //Utilizar el SPI Module 0

  
  lcdInit();
  lcdClear(0x00);

//---------------------------configuracion para el uart-----------------------------------------
  Serial2.begin(9600); //iniica la velocidad del uart 2 
  Serial3.begin(9600); //iniica la velocidad del uart 2 

//-------------------------------------------------------------------------------------

 if (!SD.begin(PA_3)){ //Verificar que el slave select este en 0, y se responda un 1
    Serial.println("initialization failed!"); //Si no falló la inicialización
    return;
  }
  Serial.println("initialization done."); //Se inició la comunicación

//---------------------configuracion de pantalla--------------------------------
  //extern uint8_t back1[];
  //LCD_Bitmap(0, 0, 320, 240, back1);
  menuflag = 1;
  //LCD_Print("Vidas Frog1:", 0, 0, 2, 0xFFFF, 0x0000);
  //LCD_Print("Vidas Frog2:", 0, 1, 2, 0xFFFF, 0x0000);
  myFile = SD.open("menuinit.txt");
  mapeoSD();
  while (menuflag == 1){
    //myFile = SD.open("menuinit.txt");
    //mapeoSD();
    digitalWrite(MENU, HIGH);
    digitalWrite(BATTLE, LOW);
    digitalWrite(WINNER, LOW);
    digitalWrite(SONGSELECT, LOW);
    digitalWrite(SONGSELECT, HIGH);
    gamemode = Serial.read(); //Leer la respuesta del usuario
    if (gamemode == 'r'){
      menuflag = 2;
    }
  }
  myFile = SD.open("bg_game.txt");
  mapeoSD();
}

//-----------------------------------FUNCION DEL SALTO FROG1---------------------------------------------------
void saltar() {
  enElAire = true; // Al iniciar el salto, el personaje está en el aire
  alturaActual = posYf1;  // Actualiza la altura actual antes de iniciar el salto
  int alturaSalto = 84;  // Define la altura total del salto
  int avanceHorizontalf1 = 1;  // Define cuántos píxeles se moverá horizontalmente en cada paso del salto
  bool landed = false;  // Variable para rastrear si el personaje ha aterrizado en la plataforma
  isJumping = true;  // Activa la bandera de salto

  // Fase de subida del salto
  for (int j = 0; j < alturaSalto / 2; j++) { 
    FillRect(posXf1, posYf1, frogswidth+1, frogsheight, fillmovecolor);  // Borra el sprite anterior dibujando un rectángulo del color de fondo
    posYf1--;  // Decrementa la posición vertical para mover el sprite hacia arriba
    if (movimiento == 1 && posXf1 < 320 - 26) {  // Si el movimiento es a la derecha y no se ha llegado al límite derecho
      posXf1 += avanceHorizontalf1;  // Incrementa la posición horizontal
    } else if (movimiento == -1 && posXf1 > 0) {  // Si el movimiento es a la izquierda y no se ha llegado al límite izquierdo
      posXf1 -= avanceHorizontalf1;  // Decrementa la posición horizontal
    }
    for(uint16_t i = 0; i < 3; i++) { 
       LCD_Sprite(posXf1, posYf1, frogswidth, frogsheight, jumpfrog1, 5, i, movimiento == -1, 0); // Dibuja el sprite de salto correspondiente
        delay(5);  // Introduce un pequeño retardo para visualizar el sprite
    }
  }

  // Fase de bajada del salto
  for (int j = 0; j < alturaSalto / 2; j++) {
   
    // Verifica la colisión con las plataformas en la fase descendente del salto
    if ((posYf1 + frogsheight >= platformHeight && posYf1 + frogsheight <= platformHeight + 5) || 
        (posYf1 + frogsheight >= platform2Height && posYf1 + frogsheight <= platform2Height + 5)) {
       // Serial.println("Colisión detectada!");
        if (posYf1 + frogsheight >= platformHeight && posYf1 + frogsheight <= platformHeight + 5) {
            posYf1 = platformHeight - frogsheight;  // Ajusta la posición Y del personaje para que esté sobre la primera plataforma
            alturaActual = platformHeight - frogsheight;  // Actualiza la altura actual
        } else {
            posYf1 = platform2Height - frogsheight;  // Ajusta la posición Y del personaje para que esté sobre la segunda plataforma
            alturaActual = platform2Height - frogsheight;  // Actualiza la altura actual
        }
        landed = true;  // Indica que el personaje ha aterrizado en una plataforma
        break;  // Termina el bucle, ya que el personaje ha aterrizado en una plataforma
    }
    
    FillRect(posXf1, posYf1, frogswidth, frogsheight, fillmovecolor);  // Borra el sprite anterior
    posYf1++;  // Incrementa la posición vertical para mover el sprite hacia abajo
    if (movimiento == 1 && posXf1 < 320-26) {  // Si el movimiento es a la derecha y no se ha llegado al límite derecho
      posXf1 += avanceHorizontalf1;  // Incrementa la posición horizontal
    } else if (movimiento == -1 && posXf1 > 0) {  // Si el movimiento es a la izquierda y no se ha llegado al límite izquierdo
      posXf1 -= avanceHorizontalf1;  // Decrementa la posición horizontal
    }
    for(uint16_t i = 2; i < 5; i++) { 
        LCD_Sprite(posXf1, posYf1, frogswidth, frogsheight, jumpfrog1, 5, i, movimiento == -1, 0);   // Dibuja el sprite de bajada correspondiente
        delay(5);  // Introduce un pequeño retardo para visualizar el sprite
    }
  }

  // Finalización del salto
  FillRect(posXf1, posYf1, frogswidth, frogsheight, fillmovecolor);  // Borra el sprite anterior una vez que el salto ha finalizado

  if (landed) {
    enElAire = false; // Si el personaje ha aterrizado en una plataforma, restablece enElAire
  }

  isJumping = false;  // Desactiva la bandera de salto

  // Dibuja el sprite original después de finalizar el salto, con o sin flip dependiendo de la dirección
  if (movimiento == 1) {  // Si el movimiento fue a la derecha
    LCD_Sprite(posXf1+1, posYf1, frogswidth, frogsheight, runf1, 4, 0, 0, 0);  // Dibuja el sprite en reposo sin flip
  } else if (movimiento == -1) {  // Si el movimiento fue a la izquierda
    LCD_Sprite(posXf1, posYf1, frogswidth, frogsheight, runf1, 4, 0, 1, 0);  // Dibuja el sprite en reposo con flip
  }
} 

//--------------------------------FIN FUNCION DE SALTO FROG 1-----------------------------------------------------

//-----------------------------------FUNCION DEL SALTO FROG2---------------------------------------------------
void saltarf2() {
  enElAiref2 = true; // Al iniciar el salto, el personaje está en el aire
  alturaActualf2 = posYf2;  // Actualiza la altura actual antes de iniciar el salto
  int alturaSalto = 84;  // Define la altura total del salto
  int avanceHorizontalf2 = 1;  // Define cuántos píxeles se moverá horizontalmente en cada paso del salto
  bool landed2 = false;  // Variable para rastrear si el personaje ha aterrizado en la plataforma
  isJumpingf2 = true;  // Activa la bandera de salto

  // Fase de subida del salto
  for (int j = 0; j < alturaSalto / 2; j++) { 
    FillRect(posXf2, posYf2, frogswidth+1, frogsheight, fillmovecolor);  // Borra el sprite anterior
    posYf2--;  // Decrementa la posición vertical
    if (movimientof2 == 1 && posXf2 < 320 - 26) {  // Derecha
      posXf2 += avanceHorizontalf2;  
    } else if (movimientof2 == -1 && posXf2 > 0) {  // Izquierda
      posXf2 -= avanceHorizontalf2;  
    }
    for(uint16_t i = 0; i < 3; i++) { 
       LCD_Sprite(posXf2, posYf2, frogswidth, frogsheight, jumpfrog2, 5, i, movimientof2 == -1, 0);
        delay(5);  
    }
  }

  // Fase de bajada del salto
  for (int j = 0; j < alturaSalto / 2; j++) {
    if ((posYf2 + frogsheight >= platformHeight && posYf2 + frogsheight <= platformHeight + 5) || 
        (posYf2 + frogsheight >= platform2Height && posYf2 + frogsheight <= platform2Height + 5)) {
        if (posYf2 + frogsheight >= platformHeight && posYf2 + frogsheight <= platformHeight + 5) {
            posYf2 = platformHeight - frogsheight;
            alturaActualf2 = platformHeight - frogsheight;
            landed2 = true;
            break;
        } else {
            posYf2 = platform2Height - frogsheight;
            alturaActualf2 = platform2Height - frogsheight;
            landed2 = true;
            break;
        }
    }
    
    FillRect(posXf2, posYf2, frogswidth, frogsheight, fillmovecolor);  // Borra el sprite anterior
    posYf2++;  // Incrementa la posición vertical
    if (movimientof2 == 1 && posXf2 < 320-26) {  // Derecha
      posXf2 += avanceHorizontalf2;  
    } else if (movimientof2 == -1 && posXf2 > 0) {  // Izquierda
      posXf2 -= avanceHorizontalf2;  
    }
    for(uint16_t i = 2; i < 5; i++) { 
        LCD_Sprite(posXf2, posYf2, frogswidth, frogsheight, jumpfrog2, 5, i, movimientof2 == -1, 0);
        delay(5);  
    }
  }

  if (landed2) {
    enElAiref2 = false; // Si el personaje ha aterrizado en una plataforma, restablece enElAiref2
  }

  isJumpingf2 = false;  // Desactiva la bandera de salto

  // Dibuja el sprite original después de finalizar el salto, con o sin flip dependiendo de la dirección
  if (movimientof2 == 1) {  // Derecha
    LCD_Sprite(posXf2+1, posYf2, frogswidth, frogsheight, runf2, 4, 0, 0, 0);
  } else if (movimientof2 == -1) {  // Izquierda
    LCD_Sprite(posXf2, posYf2, frogswidth, frogsheight, runf2, 4, 0, 1, 0);
  }
}




//--------------------------------FIN FUNCION DE SALTO FROG 2-----------------------------------------------------

//-------------------------------------FUNCION DE PLATAFORMA-------------------------------------------

bool chequearPlataformaf1() {
    return (
        // Chequea la primera plataforma
        (posXf1 + frogswidth >= platformStartX && posXf1 <= platformEndX && alturaActual == platformHeight - frogsheight) ||
        // Chequea la segunda plataforma
        (posXf1 + frogswidth >= platform2StartX && posXf1 <= platform2EndX && alturaActual == platform2Height - frogsheight)
    );
}

void caerf1() {
    enElAire = true;  // El personaje está en el aire
    uint8_t altinicial = 193;
    unsigned long currentMillis = millis();  // Obtiene el tiempo actual

    // Si el personaje aún no ha llegado al suelo o a una plataforma
    if (posYf1 < altinicial) {
        // Chequear colisión con la primera plataforma
        if (posXf1 + frogswidth >= platformStartX && posXf1 <= platformEndX &&
            posYf1 + frogsheight >= platformHeight && posYf1 + frogsheight <= platformHeight + 5) {
            alturaActual = platformHeight - frogsheight;  // Ajusta la altura actual
            posYf1 = alturaActual;  // Ajusta la posición Y del personaje
            enElAire = false;  // El personaje ha aterrizado
            return;  // Sale de la función
        }
        
        // Chequear colisión con la segunda plataforma
        if (posXf1 + frogswidth >= platform2StartX && posXf1 <= platform2EndX &&
            posYf1 + frogsheight >= platform2Height && posYf1 + frogsheight <= platform2Height + 5) {
            alturaActual = platform2Height - frogsheight;  // Ajusta la altura actual
            posYf1 = alturaActual;  // Ajusta la posición Y del personaje
            enElAire = false;  // El personaje ha aterrizado
            return;  // Sale de la función
        }

        if (currentMillis - lastFrameUpdate >= frameDuration) {  // Verifica si ha pasado el tiempo de duración de un frame
            posYf1++;  // Incrementa la posición vertical para mover el sprite hacia abajo
            FillRect(posXf1, posYf1 - frogsheight, frogswidth, frogsheight, fillmovecolor);  // Borra el sprite anterior

            for (uint16_t i = 2; i < 5; i++) {
                LCD_Sprite(posXf1, posYf1, frogswidth, frogsheight, jumpfrog1, 5, i, movimiento == -1, 0);  // Dibuja el sprite de caída correspondiente
            }
            lastFrameUpdate = currentMillis;  // Actualiza la última vez que se cambió el frame
        }
    } else {
        alturaActual = posYf1;  // Actualiza la altura actual una vez que el personaje ha llegado al suelo (solo si no ha aterrizado en una plataforma)
        enElAire = false;  // El personaje ha aterrizado
    }
}




//-----------------------PLATAFORMA FROG2------------------------------
bool chequearPlataformaf2() {
    return (
        // Chequea la primera plataforma
        (posXf2 + frogswidth >= platformStartX && posXf2 <= platformEndX && alturaActualf2 == platformHeight - frogsheight) ||
        // Chequea la segunda plataforma
        (posXf2 + frogswidth >= platform2StartX && posXf2 <= platform2EndX && alturaActualf2 == platform2Height - frogsheight)
    );
}

void caerf2() {
    enElAiref2 = true;  // El personaje Frog2 está en el aire
    uint8_t altinicial = 193;
    unsigned long currentMillisf2 = millis();  // Obtiene el tiempo actual

    // Si el personaje Frog2 aún no ha llegado al suelo o a una plataforma
    if (posYf2 < altinicial) {
        // Chequear colisión con la primera plataforma
        if (posXf2 + frogswidth >= platformStartX && posXf2 <= platformEndX &&
            posYf2 + frogsheight >= platformHeight && posYf2 + frogsheight <= platformHeight + 5) {
            alturaActualf2 = platformHeight - frogsheight;  // Ajusta la altura actual de Frog2
            posYf2 = alturaActualf2;  // Ajusta la posición Y del personaje Frog2
            enElAiref2 = false;  // El personaje Frog2 ha aterrizado
            return;  // Sale de la función
        }
        
        // Chequear colisión con la segunda plataforma
        if (posXf2 + frogswidth >= platform2StartX && posXf2 <= platform2EndX &&
            posYf2 + frogsheight >= platform2Height && posYf2 + frogsheight <= platform2Height + 5) {
            alturaActualf2 = platform2Height - frogsheight;  // Ajusta la altura actual de Frog2
            posYf2 = alturaActualf2;  // Ajusta la posición Y del personaje Frog2
            enElAiref2 = false;  // El personaje Frog2 ha aterrizado
            return;  // Sale de la función
        }

        if (currentMillisf2 - lastFrameUpdatef2 >= frameDurationf2) {  // Verifica si ha pasado el tiempo de duración de un frame para Frog2
            posYf2++;  // Incrementa la posición vertical de Frog2 para mover el sprite hacia abajo
            FillRect(posXf2, posYf2 - frogsheight, frogswidth, frogsheight, fillmovecolor);  // Borra el sprite anterior de Frog2

            for (uint16_t i = 2; i < 5; i++) {
                LCD_Sprite(posXf2, posYf2, frogswidth, frogsheight, jumpfrog2, 5, i, movimientof2 == -1, 0);  // Dibuja el sprite de caída correspondiente para Frog2
            }
            lastFrameUpdatef2 = currentMillisf2;  // Actualiza la última vez que se cambió el frame para Frog2
        }
    } else {
        alturaActualf2 = posYf2;  // Actualiza la altura actual de Frog2 una vez que el personaje ha llegado al suelo (solo si no ha aterrizado en una plataforma)
        enElAiref2 = false;  // El personaje Frog2 ha aterrizado
    }
}
//-----------------------PLATAFORMA FROG2------------------------------



//-------------------------------FUNCION DE COLISION----------------------------------

bool Colision() {
    // Lados del rectángulo de frog1
    int left1 = posXf1;
    int right1 = posXf1 + frogswidth;
    int top1 = posYf1;
    int bottom1 = posYf1 + frogsheight;

    // Lados del rectángulo de frog2
    int left2 = posXf2;
    int right2 = posXf2 + frogswidth;
    int top2 = posYf2;
    int bottom2 = posYf2 + frogsheight;

    // Comprueba si hay colisión
    if (left1 < right2 && right1 > left2 && top1 < bottom2 && bottom1 > top2) {
        return true;  // Hay colisión
    }
    return false;  // No hay colisión
}

//-------------------animaciones de colision--------------------------------




//------------------------------------------------------------------------------------

//------------------------------------------FIN FUNCION DE PLATAFORMA---------------------------------
//***************************************************************************************************************************************
// Loop Infinito//
//***************************************************************************************************************************************
void loop() {
  while (menuflag == 2){

    //Iniciar la cancion de batalla
    digitalWrite(MENU, LOW);
    digitalWrite(BATTLE, HIGH);
    digitalWrite(WINNER, LOW);
    digitalWrite(SONGSELECT, LOW);
    digitalWrite(SONGSELECT, HIGH);
    
    uint16_t currentTime = millis();

//marcadores 
    // Verifica si las vidas de Frog1 han cambiado
    if (vidasFrog1 != vidasAnterioresFrog1) {
        LCD_Print("F1: " + String(vidasFrog1), 1, 2, 2, 0xFFFF, 0x0000);
        vidasAnterioresFrog1 = vidasFrog1; // Actualiza las vidas anteriores
    }


    // Verifica si las vidas de Frog2 han cambiado
    if (vidasFrog2 != vidasAnterioresFrog2) {
        LCD_Print("F2: " + String(vidasFrog2), 3, 20, 2, 0xFFFF, 0x0000); // Ajusta la posición y según dónde quieras mostrarlo
        vidasAnterioresFrog2 = vidasFrog2; // Actualiza las vidas anteriores
    }


    if (!chequearPlataformaf1() && !isJumping) {
      caerf1();  // Si el personaje está fuera del rango de la plataforma y no está saltando, llamar a la función caer
    }
  
    if (!chequearPlataformaf2() && !isJumpingf2) {
      caerf2();  // Si el personaje está fuera del rango de la plataforma y no está saltando, llamar a la función caer
    }
  
      // Manejo de la animación de colisión
      if (animacionColisionActiva) {
          if (millis() - tiempoInicioAnimacion > 100) {  // Si ha pasado 100ms
              frameActualAnimacion++;  // Avanza al siguiente frame
              tiempoInicioAnimacion = millis();  // Actualiza el tiempo de inicio
  
              if (frameActualAnimacion < 2) {  // Si todavía hay frames por mostrar
                  LCD_Sprite(posXf2, alturaActualf2, frogswidth, frogsheight, frogcol1, 2, frameActualAnimacion, 1, 0);
              } else {
                  // Si todos los frames se han mostrado, finaliza la animación y devuelve a Frog2 a su estado inicial
                  LCD_Sprite(posXf2, alturaActualf2, frogswidth, frogsheight, runf2, 4, 0, 0, 0);
                  animacionColisionActiva = false;  // Finaliza la animación de colisión
              }
          }
      }
  
  // Verificar y actualizar la animación de colisión para Frog1
  if (animacionColisionActivaFrog1) {
      if (millis() - tiempoInicioAnimacionFrog1 > 100) {  // Si ha pasado 100ms
          frameActualAnimacionFrog1++;  // Avanza al siguiente frame
          tiempoInicioAnimacionFrog1 = millis();  // Actualiza el tiempo de inicio
  
          if (frameActualAnimacionFrog1 < 2) {  // Si todavía hay frames por mostrar
              LCD_Sprite(posXf1, alturaActual, frogswidth, frogsheight, frogcol2, 2, frameActualAnimacionFrog1, 1, 0);
          } else {
              // Si todos los frames se han mostrado, finaliza la animación y devuelve a Frog1 a su estado inicial
              LCD_Sprite(posXf1, alturaActual, frogswidth, frogsheight, runf1, 4, 0, 0, 0);
              animacionColisionActivaFrog1 = false;  // Finaliza la animación de colisión
          }
      }
  }
  
  
  
  //tiles, estas tiles de las plataformas se deben dibujar para que reaparezcan cuan el personaje las borre
      LCD_Bitmap(55, 170, 216, 3, platlow);// plataforma 1
      LCD_Bitmap(87, 173, 10, 43, vigap1low);// madera 1
      LCD_Bitmap(225, 173, 10, 43, viga2p1low); //madera 2 plat2high
      LCD_Bitmap(88, 130, 144, 3, plat2high);// plataforma 2    
      LCD_Bitmap(106, 133, 3, 37, viga1p2high);// madera 1 viga2p2high[]
      LCD_Bitmap(213, 133, 3, 37, viga2p2high);// madera 1 
  
  
  
  //-----------------------------control frog 1------------------------------------
    if (Serial2.available()) { // Si hay datos disponibles en el puerto serial UART2
      char t = Serial2.read(); // Lee un caracter de UART2
      Serial.println(t); // Imprime el caracter recibido en el Serial Monitor
  
  
  // Movimiento hacia la derecha de Frog1
  if (t == 'B' && !enElAire) {
      movimiento = 1;
  
      posXf1 += 2;
      if (posXf1 > 320 - frogswidth)
          posXf1 = 320 - frogswidth;
  
      // Borra el sprite anterior
      V_line(posX2f1 - 1, alturaActual, 22, fillmovecolor);
      V_line(posX2f1, alturaActual, 22, fillmovecolor);
      V_line(posX2f1 + 2, alturaActual, 22, fillmovecolor);
  
      uint8_t animrun = (posXf1 / 2) % 4;
      LCD_Sprite(posXf1, alturaActual, frogswidth, frogsheight, runf1, 4, animrun, 0, 0);
      V_line(posXf1 - 1, alturaActual, 22, fillmovecolor);
  
      posX2f1 = posXf1;
  }
  
  // Movimiento hacia la izquierda de Frog1
  if (t == 'D' && !enElAire) {
      movimiento = -1;
  
      posXf1 -= 2;
      if (posXf1 < 0)
          posXf1 = 0;
  
      // Borra el sprite anterior
      V_line(posX2f1 + 24, alturaActual, 22, fillmovecolor);
      V_line(posX2f1 + 25, alturaActual, 22, fillmovecolor);
      V_line(posX2f1 + 26, alturaActual, 22, fillmovecolor);
  
      uint8_t animrun2 = (posXf1 / 2) % 4;
      LCD_Sprite(posXf1, alturaActual, frogswidth, frogsheight, runf1, 4, animrun2, 1, 0);
      V_line(posXf1 + 25, alturaActual, 22, fillmovecolor);
  
      posX2f1 = posXf1;
  }
  
  // Salto de Frog1
  if (t == 'J' && millis() - lastJumpTime > jumpDebounceTime) {
      saltar();
      lastJumpTime = millis();
  }
  
  // Verifica si se ha presionado el botón de bateo y si ha pasado el tiempo mínimo requerido desde el último bateo
  if (t == '3' && millis() - lastBatTime > batDebounceTime) {
      isBatting = true;  // Indica que Frog1 está bateando
      
      // Anima la acción de bateo mostrando 4 frames
      for (uint16_t i = 0; i < 4; i++) {
          LCD_Sprite(posXf1, alturaActual, frogswidth, frogsheight, batfrog1, 4, i, 1, 0);
          delay(100);  // Pequeña pausa para que la animación sea visible
      }
  
      // Devuelve a Frog1 a su posición inicial después de batear
      LCD_Sprite(posXf1, alturaActual, frogswidth, frogsheight, runf1, 4, 0, 0, 0);
      
      // Después de batear, verifica si hay colisión con frog2
      if (Colision()) {
          Serial.println(vidasFrog2);
  
          // Iniciar la animación de colisión para Frog2
          animacionColisionActiva = true;
          tiempoInicioAnimacion = millis();
          frameActualAnimacion = 0;
          LCD_Sprite(posXf2, alturaActualf2, frogswidth, frogsheight, frogcol1, 2, frameActualAnimacion, 1, 0);
  
          vidasFrog2--;  // Si hay colisión, reduce una vida de frog2
      }
  
      isBatting = false;  // Indica que Frog1 ha terminado de batear
      lastBatTime = millis();  // Actualiza el tiempo desde el último bateo
  }
  
    
    }//----------------------------------final control frog 1------------------------------------------
  //-----------------------------control frog 2------------------------------------
    if (Serial3.available()) { // Si hay datos disponibles en el puerto serial UART2
      char m = Serial3.read(); // Lee un caracter de UART2
      Serial.println(m); // Imprime el caracter recibido en el Serial Monitor
  
  
    //moviemieto a la derecha
  // Movimiento hacia la derecha de Frog2
  if (m == 'B' && !enElAiref2) {  // Solo permitir movimiento si no está en el aire
      movimientof2 = 1; 
      posXf2 += 2;
      if (posXf2 > 320 - frogswidth)
          posXf2 = 320 - frogswidth;
      
      // Borra el sprite anterior
      V_line( posX2f2 -1, alturaActualf2, 22, fillmovecolor);
      V_line( posX2f2, alturaActualf2, 22, fillmovecolor);
      V_line( posX2f2 +2, alturaActualf2, 22, fillmovecolor);
  
      uint8_t animrunf2 = (posXf2/2)%4;
      LCD_Sprite(posXf2, alturaActualf2, frogswidth, frogsheight, runf2, 4, animrunf2, 0, 0);
      V_line( posXf2 -1, alturaActualf2, 22, fillmovecolor);
      posX2f2 = posXf2; 
  }
  
  // Movimiento hacia la izquierda de Frog2
  if (m == 'D' && !enElAiref2) {  // Solo permitir movimiento si no está en el aire
      movimientof2 = -1;
      posXf2 -= 2;
      if (posXf2 < 0)
          posXf2 = 0;
  
      // Borra el sprite anterior
      V_line( posX2f2 +24, alturaActualf2, 22, fillmovecolor);
      V_line( posX2f2 +25, alturaActualf2, 22, fillmovecolor);
      V_line( posX2f2 +26, alturaActualf2, 22, fillmovecolor);
  
      uint16_t animrunf22 = (posXf2/2)%4;
      LCD_Sprite(posXf2, alturaActualf2, frogswidth, frogsheight, runf2, 4, animrunf22, 1, 0);
      V_line( posXf2 +25, alturaActualf2, 22, fillmovecolor);
      posX2f2 = posXf2;
  }
  
    // Salto
    if (m == 'J' && millis() - lastJumpTimef2 > jumpDebounceTimef2) { 
      //Serial.println(posXf2); // Imprime el caracter recibido en el Serial Monitor
      saltarf2();
      lastJumpTimef2 = millis(); // Actualizar el tiempo del último salto
    } //final salto
  
  // Verifica si se ha presionado el botón de bateo y si ha pasado el tiempo mínimo requerido desde el último bateo
  // Función de bateo para Frog2
  
      if (m == '3' && millis() - lastBatTimef2 > batDebounceTimef2) {
          isBattingf2 = true;  // Indica que Frog2 está bateando
          
          // Anima la acción de bateo mostrando 4 frames
          for (uint16_t i = 0; i < 4; i++) {
              LCD_Sprite(posXf2, alturaActualf2, frogswidth, frogsheight, batfrog2, 4, i, 1, 0);
              delay(100);  // Pequeña pausa para que la animación sea visible
          }
  
          // Devuelve a Frog2 a su posición inicial después de batear
          LCD_Sprite(posXf2, alturaActualf2, frogswidth, frogsheight, runf2, 4, 0, 0, 0);
          
          // Después de batear, verifica si hay colisión con Frog1
          if (Colision()) {
              Serial.println(vidasFrog1);
  
              // Iniciar la animación de colisión para Frog1
              animacionColisionActivaFrog1 = true;
              tiempoInicioAnimacionFrog1 = millis();
              frameActualAnimacionFrog1 = 0;
              LCD_Sprite(posXf1, alturaActual, frogswidth, frogsheight, frogcol2, 2, frameActualAnimacionFrog1, 1, 0);            vidasFrog1--;  // Si hay colisión, reduce una vida de Frog1
          }
  
          isBattingf2 = false;  // Indica que Frog2 ha terminado de batear
          lastBatTimef2 = millis();  // Actualiza el tiempo desde el último bateo
      }
  
  
    }//----------------------------------final control frog 2------------------------------------------
    // REVISAR SI GANA FROG 1, ES DECIR SI FROG2 SE QUEDA SIN VIDAS
    if (vidasFrog2 == 0){
      menuflag = 3; // Cambiar a menu de victoria de frog1
    }
  
    // Repetir lo anterior para saber si gana FROG 2
    if (vidasFrog1 == 0){
      menuflag = 4; // Cambiar a menu de victoria de frog2
    }
  }

  // Si gana frog1, pasar a pantalla de gana frog1 y activar cancion de ganador - blanco
  while (menuflag == 3){
    digitalWrite(MENU, LOW);
    digitalWrite(BATTLE, LOW);
    digitalWrite(WINNER, HIGH);
    digitalWrite(SONGSELECT, LOW);
    digitalWrite(SONGSELECT, HIGH);
    myFile = SD.open("bfwins.txt");
    mapeoSD();
  }

  // Si gana frog2, pasar a pantalla de gana frog1 y activar cancion de ganador - verde
  while (menuflag == 4){
    digitalWrite(MENU, LOW);
    digitalWrite(BATTLE, LOW);
    digitalWrite(WINNER, HIGH);
    digitalWrite(SONGSELECT, LOW);
    digitalWrite(SONGSELECT, HIGH);
    myFile = SD.open("gfwins.txt");
    
    mapeoSD();
  }
}



int asciitohex(int a){
  switch (a){
    case 48: //Si se lee un '0' 
      return 0x00; //Devolver un 0
    case 49: //Si se lee un '1'
      return 0x01; //Devolver un 1
    case 50: //Si se lee un '2'
      return 0x02; //Devolver un 2
    case 51: //Si se lee un '3'
      return 0x03; //Devolver un 3
    case 52: //Si se lee un '4'
      return 0x04; //Devolver un 4
    case 53: //Si se lee un '5'
      return 0x05; //Devolver un 5
    case 54: //Si se lee un '6'
      return 0x06; //Devolver un 6
    case 55: //Si se lee un '7'
      return 0x07; //Devolver un 7
    case 56: //Si se lee un '8' 
      return 0x08; //Devolver un 8
    case 57: //Si se lee un '9' 
      return 0x09; //Devolver un 9
    case 97: //Si se lee un 'a'
      return 0x0A; //Devolver un 10
    case 98: //Si se lee un 'b'
      return 0x0B; //Devolver un 11
    case 99: //Si se lee un 'c' 
      return 0x0C; //Devolver un 12
    case 100: //Si se lee un 'd'
      return 0x0D; //Devolver un 13
    case 101: //Si se lee un 'e'
      return 0x0E; //Devolver un 14
    case 102: //Si se lee un 'f'
      return 0x0F; //Devolver un 15
  }
}

//Función para leer de la SD y mostrar en la pantalla
void mapeoSD(){
  int hex1 = 0; //Variable para agarrar lo leído de la SD
  int val1 = 0; //Variable para guardar decena del valor hexadecimal
  int val2 = 0; //Variable para guardar la unidad hexadecimal
  int mapear = 0; //Iniciar con la posición del arreglo en 0
  int vertical = 0; //Iniciar con la coordenada en y en 0
  unsigned char maps[640]; //Definir un arreglo de 640 espacios

  if (myFile){ //Si se encontró un archivo con el nombre definido previamente ejecutar
    while (myFile.available()){ //Siempre que haya contenido por leer en la SD ejecutar
      mapear = 0; //Iniciar la posición de mapear en 0
      while (mapear < 640){ //Mientras se llegue 
        hex1 = myFile.read(); //Leer del archivo de texto
        if (hex1 == 120){ //Si se leyó una "x", significa que vienen dos valores en hexadecimal que si hay que guardar
          val1 = myFile.read(); //Leer y almacenar en una variable
          val2 = myFile.read(); //Leer y almacenar en una variable
          val1 = asciitohex(val1); //Convertir el valor de ASCII a hexadecimal
          val2 = asciitohex(val2); //Convertir el valor de ASCII a hexadecimal
          maps[mapear] = val1*16 + val2; //Guardar el valor de mapeo en el arreglo
          mapear++; //Aumentar la posición del arreglo
        }
      }

      LCD_Bitmap(0, vertical, 320, 1, maps); //Mostrar una fila de contenido de la imagen
      vertical++; //Aumentar la línea vertical
    }

    myFile.close(); //Cerrar el archivo si ya hay contenido por leer
  }
  else {
    myFile.close(); //Si no se encontró el arhivo cerrar para evitar algún error
    Serial.println("NO SE ABRIO");
  }
}
