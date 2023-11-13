#include <iostream>
#include <vector>
#include <fstream>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

using namespace std;

void OpenFiles();
void ReadFiles();
void WriteFile();
void CloseFiles();
void StartTimer();
void FinishTimer();

unsigned long to_ms(struct timespec* tm);

ifstream f_in;
ofstream f_out, f_time;
int threads_count = 0, N = 0;
vector<int> arr;
struct timespec tm_start, tm_end;

void Merge(vector<int>& arr, int left, int middle, int right);
void MergeSort(vector<int>& arr, int left, int right);
void* ThreadMergeSort(void* arg);
void ParalSorting();

// Структура для передачи параметров в потоки
struct ThreadData {
    vector<int>* arr;
    int left;
    int right;
};

int main()
{
    OpenFiles();
    ReadFiles();
    StartTimer();
    ParalSorting();
    FinishTimer();
    WriteFile();
    CloseFiles();

    return 0;
}

void ParalSorting()
{
    sem_t semaphore;
    sem_init(&semaphore, 0, 0);

    vector<pthread_t> threads(threads_count);
    vector<ThreadData> threadData(threads_count);

    // Запускаем потоки для сортировки частей массива
    for (int i = 0; i < threads_count; i++)
    {
        threadData[i] = {&arr, i * (N / threads_count), (i + 1) * (N / threads_count) - 1};
        pthread_create(&threads[i], NULL, ThreadMergeSort, &threadData[i]);
    }

    // Ожидаем завершения всех потоков
    for (int i = 0; i < threads_count; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Выполняем слияние всех частей массива
    for (int size = N / threads_count, step = 0; size < N; size *= 2, step++)
    {
        for (int left = 0; left < N - size; left += 2 * size)
        {
            int middle = left + size - 1;
            int right = std::min(left + 2 * size - 1, N - 1);
            Merge(arr, left, middle, right);
        }
    }
    sem_destroy(&semaphore);
}

void OpenFiles()
{
    f_in.open("input.txt", ios::in | ios::binary);
    f_out.open("output.txt", ios::out | ios::binary);
    f_time.open("time.txt", ios::out | ios::binary);
    if (!f_in || !f_out || !f_time)
    {
        cerr << "File not open" << endl;
        exit(1);
    }
}

void ReadFiles()
{
    f_in >> threads_count;
    f_in >> N;

    int num;
    while (f_in >> num) {
        arr.push_back(num);
    }
}

void WriteFile()
{
    f_out << threads_count << endl;
    f_out << N << endl;
    
    for (int i : arr) {
        f_out << i << " ";
    }
}

unsigned long to_ms(struct timespec* tm)
{
 return ((unsigned long) tm->tv_sec * 1000 + (unsigned long) tm->tv_nsec / 1000000);
}

void StartTimer()
{
    clock_gettime(CLOCK_REALTIME, &tm_start);
}

void FinishTimer()
{
    clock_gettime(CLOCK_REALTIME, &tm_end);
    f_time << (to_ms(&tm_end) - to_ms(&tm_start));
}

void CloseFiles()
{
    fcloseall();
}

// Функция слияния двух отсортированных массивов
void Merge(vector<int>& arr, int left, int middle, int right)
{
    int n1 = middle - left + 1;
    int n2 = right - middle;

    vector<int> leftArr(n1);
    vector<int> rightArr(n2);

    for (int i = 0; i < n1; i++)
    {
        leftArr[i] = arr[left + i];
    }
    for (int i = 0; i < n2; i++)
    {
        rightArr[i] = arr[middle + 1 + i];
    }

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2)
    {
        if (leftArr[i] <= rightArr[j])
        {
            arr[k++] = leftArr[i++];
        }
        else
        {
            arr[k++] = rightArr[j++];
        }
    }

    while (i < n1)
    {
        arr[k++] = leftArr[i++];
    }

    while (j < n2)
    {
        arr[k++] = rightArr[j++];
    }
}

// Функция сортировки слиянием для одного потока
void MergeSort(vector<int>& arr, int left, int right)
{
    if (left < right)
    {
        int middle = left + (right - left) / 2;

        MergeSort(arr, left, middle);
        MergeSort(arr, middle + 1, right);

        Merge(arr, left, middle, right);
    }
}

// Функция потока для параллельной сортировки
void* ThreadMergeSort(void* arg)
{
    ThreadData* data = static_cast<ThreadData*>(arg);
    vector<int>* arr = data->arr;
    int left = data->left;
    int right = data->right;

    MergeSort(*arr, left, right);

    pthread_exit(NULL);
}