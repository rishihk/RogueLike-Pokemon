#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <assert.h>

#include "heap.h"

#define malloc(size) ({          \
  void *_tmp;                    \
  assert((_tmp = malloc(size))); \
  _tmp;                          \
})

typedef struct path
{
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
} path_t;

typedef enum dim
{
  dim_x,
  dim_y,
  num_dims
} dim_t;

typedef int16_t pair_t[num_dims];

#define MAP_X 80
#define MAP_Y 21
#define MIN_TREES 20
#define MIN_BOULDERS 20
#define TREE_PROB 95
#define BOULDER_PROB 95
#define WORLD_SIZE 401

#define mappair(pair) (m->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (m->map[y][x])
#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]])
#define heightxy(x, y) (m->height[y][x])

typedef enum __attribute__((__packed__)) terrain_type
{
  ter_boulder,
  ter_tree,
  ter_path,
  ter_mart,
  ter_center,
  ter_grass,
  ter_clearing,
  ter_mountain,
  ter_forest,
  ter_water,
  ter_gate,
  ter_debug
} terrain_type_t;

typedef struct map
{
  terrain_type_t map[MAP_Y][MAP_X];
  uint8_t height[MAP_Y][MAP_X];
  int8_t n, s, e, w;
} map_t;

typedef struct queue_node
{
  int x, y;
  struct queue_node *next;
} queue_node_t;

typedef struct pc
{
  pair_t pos;
} pc_t;

typedef struct world
{
  map_t *world[WORLD_SIZE][WORLD_SIZE];
  pair_t cur_idx;
  map_t *cur_map;

  int hiker_cost[MAP_Y][MAP_X];
  int rival_cost[MAP_Y][MAP_X];

  pc_t pc;

} world_t;

#define IMPASSIBLE INT_MAX
#define FASTER 10
#define SLOWEST 50
#define FASTEST 7
#define FAST 15
#define SLOW 20

world_t world;

static int32_t path_cmp(const void *key, const void *with)
{
  return ((path_t *)key)->cost - ((path_t *)with)->cost;
}

static int32_t edge_penalty(int8_t x, int8_t y)
{
  return (x == 1 || y == 1 || x == MAP_X - 2 || y == MAP_Y - 2) ? 2 : 1;
}

static void dijkstra_path(map_t *m, pair_t from, pair_t to)
{
  static path_t path[MAP_Y][MAP_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized)
  {
    for (y = 0; y < MAP_Y; y++)
    {
      for (x = 0; x < MAP_X; x++)
      {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, path_cmp, NULL);

  for (y = 1; y < MAP_Y - 1; y++)
  {
    for (x = 1; x < MAP_X - 1; x++)
    {
      path[y][x].hn = heap_insert(&h, &path[y][x]);
    }
  }

  while ((p = heap_remove_min(&h)))
  {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x])
    {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y])
      {
        mapxy(x, y) = ter_path;
        heightxy(x, y) = 0;
      }
      heap_delete(&h);
      return;
    }

    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1))))
    {
      path[p->pos[dim_y] - 1][p->pos[dim_x]].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1));
      path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]]
                                               .hn);
    }
    if ((path[p->pos[dim_y]][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]][p->pos[dim_x] - 1].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y]))))
    {
      path[p->pos[dim_y]][p->pos[dim_x] - 1].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y]));
      path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]]
                                           [p->pos[dim_x] - 1]
                                               .hn);
    }
    if ((path[p->pos[dim_y]][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]][p->pos[dim_x] + 1].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y]))))
    {
      path[p->pos[dim_y]][p->pos[dim_x] + 1].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y]));
      path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]]
                                           [p->pos[dim_x] + 1]
                                               .hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1))))
    {
      path[p->pos[dim_y] + 1][p->pos[dim_x]].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1));
      path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]]
                                               .hn);
    }
  }
}

