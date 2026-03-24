//
// Created by REXE on 22.03.26.
//

#ifndef INFO_THEORY_LFSR_H
#define INFO_THEORY_LFSR_H

#include <stdint.h>
#include <stdlib.h>

#define REGISTER_LENGTH         36

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
    manager->reg = reg & 0xFFFFFFFFFULL;
}

uint8_t Crypto_LFSRAlgorithm_GenerateByte(Crypto_LFSRAlgorithm *manager)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t new_bit = ((manager->reg >> (REGISTER_LENGTH - 1)) & 1) ^ ((manager->reg >> 10) & 1);
        manager->reg = ((manager->reg << 1) | new_bit);
    }

    uint8_t out = (manager->reg >> (REGISTER_LENGTH - 1)) & 0xFF;
    manager->reg &= 0xFFFFFFFFFULL;

    return out;
}

#endif //INFO_THEORY_LFSR_H
