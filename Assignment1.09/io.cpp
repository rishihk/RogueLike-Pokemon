#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
# include <vector>

#include "parse.h"
#include "io.h"
#include "character.h"
#include "poke.h"

typedef struct io_message {
  /* Will print " --more-- " at end of line when another message follows. *
   * Leave 10 extra spaces for that.                                      */
  char msg[71];
  struct io_message *next;
} io_message_t;

static io_message_t *io_head, *io_tail;

void io_init_terminal(void)
{
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void io_reset_terminal(void)
{
  endwin();

  while (io_head) {
    io_tail = io_head;
    io_head = io_head->next;
    free(io_tail);
  }
  io_tail = NULL;
}

void io_queue_message(const char *format, ...)
{
  io_message_t *tmp;
  va_list ap;

  if (!(tmp = (io_message_t *) malloc(sizeof (*tmp)))) {
    perror("malloc");
    exit(1);
  }

  tmp->next = NULL;

  va_start(ap, format);

  vsnprintf(tmp->msg, sizeof (tmp->msg), format, ap);

  va_end(ap);

  if (!io_head) {
    io_head = io_tail = tmp;
  } else {
    io_tail->next = tmp;
    io_tail = tmp;
  }
}

static void io_print_message_queue(uint32_t y, uint32_t x)
{
  while (io_head) {
    io_tail = io_head;
    attron(COLOR_PAIR(COLOR_CYAN));
    mvprintw(y, x, "%-80s", io_head->msg);
    attroff(COLOR_PAIR(COLOR_CYAN));
    io_head = io_head->next;
    if (io_head) {
      attron(COLOR_PAIR(COLOR_CYAN));
      mvprintw(y, x + 70, "%10s", " --more-- ");
      attroff(COLOR_PAIR(COLOR_CYAN));
      refresh();
      getch();
    }
    free(io_tail);
  }
  io_tail = NULL;
}

/**************************************************************************
 * Compares trainer distances from the PC according to the rival distance *
 * map.  This gives the approximate distance that the PC must travel to   *
 * get to the trainer (doesn't account for crossing buildings).  This is  *
 * not the distance from the NPC to the PC unless the NPC is a rival.     *
 *                                                                        *
 * Not a bug.                                                             *
 **************************************************************************/
static int compare_trainer_distance(const void *v1, const void *v2)
{
  const character *const *c1 = (const character * const *) v1;
  const character *const *c2 = (const character * const *) v2;

  return (world.rival_dist[(*c1)->pos[dim_y]][(*c1)->pos[dim_x]] -
          world.rival_dist[(*c2)->pos[dim_y]][(*c2)->pos[dim_x]]);
}

static character *io_nearest_visible_trainer()
{
  character **c, *n;
  uint32_t x, y, count;

  c = (character **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = world.cur_map->cmap[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  n = c[0];

  free(c);

  return n;
}

void io_display()
{
  uint32_t y, x;
  character *c;

  clear();
for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (world.cur_map->cmap[y][x]) {
        mvaddch(y + 1, x, world.cur_map->cmap[y][x]->symbol);
      } else {
        switch (world.cur_map->map[y][x]) {
        case ter_boulder:
          attron(COLOR_PAIR(COLOR_RED));
          mvaddch(y + 1, x, BOULDER_SYMBOL);
          attroff(COLOR_PAIR(COLOR_RED));
          break;
        case ter_mountain:
          attron(COLOR_PAIR(COLOR_MAGENTA));
          mvaddch(y + 1, x, MOUNTAIN_SYMBOL);
          attroff(COLOR_PAIR(COLOR_MAGENTA));
          break;
        case ter_tree:
          attron(COLOR_PAIR(COLOR_GREEN) | A_BOLD); 
          mvaddch(y + 1, x, TREE_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN) | A_BOLD);
          break;
        case ter_forest:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, FOREST_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_path:
          attron(COLOR_PAIR(COLOR_YELLOW));
          mvaddch(y + 1, x, PATH_SYMBOL);
          attroff(COLOR_PAIR(COLOR_YELLOW));
          break;
        case ter_gate:
          attron(COLOR_PAIR(COLOR_YELLOW));
          mvaddch(y + 1, x, GATE_SYMBOL);
          attroff(COLOR_PAIR(COLOR_YELLOW));
          break;
        case ter_mart:
          attron(COLOR_PAIR(COLOR_BLUE));
          mvaddch(y + 1, x, POKEMART_SYMBOL);
          attroff(COLOR_PAIR(COLOR_BLUE));
          break;
        case ter_center:
          attron(COLOR_PAIR(COLOR_RED));
          mvaddch(y + 1, x, POKEMON_CENTER_SYMBOL);
          attroff(COLOR_PAIR(COLOR_RED));
          break;
        case ter_grass:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, TALL_GRASS_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_clearing:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, SHORT_GRASS_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_water:
          attron(COLOR_PAIR(COLOR_CYAN));
          mvaddch(y + 1, x, WATER_SYMBOL);
          attroff(COLOR_PAIR(COLOR_CYAN));
          break;
        default:
          attron(COLOR_PAIR(COLOR_CYAN));
          mvaddch(y + 1, x, ERROR_SYMBOL);
          attroff(COLOR_PAIR(COLOR_CYAN)); 
       }
      }
    }
  }

  mvprintw(23, 1, "PC position is (%2d,%2d) on map %d%cx%d%c.",
           world.pc.pos[dim_x],
           world.pc.pos[dim_y],
           abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_x] - (WORLD_SIZE / 2) >= 0 ? 'E' : 'W',
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2) <= 0 ? 'N' : 'S');
  mvprintw(22, 1, "%d known %s.", world.cur_map->num_trainers,
           world.cur_map->num_trainers > 1 ? "trainers" : "trainer");
  mvprintw(22, 30, "Nearest visible trainer: ");
  if ((c = io_nearest_visible_trainer())) {
    attron(COLOR_PAIR(COLOR_RED));
    mvprintw(22, 55, "%c at vector %d%cx%d%c.",
             c->symbol,
             abs(c->pos[dim_y] - world.pc.pos[dim_y]),
             ((c->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              'N' : 'S'),
             abs(c->pos[dim_x] - world.pc.pos[dim_x]),
             ((c->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              'W' : 'E'));
    attroff(COLOR_PAIR(COLOR_RED));
  } else {
    attron(COLOR_PAIR(COLOR_BLUE));
    mvprintw(22, 55, "NONE.");
    attroff(COLOR_PAIR(COLOR_BLUE));
  }

  io_print_message_queue(0, 0);

  refresh();
}

uint32_t io_teleport_pc(pair_t dest)
{
  /* Just for fun. And debugging.  Mostly debugging. */

  do {
    dest[dim_x] = rand_range(1, MAP_X - 2);
    dest[dim_y] = rand_range(1, MAP_Y - 2);
  } while (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]                  ||
           move_cost[char_pc][world.cur_map->map[dest[dim_y]]
                                                [dest[dim_x]]] == INT_MAX ||
           world.rival_dist[dest[dim_y]][dest[dim_x]] < 0);

  return 0;
}

