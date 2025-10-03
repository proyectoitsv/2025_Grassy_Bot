#include <BluetoothSerial.h>
#include <BluetoothSerial.cpp>
BluetoothSerial SerialBT;
String receivedData = "";

/* para los motores finales

const int StepD = 2;
const int StepI = 3;
const int DirD = 5;
const int DirI = 6;

*/

const int StepD = 27; //(z)
const int StepI = 13; //(y)
const int DirD = 26; //(z)
const int DirI = 14; //(y)


// Declaraciones de funciones
void avanzar(int repeticiones);
void retroceder(int repeticiones);
void derecha(int repeticiones);
void izquierda(int repeticiones);
void paso();
void giro();
void frente();
void reversa();

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Grassy Bot 0.3");
  Serial.println("Bluetooth iniciado. Listo para emparejar!");
  pinMode(StepD, OUTPUT);
  pinMode(StepI, OUTPUT);
  pinMode(DirD, OUTPUT);
  pinMode(DirI, OUTPUT);
}

void loop() {

  if (SerialBT.available()) {
    char c = SerialBT.read();
    receivedData += c;
    Serial.write(c);

    if (c == '\n') {

      receivedData.trim();
      if (receivedData.startsWith("AVA")) {
        String numeroStr = receivedData.substring(3);
        int x = numeroStr.toInt();
        avanzar(x);
      }
      if (receivedData.startsWith("REV")) {
        String numeroStr = receivedData.substring(3);
        int x = numeroStr.toInt();
        retroceder(x);
      }
      if (receivedData.startsWith("DER")) {
        String numeroStr = receivedData.substring(3);
        int x = numeroStr.toInt();
        derecha(x);
      }
      if (receivedData.startsWith("IZQ")) {
        String numeroStr = receivedData.substring(3);
        int x = numeroStr.toInt();
        izquierda(x);
      }
      receivedData = "";
      Serial.println();
    }
  }


  if (Serial.available()) {

    char c = Serial.read();
    SerialBT.write(c);
    if (c == '\n') {
      SerialBT.flush();
    }
  }
}

void avanzar(int repeticiones) {
  Serial.print("Iniciando avanzar con ");
  Serial.print(repeticiones);
  Serial.println(" repeticiones:");
  frente();
  if (repeticiones == 0) {
    repeticiones = 1;
  }
  for (int i = 0; i < repeticiones; i++) {
    Serial.print("paso ");
    Serial.println(i + 1);
    paso();
  }
}

void retroceder(int repeticiones) {
  Serial.print("Iniciando reversa con ");
  Serial.print(repeticiones);
  Serial.println(" repeticiones:");
  reversa();
  if (repeticiones == 0) {
    repeticiones = 1;
  }
  for (int i = 0; i < repeticiones; i++) {
    Serial.print("paso ");
    Serial.println(i + 1);
    paso();
  }
}

void paso() {
  for (int x = 0; x < 200; x++) {  
    digitalWrite(StepD, HIGH);
    digitalWrite(StepI, HIGH);
    delayMicroseconds(1000);
    digitalWrite(StepD, LOW);
    digitalWrite(StepI, LOW);
    delayMicroseconds(1000);
  }
}

void giro() {
  for (int x = 0; x < 200; x++) {  
    digitalWrite(StepD, HIGH);
    digitalWrite(StepI, HIGH);
    delayMicroseconds(1000);
    digitalWrite(StepD, LOW);
    digitalWrite(StepI, LOW);
    delayMicroseconds(1000);
  }
}


void frente() {
  digitalWrite(DirD, HIGH);
  digitalWrite(DirI, HIGH);
}
void reversa() {
  digitalWrite(DirD, LOW);
  digitalWrite(DirI, LOW);
}
void izquierda(int repeticiones) {
  digitalWrite(DirD, HIGH);
  digitalWrite(DirI, LOW);
  for (int x = 0; x < repeticiones; x++) {  
    giro();
  }
}
void derecha(int repeticiones) {
  digitalWrite(DirD, LOW);
  digitalWrite(DirI, HIGH);
  for (int x = 0; x < repeticiones; x++) {  
    giro();
  }
}
