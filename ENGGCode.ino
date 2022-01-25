//Libraries 
#include <DHT.h> 
#include<LiquidCrystal_I2C.h> 
 
//Constants 
#define DHTPIN 7     // what pin we're connected to 
#define DHTTYPE DHT22   // DHT 22  (AM2302) 
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino 
 
 
 
//Variables for humidity sensor 
int chk; 
float hum;  //Stores humidity value 
float temp; //Stores temperature value 
int ThermistorPin = 0; 
 
//Variables for thermistor 
int Vo; 
float R1 = 10000; 
float logR2, R2, Te; 
float c1 = 0.000385937250220100, c2 = 0.000330024264854518, c3 = -9.26857851202681e-08;  
double temperature; 
 
//variables for BPM 
#define pulsePin A2 
int rate[10];                     
unsigned long sampleCounter = 0;  
unsigned long lastBeatTime = 0;   
unsigned long lastTime = 0, N; 
int BPM = 0; 
int IBI = 0; 
int P = 512; 
int T = 512; 
int thresh = 650;   
int amp = 100;                    
int Signal; 
boolean Pulse = false; 
boolean firstBeat = true;           
boolean secondBeat = true; 
boolean QS = false;     
 
//detection variables 
int result; 
 
//define LCD  
LiquidCrystal_I2C lcd(0x27,20,4); 
 
void setup() 
{ 
  dht.begin();  //initalize the humidity sensor 
  lcd.init();     // initialize the lcd  
  delay(20); //apparanetly this is best practice 
   
} 
 
void loop() 
{ 
    //humidity sensor 
    //Read data and store it to variables hum and temp 
    hum = dht.readHumidity(); 
    temp = dht.readTemperature(); 
    //Print temp and humidity values to lcd screen 
 
    //thermistor 
    Vo = analogRead(ThermistorPin);  //read voltage 
    temperature = calculateTemp(Vo); //calculate actual temperature 
     
    //heart rate 
    if (QS == true)  
    { 
       QS = false; 
       }  
       else if (millis() >= (lastTime + 20)) { 
       readPulse();  
       lastTime = millis(); 
    } 
     
    //detectHS 
    result = detectHS(temperature, BPM, temp, hum); 
    if(result == 0) 
    { 
      displayWarning(); 
    } 
    else if(result == 1) 
    { 
      displayReminder(); 
    } 
    else 
    { 
      lcd.clear(); 
      lcd.noBacklight(); 
    } 
     
    delay(100); //Delay 100ms 
 
} 
 
double calculateTemp(int Vo) 
{ 
  
  R2 = R1 * ((1023.0 /(float) Vo) - 1.0); 
  logR2 = log(R2); 
  Te = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); 
  Te = Te - 273.15; 
  return Te; 
 
} 
 
int detectHS(double temperature, int BPM, float temp, float hum) 
{ 
  if((BPM >= 125) && (temperature > 35)) 
  { 
    return 0;     
  } 
  else if ((BPM >= 90) && (temperature <= 33) && (temp >= 37) && (hum >= 70) 
  { 
    return 1; 
  } 
  else 
  { 
    return 2; 
  } 
 
} 
void displayWarning() 
{ 
  lcd.backlight(); 
  lcd.setCursor(0,0); 
  for(int i = 0; i <= 10; i++) 
  { 
    lcd.print("WARNING: AT RISK FOR HEAT STROKE"); 
    delay(500); 
  }  
} 
 
void displayReminder() 
{ 
  lcd.backlight(); 
  lcd.setCursor(0,0); 
  if (millis() % 900000 == 0) 
  { 
    lcd.print("DRINK WATER");  
    delay(10000); 
  } 
  lcd.noBacklight(); 
  lcd.clear(); 
   
} 
 
void readPulse() { 
 
  Signal = analogRead(pulsePin);               
  sampleCounter += 20;                                    // keep track of the time in mS  
  int N = sampleCounter - lastBeatTime;                   // monitor the time since the last beat to avoid noise 
 
  detectSetHighLow(); 
 
  if (N > 200) {                                                              // avoid high frequency noise 
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3) )        
      pulseDetected(); 
  } 
 
  if (Signal < thresh && Pulse == true) {  // when the values are going down, the beat is over 
    Pulse = false;                         // reset the Pulse flag so we can do it again      
    amp = P - T;                           // get amplitude of the pulse wave 
    thresh = amp / 2 + T;                   // set thresh at 50% of the amplitude 
    P = thresh;                              // reset these for next time 
    T = thresh; 
  } 
 
  if (N > 2500) {                               // if 2.5 seconds go by without a beat 
    thresh = 512;                               // set thresh default 
    P = 512;                                    // set P default 
    T = 512;                                    // set T default 
    lastBeatTime = sampleCounter;               // bring the lastBeatTime up to date 
    firstBeat = true;                            
    secondBeat = true;            
  } 
 
} 
 
void detectSetHighLow() { 
 
  if (Signal < thresh && N > (IBI / 5) * 3) { 
    if (Signal < T) {                        
      T = Signal;                // keep track of lowest point in pulse wave           
    } 
  } 
 
  if (Signal > thresh && Signal > P) {     
    P = Signal;                  // keep track of highest point in pulse wave          
  }                                        
 
} 
 
void pulseDetected() { 
  Pulse = true;                                   // set the Pulse flag when we think there is a pulse 
  IBI = sampleCounter - lastBeatTime;             // measure time between beats in mS 
  Serial.print("IBI = "); 
  Serial.println(IBI);    
  lastBeatTime = sampleCounter;                   // keep track of time for next pulse 
 
  if (firstBeat) {                                // if it's the first time we found a beat, if firstBeat == TRUE 
    firstBeat = false;                            // clear firstBeat flag 
    secondBeat = true; 
    return;                             
  } 
  if (secondBeat) {                                // if it's the second time we found a beat, if secondBeat == TRUE 
    secondBeat = false;                            // clear SecondBeat flag 
    for (int i = 0; i <= 9; i++) {                  
      rate[i] = IBI; 
    } 
  } 
 
  word runningTotal = 0;                            // clear the runningTotal variable          
 
  for (int i = 0; i <= 8; i++) {                    // shift data in the rate array 
    rate[i] = rate[i + 1];                          // and drop the oldest IBI value 
    runningTotal += rate[i];                        // add up the 9 oldest IBI values 
  } 
 
  rate[9] = IBI;                                    // add the latest IBI to the rate array 
  runningTotal += rate[9];                          // add the latest IBI to runningTotal 
  runningTotal /= 10;                               // average the last 10 IBI values 
  BPM = 60000 / runningTotal;                       // how many beats can fit into a minute? that's BPM! 
  QS = true;                                        // set Quantified Self flag (we detected a beat) 
} 

