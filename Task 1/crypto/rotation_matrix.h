//
// Created by REXE on 12.02.26.
//

#ifndef INFO_THEORY_ROTATION_MATRIX_H
#define INFO_THEORY_ROTATION_MATRIX_H

#include <stdlib.h>

typedef struct
{
    int matrix_length;
    char** process_matrix;
} Crypto_RotationMatrix;

typedef struct {
    int r;
    int c;
} GridPoint;

Crypto_RotationMatrix* Crypto_RotationMatrix_Create(const int length)
{
    Crypto_RotationMatrix* matrix = (Crypto_RotationMatrix*)calloc(1, sizeof(Crypto_RotationMatrix));

    matrix->matrix_length = length;
    matrix->process_matrix = (char**)calloc(length, sizeof(char*));
    for (int i = 0; i < length; i++)
        matrix->process_matrix[i] = (char*)calloc(length, sizeof(char));

    return matrix;
}
void Crypto_RotationMatrix_Free(Crypto_RotationMatrix** matrix)
{
    for (int i = 0; i < (*matrix)->matrix_length; i++)
        free((*matrix)->process_matrix[i]);
    free((*matrix)->process_matrix);
    free(*matrix);

    *matrix = NULL;
}
int Crypto_RotationMatrix_Decode(Crypto_RotationMatrix* manager, const int len, const char* string, char* out)
{
    int processed = 0;
    int out_idx = 0;

    while (processed < len) {
        for (int r = 0; r < manager->matrix_length; r++) {
            for (int c = 0; c < manager->matrix_length; c++) {
                if (processed < len) {
                    manager->process_matrix[r][c] = string[processed++];
                } else {
                    manager->process_matrix[r][c] = ' ';
                }
            }
        }

        GridPoint points[4] = { {0,0}, {1,3}, {2,2}, {3,1} };

        for (int rot = 0; rot < 4; rot++) {
            for (int i = 0; i < 4; i++) {
                out[out_idx++] = manager->process_matrix[points[i].r][points[i].c];
            }

            for (int i = 0; i < 4; i++) {
                int temp_r = points[i].r;
                points[i].r = points[i].c;
                points[i].c = manager->matrix_length - 1 - temp_r;
            }
        }
    }

    out[out_idx] = '\0';
    return 0;
}
int Crypto_RotationMatrix_Encode(Crypto_RotationMatrix* manager, const int len, const char* string, char* out)
{
    int processed = 0;
    int out_idx = 0;

    while (processed < len) {
        for (int r = 0; r < manager->matrix_length; r++)
            memset(manager->process_matrix[r], ' ', manager->matrix_length);

        GridPoint points[4] = { {0,0}, {1,3}, {2,2}, {3,1} };

        for (int rot = 0; rot < 4; rot++) {
            for (int i = 0; i < 4; i++)
                if (processed < len) {
                    manager->process_matrix[points[i].r][points[i].c] = string[processed++];
                } else {
                    manager->process_matrix[points[i].r][points[i].c] = ' ';
                }

            for (int i = 0; i < 4; i++) {
                int temp_r = points[i].r;
                points[i].r = points[i].c;
                points[i].c = manager->matrix_length - 1 - temp_r;
            }
        }

        for (int r = 0; r < manager->matrix_length; r++)
            for (int c = 0; c < manager->matrix_length; c++)
                out[out_idx++] = manager->process_matrix[r][c];
    }

    out[out_idx] = '\0';
    return 0;
}

#endif //INFO_THEORY_ROTATION_MATRIX_H
