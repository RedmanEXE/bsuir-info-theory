//
// Created by REXE on 12.02.26.
//

#ifndef INFO_THEORY_ROTATION_MATRIX_H
#define INFO_THEORY_ROTATION_MATRIX_H

#include <stdlib.h>
#include <wchar.h>

typedef struct
{
    int key_length;
    wchar_t *process_key;
} Crypto_VigenereAlgorithm;

static const wchar_t *ALPHABET_UPPER = L"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
static const wchar_t *ALPHABET_LOWER = L"абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
static const int ALPHA_LEN = 33;

static int get_char_index(wchar_t c) {
    const wchar_t *ptr;

    ptr = wcschr(ALPHABET_LOWER, c);
    if (ptr)
        return ptr - ALPHABET_LOWER;

    ptr = wcschr(ALPHABET_UPPER, c);
    if (ptr)
        return ptr - ALPHABET_UPPER;

    return -1;
}

static void generate_progressive_key(Crypto_VigenereAlgorithm* manager, int text_len, const wchar_t* base_key) {
    if (manager->process_key != NULL)
        free(manager->process_key);

    manager->process_key = (wchar_t *)malloc((text_len + 1) * sizeof(wchar_t));
    int base_key_len = wcslen(base_key);

    for (int i = 0; i < text_len; i++) {
        wchar_t base_char = base_key[i % base_key_len];
        int base_idx = get_char_index(base_char);

        if (base_idx == -1) {
            manager->process_key[i] = base_char;
            continue;
        }

        int round_shift = i / base_key_len;
        int new_key_idx = (base_idx + round_shift) % ALPHA_LEN;

        manager->process_key[i] = ALPHABET_UPPER[new_key_idx];
    }
    manager->process_key[text_len] = L'\0';
}

Crypto_VigenereAlgorithm* Crypto_VigenereAlgorithm_Create()
{
    Crypto_VigenereAlgorithm* matrix = (Crypto_VigenereAlgorithm*)calloc(1, sizeof(Crypto_VigenereAlgorithm));

    matrix->key_length = 0;
    matrix->process_key = NULL;

    return matrix;
}
void Crypto_VigenereAlgorithm_Free(Crypto_VigenereAlgorithm** matrix)
{
    if ((*matrix)->process_key != NULL)
        free((*matrix)->process_key);
    free(*matrix);

    *matrix = NULL;
}

int Crypto_VigenereAlgorithm_Decode(
    Crypto_VigenereAlgorithm* manager,
    const int len, const wchar_t* string,
    const int key_len, const wchar_t* key,
    wchar_t* out)
{
    generate_progressive_key(manager, len, key);

    for (int i = 0; i < len; i++) {
        int text_idx = get_char_index(string[i]);

        if (text_idx == -1) {
            out[i] = string[i];
            continue;
        }

        int key_idx = get_char_index(manager->process_key[i]);
        if (key_idx == -1) key_idx = 0;

        int decoded_idx = (text_idx - key_idx + ALPHA_LEN) % ALPHA_LEN;
        out[i] = ALPHABET_UPPER[decoded_idx];
    }
    out[len] = L'\0';
    return 0;
}
int Crypto_VigenereAlgorithm_Encode(
    Crypto_VigenereAlgorithm* manager,
    const int len, const wchar_t* string,
    const int key_len, const wchar_t* key,
    wchar_t* out)
{
    generate_progressive_key(manager, len, key);

    for (int i = 0; i < len; i++) {
        int text_idx = get_char_index(string[i]);
        int key_idx = get_char_index(manager->process_key[i]);

        int encoded_idx = (text_idx + key_idx) % ALPHA_LEN;
        out[i] = ALPHABET_UPPER[encoded_idx];
    }
    out[len] = L'\0';
    return 0;
}

#endif //INFO_THEORY_ROTATION_MATRIX_H
