#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Keypad.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* Gemini_Token = "";

unsigned long channelID = 1;
const char* writeAPIKey = "";

#define TFT_DC 2
#define TFT_CS 15
#define LED 16

const uint8_t ROWS = 4;
const uint8_t COLS = 3;
char keys[ROWS][COLS] = {
  { '1', '2', '3'},
  { '4', '5', '6'},
  { '7', '8', '9'},
  { 'H', 'W', 'P'}
};

uint8_t colPins[COLS] = {12, 13, 17};
uint8_t rowPins[ROWS] = { 14, 27, 26, 25 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

const String initialTopics[] = {
  "Coping with anxiety",
  "Managing stress",
  "Dealing with sadness",
  "Improving sleep",
  "Handling loneliness",
  "Building self-confidence",
  "Overcoming fear",
  "Finding motivation",
  "Talking about a recent extreme event"
};

String geminiCurrentFlow[9];

enum DisplayState {
  INITIAL_TOPICS,
  GEMINI_PROVIDED_OPTIONS,
  DIRECT_ANSWER_RECEIVED
};

DisplayState currentDisplayState = INITIAL_TOPICS;

int ts_field1_topicsCount = 0;
int ts_field2_initialTopic = 0;
int ts_field3_geminiHttpCode = 0;

unsigned long lastThingSpeakBatchUpdate = 0;
const unsigned long THING_SPEAK_BATCH_INTERVAL = 20000;

void sendThingSpeakUpdate(int field1_val, int field2_val, int field3_val) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected for ThingSpeak batch update.");
    return;
  }

  if (millis() - lastThingSpeakBatchUpdate < THING_SPEAK_BATCH_INTERVAL) {
    Serial.println("ThingSpeak batch update throttled. Waiting for interval.");
    delay(THING_SPEAK_BATCH_INTERVAL - (millis() - lastThingSpeakBatchUpdate));
    return;
  }

  HTTPClient http;
  String url = "http://api.thingspeak.com/update?";
  url += "api_key=";
  url += writeAPIKey;
  url += "&field1=";
  if (field3_val = 0){
    field1_val = -1;
  }
  url += String(field1_val);
  if (field2_val != 0) {
    url += "&field2=";
    url += String(field2_val);
  }
  url += "&field3=";
  url += String(field3_val);

  Serial.print("ThingSpeak Batch Update URL: ");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.printf("ThingSpeak batch update successful. Response: %s\n", payload.c_str());
  } else {
    Serial.printf("ThingSpeak batch update failed. HTTP Code: %d, Error: %s\n", httpCode, http.errorToString(httpCode).c_str());
  }
  http.end();

  lastThingSpeakBatchUpdate = millis();
}


void clearAndSetupTFT() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.setTextWrap(true);
}

void tftPrintWrapped(const String& text, uint16_t color) {
  tft.setTextColor(color);
  tft.print(text);
}

bool parseGeminiResponseForOptions(const String& geminiResponse) {
  for (int i = 0; i < 9; ++i) {
    geminiCurrentFlow[i] = "";
  }

  bool optionsFound = false;
  int currentOptionIndex = 0;
  int startIndex = 0;

  for (int i = 1; i <= 9; ++i) {
    String searchPattern = String(i) + ". ";
    int pos = geminiResponse.indexOf(searchPattern, startIndex);

    if (pos != -1) {
      optionsFound = true;
      int endPos = geminiResponse.indexOf('\n', pos + 3);
      String optionText;
      if (endPos != -1) {
        optionText = geminiResponse.substring(pos + 3, endPos);
        startIndex = endPos + 1;
      } else {
        optionText = geminiResponse.substring(pos + 3);
        startIndex = geminiResponse.length();
      }
      optionText.trim();
      if (currentOptionIndex < 9) {
        geminiCurrentFlow[currentOptionIndex++] = optionText;
      }
    } else {
      if (i == 1) return false;
      break;
    }
  }
  ts_field1_topicsCount = currentOptionIndex;
  return optionsFound;
}

