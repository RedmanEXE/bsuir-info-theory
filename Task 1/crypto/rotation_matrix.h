//
// Created by REXE on 12.02.26.
//

#ifndef INFO_THEORY_ROTATION_MATRIX_H
#define INFO_THEORY_ROTATION_MATRIX_H

#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

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

void Crypto_RotationMatrix_CleanUpCachedSteps(Crypto_RotationMatrix* manager)
{
    if (manager == NULL)
        return;

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < manager->matrix_length; j++)
            memset(manager->steps[i][j], ' ', manager->matrix_length);
}

void Crypto_RotationMatrix_Resize(Crypto_RotationMatrix* manager, const int new_len)
{
    if (manager == NULL || manager->matrix_length == new_len)
        return;

    if (manager->process_matrix != NULL)
    {
        for (int i = 0; i < manager->matrix_length; i++)
            free(manager->process_matrix[i]);
        free(manager->process_matrix);
    }

    for (int i = 0; i < 4; i++)
    {
        if (manager->steps[i] != NULL)
        {
            for (int j = 0; j < manager->matrix_length; j++)
                free(manager->steps[i][j]);
            free(manager->steps[i]);
        }
    }

    manager->matrix_length = new_len;
    manager->process_matrix = (char **)calloc(new_len, sizeof(char *));
    for (int i = 0; i < new_len; i++)
        manager->process_matrix[i] = (char *)calloc(new_len, sizeof(char));

    for (int i = 0; i < 4; i++)
    {
        manager->steps[i] = (Crypto_RotationMatrix_StepItem **)calloc(new_len, sizeof(Crypto_RotationMatrix_StepItem *));
        for (int j = 0; j < new_len; j++)
            manager->steps[i][j] = (Crypto_RotationMatrix_StepItem *)calloc(new_len, sizeof(Crypto_RotationMatrix_StepItem));
    }
}

GridPoint* Crypto_RotationMatrix_GenerateBaseHoles(int N, int* out_num_holes)
{
    *out_num_holes = (N * N) / 4;
    GridPoint* points = (GridPoint*)calloc(*out_num_holes, sizeof(GridPoint));

    int idx = 0;
    for (int r = 0; r < (N + 1) / 2; r++)
    {
        for (int c = 0; c < N / 2; c++)
        {
            points[idx].r = r;
            points[idx].c = c;
            idx++;
        }
    }
    return points;
}

Crypto_RotationMatrix* Crypto_RotationMatrix_Create(const int length)
{
    Crypto_RotationMatrix* matrix = (Crypto_RotationMatrix *)calloc(1, sizeof(Crypto_RotationMatrix));

    matrix->matrix_length = 0;
    matrix->process_matrix = NULL;
    for (int i = 0; i < 4; i++)
        matrix->steps[i] = NULL;

    Crypto_RotationMatrix_Resize(matrix, length);
    return matrix;
}

void Crypto_RotationMatrix_Free(Crypto_RotationMatrix** matrix)
{
    if (matrix == NULL || *matrix == NULL) return;

    for (int i = 0; i < (*matrix)->matrix_length; i++)
        free((*matrix)->process_matrix[i]);
    free((*matrix)->process_matrix);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < (*matrix)->matrix_length; j++)
            free((*matrix)->steps[i][j]);
        free((*matrix)->steps[i]);
    }

    free(*matrix);
    *matrix = NULL;
}

