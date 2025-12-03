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
#define triggers 32
#define USD 33 //1 
#define USA 25 //3
#define USI 4 //4
#define USF 5 //2


int modo = 0; //0 Esperando /1 Manual /2 Automatico
bool cuchilla = 0; //Activador Cuchilla
int ancho = 0;  //Largo Modo Automatico
int largo = 0;  //Largo Modo Automatico
int pulsos = 200;  //Pulsos Enviados a los pasos
int Mpasos = 400;  //Micropasos en los drivers
float Mpul = float(pulsos) / 400.0;  //Multi de pulsos
float Mult = 400.0 / float(Mpasos);  //Multi de pasos
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
bool flagUS = 0;
int verde = 0;
bool contar = 0;
int ordenUS = 1;
float distanciaD = 0;
float distanciaF = 0;
float distanciaI = 0;
float distanciaA = 0;
uint32_t duracionD = 0;
uint32_t duracionF = 0;
uint32_t duracionI = 0;
uint32_t duracionA = 0;
volatile unsigned long startPulse = 0;
volatile unsigned long endPulse = 0;
volatile bool ready = false;
hw_timer_t *timer1 = NULL;
String cola = "";
char letra ;
bool moviendose = 0;
char comando;
int movpulsos = 0;
volatile bool iniciarUS = false;   
bool pulsoOn = false;            
uint64_t tiempoPulso = 0;   
uint64_t t_now = 0;


// Declaraciones de funciones
void avanzar(int repeticiones);
void retroceder(int repeticiones);
void derecha(int repeticiones);
void izquierda(int repeticiones);
void paso(int pulsos);
void giro(int pulsos);
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
void Ultrasonicos();
void IRAM_ATTR onTimer1();
void IRAM_ATTR echoISR();
void avanzarN();
void retrocederN();
void derechaN();
void izquierdaN();

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
  pinMode(triggers, OUTPUT);
  pinMode(USD, INPUT);
  pinMode(USF, INPUT);
  pinMode(USI, INPUT);
  pinMode(USA, INPUT); 

  digitalWrite(SC2, HIGH);
  digitalWrite(SC3, HIGH);
  digitalWrite(triggers, LOW);


  if (!mpu.begin()) {
    lcd.setCursor(0,1);
    lcd.println(" MPU6050 Error  ");
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  timer1 = timerBegin(1000000); // frequency en Hz
  
  timerAttachInterrupt(timer1, &onTimer1);
  timerAlarm(timer1, 100000ULL, true, 0);

  attachInterrupt(digitalPinToInterrupt(USD), echoISR, CHANGE);

}

void loop() {

  unsigned long t = micros();

    // generar trigger sin delay
    if (iniciarUS) {
        iniciarUS = false;
        digitalWrite(triggers, HIGH);
        pulsoOn = true;
        tiempoPulso = t;
    }

    if (pulsoOn && (t - tiempoPulso >= 10)) {
        digitalWrite(triggers, LOW);
        pulsoOn = false;
    }

    // procesar lectura
    if (ready) {
        ready = false;

        unsigned long duracion = endPulse - startPulse;

        float distanciaD = duracion * 0.0343f / 2.0f;

        // tu condición corregida
        if (distanciaD > 2 && distanciaD < 10) {
            //avanzar(1);
        } 
    }

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
          if(x == 0){
            x = 1;
          }
          for(int i = 0;i < x;i++){
            letra = 'A';
            cola += letra;
          };
          
        }
        if (receivedData.startsWith("REV")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          if(x == 0){
            x = 1;
          }
          for(int i = 0;i < x;i++){
            letra = 'R';
            cola += letra;
          };
        }
        if (receivedData.startsWith("DER")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          if(x == 0){
            x = 1;
          }
          for(int i = 0;i < x;i++){
            letra = 'D';
            cola += letra;
          };
        }
        if (receivedData.startsWith("IZQ")) {
          String numeroStr = receivedData.substring(3);
          int x = numeroStr.toInt();
          if(x == 0){
            x = 1;
          }
          for(int i = 0;i < x;i++){
            letra = 'I';
            cola += letra;
          };
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

  if (cola.length() > 0 && moviendose == 0) {
    comando = cola[0];
    movpulsos = pulsos;
    moviendose = 1;
    cola.remove(0, 1);   // Eliminar la primera letra (shift)
  }

  if(movpulsos > 0){
    if(comando == 'A'){
      avanzarN();
    }
    else if(comando == 'R'){
      retrocederN();
    }
    else if(comando == 'I'){
      izquierdaN();
    }
    else if(comando == 'D'){
      derechaN();
    }
    movpulsos--;
  }
  if(movpulsos == 0){
    moviendose = 0;
  }

  //SerialBT.print(progresoact);

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

  void avanzarN() { frente(); paso(1); }
  void retrocederN() { reversa(); paso(1); }
  void izquierdaN() { zurdo(); giro(1); }
  void derechaN() { diestro(); giro(1); }
  void avanzarKeep() { frente(); if (dirkeep == 1) paso(1); }
  void retrocederKeep() { reversa(); if (dirkeep == 2) paso(1); }
  void derechaKeep() { diestro(); if (dirkeep == 3) giro(1); }
  void izquierdaKeep() { zurdo(); if (dirkeep == 4) giro(1); }

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
            progresoact = progresoact + progreso;
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
      delayMicroseconds(1000);
      digitalWrite(StepD, LOW);
      digitalWrite(StepI, LOW);
      delayMicroseconds(1000);
    }
  }

  void giro(int pulsos) {
    for (int x = 0; x < pulsos; x++) {
      digitalWrite(StepD, HIGH);
      digitalWrite(StepI, HIGH);
      delayMicroseconds(1000);
      digitalWrite(StepD, LOW);
      digitalWrite(StepI, LOW);
      delayMicroseconds(1000);
    }
  }

//DIRECCIONES

  void frente() { digitalWrite(DirD, HIGH);digitalWrite(DirI, LOW);}
  void reversa() {digitalWrite(DirD, LOW);digitalWrite(DirI, HIGH); }
  void zurdo() {digitalWrite(DirD, HIGH);digitalWrite(DirI, HIGH);}
  void diestro() {digitalWrite(DirD, LOW);digitalWrite(DirI, LOW);}

//INTERRUPCIONES
  
  void IRAM_ATTR onTimer1() {
    iniciarUS = 1;
  }

  void IRAM_ATTR echoISR() {
    if (digitalRead(USD)) {
        startPulse = micros();     // flanco ascendente
    } else {
        endPulse = micros();       // flanco descendente
        ready = true;              // hay nueva lectura
    }
}

//SENSORES

  void expansor(){
    
  }
  
  void Ultrasonicos(){

  }

  void inclinacion(){

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    roll = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
    pitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;
  }

  void color(){

    verde = pulseIn(SCOut, LOW, 20000); // 20 ms
  }
