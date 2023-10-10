//Luis Pedro Gonzalez 21513
//Gabriel Carrera 21216
//Proyecto 2, Frog Smashers

//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};  

//--------------generales-----------------
#define frogswidth 25
#define frogsheight 22



//---------------------------------------posiciones --------------------------------------
int posXf1 = 25; //posicion de inicio en x de frog1
int posX2f1;
int posYf1 = 193; // posición vertical inicial para el salto 
//int posFrog1; //posicion en x y y para cheuqeo de  plataformas 


int altinicial = 193; //posicion vertical general de inicio del personaje

int movimiento =  0; // = quieto, 1 = derecha, -1 = izquierda
int alturaActual = 193;  // mapae la altura del persnaje


//plataforma 
  // altura de la plataforma
  int platformHeight = 169;
  //  límites x de la plataforma
  int platformStartX = 74;
  int platformEndX = 250;

//--------------------------------------colores-----------------------------------------

int fillmovecolor = 0xFDF1; //color con el que se rellena cuando el perosnje borra 
extern uint8_t platlow[]; //tile de la primera plataforma
extern uint8_t vigap1low[]; //tile de madera en la primera plataforma 
extern uint8_t viga2p1low[]; //tile madera 2 en primera plataforma
extern uint8_t plat2high[]; // tile de la segunda plataforma
extern uint8_t viga1p2high[]; //tile madera 1 en plat high 
extern uint8_t viga2p2high[];// madera 2 plat high 



//antirrebote para el salto
bool isJumping = false; //chequea si se esta saltando 
unsigned long lastJumpTime = 0; //banddera para saber cuando salto
const unsigned long jumpDebounceTime = 1500; // Tiempo de anti-rebote en milisegundos

//antirrebote para el bateo
bool isBatting = false; // Variable para revisar si se está bateando
unsigned long lastBatTime = 0; // Tiempo del último bateo
const unsigned long batDebounceTime = 500; // Tiempo de anti-rebote para el bateo

//**************************************************************

//************************************************************

//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);



//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(115200);//comunicacion con la pnatalla lcd

  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  LCD_Init();
  LCD_Clear(0x00);

//---------------------------configuracion para el uart-----------------------------------------
  Serial2.begin(9600); //iniica la velocidad del uart 2 

//-------------------------------------------------------------------------------------



//---------------------configuracion de pantalla--------------------------------
extern uint8_t back1[];
LCD_Bitmap(0, 0, 320, 240, back1);
 
}

