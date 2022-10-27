#include <U8g2lib.h>
#include <SPI.h>

#define VRX A0
#define VRY A1
#define SW A2
#define GAME_STATE_PREPARE 0
#define GAME_STATE_GAME 2
#define GAME_STATE_END 3
#define buttonOne 4

boolean game = false;
boolean dead = false;
boolean debug = false;
boolean spawned = false;
boolean rocketSpawned = false;

int xVal;
int yVal;
int swVal;
int Icheck = 0;
int rocket_cooldown;

char line[32];


uint8_t game_state = GAME_STATE_PREPARE;

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);




/*================================================================
  Player Data
  ================================================================*/
class Player {
  public:
    String username;
    int x;
    int y;
    int score;
    int lives;
};

Player player;
/*================================================================
  Game Data
  ================================================================*/
boolean objectSpawned;

class Asteroid {
  public:
    int x;
    int y;
    int worth;
    int radius;
    int fast;
};

class Bonus {
  public:
    int x;
    int y;
    int bonus;
};

class Rocket {
  public:
    int x;
    int y;
    boolean drawn;
};

Rocket rockets[5] = {Rocket(), Rocket(), Rocket(), Rocket(), Rocket()};
Asteroid asteroids[5] = {Asteroid(), Asteroid(), Asteroid(), Asteroid()};

Bonus healthBonus;
/*================================================================
  Setup
  ================================================================*/

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  pinMode(VRX, INPUT);
  pinMode(VRY, INPUT);
  pinMode(SW, INPUT);
  pinMode(buttonOne, INPUT);

  objectSpawned = true;
  player.x = 5;
  player.y = 30;
  player.lives = 5;

  for (int i = 0; i < 5; i++) {
    rockets[i].drawn = false;
  }

  if (debug) {
    player.score = 9000;
  }

  delay(1000);
}

/*================================================================
  Loop
  ================================================================*/

void loop() {
  // put your main code here, to run repeatedly:
  int xVal = analogRead(VRX);
  int yVal = analogRead(VRY);
  int swVal = analogRead(SW);

  if (swVal <= 10) {
    game = true;
  }
  if (game) {
    game_frame(xVal, yVal);
    if (digitalRead(buttonOne) == 1) {
      useRocket();
    }
    while (objectSpawned) {
      generateObjects();
    }
    updateObjects();
    checkCollisions();

  } else {
    if (!dead) {
      intro_frame();
    } else {
      death_frame();
      player.lives = 5;
      player.score = 0;
      objectSpawned = true;
    }
  }
}

/*================================================================
  Introduction Frame
  ================================================================*/

void intro_frame() {
  u8g2.clearBuffer();
  //
  dText(0, 15, "Space Sweepers");
  dText(0, 30, "Press      Down");
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.drawGlyph(50, 30, 0x2126);
  dText(0, 40, "To Begin");
  //
  u8g2.sendBuffer();
}

/*================================================================
  Game Frame
  ================================================================*/

void game_frame(int xVal, int yVal) {
  u8g2.clearBuffer();
  //Up
  if (xVal > 700 && xVal < 1000 && yVal > 500 && yVal < 550) {
    if (player.y != 9) {
      player.y -= 1;
    }
  }
  //Down
  else if (xVal >= 0 && xVal < 150 && yVal > 300 && yVal < 600) {
    if (player.y != 54) {
      player.y += 1;
    }
  }
  //Right
  else if (xVal >= 500 && xVal < 550 && yVal > 700 && yVal < 1050) {
    if (player.x != 115) {
      player.x += 1;
    }
  }
  //Left
  else if (xVal > 150 && xVal < 1050 && yVal >= 0 && yVal < 400) {
    if (player.x != 0) {
      player.x -= 1;
    }
  }

  //====================================================================
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.drawGlyph(player.x, player.y, 0x23e9);
  //Draw Objects
  dText(0, 62, "Score:");
  dText(50, 62, toChar(player.score));
  dText(110, 62, toChar(player.lives));
  u8g2.drawLine(0, 54, 128, 54);
  if (!spawned) {
    if (random(0, 50) == 30) {
      healthBonus.x = random(30, 110);
      healthBonus.bonus = random(1, 2);
      healthBonus.y = random(1, 54);
      u8g2.setFont(u8g2_font_unifont_t_symbols);
      u8g2.drawGlyph(healthBonus.x, healthBonus.y, 0x25d0);
      spawned = true;
    }
  }
  for (int i = 0 ; i < 5; i++) {
    dCircle(asteroids[i].x, asteroids[i].y, asteroids[i].radius, 1);
    if (rockets[i].drawn) {
      dSquare(rockets[i].x, rockets[i].y, 2, 2, 1);
    }
  }
  //=========================================================================
  u8g2.sendBuffer();
}

