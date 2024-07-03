/*
  Example animated analogue meters using a ILI9341 TFT LCD screen
  Needs Font 2 (also Font 4 if using large scale label)
  Make sure all the display driver and pin connections are correct by
  editing the User_Setup.h file in the TFT_eSPI library folder.

  #########################################################################
  ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
  #########################################################################
  
  Library TFT_eSPI
  File User_Setup_Select.h 
  #include <User_Setup.h> - comment
  #include <User_Setups/Setup35_ILI9341_STM32_Port_Bus.h> - uncomment
  
  SKU_MRB3205_STM32F401CCU6 pinout
  RS      -> PB8 (DC по док.)
  RD      -> PB1
  WR      -> PB9
  RST     -> PB0
  CS      -> PB7
  DB8     -> PA0
  DB9     -> PA1
  DB10    -> PA2
  DB11    -> PA3
  DB12    -> PA4
  DB13    -> PA5
  DB14    -> PA6
  DB15    -> PA7
  BL      -> +3.3V
  VDD     -> +3.3V
  VDD     -> +3.3V
  GND     -> GND
  GND     -> GND

  DHT11    -> PB10
*/

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <DHT.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library      

#define TFT_GREY 0x5AEB
#define LOOP_PERIOD pdMS_TO_TICKS(35) // Display updates every 35 ms

float ltx1 = 0;    // Saved x coord of bottom of needle1
uint16_t osx1 = 120, osy1 = 120; // Saved x & y coords

float ltx2 = 0;   // Saved x coord of bottom of needle2
uint16_t osx2 = 120, osy2 = 250; // Saved x & y coords

uint32_t updateTime = 0;       // time for next update


// needle1
int old_analog1 =  -999; // Value last displayed
int old_digital1 = -999; // Value last displayed

// needle2
int old_analog2 =  -999; // Value last displayed
int old_digital2 = -999; // Value last displayed

int value[6] = {0, 0, 0, 0, 0, 0};
int old_value[6] = { -1, -1, -1, -1, -1, -1};
//int d1 = 0;
//int d2 = 0;

//DHT11
#define DHTPIN PB10
#define DHTTYPE DHT11
#define LOOP_PERIOD 2000
uint32_t updateTimeDht = 0;
float humidity = 0;
float temperature = 0;

DHT dht(DHTPIN, DHTTYPE);


void setup(void)
{
  Serial.begin(115200);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  // Draw analogue meters
  analogMeter1();
  analogMeter2();
  
  // Draw text
  //tft.setCursor(5, 280);
  //tft.setTextColor(TFT_WHITE);
  //tft.print("Test text.");
  //tft.drawCentreString("Test text.", 5, 290, 2);

  // Initialize temperature sensor
	updateTimeDht = millis();
  dht.begin();
}