static int build_paths(map_t *m)
{
  pair_t from, to;

  if (m->e != -1 && m->w != -1)
  {
    from[dim_x] = 1;
    to[dim_x] = MAP_X - 2;
    from[dim_y] = m->w;
    to[dim_y] = m->e;

    dijkstra_path(m, from, to);
  }

  if (m->n != -1 && m->s != -1)
  {
    from[dim_y] = 1;
    to[dim_y] = MAP_Y - 2;
    from[dim_x] = m->n;
    to[dim_x] = m->s;

    dijkstra_path(m, from, to);
  }

  if (m->e == -1)
  {
    if (m->s == -1)
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }
    else
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->w == -1)
  {
    if (m->s == -1)
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }
    else
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->n == -1)
  {
    if (m->e == -1)
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }
    else
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->s == -1)
  {
    if (m->e == -1)
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }
    else
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }

    dijkstra_path(m, from, to);
  }

  return 0;
}

static int gaussian[5][5] = {
    {1, 4, 7, 4, 1},
    {4, 16, 26, 16, 4},
    {7, 26, 41, 26, 7},
    {4, 16, 26, 16, 4},
    {1, 4, 7, 4, 1}};

static int smooth_height(map_t *m)
{
  int32_t i, x, y;
  int32_t s, t, p, q;
  queue_node_t *head, *tail, *tmp;
  uint8_t height[MAP_Y][MAP_X];

  memset(&height, 0, sizeof(height));

  for (i = 1; i < 255; i += 20)
  {
    do
    {
      x = rand() % MAP_X;
      y = rand() % MAP_Y;
    } while (height[y][x]);
    height[y][x] = i;
    if (i == 1)
    {
      head = tail = malloc(sizeof(*tail));
    }
    else
    {
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

  while (head)
  {
    x = head->x;
    y = head->y;
    i = height[y][x];

    if (x - 1 >= 0 && y - 1 >= 0 && !height[y - 1][x - 1])
    {
      height[y - 1][x - 1] = i;
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y - 1;
    }
    if (x - 1 >= 0 && !height[y][x - 1])
    {
      height[y][x - 1] = i;
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y;
    }
    if (x - 1 >= 0 && y + 1 < MAP_Y && !height[y + 1][x - 1])
    {
      height[y + 1][x - 1] = i;
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y + 1;
    }
    if (y - 1 >= 0 && !height[y - 1][x])
    {
      height[y - 1][x] = i;
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y - 1;
    }
    if (y + 1 < MAP_Y && !height[y + 1][x])
    {
      height[y + 1][x] = i;
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y + 1;
    }
    if (x + 1 < MAP_X && y - 1 >= 0 && !height[y - 1][x + 1])
    {
      height[y - 1][x + 1] = i;
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y - 1;
    }
    if (x + 1 < MAP_X && !height[y][x + 1])
    {
      height[y][x + 1] = i;
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y;
    }
    if (x + 1 < MAP_X && y + 1 < MAP_Y && !height[y + 1][x + 1])
    {
      height[y + 1][x + 1] = i;
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y + 1;
    }

    tmp = head;
    head = head->next;
    free(tmp);
  }

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      for (s = t = p = 0; p < 5; p++)
      {
        for (q = 0; q < 5; q++)
        {
          if (y + (p - 2) >= 0 && y + (p - 2) < MAP_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < MAP_X)
          {
            s += gaussian[p][q];
            t += height[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      m->height[y][x] = t / s;
    }
  }
  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      for (s = t = p = 0; p < 5; p++)
      {
        for (q = 0; q < 5; q++)
        {
          if (y + (p - 2) >= 0 && y + (p - 2) < MAP_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < MAP_X)
          {
            s += gaussian[p][q];
            t += height[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      m->height[y][x] = t / s;
    }
  }
  return 0;
}

static void find_building_location(map_t *m, pair_t p)
{
  do
  {
    p[dim_x] = rand() % (MAP_X - 3) + 1;
    p[dim_y] = rand() % (MAP_Y - 3) + 1;

    if ((((mapxy(p[dim_x] - 1, p[dim_y]) == ter_path) &&
          (mapxy(p[dim_x] - 1, p[dim_y] + 1) == ter_path)) ||
         ((mapxy(p[dim_x] + 2, p[dim_y]) == ter_path) &&
          (mapxy(p[dim_x] + 2, p[dim_y] + 1) == ter_path)) ||
         ((mapxy(p[dim_x], p[dim_y] - 1) == ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y] - 1) == ter_path)) ||
         ((mapxy(p[dim_x], p[dim_y] + 2) == ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 2) == ter_path))) &&
        (((mapxy(p[dim_x], p[dim_y]) != ter_mart) &&
          (mapxy(p[dim_x], p[dim_y]) != ter_center) &&
          (mapxy(p[dim_x] + 1, p[dim_y]) != ter_mart) &&
          (mapxy(p[dim_x] + 1, p[dim_y]) != ter_center) &&
          (mapxy(p[dim_x], p[dim_y] + 1) != ter_mart) &&
          (mapxy(p[dim_x], p[dim_y] + 1) != ter_center) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_mart) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_center))) &&
        (((mapxy(p[dim_x], p[dim_y]) != ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y]) != ter_path) &&
          (mapxy(p[dim_x], p[dim_y] + 1) != ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_path))))
    {
      break;
    }
  } while (1);
}

