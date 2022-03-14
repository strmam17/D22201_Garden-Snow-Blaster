#include <Wire.h>
#include <SHT21.h>  // include SHT21 library
#include <LiquidCrystal_I2C.h> // Vorher hinzugefügte LiquidCrystal_I2C Bibliothek einbinden
#include <Servo.h>  //Bibliothek zum ansteuern von Servo

#include <SoftwareSerial.h>

SoftwareSerial Blue(2, 3);
char data = ' ';
long int v_button_motorup = 2;// Motor hoch
long int v_button_motordown = 3;//  Motor runter


LiquidCrystal_I2C lcd(0x27, 16, 2); //Hier wird festgelegt um was für einen Display es sich handelt. In diesem Fall eines mit 16 Zeichen in 2 Zeilen und der HEX-Adresse 0x27. Für ein vierzeiliges I2C-LCD verwendet man den Code "LiquidCrystal_I2C lcd(0x27, 20, 4)" 
SHT21 sht; //Luftfeuchtigkeitssensor Temperatursensor
Servo servo;
Servo servo2;
Servo servo3;

int position_durchfluss = 0; //Variable zum speichern von der aktuellen Position des Servos "Normalen Wasser" düsen
int current_Motordrehzahl = 0; //Variable zum speichern von der aktuellen Motordrehzal;

int schritte_PWM_Motordrehzahl = 15; //PWM schritte
int schritte_nukleator_air  = 20;
int schritteDurchflussServo = 20; //Die Schritte um wieviel der Servomotor sich bewegen soll z.B. 20 => 20°
int schritte_nukleator_water = 20;

int anzeigeAmLCD = 1; //Für die Abfrage des Menüs, abgleich mit dem Enum beachten
int position_durchfluss_nukleator_air = 0; //Variable zum speichern von der aktuellen Position des Servos Air Servo
int position_durchfluss_nukleator_water = 0; //Variable zum speichern von der aktuellen Position des Servos von den Nukleatorwasserdüsen

String ventistate = "Aus";
String lightstate = "Aus";

//Button Def.
int pin_button_count_up = 12; //D12 D2!
int pin_button_count_down = 9; //D13 D3!
int pin_button_light = 4;//D4
int pin_button_menu = 5; //D5

int pin_relai_light = 8; //D8
int pin_control_ventilator_pwm = 11; //D11
int pin_control_servo = 6; //D6 wasser nebeldüse servo
int pin_control_servo_air = 7; //D7 Luft servo
int pin_control_servo_nukleator_water = 10; //D10 Nukleator Düsen Wasser Servo

//luft servo D7
//alg servo wasser D10
//d2 grün
//flag

bool flag_button_light = false; //Flag für den Button Relai Licht ein/aus
bool flag_light_state = false; //Flag für den Status des Relais

bool flag_button_count_up = false; //Flag für den Button Hochzählen
bool flag_button_count_up_state = false; //Nicht definiert

bool flag_button_count_down = false; //Flag für den Button Herunterzählen
bool flag_button_count_down_state = false; //Nicht definiert

bool flag_button_menu = false; //Flag für Menu

bool isAnUpdate = false;



float cfactor = 4.77;

byte interrupt = 0;
byte sensorPin = 2;
byte pulseCount;
float flowRate;
unsigned int f_ml;
unsigned long t_ml;
unsigned long oldTime;


//Flags für die Anzeigen am LCD
bool flag_main_menu = true; //Für einmalige Anzeige im Menu bzw. für lcd update nur einmal bei ersten wechsel
bool flag_durchfluss_menu = true; //Für einmalige Anzeige der Durchflussmenge
bool flag_motordrehzahl_menu = true; //Für einmalige Anzeige der Motordrehzahl
bool flag_durchfluss_menu_nukleator_air = true; //Für einmalige Anzeige der Durchflussmenge von der Luft
bool flag_durchfluss_menu_nukleator_water = true; //Für einmalige Anzeige der Nukleator Düsen






  

//Def. Temperatur und Luftfeuchtigkeitssensor
float temp;   // Variable zum Spechern der Temperatur
float humidity; // variable to store hemidity

