#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#include <SoftwareSerial.h> // ket noi voi ESP
SoftwareSerial espSerial(10,11); // RX/TX

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int measurePin = A1;
int ledPower = 3;

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

unsigned long timer1, timer2;

int lm35Pin = A2;

float data1, data2, data3, data4;
//khai báo cảm biến dht
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int pinOut = A0 ; 
float A,Um,R1,RL,R,C;
int R0 = 156200 ;

void setup(){
  espSerial.begin(9600);
  pinMode(ledPower,OUTPUT);
  pinMode(measurePin, INPUT);
  Wire.begin();
  dht.begin();
  pinMode(DHTPIN, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);// initialize with the I2C addr 0x3C (for the 128x32)(initializing the display)
  display.setTextSize(1,1);
  display.setTextColor(WHITE);
}
void loop(){
  lm35();
//    int h = dht22();
//    int t = dht22();
//    int dustDensity = dust();
    delay(3000); // đợi 3s rồi đọc giá trị cảm biến dht22
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if(millis() - timer1 >30000){
      getdust();
      timer1 = millis();
      }
    if(millis() - timer2 >30000){
      getCO();
      timer2 = millis();
    }  
    //gửi dữ liệu các cảm biến sang esp8266
    data1 = t;
    data2 = h;
    data3 = dustDensity;
    data4 = C;
    espSerial.print(data1, 2); espSerial.print("A");
    espSerial.print(data2, 2); espSerial.print("B");
    espSerial.print(data3, 2); espSerial.print("C");
    espSerial.print(data4, 2); espSerial.print("D");
    espSerial.print("\n");
    // hiển thị ra màn hình OLED
    display.clearDisplay();
    display.print("Humidity: ");
    display.print(h);
    display.print(" %");
    display.setCursor(5, 10);
    display.print("Temperature: ");
    display.print(t);
    display.print(" C");
    display.setCursor(5, 25);
    display.print("PM2.5: ");
    display.print(dustDensity);
    display.print(" ug/m3");
    display.setCursor(5, 40);
    display.print("CO : ");
    display.print(C);
    display.print("ppm");
    display.setCursor(5, 55);
    display.display();
}
void getdust(){ // đọc giá trị cảm biến bụi 
  digitalWrite(ledPower,LOW); // Bật IR LED
    delayMicroseconds(samplingTime);  //Delay 0.28ms
    voMeasured = analogRead(measurePin); // Đọc giá trị ADC V0
    delayMicroseconds(deltaTime); //Delay 0.04ms
    digitalWrite(ledPower,HIGH); // Tắt LED
    delayMicroseconds(sleepTime); //Delay 9.68ms
  // Tính điện áp từ giá trị ADC
    calcVoltage = voMeasured * (5.0 / 1024); //Điệp áp Vcc của cảm biến (5.0 hoặc 3.3)
  //dustDensity = (0.17 * calcVoltage - 0.1)*1000;
    dustDensity = ((calcVoltage - 0.6)/5.8)*1000; // mg/m3
    if(dustDensity > 500){
      dustDensity = 500;
      }
    if(dustDensity <= 0){
      dustDensity = 0;
      }
      return dustDensity;
  }
//void storeData(){ lưu dữ liệu mỗi 5p
//  
//  }

void getCO(){ 
  A = analogRead(pinOut);
  Um = (5*A)/1023;
  R1 = ((5-Um)/Um)*10000;
  R = (R1 / R0);
  C = (pow (R, -1.409)) * 106,13; //ppm
  return C; 
  
}
void lm35(){
  int reading = analogRead(lm35Pin);  
  float voltage = reading * 5.0 / 1024.0;
  float temp = voltage * 100.0;
  Serial.println(temp);
  }
void updateAQI(){
  
  }
