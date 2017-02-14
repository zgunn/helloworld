#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>

struct player{
    int x;
    int y;

    float shields;
    float max_shields;
    float shield_percent;
    float hull;
    float max_hull;

    float power;
    float max_power;
    int missiles;
    int max_missiles;

    char facing;
    int score;

    char character[1];
};

struct missile{
    int x;
    int y;

    float payload;

    char character[1];
};

struct asteroid{
    int x;
    int y;

    float density;

    char character[1];
};

struct station{
    int x;
    int y;

    int ammo_load;
    float shield_load;
    float repair_load;

    bool full;

    char character[1];
};

typedef struct player Player;
typedef struct asteroid Asteroid;
typedef struct missile Ammo;
typedef struct station Station;

int main(int argc,char *argv[]){
    srand(time(0));
    printf("Space v0.1 coded by Zach Gunn.\n");

    bool running = true;
    bool jumped = false;
    bool fired = false;
    int ch;

    Player ship;
    ship.x = 1;
    ship.y = 1;
    strcpy(ship.character,"@");
    ship.max_shields = 50;
    ship.shields = ship.max_shields;
    ship.max_hull = 20;
    ship.hull = ship.max_hull;
    ship.max_power = 100;
    ship.power = ship.max_power;
    ship.missiles = 10;
    ship.max_missiles = 15;
    ship.facing = 's';
    ship.score = 0;

    Ammo missile;
    missile.payload = rand()%50;

    Asteroid asteroid;
    asteroid.x = rand()%10+1;
    asteroid.y = rand()%50+1;
    asteroid.density = rand()%50+1;
    strcpy(asteroid.character,"#");

    Station station;
    station.x = rand()%10+1;
    station.y = rand()%50+1;
    station.ammo_load = rand()%3+1;
    station.shield_load = rand()%40;
    station.repair_load = rand()%5;
    station.full = true;
    strcpy(station.character,"R");

    initscr();
    curs_set(FALSE);
    keypad(stdscr,TRUE);
    noecho();

    while(running){
        ship.shield_percent = (ship.shields/ship.max_shields)*100;

        if(ship.shield_percent <= 0){
            ship.shield_percent = 0;
        }

        if(ship.shields <= 0){
            ship.shields = 0;
        }
        if(ship.power <= 0){
            ship.power = 0;
        }
        if(ship.hull <= 0){
            ship.hull = 0;
            clear();
            endwin();
            printf("\nYour spaceship has been destroyed!\nYour final score was %d\n\n",ship.score);
            exit(0);
        }
        if(ship.power < ship.max_power && jumped == false){
            if(fired == false){
                ship.power++;
            }else{
                fired = false;
            }
        }else{
            jumped = false;
        }
        if(ship.shields >= ship.max_shields){
            ship.shields = ship.max_shields;
        }
        if(ship.hull >= ship.max_hull){
            ship.hull = ship.max_hull;
        }
        if(ship.missiles >= ship.max_missiles){
            ship.missiles = ship.max_missiles;
        }

        clear();

        mvprintw(0,0,"Hull: %.2f    Shields: %.2f (%.2f%)    Power: %.2f/%.0f    X:%d Y:%d    Missiles: %d    Facing: %c\tScore: %d",ship.hull,ship.shields,ship.shield_percent,ship.power,ship.max_power,ship.x,ship.y,ship.missiles,ship.facing,ship.score);

        mvprintw(ship.x,ship.y,ship.character); 

        if(asteroid.density >= 0){
            if((rand()%100+1) <= 15 && asteroid.x != 21){
                asteroid.x++;
            }
            if((rand()%100+1) <= 15 && asteroid. x != 1){
                asteroid.x--;
            }
            if((rand()%100+1) <= 15 && asteroid.y != 61){
                asteroid.y++;
            }
            if((rand()%100+1) <= 15 && asteroid.y != 1){
                asteroid.y--;
            }
            mvprintw(asteroid.x,asteroid.y,asteroid.character);
        }else{
            asteroid.x = rand()%10+1;
            asteroid.y = rand()%50+1;
            asteroid.density = rand()%100+1;
        }

        if(station.full == true){
            mvprintw(station.x,station.y,station.character);
        }
        else if((rand()%100+1) <= 1){
            station.x = rand()%10+1;
            station.y = rand()%50+1;
            station.ammo_load = rand()%3+1;
            station.shield_load = rand()%40;
            station.repair_load = rand()%5;
            station.full = true;
        }

        refresh();

        ch = getch();

        if(ch == KEY_UP && ship.x != 1){
            ship.x--;
            ship.facing = 'n';
        }
        if(ch == KEY_DOWN && ship.x != 21){
            ship.x++;
            ship.facing = 's';
        }
        if(ch == KEY_RIGHT && ship.y != 61){
            ship.y++;
            ship.facing = 'e';
        }
        if(ch == KEY_LEFT && ship.y != 1){
            ship.y--;
            ship.facing = 'w';
        }

        if(ch == 'f' && ship.missiles > 0){
            missile.payload = rand()%50+1;
            int m=0;
            missile.x = ship.x;
            missile.y = ship.y;
            if(ship.facing == 'n'){
                strcpy(missile.character,"|");
                ship.missiles--;
                while(m<=7){
                    missile.x--;
                    mvprintw(missile.x,missile.y,missile.character);
                    if(missile.x == asteroid.x && missile.y == asteroid.y){
                        asteroid.density -= missile.payload;
                        ship.score++;
                    }
                    refresh();
                    usleep(50000);
                    m++;
                }
            }
            if(ship.facing == 'e'){
                ship.missiles--;
                strcpy(missile.character,"-");
                while(m<=7){
                    missile.y++;
                    mvprintw(missile.x,missile.y,missile.character);
                    if(missile.x == asteroid.x && missile.y == asteroid.y){
                        asteroid.density -= missile.payload;
                        ship.score++;
                    }
                    refresh();
                    usleep(50000);
                    m++;
                }
            }
            if(ship.facing == 's'){
                ship.missiles--;
                strcpy(missile.character,"|");
                while(m<=7){
                    missile.x++;
                    mvprintw(missile.x,missile.y,missile.character);
                    if(missile.x == asteroid.x && missile.y == asteroid.y){
                        asteroid.density -= missile.payload;
                        ship.score++;
                    }
                    refresh();
                    usleep(50000);
                    m++;
                }
            }
            if(ship.facing == 'w'){
                ship.missiles--;
                strcpy(missile.character,"-");
                while(m<=7){
                    missile.y--;
                    mvprintw(missile.x,missile.y,missile.character);
                    if(missile.x == asteroid.x && missile.y == asteroid.y){
                        asteroid.density -= missile.payload;
                        ship.score++;
                    }
                    refresh();
                    usleep(50000);
                    m++;
                }
            }
            fired = true;
        }

        if(ch == 'j' && ship.power >= 10){
            if(ship.facing == 'n'){
                ship.x -= 5;
                ship.power -= 10;
                refresh();
            }
            if(ship.facing == 'e'){
                ship.y += 5;
                ship.power -= 10;
                refresh();
            }
            if(ship.facing == 's'){
                ship.x += 5;
                ship.power -= 10;
                refresh();
            }
            if(ship.facing == 'w'){
                ship.y -= 5;
                ship.power -= 10;
                refresh();
            }
            jumped = true;
        }


        if(ship.x == asteroid.x && ship.y == asteroid.y){
            if(ship.shields > 0){
                ship.shields -= asteroid.density;
            }else{
                ship.hull -= asteroid.density*1.5;
            }
        }
        if(ship.x == station.x && ship.y == station.y){
            ship.missiles += station.ammo_load;
            if(ship.shields < ship.max_shields){
                ship.shields += station.shield_load;
            }
            if(ship.hull < ship.max_hull){
                ship.hull += station.repair_load;
            }
            station.full = false;
        }

    }

    endwin();
}