void death_frame() {
  u8g2.clearBuffer();
  //
  dText(0, 15, "You Died");
  dText(0, 30, toChar(player.score));
  //
  u8g2.sendBuffer();
  delay(10000);
  dead = false;
  intro_frame();
}

char* toChar(int N)
{
  sprintf(line, "%d", N);
  return line;
}

/*================================================================
  Draw Functions
  ================================================================*/

void dText(int x, int y, char* text) {
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.drawStr(x, y, text);
}

void dCircle(int x, int y, int r, int s) {
  u8g2.setDrawColor(s);
  u8g2.drawCircle(x, y, r);
}
void dSquare(int x, int y, int w, int h, int s) {
  u8g2.setDrawColor(s);
  u8g2.drawBox(x, y, w, h);
}

//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////

void checkCollisions() {
  for (int i = 0; i < 5; i++) {
    if (abs(player.x - asteroids[i].x) < asteroids[i].radius && abs(player.y - asteroids[i].y) < asteroids[i].radius) {
      player.lives -= 1;
      asteroids[i].x = 128;
      asteroids[i].y = random(1, 54);
      asteroids[i].worth = random(1, 100);
      asteroids[i].radius = random(4, 8);
    }
    if (abs(rockets[i].x - asteroids[i].x) < asteroids[i].radius && abs(rockets[i].y - asteroids[i].y) < asteroids[i].radius) {
      asteroids[i].x = 128;
    }
    if (rockets[i].x = 130) {
      rockets[i].drawn = false;
    }
  }

  if (abs(player.x - healthBonus.x) < 10 && abs(player.y - healthBonus.y) < 10) {
    player.lives += healthBonus.bonus;
    spawned = false;
  }

  if (player.lives == 0) {
    dead = true;
    game = false;
    death_frame;
  }
}

void updateObjects() {
  if (player.score > 1500 && player.score < 2000) {
    asteroid.fast = random(1, 2);
    asteroid1.fast = random(1, 2);
    asteroid1.x -= asteroid1.fast;
  } else if (player.score > 2000 && player.score < 5000) {
    asteroid.fast = random(1, 3);
    asteroid1.fast = random(1, 3);
    asteroid1.x -= asteroid1.fast;
  } else if (player.score > 5000 && player.score < 10000) {
    asteroid.fast = random(1, 3);
    asteroid1.fast = random(1, 3);
    asteroid1.x -= asteroid1.fast;
    asteroid2.x -= asteroid2.fast;
  } else if (player.score > 10000) {
    finalLevel();
  }
  for (int i = 0; i < 5 ; i++) {
    if (rockets[i].drawn) {
      rockets[i].x += 1;
    }
  }
  asteroid.x -= asteroid.fast;


  for (int i = 0; i < 5; i ++) {
    if (asteroids[i].x <= - 1) {
      player.score += asteroids[]i.worth;
      asteroids[i].x = 128;
      asteroids[i].y = random(1, 54);
      asteroids[i].worth = random(1, 100);
      asteroids[i].radius = random(4, 8);
    }
  }
}

void generateObjects() {
  for (int i = 0 ; i < 5; i++) {
    asteroids[i].x = 128;
    asteroids[i].y = random(1, 54);
    asteroids[i].radius = random(4, 8);
    asteroids[i].worth = random(10, 100);
    asteroids[i].fast = 1;
  }

  objectSpawned = false;
}

void useRocket() {
  if (rocket_cooldown  == 0) {
    Icheck += 1;
    if (rockets[Icheck].drawn == false) {
      if (rocket_cooldown == 0) {
        rockets[Icheck].x = player.x;
        rockets[Icheck].y = player.y;
        rockets[Icheck].drawn = true;
      }
    }
    if (Icheck == 4) {
      Icheck = 0;
    }
  }
}

void finalLevel() {


}