//-----------------------------------FUNCION DEL SALTO FROG1---------------------------------------------------
void saltar() {
  alturaActual = posYf1;  // Actualiza la altura actual antes de iniciar el salto
  int alturaSalto = 84;  // Define la altura total del salto
  int avanceHorizontal = 1;  // Define cuántos píxeles se moverá horizontalmente en cada paso del salto
  bool landed = false;  // Variable para rastrear si el personaje ha aterrizado en la plataforma
  isJumping = true;  // Activa la bandera de salto

  // Fase de subida del salto
  for (int j = 0; j < alturaSalto / 2; j++) { 
    FillRect(posXf1, posYf1, frogswidth+1, frogsheight, fillmovecolor);  // Borra el sprite anterior dibujando un rectángulo del color de fondo
    posYf1--;  // Decrementa la posición vertical para mover el sprite hacia arriba
    if (movimiento == 1 && posXf1 < 320 - 26) {  // Si el movimiento es a la derecha y no se ha llegado al límite derecho
      posXf1 += avanceHorizontal;  // Incrementa la posición horizontal
    } else if (movimiento == -1 && posXf1 > 0) {  // Si el movimiento es a la izquierda y no se ha llegado al límite izquierdo
      posXf1 -= avanceHorizontal;  // Decrementa la posición horizontal
    }
    for(uint16_t i = 0; i < 3; i++) { 
       LCD_Sprite(posXf1, posYf1, frogswidth, frogsheight, jumpfrog2, 5, i, movimiento == -1, 0); // Dibuja el sprite de salto correspondiente
        delay(5);  // Introduce un pequeño retardo para visualizar el sprite
    }
  }

  // Fase de bajada del salto
  for (int j = 0; j < alturaSalto / 2; j++) {
   
    
    // Verifica la colisión con la plataforma en la fase descendente del salto
    if (posYf1 + frogsheight >= platformHeight && posYf1 + frogsheight <= platformHeight + 5) {
        Serial.println("Colisión detectada!");
        posYf1 = platformHeight - frogsheight;  // Ajusta la posición Y del personaje para que esté sobre la plataforma
        alturaActual = platformHeight - frogsheight;  // Actualiza la altura actual

        landed = true;  // Indica que el personaje ha aterrizado en la plataforma
        break;  // Termina el bucle, ya que el personaje ha aterrizado en la plataforma
    }
    
    FillRect(posXf1, posYf1, frogswidth, frogsheight, fillmovecolor);  // Borra el sprite anterior
    posYf1++;  // Incrementa la posición vertical para mover el sprite hacia abajo
    if (movimiento == 1 && posXf1 < 320-26) {  // Si el movimiento es a la derecha y no se ha llegado al límite derecho
      posXf1 += avanceHorizontal;  // Incrementa la posición horizontal
    } else if (movimiento == -1 && posXf1 > 0) {  // Si el movimiento es a la izquierda y no se ha llegado al límite izquierdo
      posXf1 -= avanceHorizontal;  // Decrementa la posición horizontal
    }
    for(uint16_t i = 2; i < 5; i++) { 
        LCD_Sprite(posXf1, posYf1, frogswidth, frogsheight, jumpfrog2, 5, i, movimiento == -1, 0);   // Dibuja el sprite de bajada correspondiente
        delay(5);  // Introduce un pequeño retardo para visualizar el sprite
    }
  }

  // Finalización del salto
  FillRect(posXf1, posYf1, frogswidth, frogsheight, fillmovecolor);  // Borra el sprite anterior una vez que el salto ha finalizado
  if (!landed) {  // Solo restablece la posición vertical si el personaje no ha aterrizado en la plataforma
      posYf1 = alturaActual;  // Restablece la posición vertical del sprite a su valor inicial
  }
  isJumping = false;  // Desactiva la bandera de salto

  // Dibuja el sprite original después de finalizar el salto, con o sin flip dependiendo de la dirección
  if (movimiento == 1) {  // Si el movimiento fue a la derecha
    LCD_Sprite(posXf1+1, posYf1, frogswidth, frogsheight, runf1, 4, 0, 0, 0);  // Dibuja el sprite en reposo sin flip
  } else if (movimiento == -1) {  // Si el movimiento fue a la izquierda
    LCD_Sprite(posXf1, posYf1, frogswidth, frogsheight, runf1, 4, 0, 1, 0);  // Dibuja el sprite en reposo con flip
  
  }//finn de flip al caaer
}//fin de funcion de salto 

//--------------------------------FIN FUNCION DE SALTO FROG 1-----------------------------------------------------

//-------------------------------------FUNCION DE PLATAFORMA-------------------------------------------

bool chequearPlataformaf1() {
    return (posXf1 + frogswidth >= platformStartX && posXf1 <= platformEndX && alturaActual == platformHeight - frogsheight);
}

void caerf1() {
  while (posYf1 < altinicial) {  // Continuar cayendo hasta llegar al suelo
    FillRect(posXf1, posYf1, frogswidth, frogsheight, fillmovecolor);  // Borra el sprite anterior
    posYf1++;  // Incrementa la posición vertical para mover el sprite hacia abajo
    for (uint16_t i = 2; i < 5; i++) {
      LCD_Sprite(posXf1, posYf1, frogswidth, frogsheight, jumpfrog2, 5, i, movimiento == -1, 0);  // Dibuja el sprite de caída correspondiente
      delay(5);  // Introduce un pequeño retardo para visualizar el sprite
    }
  }
  alturaActual = posYf1;  // Actualiza la altura actual una vez que el personaje ha llegado al suelo
}


