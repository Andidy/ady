#pragma once

// Fixed Sized HashTable ======================================================
// Int key, Int value
// developed based on hashtable with chaining, replacing linked list with fixed
// sized arrays

struct HashTable {
  struct KeyValue {
    int key;
    int value;
  };
  
  static const int data_len = 1024;
  static const int num_buckets = 64;
  static const int bucket_size = 16;
  KeyValue data[data_len];
  KeyValue empty;

  HashTable() {
    empty = { -2147483648, -2147483648 }; // 32 bit integer minimum
    Clear();
  }

  // 32bit mix function from here: http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
  int Hash(int key) {
    key = ~key + (key << 15); // key = (key << 15) - key - 1;
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057; // key = (key + (key << 3)) + (key << 11);
    key = key ^ (key >> 16);
    return key;
  }

  void Clear() {
    for (int i = 0; i < data_len; i++) {
      data[i] = empty;
    }
  }

  void Insert(int key, int value) {
    int hashcode = Hash(key);
    hashcode = hashcode % num_buckets;

    for (int i = 0; i < bucket_size; i++) {
      int index = hashcode + i * num_buckets;
      if ((data[index].key == empty.key) && (data[index].value == empty.value)) {
        data[index] = { key, value };
        return;
      }
      else if (data[index].key == key) {
        data[index].value = value;
        return;
      }
    }

    // error
    std::cout << "error in hash table insert\n";
  }

  int Lookup(int key) {
    int hashcode = Hash(key);
    hashcode = hashcode % num_buckets;

    for (int i = 0; i < bucket_size; i++) {
      int index = hashcode + i * num_buckets;
      if (data[index].key == key) {
        return data[index].value;
      }
    }

    // failed to find the key's value, return error value
    return empty.value;
  }
};

// ============================================================================
// Fixed Sized HashTable ======================================================
// Int key, Float value

struct HashTableF {
  struct KeyValue {
    int key;
    float value;
  };

  static const int data_len = 1024;
  static const int num_buckets = 64;
  static const int bucket_size = 16;
  KeyValue data[data_len];
  KeyValue empty;

  HashTableF() {
    empty = { -2147483648, -2147483648.0f }; // 32 bit integer minimum
    Clear();
  }

  // 32bit mix function from here: http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
  int Hash(int key) {
    key = ~key + (key << 15); // key = (key << 15) - key - 1;
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057; // key = (key + (key << 3)) + (key << 11);
    key = key ^ (key >> 16);
    return key;
  }

  void Clear() {
    for (int i = 0; i < data_len; i++) {
      data[i] = empty;
    }
  }

  void Insert(int key, int value) {
    int hashcode = Hash(key);
    hashcode = hashcode % num_buckets;

    for (int i = 0; i < bucket_size; i++) {
      int index = hashcode + i * num_buckets;
      if ((data[index].key == empty.key) && (data[index].value == empty.value)) {
        data[index] = { key, value };
        return;
      }
      else if (data[index].key == key) {
        data[index].value = value;
        return;
      }
    }

    // error
    std::cout << "error in hash table insert\n";
  }

  float Lookup(int key) {
    int hashcode = Hash(key);
    hashcode = hashcode % num_buckets;

    for (int i = 0; i < bucket_size; i++) {
      int index = hashcode + i * num_buckets;
      if (data[index].key == key) {
        return data[index].value;
      }
    }

    // failed to find the key's value, return error value
    return empty.value;
  }
};