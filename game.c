#include <avr/io.h>
#include "graphics.h"
#include "cpu_speed.h"
#include "sprite.h"
#include "lcd.h"
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <math.h>
#include "stdlib.h"
#include "usb_serial.h"



//playfield boundaries
#define MIN_X 0
#define MAX_X 83
#define MIN_Y 8
#define MAX_Y 47


//Global overflow count for TIMER0
volatile unsigned long timer0_ovf_count = 0;
//Glolbal overflow count for TIMER1
volatile unsigned long timer1_ovf_count = 0;
volatile unsigned long counter = 0;
//Used in system time calculations
#define FREQUENCY 8000000.0
#define PRESCALER 1024.0
/*
Variables
*/
bool first_attack = true;
bool boss_battle = false;
bool start_screen = true;
bool gameplay;
bool game_over;
bool in_flight = false;
int direction = 1;
int mother_dir = 1;
int score;
int lives;
int mother_lives = 10;

int random_number = 0;
int seconds = 0;
int seconds_front = 0;
int minutes = 0;
int minutes_front = 0;

int tall = 0;
int tall2 = 0;
double a = 0.0;
double future_time = 0.0;
int teller = 0;


/////////////////


void init_hardware(void);

void draw_ship_and_gun(void);
void draw_game(void);
void draw_start_screen(void);
void count_down(void);
void move_spaceship(void);
void debounce_PINF(int pin_number);
void init_ship(void);
void move_player(void);
bool mother_collided_with_border(void);
// void init_gun(Sprite* sprite_id, int x, int y, unsigned char image);
void shoot_bullet(void);
void sprite_turn_to(Sprite* sprite_id, double dx, double dy);
bool bullet_collision_with_border(void);
bool sprite_collision(Sprite* sprite_1, Sprite* sprite_2);
int random_x_value(Sprite* sprite);
int random_y_value(Sprite* sprite);
void init_alien(void);
void move_alien(void);
bool alien_collided_with_border(void);
void init_gun(void);
void draw_game_over(void);
void process(void);
double get_system_time(void);
void send_debug_string(char* string);
void send_line(char* string);
void move_mother(void);

//Sprites
unsigned char ship[25] = {
0b01110000,
0b10001000,
0b10001000,
0b01110000,
};

unsigned char gun_vertical[4] = {
0b10000000,
0b10000000,
0b10000000,
0b10000000,
};

unsigned char gun_horizontal[4] = {
0b11110000,
};

unsigned char bullet[] = {
0b11000000,
0b11000000,
};

unsigned char alien[] = {
0b10001000,
0b10101000,
0b11111000,
0b10101000,
0b10001000,
};
unsigned char mother[] = {
0b11111111, 0b11000000,
0b01111111, 0b00000000,
0b01000001, 0b00000000,
0b01000001, 0b00000000,
0b01000001, 0b00000000,
0b01000001, 0b00000000,
0b01000001, 0b00000000,
0b01111111, 0b00000000,
};
Sprite space_ship;
Sprite ship_gun;
Sprite Bullet; 
Sprite Alien;
Sprite Mother;
/*
HElPER FUNCTIONS
*/
double sprite_x(Sprite* sprite);
double sprite_y(Sprite* sprite);
int sprite_width(Sprite* sprite);
int sprite_height(Sprite* sprite);
void sprite_move( Sprite* sprite, double dx, double dy);
void sprite_step( Sprite* sprite);
void sprite_turn(Sprite* sprite, double degrees);
void sprite_hide(Sprite* sprite);
void sprite_show(Sprite* sprite);
bool is_visible(Sprite* sprite);
void sprite_move_to(Sprite* sprite, double x, double y);
double sprite_dx(Sprite* sprite);
double sprite_dy(Sprite* sprite);
char buffer_1[16];  
  