void setup() {

  Wire.begin();   // begin Wire(I2C)
  Serial.begin(9600);
  Blue.begin(9600);
  servo.attach(6); //setzen des Servo Objektes auf den digitalen PIN 6
  servo2.attach(7); //nukleatordüse Luft
  servo3.attach(10); //Nukleator wasser
  servo.write(0);
  servo2.write(0);
  servo3.write(0);
  Serial.print(servo.read());
  delay(1000); //Kann noch geändert werden für die Setup funkton / Servo stellung null
  
  lcd.init(); //Im Setup wird der LCD gestartet 
  lcd.backlight(); //Hintergrundbeleuchtung einschalten (lcd.noBacklight(); schaltet die Beleuchtung aus).

  //Als Digitale eingänge definieren
  pinMode(pin_button_count_up, INPUT);
  pinMode(pin_button_count_down, INPUT);
  pinMode(pin_button_light, INPUT);
  pinMode(pin_button_menu, INPUT);

  //Als Digitale ausgänge definieren
  pinMode(pin_control_ventilator_pwm, OUTPUT);
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pin_relai_light, OUTPUT);
  pinMode(pin_control_servo_air, OUTPUT);
  pinMode(pin_control_servo_nukleator_water, OUTPUT);
  
  
  //Schalten das Licht 
  digitalWrite(pin_relai_light , HIGH); 
   digitalWrite(pin_control_ventilator_pwm, HIGH);
  //analogWrite(pin_control_ventilator_pwm, current_Motordrehzahl);


  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  attachInterrupt(interrupt, pulseCounter, FALLING);


}

void loop() {


//while(Blue.available()==0) ;     
  if(Blue.available()){
    data = Blue.read();

    //Blue.write(data);
    //  
  }
 /*if(isAnUpdate == true){
    dataToSend();
    isAnUpdate=false;
 } */
if(data == '1'){
   flag_button_light = true;
   data = ' ';
   isAnUpdate=true;
}else if(data == '2'){
   druchflussUp();
   data = ' ';
   isAnUpdate=true;
   flag_durchfluss_menu = true;
}else if(data == '3'){
   druchflussDown();
   data = ' ';
   isAnUpdate=true;
   flag_durchfluss_menu = true;
}else if(data == '4'){
   druchflussNukleatorWaterUp();
   data = ' ';
   isAnUpdate=true;
   flag_durchfluss_menu_nukleator_water = true;
}else if(data == '5'){
   druchflussNukleatorWaterDown();
   data = ' ';
   isAnUpdate=true;
   flag_durchfluss_menu_nukleator_water = true;
}else if(data == '6'){
   druchflussNukleatorAirUp();
   data = ' ';
   isAnUpdate=true;
   flag_durchfluss_menu_nukleator_air = true;
}else if(data == '7'){
   druchflussNukleatorAirDown();
   data = ' ';
   isAnUpdate=true;
   flag_durchfluss_menu_nukleator_air = true;
}else if(data == '8'){
   digitalWrite(pin_control_ventilator_pwm, LOW);
   ventistate = "Ein";
   isAnUpdate=true;
   data = ' ';
}else if(data == '9'){
  digitalWrite(pin_control_ventilator_pwm, HIGH);
  ventistate = "Aus";
  isAnUpdate=true;
   data = ' ';
}else{
  Serial.println("Es wurde eine Falscher Befehl gesendet");
}


 

 
//begin Flag

  //flag button light
  if(digitalRead(pin_button_light) == HIGH){
    flag_button_light = true;
  isAnUpdate=true;
  
  }
  //flag button count up
  if(digitalRead(pin_button_count_up) == HIGH){
    flag_button_count_up = true;
  isAnUpdate=true;
  }
  //flag button count down
  if(digitalRead(pin_button_count_down) == HIGH){
    flag_button_count_down = true;
  isAnUpdate=true;
  }
  //flag button menu
  if(digitalRead(pin_button_menu) == HIGH){
    flag_button_menu =true;
    isAnUpdate=true;
  }

  
  
  //flag button anzeige LCD
  if(flag_button_menu == true) {
    anzeigeAmLCD++;
    
    flag_button_menu = false;
  delay(800);//Wenn programm fertig ist delay abändern
  }
  
  menuAbfrage();
  relaiLicht();
//end Flag
//begin Abfrage
//tempLCD();

}

