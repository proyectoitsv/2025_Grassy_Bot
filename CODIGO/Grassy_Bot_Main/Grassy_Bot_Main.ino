#include <Arduino.h>
#include <BluetoothSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <PCF8574.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
LiquidCrystal_I2C lcd(0x27,16,2); // si no te sale con esta direccion  puedes usar (0x3f,16,2) || (0x27,16,2)  ||(0x20,16,2) 
BluetoothSerial SerialBT;
PCF8574 pcf8574(0x24);
Adafruit_MPU6050 mpu;
String receivedData = "";
#define StepD 27
#define StepI 13
#define DirD 26
#define DirI 14
#define Motor 2
#define SDA_PIN 21
#define SCL_PIN 22
#define SC2 19
#define SC3 23
#define SCOut 18


int modo = 0; //0 Esperando /1 Manual /2 Automatico
bool cuchilla = 0;
int ancho = 0;
int largo = 0;
float roll = 0;
float pitch = 0;
int verde = 0;
hw_timer_t *timer1 = NULL;

bool Bandera1 = 0;


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
void expansor();
void inclinacion();
void color();
void IRAM_ATTR onTimer1();

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  pcf8574.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  SerialBT.begin("Grassy Bot 0.9");
  Serial.println("Bluetooth iniciado. Listo para emparejar!");
  lcd.print(  "GrassyBot 9");
  lcd.display();
  pinMode(StepD, OUTPUT);
  pinMode(StepI, OUTPUT);
  pinMode(DirD, OUTPUT);
  pinMode(DirI, OUTPUT);
  pinMode(Motor, OUTPUT);
  pinMode(SC2, OUTPUT);
  pinMode(SC3, OUTPUT);
  pinMode(SCOut, INPUT);

  digitalWrite(SC2, HIGH);
  digitalWrite(SC3, HIGH);

  if (!mpu.begin()) {
    lcd.setCursor(0,1);
    lcd.println(" MPU6050 Error  ");
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  timer1 = timerBegin(1000000); // frequency en Hz
  timerAttachInterrupt(timer1, &onTimer1);
  timerAlarm(timer1, 1000000ULL, true, 0);
  
}

void loop() {

//ZONA COMANDOS BLUETOOTH

 
  if (SerialBT.available()) {
    char c = SerialBT.read();
    receivedData += c;
    Serial.write(c);
    
    if (c == '\n') {
      receivedData.trim();
      
      if(modo == 0){
        if (receivedData.startsWith("MAN")) {
          modo = 1;
        }
        if (receivedData.startsWith("AUT")) {
          modo = 2;
        }
      }
      else if(modo == 1){
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
        if (receivedData.startsWith("MOT")) {
          cuchilla = !cuchilla;
          digitalWrite(Motor, cuchilla);
        }
        if (receivedData.startsWith("ESP")) {
          modo = 0;
        }
      }
      if(modo == 2){
        if (receivedData.startsWith("ANC")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          ancho = x;
        }
        if (receivedData.startsWith("LAR")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          largo = x;
        }
        if (receivedData.startsWith("ESP")) {
          modo = 0;
        }
      }
      receivedData = "";
      Serial.println();
    }
  }

  if (Serial.available()) {//Mandar Serial por BT
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

//TIMERS
  
  void IRAM_ATTR onTimer1() {
    
  }

//SENSORES

  void expansor(){
    
  }

  void inclinacion(){

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    roll = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
    pitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;
  }

  void color(){

    verde = pulseIn(SCOut, LOW);
  }