/* LOOP */
void loop() {
  if (updateTime <= millis()) {
    updateTime = millis() + LOOP_PERIOD;

    if (updateTimeDht <= millis()) {
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      //float f = dht.readTemperature(true);

      if (!isnan(h) || !isnan(t)) {
        humidity = h;
        temperature = t;
        //output mesures to terminal
        Serial.println("T = " + String(temperature) + "C / H = " + String(humidity)+ "%RH");
      }
      //taimestamp to terminal
      uint32_t time = millis();
      Serial.println(millis()-time);
    }

    plotNeedle(humidity, 0);
    plotNeedle1(temperature, 0);

    // Compute heat index in Celsius (isFahreheit = false)
    float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    float dewPoint = computeDewPoint(temperature, humidity, false);
    
    // Compute comfort status in Celsius (isFahreheit = false)
    String comfortStatus = getComfortRatio(temperature, humidity, false);
    String outputLine;

    tft.fillRect(0, 260, 239, 35, TFT_GREY);
    tft.fillRect(5, 263, 230, 28, TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.drawString(comfortStatus, 7, 265, 2);

    outputLine = " T:" + String(temperature) + " H:" + String(humidity) + " I:" + String(heatIndex) + " D:" + String(dewPoint);
    
    tft.setTextColor(TFT_WHITE);
    tft.fillRect(5, 300, 230, 15, TFT_BLACK);
    tft.drawString(outputLine, 5, 300, 2);
  }
}

// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################
void analogMeter1()
{
  // Meter outline
  tft.fillRect(0, 0, 239, 126, TFT_GREY);
  tft.fillRect(5, 3, 230, 119, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);  // Text colour

  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coodinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (100 + tl) + 120;
    uint16_t y0 = sy * (100 + tl) + 140;
    uint16_t x1 = sx * 100 + 120;
    uint16_t y1 = sy * 100 + 140;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (100 + tl) + 120;
    int y2 = sy2 * (100 + tl) + 140;
    int x3 = sx2 * 100 + 120;
    int y3 = sy2 * 100 + 140;

    // Yellow zone limits
    //if (i >= -50 && i < 0) {
    //  tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
    //  tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
    //}

    // Green zone limits
    if (i >= 0 && i < 25) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
    }

    // Orange zone limits
    if (i >= 25 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
    }

    // Short scale tick length
    if (i % 25 != 0) tl = 8;

    // Recalculate coords incase tick lenght changed
    x0 = sx * (100 + tl) + 120;
    y0 = sy * (100 + tl) + 140;
    x1 = sx * 100 + 120;
    y1 = sy * 100 + 140;

    // Draw tick
    tft.drawLine(x0, y0, x1, y1, TFT_BLACK);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (100 + tl + 10) + 120;
      y0 = sy * (100 + tl + 10) + 140;
      switch (i / 25) {
        case -2: tft.drawCentreString("0", x0, y0 - 12, 2); break;
        case -1: tft.drawCentreString("25", x0, y0 - 9, 2); break;
        case 0: tft.drawCentreString("50", x0, y0 - 6, 2); break;
        case 1: tft.drawCentreString("75", x0, y0 - 9, 2); break;
        case 2: tft.drawCentreString("100", x0, y0 - 12, 2); break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * 100 + 120;
    y0 = sy * 100 + 140;
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
  }

  tft.drawString("%RH", 5 + 230 - 40, 119 - 20, 2); // Units at bottom right
  tft.drawCentreString("%RH", 120, 70, 4); // Comment out to avoid font 4
  tft.drawRect(5, 3, 230, 119, TFT_BLACK); // Draw bezel line

  plotNeedle(0, 0); // Put meter needle at 0
}


// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################
void analogMeter2()
{
  // Meter outline
  tft.fillRect(0, 130, 239, 126, TFT_GREY);
  tft.fillRect(5, 133, 230, 119, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);  // Text colour

  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coodinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (100 + tl) + 120;
    uint16_t y0 = sy * (100 + tl) + 270;//140
    uint16_t x1 = sx * 100 + 120;
    uint16_t y1 = sy * 100 + 270; //140

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (100 + tl) + 120;
    int y2 = sy2 * (100 + tl) + 270;//140
    int x3 = sx2 * 100 + 120;
    int y3 = sy2 * 100 + 270;//140

    // Yellow zone limits
    //if (i >= -50 && i < 0) {
    //  tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
    //  tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
    //}

    // Green zone limits
    if (i >= 0 && i < 25) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
    }

    // Orange zone limits
    if (i >= 25 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
    }

    // Short scale tick length
    if (i % 25 != 0) tl = 8;

    // Recalculate coords incase tick lenght changed
    x0 = sx * (100 + tl) + 120;
    y0 = sy * (100 + tl) + 270;//140
    x1 = sx * 100 + 120;
    y1 = sy * 100 + 270; //140

    // Draw tick
    tft.drawLine(x0, y0, x1, y1, TFT_BLACK);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (100 + tl + 10) + 120;
      y0 = sy * (100 + tl + 10) + 270;//140
      switch (i / 25) {
        case -2: tft.drawCentreString("0", x0, y0 - 12, 2); break;
        case -1: tft.drawCentreString("25", x0, y0 - 9, 2); break;
        case 0: tft.drawCentreString("50", x0, y0 - 6, 2); break;
        case 1: tft.drawCentreString("75", x0, y0 - 9, 2); break;
        case 2: tft.drawCentreString("100", x0, y0 - 12, 2); break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * 100 + 120;
    y0 = sy * 100 + 270; //140
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
  }

  tft.drawString("C", 5 + 230 - 40, 249 - 20, 2); // Units at bottom right 119=249
  tft.drawCentreString("C", 250, 70, 4); // Comment out to avoid font 4 120=250
  tft.drawRect(5, 3, 230, 119, TFT_BLACK); // Draw bezel line

  plotNeedle1(0, 0); // Put meter needle at 0
}


// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(int value, byte ms_delay)
{
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  char buf[8]; 
  dtostrf(value, 4, 0, buf);
  tft.drawRightString(buf, 40, 119 - 20, 2);

  if (value < -10) value = -10; // Limit value to emulate needle end stops
  if (value > 110) value = 110;

  // Move the needle util new value reached
  while (!(value == old_analog1)) {
    if (old_analog1 < value) old_analog1++;
    else old_analog1--;

    if (ms_delay == 0) old_analog1 = value; // Update immediately id delay is 0

    float sdeg = map(old_analog1, -10, 110, -150, -30); // Map value to angle
    // Calcualte tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg + 90) * 0.0174532925);

    // Erase old needle image
    tft.drawLine(120 + 20 * ltx1 - 1, 140 - 20, osx1 - 1, osy1, TFT_WHITE);
    tft.drawLine(120 + 20 * ltx1, 140 - 20, osx1, osy1, TFT_WHITE);
    tft.drawLine(120 + 20 * ltx1 + 1, 140 - 20, osx1 + 1, osy1, TFT_WHITE);

    // Re-plot text under needle
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString("%RH", 120, 70, 4); // // Comment out to avoid font 4

    // Store new needle end coords for next erase
    ltx1 = tx;
    osx1 = sx * 98 + 120;
    osy1 = sy * 98 + 140;

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(120 + 20 * ltx1 - 1, 140 - 20, osx1 - 1, osy1, TFT_RED);
    tft.drawLine(120 + 20 * ltx1, 140 - 20, osx1, osy1, TFT_MAGENTA);
    tft.drawLine(120 + 20 * ltx1 + 1, 140 - 20, osx1 + 1, osy1, TFT_RED);

    // Slow needle down slightly as it approaches new postion
    if (abs(old_analog1 - value) < 10) ms_delay += ms_delay / 5;
  }
}

// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle1(int value, byte ms_delay)
{
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  char buf[8]; 
  dtostrf(value, 4, 0, buf);
  tft.drawRightString(buf, 40, 249 - 20, 2); //119

  if (value < -10) value = -10; // Limit value to emulate needle end stops
  if (value > 110) value = 110;

  // Move the needle util new value reached
  while (!(value == old_analog2)) {
    if (old_analog2 < value) old_analog2++;
    else old_analog2--;

    if (ms_delay == 0) old_analog2 = value; // Update immediately if delay is 0

    float sdeg = map(old_analog2, -10, 110, -150, -30); // Map value to angle
    
    // Calcualte tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg + 90) * 0.0174532925);

    // Erase old needle image
    tft.drawLine(120 + 20 * ltx2 - 1, 270 - 20, osx2 - 1, osy2, TFT_WHITE); //140
    tft.drawLine(120 + 20 * ltx2, 270 - 20, osx2, osy2, TFT_WHITE);//140
    tft.drawLine(120 + 20 * ltx2 + 1, 270 - 20, osx2 + 1, osy2, TFT_WHITE);//140

    // Re-plot text under needle
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString("C", 120, 200, 4); // // Comment out to avoid font 4 y=70

    // Store new needle end coords for next erase
    ltx2 = tx;
    osx2 = sx * 98 + 120;
    osy2 = sy * 98 + 270; //140

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(120 + 20 * ltx2 - 1, 270 - 20, osx2 - 1, osy2, TFT_RED);//140
    tft.drawLine(120 + 20 * ltx2, 270 - 20, osx2, osy2, TFT_MAGENTA);//140
    tft.drawLine(120 + 20 * ltx2 + 1, 270 - 20, osx2 + 1, osy2, TFT_RED);//140

    // Slow needle down slightly as it approaches new postion
    if (abs(old_analog2 - value) < 10) ms_delay += ms_delay / 5;
  }
}