void dataToSend(){
  /*
   position_durchfluss  
   position_durchfluss_nukleator_air
   position_durchfluss_nukleator_water

   */

   String druchfluss = String(position_durchfluss);
   String nukleator_air = String(position_durchfluss_nukleator_air);
   String nukleator_water = String(position_durchfluss_nukleator_water);


//String tosend = String("a" + druchfluss + "b" + nukleator_air + "c"  +nukleator_water + "d" + ventistate + "z");

//String tosend = String("Water" + druchfluss + ";Air_N" + nukleator_air + ";Water_N "  +nukleator_water + ";Venti " + ventistate + ";Licht " + lightstate);
String tosend = String(druchfluss + ";" + nukleator_air + ";"  +nukleator_water + ";" + ventistate + ";" + lightstate);
//char Buf[50];

//tosend.toCharArray(Buf, 50);

Blue.print(tosend);

}



  int gradInProzent(int now, int max){
    
    return(now*100)/max;  
    
  }
  

//Funktionen 

//Funktion für anzeige der Tempertur am LCD;
void tempLCD(){

  temp = sht.getTemperature();  // get temp from SHT 
  humidity = sht.getHumidity(); // get temp from SHT

  Serial.print("Temp: ");     // print readings
  Serial.print(temp);
  Serial.print("\t Humidity: ");
  Serial.println(humidity);


          
    lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Temp:"); // 5
        lcd.setCursor(7, 0); 
        lcd.print(temp);
        lcd.setCursor(13,0);
        lcd.print("C");

        lcd.setCursor(0,1);
        lcd.print("Humi:"); // 5
        lcd.setCursor(7, 1); 
        lcd.print(humidity);
        lcd.setCursor(13,1);
        lcd.print("%");

  

  delay(85);  // min delay for 14bit temp reading is 85ms
}

void literProMinuteLCD(){

   if ((millis() - oldTime) > 1000)
  {
    int semic;
    detachInterrupt(interrupt);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / cfactor;
    oldTime = millis();
    f_ml = (flowRate / 60) * 1000;
    t_ml += f_ml;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Druchfluss");
    lcd.setCursor(0,1);
    
    Serial.print(int(flowRate));
    Serial.print(".");
    semic = (flowRate - int(flowRate)) * 10;
    Serial.print(semic, DEC) ;
    lcd.print(f_ml);
    lcd.setCursor(11,1);
    lcd.print("L/min");
    Serial.print("L/min");
    Serial.print("  Total: ");
    Serial.print(t_ml);
    Serial.println("mL");
    pulseCount = 0;
    attachInterrupt(interrupt, pulseCounter, FALLING);


  }

  
}


void pulseCounter()
{
  pulseCount++;
}



//Funktion für Servostellung um bestimmte Grade zu erhöhen
void druchflussUp(){
  
  if(position_durchfluss != 180){
    position_durchfluss = position_durchfluss + schritteDurchflussServo;
   
    servo.write(position_durchfluss);
    Serial.println(position_durchfluss);
    //delay(15);
  }

  }


//Funktion für Servostellung um bestimmte Grade zu verringern
void druchflussDown(){
  
  if(position_durchfluss != 0){
    position_durchfluss = position_durchfluss - schritteDurchflussServo;
    servo.write(position_durchfluss);
    Serial.println(position_durchfluss);
    //delay(15);
  }
}


//---------------------------------------------------------


void druchflussNukleatorWaterUp(){
  
  if(position_durchfluss_nukleator_water != 180){
  
    for(int i = position_durchfluss_nukleator_water; i <= position_durchfluss_nukleator_water + schritte_nukleator_water; i++){
    servo3.write(i);
    Serial.println(i);
    //delay(15);
  }
 if(position_durchfluss_nukleator_water != 180){
 position_durchfluss_nukleator_water = position_durchfluss_nukleator_water + schritte_nukleator_water;
 }

  }
}


void druchflussNukleatorWaterDown(){
  
  if(position_durchfluss_nukleator_water != 0){
  for(int i = position_durchfluss_nukleator_water; i >= position_durchfluss_nukleator_water - schritte_nukleator_water; i--){
    servo3.write(i);
    Serial.println(i);
  //  delay(15);
  }
  
  if(position_durchfluss_nukleator_water != 0){
    position_durchfluss_nukleator_water = position_durchfluss_nukleator_water - schritte_nukleator_water;
  }
  }
}



