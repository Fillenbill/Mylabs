#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <chrono>

HANDLE ThreadArray[100] = { };
HANDLE Event = NULL;
unsigned short ThreadCount = 0;
unsigned short SplitNumber = 0;
unsigned int Result = 0;
int* CurNumber = 0;
int i = 0;
int Index = 0;

DWORD WINAPI ThreadFunction(void* parameterVoid) {
    WaitForSingleObject(Event, INFINITE);
    int Count = 0;
    while (TRUE) {
        Result++;
        //Проверка, что число уже разложено -> Выход
        if (CurNumber[0] == 1) {
            SetEvent(Event);
            return 0;
        }

        //Проверка на то, что поток выполнил все свои разложения -> Выход
        if (i == 0) {
            Count++;
            if (Count == Index) {
                SetEvent(Event);
                return 0;
            }
        }
        //Количество единиц сверху
        int Val = 0;
        while (i >= 0 && CurNumber[i] == 1) {
            Val++;
            i--;
        }
        //Если все единицы -> Выход
        if (i < 0) {
            SetEvent(Event);
            return 0;
        }
        //Вычитаем из не единицы
        CurNumber[i]--;
        Val++;
        //
        while (Val > CurNumber[i]) {
            CurNumber[i + 1] = CurNumber[i];
            Val -= CurNumber[i];
            i++;
        }
        i++;
        CurNumber[i] = Val;
        for (int k = i + 1; k < SplitNumber; k++) CurNumber[k] = 0;
        //for (int k = 0; CurNumber[k] > 0; k++)printf("%d", CurNumber[k]);
        //printf("\n");
    }
}


//Функция для чтения данных из файла
void ReadInputFile() {
    FILE* InputFile = fopen("input.txt", "r");
    if (InputFile == NULL) {
        printf("Error: failed to open input file.\n");
        exit(1);
    }
    if (!fscanf(InputFile, "%hu%hu", &ThreadCount, &SplitNumber)) {
        //Вывод ошибки
        exit(1);
    }
    fclose(InputFile);
}
//Функция для записи данных в файл
void write(time_t Time) {
    FILE* OutputFile = fopen("output.txt", "w");
    FILE* TimeFile = fopen("time.txt", "w");


    fprintf(OutputFile, "%u\n", ThreadCount);
    fprintf(OutputFile, "%u\n", SplitNumber);
    fprintf(OutputFile, "%u\n", Result - ThreadCount);
    fprintf(TimeFile, "%lf", ((double)Time) / CLOCKS_PER_SEC);

    fclose(TimeFile);
    fclose(OutputFile);

}
void Close() {
    for (i = 0; i < ThreadCount; i++)
        CloseHandle(ThreadArray[i]);
    if (Event != 0)
        CloseHandle(Event);
}
void Create() {
    for (int k = 0; k < ThreadCount; k++)
        ThreadArray[k] = CreateThread(0, 0, ThreadFunction, (char*)0 + k, 0, 0);
}
void GetIndex() {
    Index = SplitNumber - 1;
    if (ThreadCount >= Index) ThreadCount = Index - 1;
    if (Index % ThreadCount == 0) Index = Index / ThreadCount;
    else Index = (Index / ThreadCount) + 1;
}
void ActionMain() {
    //Чтение входных данных
    ReadInputFile();
    //Создание начальной последовательности
    CurNumber = new int[SplitNumber];
    CurNumber[i] = SplitNumber;
    //Получение количества действий для одного потока
    GetIndex();
    //Создание потоков
    Create();
    clock_t Time = clock();
    // Ожидание завершения всех потоков
    while (TRUE)
        if (WAIT_TIMEOUT != WaitForMultipleObjects(ThreadCount, ThreadArray, TRUE, 0)) break;
    Time = clock() - Time;
    delete[] CurNumber;
    //Запись в файл результатов
    write(Time);
}

int main(void) {
    Event = CreateEvent(NULL, FALSE, TRUE, NULL);
    // Запускаем выполнение программы
    ActionMain();
    //
    Close();
    return 0;
}

