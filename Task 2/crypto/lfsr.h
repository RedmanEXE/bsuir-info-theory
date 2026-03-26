//
// Created by REXE on 22.03.26.
//

#ifndef INFO_THEORY_LFSR_H
#define INFO_THEORY_LFSR_H

#include <stdint.h>
#include <stdlib.h>

// Some params
// Don't forget change to your own ;)
#define REGISTER_LENGTH         36
#define REGISTER_MASK           0xFFFFFFFFFULL // aka 1 << 35 | 1 << 34 | ... | 1 << 0

typedef struct
{
    uint64_t reg;
} Crypto_LFSRAlgorithm;

Crypto_LFSRAlgorithm* Crypto_LFSRAlgorithm_Create()
{
    Crypto_LFSRAlgorithm *data = (Crypto_LFSRAlgorithm*)calloc(1, sizeof(Crypto_LFSRAlgorithm));
    return data;
}

void Crypto_LFSRAlgorithm_SetRegister(Crypto_LFSRAlgorithm *manager, uint64_t reg)
{
    manager->reg = reg & REGISTER_MASK;
}

uint8_t Crypto_LFSRAlgorithm_GenerateByte(Crypto_LFSRAlgorithm *manager)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        // For polynome: x^36 + x^11 + 1
        // Don't forget change to your own ;)
        uint8_t new_bit = ((manager->reg >> (REGISTER_LENGTH - 1)) & 1) ^ ((manager->reg >> 10) & 1);
        manager->reg = ((manager->reg << 1) | new_bit);
    }

    uint8_t out = (manager->reg >> (REGISTER_LENGTH - 1)) & 0xFF;
    manager->reg &= REGISTER_MASK;

    return out;
}

#endif //INFO_THEORY_LFSR_H
