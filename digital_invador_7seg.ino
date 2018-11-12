
//We always have to include the library
#include "LedControl.h"
#include <MsTimer2.h>

#define EMPTY -2
#define UFO -1
#define LED_EMPTY 0B00000000
#define LED_WALL 0B01001001
#define LED_UFO 0B00010101
#define PUSH_SHORT 200
#define BUTTON_START 4
#define BUTTON_SELECT 3
#define BUTTON_FIRE 2
#define VOL_INTENSITY A0
#define BEEP_PORT 13

#define FREQ_START 1319 //E6
#define FREQ_ADVANCE0 831 //GS5
#define FREQ_ADVANCE1 880 //A5
#define FREQ_SELECT 1047 //C6
#define FREQ_HIT1 587    //D5
#define FREQ_HIT2 659    //E5
#define FREQ_MISS 208    //GS3
#define FREQ_OVER1 349 //F4
#define FREQ_OVER2 330 //E4
#define FREQ_HIGH1 1319 //E6
#define FREQ_HIGH2 1397 //F6
#define FREQ_END1 587 //D5
#define FREQ_END2 523 //C5
#define FREQ_END3 494 //B4

#define SPEED1 400
#define SPEED2 300
#define SPEED3 200
#define SPEED4 150
#define SPEED5 80

#define LEVEL2 50 //20
#define LEVEL3 100 //40
#define LEVEL4 150 //60
#define LEVEL5 200 //80

#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262     // ド
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
int advance_step;
int advance_speed;
int soundtest;

int selection;
int fire;
int intensity;
bool nowGame;
int enemy[6];
int highscore;
int myscore;
/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc1=LedControl(12,11,10,1);//(6,5,4,1);//(12,11,10,1);
LedControl lc2=LedControl(9,8,7,1);

void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(5));
  
  intensity = 6;
  init_led(1);
  
  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_FIRE, INPUT_PULLUP);
  pinMode(BEEP_PORT, OUTPUT);

  nowGame = false;
  highscore = 0;

  soundtest = 1;
  advance_speed = SPEED1;
  advance_step = 0;
}

void init_led(int clear_display) {
    /*
     The MAX72XX is in power-saving mode on startup,
     we have to do a wakeup call
     */
    lc1.shutdown(0,false);
    lc2.shutdown(0,false);
    /* Set the brightness to a medium values */
    lc1.setIntensity(0,intensity);
    lc2.setIntensity(0,intensity);
    /* and clear the display */
    if(clear_display){
      lc1.clearDisplay(0);
      lc2.clearDisplay(0);
    }
}

void advance() {
  interrupts();

  if(advance_step == 0){
    tone(BEEP_PORT, FREQ_ADVANCE0, 50);
    advance_step = 1;
    return;
  }else{
    tone(BEEP_PORT, FREQ_ADVANCE1, 50);
    advance_step = 0;
  }
  
  if(enemy[0] != EMPTY){
    game_over();
    return;
  }
  enemy[0] = enemy[1];
  enemy[1] = enemy[2];
  enemy[2] = enemy[3];
  enemy[3] = enemy[4];
  enemy[4] = enemy[5];
  enemy[5] = random(UFO, 9);
}

boolean pushed(int pin) {
  long gauge = 0;
  while (!digitalRead(pin)) gauge++;
  if (gauge > PUSH_SHORT){
    Serial.print("push pin="); Serial.println(pin);
    return true;
  }
  else return false;
}

void game_start() {
  init_led(1);

  selection = 0;
  fire = 0;
  for(int i = 0; i < 6; i++){
    enemy[i] = EMPTY;
  }
  myscore = 0;

  //スコア表示
  disp_highscore();
  disp_myscore();

  tone(BEEP_PORT, FREQ_START, 100);
  delay(110);
  tone(BEEP_PORT, FREQ_START, 150);
  delay(300);
  tone(BEEP_PORT, FREQ_START, 100);
  delay(180);
  tone(BEEP_PORT, FREQ_START, 1000);
  delay(1000);
  
  advance_speed = SPEED1;
  MsTimer2::set(advance_speed, advance);
  MsTimer2::start();
  
  nowGame = true;
}

void game_over() {
  MsTimer2::stop();
  nowGame = false;

  for(int i = 0; i < 4; i++){
    disp_game_clear();
    tone(BEEP_PORT, FREQ_OVER1, 400);
    delay(400);
    disp_game();
    tone(BEEP_PORT, FREQ_OVER2, 400);
    delay(400);
  }

  delay(1000);
  if(myscore > highscore){
    tone(BEEP_PORT, FREQ_HIGH1, 200);
    delay(200);
    tone(BEEP_PORT, FREQ_HIGH2, 200);
    delay(200);
    tone(BEEP_PORT, FREQ_HIGH1, 200);
    delay(200);
    tone(BEEP_PORT, FREQ_HIGH2, 200);
    delay(200);
    tone(BEEP_PORT, FREQ_HIGH1, 200);
    delay(200);
    tone(BEEP_PORT, FREQ_HIGH2, 800);
    delay(800);
    highscore = myscore;
  }else{
    tone(BEEP_PORT, NOTE_F4, 300);
    delay(330);
    tone(BEEP_PORT, NOTE_F4, 190);
    delay(220);
    tone(BEEP_PORT, NOTE_F4, 80);
    delay(110);
    tone(BEEP_PORT, NOTE_F4, 300);
    delay(330);
    tone(BEEP_PORT, NOTE_GS4, 190);
    delay(220);
    tone(BEEP_PORT, NOTE_G4, 80);
    delay(110);
    tone(BEEP_PORT, NOTE_G4, 190);
    delay(220);
    tone(BEEP_PORT, NOTE_F4, 80);
    delay(110);
    tone(BEEP_PORT, NOTE_F4, 190);
    delay(220);
    tone(BEEP_PORT, NOTE_E4, 80);
    delay(110);
    tone(BEEP_PORT, NOTE_F4, 600);
    delay(660);
  }
  for(int i = 0; i < 6; i++){
    enemy[i] = EMPTY;
  }
  selection = 0;
  disp_game();
}

