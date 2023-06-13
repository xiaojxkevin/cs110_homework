/*to include all headers*/
#include "dir.h"
#include "explorer.h"
#include "file.h"
#include "node.h"
/*from std*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//static bool dir_add_sub(struct directory *dirnode, struct node *sub);

struct directory *dir_new(char *name) {
  /* Initialization */
  struct directory *dir = NULL;
  /* Check for null pointer */
  if (!name) {
    return NULL;
  }
  /* Allocate memory */
  dir = calloc(1, sizeof(struct directory));
  dir->capacity = DEFAULT_DIR_SIZE;
  dir->subordinates = calloc(dir->capacity, sizeof(struct node));
  dir->parent = NULL;
  /* Create base node */
  dir->base = node_new(true, name, dir);
  return dir;
}

void dir_release(struct directory *dir) {
  /* Initialization */
  int i = 0;
  if (!dir) {
    return;
  }
  /* Release all the subordiniates */
  for (i = 0; i < dir->size; i++) {
    node_release(dir->subordinates[i]);
  }
  /* Release the resources */
  /* Check if base has already been released. Prevent circular call. */
  if (dir->base) {
    dir->base->inner.dir = NULL;
    node_release(dir->base);
  }
  /* Release data and self. */
  free(dir->subordinates);
  free(dir);
}

struct node *dir_find_node(const struct directory *dir, const char *name) {
  /*Some simple pointer check*/
  if (!dir)
    return NULL;
  bool flag = false;
  int i = 0;
  /*to look through the dir*/
  for (; i != dir->size; ++i)
  {
    if (!strcmp(name, dir->subordinates[i]->name)) /*to compare if it is the right one*/
    {
      flag = true;
      break;
    }
  }
  /*give the result*/
  return flag ? dir->subordinates[i] : NULL;
}



bool dir_add_file(struct directory *dir, int type, char *name) {
  /*Check for pointer */
  if (!dir)
    return false;
  if (dir_find_node(dir, name))
    return false;
  /*To deal with capacity-outbound*/
  if (dir->size == dir->capacity)
  {
    dir->subordinates = realloc(dir->subordinates, 2 * dir->capacity * sizeof(struct node *));
    dir->capacity *= 2;
  }
  /*To start to create the new node and new file*/
  struct file *new_file = file_new(type, name);
  dir->subordinates[dir->size] = new_file->base; /*to point at it*/
  dir->size++;
  /*remember to do give a return*/
  return true;
}

bool dir_add_subdir(struct directory *dir, char *name) {
  /*check for pointer*/
  if (!dir)
    return false;
  if (dir_find_node(dir, name))
    return false;
  /*In case exceed capacity*/
  if (dir->size == dir->capacity)
  {
    dir->subordinates = realloc(dir->subordinates, 2 * dir->capacity * sizeof(struct node *));
    dir->capacity *= 2;
  }
  /*start to create new nodes and new dir*/
  struct directory *new_dir = dir_new(name);
  new_dir->parent = dir;
  dir->subordinates[dir->size] = new_dir->base; /*to point at it*/
  dir->size++;
  /*remember to do give a return*/
  return true;
}

bool dir_delete(struct directory *dir, const char *name) {
  /*check null*/
  if (!dir || !name)
    return false;
  struct node *target = dir_find_node(dir, name);
  if (target == NULL)
    return false;
  /*to store the info*/
  int index = -1;
  for (int i = 0; i != dir->size; ++i) 
  {
    if (dir->subordinates[i] == target) /*to find the target*/
    {
      index = i; 
      break;
    }
  }
  /*start to move the files*/
  if (index == -1)
    return false;
  node_release(target); // remember to free the targer dir
  for (int i = index; i < dir->size - 1; i++)
  {
    dir->subordinates[i] = dir->subordinates[i + 1]; /*to move  one position left*/
  }
  dir->size--;
  /*give a return*/
  return true;
}

