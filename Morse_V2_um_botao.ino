#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUZZER_PIN 2
#define BUTTON_PIN 5 // Botão para registrar pontos e traços
#define CLEAR_BUTTON_PIN 6 // Botão para limpar o display
#define LED_PIN 12
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String myWord = "";
String decodedText = "";
unsigned long buttonPressStartTime = 0; // Para rastrear o tempo de pressão do botão
bool buttonPressed = false;
unsigned long lastPressTime = 0; // Para rastrear o tempo da última pressão
const unsigned long dotDuration = 200; // Duração para considerar um ponto
const unsigned long dashDuration = 150; // Duração para considerar um traço
const unsigned long debounceDelay = 200; // Tempo para considerar uma sequência completa

// Tabela de Morse
const char* morseToLetter[36] = {
    ".-",   // A
    "-...", // B
    "-.-.", // C
    "-..",  // D
    ".",    // E
    "..-.", // F
    "--.",  // G
    "....", // H
    "..",   // I
    ".---", // J
    "-.-",  // K
    ".-..", // L
    "--",   // M
    "-.",   // N
    "---",  // O
    ".--.", // P
    "--.-", // Q
    ".-.",  // R
    "...",  // S
    "-",    // T
    "..-",  // U
    "...-", // V
    ".--",  // W
    "-..-", // X
    "-.--", // Y
    "--..", // Z
    "-----", // 0
    ".----", // 1
    "..---", // 2
    "...--", // 3
    "....-", // 4
    ".....", // 5
    "-....", // 6
    "--...", // 7
    "---..", // 8
    "----."  // 9
};

const char letters[36] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
    'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', 
    '4', '5', '6', '7', '8', '9'
};

void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Botão para pontos e traços
    pinMode(CLEAR_BUTTON_PIN, INPUT_PULLUP); // Botão para limpar o display
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    // Inicializa o display OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.display();
    delay(1000);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
}

void loop() {
    // Lê o estado do botão principal
    int buttonState = digitalRead(BUTTON_PIN);
    // Lê o estado do botão de limpar
    int clearButtonState = digitalRead(CLEAR_BUTTON_PIN);

    // Verifica se o botão principal foi pressionado
    if (buttonState == LOW && !buttonPressed) {
        buttonPressed = true;
        buttonPressStartTime = millis(); // Marca o tempo de início da pressão
    } else if (buttonState == HIGH && buttonPressed) {
               buttonPressed = false; // Reseta o estado do botão
        unsigned long pressDuration = millis() - buttonPressStartTime; // Duração da pressão

        // Adiciona ponto ou traço com base na duração da pressão
        if (pressDuration < dotDuration) {
            myWord += "."; // Adiciona ponto
            tone(BUZZER_PIN, 1000, 100); // Som para ponto
        } else if (pressDuration >= dashDuration) {
            myWord += "-"; // Adiciona traço
            tone(BUZZER_PIN, 1000, 250); // Som para traço
        }
        digitalWrite(LED_PIN, HIGH);
        delay(150);
        digitalWrite(LED_PIN, LOW);
        lastPressTime = millis(); // Atualiza o tempo da última pressão
    }

    // Verifica se uma letra de Morse completa foi inserida
    if (!buttonPressed) {
        // Verifica se o tempo desde a última pressão é maior que o debounceDelay
        if (millis() - lastPressTime > debounceDelay && myWord.length() > 0) {
            char letter = getLetterFromMorse(myWord);
            if (letter != '?') {
                decodedText += letter; // Adiciona a letra decodificada ao texto
            }
            // Atualiza o display com a sequência de Morse e o texto decodificado
            updateDisplay();
            myWord = ""; // Reseta a string de Morse
        }
    }

    // Verifica se o botão de limpar foi pressionado
    if (clearButtonState == LOW) {
        delay(1000); // Aguarda um segundo para evitar múltiplas leituras
        display.clearDisplay(); // Limpa o display
        myWord = ""; // Reseta a string de Morse
        decodedText = ""; // Reseta o texto decodificado
        updateDisplay(); // Atualiza o display
    }
}

// Função para atualizar o display com a sequência de Morse e o texto decodificado
void updateDisplay() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 0);
    display.println(myWord); // Exibe a sequência de Morse
    display.setTextSize(2);
    display.setCursor(10, 30); // Ajusta a posição do texto decodificado
    display.print(decodedText); // Exibe o texto decodificado
    display.display();
}

// Função para converter Morse para letra
char getLetterFromMorse(String morseCode) {
    for (int i = 0; i < 36; i++) {
        if (morseCode == morseToLetter[i]) {
            return letters[i]; // Retorna a letra correspondente
        }
    }
    return '?'; // Retorna '?' se não encontrar a letra
}