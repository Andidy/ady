#pragma once


// Example Allocator to act as source of memory for QueueNodes ================
// I based this allocator on the one shown at:
// https://www.gingerbill.org/article/2019/02/16/memory-allocation-strategies-004/
// I turned the functions into methods and changed the init code to fit my use
// case.

#include <Windows.h>

struct PoolFreeNode {
  PoolFreeNode* next;
};

struct Pool {
  unsigned char* buffer;
  long long buf_size;
  int chunk_size; // in bytes

  PoolFreeNode* free_list_head;

  Pool(int num_chunks, int chunk_size_in_bytes) {
    this->chunk_size = chunk_size_in_bytes;
    buf_size = (long long)chunk_size_in_bytes * (long long)num_chunks;
    buffer = (unsigned char*)VirtualAlloc(0, buf_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    free_list_head = NULL;

    FreeAll();
  }

  void* Allocate() {
    PoolFreeNode* chunk = free_list_head;

    if (chunk == NULL) {
      // no free chunk to allocate
      return NULL;
    }

    free_list_head = free_list_head->next;
    return memset(chunk, 0, chunk_size);
  }

  void Free(void* ptr) {
    PoolFreeNode* node;

    void* start = buffer;
    void* end = &buffer[buf_size];

    if (ptr == NULL) {
      // ignore null pointers
      return;
    }

    if (!(start <= ptr && ptr < end)) {
      // memory out of the bounds of the pool
      return;
    }

    node = (PoolFreeNode*)ptr;
    node->next = free_list_head;
    free_list_head = node;
  }

  void FreeAll() {
    long long num_chunks = buf_size / chunk_size;
    for (int i = 0; i < num_chunks; i++) {
      void* ptr = &buffer[i * chunk_size];

      // push node onto the free list
      PoolFreeNode* node = (PoolFreeNode*)ptr;
      node->next = free_list_head;
      free_list_head = node;
    }
  }
};

// ============================================================================
// Expanding Queue ============================================================

struct ExpandingQueueNode {
  ExpandingQueueNode* next;
  static const int num_slots = 4;
  int slot[num_slots];
};

struct ExpandingQueue {
  ExpandingQueueNode* front;
  int front_offset;

  ExpandingQueueNode* back;
  int back_offset;

  Pool* pool;

  ExpandingQueue(Pool* pool) {
    this->pool = pool;
    front = (ExpandingQueueNode*)pool->Allocate();
    front_offset = 0;
    back = front;
    back_offset = 0;
  }

  bool IsEmpty() {
    return ((front == back) && (front_offset == back_offset));
  }

  void Enqueue(int item) {
    back->slot[back_offset] = item;
    back_offset += 1;
    if (back_offset == back->num_slots) {
      // allocate a new queue node from pool allocator
      ExpandingQueueNode* temp = (ExpandingQueueNode*)pool->Allocate();

      back_offset = 0;
      back->next = temp;
      back = temp;
    }
  }

  int Dequeue() {
    if (IsEmpty()) {
      return -1;
    }
    int temp = front->slot[front_offset];
    front_offset += 1;
    if (front_offset == front->num_slots) {
      front_offset = 0;

      // free the queue node front points to;
      ExpandingQueueNode* temp_ptr = front->next;
      pool->Free(front);
      front = temp_ptr;
    }
    return temp;
  }
};

// ============================================================================
// Expanding Priority Queue (uses same Pool allocator) ========================

struct Item {
  int item = 0;
  float priority = 0.0f;
};

struct ExpandingPriorityQueueNode {
  ExpandingPriorityQueueNode* next;
  static const int num_slots = 4;
  Item slot[num_slots];
};

struct ExpandingPriorityQueue {
  ExpandingPriorityQueueNode* front;
  int front_offset;

  ExpandingPriorityQueueNode* back;
  int back_offset;

  ExpandingPriorityQueueNode* insert;
  int insert_offset;

  Pool* pool;

  ExpandingPriorityQueue(Pool* pool) {
    this->pool = pool;
    front = (ExpandingPriorityQueueNode*)pool->Allocate();
    front_offset = 0;
    back = front;
    back_offset = 0;
    insert = front;
    insert_offset = 0;
  }

  bool IsEmpty() {
    return ((front == back) && (front_offset == back_offset));
  }

  void Enqueue(int value, float priority) {
    Item item = { value, priority };

    insert = front;
    insert_offset = front_offset;

    if (!IsEmpty()) {
      while (!((insert == back) && (insert_offset == back_offset))) {
        if (item.priority < insert->slot[insert_offset].priority) {
          // Found location of enqueue'd item

          // put enqueue'd item in place, pick up old item to move back 
          Item temp = insert->slot[insert_offset];
          insert->slot[insert_offset] = item;
          item = temp;
        }

        insert_offset += 1;
        if (insert_offset == insert->num_slots) {
          insert_offset = 0;
          insert = insert->next;
        }
      }
    }

    // reached last element, add as if its a regular expanding queue
    back->slot[back_offset] = item;
    back_offset += 1;
    if (back_offset == back->num_slots) {
      // allocate a new queue node from pool allocator
      ExpandingPriorityQueueNode* temp = (ExpandingPriorityQueueNode*)pool->Allocate();

      back_offset = 0;
      back->next = temp;
      back = temp;
    }

    // possibly redundant?
    // insert = front;
    // insert_offset = front_offset;
  }

  Item Dequeue() {
    if (IsEmpty()) {
      return { -1, -1.0f };
    }

    Item temp = front->slot[front_offset];
    front_offset += 1;
    if (front_offset == front->num_slots) {
      front_offset = 0;

      // free the queue node front points to;
      ExpandingPriorityQueueNode* temp_ptr = front->next;
      pool->Free(front);
      front = temp_ptr;
    }
    return temp;
  }
};

// ============================================================================