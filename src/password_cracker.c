#define _GNU_SOURCE
#include <assert.h>
#include <crypt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary_words.h"
#include "thread_pool.h"

const char HASH_START[] = "$6$";
const size_t SALT_LENGTH = 20;
const size_t HASH_LENGTH = 106;
const size_t NUM_THREADS = 16;

static size_t hash_count = 0;
static char **hashes = NULL;

static inline bool hashes_match(const char *password, const char *hash) {
    char salt[SALT_LENGTH + 1];
    memcpy(salt, hash, sizeof(char[SALT_LENGTH]));
    salt[SALT_LENGTH] = '\0';
    struct crypt_data data;
    memset(&data, 0, sizeof(data));
    char *hashed = crypt_r(password, salt, &data);
    char *hashed_hash = &hashed[SALT_LENGTH];
    const char *hash_hash = &hash[SALT_LENGTH];
    return memcmp(hashed_hash, hash_hash, sizeof(char[HASH_LENGTH - SALT_LENGTH])) == 0;
}

void try_variants(void *word){
    char *dict_word = (char *)word;
    size_t str_len = strlen(dict_word);
    for(size_t i = 0; i <= str_len; i++){
        char *attempt = malloc(sizeof(char) * (str_len + 1));
        strncpy(attempt, dict_word, i);
        strncpy(attempt + i + 1, dict_word + i, str_len - i);
        for(char num = '0'; num <= '9'; num++){
            attempt[i] = num;
            for(size_t hash_idx = 0; hash_idx < hash_count; hash_idx++){
                if(hashes_match(attempt, hashes[hash_idx])){
                    printf("%s\n", attempt);
                }
            } 
        }
        free(attempt);
    }

}

int main(void) {
    // Read in the hashes from the standard input
    char *line = NULL;
    size_t line_capacity = 0;
    // Stop when the end of the input or an empty line is reached
    while (getline(&line, &line_capacity, stdin) > 0 && line[0] != '\n') {
        // Check that the line looks like a hash
        size_t line_length = strlen(line);
        assert(
            line_length == HASH_LENGTH ||
            (line_length == HASH_LENGTH + 1 && line[HASH_LENGTH] == '\n')
        );
        assert(memcmp(line, HASH_START, sizeof(HASH_START) - sizeof(char)) == 0);

        // Extend the hashes array and add the hash to it
        hashes = realloc(hashes, sizeof(char * [hash_count + 1]));
        assert(hashes != NULL);
        char *hash = malloc(sizeof(char[HASH_LENGTH + 1]));
        assert(hash != NULL);
        memcpy(hash, line, sizeof(char[HASH_LENGTH]));
        hash[HASH_LENGTH] = '\0';
        hashes[hash_count++] = hash;
    }
    free(line);

    thread_pool_t *pool = thread_pool_init(NUM_THREADS);
    for(size_t i = 0; i < NUM_DICTIONARY_WORDS; i++){
        thread_pool_add_work(pool, try_variants, (void *) DICTIONARY[i]);
    }
    thread_pool_finish(pool);
    
}
