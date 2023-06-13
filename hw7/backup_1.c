#include "cashier.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
// #include <math.h>

uint64_t log_2(uint64_t a)
{
  uint64_t count = 0;
  while (a /= 2)
    ++count;
  return count;
}

struct cashier *cashier_init(struct cache_config config) {
  // malloc a new cashier
  struct cashier *cache = (struct cashier *)malloc(sizeof(struct cashier));
  // check null
  if (cache == NULL)
    return NULL;
  // assign cache_config value
  cache->config = config;
  // assign the total size(bytes) of the cache
  cache->size = config.line_size * config.lines;

  // calculate tag, index and byte offsets
  cache->offset_bits = (uint64_t)log_2(config.line_size);
  cache->index_bits = (uint64_t)log_2(config.lines / config.ways);
  cache->tag_bits = config.address_bits - cache->index_bits - cache->offset_bits;

  // assign bit mask
  cache->tag_mask = (1UL << cache->tag_bits) - 1;
  cache->index_mask = ((1UL << cache->index_bits) - 1) << cache->offset_bits;
  cache->offset_mask = (1UL << cache->offset_bits) - 1;

  // malloc the cache line
  cache->lines = (struct cache_line *)malloc(sizeof(struct cache_line) * config.lines);
  // check null
  if (cache->lines == NULL)
  {
    free(cache); // remember to free the cache that we have malloced
    return NULL;
  }

  // assign cache line
  for (uint64_t i = 0; i < config.lines; ++i)
  {
    cache->lines[i].valid = false; // 0
    cache->lines[i].dirty = false; // 0
    cache->lines[i].tag = 0; // 0
    cache->lines[i].last_access = 0; //0
    cache->lines[i].data = (uint8_t *)calloc(config.line_size, sizeof(uint8_t)); // 0
    // check null
    if (cache->lines[i].data == NULL)
    {
      for (uint64_t j = 0; j < i; ++j)
        free(cache->lines[j].data); // free all data
      free(cache->lines);           // free the cache line
      free(cache);                  // free the cache
      return NULL;
    }
  }
  // success
  return cache;
}

void cashier_release(struct cashier *cache) {
  // check write back
  for (uint64_t i = 0; i < cache->config.lines; ++i)
  {
    if (cache->lines[i].dirty && cache->lines[i].valid) // if it is dirty
    {
      before_eviction(i % (cache->config.lines / cache->config.ways), &cache->lines[i]);
      // to find the correct addr
      uint64_t addr = (cache->lines[i].tag << (cache->index_bits + cache->offset_bits)) |
                      ((i % (cache->config.lines / cache->config.ways)) << cache->offset_bits); 
      // write back
      for (uint64_t j = 0; j < cache->config.line_size; ++j)
        mem_write(addr + j, cache->lines[i].data[j]);
    }
  }
  // free all data
  for (uint64_t i = 0; i < cache->config.lines; ++i)
    free(cache->lines[i].data);
  // free cache line && cache
  free(cache->lines);
  free(cache);
}

bool cashier_read(struct cashier *cache, uint64_t addr, uint8_t *byte) {
  // check null
  if (cache == NULL || byte == NULL)
    return false;
  // set tag, index and offset
  uint64_t tag = addr >> (cache->index_bits + cache->offset_bits);
  uint64_t index = (addr & cache->index_mask) >> cache->offset_bits;
  uint64_t offset = addr & cache->offset_mask;
  // find the cache line
  for (uint64_t i = 0; i < cache->config.ways; ++i)
  {
    struct cache_line *tmp = &cache->lines[index + i * (cache->config.lines / cache->config.ways)];
    if (tmp->valid && tmp->tag == tag) // check if the line is matched
    {
      tmp->last_access = get_timestamp();
      *byte = tmp->data[offset];
      return true; // cache hit
    }
  }

  // cache miss
  struct cache_line *evict_line = &cache->lines[index];
  // interate all ways
  for (uint64_t i = 0; i < cache->config.ways; ++i)
  {
    struct cache_line *tmp = &cache->lines[index + i * (cache->config.lines / cache->config.ways)];
    if (!tmp->valid) // this is not valid, then this will be the line to evict
    {
      evict_line = tmp;
      break; // step out of the loop
    }
    else if (tmp->last_access < evict_line->last_access) // if there is one way, and update lru
    {
      evict_line = tmp; // this cache line is lru
    }
  }
  // now we have find the evict cache line
  if (evict_line->dirty && evict_line->valid) // this line is modified, we have to write back
  {
    uint64_t evict_addr = (evict_line->tag << (cache->index_bits + cache->offset_bits)) |
                          (index << cache->offset_bits);
    // before eviction
    before_eviction(index, evict_line);
    // write back
    for (uint64_t j = 0; j < cache->config.line_size; ++j)
        mem_write(evict_addr + j, evict_line->data[j]);
  }
  
  // update the evicted line
  evict_line->dirty = false;
  evict_line->valid = true;
  evict_line->tag = tag;
  evict_line->last_access = get_timestamp(); // update time
  // for (uint64_t j = 0; j < cache->config.line_size; ++j)
  //   evict_line->data[j] = mem_read(addr - offset + j);
  // to store the result in byte
  *byte = evict_line->data[offset];
  return false; // cache miss
}


bool cashier_write(struct cashier *cache, uint64_t addr, uint8_t byte) {
  // check null
  if (cache == NULL)
    return false;
  // set tag, index and offset
  uint64_t tag = addr >> (cache->index_bits + cache->offset_bits);
  uint64_t index = (addr & cache->index_mask) >> cache->offset_bits;
  uint64_t offset = addr & cache->offset_mask;
  // find the cache line
  for (uint64_t i = 0; i < cache->config.ways; ++i)
  {
    struct cache_line *tmp = &cache->lines[index + i * (cache->config.lines / cache->config.ways)];
    if (tmp->valid && tmp->tag == tag) // check if the line is matched
    {
      tmp->last_access = get_timestamp();
      tmp->data[offset] = byte;
      tmp->dirty = true;
      return true; // cache hit
    }
  }

  // cache miss
  struct cache_line *evict_line = &cache->lines[index];
  // interate all ways
  for (uint64_t i = 0; i < cache->config.ways; ++i)
  {
    struct cache_line *tmp = &cache->lines[index + i * (cache->config.lines / cache->config.ways)];
    if (!tmp->valid) // this is not valid, then this will be the line to evict
    {
      evict_line = tmp;
      break; // step out of the loop
    }
    else if (tmp->last_access < evict_line->last_access) // if there is one way, and update lru
    {
      evict_line = tmp; // this cache line is lru
    }
  }
  // now we have find the evict cache line
  if (evict_line->dirty && evict_line->valid) // this line is modified, we have to write back
  {
    uint64_t evict_addr = (evict_line->tag << (cache->index_bits + cache->offset_bits)) |
                          (index << cache->offset_bits);
    // before eviction
    before_eviction(index, evict_line);
    // write back
    for (uint64_t j = 0; j < cache->config.line_size; ++j)
        mem_write(evict_addr + j, evict_line->data[j]);
  }
  // update the evicted line
  evict_line->dirty = false;
  evict_line->valid = true;
  evict_line->tag = tag;
  evict_line->last_access = get_timestamp(); // update time
  // for (uint64_t j = 0; j < cache->config.line_size; ++j)
  //   evict_line->data[j] = mem_read(addr - offset + j);
  // to store the result in byte
  evict_line->data[offset] = byte;
  return false; // cache miss
}
