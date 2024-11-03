//GENERADA POR GEMINI
#include <windows.h> //libreria para windows para la utilizacion de semaforos
#include <iostream>
#include <queue>//libreria para la utilizacion de la estructura de cola para el buffer (FIFO)


#define BUFFER_SIZE 5 //defino el tama;o del buffer

// HANDLE hace referencia a la utilizacion de objetos del kernel donde,
HANDLE mutex;        // identifica el objeto de mutua exclusion
HANDLE semVacio;    // semaforo que cuenta la cantidad de espacios vacios en el buffer
HANDLE semLleno;    // semaforo que cuenta la cantidad de espacios con items en el buffer

// Utilizo una queue de int para el buffer, este es FIFO
std::queue<int> buffer;

//
DWORD WINAPI Producer(LPVOID lpParam) {
  int item = 1;
  while (true) {
    // Produce un elemento
    std::cout << "Productor: produciendo elemento " << item << std::endl;
    Sleep(1000); // Simula tiempo de producción

    // Espera a que haya espacio en el buffer
    WaitForSingleObject(semVacio, INFINITE);

    // Acceso exclusivo al buffer
    WaitForSingleObject(mutex, INFINITE);
    buffer.push(item++);
    ReleaseMutex(mutex);

    // Señaliza que hay un elemento más en el buffer
    ReleaseSemaphore(semLleno, 1, NULL);
  }
  return 0;
}


// Función del consumidor
DWORD WINAPI Consumer(LPVOID lpParam) {
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

    // Señaliza que hay un espacio vacío más en el buffer
    ReleaseSemaphore(semVacio, 1, NULL);
  }
  return 0;
}

int main() {
  // Crea los semáforos
  mutex = CreateMutex(NULL, FALSE, NULL);
  semVacio = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
  semLleno = CreateSemaphore(NULL, 0, BUFFER_SIZE, NULL);

  // Crea los hilos productor y consumidor
  HANDLE hProducer = CreateThread(NULL, 0, Producer, NULL, 0, NULL);
  HANDLE hConsumer = CreateThread(NULL, 0, Consumer, NULL, 0, NULL);

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