static int place_pokemart(map_t *m)
{
  pair_t p;

  find_building_location(m, p);

  mapxy(p[dim_x], p[dim_y]) = ter_mart;
  mapxy(p[dim_x] + 1, p[dim_y]) = ter_mart;
  mapxy(p[dim_x], p[dim_y] + 1) = ter_mart;
  mapxy(p[dim_x] + 1, p[dim_y] + 1) = ter_mart;

  return 0;
}

static int place_center(map_t *m)
{
  pair_t p;

  find_building_location(m, p);

  mapxy(p[dim_x], p[dim_y]) = ter_center;
  mapxy(p[dim_x] + 1, p[dim_y]) = ter_center;
  mapxy(p[dim_x], p[dim_y] + 1) = ter_center;
  mapxy(p[dim_x] + 1, p[dim_y] + 1) = ter_center;

  return 0;
}
                                                   
static terrain_type_t border_type(map_t *m, int32_t x, int32_t y)
{
  int32_t p, q;
  int32_t r, t;
  int32_t miny, minx, maxy, maxx;

  r = t = 0;

  miny = y - 1 >= 0 ? y - 1 : 0;
  maxy = y + 1 <= MAP_Y ? y + 1 : MAP_Y;
  minx = x - 1 >= 0 ? x - 1 : 0;
  maxx = x + 1 <= MAP_X ? x + 1 : MAP_X;

  for (q = miny; q < maxy; q++)
  {
    for (p = minx; p < maxx; p++)
    {
      if (q != y || p != x)
      {
        if (m->map[q][p] == ter_mountain ||
            m->map[q][p] == ter_boulder)
        {
          r++;
        }
        else if (m->map[q][p] == ter_forest ||
                 m->map[q][p] == ter_boulder)
        {
          t++;
        }
      }
    }
  }

  if (t == r)
  {
    return rand() & 1 ? ter_boulder : ter_boulder;
  }
  else if (t > r)
  {
    if (rand() % 10)
    {
      return ter_boulder;
    }
    else
    {
      return ter_boulder;
    }
  }
  else
  {
    if (rand() % 10)
    {
      return ter_boulder;
    }
    else
    {
      return ter_boulder;
    }
  }
}

typedef enum __attribute__((__packed__)) trainer_type
{
  hiker_npc, 
  rival_npc
} trainer_type_t;

static int32_t get_hiker_cost(const path_t *path) {
    return world.hiker_cost[path->pos[dim_y]][path->pos[dim_x]];
}

static int32_t get_rival_cost(const path_t *path) {
    return world.rival_cost[path->pos[dim_y]][path->pos[dim_x]];
}