void get_intensity() {
  intensity = analogRead(VOL_INTENSITY) / (1024/8);
  lc1.setIntensity(0, intensity);
  lc2.setIntensity(0, intensity);
}

void hantei() {
  init_led(0);
  //スコア表示
  disp_highscore();
  disp_myscore();

  int hittype = 0;
  bool hit = false;
  for(int i = 0; i < 6; i++){
    if(enemy[i] == selection){
      if(enemy[i] == -1){
        myscore+=10;
        hittype = 2;
      }else{
        myscore+=(i+1);
        hittype = 1;
      }
      for(int j = i; j > 0; j--){
        enemy[j] = enemy[j - 1];
      }
      enemy[0] = EMPTY;
      hit = true;
      break;
    }
  }

  int old_speed = advance_speed;
  if((myscore >= LEVEL2)&&(myscore < LEVEL3)){
    advance_speed = SPEED2;
  }else if((myscore >= LEVEL3)&&(myscore < LEVEL4)){
    advance_speed = SPEED3;
  }else if((myscore >= LEVEL4)&&(myscore < LEVEL5)){
    advance_speed = SPEED4;
  }else if(myscore >= LEVEL5){
    advance_speed = SPEED5;
  }
  if(old_speed != advance_speed){
    MsTimer2::stop();
    MsTimer2::set(advance_speed, advance);
    MsTimer2::start();
  }

  if(hittype == 0){
    tone(BEEP_PORT, FREQ_MISS, 100);
  }else if(hittype == 1){
    tone(BEEP_PORT, FREQ_HIT1, 100);
  }else{
    tone(BEEP_PORT, FREQ_HIT1, 50);
    delay(50);
    tone(BEEP_PORT, FREQ_HIT2, 100);
  }
}

void disp_myscore() {
  int dvalue = myscore;
  if(dvalue > 9999) dvalue = 9999;
  
  lc2.setDigit(0, 0, (dvalue % 10), false);
  if(dvalue >= 10){
    lc2.setDigit(0, 1, (dvalue / 10) % 10, false);
  }else{
    lc2.setRow(0, 1, LED_EMPTY);
  }
  if(dvalue >= 100){
    lc2.setDigit(0, 2, (dvalue / 100) % 10, false);
  }else{
    lc2.setRow(0, 2, LED_EMPTY);
  }
  if(dvalue >= 1000){
    lc2.setDigit(0, 3, (dvalue / 1000) % 10, false);
  }else{
    lc2.setRow(0, 3, LED_EMPTY);
  }
}

void disp_highscore() {
  int dvalue = highscore;
  if(dvalue > 9999) dvalue = 9999;
  
  lc2.setDigit(0, 4, (dvalue % 10), false);
  if(dvalue >= 10){
    lc2.setDigit(0, 5, (dvalue / 10) % 10, false);
  }else{
    lc2.setRow(0, 5, LED_EMPTY);
  }
  if(dvalue >= 100){
    lc2.setDigit(0, 6, (dvalue / 100) % 10, false);
  }else{
    lc2.setRow(0, 6, LED_EMPTY);
  }
  if(dvalue >= 1000){
    lc2.setDigit(0, 7, (dvalue / 1000) % 10, false);
  }else{
    lc2.setRow(0, 7, LED_EMPTY);
  }
}

void disp_game() {
  int i;
  //自分の照準
  if(selection >= 0 && selection <= 9){
    lc1.setDigit(0, 7, selection, false);
  }else if(selection == UFO){
    lc1.setRow(0, 7, LED_UFO);
  }
  //壁
  lc1.setRow(0, 6, LED_WALL);
  //敵
  for(i = 0; i < 6; i++){
    int keta = 5-i;
    if(enemy[i] == UFO){
      lc1.setRow(0, keta, LED_UFO);
    }else if(enemy[i] == EMPTY){
      lc1.setRow(0, keta, LED_EMPTY);
    }else{
      lc1.setDigit(0, keta, enemy[i], false);
    }
  }

  disp_highscore();
  disp_myscore();
}

void disp_game_clear() {
  for(int i = 0; i < 8; i++){
    lc1.setRow(0, i, 0);
  }
}

void disp_score_clear() {
  for(int i = 0; i < 8; i++){
    lc2.setRow(0, i, 0);
  }
}

void loop() {

  if(nowGame){
    if(pushed(BUTTON_SELECT)){
      tone(BEEP_PORT, FREQ_SELECT, 100);
      selection++;
      if(selection > 9){
        selection = UFO;
      }
    }

    if(pushed(BUTTON_FIRE)){
      hantei();
    }
    disp_game();
  }else{
    //スコア表示
    disp_highscore();
    disp_myscore();
    disp_game();
    
    if(pushed(BUTTON_SELECT) || pushed(BUTTON_FIRE) || pushed(BUTTON_START)){
      game_start();
    }
  }

  get_intensity();
  lc1.setTestmode(0, false);
  lc2.setTestmode(0, false);
}