//------------------------------------------FIN FUNCION DE PLATAFORMA---------------------------------
//***************************************************************************************************************************************
// Loop Infinito//
//***************************************************************************************************************************************
void loop() {
  if (!chequearPlataformaf1() && !isJumping) {
    caerf1();  // Si el personaje está fuera del rango de la plataforma y no está saltando, llamar a la función caer
  }
    // Serial.println(posYf1);

//tiles, estas tiles de las plataformas se deben dibujar para que reaparezcan cuan el personaje las borre
    LCD_Bitmap(55, 170, 216, 3, platlow);// plataforma 1
    LCD_Bitmap(87, 173, 10, 43, vigap1low);// madera 1
    LCD_Bitmap(225, 173, 10, 43, viga2p1low); //madera 2 plat2high
    LCD_Bitmap(88, 130, 144, 3, plat2high);// plataforma 1     
    LCD_Bitmap(106, 133, 3, 37, viga1p2high);// madera 1 viga2p2high[]
    LCD_Bitmap(213, 133, 3, 37, viga2p2high);// madera 1 



//-----------------------------control frog 1------------------------------------
  if (Serial2.available()) { // Si hay datos disponibles en el puerto serial UART2
    char t = Serial2.read(); // Lee un caracter de UART2
    Serial.println(t); // Imprime el caracter recibido en el Serial Monitor


  //moviemieto a la derecha
  if (t == 'B' ) { // Si se recibe 'B', mover a la derecha
    movimiento = 1; // Establece bandera a 1 cuando se mueve hacia la derecha

    posXf1 += 2; // se puede ir modificando para arreglar la velocidad, tambien se debe ir modificando el vline
    if (posXf1 != posX2f1) {
          //Serial2.println("Derecha");
         //Serial2.println(posXf1);
         if (posXf1 > 320-frogswidth)
            posXf1 = (320-frogswidth);
          delay(5);
          uint16_t animrun = (posXf1/2)%4;// numero de frames, 2 cada cuandtos frames (maneja velocidad)
        //uint16_t animframe = anim2%4;

        // Borra el sprite anterior
        // Dibuja líneas en las posiciones anteriores para "borrar" el sprite anterior
          V_line( posX2f1 -1, alturaActual, 22, fillmovecolor);
          V_line( posX2f1, alturaActual, 22, fillmovecolor);
          V_line( posX2f1 +2, alturaActual, 22, fillmovecolor);

    
    // Dibuja el nuevo sprite
      LCD_Sprite(posXf1 ,alturaActual ,frogswidth ,frogsheight ,runf1 ,4 ,animrun ,0 ,0);
      V_line( posXf1 -1, alturaActual, 22, fillmovecolor);
      posX2f1 = posXf1; // Actualiza posX2f1 con el valor actual de posXf1
    }
  }//final derecha

  //izquierda
  if (t == 'D' ){
    movimiento = -1; // Establece bandera a -1 cuando se mueve hacia la izquierda

    posXf1 -= 2;
    if (posXf1 != posX2f1) {

   
        //Serial.println("Izquierda");
        //Serial.println(posXf1);
        if (posXf1<(0))
        posXf1 = (0);
        delay(5);
        uint16_t animrun2 = (posXf1/2)%4;
        //uint16_t animframe = anim2%4;

        // Borra el sprite anterior
       // Dibuja líneas en las posiciones anteriores para "borrar" el sprite anterior
        V_line( posX2f1 +24, alturaActual, 22, fillmovecolor);
        V_line( posX2f1 +25, alturaActual, 22, fillmovecolor);
        V_line( posX2f1 +26, alturaActual, 22, fillmovecolor);

    
    // Dibuja el nuevo sprite
      LCD_Sprite(posXf1 ,alturaActual ,frogswidth ,frogsheight ,runf1 ,4 ,animrun2 ,1 ,0);
      V_line( posXf1 +25, alturaActual, 22, fillmovecolor);
      posX2f1 = posXf1; // Actualiza posX2f1 con el valor actual de posXf1
    }
  }//final izquierda 

  // Salto
  if (t == 'J' && millis() - lastJumpTime > jumpDebounceTime) { 
    //Serial.println(posXf1); // Imprime el caracter recibido en el Serial Monitor
    saltar();
    lastJumpTime = millis(); // Actualizar el tiempo del último salto
  } //final salto

  // Bateo de rana
  if (t == '3' && millis() - lastBatTime > batDebounceTime) {
    isBatting = true; // Indicador de que se está bateando
    for(uint16_t i = 0; i < 4; i++) {
        LCD_Sprite(posXf1, alturaActual, frogswidth, frogsheight, batfrog2, 4, i, 1, 0); // Ejecuta la animación de bateo
        delay(100); // Retardo para visualizar cada frame
    }
    LCD_Sprite(posXf1, alturaActual, frogswidth, frogsheight, runf1, 4, 0, 0, 0); // Restablece al estado original
    isBatting = false; // Indicador de que el bateo ha terminado
    lastBatTime = millis(); // Actualiza el tiempo del último bateo
  }

  
  }//----------------------------------final control frog 1------------------------------------------

}//final del loop

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
/*void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
*/

void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+w;
  y2 = y+h;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = w*h*2-1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
      
      //LCD_DATA(bitmap[k]);    
      k = k - 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    
    
    }
  digitalWrite(LCD_CS, HIGH);
}