static int map_terrain(map_t *m, int8_t n, int8_t s, int8_t e, int8_t w)
{
  int32_t i, x, y;
  queue_node_t *head, *tail, *tmp;
  int num_grass, num_clearing, num_mountain, num_forest, num_water, num_total;
  terrain_type_t type;
  int added_current = 0;

  num_grass = rand() % 4 + 2;
  num_clearing = rand() % 4 + 2;
  num_mountain = rand() % 2 + 1;
  num_forest = rand() % 2 + 1;
  num_water = rand() % 2 + 1;
  num_total = num_grass + num_clearing + num_mountain + num_forest + num_water;

  memset(&m->map, 0, sizeof(m->map));

  for (i = 0; i < num_total; i++)
  {
    do
    {
      x = rand() % MAP_X;
      y = rand() % MAP_Y;
    } while (m->map[y][x]);
    if (i == 0)
    {
      type = ter_grass;
    }
    else if (i == num_grass)
    {
      type = ter_clearing;
    }
    else if (i == num_grass + num_clearing)
    {
      type = ter_clearing;
    }
    else if (i == num_grass + num_clearing + num_mountain)
    {
      type = ter_clearing;
    }
    else if (i == num_grass + num_clearing + num_mountain + num_forest)
    {
      type = ter_water;
    }
    m->map[y][x] = type;
    if (i == 0)
    {
      head = tail = malloc(sizeof(*tail)); 
    }
    else
    {
      tail->next = malloc(sizeof(*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

  while (head)
  {
    x = head->x;
    y = head->y;
    i = m->map[y][x];

    if (x - 1 >= 0 && !m->map[y][x - 1])
    {
      if ((rand() % 100) < 80)
      {
        m->map[y][x - 1] = i;
        tail->next = malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x - 1;
        tail->y = y;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = i;
        tail->next = malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (y - 1 >= 0 && !m->map[y - 1][x])
    {
      if ((rand() % 100) < 20)
      {
        m->map[y - 1][x] = i;
        tail->next = malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y - 1;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = i;
        tail->next = malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (y + 1 < MAP_Y && !m->map[y + 1][x])
    {
      if ((rand() % 100) < 20)
      {
        m->map[y + 1][x] = i;
        tail->next = malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y + 1;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = i;
        tail->next = malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (x + 1 < MAP_X && !m->map[y][x + 1])
    {
      if ((rand() % 100) < 80)
      {
        m->map[y][x + 1] = i;
        tail->next = malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x + 1;
        tail->y = y;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = i;
        tail->next = malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    added_current = 0;
    tmp = head;
    head = head->next;
    free(tmp);
  }

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      if (y == 0 || y == MAP_Y - 1 ||
          x == 0 || x == MAP_X - 1)
      {
        mapxy(x, y) = border_type(m, x, y);
      }
    }
  }

  m->n = n;
  m->s = s;
  m->e = e;
  m->w = w;

  if (n != -1)
  {
    mapxy(n, 0) = ter_gate;
    mapxy(n, 1) = ter_gate;
  }
  if (s != -1)
  {
    mapxy(s, MAP_Y - 1) = ter_gate;
    mapxy(s, MAP_Y - 2) = ter_gate;
  }
  if (w != -1)
  {
    mapxy(0, w) = ter_gate;
    mapxy(1, w) = ter_gate;
  }
  if (e != -1)
  {
    mapxy(MAP_X - 1, e) = ter_gate;
    mapxy(MAP_X - 2, e) = ter_gate;
  }

  return 0;
}

static int32_t compare_costs(int32_t cost1, int32_t cost2) {
    return cost1 - cost2;
}

void place_pc() {
    int posX, posY;

    int is_valid_position(int generatedX, int generatedY) {
        return world.cur_map->map[generatedY][generatedX] == ter_path;
    }

    do {
        posX = rand() % (MAP_X - 2) + 1;
        posY = rand() % (MAP_Y - 2) + 1;
    } while (!is_valid_position(posX, posY));

    world.pc.pos[dim_x] = posX;
    world.pc.pos[dim_y] = posY;
}

static int place_boulders(map_t *m)
{
  int i;
  int x, y;

  for (i = 0; i < MIN_BOULDERS || rand() % 100 < BOULDER_PROB; i++)
  {
    y = rand() % (MAP_Y - 2) + 1;
    x = rand() % (MAP_X - 2) + 1;
    if (m->map[y][x] != ter_forest && m->map[y][x] != ter_path)
    {
      m->map[y][x] = ter_boulder;
    }
  }

  return 0;
}

static int32_t priority_rival(const void *key, const void *with) {
    const path_t *path1 = (const path_t *)key;
    const path_t *path2 = (const path_t *)with;
    
    return compare_costs(get_rival_cost(path1), get_rival_cost(path2));
}

static int place_trees(map_t *m)
{
  int i;
  int x, y;

  for (i = 0; i < MIN_TREES || rand() % 100 < TREE_PROB; i++)
  {
    y = rand() % (MAP_Y - 2) + 1;
    x = rand() % (MAP_X - 2) + 1;
    if (m->map[y][x] != ter_mountain &&
        m->map[y][x] != ter_path &&
        m->map[y][x] != ter_water)
    {
      m->map[y][x] = ter_tree;
    }
  }

  return 0;
}

static int32_t priority_hiker(const void *key, const void *with) {
    const path_t *path1 = (const path_t *)key;
    const path_t *path2 = (const path_t *)with;
    
    return compare_costs(get_hiker_cost(path1), get_hiker_cost(path2));
}

int32_t movement_cost_chart[5][11] = {
    {IMPASSIBLE, IMPASSIBLE, FASTER, FASTER, FASTER, SLOW, FASTER, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, FASTER},
    {IMPASSIBLE, IMPASSIBLE, FASTER, SLOWEST, SLOWEST, FAST, FASTER, FAST, FAST, IMPASSIBLE, IMPASSIBLE},
    {IMPASSIBLE, IMPASSIBLE, FASTER, SLOWEST, SLOWEST, SLOW, FASTER, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE},
    {IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, FASTEST, IMPASSIBLE},
    {IMPASSIBLE, IMPASSIBLE, FASTER, SLOWEST, SLOWEST, SLOW, FASTER, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE, IMPASSIBLE}
};

static inline int get_movement_cost(int x, int y, trainer_type_t c, map_t *m) {
    return movement_cost_chart[c][m->map[y][x]];
}

void initialize_path(path_t p[][MAP_X]) {
    uint32_t x, y;
    for (y = 0; y < MAP_Y; y++) {
        for (x = 0; x < MAP_X; x++) {
            p[y][x].pos[dim_y] = y;
            p[y][x].pos[dim_x] = x;
        }
    }
}

void set_initial_costs() {
    uint32_t x, y;
    for (y = 0; y < MAP_Y; y++) {
        for (x = 0; x < MAP_X; x++) {
            world.hiker_cost[y][x] = world.rival_cost[y][x] = INT_MAX;
        }
    }
    world.hiker_cost[world.pc.pos[dim_y]][world.pc.pos[dim_x]] =
    world.rival_cost[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = 0;
}

void insert_into_heap(heap_t *h, path_t p[][MAP_X], trainer_type_t npc_type, map_t *m) {
    uint32_t x, y;
    for (y = 1; y < MAP_Y - 1; y++) {
        for (x = 1; x < MAP_X - 1; x++) {
            if (get_movement_cost(x, y, npc_type, m) != INT_MAX) {
                p[y][x].hn = heap_insert(h, &p[y][x]);
            } else {
                p[y][x].hn = NULL;
            }
        }
    }
}

void update_cost(int x, int y, int dx, int dy, int cost, path_t p[][MAP_X], int (*world_cost)[MAP_X], heap_t *h, trainer_type_t npc, map_t *m) {
    int nx = x + dx;
    int ny = y + dy;

    if (p[ny][nx].hn && world_cost[ny][nx] > cost + get_movement_cost(nx, ny, npc, m)) {
        world_cost[ny][nx] = cost + get_movement_cost(nx, ny, npc, m);
        heap_decrease_key_no_replace(h, p[ny][nx].hn);
    }
}

void update_neighbors_costs(int current_x, int current_y, int cost, path_t p[][MAP_X], int (*world_cost)[MAP_X], heap_t *h, trainer_type_t npc_type, map_t *m) {
    int deltas[3] = {-1, 0, 1};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i != 1 || j != 1) 
                update_cost(current_x, current_y, deltas[i], deltas[j], cost, p, world_cost, h, npc_type, m);
        }
    }
}

void calc_path_costs(map_t *game_map) {
    heap_t heap_instance;
    static path_t path_array[MAP_Y][MAP_X], *current_path;
    static uint32_t is_initialized = 0;

    if (!is_initialized) {
        is_initialized = 1;
        initialize_path(path_array);
    }

    set_initial_costs();

    for (trainer_type_t trainer_kind = hiker_npc; trainer_kind <= rival_npc; trainer_kind++) {
        heap_init(&heap_instance, trainer_kind == rival_npc ? priority_rival : priority_hiker, NULL);
        int (*path_cost_matrix)[MAP_X] = trainer_kind == rival_npc ? world.rival_cost : world.hiker_cost;

        insert_into_heap(&heap_instance, path_array, trainer_kind, game_map);

        while ((current_path = heap_remove_min(&heap_instance))) {
            int path_pos_x = current_path->pos[dim_x];
            int path_pos_y = current_path->pos[dim_y];
            int current_cost = path_cost_matrix[path_pos_y][path_pos_x];
            current_path->hn = NULL;

            update_neighbors_costs(path_pos_x, path_pos_y, current_cost, path_array, path_cost_matrix, &heap_instance, trainer_kind, game_map);
        }

        heap_delete(&heap_instance);
    }
}

static int new_map()
{
  int d, p;
  int e, w, n, s;

  if (world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]])
  {
    world.cur_map = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]];
    return 0;
  }

  world.cur_map =
      world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]] =
          malloc(sizeof(*world.cur_map));

  smooth_height(world.cur_map);

  if (!world.cur_idx[dim_y])
  {
    n = -1;
  }
  else if (world.world[world.cur_idx[dim_y] - 1][world.cur_idx[dim_x]])
  {
    n = world.world[world.cur_idx[dim_y] - 1][world.cur_idx[dim_x]]->s;
  }
  else
  {
    n = 1 + rand() % (MAP_X - 2);
  }
  if (world.cur_idx[dim_y] == WORLD_SIZE - 1)
  {
    s = -1;
  }
  else if (world.world[world.cur_idx[dim_y] + 1][world.cur_idx[dim_x]])
  {
    s = world.world[world.cur_idx[dim_y] + 1][world.cur_idx[dim_x]]->n;
  }
  else
  {
    s = 1 + rand() % (MAP_X - 2);
  }
  if (!world.cur_idx[dim_x])
  {
    w = -1;
  }
  else if (world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] - 1])
  {
    w = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] - 1]->e;
  }
  else
  {
    w = 1 + rand() % (MAP_Y - 2);
  }
  if (world.cur_idx[dim_x] == WORLD_SIZE - 1)
  {
    e = -1;
  }
  else if (world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] + 1])
  {
    e = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] + 1]->w;
  }
  else
  {
    e = 1 + rand() % (MAP_Y - 2);
  }

  map_terrain(world.cur_map, n, s, e, w);

  place_boulders(world.cur_map);
  place_trees(world.cur_map);
  build_paths(world.cur_map);
  d = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
       abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)));
  p = d > 200 ? 5 : (50 - ((45 * d) / 200));
  if ((rand() % 100) < p || !d)
  {
    place_pokemart(world.cur_map);
  }
  if ((rand() % 100) < p || !d)
  {
    place_center(world.cur_map);
  }

  return 0;
}