String Gemini_API(const String& user_input) {
  clearAndSetupTFT();
  tftPrintWrapped("Loading answer...", ILI9341_WHITE);
  HTTPClient https;
  String url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + (String)Gemini_Token;

  if (https.begin(url)) {
    String instructions = "You are Eliza, a supportive and empathetic psychologist specializing in coping with extreme events. Your primary goal is to provide concise, helpful guidance. When a user's input requires a direct, factual, or singular supportive answer (e.g., 'What is anxiety?' or 'I feel tired.'), respond with a clear, direct text (with no questions back to the user). When a user's input indicates a need for deeper exploration, more details, or a guided choice (e.g., 'I'm struggling with everything,' or 'What should I do next?'), provide a numbered list of 1-9 concise topics or solutions for them to choose from. Each option must be a single line. If the user provides a free-form response when numbered options were expected, acknowledge it and gently guide them back to the numbered options if further detail is needed. If their input is clearly the solution to a previous problem, congratulate them and provide a direct answer without options or questions. All user input is via a keypad with numbers 1-9, H, W, P. Prioritize clarity and conciseness in all interactions.";

    https.addHeader("Content-Type", "application/json");

    String payload = "{\"contents\": [{\"parts\":[{\"text\":\"" + instructions + "\\n" + user_input + "\"}]}]}";

    Serial.println("Sending payload: " + payload);

    int httpCode = https.POST(payload);
    ts_field3_geminiHttpCode = httpCode;

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String responsePayload = https.getString();
      Serial.println("Received payload: " + responsePayload);

      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, responsePayload);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        currentDisplayState = DIRECT_ANSWER_RECEIVED;
        https.end();
        return "Error parsing API response.";
      }

      String answer = doc["candidates"][0]["content"]["parts"][0]["text"];
      https.end();
      answer.trim();

      if (parseGeminiResponseForOptions(answer)) {
        currentDisplayState = GEMINI_PROVIDED_OPTIONS;
      } else {
        currentDisplayState = DIRECT_ANSWER_RECEIVED;
      }
      return answer;
    } else {
      Serial.printf("[HTTPS] POST failed, error: %s (HTTP Code: %d)\n", https.errorToString(httpCode).c_str(), httpCode);
      currentDisplayState = DIRECT_ANSWER_RECEIVED;
      https.end();
      return "Error: Could not get response from Gemini API. HTTP Code: " + String(httpCode);
    }
  } else {
    Serial.printf("[HTTPS] Unable to connect to Gemini API\n");
    currentDisplayState = DIRECT_ANSWER_RECEIVED;
    return "Error: Unable to connect to Gemini API.";
  }
}

