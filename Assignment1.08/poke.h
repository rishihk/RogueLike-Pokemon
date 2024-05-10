#ifndef POKE_H
# define POKE_H

# include <stdlib.h>
# include <assert.h>


typedef struct poke {
  char* name;
  int level; 
  int gender; 
  int iv_hp;
  int iv_attack;
  int iv_defense;
  int iv_speed;
  int iv_special_attack;
  int iv_special_defense;
  int health;
  int attack;
  int defense;
  int speed;
  int special_attack;
  int special_defense;
  int pokemon_index;
  int pokemon_id;

  int base_hp;
  int base_attack;
  int base_defense;
  int base_special_attack;
  int base_special_defense;
  int base_speed;

  char* move1;
  char* move2;
  char* move3;
  char* move4;

  int move_count;
  int shiny;

} poke_t;




#endif