void print_cost_matrix(int cost_matrix[MAP_Y][MAP_X]) {
    int row, col;

    for (row = 0; row < MAP_Y; row++) {
        for (col = 0; col < MAP_X; col++) {
            if (cost_matrix[row][col] == INT_MAX || cost_matrix[row][col] < 0) {
                printf("    ");
            } else {
                printf(" %02d", cost_matrix[row][col] % 100);
            }
        }
        printf("\n");
    }
}

void display_hiker_costs() {
    print_cost_matrix(world.hiker_cost);
}

void display_rival_costs() {
    print_cost_matrix(world.rival_cost);
}

static void print_map()
{
  int x, y;
  int default_reached = 0;

  printf("\n");

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      if (world.pc.pos[dim_y] == y &&
          world.pc.pos[dim_x] == x)
      {
        printf("\033[1;35;103m@\033[0m"); // Yellow Background, Pink @
      }
      else
      {
        switch (world.cur_map->map[y][x])
        {
        case ter_mountain:
        case ter_boulder:
          printf("\033[1;31m%%\033[0m"); // Bright Red
          break;
        case ter_tree:
        case ter_forest:
            printf("\033[1;37m^\033[0m"); // White
            break;
        case ter_path:
        case ter_gate:
          printf("\033[1;33m#\033[0m"); // Bright Yellow
          break;
        case ter_mart:
          printf("\033[1;35mM\033[0m"); // Magenta
          break;
        case ter_center:
          printf("\033[0;34mC\033[0m"); // Dark Blue
          break;
        case ter_grass:
          printf("\033[1;32m:\033[0m"); // Bright Green
          break;
        case ter_clearing:
        printf("\033[0;33m.\033[0m"); // Dark Brown (using Yellow as it often appears brownish)
        break;
        case ter_water:
          printf("\033[1;36m~\033[0m"); // Light Blue (Bright Cyan)
          break;
        default:
          default_reached = 1;
          break;
        }
      }
    }
    putchar('\n');
  }

  if (default_reached)
  {
    fprintf(stderr, "Default reached in %s\n", __FUNCTION__);
  }
}

