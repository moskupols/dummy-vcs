#pragma once

// Коды возможных ошибок

typedef enum
{
    // Нет ошибки
    SUCCESS,

    // Попытка обратиться за границу текста
    ERR_INVALID_RANGE,

    // Ошибка в сохранённых изменениях
    ERR_INVALID_DELTA,

    // Операция невозможно из-за неподходящего номера версии
    ERR_INVALID_VERSION,

    // Не удалось открыть файл
    ERR_NO_SUCH_FILE,

    // Файл удалось открыть, но произошла ошибка при чтении:
    // например, он неожиданно оборвался
    ERR_READ,

    // Не получилось записать в файл
    ERR_WRITE,

    // Не удалось найти свободный номер версии
    ERR_VERSIONS_LIMIT
} return_t;

const char* error_description(return_t r);
