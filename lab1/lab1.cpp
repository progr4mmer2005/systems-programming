// Лабораторная работа №1
// Двухпросмотровый ассемблер для программы в абсолютном формате.
//
// Сборка: g++ lab1.cpp -o bin/lab1 (или Ctrl+Alt+N в VS Code).
// Остальные файлы подключаются отсюда:
//   asm/        ассемблер: форматы команд, ТКО, первый и второй проход
//   examples.h  примеры программ
//   config.h    настройки лабы
//   app/        окно лабы
//   ui/         таблицы, кнопки, цвета, шрифты
//   platform/   служебный код для Windows
//   util/       строки и числа

#include "platform/winapi.h"

#include "app/window.h"

int main() {
    hideConsoleWindow();
    return runApp();
}