int Crypto_RotationMatrix_Decode(Crypto_RotationMatrix* manager, const int len, const char* string, char* out)
{
    if (len <= 0) return -1;

    int N = 2;
    while (N * N < len) N++;

    Crypto_RotationMatrix_Resize(manager, N);
    Crypto_RotationMatrix_CleanUpCachedSteps(manager);

    int processed = 0;
    int out_index = 0;

    int num_base_holes = 0;
    GridPoint* base_points = Crypto_RotationMatrix_GenerateBaseHoles(N, &num_base_holes);

    int has_center = (N % 2 != 0) ? 1 : 0;
    int center_r = N / 2;
    int center_c = N / 2;

    while (processed < len)
    {
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

        GridPoint* current_points = (GridPoint*)malloc(num_base_holes * sizeof(GridPoint));
        memcpy(current_points, base_points, num_base_holes * sizeof(GridPoint));

        for (int rot = 0; rot < 4; rot++)
        {
            int active_count = num_base_holes + ((rot == 0 && has_center) ? 1 : 0);
            GridPoint* active_points = (GridPoint*)malloc(active_count * sizeof(GridPoint));

            memcpy(active_points, current_points, num_base_holes * sizeof(GridPoint));
            if (rot == 0 && has_center)
            {
                active_points[num_base_holes].r = center_r;
                active_points[num_base_holes].c = center_c;
            }

            for (int i = 0; i < active_count; i++)
            {
                out[out_index++] = manager->process_matrix[active_points[i].r][active_points[i].c];

                for (int step = (3 - rot); step < 4; step++)
                {
                    manager->steps[3 - step][active_points[i].r][active_points[i].c].c = manager->process_matrix[active_points[i].r][active_points[i].c];
                    manager->steps[3 - step][active_points[i].r][active_points[i].c].is_new = ((3 - step) == rot);
                }
            }

            for (int i = 0; i < num_base_holes; i++)
            {
                int temp_r = current_points[i].r;
                current_points[i].r = current_points[i].c;
                current_points[i].c = manager->matrix_length - 1 - temp_r;
            }

            free(active_points);
        }
        free(current_points);
    }

    out[out_index] = '\0';
    free(base_points);
    return 0;
}

int Crypto_RotationMatrix_Encode(Crypto_RotationMatrix* manager, const int len, const char* string, char* out)
{
    if (len <= 0) return -1;

    int N = 2;
    while (N * N < len) N++;

    Crypto_RotationMatrix_Resize(manager, N);
    Crypto_RotationMatrix_CleanUpCachedSteps(manager);

    int processed = 0;
    int out_index = 0;

    int num_base_holes = 0;
    GridPoint* base_points = Crypto_RotationMatrix_GenerateBaseHoles(N, &num_base_holes);

    int has_center = (N % 2 != 0) ? 1 : 0;
    int center_r = N / 2;
    int center_c = N / 2;

    while (processed < len)
    {
        for (int r = 0; r < manager->matrix_length; r++)
            memset(manager->process_matrix[r], ' ', manager->matrix_length);

        GridPoint* current_points = (GridPoint*)malloc(num_base_holes * sizeof(GridPoint));
        memcpy(current_points, base_points, num_base_holes * sizeof(GridPoint));

        for (int rot = 0; rot < 4; rot++)
        {
            int active_count = num_base_holes + ((rot == 0 && has_center) ? 1 : 0);
            GridPoint* active_points = (GridPoint*)malloc(active_count * sizeof(GridPoint));

            memcpy(active_points, current_points, num_base_holes * sizeof(GridPoint));
            if (rot == 0 && has_center)
            {
                active_points[num_base_holes].r = center_r;
                active_points[num_base_holes].c = center_c;
            }

            for (int i = 0; i < active_count; i++)
            {
                if (processed < len)
                {
                    manager->process_matrix[active_points[i].r][active_points[i].c] = (char)toupper(string[processed++]);
                }
                else
                {
                    manager->process_matrix[active_points[i].r][active_points[i].c] = 'A' + (processed - len) % ('Z' - 'A' + 1);
                    processed++;
                }

                for (int step = rot; step < 4; step++)
                {
                    manager->steps[step][active_points[i].r][active_points[i].c].c = manager->process_matrix[active_points[i].r][active_points[i].c];
                    manager->steps[step][active_points[i].r][active_points[i].c].is_new = (step == rot);
                }
            }

            for (int i = 0; i < num_base_holes; i++)
            {
                int temp_r = current_points[i].r;
                current_points[i].r = current_points[i].c;
                current_points[i].c = manager->matrix_length - 1 - temp_r;
            }

            free(active_points);
        }

        for (int r = 0; r < manager->matrix_length; r++)
        {
            for (int c = 0; c < manager->matrix_length; c++)
            {
                out[out_index++] = manager->process_matrix[r][c];
            }
        }
        free(current_points);
    }

    out[out_index] = '\0';
    free(base_points);
    return 0;
}

#endif //INFO_THEORY_ROTATION_MATRIX_H