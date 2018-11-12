
#include <MsTimer2.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    7
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#define SCORE_Y 0
#define GAME_Y 40
#define LEVEL_Y 40
#define CHAR_W 14
#define CHAR_UFO "n"
#define CHAR_EMPTY " "

#define EMPTY -2
#define UFO -1
#define LED_EMPTY 0B00000000
#define LED_WALL 0B01001001
#define LED_UFO 0B00010101
#define PUSH_SHORT 10
#define BUTTON_START 4
#define BUTTON_SELECT 3
#define BUTTON_FIRE 2
#define BEEP_PORT 5

#define FREQ_START 1319 //E6
#define FREQ_ADVANCE0 831 //GS5
#define FREQ_ADVANCE1 880 //A5
#define FREQ_SELECT 1047 //C6
#define FREQ_HIT1 587    //D5
#define FREQ_HIT2 659    //E5
#define FREQ_MISS 104    //GS2
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

int advance_step;
int advance_speed;
int soundtest;

int selection;
int fire;
bool nowGame;
int enemy[6];
int highscore;
int myscore;
int level;

void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(0));
  
  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_FIRE, INPUT_PULLUP);
  pinMode(BEEP_PORT, OUTPUT);

  init_oled();
  disp_title();
  
  nowGame = false;
  highscore = 0;
  level = 1;
  
  soundtest = 1;
  advance_speed = SPEED1;
  advance_step = 0;
  Serial.println("setup complete");
}

void init_oled() {
  Serial.println("init_oled");
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void disp_title() {
  display.clearDisplay();
  display.display();
  display.setTextSize(3);
  display.setCursor( 0, 0);display.println("Digital");
  display.display();
  delay(1000);
  display.setCursor( 0,26);display.println("Invador");
  display.display();
  delay(1000);
  display.setTextSize(1);
  display.setCursor(16, 52);display.print("push start button");
  display.display();
  display.setTextSize(2);
  //delay(3000);
  //display.clearDisplay();
  //display.display();

  //disp_highscore();
  //disp_myscore();
  //disp_game();
}

void disp_refresh() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor( 0, 20);//display.println('temp);
  display.setCursor( 0, 50);//display.println(meter);
  display.display();
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
  while (!digitalRead(pin)){
    gauge++;
  }
  if (gauge > PUSH_SHORT){
    return true;
  }
  else return false;
}

void game_start() {
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Game Start");
  display.display();
  
  selection = 0;
  fire = 0;
  for(int i = 0; i < 6; i++){
    enemy[i] = EMPTY;
  }
  myscore = 0;
  level = 1;
  
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
  
  display.clearDisplay();
  display.display();
  nowGame = true;
}

void game_over() {
  MsTimer2::stop();
  nowGame = false;

  for(int i = 0; i < 4; i++){
    display.clearDisplay();
    display.display();
    tone(BEEP_PORT, FREQ_OVER1, 400);
    delay(400);
    
    disp_game();
    display.display();
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
    tone(BEEP_PORT, NOTE_F3, 300);
    delay(330);
    tone(BEEP_PORT, NOTE_F3, 190);
    delay(220);
    tone(BEEP_PORT, NOTE_F3, 80);
    delay(110);
    tone(BEEP_PORT, NOTE_F3, 300);
    delay(330);
    tone(BEEP_PORT, NOTE_GS3, 190);
    delay(220);
    tone(BEEP_PORT, NOTE_G3, 80);
    delay(110);
    tone(BEEP_PORT, NOTE_G3, 190);
    delay(220);
    tone(BEEP_PORT, NOTE_F3, 80);
    delay(110);
    tone(BEEP_PORT, NOTE_F3, 190);
    delay(220);
    tone(BEEP_PORT, NOTE_E3, 80);
    delay(110);
    tone(BEEP_PORT, NOTE_F3, 600);
    delay(660);
  }
  disp_highscore();
  disp_myscore();
  disp_game();
  delay(3000);
  
  for(int i = 0; i < 6; i++){
    enemy[i] = EMPTY;
  }
  selection = 0;
  disp_title();
}

void hantei() {
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
    level = 2;
  }else if((myscore >= LEVEL3)&&(myscore < LEVEL4)){
    advance_speed = SPEED3;
    level = 3;
  }else if((myscore >= LEVEL4)&&(myscore < LEVEL5)){
    advance_speed = SPEED4;
    level = 4;
  }else if(myscore >= LEVEL5){
    advance_speed = SPEED5;
    level = 5;
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
  String score = "    ";
  score.concat(myscore);
  String dscore = score.substring(score.length() - 4);
  display.setTextSize(2);
  display.setCursor(80, SCORE_Y);display.print(dscore);
  //display.display();
}

void disp_highscore() {
  String score = "    ";
  score.concat(highscore);
  String dscore = score.substring(score.length() - 4);
  display.setTextSize(2);
  display.setCursor(20, SCORE_Y);display.print(dscore);
  //display.display();
}

void disp_game() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  int i;
  //自分の照準
  display.setCursor(CHAR_W, GAME_Y);
  if(selection >= 0 && selection <= 9){
    display.print(selection);
  }else if(selection == UFO){
    display.print(CHAR_UFO);
  }
  //壁
  //display.setCursor(CHAR_W*2, GAME_Y);
  //display.print("E");
  display.drawLine(CHAR_W*2, GAME_Y,    CHAR_W*3-2, GAME_Y, WHITE);
  display.drawLine(CHAR_W*2, GAME_Y+6,  CHAR_W*3-2, GAME_Y+6, WHITE);
  display.drawLine(CHAR_W*2, GAME_Y+12, CHAR_W*3-2, GAME_Y+12, WHITE);
  
  //敵
  for(i = 0; i < 6; i++){
    int keta = i;
    display.setCursor(CHAR_W*keta+20+CHAR_W, GAME_Y);
    display.print(CHAR_EMPTY);
    if(enemy[i] == UFO){
      display.print(CHAR_UFO);
    }else if(enemy[i] == EMPTY){
      display.print(CHAR_EMPTY);
    }else{
      display.print(enemy[i]);
    }
  }

  //display.setCursor(80, LEVEL_Y);
  //display.print(level);
  
  disp_highscore();
  disp_myscore();
  display.display();
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
    //disp_highscore();
    //disp_myscore();
    //disp_game();
    
    if(pushed(BUTTON_SELECT) || pushed(BUTTON_FIRE) || pushed(BUTTON_START)){
      Serial.println("button pushed then game start");
      game_start();
    }
  }
}
