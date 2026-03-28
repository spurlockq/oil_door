#include <DHT11.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
DHT11 dht11(8);

int d_pin = 10;
int in_1 = 6;
int in_2 = 7;

const float mintempF = 65.0;
const float maxtempF = 110.0;

const unsigned long fullStroketime = 10000;
//0 = fully retracted, 100 = fully extended
float estposition = 0.0;

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.setCursor(0, 1);
  lcd.print("Hum:");

  Serial.begin(9600);

  pinMode(d_pin, OUTPUT);
  pinMode(in_1, OUTPUT);
  pinMode(in_2, OUTPUT);

  // Home actuator to fully retracted at startup
  retractactuator(255);
  delay(fullStroketime + 1000);
  stopactuator();
  estposition = 0.0;
}

void loop() {
  //DHT11 logic
  int tempC = 0;
  int humidity = 0;
  int result = dht11.readTemperatureHumidity(tempC, humidity);
  
    //lcd.print("  Target Position: ");
    //lcd.print(targetPercent);
    //lcd.println("%");

    // Check the results of the readings.
    // If the reading is successful, print the temperature and humidity values.
    // If there are errors, print the appropriate error messages.
    if (result == 0) {
      float tempF = (tempC * 9.0/5.0) + 32.0;
      float targetPercent = (tempF - mintempF) * 100.0 / (maxtempF - mintempF);
      targetPercent = constrain(targetPercent, 0.0, 100.0);
      float difference = targetPercent - estposition;

      Serial.print("TempF: ");
      Serial.print(tempF);
      Serial.print("  Target: ");
      Serial.print(targetPercent);
      Serial.print("  Est: ");
      Serial.print(estposition);
      Serial.print("  Diff: ");
      Serial.println(difference);
      // if temp changes 0.5%, actuator will move to appropiate position
      if (fabs(difference) > 0.5){
        moveToPosition(targetPercent);
      }
      lcd.setCursor(6, 0);     // position after "Temp:"
      lcd.print("     ");      // clear old number
      lcd.setCursor(6, 0);
      lcd.print(tempF);
      lcd.print("F");

      lcd.setCursor(5, 1);     // position after "Hum:"
      lcd.print("     ");      // clear old number
      lcd.setCursor(5, 1);
      lcd.print(humidity);
      lcd.print("%");
    } else {
        // Print error message based on the error code.
        lcd.println(DHT11::getErrorString(result));
    }
  delay(500);
}

//void functions
void moveToPosition(float targetPercent){
  float delta = targetPercent - estposition;
  if (fabs(delta) < 1.0){
    stopactuator();
    return;
  }
  unsigned long moveTime = (unsigned long)(fabs(delta) / 100.0 * fullStroketime);

  if (delta > 0) {
    extendactuator(255);
  }
  else {
    retractactuator(255);
  }
  delay(moveTime);
  stopactuator();
  estposition = targetPercent;
}
void extendactuator(int speed_val){
  digitalWrite(in_1, LOW);
  digitalWrite(in_2, HIGH);
  analogWrite(d_pin, speed_val);
}
void retractactuator(int speed_val){
  digitalWrite(in_1, HIGH);
  digitalWrite(in_2, LOW);
  analogWrite(d_pin, speed_val);
}
void stopactuator(){
  analogWrite(d_pin, 0);
  digitalWrite(in_1, LOW);
  digitalWrite(in_2, LOW);
}