/*
///////////////////////////////////////////////////////////////////////
*/
int main(){
    //Set the CPU speed
    set_clock_speed(CPU_8MHz);
    init_hardware();
    clear_screen();
    
    draw_string(MIN_X + 5, MIN_Y, "Waiting for ");
    draw_string(MIN_X + 5, MIN_Y + 8, " connection");
    show_screen();
    while(!usb_configured() || !usb_serial_get_control());

    send_line("Welcome to Alien Advance debugging");
    send_line("Connection between teensy and PC is now active");    
    clear_screen();
    draw_string(MIN_X + 5, MIN_Y - 8, "Connection");
    draw_string(MIN_X + 5, MIN_Y , "established");
    draw_string(MIN_X + 5, MIN_Y + 8, "Play with");
    draw_string(MIN_X + 5, MIN_Y + 16, "keyboard: wasd");        
    draw_string(MIN_X + 5, MIN_Y + 24, "hold space");    
    show_screen();
    _delay_ms(2500);
    clear_screen();
    // init_ship();   
    // init_alien();
    // init_gun();
    // init_sprite(&Mother, 50, 20, 8, 8, mother);
    while(1){
        
        //Have all the while loops in a process function so that we never exit this while loop
        process();
        show_screen();
    }
    return 0;
}

void process(){
// double alien_dx = sprite_dx(&Alien);
// double alien_dy = sprite_dy(&Alien);

// Spaceship x pos: 
// Spaceship y pos:
     while(start_screen){
        clear_screen();
        draw_start_screen();
        show_screen();
        //IF one of the buttons is pushed
        if(((PINF>>PF6) & 1) | ((PINF>>PF5) & 1)){
            //Debounce
            debounce_PINF(6);
            debounce_PINF(5);
            //Start the count_down and then draw the board
            //Also set the gameStarted to true
            //Reset settings here

            init_ship();   
            init_alien();
            init_gun();
            mother_lives = 10;
            score = 0;
            lives = 5;                                     
            teller = 0;
            seconds = 0;
            seconds_front = 0;
            minutes = 0;
            minutes_front = 0;  
            count_down();
            draw_game();
            show_screen();
            start_screen = false;
            boss_battle = false;            
            gameplay = true;
        }
    }
    // init_ship();   
    // init_alien();
    // init_gun();
    // init_sprite(&Mother, 50, 20, 8, 8, mother);
    tall2 = (rand() % 3) + 2; 
    a = rand() % 10;
    a = a/10;
    a = a * 2 + 2;  
    while(gameplay){  
        int spaceship_x = sprite_x(&space_ship);
        int spaceship_y = sprite_y(&space_ship);
        clear_screen();
        draw_game();
        draw_sprite(&Alien);
        move_player();
        shoot_bullet();
        show_screen();
        double time_played = get_system_time();  
        // char b[2];
        // sprintf(b, "%d", tall);
        // draw_string(14, 15, b);
        // sprintf(b, "%d", tall2);            
        // draw_string(25, 25, b);
        // int kay = 0;
        // if(in_flight){
            // kay = 1;
        // }
        // else{kay = 0;}
        // sprintf(b, "%d", kay);
        // draw_string(35, 35, b);
        if(teller == 5 && !boss_battle){
            boss_battle = true;
            sprite_hide(&Alien);     
            sprite_move_to(&Alien, 0, 0);
            in_flight = false;
            Alien.dx = 0;
            Alien.dy = 0;
            init_sprite(&Mother, 50, 20, 10, 8, mother);  
            move_mother();     
        }
        if(boss_battle){
            draw_sprite(&Mother);
            if(!mother_collided_with_border()){ 
                move_mother(); 
            }
            if(time_played >= future_time){
                draw_sprite(&Mother);
                sprite_step(&Mother);
            }
            if(time_played >= future_time && !mother_collided_with_border()){
                a = rand() % 10;
                a = a/10;
                a = a * 4 + 2; 
                future_time = time_played + a; 
            }
        }
        //First time it spawns
        if(first_attack && boss_battle){
            draw_sprite(&Mother);
            sprite_step(&Mother);
            if(!mother_collided_with_border()){
                move_mother();  
                a = rand() % 10;
                a = a/10;
                a = a * 4 + 2;  
                future_time = time_played + a;     
                first_attack = false;
            }
            show_screen();
        }
        if(sprite_collision(&Bullet, &Mother) && is_visible(&Bullet)){
            sprite_move_to(&Bullet, spaceship_x, spaceship_y);
            sprite_hide(&Bullet); 
            mother_lives--;
            if(mother_lives == 9){
                mother[1] = 0b10000000;                 
            }
            if(mother_lives == 8){
                mother[1] = 0b00000000;                 
            }
            if(mother_lives == 7){
                mother[0] = 0b11111110;  
                mother[1] = 0b00000000;                
            }
            if(mother_lives == 6){
                mother[0] = 0b11111100; 
                mother[1] = 0b00000000;                 
            }
            if(mother_lives == 5){
                mother[0] = 0b11111000; 
                mother[1] = 0b00000000;                 
            }
            if(mother_lives == 4){
                mother[0] = 0b11110000; 
                mother[1] = 0b00000000;                 
            }
            if(mother_lives == 3){
                mother[0] = 0b11100000; 
                mother[1] = 0b00000000;                 
            }
            if(mother_lives == 2){
                mother[0] = 0b11000000; 
                mother[1] = 0b00000000;                 
            }
            if(mother_lives == 1){
                mother[0] = 0b10000000;  
                mother[1] = 0b00000000;                
            }
            if(mother_lives == 0){
                sprite_hide(&Mother);
                sprite_move_to(&Mother, 0, 0);
                score += 10;
                init_alien();                
                sprite_show(&Alien);
                mother_lives = 10;
                teller = 0;
                mother[0] = 0b11111111;
                mother[1] = 0b11000000;
                // in_flight = true;
                boss_battle = false;
                send_debug_string("PLAYER KILLED THE MOTHERSHIP");
                show_screen();
            }
        }
        if(sprite_collision(&Mother, &space_ship)){
            // tall = 0;
            sprite_hide(&Bullet);        
            init_ship();
            sprite_move_to(&Bullet, spaceship_x, spaceship_y);
            send_debug_string("MOTHERSHIP KILLED THE PLAYER");
            lives--;             
        }
        if(sprite_collision(&Mother, &ship_gun)){
            tall = 0;
            sprite_hide(&Bullet);        
            init_ship();
            sprite_move_to(&Bullet, spaceship_x, spaceship_y);
            send_debug_string("MOTHERSHIP KILLED THE PLAYER");
            lives--;  
        }
        if(!bullet_collision_with_border()){
            sprite_hide(&Bullet);
            sprite_move_to(&Bullet, spaceship_x, spaceship_y);
        }
        if(sprite_collision(&Bullet, &Alien) && is_visible(&Bullet)){
            sprite_move_to(&Bullet, spaceship_x, spaceship_y);
            score++;
            teller++;
            in_flight = false;
            tall = 0;            
            send_debug_string("THE PLAYER KILLED THE ALIEN");            
            sprite_hide(&Bullet); 
            init_alien();       
        }
        if(sprite_collision(&space_ship, &Alien)){
            tall = 0;            
            sprite_hide(&Bullet);        
            init_ship();
            sprite_move_to(&Bullet, spaceship_x, spaceship_y);
            send_debug_string("ALIEN KILLED THE PLAYER");
            lives--;
        }
        if(sprite_collision(&Alien, &ship_gun)){
            tall = 0;            
            sprite_hide(&Bullet);        
            init_ship();
            sprite_move_to(&Bullet, spaceship_x, spaceship_y);
            send_debug_string("ALIEN KILLED THE PLAYER");
            lives--;
        }
        if(alien_collided_with_border() && in_flight){
            // tall = 0;
            draw_sprite(&Alien);    
            sprite_step(&Alien);
        }
        if(!alien_collided_with_border()){
            in_flight = false; 
            Alien.dx = 0;
            Alien.dy = 0;  
            
            draw_sprite(&Alien);   
            // move_alien();
        }
        if(counter > 50){
            char biff[5];
            sprintf(biff, " (x,y) %d,%d", spaceship_x, spaceship_y);
            // char buff[2];
            // sprintf(buff, "%d", c);
            // send_line(buff);
            if(direction == 4){
                send_debug_string("Heading: Left");
                send_debug_string(biff);            
            }
            else if(direction == 1){
                send_debug_string("Heading: Up");        
                send_debug_string(biff);            
            }
            else if(direction == 2){
                send_debug_string("Heading: Right");        
                send_debug_string(biff);            
            }
            else if(direction == 3){
                send_debug_string(biff);            
                send_debug_string("Heading: Down");    
            }
            counter = 0;
        }
        if(lives <= 0){
            game_over = true;
            gameplay = false;
            in_flight = false;
            boss_battle = false;
        }
        //used for random dash between 2 - 4 seconds
        //Random num is compared with this value, so it needs to be reset when it reaches 4
        if(tall > 4){
            tall = 0;
        }
        if(timer0_ovf_count > 100){
            if(seconds_front == 5 && seconds == 9){
                seconds = 0;
                seconds_front = 0;
                minutes++;                
            }
            else if(seconds == 9){
                seconds = 0;
                seconds_front++;
            }
            else{
                seconds++;   
            }
            if(!in_flight)
            // if(!in_flight){
                tall++;                                                
            // }
            timer0_ovf_count = 0;
        }
        if(tall == tall2 && !boss_battle){
            tall2 = (rand() % 2) + 2;                                                                                                     
            tall = 0;
            move_alien();
            sprite_step(&Alien);
            in_flight = true;
        }
        show_screen();
    }
    while(game_over){
        clear_screen();
        draw_game_over();
        show_screen();
        if(((PINF>>PF6) & 1) | ((PINF>>PF5) & 1)){
            //Debounce
            debounce_PINF(6);
            debounce_PINF(5);
            //Start the count_down and then draw the board
            //Also set the gameStarted to true
            start_screen = true;
            // gameplay = false;
            game_over = false;
        }
    }
}
//taken from tutorial 10, question 2-3
double get_system_time(void){
    return (timer1_ovf_count * 65536 + TCNT1) * PRESCALER / FREQUENCY;
    // return timer_count*(PRESCALER/FREQUENCY);
}
void count_down(void){
    char buffer[1];
    for(int i = 3; i >= 1; i--){
        sprintf(buffer,"%d", i);
        clear_screen();
        draw_string(MIN_X + 42, MIN_Y + 10 , buffer);
        show_screen();
        _delay_ms(300);
    }
    clear_screen();
}
/*
COLLISION
*/
bool bullet_collision_with_border(){
    // int width = sprite_width(sprite);
    // int height = sprite_height(sprite);
    int x = sprite_x(&Bullet);
    int y = sprite_y(&Bullet);
    
    if(y > MIN_Y && y < MAX_Y && x < MAX_X && x > MIN_X) {
        return true;
        }
    return false;
}
bool sprite_collision(Sprite* sprite_1, Sprite* sprite_2){
    int sprite_1_top = round(sprite_y(sprite_1)),
    sprite_1_bot = sprite_1_top + sprite_height(sprite_1) - 1,
    sprite_1_left = round(sprite_x(sprite_1)),
    sprite_1_right = sprite_1_left + sprite_width(sprite_1) - 1;

    int sprite_2_top = round(sprite_y(sprite_2)),
    sprite_2_bot = sprite_2_top + sprite_height(sprite_2) - 1,
    sprite_2_left = round(sprite_x(sprite_2)),
    sprite_2_right = sprite_2_left + sprite_width(sprite_2) - 1;

    return !(
        sprite_1_bot < sprite_2_top
        || sprite_1_top > sprite_2_bot
        || sprite_1_right < sprite_2_left
        || sprite_1_left > sprite_2_right
        );
}
bool alien_collided_with_border(){
    int x = sprite_x(&Alien);
    int y = sprite_y(&Alien);
    
    if(y > MIN_Y + 1 && y < MAX_Y - 5 && x < MAX_X - 6 && x > MIN_X + 1) {
        return true;
        }
    return false;
}
bool mother_collided_with_border(){
    int x = sprite_x(&Mother);
    int y = sprite_y(&Mother);
    
    if(y > MIN_Y + 1 && y < MAX_Y - 8 && x < MAX_X - 8 && x > MIN_X + 1) {
        return true;
        }
    return false;
}
/*
MOVING FUNCTIONS
*/
void move_player(){
    int height_ship = sprite_height(&space_ship);
    int width_ship = sprite_width(&space_ship);
    int x = sprite_x(&space_ship); 
    int y = sprite_y(&space_ship); 
    // unsigned char* map = ship_gun.bitmap;
    int height_gun = sprite_height(&ship_gun);
    int width_gun = sprite_width(&ship_gun);
    // char b[10];
    // sprintf(b, map);
    // send_debug_string(b);
    int16_t curr_char = 0;
    if(usb_serial_available()){
         curr_char = usb_serial_getchar();
    }
     //LEFT
    if((((PINB >> PB1) & 1) && x > MIN_X + width_gun - 1) || (curr_char == 'a' && x > MIN_X + width_gun - 1)){
        // init_sprite(&ship_gun_horizontal, x - 3, y + 2, 4, 2, gun_horizontal);
        ship_gun.bitmap = gun_horizontal;
        ship_gun.width = 4;
        ship_gun.height = 1;
        ship_gun.x = x - 4;
        ship_gun.y = y + 2;
        sprite_move(&space_ship, - 1, 0);
        draw_sprite(&ship_gun);
        
        direction = 4;
        // _delay_ms(50);
    }
    //UP
    if((((PIND >> PD1) & 1 ) && (y > MIN_Y + height_gun - 1)) || (curr_char == 'w' && y > MIN_Y + height_gun - 1) ){
        // init_sprite(&ship_gun_vertical, x + 2, y - 3, 2, 4, gun_vertical);   
        ship_gun.bitmap = gun_vertical;
        ship_gun.x = x + 2;
        ship_gun.y = y - 4;  
        ship_gun.width = 1;
        ship_gun.height = 4;   
        sprite_move(&space_ship, 0, - 1);
        direction = 1;
        draw_sprite(&ship_gun);
        
        // _delay_ms(50);            
    }
    //DOWN
    if((((PINB >> PB7) & 1 ) || curr_char =='s') && (y < MAX_Y - height_gun - height_ship + 2)){
        // init_sprite(&ship_gun_vertical, x + 2, y  + height_ship - 1, 2, 4, gun_vertical);   
        ship_gun.bitmap = gun_vertical;
        ship_gun.x = x + 2;
        ship_gun.y = y + height_ship;  
        ship_gun.width = 1;
        ship_gun.height = 4;           
        sprite_move(&space_ship, 0, + 1);
        direction = 3;
        draw_sprite(&ship_gun);
        
        // _delay_ms(50);                    
    }
    //RIGHT
    if((((PIND >> PD0) & 1 ) || curr_char == 'd') && x < (MAX_X - width_gun - width_ship + 2)){          
        // init_sprite(&ship_gun_horizontal, x + width_ship - 1, y + 2, 4, 2, gun_horizontal);   
        ship_gun.bitmap = gun_horizontal;
        ship_gun.x = x + width_ship;
        ship_gun.width = 4;
        ship_gun.height = 1;
        ship_gun.y = y + 2;             
        sprite_move(&space_ship, + 1, 0);
        draw_sprite(&ship_gun);
        
        direction = 2;
        // _delay_ms(50);            
    }
    //Draw the gun based on the direction 
    // if(direction == 4 || direction == 2){
    //     draw_sprite(&ship_gun_horizontal);
    // }
    // if(direction == 1 || direction == 3){
    //     draw_sprite(&ship_gun_vertical);
    // }
    draw_sprite(&ship_gun);
    
    draw_sprite(&space_ship);   
}
void shoot_bullet(){
    int x = sprite_x(&ship_gun);
    int y = sprite_y(&ship_gun);
    int16_t curr_char = 0;
    if(usb_serial_available()){
         curr_char = usb_serial_getchar();
    }
    // int x_vertical = sprite_x(&ship_gun);
    // int y_vertical = sprite_y(&ship_gun);

    if(((PINF >> PF5) & 1 || curr_char == ' ')  && gameplay){
        // debounce_PINF(5);dddddddddd
        //Right
        if(direction == 2 && !is_visible(&Bullet)){
            init_sprite(&Bullet, x, y, 2, 2, bullet);
            sprite_turn_to(&Bullet, 1.2, 0);
            sprite_show(&Bullet);
        }
        //UP
        if(direction == 1 && !is_visible(&Bullet)){
            init_sprite(&Bullet, x, y + 1, 2, 2, bullet);
            sprite_turn_to(&Bullet, 0, - 1.2);
            sprite_show(&Bullet);            
        }
        //Down
        if(direction == 3 && !is_visible(&Bullet)){
            init_sprite(&Bullet, x, y, 2, 2, bullet);
            sprite_turn_to(&Bullet, 0, 1.2);
            sprite_show(&Bullet);             
        }    
        //Left
        if(direction == 4 && !is_visible(&Bullet)){
            init_sprite(&Bullet, x + 1, y, 2, 2, bullet);
            sprite_turn_to(&Bullet, - 1.2, 0);
            sprite_show(&Bullet);            
        }

    }
            draw_sprite(&Bullet);
        sprite_step(&Bullet);
}
void move_alien(){
    int x = sprite_x(&Alien);
    int y = sprite_y(&Alien);
    int ship_x = sprite_x(&space_ship);
    int ship_y = sprite_y(&space_ship);
    double dx = ship_x - x;
    double dy = ship_y - y;

    double dist = sqrt(dx * dx + dy * dy);

    dx = (dx*0.7)/dist;
    dy = (dy*0.7)/dist; 

    sprite_turn_to(&Alien, dx, dy);
}
void move_mother(){
    int x = sprite_x(&Mother);
    int y = sprite_y(&Mother);
    int ship_x = sprite_x(&space_ship);
    int ship_y = sprite_y(&space_ship);
    double dx = ship_x - x;
    double dy = ship_y - y;

    double dist = sqrt(dx * dx + dy * dy);

    dx = (dx*.4)/dist;
    dy = (dy*.4)/dist; 

    sprite_turn_to(&Mother, dx, dy);
}

