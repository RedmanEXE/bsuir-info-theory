//
// Created by REXE on 15.02.26.
//

#ifndef INFO_THEORY_RU_STRINGS_H
#define INFO_THEORY_RU_STRINGS_H

// "General App" strings
static const char *APP_STR_TITLE =
    "ТИ, лабораторная работа 2 (вариант 14)";

static const char *APP_STR_OPEN_FILE_TITLE =
    "Открыть файл...";
static const char *APP_STR_SAVE_FILE_TITLE =
    "Сохранить в файл...";

static const char *APP_STR_ENCRYPT_TITLE =
    "Зашифровать";
static const char *APP_STR_DECRYPT_TITLE =
    "Расшифровать";
static const char *APP_STR_CLEAR_TITLE =
    "Очистить";

static const char *APP_STR_FILE_OPEN_DIALOG_TITLE =
    "Выберите входной файл";
static const char *APP_STR_FILE_OPEN_DIALOG_FILTER_ANY =
    "Файл с любым расширением (*.*)";

static const char *APP_STR_FILE_SAVE_DIALOG_TITLE =
    "Выберите место для итогового файла";
// END

// "Rotation Matrix Page" strings
static const char *RTMP_STR_TITLE =
    "Поворачивающаяся решётка";

static const char *RTMP_STR_ENTRY_SOURCE_TEXT_TITLE =
    "Исходный текст";
static const char *RTMP_STR_ENTRY_SUMMARY_TEXT_TITLE =
    "Итоговый текст";

static const char *RTMP_STR_NON_VALID_CHARS_IN_SOURCE_TEXT_WARNING =
    "В исходном тексте содержатся символы, которые не являются символами английского алфавита. Они были пропущены в ходе операции!";
static const char *RTMP_STR_NO_VALID_CHARS_IN_SOURCE_TEXT_ERROR =
    "Исходный текст не содержит символов английского алфавита!";
static const char *RTMP_STR_EMPTY_SOURCE_TEXT_ERROR =
    "Это поле не может быть пустым!";

static const char *RTMP_STR_MATRIX_TITLE =
    "Матрица, полученная в ходе операции (по шагам)";
static const char *RTMP_STR_MATRIX_STEP_TITLE =
    "Шаг %d из %d";
// END

// "Vigenere Algorithm Page" strings
static const char *VALP_STR_TITLE =
    "Алгоритм Виженера (прогрессивный ключ)";

static const char *VALP_STR_ENTRY_KEY_TITLE =
    "Ключ";
static const char *VALP_STR_ENTRY_SOURCE_TEXT_TITLE =
    "Исходный текст";
static const char *VALP_STR_ENTRY_SUMMARY_TEXT_TITLE =
    "Итоговый текст";
static const char *VALP_STR_ENTRY_PROGRESSIVE_KEY_TITLE =
    "Прогрессивный ключ, полученный из оригинального";

static const char *VALP_STR_NON_VALID_CHARS_IN_KEY_WARNING =
    "Ключ содержит символы, которые не являются символами русского алфавита. Они были пропущены в ходе операции!";
static const char *VALP_STR_NON_VALID_CHARS_IN_SOURCE_TEXT_WARNING =
    "В исходном тексте содержатся символы, которые не являются символами русского алфавита. Они были пропущены в ходе операции!";
static const char *VALP_STR_NO_VALID_CHARS_IN_SOURCE_TEXT_ERROR =
    "Исходный текст не содержит символов русского алфавита!";
static const char *VALP_STR_NO_VALID_CHARS_IN_KEY_ERROR =
    "Ключ не содержит символов русского алфавита!";
static const char *VALP_STR_EMPTY_ENTRY_ERROR =
    "Это поле не может быть пустым!";

static const char *VALP_STR_MATRIX_TITLE =
    "Сопоставление итогового текста с исходными данными";
// END

#endif //INFO_THEORY_RU_STRINGS_H
