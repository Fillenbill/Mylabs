#include <cstring>
#include <cstdio>
#include <pthread.h>
#include <semaphore.h>

// Максимальное количество потоков
#define THREAD_MAX_COUNT 64

// Количество потоков и размер массива для сортировки
unsigned long thread_count;
unsigned long array_size;

// Указатель на массив, который будем сортировать
int *array_ = nullptr;

// Массив потоков и семафор для работы с ними
pthread_t threads[THREAD_MAX_COUNT];
sem_t sem;

// Количество потоков, которые реально используются, и текущая часть массива
unsigned char active_threads_count = 0;
int part = 0;

// Прототипы функций
void *ThreadMergeSort(void *);
void InitSorting(void);
void FinalizeSorting(void);
void Merge(int low, int mid, int high);
void MergeSort(int low, int high);

// Функция инициализации семафора и запуска потоков
void InitSorting(void)
{
	// Инициализируем семафор
	sem_init(&sem, 1, 1);

	// Создаем указанное количество потоков
	memset(&threads, 0, sizeof(threads));
	for (unsigned char i = 0; i < active_threads_count; i++)
	{
		pthread_create(&threads[i], nullptr, ThreadMergeSort, nullptr);
	}
}

// Функция завершения работы потоков и уничтожения семафора
void FinalizeSorting(void)
{
	sem_destroy(&sem);

	// Ждем завершения всех потоков
	for (unsigned char i = 0; i < active_threads_count; i++)
		pthread_join(threads[i], nullptr);
}

// Функция слияния двух отсортированных частей массива в один
void Merge(int low, int mid, int high)
{
	int *left = new int[mid - low + 1];
	int *right = new int[high - mid];

	int n1 = mid - low + 1, n2 = high - mid, i, j;

	for (i = 0; i < n1; i++)
		left[i] = array_[i + low];

	for (i = 0; i < n2; i++)
		right[i] = array_[i + mid + 1];

	int k = low;
	i = j = 0;

	while (i < n1 && j < n2)
	{
		if (left[i] <= right[j])
			array_[k++] = left[i++];
		else
			array_[k++] = right[j++];
	}

	while (i < n1)
	{
		array_[k++] = left[i++];
	}

	while (j < n2)
	{
		array_[k++] = right[j++];
	}
	delete[] left;
	delete[] right;
}

// Рекурсивная функция сортировки слиянием
void MergeSort(int low, int high)
{
	int mid = low + (high - low) / 2;
	if (low < high)
	{
		MergeSort(low, mid);
		MergeSort(mid + 1, high);
		Merge(low, mid, high);
	}
}

// Точка входа для потоков, которые выполняют частичную сортировку массива
void *ThreadMergeSort(void *arg)
{
	sem_wait(&sem);
	int thread_part = part++;
	sem_post(&sem);

	// Вычисляем границы части массива, которую будем сортировать
	int low = thread_part * (array_size / active_threads_count);
	int high = (thread_part + 1) * (array_size / active_threads_count) - 1;

	// Выполняем сортировку этой части массива
	int mid = low + (high - low) / 2;
	if (low < high)
	{
		MergeSort(low, mid);
		MergeSort(mid + 1, high);
		Merge(low, mid, high);
	}

	return nullptr;
}

// Функция чтения данных из файла
void ReadDataFromFile(const char *filename)
{
	FILE *input_file = fopen(filename, "r");
	// Считываем количество потоков и размер массива
	fscanf(input_file, "%ld%ld", &thread_count, &array_size);

	// Выделяем память для массива
	array_ = new int[array_size];

	// Читаем элементы массива из файла
	if (array_size % 10 != 0)
	{
		for (unsigned int i = 0; i < array_size; i++)
			fscanf(input_file, "%d", array_ + i);
	}
	else
	{
		for (unsigned int i = 0; i < array_size; i += 10)
			fscanf(input_file, "%d%d%d%d%d%d%d%d%d%d", array_ + i, array_ + i + 1, array_ + i + 2, array_ + i + 3, array_ + i + 4, array_ + i + 5, array_ + i + 6, array_ + i + 7, array_ + i + 8, array_ + i + 9);
	}

	fclose(input_file);
}

// Функция записи данных в файл
void WriteDataToFile(const char *filename)
{
	FILE *output_file = fopen(filename, "w");

	fprintf(output_file, "%ld\n%ld\n", thread_count, array_size);

	if (array_size % 10 != 0)
	{
		for (unsigned int i = 0; i < array_size; i++)
			fprintf(output_file, "%d ", array_[i]);
	}
	else
	{
		for (unsigned int i = 0; i < array_size; i += 10)
			fprintf(output_file, "%d %d %d %d %d %d %d %d %d %d ", array_[i], array_[i + 1], array_[i + 2], array_[i + 3], array_[i + 4], array_[i + 5], array_[i + 6], array_[i + 7], array_[i + 8], array_[i + 9]);
	}

	fclose(output_file);
}

int main()
{
	ReadDataFromFile("input.txt");

	clock_t time_elapsed = clock();

	// Определяем, сколько потоков реально будем использовать
	if (array_size % 4 == 0 && array_size >= 100000)
		active_threads_count = 4;
	else
		active_threads_count = 1;

	InitSorting();
	FinalizeSorting();

	// Если массив делится на 4 части и его длина достаточно большая,
	// то выполняем еще одну фазу слияния для оптимизации производительности
	if (array_size % 4 == 0 && array_size >= 10000)
	{
		Merge(0, (array_size / 2 - 1) / 2, array_size / 2 - 1);
		Merge(array_size / 2, array_size / 2 + (array_size - 1 - array_size / 2) / 2, array_size - 1);
		Merge(0, (array_size - 1) / 2, array_size - 1);
	}

	time_elapsed = (clock() - time_elapsed) / (CLOCKS_PER_SEC / 1000);

	WriteDataToFile("output.txt");

	FILE *time_file = fopen("time.txt", "w");
	fprintf(time_file, "%ld", time_elapsed);
	fclose(time_file);

	// Освобождаем память
	delete[] array_;

	return 0;
}
