#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <queue>
#include <windows.h>

using namespace std;

struct Subtask {
    int start;
    int end;
};

int thread_count;  // Количество потоков
int array_size;    // Размер массива
vector<int> array_; // Массив, который будем сортировать
HANDLE task_semaphore; // Семафор для управления задачами
CRITICAL_SECTION critical_section; // Критическая секция для синхронизации доступа к общим данным
queue<Subtask> task_queue; // Очередь задач для потоков

void QuickSort(int low, int high); // Прототип функции быстрой сортировки
void EnqueueTask(Subtask task); // Прототип функции добавления задачи в очередь
bool DequeueTask(Subtask& task); // Прототип функции извлечения задачи из очереди
void NotifyAllThreads(); // Прототип функции оповещения всех потоков

DWORD WINAPI ThreadQuickSort(LPVOID lpParam); // Прототип функции потока быстрой сортировки

DWORD WINAPI ThreadQuickSort(LPVOID lpParam) {
    Subtask task;

    while (DequeueTask(task)) {
        QuickSort(task.start, task.end); // Выполнение быстрой сортировки для задачи
    }

    return 0;
}

void QuickSort(int low, int high) {
    int i = low;
    int j = high;
    int pivot = array_[(low + high) / 2];

    while (i <= j) {
        while (array_[i] < pivot) {
            i++;
        }
        while (array_[j] > pivot) {
            j--;
        }
        if (i <= j) {
            swap(array_[i++], array_[j--]); // Обмен элементов местами
        }
    }

    if (low < j) {
        Subtask left_task = { low, j };
        EnqueueTask(left_task); // Добавление левой части массива в очередь
    }
    if (i < high) {
        Subtask right_task = { i, high };
        EnqueueTask(right_task); // Добавление правой части массива в очередь
    }
}

void EnqueueTask(Subtask task) {
    EnterCriticalSection(&critical_section); // Вход в критическую секцию
    task_queue.push(task); // Добавление задачи в очередь
    LeaveCriticalSection(&critical_section); // Выход из критической секции
}

bool DequeueTask(Subtask& task) {
    EnterCriticalSection(&critical_section); // Вход в критическую секцию

    if (!task_queue.empty()) {
        task = task_queue.front(); // Извлечение задачи из очереди
        task_queue.pop();
        LeaveCriticalSection(&critical_section); // Выход из критической секции
        return true;
    }

    LeaveCriticalSection(&critical_section); // Выход из критической секции
    return false;
}

int main() {
    ifstream input_file("input.txt");
    ofstream output_file("output.txt");
    ofstream time_file("time.txt");

    input_file >> thread_count >> array_size;

    array_.resize(array_size);
    for (int i = 0; i < array_size; ++i) {
        input_file >> array_[i];
    }

    task_semaphore = CreateSemaphore(NULL, 1, thread_count, NULL); // Создание семафора
    InitializeCriticalSection(&critical_section); // Инициализация критической секции

    vector<HANDLE> threads(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        threads[i] = CreateThread(NULL, 0, ThreadQuickSort, NULL, 0, NULL); // Создание потоков
    }

    DWORD start_time = GetTickCount(); // Запуск таймера

    Subtask initial_task = { 0, array_size - 1 };
    EnqueueTask(initial_task); // Добавление начальной задачи в очередь

    WaitForMultipleObjects(thread_count, threads.data(), TRUE, INFINITE); // Ожидание завершения потоков

    DWORD end_time = GetTickCount(); // Остановка таймера
    DWORD duration = end_time - start_time; // Вычисление длительности выполнения

    output_file << thread_count << "\n" << array_size << "\n";
    for (int i = 0; i < array_size; ++i) {
        output_file << array_[i] << " "; // Запись отсортированного массива в файл
    }

    time_file << duration; // Запись времени выполнения в файл

    CloseHandle(task_semaphore); // Закрытие семафора
    DeleteCriticalSection(&critical_section); // Уничтожение критической секции

    input_file.close();
    output_file.close();
    time_file.close();

    return 0;
}
