//instalar liberia dabble en arduino ide 
//instalar simulador de control pra corrar app dabble creada de control dabble 

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <DabbleESP32.h>

void setup() {
  Serial.begin(9600);      
  Dabble.begin("MyEsp32");   //poner el nombre que aparece para concetar a bluetooth
}

void loop() {
  Dabble.processInput();  

  if (GamePad.isUpPressed()) {
    Serial.write('J'); // Envía 'J' cuando se presiona el botón Up.
  }
  else if (GamePad.isDownPressed()) {
    Serial.write('K'); // Envía 'K' cuando se presiona el botón Down.
  }
  else if (GamePad.isLeftPressed()) {
    Serial.write('D'); // Envía 'D' cuando se presiona el botón Left.
  }
  else if (GamePad.isRightPressed()) {
    Serial.write('B'); // Envía 'B' cuando se presiona el botón Right.
  }

else if (GamePad.isTrianglePressed())
  {
    Serial.write('3'); // Envía 'B' cuando se presiona el botón Right.
  }
  // aqui se puede inr agragando ma sbotones

  delay(100); // Agrega un pequeño delay para evitar la saturación del puerto Serial.
}
