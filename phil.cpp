#include <iostream>
#include <windows.h>

// Количество философов
const int Max_Phil = 5;
// Количество философов за столом
const int Phil_Count = 2;
//Семафор
HANDLE Semaphore = 0;
// Индекс текущего философа
int Phil_Index[2] = { 1, 3 };
int Flag_First = 0;
int Flag_Second = 0;
HANDLE Threads_Array[Max_Phil] = { 0 };
//Критическая зона
CRITICAL_SECTION CriticalSection = { 0 };
// Структура для хранения временных значений
struct TimeValues{
    DWORD Time_Begin; // Время начала работы программы
    DWORD Time_End;   // Время окончания работы программы
    DWORD Time_Sleep; // Время задержки между действиями философов
} Time_Values = { 0 };

DWORD WINAPI Thread_Function(void* Empty_Parametr) {
    long Value = 0;
    while (true) {
        int Cur_Phil = 0;
        //Захват семафора
        WaitForSingleObjectEx(Semaphore, INFINITE, TRUE);
        //Флаги для получения номера философа
        if (Flag_First == 1 && Flag_Second == 0) {
            Flag_Second = 1;
            Cur_Phil = Phil_Index[1];
        }
        if (Flag_First == 0) {
            Flag_First = 1;
            Cur_Phil = Phil_Index[0];
        }

        //Если программа работает больше времени, чем требуется, то выход из цикла
        if (Time_Values.Time_End <= GetTickCount64() + 3*(Time_Values.Time_Sleep)) {
            ReleaseSemaphore(Semaphore, 1, &Value);
            break;
        }
        //Чтобы успел закончиться второй поток из предыдущей ситуации
        Sleep(5);
        //Вход в критическую секцию для записи
        EnterCriticalSection(&CriticalSection);
        std::cout << GetTickCount64() - Time_Values.Time_Begin << ':' << Cur_Phil << ":T->E" << std::endl;
        LeaveCriticalSection(&CriticalSection);
        //Поток уходит в сон
        Sleep(Time_Values.Time_Sleep);
        //Вход в критическую секцию для записи
        EnterCriticalSection(&CriticalSection);
        std::cout << GetTickCount64() - Time_Values.Time_Begin << ':' << Cur_Phil  << ":E->T" << std::endl;
        //Выход из критической секции для записи
        LeaveCriticalSection(&CriticalSection);
        //Смена индекса философов
        if (Flag_First == 1 && Flag_Second == 1) {
            if (Phil_Index[0] == 5) Phil_Index[0] = 1;
            else Phil_Index[0] = Phil_Index[0] + 1;
            if (Phil_Index[0] == 4) Phil_Index[1] = 1;
            else if (Phil_Index[0] == 5) Phil_Index[1] = 2;
            else Phil_Index[1] = Phil_Index[0] + 2;
            Flag_First = 0;
        }
        Flag_Second = 0;
        //Освобождение семафора
        ReleaseSemaphore(Semaphore, 1, &Value);
    }
    return 0;
}
void Init_Time(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Incorrect input data" << std::endl;
        exit(1);
    }
    else {
        Time_Values.Time_Begin = GetTickCount64();
        Time_Values.Time_End = Time_Values.Time_Begin + atoi(argv[1]);
        Time_Values.Time_Sleep = atoi(argv[2]);
    }
}
void Create() {
    for (int i = 0; i < Max_Phil; i++)
        Threads_Array[i] = CreateThread(0, 0, Thread_Function, 0, 0, 0);
}

void Close() {
    for (int i = 0; i < Max_Phil; i++)
        CloseHandle(Threads_Array[i]);
    CloseHandle(Semaphore);
    DeleteCriticalSection(&CriticalSection);
}

void ActionMain(int argc, char** argv) {
    //Обработка входных данных
    Init_Time(argc, argv);
    InitializeCriticalSection(&CriticalSection);
    Semaphore = CreateSemaphore(0, 2, 2, 0);
    //Создание потоков
    Create();
    // Ожидание завершения всех потоков
    while (TRUE)
        if (WAIT_TIMEOUT != WaitForMultipleObjects(Max_Phil, Threads_Array, TRUE, 0)) break;
    Close();
}

int main(int argc, char** argv){
    ActionMain(argc, argv);
    return 0;
}