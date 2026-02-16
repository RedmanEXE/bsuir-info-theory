//
// Created by REXE on 12.02.26.
//

#ifndef INFO_THEORY_ROTATION_MATRIX_H
#define INFO_THEORY_ROTATION_MATRIX_H

#include <ctype.h>
#include <stdlib.h>

typedef struct
{
    char c;
    uint8_t is_new;
} Crypto_RotationMatrix_StepItem;

typedef struct
{
    int matrix_length;
    char** process_matrix;

    Crypto_RotationMatrix_StepItem** steps[4];
} Crypto_RotationMatrix;

typedef struct {
    int r;
    int c;
} GridPoint;

Crypto_RotationMatrix* Crypto_RotationMatrix_Create(const int length)
{
    Crypto_RotationMatrix* matrix = (Crypto_RotationMatrix *)calloc(1, sizeof(Crypto_RotationMatrix));

    matrix->matrix_length = length;
    matrix->process_matrix = (char **)calloc(length, sizeof(char *));
    for (int i = 0; i < length; i++)
        matrix->process_matrix[i] = (char *)calloc(length, sizeof(char));

    for (int i = 0; i < 4; i++)
    {
        matrix->steps[i] = (Crypto_RotationMatrix_StepItem **)calloc(length, sizeof(Crypto_RotationMatrix_StepItem *));
        for (int j = 0; j < length; j++)
            matrix->steps[i][j] = (Crypto_RotationMatrix_StepItem *)calloc(length, sizeof(Crypto_RotationMatrix_StepItem));
    }

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
    int out_index = 0;

    while (processed < len) {
        for (int r = 0; r < manager->matrix_length; r++)
        {
            for (int c = 0; c < manager->matrix_length; c++)
            {
                if (processed < len)
                    manager->process_matrix[r][c] = (char)toupper(string[processed++]);
                else
                    manager->process_matrix[r][c] = ' ';
            }
        }

        GridPoint points[4] = { {0,0}, {1,3}, {2,2}, {3,1} };

        for (int rot = 0; rot < 4; rot++)
        {
            for (int i = 0; i < 4; i++)
            {
                out[out_index++] = manager->process_matrix[points[i].r][points[i].c];

                for (int step = rot; step < 4; step++)
                {
                    manager->steps[step][points[i].r][points[i].c].c = manager->process_matrix[points[i].r][points[i].c];
                    manager->steps[step][points[i].r][points[i].c].is_new = (step == rot);
                }
            }

            for (int i = 0; i < 4; i++)
            {
                int temp_r = points[i].r;
                points[i].r = points[i].c;
                points[i].c = manager->matrix_length - 1 - temp_r;
            }
        }
    }

    out[out_index] = '\0';
    return 0;
}
int Crypto_RotationMatrix_Encode(Crypto_RotationMatrix* manager, const int len, const char* string, char* out)
{
    int processed = 0;
    int out_index = 0;

    while (processed < len)
    {
        for (int r = 0; r < manager->matrix_length; r++)
            memset(manager->process_matrix[r], ' ', manager->matrix_length);

        GridPoint points[4] = { {0,0}, {1,3}, {2,2}, {3,1} };

        for (int rot = 0; rot < 4; rot++)
        {
            for (int i = 0; i < 4; i++)
            {
                if (processed < len)
                    manager->process_matrix[points[i].r][points[i].c] = (char)toupper(string[processed++]);
                else
                {
                    manager->process_matrix[points[i].r][points[i].c] = 'A' + (processed - len) % ('Z' - 'A');
                    processed++;
                }

                for (int step = rot; step < 4; step++)
                {
                    manager->steps[step][points[i].r][points[i].c].c = manager->process_matrix[points[i].r][points[i].c];
                    manager->steps[step][points[i].r][points[i].c].is_new = (step == rot);
                }
            }

            for (int i = 0; i < 4; i++)
            {
                int temp_r = points[i].r;
                points[i].r = points[i].c;
                points[i].c = manager->matrix_length - 1 - temp_r;
            }
        }

        for (int r = 0; r < manager->matrix_length; r++)
            for (int c = 0; c < manager->matrix_length; c++)
            {
                out[out_index++] = manager->process_matrix[r][c];
            }
    }

    out[out_index] = '\0';
    return 0;
}

#endif //INFO_THEORY_ROTATION_MATRIX_H