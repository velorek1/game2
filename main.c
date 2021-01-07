/* *************************** Game2    ***************************   */
/* Author: 				       		      */
/* Date : 2020				               		      */
/* [+] STRUCTURE: 		|-------------------v		      */
/* [MAIN] -> [NEWGAME] -> [MAIN_LOOP] ->[UPDATEGAME && HANDLE_EVENTS] */  
/*				^	            |	              */
/*                     [DRAWTOSCREEN] <-      [MOVEOBJECTS]           */
/* *****************************************************************  */
/* [Authors of Assets]		      				      */
/* *****************************************************************  */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "list.h" 

//GAME CONSTANTS
#define SCREEN_W 640
#define SCREEN_H 480
#define MAX_KEY 1000
#define SPEED 10
#define MAX_LIVES 3
//MESSAGES
#define STS_MSG0 "SDL Graphics loaded successfully.\n"
#define ERR_MSG0 "SDL Graphics could not be loaded. \n"
#define ERR_MSG1 "Failed to load asset. \n"


typedef struct _sprite{
   SDL_Surface *Img;
} SPRITE;

enum PLAYERSTATE{HALTED, FORWARD, BACKWARD, JUMP, DEAD};
/* GLOBALS */
OBJECT player;
OBJECT *enemies;
//OBJECT *projectiles;
SDL_Surface *background,*playerSprite;
SDL_Event ev;
SDL_Renderer *ren1;
SDL_Window *win1;
long KeyState[MAX_KEY];
BOOL Running=TRUE;
BOOL keypressed=FALSE;
BOOL mouseclicked=FALSE;
BOOL moving = FALSE;
time_t t;
//Mix_Music *Theme;
//Mix_Chunk *sound,*shot, *expsnd, *shield;
enum PLAYERSTATE PlayerState;
int level=0; int lives=MAX_LIVES;
int movingframe=0;
int scrollframe=0;
long mapX = 0;
int enemySpeed= 0;
double gettime=0; double timetemp=0;
double jump=0;
int oldX, oldY;
BOOL scroll = FALSE;
BOOL jumping = FALSE;
/* ---------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* ---------------------------------------------- */
/* Mathematics & Physics */
BOOL Collision(int AX1, int AY1, int AX2, int AY2, int BX1, int BY1, int BX2, int BY2); 
int randnum(int number);
/*SDL Related */
BOOL  InitVideo();
BOOL  InitAudio();
void  ToggleFullscreen(SDL_Window* Window);
void  CleanMemory();
/* EVENTS */
BOOL Key(long K);
void HandleKey(long Sym, BOOL Down);
void HandleEvents();
BOOL timer1(int *ticks, double *time, int ms);
BOOL lerp(double *value, double *time, int ms);

/* Game engine */

void  LoadAssets();
void  NewGame();
void  UpdateGame();
void  Main_Loop();
void  movePlayerXY(int speed, int direction);
void  Player_Behaviour();
void  Enemy_Behaviour();
/* Drawing */
void Draw(int X, int Y, SDL_Surface *Img);
void DrawObject(OBJECT Object);
void DrawAnimation(int X, int Y, int startY, int H, int W, int frame, SDL_Surface *Img);
void DrawDynamicObject(OBJECT *Object);
void DrawScroll(int X, int Y, int W, int H, int startX, int frame, SDL_Surface *Img);
void DrawText(char* string,int size, int x, int y,int fR, int fG, int fB,int bR, int bG, int bB, BOOL transparent);
void DrawEnemy(int x, int y, int w, int h);
void DrawScreen();
 
/* ---------------------------------------------- */
/* MAIN CODE */ 
/* ---------------------------------------------- */
int main(int argc, char ** argv){
  InitVideo();
  InitAudio();
  TTF_Init();
  LoadAssets();
  NewGame();
  Main_Loop(); //UPDATE EVENTS AND DRAW TO SCREEN
  CleanMemory();
  return 0;
}

/* FUNCTIONS */
//Mathematics and Physics
BOOL Collision(int AX1, int AY1, int AX2, int AY2, int BX1, int BY1, int BX2, int BY2){
  return (AX1 < BX1 + (BX2-BX1)) && (AX1 + (AX2-AX1) > BX1) && (AY1 < BY1 + (BY2-BY1)) && (AY1 + (AY2-AY1) > BY1);
}

int randnum(int number){
  //srand((unsigned) time(&t));
  return rand() % number;
}

