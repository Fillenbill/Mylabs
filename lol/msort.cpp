#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <algorithm>
#include <ctime>

using namespace std;

// Структура для представления подзадачи сортировки
struct Subtask {
    int start;
    int end;
};

// Глобальные переменные
int thread_count; // Количество потоков
int array_size;   // Размер массива
vector<int> array_; // Массив для сортировки
sem_t task_semaphore; // Семафор для синхронизации задач

// Прототипы функций
void* ThreadMergeSort(void*);
void Merge(int low, int mid, int high);
void MergeSort(int low, int high);
void EnqueueTask(Subtask task);
bool DequeueTask(Subtask& task);

int main() {
    ifstream input_file("input.txt");
    ofstream output_file("output.txt");
    ofstream time_file("time.txt");

    // Читаем входные данные
    input_file >> thread_count >> array_size;

    array_.resize(array_size);
    for (int i = 0; i < array_size; ++i) {
        input_file >> array_[i];
    }

    sem_init(&task_semaphore, 0, 1);

    // Создаем потоки
    pthread_t threads[thread_count];
    for (int i = 0; i < thread_count; ++i) {
        pthread_create(&threads[i], nullptr, ThreadMergeSort, nullptr);
    }

    clock_t start_time = clock();

    // Добавляем начальную задачу для всего массива в очередь
    Subtask initial_task = {0, array_size - 1};
    EnqueueTask(initial_task);

    // Ждем завершения всех потоков
    for (int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], nullptr);
    }

    clock_t end_time = clock();
    clock_t duration = end_time - start_time;

    // Записываем выходные данные
    output_file << thread_count << "\n" << array_size << "\n";
    for (int i = 0; i < array_size; ++i) {
        output_file << array_[i] << " ";
    }

    // Записываем время сортировки в файл времени
    time_file << duration;

    // Очистка
    sem_destroy(&task_semaphore);

    input_file.close();
    output_file.close();
    time_file.close();

    return 0;
}

void* ThreadMergeSort(void* arg) {
    Subtask task;

    while (DequeueTask(task)) {
        MergeSort(task.start, task.end);
    }

    pthread_exit(nullptr);
}

void Merge(int low, int mid, int high) {
    vector<int> left(array_.begin() + low, array_.begin() + mid + 1);
    vector<int> right(array_.begin() + mid + 1, array_.begin() + high + 1);

    int i = 0, j = 0, k = low;
    while (i < left.size() && j < right.size()) {
        if (left[i] <= right[j]) {
            array_[k++] = left[i++];
        } else {
            array_[k++] = right[j++];
        }
    }

    while (i < left.size()) {
        array_[k++] = left[i++];
    }

    while (j < right.size()) {
        array_[k++] = right[j++];
    }
}

void MergeSort(int low, int high) {
    if (low < high) {
        int mid = low + (high - low) / 2;
        MergeSort(low, mid);
        MergeSort(mid + 1, high);
        Merge(low, mid, high);
    }
}

void EnqueueTask(Subtask task) {
    sem_wait(&task_semaphore);
    // Добавляем задачу в очередь задач
    MergeSort(task.start, task.end);
    sem_post(&task_semaphore);
}

bool DequeueTask(Subtask& task) {
    sem_wait(&task_semaphore);
    sem_post(&task_semaphore);
    return false;  // возвращаем false, чтобы сигнализировать об отсутствии задач.
}