float computeDewPoint(float temperature, float percentHumidity, bool isFahrenheit)
  {
	  // reference: http://wahiduddin.net/calc/density_algorithms.htm
	  if (isFahrenheit)
	  {
		  temperature = dht.convertFtoC(temperature);
	  }
	  double calc_temp = 373.15 / (273.15 + (double)temperature);
	  double calc_sum = -7.90298 * (calc_temp - 1);
	  calc_sum += 5.02808 * log10(calc_temp);
	  calc_sum += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / calc_temp))) - 1);
	  calc_sum += 8.1328e-3 * (pow(10, (-3.49149 * (calc_temp - 1))) - 1);
	  calc_sum += log10(1013.246);
	  double calc_value = pow(10, calc_sum - 3) * (double)percentHumidity;
	  double calc_dew_temp = log(calc_value / 0.61078); // temp var
	  calc_dew_temp = (241.88 * calc_dew_temp) / (17.558 - calc_dew_temp);
	  return isFahrenheit ? dht.convertCtoF(calc_dew_temp) : calc_dew_temp;
  }

String getComfortRatio(float temperature, float humidity, bool isFahrenheit)
{
  /*
    https://github.com/beegee-tokyo/arduino-DHTesp
    Written by Mark Ruys, mark@paracas.nl.
    Updated to work with ESP32 by Bernd Giesecke, bernd@giesecke.tk
    Updated to work with STM32 BlackPill by Aleksey Tkachenko, alexei.tkch@gmail.com
    GNU General Public License, check LICENSE for more information.
    All text above must be included in any redistribution.
  */

	float ratio = 100; //100%
	float distance = 0;
	float kTempFactor = 3;	//take into account the slope of the lines
	float kHumidFactor = 0.1; //take into account the slope of the lines
	uint8_t tempComfort = 0;
  
  String comfortStatus = "Comfort_Undefined";
  
  //Too hot line AB
  float tooHot_m = -0.095;
	float tooHot_b = 32.85;
	//Too humid line BC
	float tooHumid_m = -56.5;
	float tooHumid_b = 3981.2;
	//Too cold line DC
	float tooCold_m = -0.04175;
	float tooHCold_b = 23.476675;
	//Too dry line AD
	float tooDry_m = -77.8;
	float tooDry_b = 2364;

	if (isFahrenheit)
	{
		temperature = dht.convertFtoC(temperature);
	}

	distance = temperature - (humidity * tooHot_m + tooHot_b);
	if (distance > 0)
	{
		//update the comfort descriptor
		tempComfort += 1;//(uint8_t)Comfort_TooHot;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kTempFactor;
	}

	distance = temperature - (humidity * tooHumid_m + tooHumid_b);
	if (distance > 0)
	{
		//update the comfort descriptor
		tempComfort += 8;//(uint8_t)Comfort_TooHumid;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kHumidFactor;
	}

	distance = (humidity * tooCold_m + tooHCold_b) - temperature;
	if (distance > 0)
	{
		//update the comfort descriptor
		tempComfort += 2;//(uint8_t)Comfort_TooCold;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kTempFactor;
	}

	distance = (humidity * tooDry_m + tooDry_b) - temperature;
	if (distance > 0)
	{
		//update the comfort descriptor
		tempComfort += 4;//(uint8_t)Comfort_TooDry;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kHumidFactor;
	}

  switch(tempComfort) {
      case 0:
        comfortStatus = "Comfort_OK";
        break;
      case 1:
        comfortStatus = "Comfort_TooHot";
        break;
      case 2:
        comfortStatus = "Comfort_TooCold";
        break;
      case 4:
        comfortStatus = "Comfort_TooDry";
        break;
      case 8:
        comfortStatus = "Comfort_TooHumid";
        break;
      case 9:
        comfortStatus = "Comfort_HotAndHumid";
        break;
      case 5:
        comfortStatus = "Comfort_HotAndDry";
        break;
      case 10:
        comfortStatus = "Comfort_ColdAndHumid";
        break;
      case 6:
        comfortStatus = "Comfort_ColdAndDry";
        break;
      default:
        comfortStatus = "Comfort_Undefined";
        break;
    };
    

	if (ratio < 0) ratio = 0;

	return comfortStatus;
}


/*
float computeAbsoluteHumidity(float temperature, float percentHumidity, bool isFahrenheit)
{
	// Calculate the absolute humidity in g/m³
	// https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
	if (isFahrenheit)
	{
		temperature = dht.convertFtoC(temperature);
	}

	float absHumidity;
	float absTemperature;
	absTemperature = temperature + 273.15;

	absHumidity = 6.112;
	absHumidity *= exp((17.67 * temperature) / (243.5 + temperature));
	absHumidity *= percentHumidity;
	absHumidity *= 2.1674;
	absHumidity /= absTemperature;

	return absHumidity;
}
*/