//SDL Initialization
BOOL InitVideo(){
   SDL_Init(SDL_INIT_VIDEO);
   IMG_Init(IMG_INIT_PNG);
#ifdef __linux__ 
   win1 = SDL_CreateWindow(" > Game 2 <", 50,0,SCREEN_W,SCREEN_H,SDL_WINDOW_SHOWN);
#elif _WIN32
   win1 = SDL_CreateWindow(" > Game 2 <", 50,0,SCREEN_W,SCREEN_H,SDL_WINDOW_FULLSCREEN);
#else
#endif
   ren1 = SDL_CreateRenderer(win1, -1, 0);
   SDL_SetWindowBordered(win1,SDL_TRUE);
   return (ren1 != NULL) && (win1 != NULL);
}

BOOL InitAudio(){
  if (SDL_Init(SDL_INIT_AUDIO) < 0){
    return FALSE;
  }
  Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 
  MIX_DEFAULT_CHANNELS, 4096);
  return TRUE;
} 
void ToggleFullscreen(SDL_Window* Window) {
    Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
    BOOL IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;
    SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);
    SDL_ShowCursor(IsFullscreen);
}

void CleanMemory(){
  SDL_DestroyRenderer(ren1);
  SDL_DestroyWindow(win1);
  Mix_Quit();
  IMG_Quit(); 
  SDL_Quit();
} 

//Events Functions

BOOL Key(long K){
  if ((K>= 0) && (K <= MAX_KEY)) 
    return KeyState[K]; 
  else 
    return FALSE;
}

void HandleKey(long Sym, BOOL Down){
  if (Sym == SDLK_UP) Sym = SDL_SCANCODE_UP;
  if (Sym == SDLK_DOWN) Sym = SDL_SCANCODE_DOWN;
  if (Sym == SDLK_LEFT) Sym = SDL_SCANCODE_LEFT;
  if (Sym == SDLK_RIGHT) Sym = SDL_SCANCODE_RIGHT;
  if (Sym == SDLK_SPACE) Sym = SDL_SCANCODE_SPACE;
   if ((Sym >= 0) && (Sym <= MAX_KEY)) {
    KeyState[Sym] = Down;
    if (Sym == SDLK_ESCAPE) Running = FALSE;    
  }
} 

void HandleEvents(){
  SDL_Event e;
  if (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      Running = FALSE;
    }
    
    if (e.type == SDL_KEYDOWN){
      keypressed = TRUE;
      HandleKey(e.key.keysym.sym, TRUE);
    }
    if (e.type == SDL_MOUSEBUTTONDOWN){
      mouseclicked = TRUE;
    }
    if (e.type == SDL_MOUSEBUTTONUP){
      mouseclicked = FALSE;
    }
    if (e.type == SDL_KEYUP){
      keypressed = FALSE;
      HandleKey(e.key.keysym.sym, FALSE);
    }  
  }
}

// Game Engine

void LoadAssets(){
  /* Images */
  background = SDL_LoadBMP("data/pics/fondo2.bmp");
  if (background == NULL) {fprintf(stderr, ERR_MSG1); exit(0);} 

  playerSprite = IMG_Load("data/pics/psprite2.png");
  if (playerSprite == NULL) {fprintf(stderr, ERR_MSG1); printf("here\n"); exit(0);} 


 /* Music and Sounds */
  /* 
  Theme = Mix_LoadMUS("data/snd/theme.ogg");
   if (Theme == NULL)  {fprintf(stderr, ERR_MSG1); exit(0);} 
   sound = Mix_LoadWAV("data/snd/rockets.wav");
   if (sound == NULL) {fprintf(stderr, ERR_MSG1); exit(0);} 
   Mix_VolumeChunk( sound, MIX_MAX_VOLUME );

   shot = Mix_LoadWAV("data/snd/shot.wav");
   if (shot == NULL) {fprintf(stderr, ERR_MSG1); exit(0);}    
   Mix_VolumeChunk(shot, MIX_MAX_VOLUME ); 


   expsnd = Mix_LoadWAV("data/snd/explosion.wav");
   if (expsnd == NULL) {fprintf(stderr, ERR_MSG1); exit(0);}    
   Mix_VolumeChunk(expsnd, MIX_MAX_VOLUME ); 

   shield = Mix_LoadWAV("data/snd/shield.wav");
   if (shield == NULL) {fprintf(stderr, ERR_MSG1); exit(0);}    
   Mix_VolumeChunk(shield, MIX_MAX_VOLUME ); 
*/
}

void NewGame(){
    //addEnemy(tX,tY,tDIRX, tDIRY, tSIZE);
   moving = FALSE;
   jumping = FALSE;
   PlayerState = FORWARD;
   scrollframe = 0;
   player.X = 10;
   player.Y = 360;
   /* Music */
    //Mix_PlayMusic(Theme, -1); 
}