void init_world()
{
  world.cur_idx[dim_x] = world.cur_idx[dim_y] = WORLD_SIZE / 2;
  new_map();
}

void delete_world()
{
  int x, y;

  for (y = 0; y < WORLD_SIZE; y++)
  {
    for (x = 0; x < WORLD_SIZE; x++)
    {
      if (world.world[y][x])
      {
        free(world.world[y][x]);
        world.world[y][x] = NULL;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  struct timeval tv;
  uint32_t seed;
  char c;
  int x, y;

  if (argc == 2)
  {
    seed = atoi(argv[1]);
  }
  else
  {
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }

  srand(seed);
  init_world();
  place_pc();
  
  calc_path_costs(world.cur_map);
  print_map();

  printf("\nHiker Cost\n");
  display_hiker_costs();
  printf("\nRival Cost\n");
  display_rival_costs();
  printf("Welcome to Pokemon!\n");

  do
  {
        printf("You are at %d%cx%d%c (%d,%d).  \n"
           "Enter a command to travel the world (n, s, e, w, f x y, q): ",
           abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_x] - (WORLD_SIZE / 2) >= 0 ? 'E' : 'W',
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2) <= 0 ? 'N' : 'S',
           world.cur_idx[dim_x] - (WORLD_SIZE / 2),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2));
    if (scanf(" %c", &c) != 1)
    {
      putchar('\n');
      break;
    }
    switch (c)
    {
    case 'n':
      if (world.cur_idx[dim_y])
      {
        world.cur_idx[dim_y]--;
        new_map();
        place_pc();
        calc_path_costs(world.cur_map);

        print_map();

        printf("\nHiker Cost\n");
        display_hiker_costs();
        printf("\nRival Cost\n");
        display_rival_costs();
      }
      break;
    case 's':
      if (world.cur_idx[dim_y] < WORLD_SIZE - 1)
      {
        world.cur_idx[dim_y]++;
        new_map();
        place_pc();
        calc_path_costs(world.cur_map);

        print_map();

        printf("\nHiker Cost\n");
        display_hiker_costs();
        printf("\nRival Cost\n");
        display_rival_costs();
      }
      break;
    case 'e':
      if (world.cur_idx[dim_x] < WORLD_SIZE - 1)
      {
        world.cur_idx[dim_x]++;
        new_map();
        place_pc();
        calc_path_costs(world.cur_map);

        print_map();

        printf("\nHiker Cost\n");
        display_hiker_costs();
        printf("\nRival Cost\n");
        display_rival_costs();
      }
      break;
    case 'w':
      if (world.cur_idx[dim_x])
      {
        world.cur_idx[dim_x]--;
        new_map();
        place_pc();
        calc_path_costs(world.cur_map);

        print_map();

        printf("\nHiker Cost\n");
        display_hiker_costs();
        printf("\nRival Cost\n");
        display_rival_costs();
      }
      break;
    case 'q':
      break;
    case 'f':
      scanf(" %d %d", &x, &y);
      if (x >= -(WORLD_SIZE / 2) && x <= WORLD_SIZE / 2 &&
          y >= -(WORLD_SIZE / 2) && y <= WORLD_SIZE / 2)
      {
        world.cur_idx[dim_x] = x + (WORLD_SIZE / 2);
        world.cur_idx[dim_y] = y + (WORLD_SIZE / 2);
        new_map();
        place_pc();
        calc_path_costs(world.cur_map);
        print_map();

        printf("\nHiker Cost\n");
        display_hiker_costs();
        printf("\nRival Cost\n");
        display_rival_costs();
      }
      break;
    case '?':
    case 'h':
      printf("Move with 'e'ast, 'w'est, 'n'orth, 's'outh or 'f'ly x y.\n"
             "Quit with 'q'.  '?' and 'h' print this help message.\n");
      break;
    default:
      fprintf(stderr, "%c: Invalid input.  Enter '?' for help.\n", c);
      break;
    }
  } while (c != 'q');

  delete_world();

  printf("Thank you for playing.\nNow Exiting...\n");

  return 0;
}