void displayCurrentOptions() {
  clearAndSetupTFT();

  if (currentDisplayState == INITIAL_TOPICS) {
    tftPrintWrapped("Welcome! I'm Eliza your personal AI psychologist.\n\n", ILI9341_WHITE);
    tftPrintWrapped("How can I help you today? Please choose one (1-9):\n\n", ILI9341_WHITE);
    for (int i = 0; i < 9; ++i) {
      geminiCurrentFlow[i] = initialTopics[i];
      tftPrintWrapped(String(i + 1) + ". " + geminiCurrentFlow[i] + "\n\n", ILI9341_WHITE);
    }
  } else if (currentDisplayState == GEMINI_PROVIDED_OPTIONS) {
    tftPrintWrapped("Eliza suggests:\n", ILI9341_WHITE);
    bool anyOptionsFound = false;
    for (int i = 0; i < 9; ++i) {
      if (geminiCurrentFlow[i].length() > 0) {
        tftPrintWrapped(String(i + 1) + ". " + geminiCurrentFlow[i] + "\n\n", ILI9341_WHITE);
        anyOptionsFound = true;
      }
    }
    if (!anyOptionsFound) {
      tftPrintWrapped("No new options provided by Eliza. Press 'H' to return to main topics.\n", ILI9341_WHITE);
    }
  } else if (currentDisplayState == DIRECT_ANSWER_RECEIVED) {
    tftPrintWrapped("Eliza provided a direct answer. Press 'H' to return to main topics.\n", ILI9341_WHITE);
  }

  tftPrintWrapped("\n--- Controls ---\n\n", ILI9341_WHITE);
  tftPrintWrapped("H: Go to topics.\n", ILI9341_WHITE);
  tftPrintWrapped("W: WiFi toggle.\n", ILI9341_WHITE);
  tftPrintWrapped("P: Restart device.\n", ILI9341_WHITE);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  tft.begin();
  tft.setRotation(1);

  clearAndSetupTFT();
  tft.println("Connecting to WiFi");
  digitalWrite(LED, HIGH);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
    Serial.print(".");
  }

  tft.print("\nOK! IP=");
  tft.println(WiFi.localIP());
  Serial.println("\nWiFi connected with IP: " + WiFi.localIP().toString());
  tft.println("\n");
  tft.println("#######  ##       ##  ########   #### ");
  tft.println("##       ##       ##     ###    ##  ##");
  tft.println("#####    ##       ##    ###     ######");
  tft.println("##       ##       ##   ###      ##  ##");
  tft.println("#######  #######  ##  ########  ##  ##");
  delay(2000);

  displayCurrentOptions();
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (WiFi.status() != WL_CONNECTED) {
      if (key == 'W') {
        clearAndSetupTFT();
        tft.println("Reconnecting to WiFi...");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          tft.print(".");
          Serial.print(".");
        }
        tft.println("\nReconnected! IP=");
        tft.println(WiFi.localIP());
        digitalWrite(LED, HIGH);
        Serial.println("\nWiFi Reconnected with IP: " + WiFi.localIP().toString());
        delay(2000);
        displayCurrentOptions();
        return;
      }
      else if (key == 'P') {
        clearAndSetupTFT();
        tftPrintWrapped("Device is restarting...", ILI9341_RED);
        digitalWrite(LED, LOW);
        delay(1000);
        ESP.restart();
        return;
      } else {
        clearAndSetupTFT();
        tftPrintWrapped("WiFi is not connected. Press 'W' to reconnect or 'P' to restart", ILI9341_RED);
        delay(3000);
        return;
      }
    }

    clearAndSetupTFT();
    tft.setTextColor(ILI9341_WHITE);
    tft.print("You selected: ");
    tft.println(key);
    Serial.println("Key pressed: " + String(key));
    delay(500);

    if (key >= '1' && key <= '9') {
      int index = key - '1';
      String user_input_to_gemini = "";

      if (currentDisplayState == INITIAL_TOPICS || currentDisplayState == GEMINI_PROVIDED_OPTIONS) {
        if (currentDisplayState == INITIAL_TOPICS) {
          ts_field2_initialTopic = key - '0';
        }
        if (geminiCurrentFlow[index].length() > 0) {
          user_input_to_gemini = "I choose option " + String(index + 1) + ": " + geminiCurrentFlow[index];
        } else {
          tftPrintWrapped("Invalid option selected. Please choose from the displayed options.", ILI9341_RED);
          delay(2000);
          displayCurrentOptions();
          return;
        }
      } else {
        tftPrintWrapped("Eliza provided a direct answer. Please press 'H' to return to topics.", ILI9341_RED);
        delay(2000);
        displayCurrentOptions();
        return;
      }

      String Answer = Gemini_API(user_input_to_gemini);
      Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
      clearAndSetupTFT();
      tftPrintWrapped(Answer, ILI9341_GREEN);
      delay(10000); 
      tft.println("\n---");

      
      sendThingSpeakUpdate(ts_field1_topicsCount, ts_field2_initialTopic, ts_field3_geminiHttpCode);

      ts_field1_topicsCount = 0;
      ts_field2_initialTopic = 0;
      ts_field3_geminiHttpCode = 0;

      displayCurrentOptions();
    } else if (key == 'H') {
      currentDisplayState = INITIAL_TOPICS;
      displayCurrentOptions();
    } else if (key == 'W') {
      if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
        tft.println("Disconnected from WiFi.");
        digitalWrite(LED, LOW);
        Serial.println("WiFi Disconnected.");
      }
    } else if (key == 'P') {
      tft.println("Device is restarting...");
      digitalWrite(LED, LOW);
      delay(1000);
      ESP.restart();
    } else {
      tftPrintWrapped("Invalid selection. Please choose a number between 1 and 9, or H, W, P.", ILI9341_RED);
      delay(2000);
      displayCurrentOptions();
    }
  }
  delay(50);
}