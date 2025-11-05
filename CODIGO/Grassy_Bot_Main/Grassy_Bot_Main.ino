#include <Arduino.h>
#include <BluetoothSerial.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // si no te sale con esta direccion  puedes usar (0x3f,16,2) || (0x27,16,2)  ||(0x20,16,2) 
BluetoothSerial SerialBT;
String receivedData = "";
#define StepD 27
#define StepI 13
#define DirD 26
#define DirI 14
#define Motor 2


int modo = 0; //0 Esperando /1 Manual /2 Automatico
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

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  SerialBT.begin("Grassy Bot 0.7");
  Serial.println("Bluetooth iniciado. Listo para emparejar!");
  lcd.print(  "GrassyBot 7")
  lcd.setCursor(0,1);
  lcd.print(" Test LCD e I2C")
  pinMode(StepD, OUTPUT);
  pinMode(StepI, OUTPUT);
  pinMode(DirD, OUTPUT);
  pinMode(DirI, OUTPUT);
  pinMode(Motor, OUTPUT);

  //TIMER
  timer1 = timerBegin(1000000); // frequency en Hz
  timerAttachInterrupt(timer1, &onTimer1);
  timerAlarm(timer1, 1000000ULL, true, 0);
  
}

void loop() {

//ZONA COMANDOS BLUETOOTH

  if (SerialBT.available()) {
    char c = SerialBT.read();
    receivedData += c;
    lcd.setCursor(0,1);
    lcd.print("Comando: ")
    lcd.print(c);

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

//TIMER VOIDS
  
  void IRAM_ATTR onTimer1() {
    
  }

//SENSORES
