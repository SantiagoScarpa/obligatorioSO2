//GENERADA POR GEMINI
#include <windows.h> //libreria para windows para la utilizacion de semaforos
#include <iostream>
#include <queue>//libreria para la utilizacion de la estructura de cola para el buffer (FIFO)


#define BUFFER_SIZE 5 //defino el tama;o del buffer

// HANDLE hace referencia a la utilizacion de objetos del kernel donde,
HANDLE mutex;        // identifica el objeto de mutua exclusion, se comporta como semaforo binario
HANDLE semVacio;    // semaforo que cuenta la cantidad de espacios vacios en el buffer, se comporta como semaforo contador
HANDLE semLleno;    // semaforo que cuenta la cantidad de espacios con items en el buffer, se comporta como semaforo contador

// Utilizo una queue de int para el buffer, este es FIFO
std::queue<int> buffer;

//Clase productora
//DWORD es un tipo de dato de windows que se utiliza como INT, usamos esto en vez de int, porque el manejo de procesos concurrentes lo espera. Este tiene una mejor compatibilidad y consistencia que INT en windows
//WINAPI es utilizado para procesos que van a correr en hilo de windows, tiene una serie de llamadas estandarizadas para llamadas del sistema de windows
//LPVOID, al igual que DWORD es un tipo de dato que es necesario para la programacion en hilos de windows, este es un puntero .
DWORD WINAPI Productor(LPVOID lpParam) {
  int item = 1;
  //Creo elementos mientras sea posible
  while (true) {
    // Produce un elemento
    cout << "Productor: produciendo elemento " << item <<endl;
    Sleep(1000); // Simulo una espera para que sea legible en ejecucion

    // Es una funcion de windows.h que espera un objeto HANDLE el cual debe estar disponible, en este caso, el semaforo de vacia para marcar que hay espacio en el buffer
    //el INFINITE hace referencia a cuandos milisegundos debe esperar por este semaforo, se pone infinite ya que no se desea que de time out
    WaitForSingleObject(semVacio, INFINITE);

    // Acceso exclusivo al buffer
    WaitForSingleObject(mutex, INFINITE);
    buffer.push(item++);
    ReleaseMutex(mutex);

    // Se�aliza que hay un elemento m�s en el buffer
    ReleaseSemaphore(semLleno, 1, NULL);
  }
  return 0;
}


// Funci�n del consumidor
DWORD WINAPI Consumidor(LPVOID lpParam) {
  while (true) {
    // Espera a que haya un elemento en el buffer
    WaitForSingleObject(semLleno, INFINITE);

    // Acceso exclusivo al buffer
    WaitForSingleObject(mutex, INFINITE);
    int item = buffer.front();
    buffer.pop();
    ReleaseMutex(mutex);

    // Consume el elemento
    std::cout << "Consumidor: consumiendo elemento " << item << std::endl;
    Sleep(1500); // Simula tiempo de consumo

    // Se�aliza que hay un espacio vac�o m�s en el buffer
    ReleaseSemaphore(semVacio, 1, NULL);
  }
  return 0;
}

int main() {
  // Crea los sem�foros
  mutex = CreateMutex(NULL, FALSE, NULL);
  semVacio = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
  semLleno = CreateSemaphore(NULL, 0, BUFFER_SIZE, NULL);

  // Crea los hilos productor y consumidor
  HANDLE hProducer = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hConsumer = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);

  // Espera a que los hilos terminen (en este caso, nunca terminan)
  WaitForSingleObject(hProducer, INFINITE);
  WaitForSingleObject(hConsumer, INFINITE);

  // Cierra los handles
  CloseHandle(hProducer);
  CloseHandle(hConsumer);
  CloseHandle(mutex);
  CloseHandle(semVacio);
  CloseHandle(semLleno);

  return 0;
}