void druchflussNukleatorAirUp(){
  
  if(position_durchfluss_nukleator_air != 180){
  
    for(int i = position_durchfluss_nukleator_air; i <= position_durchfluss_nukleator_air + schritte_nukleator_air; i++){
    servo2.write(i);
    Serial.println(i);
    //delay(15);
  }
 if(position_durchfluss_nukleator_air != 180){
 position_durchfluss_nukleator_air = position_durchfluss_nukleator_air + schritte_nukleator_air;
 }

  }
}


void druchflussNukleatorAirDown(){
  
  if(position_durchfluss_nukleator_air != 0){
  for(int i = position_durchfluss_nukleator_air; i >= position_durchfluss_nukleator_air - schritte_nukleator_air; i--){
    servo2.write(i);
    Serial.println(i);
    //delay(15);
  }
  
  if(position_durchfluss_nukleator_air != 0){
    position_durchfluss_nukleator_air = position_durchfluss_nukleator_air - schritte_nukleator_air;
  }
  }
}

//-------------------------------------------------------
//enum definiert für anzeige am LCD-Screen
enum{
  
  mainScreen = 1,
  motordrehzahlScreen, 
  umgebungsdatenScreen, //flag flag_main_menu auf true setzen für mainScreen title 
  durchflussScreen,
  nukleatorduesenWasserScreen,
  nukleatorAirScreen,
  literprominute
};

//Abfrage: Knopf für relai ist gedrückt => Relai schalten
void relaiLicht(){
  
  if(flag_button_light == true){
    //Abfrage: Welcher status hat das Relai
    if(flag_light_state == false){
      //Ein
      digitalWrite(pin_relai_light, LOW);
      lightstate = "Ein";
      isAnUpdate=true;
      flag_light_state = true;
    }else{
      //Aus
      digitalWrite(pin_relai_light , HIGH);
     lightstate = "Aus";
     isAnUpdate=true;
     flag_light_state = false;
    }
  
  delay(500-85);
  
  //Rücksetzen des Flag aus
  flag_button_light = false;
  }
  
  
}

