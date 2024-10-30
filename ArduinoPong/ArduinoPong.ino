/*
 * Pong implementation using a .96in OLED and an Arduino Uno for CSIS2810. 
 * This project is based on the work by @shveytank (https://github.com/shveytank/Arduino_Pong_Game)
 * 
 * Authors: Tommy Collier, Alex Adams, Ty Greenburg, Jacob Gridley
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Screen size
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Input buttons
#define UP_BUTTON 2
#define DOWN_BUTTON 3

// Ball and Paddle (CPU & Player) values
const unsigned long BALL_RATE = 16;
const unsigned long PADDLE_RATE = 33;
const uint8_t PADDLE_HEIGHT = 24;

// Ball position and direction variables
uint8_t ball_x = 64, ball_y = 32;
uint8_t ball_dir_x = 1, ball_dir_y = 1;

// Player position variables
const uint8_t CPU_X = 12, PLAYER_X = 115; // Constants, values don't change
uint8_t cpu_y = 16, player_y = 16;

// Time tracking variables
unsigned long ball_update;
unsigned long paddle_update;

void setup() {
  // Input button setup
  pinMode(UP_BUTTON, INPUT);
  pinMode(DOWN_BUTTON, INPUT);
  digitalWrite(UP_BUTTON,1);
  digitalWrite(DOWN_BUTTON,1);

  // Display setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize display
  display.clearDisplay();  // Clear Adafruit logo

  // Set title screen
  display.setTextColor(WHITE);
  display.setCursor(15, 0); // Move cursor for 1st line
  display.setTextSize(2);
  display.print("CSIS2810");
  display.setCursor(17, 25); // Move cursor for 2nd line
  display.setTextSize(4);
  display.print("PONG");

  // Display title screen briefly
  display.display();
  delay(2000); // 2s
  display.clearDisplay();
  drawCourt();
  
  // Start time
  unsigned long start = millis();
  ball_update = millis();
  paddle_update = ball_update;
}

void loop() {
  unsigned long time = millis(); // Set current time
  bool has_changed = false; // Tracks whether a change has occured in current loop (paddle/ball movement)
  static bool up_state = false; // UP button value
  static bool down_state = false; // DOWN button value
  
  // Set buttons true if currently true OR now true
  up_state |= (digitalRead(UP_BUTTON) == LOW);
  down_state |= (digitalRead(DOWN_BUTTON) == LOW);

  if(time > ball_update) {
      uint8_t new_x = ball_x + ball_dir_x;
      uint8_t new_y = ball_y + ball_dir_y;

      // Check if we hit the vertical walls
      if(new_x == 0 || new_x == 127) {
          ball_dir_x = -ball_dir_x;
          new_x += ball_dir_x + ball_dir_x;
      }

      // Check if we hit the horizontal walls.
      if(new_y == 0 || new_y == 63) {
          ball_dir_y = -ball_dir_y;
          new_y += ball_dir_y + ball_dir_y;
      }

      // Check if we hit the CPU paddle
      if(new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + PADDLE_HEIGHT) {
          ball_dir_x = -ball_dir_x;
          new_x += ball_dir_x + ball_dir_x;
      }

      // Check if we hit the player paddle
      if(new_x == PLAYER_X
          && new_y >= player_y
          && new_y <= player_y + PADDLE_HEIGHT)
      {
          ball_dir_x = -ball_dir_x;
          new_x += ball_dir_x + ball_dir_x;
      }

      display.drawPixel(ball_x, ball_y, BLACK);
      display.drawPixel(new_x, new_y, WHITE);
      ball_x = new_x;
      ball_y = new_y;

      ball_update += BALL_RATE;

      has_changed = true;
  }

  if(time > paddle_update) {
      paddle_update += PADDLE_RATE;

      // CPU paddle
      display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT, BLACK);
      const uint8_t half_paddle = PADDLE_HEIGHT >> 1;
      if(cpu_y + half_paddle > ball_y) {
          cpu_y -= 1;
      }
      if(cpu_y + half_paddle < ball_y) {
          cpu_y += 1;
      }
      if(cpu_y < 1) cpu_y = 1;
      if(cpu_y + PADDLE_HEIGHT > 63) cpu_y = 63 - PADDLE_HEIGHT;
      display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT, WHITE);

      // Player paddle
      display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK);
      if(up_state) {
          player_y -= 1;
      }
      if(down_state) {
          player_y += 1;
      }
      up_state = down_state = false;
      if(player_y < 1) player_y = 1;
      if(player_y + PADDLE_HEIGHT > 63) player_y = 63 - PADDLE_HEIGHT;
      display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, WHITE);

      has_changed = true;
  }

  if(has_changed)
      display.display();
}

void drawCourt() {
    display.drawRect(0, 0, 128, 64, WHITE);
}