// Drawing Functions 
void Draw(int X, int Y, SDL_Surface *Img) {
  SDL_Rect R;
  SDL_Texture *text;
  
  R.x = X;
  R.y = Y;
  R.w = Img->w;
  R.h = Img->h;
  text = SDL_CreateTextureFromSurface(ren1,Img);
  SDL_RenderCopy(ren1, text, NULL, &R);
  SDL_DestroyTexture(text);
}

void DrawAnimation(int X, int Y, int startY, int W, int H, int frame, SDL_Surface *Img) {
  SDL_Rect R, D;
  SDL_Texture *text;
  
  R.x = X;
  R.y = Y;
  R.w = W;
  R.h = H;
  D.x = W*frame;
  D.y = startY;
  D.w = W;
  D.h = H;
  text = SDL_CreateTextureFromSurface(ren1,Img);
  SDL_RenderCopy(ren1, text, &D, &R);
  SDL_DestroyTexture(text);
}

void DrawScroll(int X, int Y, int W, int H, int startX, int frame, SDL_Surface *Img) {
  SDL_Rect R, D;
  SDL_Texture *text;
  
  R.x = X;
  R.y = Y;
  R.w = W;
  R.h = H;
  D.x = startX + frame;
  D.y = 0;
  D.w = W;
  D.h = H;
  text = SDL_CreateTextureFromSurface(ren1,Img);
  SDL_RenderCopy(ren1, text, &D, &R);
  SDL_DestroyTexture(text);
}



void DrawDynamicObject(OBJECT *Object){
  SDL_Rect R;
  SDL_Texture *text;
  
  R.x = Object->X;
  R.y = Object->Y;
  R.w = Object->W;
  R.h = Object->H;
  text = SDL_CreateTextureFromSurface(ren1,Object->Img);
  SDL_RenderCopyEx(ren1, text, NULL, &R, Object->Angle,NULL, SDL_FLIP_NONE);
  SDL_DestroyTexture(text);
}


void DrawObject(OBJECT Object){
  SDL_Rect R;
  SDL_Texture *text;
  
  R.x = Object.X;
  R.y = Object.Y;
  R.w = Object.W;
  R.h = Object.H;
  text = SDL_CreateTextureFromSurface(ren1,Object.Img);
  SDL_RenderCopyEx(ren1, text, NULL, &R, Object.Angle,NULL, SDL_FLIP_NONE);
  SDL_DestroyTexture(text);
}

void DrawEnemy(int x, int y, int w, int h){
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_SetRenderDrawColor(ren1, 255,69,0,0);
  SDL_RenderDrawRect(ren1, &rect);
  SDL_RenderFillRect(ren1, &rect);
}

void DrawText(char* string,int size, int x, int y,int fR, int fG, int fB,int bR, int bG, int bB, BOOL transparent)
{

//if (TTF_Init == NULL) exit(0);

TTF_Font* font = TTF_OpenFont("data/fnts/kiss_font.ttf", size);

SDL_Color foregroundColor = { fR, fG, fB };
SDL_Color backgroundColor = { bR, bG, bB };
SDL_Surface* textSurface; 

if (transparent == TRUE)
   textSurface = TTF_RenderText_Blended(font, string, foregroundColor);
else
   textSurface = TTF_RenderText_Shaded(font, string, foregroundColor,backgroundColor);

SDL_Texture* texture1 = SDL_CreateTextureFromSurface(ren1, textSurface);

SDL_Rect textLocation = { x, y, textSurface->w, textSurface->h };

SDL_RenderCopy(ren1, texture1, NULL, &textLocation);

SDL_FreeSurface(textSurface);
SDL_DestroyTexture(texture1);

TTF_CloseFont(font);

}

void DrawScreen() {
   SDL_RenderClear(ren1);
   if (!scroll){ 
	scrollframe = scrollframe;
        if (scrollframe <0) DrawScroll(0,0,640,480,640,scrollframe,background);
        else DrawScroll(0,0,640,480,0,scrollframe,background);
   }else if (player.X < 1){ 
        if (scrollframe >0) scrollframe=scrollframe*-1;
	scrollframe = scrollframe - 2; 
        DrawScroll(0,0,640,480,640,scrollframe,background);
	}  
   else {	
        if (scrollframe <0) scrollframe=scrollframe*-1;
	scrollframe = scrollframe + 2; 
        DrawScroll(0,0,640,480,0,scrollframe,background);
   }
   DrawText("Hello", 14, 10,10, 255,255,255, 0, 0, 0,TRUE);
   DrawText("there!", 14, 50,32, 255,255,255, 0, 0, 0,FALSE);
   if (moving == TRUE) {

	if (PlayerState == FORWARD)
	  DrawAnimation(player.X,player.Y,0,31,54,movingframe,playerSprite);
        if (PlayerState == BACKWARD)
	  DrawAnimation(player.X,player.Y,65,31,54,movingframe,playerSprite);	
   } else{
	if (PlayerState == FORWARD)
          DrawAnimation(player.X, player.Y,0, 31,54,0, playerSprite);
        if (PlayerState == BACKWARD)
	  DrawAnimation(player.X,player.Y,65,31,54,0,playerSprite);	
 }
   //if (mapX > 150){
       DrawEnemy(790 - enemySpeed, 360,30,50);
  //}
 SDL_RenderPresent(ren1);
}

