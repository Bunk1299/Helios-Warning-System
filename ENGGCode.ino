/*Lienne use this website to make your circuit https://microcontrollerslab.com/pulse-sensor-arduino-tutorial/ */

//Libraries
#include <DHT.h>
#include<LiquidCrystal_I2C.h>
#include <PulseSensorPlayground.h>

//Constants
#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
#define USE_ARDUINNO_INTERRUPTS true


//Variables for humidity sensor
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
int ThermistorPin = 0;

//Variables for thermistor
int Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 0.000385937250220100, c2 = 0.000330024264854518, c3 = -9.26857851202681e-08; 
double temperature;

//variables for BPM
int bpmPin = 1;
int BPM;
int threshold = 550;  //Maybe Fine tune this???
PulseSensorPlayground pulseSensor; //creates pulseSensor object


//define LCD 
LiquidCrystal_I2C lcd(0x27,20,4);

void setup()
{
  Serial.begin(9600);  //bit rate for serial monitor
  dht.begin();  //initalize the humidity sensor
  lcd.init();     // initialize the lcd 
  lcd.init();
  lcd.backlight();
  
  //initalize the pulse sensor object
  pulseSensor.analogInput(bpmPin);
  pulseSensor.setThreshold(threshold);

  delay(20); //apparanetly this is best practice
  
}

void loop()
{
    //humidity sensor
    //Read data and store it to variables hum and temp
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    //Print temp and humidity values to lcd screen
    lcd.setCursor(0,3);
    lcd.print("Humidity: ");
    lcd.print(hum);
    lcd.setCursor(0,2);
    lcd.print("Ambient Temp: ");
    lcd.print(temp);

    //thermistor
    Vo = analogRead(ThermistorPin);  //read voltage
    temperature = calculateTemp(Vo); //calculate actual temperature
    lcd.setCursor(0,1);  //print to LCD
    lcd.print("Body Temp: ");
    lcd.print(temperature);
    
    //heart rate
    BPM = pulseSensor.getBeatsPerMinute();
    lcd.setCursor(0,0);
    lcd.print("Heart Rate: ");
    lcd.print(BPM);
    
    delay(100); //Delay 100ms
}

double calculateTemp(int Vo)
{
 
  R2 = R1 * ((1023.0 /(float) Vo) - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  return T;

}





   
