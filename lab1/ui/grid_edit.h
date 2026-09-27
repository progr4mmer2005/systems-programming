// Grid: редактирование ячейки и меню по правому клику.
//
// Чтобы изменить ячейку, поверх неё ставится обычное поле ввода (editor).
// Enter - сохранить, Esc - отменить.
#pragma once

#include "grid.h"

// Открыть поле ввода над текущей ячейкой.
// typed - первая нажатая буква, если пользователь сразу начал печатать;
// пусто - тогда в поле будет текущий текст ячейки.
void Grid::beginEdit(wstring typed) {
    if (!editable || rowCount() == 0 || currentRow < 0) return;
    ListView_EnsureVisible(hwnd, currentRow, FALSE);
    RECT box = cellBox(currentRow, currentCol);
    editRow = currentRow;
    editCol = currentCol;

    wstring text = typed != L"" ? typed : toWide(cellText(currentRow, currentCol));
    editor = CreateWindowExW(0, L"EDIT", text.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, box.left, box.top,
                             box.right - box.left, box.bottom - box.top, hwnd, NULL, GetModuleHandleW(NULL), NULL);
    SendMessageW(editor, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessageW(editor, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(S(4), S(4)));
    oldEditorProc = (WNDPROC)SetWindowLongPtrW(editor, GWLP_WNDPROC, (LONG_PTR)cellEditorProc);
    SetFocus(editor);
    if (typed != L"")
        SendMessageW(editor, EM_SETSEL, text.size(), text.size());  // курсор в конец
    else
        SendMessageW(editor, EM_SETSEL, 0, -1);  // выделить весь текст
}

// Закрыть поле ввода.
//   save - сохранить текст в ячейку;
//   moveRows, moveCols - куда потом сдвинуться (Enter - на строку ниже, Tab - вправо);
//   focusGrid - вернуть фокус таблице.
void Grid::finishEdit(bool save, int moveRows, int moveCols, bool focusGrid) {
    if (editor == NULL || finishingEdit) return;
    finishingEdit = true;  // закрытие поля вызывает finishEdit повторно
    string text = trim(windowText(editor));
    HWND closing = editor;
    editor = NULL;
    DestroyWindow(closing);

    if (save && editRow < rowCount() && rows[editRow][editCol] != text) {
        rows[editRow][editCol] = text;
        changed();
    }
    if (moveRows != 0) select(min(max(currentRow + moveRows, 0), rowCount() - 1), currentCol);
    if (moveCols != 0) moveColumn(moveCols);
    if (focusGrid) SetFocus(hwnd);
    InvalidateRect(hwnd, NULL, FALSE);
    finishingEdit = false;
}

// Шаг влево/вправо; с края строки - на предыдущую/следующую строку.
void Grid::moveColumn(int delta) {
    currentCol += delta;
    if (currentCol < 0) {
        if (currentRow > 0) {
            currentRow--;
            currentCol = columnCount() - 1;
        } else {
            currentCol = 0;
        }
    }
    if (currentCol >= columnCount()) {
        if (currentRow < rowCount() - 1) {
            currentRow++;
            currentCol = 0;
        } else {
            currentCol = columnCount() - 1;
        }
    }
    select(currentRow, currentCol);
}

// Меню по правому клику: вставить / удалить строку, изменить / очистить ячейку.
void Grid::showContextMenu(LPARAM lParam) {
    if (!editable) return;
    POINT point = {mouseX(lParam), mouseY(lParam)};
    bool fromKeyboard = point.x == -1 && point.y == -1;  // клавиша "меню" на клавиатуре
    if (fromKeyboard) {
        RECT box = cellBox(currentRow, currentCol);
        point = {box.left + S(8), box.bottom};
        ClientToScreen(hwnd, &point);
    } else {
        // Выделить ячейку, по которой щёлкнули.
        LVHITTESTINFO hit = {};
        hit.pt = point;
        ScreenToClient(hwnd, &hit.pt);
        ListView_SubItemHitTest(hwnd, &hit);
        if (hit.iItem >= 0) select(hit.iItem, toDataColumn(hit.iSubItem));
    }
    finishEdit(true, 0, 0, true);

    const int ABOVE = 1, BELOW = 2, REMOVE = 3, EDIT = 4, CLEAR = 5, AS_TEXT = 6;
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, AS_TEXT, L"Редактировать как текст...\tCtrl+E");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ABOVE, L"Вставить строку выше\tIns");
    AppendMenuW(menu, MF_STRING, BELOW, L"Вставить строку ниже");
    AppendMenuW(menu, MF_STRING, REMOVE, L"Удалить строку\tCtrl+Del");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, EDIT, L"Изменить ячейку\tEnter");
    AppendMenuW(menu, MF_STRING, CLEAR, L"Очистить ячейку\tDel");
    int choice = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
    DestroyMenu(menu);

    if (choice == ABOVE) insertRow(currentRow);
    if (choice == BELOW) insertRow(currentRow + 1);
    if (choice == REMOVE) deleteRow(currentRow);
    if (choice == EDIT) beginEdit(L"");
    if (choice == CLEAR) clearCell();
    if (choice == AS_TEXT) editAsText();
}
