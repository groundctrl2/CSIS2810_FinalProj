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
const unsigned long ball_rate = 16;
const unsigned long paddle_rate = 33;
const uint8_t paddle_height = 16;
uint8_t half_paddle;

// Ball position and direction variables
uint8_t ball_x = 64, ball_y = 32;
uint8_t ball_dir_x = 1, ball_dir_y = 1;

// Player position variables
const uint8_t CPU_X = 12, PLAYER_X = 115;
uint8_t cpu_y = 16, player_y = 16;

// Time tracking variables
unsigned long ball_update;
unsigned long paddle_update;

// Score variables
uint16_t score = 0; // unsigned integer, max score of 65,535

void setup() {
  Serial.begin(9600); // Serial communication for debugging

  // Input button setup
  pinMode(UP_BUTTON, INPUT);
  pinMode(DOWN_BUTTON, INPUT);
  digitalWrite(UP_BUTTON,1);
  digitalWrite(DOWN_BUTTON,1);

  // Display setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize display
  display.clearDisplay();  // Clear Adafruit logo
  display.setTextColor(WHITE);

  // Set title screen
  display.setCursor(17, 25); // Move cursor for 2nd line
  display.setTextSize(4);
  display.print("PONG");
  display.setCursor(15, 0); // Move cursor for 1st line (done 2nd for score banner use)
  display.setTextSize(2);
  display.print("CSIS2810");

  // Display title screen briefly
  display.display();
  delay(2000); // 2s
  display.clearDisplay();

  // Display court and score banner (score value in loop)
  drawCourt();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Score:");
  
  half_paddle = paddle_height >> 1; // Bitwise shift right the paddle value to get half
  
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

  // Erase the current score banner value
  display.setCursor(38, 0);
  display.setTextColor(BLACK);
  display.print(score);
  
  // Set buttons true if currently true OR now true
  up_state |= (digitalRead(UP_BUTTON) == LOW);
  down_state |= (digitalRead(DOWN_BUTTON) == LOW);

  // Update ball
  if(time > ball_update) {
      ball_update += ball_rate; // Update ball check time
      uint8_t new_x = ball_x + ball_dir_x; // Add ball direction to new x value
      uint8_t new_y = ball_y + ball_dir_y; // Add ball direction to new y value

      // If the ball hits a vertical wall
      if(new_x == 0 || new_x == 127) {
          ball_dir_x = -ball_dir_x; // Invert the x direction
          new_x += 2 * ball_dir_x; // Send ball in the opposite direction
      }

      // If the ball hits a horizontal wall
      if(new_y == 17 || new_y == 63) {
          ball_dir_y = -ball_dir_y; // Invert the y direction
          new_y += 2 * ball_dir_y; // Send ball in the opposite direction
      }

      // If the ball hits the CPU's paddle
      if(new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + paddle_height) {
          ball_dir_x = -ball_dir_x; // Invert the x direction
          new_x += ball_dir_x + ball_dir_x; // Send ball in the opposite direction
      }

      // If the ball hits the player's paddle
      if(new_x == PLAYER_X && new_y >= player_y && new_y <= player_y + paddle_height) {
          ball_dir_x = -ball_dir_x; // Invert the x direction
          new_x += ball_dir_x + ball_dir_x; // Send ball in the opposite direction
          score++; // Add 1 to score
      }

      score = (new_x > PLAYER_X) ? 0 : score; // Reset score if ball missed
      display.drawPixel(ball_x, ball_y, BLACK); // Erase ball from previous position
      display.drawPixel(new_x, new_y, WHITE); // Draw ball in new position
      ball_x = new_x; // Update ball x value
      ball_y = new_y; // Update ball y value
      
      has_changed = true; // Record that change has happened
  }

  // Update paddles
  if(time > paddle_update) {
      paddle_update += paddle_rate;  // Update paddle check time

      // Update CPU paddle
      display.drawFastVLine(CPU_X, cpu_y, paddle_height, BLACK); // Erase paddle from previous position
      cpu_y -= (cpu_y + half_paddle > ball_y) ? 1 : 0; // If needed, subtract 1 from CPU y value to match ball's y value
      cpu_y += (cpu_y + half_paddle < ball_y) ? 1 : 0; // If needed, add 1 to CPU y value to match ball's y value
      if(cpu_y < 17) cpu_y = 17; // Ensure CPU's y value doesn't exceed top boundary
      if(cpu_y + paddle_height > 63) cpu_y = 63 - paddle_height; // Ensure CPU's y value doesn't exceed bottom boundary
      display.drawFastVLine(CPU_X, cpu_y, paddle_height, WHITE); // Draw CPU's paddle in new position

      // Update player paddle
      display.drawFastVLine(PLAYER_X, player_y, paddle_height, BLACK); // Erase paddle from previous position
      player_y -= (up_state) ? 1 : 0; // If up button pushed, subtract 1 from player's y value
      player_y += (down_state) ? 1 : 0; // If down button pushed, add 1 to player's y value
      up_state = down_state = false; // Set both button values back to false
      if(player_y < 17) player_y = 17; // Ensure player's y value doesn't exceed top boundary
      if(player_y + paddle_height > 63) player_y = 63 - paddle_height; // Ensure player's y value doesn't exceed bottom boundary
      display.drawFastVLine(PLAYER_X, player_y, paddle_height, WHITE); // Draw player's paddle in new position

      has_changed = true; // Record that change has happened
  }

  // Erase the current score banner value
  display.setCursor(38, 0);
  display.setTextColor(WHITE);
  display.print(score);

  // Update screen with changes
  if(has_changed)
      display.display();
}

// Draw the court's border
void drawCourt() {
    display.drawRect(0, 16, 128, 48, WHITE);
}
