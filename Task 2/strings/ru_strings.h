//
// Created by REXE on 15.02.26.
//

#ifndef INFO_THEORY_RU_STRINGS_H
#define INFO_THEORY_RU_STRINGS_H

// "General App" strings
static const char *APP_STR_TITLE =
    "ТИ, лабораторная работа 2 (вариант 14)";

static const char *APP_STR_LAUNCH_TITLE =
    "Запуск";
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
static const char *LFSRP_STR_TITLE =
    "Поточное LFSR шифрование";

static const char *LFSRP_STR_ENTRY_BEGIN_REGISTER_STATE_TITLE =
    "Начальное значение регистра";
static const char *LFSRP_STR_ENTRY_BEGIN_REGISTER_STATE_COUNTER_TEXT =
    "%d / %d";
static const char *LFSRP_STR_ENTRY_INPUT_FILE_TITLE =
    "Входной файл";
static const char *LFSRP_STR_ENTRY_OUTPUT_FILE_TITLE =
    "Итоговый файл";
static const char *LFSRP_STR_ENTRY_INPUT_FILE_BYTES_TITLE =
    "Входные байты";
static const char *LFSRP_STR_ENTRY_GENERATED_BYTES_TITLE =
    "Генерация";
static const char *LFSRP_STR_ENTRY_OUTPUT_FILE_BYTES_TITLE =
    "Итоговые байты";

static const char *LFSRP_STR_NO_ONES_IN_REGISTER_ERROR =
    "Регистр должен содержать хотя бы одну единицу для возможности работы алгоритма!";
static const char *LFSRP_STR_EMPTY_REGISTER_ERROR =
    "Регистр не может быть пустым!";

static const char *LFSRP_STR_DIALOG_PROGRESS_TITLE =
    "Шифрование...";
static const char *LFSRP_STR_DIALOG_PROGRESS_DESCRIPTION =
    "Процесс потокового шифрования занимает некоторое время.\n"
    "Об оставшемся количестве итераций можно узнать из полосы прогресса ниже!";
static const char *LFSRP_STR_DIALOG_PROGRESS_TEXT =
    "%s / %s";
static const char *LFSRP_STR_DIALOG_PROGRESS_VALUE_WOF_TEXT =
    "%.0f %s";
static const char *LFSRP_STR_DIALOG_PROGRESS_VALUE_WF_TEXT =
    "%.2f %s";
static const char *LFSRP_STR_DIALOG_MEMORY_UNITS_B =
    "Б";
static const char *LFSRP_STR_DIALOG_MEMORY_UNITS_KB =
    "кБ";
static const char *LFSRP_STR_DIALOG_MEMORY_UNITS_MB =
    "МБ";
static const char *LFSRP_STR_DIALOG_MEMORY_UNITS_GB =
    "ГБ";
static const char *LFSRP_STR_DIALOG_MEMORY_UNITS_TB =
    "ТБ";
static const char *LFSRP_STR_DIALOG_MEMORY_UNITS_PB =
    "ПБ";
// END

#endif //INFO_THEORY_RU_STRINGS_H