static void io_scroll_trainer_list(char (*s)[40], uint32_t count)
{
  uint32_t offset;
  uint32_t i;

  offset = 0;

  while (1) {
    for (i = 0; i < 13; i++) {
      mvprintw(i + 6, 19, " %-40s ", s[i + offset]);
    }
    switch (getch()) {
    case KEY_UP:
      if (offset) {
        offset--;
      }
      break;
    case KEY_DOWN:
      if (offset < (count - 13)) {
        offset++;
      }
      break;
    case 27:
      return;
    }

  }
}

static void io_list_trainers_display(npc **c,
                                     uint32_t count)
{
  uint32_t i;
  char (*s)[40]; /* pointer to array of 40 char */

  s = (char (*)[40]) malloc(count * sizeof (*s));

  mvprintw(3, 19, " %-40s ", "");
  /* Borrow the first element of our array for this string: */
  snprintf(s[0], 40, "You know of %d trainers:", count);
  mvprintw(4, 19, " %-40s ", *s);
  mvprintw(5, 19, " %-40s ", "");

  for (i = 0; i < count; i++) {
    snprintf(s[i], 40, "%16s %c: %2d %s by %2d %s",
             char_type_name[c[i]->ctype],
             c[i]->symbol,
             abs(c[i]->pos[dim_y] - world.pc.pos[dim_y]),
             ((c[i]->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              "North" : "South"),
             abs(c[i]->pos[dim_x] - world.pc.pos[dim_x]),
             ((c[i]->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              "West" : "East"));
    if (count <= 13) {
      /* Handle the non-scrolling case right here. *
       * Scrolling in another function.            */
      mvprintw(i + 6, 19, " %-40s ", s[i]);
    }
  }

  if (count <= 13) {
    mvprintw(count + 6, 19, " %-40s ", "");
    mvprintw(count + 7, 19, " %-40s ", "Hit escape to continue.");
    while (getch() != 27 /* escape */)
      ;
  } else {
    mvprintw(19, 19, " %-40s ", "");
    mvprintw(20, 19, " %-40s ",
             "Arrows to scroll, escape to continue.");
    io_scroll_trainer_list(s, count);
  }

  free(s);
}

static void io_list_trainers()
{
  npc **c;
  uint32_t x, y, count;

  c = (npc **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = dynamic_cast<npc *> (world.cur_map->cmap[y][x]);
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  /* Display it */
  io_list_trainers_display(c, count);
  free(c);

  /* And redraw the map */
  io_display();
}

void io_pokemart()
{
  mvprintw(0, 0, "Welcome to the Pokemart.  Could I interest you in some Pokeballs?");
  world.pc.revive = 2;
world.pc.potion = 2;
world.pc.pokeball = 10;
  refresh();
  getch();
}

void io_pokemon_center()
{
  mvprintw(0, 0, "Welcome to the Pokemon Center.  How can Nurse Joy assist you?");
    for(int i = 0; i < world.pc.pokemon_count; i++) {
    world.pc.poke_list[i].currentHP = world.pc.poke_list[i].health;
  }
  refresh();
  getch();
}

int pokemon_selection(pc *p) {
  clear();
  for(int i = 0; i < p->pokemon_count; i++ ) {
      mvprintw(i, 0, "%s %d/%d", p->poke_list[i].name, p->poke_list[i].currentHP, p->poke_list[i].health);
  }
  refresh();
      while (1) {
    switch (getch()) {
    case '1':
      return 0;
      break;
    case '2':
      return 1;
      break;
    case '3':
      return 2;
      break; 
    case '4':
      return 3;
      break;
    case '5':
      return 4;
      break;
    case '6':
      return 5;
      break;
    }

  } 
}

void io_bag(pc *p) {
  clear();
  mvprintw(0, 0, "Revive(1): %d", p->revive);
  mvprintw(1,0, "Potion(2): %d", p->potion);
  mvprintw(2, 0, "Pokeball: %d", p->pokeball);
  refresh();
  int pI = 0;
    while (1) {
    switch (getch()) {
    case 27:
      return;

      break;
    case '1':
     pI = pokemon_selection(p);
      p->poke_list[pI].currentHP = p->poke_list[pI].health / 2;
      p->revive--;
      clear();
      mvprintw(0, 0, "Revive(1): %d", p->revive);
      mvprintw(1,0, "Potion(2): %d", p->potion);
      mvprintw(2, 0, "Pokeball: %d", p->pokeball);
      refresh();
      break;

    case '2':
     pI = pokemon_selection(p);
      p->poke_list[pI].currentHP = std::min(p->poke_list[pI].currentHP + 20, p->poke_list[pI].health);
      p->potion--;
      clear();
      mvprintw(0, 0, "Revive(1): %d", p->revive);
      mvprintw(1,0, "Potion(2): %d", p->potion);
      mvprintw(2, 0, "Pokeball: %d", p->pokeball);
      refresh();
      break;

    }

  } 

}


void io_battle(character *aggressor, character *defender)
{
  npc *n = (npc *) ((aggressor == &world.pc) ? defender : aggressor);
  pc *p;
  npc *pp;
  if(aggressor == &world.pc) {
    p = (pc *) aggressor;
    pp = (npc *) defender;
  }
  else {
    p = (pc *) defender;
    pp = (npc *) aggressor;
  }

  int pI = 0;
  int ppI= 0;
  while(p->poke_list[pI].currentHP == 0 ) {
    pI++;
    if (pI>= p->pokemon_count) {
      return;
    }
  }
  pp->defeated = 1;
  n->defeated = 1;
  n->defeated = 1;
  if (n->ctype == char_hiker || n->ctype == char_rival) {
    n->mtype = move_wander;
  }
  clear();

  mvprintw(2, 0, "%s(lv %d) hp: %d/%d", p->poke_list[pI].name, p->poke_list[pI].level, p->poke_list[pI].currentHP, p->poke_list[pI].health);
  mvprintw(1, 40, "Trainer Pokemon");
  mvprintw(2, 40, "%s(lv %d) hp: %d/%d", pp->poke_list[0].name, pp->poke_list[0].level, pp->poke_list[0].currentHP, pp->poke_list[0].health);

  mvprintw(10, 0, "Fight(1)");
  mvprintw(11, 0, "Bag(2)");
  mvprintw(12, 0, "Run(3)");
  mvprintw(13, 0, "Pokemon(4)");  

  refresh();
  int flag = 0;

  while (1) {
    switch (getch()) {
    case 27:
    flag = 1;
    pp->defeated = 1;
      break;
    
    case '1': {
    attack_selection(p, &pp->poke_list[ppI], pI);
    clear();
    int q = 0;
    for(int w = 0; w < p->pokemon_count; w++) {
      if(p->poke_list[w].currentHP > 0) {
        q = 1;
      }
    }
    if(q == 0) {
      return;
    }
    if(p->poke_list[pI].currentHP <= 0 ) {
      pI = pokemon_selection(p);
    }
    if(pp->poke_list[ppI].currentHP <= 0) {
      ppI++;
    }
    if(ppI >= pp->pokemon_count) {
      return;
    }
    mvprintw(2, 0, "%s(lv %d) hp: %d/%d", p->poke_list[pI].name, p->poke_list[pI].level, p->poke_list[pI].currentHP, p->poke_list[pI].health);
  mvprintw(1, 40, "Trainer Pokemon");
  mvprintw(2, 40, "%s(lv %d) hp: %d/%d", pp->poke_list[ppI].name, pp->poke_list[ppI].level, pp->poke_list[ppI].currentHP, pp->poke_list[ppI].health);

  mvprintw(10, 0, "Fight(1)");
  mvprintw(11, 0, "Bag(2)");
  mvprintw(12, 0, "Run(3)");
  mvprintw(13, 0, "Pokemon(4)");  
  refresh();
    break;
    }

    case '2':
    io_bag(p);
  mvprintw(2, 0, "%s(lv %d) hp: %d/%d", p->poke_list[pI].name, p->poke_list[pI].level, p->poke_list[pI].currentHP, p->poke_list[pI].health);
  mvprintw(1, 40, "Trainer Pokemon");
  mvprintw(2, 40, "%s(lv %d) hp: %d/%d", pp->poke_list[0].name, pp->poke_list[0].level, pp->poke_list[0].currentHP, pp->poke_list[0].health);

  mvprintw(10, 0, "Fight(1)");
  mvprintw(11, 0, "Bag(2)");
  mvprintw(12, 0, "Run(3)");
  mvprintw(13, 0, "Pokemon(4)");  
  refresh();
    break;

    case '3':

    break;

    case '4':
    pI = pokemon_selection(p);
    clear();
      mvprintw(2, 0, "%s(lv %d) hp: %d/%d", p->poke_list[pI].name, p->poke_list[pI].level, p->poke_list[pI].currentHP, p->poke_list[pI].health);
  mvprintw(1, 40, "Trainer Pokemon");
  mvprintw(2, 40, "%s(lv %d) hp: %d/%d", pp->poke_list[0].name, pp->poke_list[0].level, pp->poke_list[0].currentHP, pp->poke_list[0].health);

  mvprintw(10, 0, "Fight(1)");
  mvprintw(11, 0, "Bag(2)");
  mvprintw(12, 0, "Run(3)");
  mvprintw(13, 0, "Pokemon(4)");  
  refresh();

    }
  if(flag == 1) {

    break;
  } 
  }


  return;
}

uint32_t move_pc_dir(uint32_t input, pair_t dest)
{
  dest[dim_y] = world.pc.pos[dim_y];
  dest[dim_x] = world.pc.pos[dim_x];

  switch (input) {
  case 1:
  case 2:
  case 3:
    dest[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    dest[dim_y]--;
    break;
  }
  switch (input) {
  case 1:
  case 4:
  case 7:
    dest[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    dest[dim_x]++;
    break;
  case '>':
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_mart) {
      io_pokemart();
    }
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_center) {
      io_pokemon_center();
    }
    break;
  }

  if (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) {
    if (dynamic_cast<npc *> (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) &&
        ((npc *) world.cur_map->cmap[dest[dim_y]][dest[dim_x]])->defeated) {
      // Some kind of greeting here would be nice
      return 1;
    } else if ((dynamic_cast<npc *>
                (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]))) {
      io_battle(&world.pc, world.cur_map->cmap[dest[dim_y]][dest[dim_x]]);
      // Not actually moving, so set dest back to PC position
      dest[dim_x] = world.pc.pos[dim_x];
      dest[dim_y] = world.pc.pos[dim_y];
    }
  }
  
  if (move_cost[char_pc][world.cur_map->map[dest[dim_y]][dest[dim_x]]] ==
      INT_MAX) {
    return 1;
  }

  return 0;
}

void io_teleport_world(pair_t dest)
{
  /* mvscanw documentation is unclear about return values.  I believe *
   * that the return value works the same way as scanf, but instead   *
   * of counting on that, we'll initialize x and y to out of bounds   *
   * values and accept their updates only if in range.                */
  int x = INT_MAX, y = INT_MAX;
  
  world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = NULL;

  echo();
  curs_set(1);
  do {
    mvprintw(0, 0, "Enter x [-200, 200]:           ");
    refresh();
    mvscanw(0, 21, "%d", &x);
  } while (x < -200 || x > 200);
  do {
    mvprintw(0, 0, "Enter y [-200, 200]:          ");
    refresh();
    mvscanw(0, 21, "%d", &y);
  } while (y < -200 || y > 200);

  refresh();
  noecho();
  curs_set(0);

  x += 200;
  y += 200;

  world.cur_idx[dim_x] = x;
  world.cur_idx[dim_y] = y;

  new_map(1);
  io_teleport_pc(dest);
}



void io_bag_pokemon_battle(pc *p, poke *pp) {

  clear();
  mvprintw(0, 0, "Revive(1): %d", p->revive);
  mvprintw(1,0, "Potion(2): %d", p->potion);
  mvprintw(2, 0, "Pokeball(3): %d", p->pokeball);
  refresh();
  int pI = 0;
    while (1) {
    switch (getch()) {
    case 27:
      return;

      break;
    case '1':
     pI = pokemon_selection(p);
      p->poke_list[pI].currentHP = p->poke_list[pI].health / 2;
      p->revive--;
      clear();
      mvprintw(0, 0, "Revive(1): %d", p->revive);
      mvprintw(1,0, "Potion(2): %d", p->potion);
      mvprintw(2, 0, "Pokeball: %d", p->pokeball);
      refresh();
      break;

    case '2':
     pI = pokemon_selection(p);
      p->poke_list[pI].currentHP = std::min(p->poke_list[pI].currentHP + 20, p->poke_list[pI].health);
      p->potion--;
      clear();
      mvprintw(0, 0, "Revive(1): %d", p->revive);
      mvprintw(1,0, "Potion(2): %d", p->potion);
      mvprintw(2, 0, "Pokeball: %d", p->pokeball);
      refresh();
      break;

case '3':
    clear();
    if(p->pokemon_count >= 6){
        mvprintw(0, 0, "You cannot catch anymore pokemons");
        p->pokeball--;
        refresh(); // Ensure screen is refreshed here
        return;
    }
    if(p->pokeball > 0){
        mvprintw(0, 0, "You have caught a wild %s", pp->name);
        refresh(); // Add refresh here to update the screen immediately
        usleep(1000000); // Consider moving this sleep after the refresh
        p->poke_list[p->pokemon_count] = *pp;
        p->pokemon_count++;
        p->pokeball--;
    } else {
        mvprintw(0, 0, "No Pokeball available");
        refresh(); // Ensure screen is refreshed here
    }
    return;
    break;

    }

  } 

}


void attack_round(pc *p, poke *pp, int pI, int moveSelection) {
  int z;
  int damage;
  if(pp->move_count < 3) {
       z = rand() % pp->move_count;
  }
  else {
    z = rand() % 3;
  }
      if(p->poke_list[pI].pm[moveSelection].priority > pp->pm[z].priority) {
        if(p->poke_list[pI].pm[moveSelection].accuracy  < rand() % 100) {
          mvprintw(20, 0, "Your move has missed");
          refresh();

          usleep(1000000);
        }
        else  {
 //         int r = 85 + rand() % 15;
          damage = (((2*p->poke_list[pI].level) *  p->poke_list[pI].pm[moveSelection].power * (p->poke_list[pI].attack/pp->defense))/50 + 2);
          if(damage > 100) {
            damage = 10;
          }
          pp->currentHP = pp->currentHP - damage;
          mvprintw(20, 0, "%s used %s", p->poke_list[pI].name, p->poke_list[pI].pm[moveSelection].move);
          refresh();
          if(pp->currentHP <= 0) {
           //  mvprintw(23, 0, "You won this battle!");
        refresh();
        usleep(2000000); 
            return;
          }

          usleep(1000000);

        }

          if(pp->pm[z].accuracy  < rand() % 100) {
          mvprintw(20, 0, "Your opponent has missed");
          refresh();

          usleep(1000000);
          }
        else  {
       //   int r = 85 + rand() % 15;
          damage = (((2*pp->level) *  pp->pm[z].power * (pp->attack/p->poke_list[pI].defense))/50 + 2);
          if(damage > 100) {
            damage = 10;
          }
          p->poke_list[pI].currentHP =  p->poke_list[pI].currentHP - damage;
          mvprintw(20, 0, "%s used %s", pp->name, pp->pm[z].move);
          refresh();
          if(p->poke_list[pI].currentHP <= 0) {
            // mvprintw(23, 0, "You won this battle!");
        refresh();
        usleep(2000000); 
            return;
          }

          usleep(1000000);

        }
      }
      else if(p->poke_list[pI].pm[moveSelection].priority < pp->pm[z].priority) {
        if(pp->pm[z].accuracy  < rand() % 100) {
          mvprintw(20, 0, "Your opponent has missed");
          refresh();

          usleep(1000000);
          }
        else  {
//          int r = 85 + rand() % 15;
          damage = (((2*pp->level) *  pp->pm[z].power * (pp->attack/p->poke_list[pI].defense))/50 + 2);
                    if(damage > 100) {
            damage = 10;
          }
          p->poke_list[pI].currentHP =  p->poke_list[pI].currentHP - damage;
          mvprintw(20, 0, "%s used %s", pp->name, pp->pm[z].move);
          refresh();
   if(p->poke_list[pI].currentHP <= 0) {
    // mvprintw(23, 0, "You won this battle!");
        refresh();
        usleep(2000000); 
            return;
          }
          usleep(1000000);

        }

                if(p->poke_list[pI].pm[moveSelection].accuracy  < rand() % 100) {
          mvprintw(20, 0, "Your move has missed");
          refresh();

          usleep(1000000);
        }
        else  {
        //  int r = 85 + rand() % 15;
          damage = (((2*p->poke_list[pI].level) *  p->poke_list[pI].pm[moveSelection].power * (p->poke_list[pI].attack/pp->defense))/50 + 2);
                    if(damage > 100) {
            damage = 10;
          }
          pp->currentHP = pp->currentHP - damage;
          mvprintw(20, 0, "%s used %s", p->poke_list[pI].name, p->poke_list[pI].pm[moveSelection].move);
          refresh();
if(pp->currentHP <= 0) {
  // mvprintw(23, 0, "You won this battle!");
        refresh();
        usleep(2000000); 
            return;
          }
          usleep(1000000);

        }


      }
      else if(p->poke_list[pI].pm[moveSelection].priority == pp->pm[z].priority) {
        if(p->poke_list[pI].speed > pp->speed) {
          if(p->poke_list[pI].pm[moveSelection].accuracy  < rand() % 100) {
          mvprintw(20, 0, "Your move has missed");
          refresh();

          usleep(1000000);
        }
        else  {
//          int r = 85 + rand() % 15;
          damage = (((2*p->poke_list[pI].level) *  p->poke_list[pI].pm[moveSelection].power * (p->poke_list[pI].attack/pp->defense))/50 + 2);
                    if(damage > 100) {
            damage = 10;
          }
          pp->currentHP = pp->currentHP - damage;
          mvprintw(20, 0, "%s used %s", p->poke_list[pI].name, p->poke_list[pI].pm[moveSelection].move);
          refresh();
          if(pp->currentHP <= 0) {
             //mvprintw(23, 0, "You won this battle!");
        refresh();
        usleep(2000000); 
            return;
          }

          usleep(1000000);

        }

          if(pp->pm[z].accuracy  < rand() % 100) {
          mvprintw(20, 0, "Your opponent has missed");
          refresh();

          usleep(1000000);
          }
        else  {
//          int r = 85 + rand() % 15;
          damage = (((2*pp->level) *  pp->pm[z].power * (pp->attack/p->poke_list[pI].defense))/50 + 2);
                    if(damage > 100) {
            damage = 10;
          }
          p->poke_list[pI].currentHP =  p->poke_list[pI].currentHP - damage;
          mvprintw(20, 0, "%s used %s", pp->name, pp->pm[z].move);
          refresh();
             if(p->poke_list[pI].currentHP <= 0) {
                     // mvprintw(23, 0, "You won this battle!");
        refresh();
        usleep(2000000); 
            
            return;
          }

          usleep(1000000);

        }

        }
        else {
          if(pp->pm[z].accuracy  < rand() % 100) {
          mvprintw(20, 0, "Your opponent has missed");
          refresh();

          usleep(1000000);
          }
        else  {
//          int r = 85 + rand() % 15;
          damage = (((2*pp->level) *  pp->pm[z].power * (pp->attack/p->poke_list[pI].defense))/50 + 2);
                    if(damage > 100) {
            damage = 10;
          }
          p->poke_list[pI].currentHP =  p->poke_list[pI].currentHP - damage;
          mvprintw(20, 0, "%s used %s", pp->name, pp->pm[z].move);
          refresh();
             if(p->poke_list[pI].currentHP <= 0) {
                   // mvprintw(23, 0, "You won this battle!");
        refresh();
        usleep(2000000); 
            return;
          }

          usleep(1000000);

        }

                if(p->poke_list[pI].pm[moveSelection].accuracy  < rand() % 100) {
          mvprintw(20, 0, "Your move has missed");
          refresh();

          usleep(1000000);
        }
        else  {
//          int r = 85 + rand() % 15;
          damage = (((2*p->poke_list[pI].level) *  p->poke_list[pI].pm[moveSelection].power * (p->poke_list[pI].attack/pp->defense))/50 + 2);
                    if(damage > 100) {
            damage = 10;
          }
          pp->currentHP = pp->currentHP - damage;
          mvprintw(20, 0, "%s used %s", p->poke_list[pI].name, p->poke_list[pI].pm[moveSelection].move);
          refresh();
          if(pp->currentHP <= 0) {
                    //mvprintw(23, 0, "You won this battle!");
        refresh();
        usleep(2000000); 
            return;
          }

          usleep(1000000);

        }
          
        }


      }
 //       mvprintw(18, 0, "%s(lv %d) hp: %d/%d", p->poke_list[pI].name, p->poke_list[pI].level, p->poke_list[pI].currentHP, p->poke_list[pI].health);
         refresh();
  }

void attack_selection(pc *p, poke *pp, int pI) {

      if(p->poke_list[pI].move_count > 1) {
        mvprintw(4,0, "%s", p->poke_list[pI].pm[0].move);
        }
      if(p->poke_list[pI].move_count > 2) {
        mvprintw(5,0, "%s", p->poke_list[pI].pm[1].move);
        }
      if(p->poke_list[pI].move_count > 3) {
        mvprintw(6,0, "%s", p->poke_list[pI].pm[2].move);
        }
      if(p->poke_list[pI].move_count > 4) {
        mvprintw(7,0, "%s", p->poke_list[pI].pm[3].move);
        }
      
      refresh();

    while (1) {
    switch (getch()) {
    case 27:
      return;

      break;
    case '1':
      attack_round(p, pp, pI, 0);
      return;
      break;
    case '2':
      attack_round(p, pp, pI, 1);
      return;
      break;    
    case '3':
      attack_round(p, pp, pI, 2);
      return;
      break;
    case '4':
      attack_round(p, pp, pI, 3);
      return;
      break;
    }

  }        
  
}

void pokemon_battle(pc *p, poke *pp) {
  clear();
  mvprintw(0, 0, "You Encountered a Wild %s", pp->name);
  mvprintw(1, 0, "PC Pokemon");
  int pI = 0;
  while(p->poke_list[pI].currentHP <= 0 ) {
    pI++;
    if (pI>= p->pokemon_count) {
      return;
    }
  }

  mvprintw(2, 0, "%s(lv %d) hp: %d/%d", p->poke_list[pI].name, p->poke_list[pI].level, p->poke_list[pI].currentHP, p->poke_list[pI].health);
  mvprintw(1, 40, "Wild Pokemon");
  mvprintw(2, 40, "%s(lv %d) hp: %d/%d", pp->name, pp->level, pp->currentHP, pp->health);

  mvprintw(10, 0, "Fight(1)");
  mvprintw(11, 0, "Bag(2)");
  mvprintw(12, 0, "Run(3)");
  mvprintw(13, 0, "Pokemon(4)");  
  refresh();

    while (1) {
    switch (getch()) {
    case 27:
      return;
    break;
    case '1':
      attack_selection(p, pp, pI);
      clear();
        mvprintw(2, 0, "%s(lv %d) hp: %d/%d", p->poke_list[pI].name, p->poke_list[pI].level, p->poke_list[pI].currentHP, p->poke_list[pI].health);
       mvprintw(1, 40, "Wild Pokemon");
        mvprintw(2, 40, "%s(lv %d) hp: %d/%d", pp->name, pp->level, pp->currentHP, pp->health);

        mvprintw(10, 0, "Fight(1)");
        mvprintw(11, 0, "Bag(2)");
        mvprintw(12, 0, "Run(3)");
        mvprintw(13, 0, "Pokemon(4)");  
      if(pp->currentHP <= 0) {
        return;
      }
      break;

    case '2':
      io_bag_pokemon_battle(p, pp);
      refresh();
      if(pp->name == p->poke_list[p->pokemon_count - 1].name) {
        return;
      }
      break;

    case '3':
      clear();
      mvprintw(0,0, "You were able to flee");
      refresh();
      return;
      break;

    case '4':
      pI = pokemon_selection(p);
      clear();
  mvprintw(0, 0, "You Encountered a Wild %s", pp->name);
  mvprintw(1, 0, "PC Pokemon");
  mvprintw(2, 0, "%s(lv %d) hp: %d/%d", p->poke_list[pI].name, p->poke_list[pI].level, p->poke_list[pI].currentHP, p->poke_list[pI].health);
  mvprintw(1, 40, "Wild Pokemon");
  mvprintw(2, 40, "%s(lv %d) hp: %d/%d", pp->name, pp->level, pp->currentHP, pp->health);

  mvprintw(10, 0, "Fight(1)");
  mvprintw(11, 0, "Bag(2)");
  mvprintw(12, 0, "Run(3)");
  mvprintw(13, 0, "Pokemon(4)");  
  refresh();
      break;
    }

  }
}



void io_handle_input(pair_t dest)
{
  uint32_t turn_not_consumed;
  int key;

  do {
    switch (key = getch()) {
    case '7':
    case 'y':
    case KEY_HOME:
      turn_not_consumed = move_pc_dir(7, dest);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      turn_not_consumed = move_pc_dir(8, dest);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      turn_not_consumed = move_pc_dir(9, dest);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      turn_not_consumed = move_pc_dir(6, dest);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      turn_not_consumed = move_pc_dir(3, dest);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      turn_not_consumed = move_pc_dir(2, dest);
      break;
    case '1':
    case 'b':
    case KEY_END:
      turn_not_consumed = move_pc_dir(1, dest);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      turn_not_consumed = move_pc_dir(4, dest);
      break;
    case '5':
    case ' ':
    case '.':
    case KEY_B2:
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    case '>':
      turn_not_consumed = move_pc_dir('>', dest);
      break;
    case 'Q':
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      world.quit = 1;
      turn_not_consumed = 0;
      break;
      break;
    case 't':
      io_list_trainers();
      turn_not_consumed = 1;
      break;
    case 'B':
      io_bag(&world.pc);
      turn_not_consumed = 1;
      break;
    case 'p':
      /* Teleport the PC to a random place in the map.              */
      io_teleport_pc(dest);
      turn_not_consumed = 0;
      break;
    case 'f':
      /* Fly to any map in the world.                                */
      io_teleport_world(dest);
      turn_not_consumed = 0;
      break;    
    case 'q':
      /* Demonstrate use of the message queue.  You can use this for *
       * printf()-style debugging (though gdb is probably a better   *
       * option.  Not that it matters, but using this command will   *
       * waste a turn.  Set turn_not_consumed to 1 and you should be *
       * able to figure out why I did it that way.                   */
      io_queue_message("This is the first message.");
      io_queue_message("Since there are multiple messages, "
                       "you will see \"more\" prompts.");
      io_queue_message("You can use any key to advance through messages.");
      io_queue_message("Normal gameplay will not resume until the queue "
                       "is empty.");
      io_queue_message("Long lines will be truncated, not wrapped.");
      io_queue_message("io_queue_message() is variadic and handles "
                       "all printf() conversion specifiers.");
      io_queue_message("Did you see %s?", "what I did there");
      io_queue_message("When the last message is displayed, there will "
                       "be no \"more\" prompt.");
      io_queue_message("Have fun!  And happy printing!");
      io_queue_message("Oh!  And use 'Q' to quit!");

      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      turn_not_consumed = 1;
    }
    refresh();
  } while (turn_not_consumed);
}
