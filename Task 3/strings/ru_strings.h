//
// Created by REXE on 15.02.26.
//

#ifndef INFO_THEORY_RU_STRINGS_H
#define INFO_THEORY_RU_STRINGS_H

// "General App" strings
static const char *APP_STR_TITLE =
    "ТИ, лабораторная работа 3 (вариант 3)";

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

// "LFSR page" strings
static const char *RABINP_STR_TITLE =
    "Шифрование криптосистемой Рабина";

static const char *RABINP_STR_ENTRY_P_TITLE =
    "P";
static const char *RABINP_STR_ENTRY_Q_TITLE =
    "Q";
static const char *RABINP_STR_ENTRY_B_TITLE =
    "B";
static const char *RABINP_STR_ENTRY_INPUT_FILE_TITLE =
    "Входной файл";
static const char *RABINP_STR_ENTRY_OUTPUT_FILE_TITLE =
    "Итоговый файл";
static const char *RABINP_STR_ENTRY_INPUT_FILE_BYTES_TITLE =
    "Входные байты";
static const char *RABINP_STR_ENTRY_OUTPUT_FILE_BYTES_TITLE =
    "Итоговые байты";

static const char *RABINP_STR_CANNOT_BE_EMPTY_ERROR =
    "Строка не может быть пустой!";
static const char *RABINP_STR_P_AND_Q_IS_EQUAL_OR_LESS_ERROR =
    "P и Q не должны быть равны, а также их произведение должно быть больше 256!";
static const char *RABINP_STR_P_MOD_NOT_EQUAL_3_ERROR =
    "P по модулю 4 должно быть равно 3!";
static const char *RABINP_STR_P_IS_NOT_PRIME_ERROR =
    "P должно быть простым числом!";
static const char *RABINP_STR_Q_MOD_NOT_EQUAL_3_ERROR =
    "Q по модулю 4 должно быть равно 3!";
static const char *RABINP_STR_Q_IS_NOT_PRIME_ERROR =
    "Q должно быть простым числом!";
static const char *RABINP_STR_B_LESS_OR_MORE_ERROR =
    "B не может быть отрицательным или больше, чем произведение P и Q!";
static const char *RABINP_STR_P_AND_Q_IS_LESS_THAN_RECOM_WARNING =
    "При произведении чисел P и Q меньше, чем 10533, точность расшифровки может снизиться! Рекомендуется увеличить числа для повышения стойкости и точности.";

static const char *RABINP_STR_DIALOG_PROGRESS_TITLE =
    "Шифрование...";
static const char *RABINP_STR_DIALOG_PROGRESS_DESCRIPTION =
    "Процесс потокового шифрования занимает некоторое время.\n"
    "Об оставшемся количестве итераций можно узнать из полосы прогресса ниже!";
static const char *RABINP_STR_DIALOG_PROGRESS_TEXT =
    "%s / %s";
static const char *RABINP_STR_DIALOG_PROGRESS_VALUE_WOF_TEXT =
    "%.0f %s";
static const char *RABINP_STR_DIALOG_PROGRESS_VALUE_WF_TEXT =
    "%.2f %s";
static const char *RABINP_STR_DIALOG_MEMORY_UNITS_B =
    "Б";
static const char *RABINP_STR_DIALOG_MEMORY_UNITS_KB =
    "кБ";
static const char *RABINP_STR_DIALOG_MEMORY_UNITS_MB =
    "МБ";
static const char *RABINP_STR_DIALOG_MEMORY_UNITS_GB =
    "ГБ";
static const char *RABINP_STR_DIALOG_MEMORY_UNITS_TB =
    "ТБ";
static const char *RABINP_STR_DIALOG_MEMORY_UNITS_PB =
    "ПБ";
// END

#endif //INFO_THEORY_RU_STRINGS_H
