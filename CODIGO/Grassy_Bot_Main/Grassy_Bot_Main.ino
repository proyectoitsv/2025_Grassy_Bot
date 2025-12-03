#include <Arduino.h>
#include <BluetoothSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <PCF8574.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_Sensor.h>
LiquidCrystal_I2C lcd(0x27,16,2); // si no te sale con esta direccion  puedes usar (0x3f,16,2) || (0x27,16,2)  ||(0x20,16,2) 
BluetoothSerial SerialBT;
PCF8574 pcf8574(0x24);
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
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
bool cuchilla = 0; //Activador Cuchilla
int ancho = 0;  //Largo Modo Automatico
int largo = 0;  //Largo Modo Automatico
float pulsos = 200; //Pulsos Enviados a los pasos
float Mpasos = 400; //Micropasos en los drivers
float Mpul = pulsos / 400;  //Multi de pulsos
float Mult = 400 / Mpasos;  //Multi de pasos
float Dpaso = 17 * Mpul * Mult;  //Distancia del paso (8,5 cm)
int plato = 17;
float GGiro = 45 * Mpul * Mult;  //Grados por giro (22.5 Grados)
int CantGiro = 90 / GGiro;  //Cantidad de giros para hacer 90°
int CantPaso = plato / Dpaso;  //Cantidad de pasos para el giro
int Cpasos = 0;  //Cantidad de pasos para cubrir el largo
int Cvueltas = 0;  //Cantidad de giros para cubrir el ancho
int DGiro = 1;  //Direccion de Giro 1 DERECHA 2 IZQUIERDA
int dirkeep = 0; //1AVA 2REV 3DER 4IZQ
float progreso = 0;
float progresoact = 0;
float roll = 0;
float pitch = 0;
bool pausa = 0;
bool automatico = 0;
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
void Mautomatico();
void avanzarKeep();
void retrocederKeep();
void derechaKeep();
void izquierdaKeep();
void IRAM_ATTR onTimer1();

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  pcf8574.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  SerialBT.begin("Grassy Bot 0.10");
  Serial.println("Bluetooth iniciado. Listo para emparejar!");
  lcd.print(  "GrassyBot 10");
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

  if(automatico == 1){
    Mautomatico();
  }
  avanzarKeep();
  retrocederKeep();
  derechaKeep();
  izquierdaKeep();

  if (SerialBT.available()) {  //ZONA COMANDOS BLUETOOTH
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
        if (receivedData.startsWith("ADM")) {
          modo = 3;
        }
      }
      else if(modo == 1 || modo == 3){    //MODO MANUAL
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
        if (receivedData.startsWith("DID")) {
          for(int i = 0;i < CantGiro/2;i++){
            derecha(1);
          };
        }
        if (receivedData.startsWith("DII")) {
          for(int i = 0;i < CantGiro/2;i++){
            izquierda(1);
          };
        }
        if (receivedData.startsWith("AVK")) {
          if(dirkeep == 1){
            dirkeep = 0;
          }
          else{
            dirkeep = 1;
          }
        }
        if (receivedData.startsWith("REK")) {
          if(dirkeep == 2){
            dirkeep = 0;
          }
          else{
            dirkeep = 2;
          }
        }
        if (receivedData.startsWith("GDK")) {
          if(dirkeep == 3){
            dirkeep = 0;
          }
          else{
            dirkeep = 3;
          }
        }
        if (receivedData.startsWith("GIK")) {
          if(dirkeep == 4){
            dirkeep = 0;
          }
          else{
            dirkeep = 4;
          }
        }
        if (receivedData.startsWith("MOT")) {
          cuchilla = !cuchilla;
          digitalWrite(Motor, cuchilla);
        }
        if (receivedData.startsWith("ESP")) {
          modo = 0;
        }
      }
      if(modo == 2 || modo == 3){  //MODO AUTOMATICO
        if (receivedData.startsWith("ANC")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          
          if(ancho == 0){
            ancho = x * 100;
            Mautomatico();
          }
          
        }
        if (receivedData.startsWith("LAR")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          
          if(largo == 0){
            largo = x * 100;
            Mautomatico();
          }   
        }
        if (receivedData.startsWith("EMP")) {
          automatico = 1;
        }
        if (receivedData.startsWith("PAU")) { 
          pausa = 1;  
        }
        if (receivedData.startsWith("CON")) {
          pausa = 0;
        }
        if (receivedData.startsWith("DET")) {
          pausa = 0;
          automatico = 0;
          Cpasos = 0;
          Cvueltas = 0;
          digitalWrite(Motor, LOW);
        }
      }
      receivedData = "";
      Serial.println();
    }
  }

  SerialBT.write(progresoact);

  /*if (Serial.available()) {//Mandar Serial por BT
    char c = Serial.read();
    SerialBT.write(c);
    if (c == '\n') {
      SerialBT.flush();
    }
  }*/

}