void menuAbfrage(){
  
  if(anzeigeAmLCD == mainScreen){
    Serial.println("MainScreen");

    if(flag_main_menu == true){
      lcd.clear();
      lcd.setCursor(5,0); // 2x16
      lcd.print("Garden");
      lcd.setCursor(2,1);
      lcd.print("Snow Blaster");
      flag_main_menu = false; //Rücksetzen des Flag //Wichtig Flag wieder auf true setzen im nächsten turn enum beachten //flag wird in der last if auf true gesetzt :)
    }
    

  //Durchfluss Wasser normale Düsen 
  }else if(anzeigeAmLCD == durchflussScreen){
    
    Serial.println("Durchfluss");
    
  
    if(flag_button_count_up == true){   //Abfrage: Knopf für Hoch
      Serial.println("Durchfluss hoch");
      druchflussUp();
      isAnUpdate=true;
      flag_durchfluss_menu = true;
      //Rücksetzen des Flag
      flag_button_count_up = false;
      
    }else if(flag_button_count_down == true){ //Abfrage: Knopf für Runter
      Serial.println("Durchfluss runter");
      druchflussDown();
      isAnUpdate=true;
      flag_durchfluss_menu = true;
   
    //Rücksetzen des Flag
    flag_button_count_down = false;
    }
      
    //Anzeigen
    
    if(flag_durchfluss_menu == true){
        lcd.clear();
    
        lcd.setCursor(0,0);
        lcd.print("Durchfluss");
    
        lcd.setCursor(0,1);
        lcd.print("Now:");
        lcd.setCursor(7,1);
        lcd.print(gradInProzent(position_durchfluss,180));
        lcd.setCursor(12, 1);
        lcd.print("%");
        flag_durchfluss_menu = false;
      }

  }else if(anzeigeAmLCD == umgebungsdatenScreen){
    Serial.println("Temperatur");
    tempLCD();
    
  }else if(anzeigeAmLCD == motordrehzahlScreen){
    Serial.println("Motordrehzahl");

    
    //Abfrage: Knopf für Runter
    if(flag_button_count_down == true){
      Serial.println("Motordrehzahl runter");
      //Aus
    digitalWrite(pin_control_ventilator_pwm , HIGH);
    ventistate = "Aus";
    isAnUpdate=true;
    flag_motordrehzahl_menu = true;
      
    
    //Rücksetzen des Flag
    flag_button_count_down = false;
    
    }else if(flag_button_count_up == true){ //Abfrage: Knopf für Hoch
      //Ein
      digitalWrite(pin_control_ventilator_pwm, LOW);
      ventistate = "Ein";
       //D11 Hiht
       isAnUpdate=true;
      flag_motordrehzahl_menu = true;    
      //Rücksetzen des Flag
      flag_button_count_up = false;
    }
    
    if(flag_motordrehzahl_menu == true){
      lcd.clear();

      lcd.setCursor(0,0);
      lcd.print("Ventilator");
    
      lcd.setCursor(0,1);
      lcd.print(ventistate);

      flag_motordrehzahl_menu = false;
      
    }


  }else if(anzeigeAmLCD == nukleatorduesenWasserScreen ){
    

    if(flag_button_count_up == true){ //Abfrage: Knopf für Hoch
      
      Serial.println("Nukleator hoch");
      druchflussNukleatorWaterUp();
      isAnUpdate=true; 
      flag_durchfluss_menu_nukleator_water = true;
      //Rücksetzen des Flag
      flag_button_count_up = false;
    
    }else if(flag_button_count_down == true){ //Abfrage: Knopf für Runter
      Serial.println("Nukleator runter");
      druchflussNukleatorWaterDown();
      isAnUpdate=true;
      flag_durchfluss_menu_nukleator_water = true;
      
    
    //Rücksetzen des Flag
    flag_button_count_down = false;
    }

    //Anzeige am LCD
    if(flag_durchfluss_menu_nukleator_water == true){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Nukleator Wasser");

      lcd.setCursor(0,1);
      lcd.print("Now:");
      lcd.setCursor(7,1);
      lcd.print(gradInProzent(position_durchfluss_nukleator_water,180));
      
      lcd.setCursor(12, 1);
      lcd.print("%");
      
      flag_durchfluss_menu_nukleator_water = false;
    }
  
  }else if(anzeigeAmLCD == nukleatorAirScreen){
    Serial.println("Nukleator Luft");
    
    //Abfrage: Knopf für Hoch
    if(flag_button_count_up == true){
      Serial.println("Nukleator hoch");
      druchflussNukleatorAirUp();
      isAnUpdate=true;
      flag_durchfluss_menu_nukleator_air = true;
      //Rücksetzen des Flag
      flag_button_count_up = false;
    }else if(flag_button_count_down == true){ //Abfrage: Knopf für Runter
      Serial.println("Nukleator runter");
      druchflussNukleatorAirDown();
      isAnUpdate=true;
      flag_durchfluss_menu_nukleator_air = true;
      
    //Rücksetzen des Flag
    flag_button_count_down = false;
    }

    //Anzeige am LCD
    if(flag_durchfluss_menu_nukleator_air == true){
        lcd.clear();
    
        lcd.setCursor(0,0);
        lcd.print("Nukleator Luft");
    
        lcd.setCursor(0,1);
        lcd.print("Now:");
        lcd.setCursor(7,1);
        lcd.print(gradInProzent(position_durchfluss_nukleator_air,180));
        lcd.setCursor(12, 1);
        lcd.print("%");
        
        flag_durchfluss_menu_nukleator_air = false;
      
    }
  }else if(anzeigeAmLCD == literprominute){ 

     literProMinuteLCD();

  }else{   

  
    Serial.println("Anzeige des LCD-Menus konnte nicht gefunden werden");
    
    //Rücksetzen der Flags um sie erneut aufrufen zu können;
    flag_main_menu = true;
    flag_durchfluss_menu = true;
    flag_motordrehzahl_menu = true;
    flag_durchfluss_menu_nukleator_air = true; 
    flag_durchfluss_menu_nukleator_water = true; 
    
    //Anzeige wieder von Vorne
    anzeigeAmLCD = mainScreen;
  
  }
}

  
