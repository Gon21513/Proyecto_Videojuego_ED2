#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial BT; // Objeto Bluetooth

void setup() {
  Serial.begin(9600); // Inicializa la conexión Serial (UART0) para depuración y comunicación UART
  BT.begin("frog2"); // Inicia el Bluetooth con un nombre dado
  Serial.println("El dispositivo Bluetooth está listo para emparejarse");
}

void loop() {
  if (BT.available()) { // Verifica si hay datos disponibles en Bluetooth
    int incoming = BT.read(); // Lee los datos recibidos por Bluetooth
    Serial.write(incoming); // Envia los datos recibidos por UART usando UART0
  }
}
