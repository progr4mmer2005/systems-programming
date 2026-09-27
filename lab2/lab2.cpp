// Лабораторная работа №2
// Двухпросмотровый ассемблер для программы в перемещаемом формате.
//
// Сборка: g++ lab2.cpp -o bin/lab2 (или Ctrl+Alt+N в VS Code).
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