//Move functions - UPDATE GAME
void movePlayerXY(int speed, int direction){
   if (PlayerState == FORWARD){
     mapX++;
     player.X = player.X + speed;
     if (mapX >150 && scroll){
	  enemySpeed = enemySpeed +10;
     }
   }
   if (PlayerState == BACKWARD){
     mapX--;
     player.X = player.X + speed;
     if (mapX >86 && scroll){
	  enemySpeed = enemySpeed - 10;
     }
   }
   moving=timer1(&movingframe,&gettime,100);
}

void Enemy_Behaviour(){
}
void Player_Behaviour(){
  if (jumping == TRUE) {
	jumping=lerp(&jump,&timetemp,80);
	if (jump>5) player.Y = player.Y + (int) jump * -1;
	else if (player.Y<360) player.Y = player.Y + (10-jump);
	if (jump == 0) player.Y = 360;
  }  
  if (player.X > SCREEN_W / 2 && PlayerState == FORWARD){
	scroll = TRUE;
	player.X = player.X -10;
	if (scrollframe >= 639) scrollframe = 0;
  }
  else if (player.X < 0  && PlayerState == BACKWARD) {
	scroll = TRUE;
	player.X = player.X + 10;
	if (scrollframe <= -639) scrollframe = 0;
  }
  else {
    scroll = FALSE;
  }
  if (Collision(player.X,player.Y,player.X+31, player.Y+54, 790-enemySpeed, 360, (790-enemySpeed) +30,360+50)) {
     printf("Collision\n");
     if (PlayerState == FORWARD) 
	player.X = player.X - 10;
     else
	player.X = player.X + 10;
   }
  printf("%ld\n",mapX);
}
//Timers
BOOL timer1(int *ticks, double *time, int ms){
BOOL value;
 if (SDL_GetTicks() - *time < ms) {
   value = TRUE;
 } else{
   *time = SDL_GetTicks();
   if (*ticks < 5) {
       *ticks = *ticks +1;
	value = TRUE;
   }else {
        *ticks = 0;
 	value = FALSE;
	}
}
 return value;
}

BOOL lerp(double *value, double *time, int ms){
BOOL res;
 if (SDL_GetTicks() - *time < ms) {
   res = TRUE;
 } else{
   *time = SDL_GetTicks();
   if (*value > 0) {
       *value = *value -1;
	res = TRUE;
   }else {
        *value = 0;
 	res = FALSE;
	}
}
 return res;
}


void UpdateGame(){
  oldX = player.X;
  if (Key(SDLK_f)) ToggleFullscreen(win1);
  if (Key(SDL_SCANCODE_SPACE)) { if (jumping==FALSE) {jump = 10; jumping=TRUE;}}
  if (Key(SDL_SCANCODE_RIGHT) || Key(SDLK_d)) {PlayerState = FORWARD; movePlayerXY(SPEED,1);}
  else if (Key(SDL_SCANCODE_LEFT) || Key(SDLK_a)) {PlayerState = BACKWARD; movePlayerXY(-SPEED,1);}
  //if (player.X != oldX) PlayerState = HALTED; 
  Player_Behaviour();
  Enemy_Behaviour();
}

void Main_Loop(){
/* Update + HandleEvents - Draw */
  unsigned int LastTime, CurrentTime;

  LastTime = SDL_GetTicks();
  while (Running==TRUE) {
    	CurrentTime = SDL_GetTicks();
 	if (CurrentTime - LastTime > 1000) LastTime = CurrentTime - 60;
 	while (CurrentTime - LastTime > 1000/30) {
 	  UpdateGame();
          LastTime = LastTime + 30;
 	}  
       HandleEvents();         
       DrawScreen();
    }
} 
/* ---------------------------------------------- */
/* END 						  */
/* ---------------------------------------------- */