/*
/////DRAW FUNCTIONS
*/
void init_ship(void){
    random_number += random_number;
    unsigned int x = random_x_value(&space_ship);
    unsigned int y = random_y_value(&space_ship);
    int alien_x = sprite_x(&Alien);
    int alien_y = sprite_y(&Alien);
    
    if(abs(alien_y - y) < 10 || abs(alien_x - x ) < 7){
        x = random_x_value(&space_ship);
        y = random_y_value(&space_ship);
    }
        init_sprite(&space_ship, x, y, 5, 4, ship);         
        init_gun();    
}
void init_gun(){
    int height_ship = sprite_height(&space_ship);
    int width_ship = sprite_width(&space_ship);
    int x = sprite_x(&space_ship); 
    int y = sprite_y(&space_ship); 
    
     //LEFT
    if(direction == 4){
        init_sprite(&ship_gun, x - 3, y + 2, 4, 1, gun_horizontal);
        direction = 4;
    }
    //UP
    if(direction == 1){
        init_sprite(&ship_gun, x + 2, y - 2, 1, 4, gun_vertical);        
    }
    //DOWN
    if(direction == 3){
        init_sprite(&ship_gun, x + 2, y  + height_ship - 2, 1, 4, gun_vertical);                
    }
    //RIGHT
    if(direction == 2){          
        init_sprite(&ship_gun, x + width_ship - 1, y + 2, 4, 1, gun_horizontal);                
    }
    //Draw the gun based on the direction 
    // if(direction == 4 || direction == 2){
    //     draw_sprite(&ship_gun_horizontal);
    // }
    // if(direction == 1 || direction == 3){
    //     draw_sprite(&ship_gun_vertical);
    // }
}
void init_alien(void){
    random_number += random_number;
    unsigned int x = random_x_value(&Alien);
    unsigned int y = random_y_value(&Alien);
    int spaceship_x = sprite_x(&space_ship);
    int spaceship_y = sprite_y(&space_ship);
    
    if(abs(x - spaceship_x) < 7 || abs(y - spaceship_y) < 10 ){
        x = random_x_value(&Alien);
        y = random_y_value(&Alien);                
    }
    if(y > MIN_Y + 1 && y < MAX_Y - 5 && x < MAX_X - 6 && x > MIN_X + 1) {
        x = random_x_value(&Alien);
        y = random_y_value(&Alien);        
    }
    init_sprite(&Alien, x, y, 5, 5, alien);    
    
}
int random_x_value(Sprite* sprite){
    int width = 8;
    srand(random_number);
    int x = rand() % (MAX_X - width + MIN_X);
    if(x > MAX_X){
        x = MAX_X - width;
    }
    return x;
}
int random_y_value(Sprite *sprite){
    int height = 8;
    srand(random_number);
    int y = rand() % (MAX_Y - height + MIN_Y);    
    if(y <= MIN_Y){
        y = MIN_Y + height + 4;
    }
    else if(y + height >= 47){
        y = 47 - height - 1;
    }
    return y;
}
void draw_game(void){
    //Left wall
    draw_line(MIN_X, MIN_Y, MIN_X, MAX_Y);
    //Bottom
    draw_line(MIN_X, MAX_Y, MAX_X, MAX_Y);
    //Right
    draw_line(MAX_X, MAX_Y, MAX_X, MIN_Y);
    //Top
    draw_line(MAX_X, MIN_Y, MIN_X, MIN_Y);
    
    char life_buffer[2];
    sprintf(life_buffer, "L:%d", lives);
    draw_string(MIN_X, MIN_Y - 8, life_buffer);

    char score_buffer[2];
    sprintf(score_buffer, "S:%d", score);
    draw_string(MIN_X + 25, MIN_Y - 8, score_buffer);

    char time_buffer[5];
    sprintf(time_buffer, "%d%d:%d%d", minutes_front, minutes, seconds_front, seconds);
    draw_string(MIN_X + 50, MIN_Y - 8, time_buffer);
}
void draw_start_screen(void){
    draw_string(MIN_X + 4, MIN_Y - 8, "Alien Advance");
    draw_string(MIN_X + 4, MIN_Y , "Marius Imingen");
    draw_string(MIN_X + 4, MIN_Y + 8, "n9884076");
    draw_string(MIN_X + 4, MIN_Y + 16, "Press any button");
    draw_string(MIN_X + 4, MIN_Y + 24, "to continue");
}
void draw_game_over(){
    draw_string(MAX_X/2 - 10, MIN_Y - 8, "GAME");
    draw_string(MAX_X/2 - 10, MIN_Y, "OVER!");   
    draw_string(MIN_X + 2, MIN_Y + 16, "Press any button");   
    draw_string(MIN_X + 4, MIN_Y + 24, "to play again...");       
  
}
//USB AND DEBUG
//Taken from tutorial question 10, exercise 2-3
void send_debug_string(char* string){
    // char buffer[16];
    sprintf(buffer_1, "[DEBUG @%06.3f ", get_system_time());
    usb_serial_write(buffer_1, 16);

    unsigned char char_count = 0;
    while(*string != '\0'){
        usb_serial_putchar(*string);
        string++;
        char_count++;
    }
    usb_serial_putchar('\r');
    usb_serial_putchar('\n');
}
void send_line(char* string){
    unsigned char char_count = 0;
    while(*string != '\0'){
        usb_serial_putchar(*string);
        string++;
        char_count++;
    }
    usb_serial_putchar('\r');
    usb_serial_putchar('\n');
}
/*
////////////////////////////////////
*/
void debounce_PINF(int pin_number){
    while ((PINF>>pin_number) & 1){
    random_number++;
    }
}
void init_hardware(void){
    lcd_init(LCD_HIGH_CONTRAST);

    //Setup the buttons as input
    DDRF &= ~((1 << PF5) | (1 << PF6));

    //Setup the joystick as input
    DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB7));
    DDRD &= ~((1 << PD0) | (1 << PD1));

    //Setup TIMER0 for compare interrupt
    //Clear timer on compare
    TCCR0A |= (1 << WGM01);
    //Set the prescaler on TIMER0 to 1024
    TCCR0B |= (1 << CS02) | (1 << CS00);

    OCR0A = 78.125;//117;
    //Enable compare interupt on TIMER0
    TIMSK0 |= (1 << OCIE0A);

    //TIMER1 for overflos
    //SET THE PRESCALER
    TCCR1B |= (1 << CS12) | (1 << CS10);
    //Enable compare interrupt
    TIMSK1 |= (1 << TOIE1);
    //Enable global interupts
    usb_init();
    sei();
}
void sprite_move(Sprite* sprite, double dx, double dy) {
	sprite->x += dx;
	sprite->y += dy;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
void sprite_move_to(Sprite* sprite, double x, double y){
    sprite->x = x;
	sprite->y = y;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
double sprite_x(Sprite* sprite ) {
	return sprite->x;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
double sprite_y(Sprite* sprite){
    return sprite->y;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
double sprite_dx(Sprite* sprite){
    return sprite->dx;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
double sprite_dy(Sprite* sprite){
    return sprite->dy;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
int sprite_width(Sprite* sprite){
    return sprite->width;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
int sprite_height(Sprite* sprite){
    return sprite->height;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
void sprite_turn_to(Sprite* sprite, double dx, double dy){
    sprite->dx = dx;
    sprite->dy = dy;
}
//Doesnt work 
void sprite_turn(Sprite* sprite, double degrees ) {
	double radians = degrees * M_PI / 180;
	double s = sin( radians );
	double c = cos( radians );
	double dx = c * sprite->dx + s * sprite->dy;
	double dy = -s * sprite->dx + c * sprite->dy;
	sprite->dx = dx;
	sprite->dy = dy;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
void sprite_step( Sprite* sprite ) {
	sprite->x += sprite->dx;
	sprite->y += sprite->dy;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
void sprite_hide(Sprite* sprite){
    sprite->is_visible = false;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
void sprite_show(Sprite* sprite){
    sprite->is_visible = true;
}
//FROM ZDK, AUTHOR IS LAWRENCE BUCKINGHAM
bool is_visible(Sprite* sprite){
    return sprite->is_visible;
}

/*
INTERUPTS
*/
ISR(TIMER0_COMPA_vect){
    counter++;
    if(!game_over && gameplay){
    timer0_ovf_count++;  
    }

}
ISR(TIMER1_OVF_vect){
    timer1_ovf_count++;
}
