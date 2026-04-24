//
// Created by REXE on 23.04.26.
//

#ifndef INFO_THEORY_RABIN_H
#define INFO_THEORY_RABIN_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    int64_t p;
    int64_t q;
    int64_t n;
    int64_t b;

    int64_t yp;
    int64_t yq;
} Crypto_RabinAlgorithm;

int Crypto_RabinAlgorithm_IsPrime(int64_t n)
{
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (int64_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

int64_t Crypto_RabinAlgorithm_Mod(int64_t a, int64_t n)
{
    int64_t r = a % n;
    return r < 0 ? r + n : r;
}

void Crypto_RabinAlgorithm_ExtGCD(int64_t a, int64_t b, int64_t *x, int64_t *y)
{
    int64_t x0 = 1, y0 = 0, x1 = 0, y1 = 1;
    while (b > 0) {
        int64_t q = a / b;
        int64_t a2 = a % b;
        a = b;
        b = a2;

        int64_t x2 = x0 - q * x1;
        int64_t y2 = y0 - q * y1;

        x0 = x1; x1 = x2;
        y0 = y1; y1 = y2;
    }
    *x = x0;
    *y = y0;
}

int64_t Crypto_RabinAlgorithm_ModExp(int64_t base, int64_t exp, int64_t mod)
{
    int64_t res = 1;
    base = Crypto_RabinAlgorithm_Mod(base, mod);
    while (exp > 0)
    {
        if (exp % 2 == 1)
            res = (int64_t)(((unsigned __int128)res * base) % (unsigned __int128)mod);

        base = (int64_t)(((unsigned __int128)base * base) % (unsigned __int128)mod);
        exp /= 2;
    }
    return res;
}

int Crypto_RabinAlgorithm_CheckRabinCondition(int64_t p)
{
    return (p % 4 == 3);
}

Crypto_RabinAlgorithm *Crypto_RabinAlgorithm_Create()
{
    Crypto_RabinAlgorithm *data = (Crypto_RabinAlgorithm*)calloc(1, sizeof(Crypto_RabinAlgorithm));
    return data;
}

void Crypto_RabinAlgorithm_SetValues(Crypto_RabinAlgorithm *manager, int64_t p, int64_t q, int64_t b)
{
    if (p % 4 != 3 || q % 4 != 3)
        return;

    manager->p = p;
    manager->q = q;
    manager->n = p * q;
    manager->b = b;

    Crypto_RabinAlgorithm_ExtGCD(manager->p, manager->q, &manager->yp, &manager->yq);
}

void Crypto_RabinAlgorithm_Destroy(Crypto_RabinAlgorithm *manager)
{
    if (manager)
        free(manager);
}

int64_t Crypto_RabinAlgorithm_EncryptByte(Crypto_RabinAlgorithm *manager, uint8_t m)
{
    // ci = mi*(mi + b) mod n
    int64_t m64 = (int64_t)m;
    int64_t c = Crypto_RabinAlgorithm_Mod(m64 * (m64 + manager->b), manager->n);
    return c;
}

uint8_t Crypto_RabinAlgorithm_DecryptValue(Crypto_RabinAlgorithm *manager, int64_t c)
{
    int64_t p = manager->p;
    int64_t q = manager->q;
    int64_t n = manager->n;
    int64_t b = manager->b;

    // D = b^2 + 4c (mod n)
    int64_t b2 = (int64_t)(((unsigned __int128)b * b) % (unsigned __int128)n);
    int64_t D = Crypto_RabinAlgorithm_Mod(b2 + Crypto_RabinAlgorithm_Mod(4 * c, n), n);

    int64_t mp = Crypto_RabinAlgorithm_ModExp(D, (p + 1) / 4, p);
    int64_t mq = Crypto_RabinAlgorithm_ModExp(D, (q + 1) / 4, q);

    int64_t safe_yp = Crypto_RabinAlgorithm_Mod(manager->yp, n);
    int64_t safe_yq = Crypto_RabinAlgorithm_Mod(manager->yq, n);

    int64_t term1 = (int64_t)((((unsigned __int128)safe_yp * p) % n * mq) % n);
    int64_t term2 = (int64_t)((((unsigned __int128)safe_yq * q) % n * mp) % n);

    int64_t roots_d[4];
    roots_d[0] = Crypto_RabinAlgorithm_Mod(term1 + term2, n);          // d1
    roots_d[1] = Crypto_RabinAlgorithm_Mod(n - roots_d[0], n);         // d2
    roots_d[2] = Crypto_RabinAlgorithm_Mod(term1 - term2, n);          // d3
    roots_d[3] = Crypto_RabinAlgorithm_Mod(n - roots_d[2], n);         // d4

    for (int i = 0; i < 4; i++)
    {
        int64_t m;
        if (Crypto_RabinAlgorithm_Mod(roots_d[i] - b, 2) == 0)
            m = Crypto_RabinAlgorithm_Mod((-b + roots_d[i]) / 2, n);
        else
            m = Crypto_RabinAlgorithm_Mod((-b + n + roots_d[i]) / 2, n);

        if (m >= 0 && m < 256)
            return (uint8_t)m;
    }

    return 0;
}

#endif //INFO_THEORY_RABIN_H
