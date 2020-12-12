#include <Arduino.h>
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <BleKeyboard.h>
#include <ArduinoJson.h>
#include <secrets.h>

BleKeyboard bleKeyboard("Soko Shuffler");

String currentUrl;
int totalResults;

void clearScreen()
{
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("                                                                                                                                                                            ");
}

void displaySoko(String topic, String title, String description)
{
  clearScreen();

  //display topic
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(5, 10);
  M5.Lcd.print(topic.substring(0, 19));

  //display title
  int titleMax = (title.length() > 26) ? 26 : title.length();
  M5.Lcd.setCursor(5, 30);
   M5.Lcd.setTextSize(3);
  M5.Lcd.print(title.substring(0, titleMax));

  //display description
  if (titleMax > 14)
  {
    M5.Lcd.setCursor(5, 90);
  }
  else
  {
    M5.Lcd.setCursor(5, 70);
  }
  M5.Lcd.setTextSize(1);
  M5.Lcd.print(description);
}

void connectToNetwork()
{
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Establishing connection to WiFi...");
    delay(1000);
  }

  Serial.println("Wifi connected!");
}

void establishBluetoothConnection()
{
  bleKeyboard.begin();

  while (!bleKeyboard.isConnected())
  {
    Serial.println("Establishing bluetooth connection...");
    delay(1000);
  }

  Serial.println("Bluetooth connection established!");
}

void initDisplay()
{
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 60);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Getting Ready!");
}

void fetchSoko(int offset = 0)
{
  Serial.println("Fetching soko...");

  HTTPClient http;

  http.begin("https://mediathekviewweb.de/api/query");
  http.addHeader("Content-Type", "text/plain");

  String postRequest = String("{\"queries\":[{\"fields\":[\"title\",\"topic\"],\"query\":\"soko\"},{\"fields\":[\"channel\"],\"query\":\"zdf\"}],\"sortBy\":\"timestamp\",\"sortOrder\":\"desc\",\"future\":false,\"offset\":") + String(offset) + String(",\"size\":1}");
  int httpCode = http.POST(postRequest);
  Serial.println(httpCode);

  if (httpCode == 200)
  {
    String response = http.getString();
    Serial.println(response);

    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    const char *topic = doc["result"]["results"][0]["topic"];
    const char *title = doc["result"]["results"][0]["title"];
    const char *description = doc["result"]["results"][0]["description"];
    String url = doc["result"]["results"][0]["url_video_hd"];
    totalResults = doc["result"]["queryInfo"]["totalResults"];

    displaySoko(topic, title, description);
    currentUrl = url;
  }
  else
  {
    Serial.println("Error fetching soko");
  }

  http.end();
}

void setup()
{
  M5.begin();
  Serial.begin(115200);
  initDisplay();
  connectToNetwork();
  establishBluetoothConnection();
  randomSeed(analogRead(0));
  fetchSoko(0);
}

void loop()
{
  M5.update();
  if (M5.BtnA.wasPressed())
  {
    int offset = random(0, totalResults + 1);
    fetchSoko(offset);
  }
  if (M5.BtnB.wasPressed())
  {
    bleKeyboard.write(KEY_F5);
    delay(1000);
    for (int i = 0; i < 10; i++)
    {
      bleKeyboard.write(KEY_RIGHT_ARROW);
      delay(100);
    }
    bleKeyboard.write(KEY_RETURN);
    delay(5000);
    for (int i = 0; i < currentUrl.length(); i++)
    {
      bleKeyboard.print(currentUrl[i]);
      delay(100);
    }
    bleKeyboard.write(KEY_RETURN);
  }
}
