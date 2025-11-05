#include <BluetoothSerial.h>
BluetoothSerial SerialBT;
String receivedData = "";

// Pines de motores
const int StepD = 27; //(z)
const int StepI = 13; //(y)
const int DirD = 26; //(z)
const int DirI = 14; //(y)

// Pines de sensores ultrasónicos
const int TRIG_PIN = 32;  // Pin común para TRIG de los 4 sensores
const int ECHO_PIN_1 = 33;
const int ECHO_PIN_2 = 25;
const int ECHO_PIN_3 = 34;
const int ECHO_PIN_4 = 35;

// Variables para las distancias medidas
volatile float distancia1 = 0;
volatile float distancia2 = 0;
volatile float distancia3 = 0;
volatile float distancia4 = 0;

// Timer
hw_timer_t *timer = NULL;
volatile bool leerSensores = false;
volatile int sensorActual = 0;

// Declaraciones de funciones
void avanzar(int repeticiones);
void retroceder(int repeticiones);
void derecha(int repeticiones);
void izquierda(int repeticiones);
void paso();
void giro();
void frente();
void reversa();
void leerUltrasonico(int echoPin, volatile float &distancia);

// Función de interrupción del timer
void IRAM_ATTR onTimer() {
  leerSensores = true;
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Grassy Bot 0.5");
  Serial.println("Bluetooth iniciado. Listo para emparejar!");
  
  // Configuración de pines de motores
  pinMode(StepD, OUTPUT);
  pinMode(StepI, OUTPUT);
  pinMode(DirD, OUTPUT);
  pinMode(DirI, OUTPUT);
  
  // Configuración de pines de sensores ultrasónicos
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(ECHO_PIN_3, INPUT);
  pinMode(ECHO_PIN_4, INPUT);
  
  // Configuración del timer (interrupción cada 1 segundo)
  // Prescaler de 80 (80MHz/80 = 1MHz)
  // Contador hasta 1000000 = 1 segundo
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);  // 1 segundo
  timerAlarmEnable(timer);
  
  Serial.println("Sistema inicializado con sensores ultrasónicos");
}

void loop() {

// Manejo de lectura de sensores ultrasónicos
  if (leerSensores) {
    leerSensores = false;
    
    // Enviar pulso TRIG
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    // Leer cada sensor de manera secuencial
    leerUltrasonico(ECHO_PIN_1, distancia1);
    delay(10);  // Pequeña pausa entre lecturas
    
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    leerUltrasonico(ECHO_PIN_2, distancia2);
    delay(10);
    
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    leerUltrasonico(ECHO_PIN_3, distancia3);
    delay(10);
    
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    leerUltrasonico(ECHO_PIN_4, distancia4);
    
    // Mostrar distancias
    Serial.print("Sensor 1: ");
    Serial.print(distancia1);
    Serial.print(" cm | Sensor 2: ");
    Serial.print(distancia2);
    Serial.print(" cm | Sensor 3: ");
    Serial.print(distancia3);
    Serial.print(" cm | Sensor 4: ");
    Serial.print(distancia4);
    Serial.println(" cm");
  }

//ZONA COMANDOS BLUETOOTH

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

//COMANDOS

void avanzar(int repeticiones) {
  frente();
  if (repeticiones == 0) {
    repeticiones = 1;
  }
  for (int x = 0; x < repeticiones; x++) {
    paso();
  }
}

void retroceder(int repeticiones) {
  reversa();
  if (repeticiones == 0) {
    repeticiones = 1;
  }
  for (int x = 0; x < repeticiones; x++) {
    paso();
  }
}

void izquierda(int repeticiones) {
  zurdo();
  if (repeticiones == 0) {
    repeticiones = 1;
  }
  for (int x = 0; x < repeticiones; x++) {
    giro();
  }
}

void derecha(int repeticiones) {
  diestro();
  if (repeticiones == 0) {
    repeticiones = 1;
  }
  for (int x = 0; x < repeticiones; x++) {
    giro();
  }
}

//MOVERSE

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

//DIRECCIONES

void frente() {
  digitalWrite(DirD, HIGH);
  digitalWrite(DirI, LOW);
}
void reversa() {
  digitalWrite(DirD, LOW);
  digitalWrite(DirI, HIGH);
}
void zurdo() {
  digitalWrite(DirD, HIGH);
  digitalWrite(DirI, HIGH);
}
void diestro() {
  digitalWrite(DirD, LOW);
  digitalWrite(DirI, LOW);
}
