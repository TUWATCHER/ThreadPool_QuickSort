#include "ThreadPool.hpp"
#include <iostream>

volatile bool make_thread = true;
RequestHandler rh;

void quicksort(int *array, long left, long right)
{
  
  if(left >= right) return;
  long left_bound = left;
  long right_bound = right;

  long middle = array[(left_bound + right_bound) / 2];

  do
  {
    while(array[left_bound] < middle)
      {
        left_bound++;
      }
       while(array[right_bound] > middle)
       {
          right_bound--;
       }

       //Меняем элементы местами
       if (left_bound <= right_bound) {
           std::swap(array[left_bound], array[right_bound]);
           left_bound++;
           right_bound--;
       }
  } while (left_bound <= right_bound);

   if(make_thread && (right_bound - left >= 10000))
   {
       // если элементов в левой части больше чем 10000
       // вызываем асинхронно рекурсию для правой части
      

      auto f = rh.pushRequest(quicksort, array, left, right_bound);
      quicksort(array, left_bound, right);
      f.wait();
       
  }
    else 
    {
      // запускаем обе части синхронно
      quicksort(array, left, right_bound);
      quicksort(array, left_bound, right);
    }
}

int main()
{
  srand(0);
   long arr_size = 100000;
   int* array = new int[arr_size];
   for(long i=0;i<arr_size; i++) {
       array[i] = rand() % 500000;
   }

   time_t start, end;


   // многопоточный запуск
   time(&start);
   quicksort(array, 0, arr_size);
   time(&end);

   double seconds = difftime(end, start);
   printf("The time: %f seconds\n", seconds);

   for(long i=0;i<arr_size-1; i++) {
       if (array[i] > array[i + 1]) {
         std::cout << "Unsorted" << std::endl;
         break;
       }
   }

   for(long i=0;i<arr_size; i++) {
       array[i] = rand() % 500000;
   }
   // однопоточный запуск
   make_thread = false;
   time(&start);
   quicksort(array, 0, arr_size);
   time(&end);
   seconds = difftime(end, start);
   printf("The time: %f seconds\n", seconds);
   delete [] array;
   return 0;
}