//COMANDOS

  void avanzar(int repeticiones) {
    frente();
    if (repeticiones == 0) {
      repeticiones = 1;
    }
    for (int x = 0; x < repeticiones; x++) {
      paso(pulsos);
    }
  }

  void retroceder(int repeticiones) {
    reversa();
    if (repeticiones == 0) {
      repeticiones = 1;
    }
    for (int x = 0; x < repeticiones; x++) {
      paso(pulsos);
    }
  }

  void izquierda(int repeticiones) {
    zurdo();
    if (repeticiones == 0) {
      repeticiones = 1;
    }
    for (int x = 0; x < repeticiones; x++) {
      giro(pulsos);
    }
  }

  void derecha(int repeticiones) {
    diestro();
    if (repeticiones == 0) {
      repeticiones = 1;
    }
    for (int x = 0; x < repeticiones; x++) {
      giro(pulsos);
    }
  }
  
  void avanzarKeep() {
    frente();
    if(dirkeep == 1){
    paso(1);
    }
  }

  void retrocederKeep() {
    reversa();
    if(dirkeep == 2){
    paso(1);
    }
  }

  void derechaKeep() {
    diestro();
    if(dirkeep == 3){
    giro(1);
    }
  }

  void izquierdaKeep() {
    zurdo();
    if(dirkeep == 4){
    giro(1);
    }
  }

  void Mautomatico(){

    if(automatico == 0){
      if(largo != 0 && ancho != 0){
      Cpasos = round(largo / Dpaso); 
      Cvueltas = round(ancho / plato);
      progreso = 100 / (Cpasos * (Cvueltas - 1));
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("P: ");
      lcd.print(Cpasos);
      lcd.setCursor(9,0);
      lcd.print("V: ");
      lcd.print(Cvueltas);
      lcd.setCursor(0,1);
      lcd.print("A:");
      lcd.print(ancho);
      lcd.print(" L:");
      lcd.print(largo);
      }
    }
    else if(automatico == 1){
      if(pausa == 0){
        if(Cvueltas > 1){
          digitalWrite(Motor, HIGH);
          for(int i = 0;i < Cpasos;i++){
            avanzar(1);
            progresoact == progresoact + progreso;
          }
          if(DGiro == 1){
            for(int i = 0;i < CantGiro;i++){
            derecha(1);
            }
            for(int i = 0;i < CantPaso;i++){
            avanzar(1);
            }
            for(int i = 0;i < CantGiro;i++){
            derecha(1);
            }
            DGiro = 2;
          }
          else if(DGiro == 2){
            for(int i = 0;i < CantGiro;i++){
            izquierda(1);
            }
            for(int i = 0;i < CantPaso;i++){
            avanzar(1);
            }
            for(int i = 0;i < CantGiro;i++){
            izquierda(1);
            }
            DGiro = 1;
          }
          Cvueltas--;
        }
        else{
          pausa = 0;
          automatico = 0;
          Cpasos = 0;
          Cvueltas = 0;
          digitalWrite(Motor, LOW);
        }
      }
      else{
      digitalWrite(Motor, LOW);
      }
    }
  }

//MOVERSE

  void paso(int pulsos) {
    for (int x = 0; x < pulsos; x++) {
      digitalWrite(StepD, HIGH);
      digitalWrite(StepI, HIGH);
      delayMicroseconds(500);
      digitalWrite(StepD, LOW);
      digitalWrite(StepI, LOW);
      delayMicroseconds(500);
    }
  }

  void giro(int pulsos) {
    for (int x = 0; x < pulsos; x++) {
      digitalWrite(StepD, HIGH);
      digitalWrite(StepI, HIGH);
      delayMicroseconds(500);
      digitalWrite(StepD, LOW);
      digitalWrite(StepI, LOW);
      delayMicroseconds(500);
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

  float BRUJULA() {
  sensors_event_t event;
  mag.getEvent(&event);

  float heading = atan2(event.magnetic.y, event.magnetic.x);

  if (heading < 0)
    heading += 2 * PI;

  return heading * 180 / PI;
}
