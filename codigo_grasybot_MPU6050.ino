#include <Arduino.h>
#include <BluetoothSerial.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>

// Sensores
Adafruit_MPU6050 mpu;

BluetoothSerial SerialBT;
String receivedData = "";

#define StepD 27
#define StepI 13
#define DirD 26
#define DirI 14
#define Motor 2
#define LED_INTEGRADO 2

// Variables globales
float roll, pitch;

int modo = 0;  //0 Esperando /1 Manual /2 Automatico
bool cuchilla = 0;
int ancho = 0;
int largo = 0;
hw_timer_t *timer1 = NULL;

// Declaraciones de funciones
void avanzar(int repeticiones);
void retroceder(int repeticiones);
void derecha(int repeticiones);
void izquierda(int repeticiones);
void paso();
void giro();
void frente();
void reversa();
void zurdo();
void diestro();
void IRAM_ATTR onTimer1();
void leerSensores();

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Grassy Bot 0.6");
  Serial.println("Bluetooth iniciado. Listo para emparejar!");

  pinMode(StepD, OUTPUT);
  pinMode(StepI, OUTPUT);
  pinMode(DirD, OUTPUT);
  pinMode(DirI, OUTPUT);
  pinMode(Motor, OUTPUT);
  pinMode(LED_INTEGRADO, OUTPUT);

  Wire.begin();

  // INICIALIZAR MPU6050
  if (!mpu.begin()) {
    Serial.println("Error: No se pudo inicializar MPU6050!");
    while (1) {
      
    }
  }
  Serial.println("MPU6050 inicializado correctamente");

  // Configurar rango del MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  //TIMER
  timer1 = timerBegin(1000000);  // frequency en Hz
  timerAttachInterrupt(timer1, &onTimer1);
  timerAlarm(timer1, 1000000ULL, true, 0);

  Serial.println("Sistema listo!");
}

void loop() {
  leerSensores();

  //ZONA COMANDOS BLUETOOTH
  if (SerialBT.available()) {
    char c = SerialBT.read();
    receivedData += c;
    Serial.write(c);

    if (c == '\n') {
      receivedData.trim();

      if (modo == 0) {
        if (receivedData.startsWith("MAN")) {
          modo = 1;
          Serial.println("Modo MANUAL activado");
        }
        if (receivedData.startsWith("AUT")) {
          modo = 2;
          Serial.println("Modo AUTOMATICO activado");
        }
      } else if (modo == 1) {
        if (receivedData.startsWith("AVA")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          Serial.print("Avanzar: ");
          Serial.println(x);
          avanzar(x);
        }
        if (receivedData.startsWith("REV")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          Serial.print("Retroceder: ");
          Serial.println(x);
          retroceder(x);
        }
        if (receivedData.startsWith("DER")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          Serial.print("Derecha: ");
          Serial.println(x);
          derecha(x);
        }
        if (receivedData.startsWith("IZQ")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          Serial.print("Izquierda: ");
          Serial.println(x);
          izquierda(x);
        }
        if (receivedData.startsWith("MOT")) {
          cuchilla = !cuchilla;
          //digitalWrite(Motor, cuchilla);
          digitalWrite(LED_INTEGRADO, HIGH);
          Serial.print("Motor: ");
          Serial.println(cuchilla ? "ON" : "OFF");
        }
        if (receivedData.startsWith("ESP")) {
          modo = 0;
          Serial.println("Modo ESPERA activado");
        }
      }
      if (modo == 2) {
        if (receivedData.startsWith("ANC")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          ancho = x;
          Serial.print("Ancho: ");
          Serial.println(ancho);
        }
        if (receivedData.startsWith("LAR")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          largo = x;
          Serial.print("Largo: ");
          Serial.println(largo);
        }
        if (receivedData.startsWith("ESP")) {
          modo = 0;
          Serial.println("Modo ESPERA activado");
        }
      }
      receivedData = "";
      Serial.println();
    }
  }

  if (Serial.available()) {  //Mandar Serial por BT
    char c = Serial.read();
    SerialBT.write(c);
    if (c == '\n') {
      SerialBT.flush();
    }
  }
}

// FUNCIONES MPU6050
void leerSensores() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  roll = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  pitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;

  if (abs(roll) > 30 || abs(pitch) > 30) {
    // Se detectó inclinación mayor a 30 grados
    digitalWrite(Motor, HIGH);
    cuchilla = 1;
    Serial.println("ALERTA: Inclinación detectada! Motor encendido");
    Serial.print("Roll: ");
    Serial.print(roll);
    Serial.print(" Pitch: ");
    Serial.println(pitch);
  }
  digitalWrite(Motor, LOW);
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

//TIMER VOIDS
void IRAM_ATTR onTimer1() {
  // Timer para funciones futuras
}
