//GENERADA POR GEMINI
#include <windows.h> //libreria para windows para la utilizacion de semaforos
#include <iostream>
#include <queue>//libreria para la utilizacion de la estructura de cola para el buffer (FIFO)


#define MAXIMO_BUFFER 1 //defino el tama;o del buffer

// HANDLE hace referencia a la utilizacion de objetos del kernel donde,
HANDLE mutex;        // identifica el objeto de mutua exclusion, se comporta como semaforo binario
HANDLE semVacio;    // semaforo que cuenta la cantidad de espacios vacios en el buffer, se comporta como semaforo contador
HANDLE semLleno;    // semaforo que cuenta la cantidad de espacios con items en el buffer, se comporta como semaforo contador

// Utilizo una queue de int para el buffer, este es FIFO
std::queue<int> buffer;

//Funcion productora
//DWORD es un tipo de dato de windows que se utiliza como INT, usamos esto en vez de int, porque el manejo de procesos concurrentes lo espera. Este tiene una mejor compatibilidad y consistencia que INT en windows
//WINAPI es utilizado para procesos que van a correr en hilo de windows, tiene una serie de llamadas estandarizadas para llamadas del sistema de windows
//LPVOID, al igual que DWORD es un tipo de dato que es necesario para la programacion en hilos de windows, este es un puntero .
DWORD WINAPI Productor(LPVOID lpParam) {
  int item = 1;
  //Creo elementos mientras sea posible
  while (true) {
    // Produce un elemento
    std::cout << "Productor: creando elemento - " << item <<std::endl;
    Sleep(1000); // Simulo una espera para que sea legible en ejecucion

    // Es una funcion de windows.h que espera un objeto HANDLE el cual debe estar disponible, en este caso, el semaforo de vacia para marcar que hay espacio en el buffer
    //el INFINITE hace referencia a cuandos milisegundos debe esperar por este semaforo, se pone infinite ya que no se desea que de time out
    //es basicamente hacer el down(semVacio)
    WaitForSingleObject(semVacio, INFINITE);

    //hago el down(mutex), o sea valido que se puede acceder al buffer con el semaforo mutex o queda esperando infinito
    WaitForSingleObject(mutex, INFINITE);
    //le agrego un item+1 al buffer
    buffer.push(item++);
    //hago el up(mutex) para liberar el semaforo
    ReleaseMutex(mutex);

    // Hago un up(semLlano) para marcar que hay un mensaje nuevo
    //el primer argumento es el semaforo, el segundo cuanto incremento, el tercero es una variable opcional del procedimiento
    ReleaseSemaphore(semLleno, 1, NULL);
  }
  return 0;
}


// Funcion Consumidor
DWORD WINAPI Consumidor(LPVOID lpParam) {
  while (true) {
    // hago un down(semLleno) para ver que haya por lo menos un mensaje en el buffer, si no espero
    WaitForSingleObject(semLleno, INFINITE);

    // hago down(mutex) para acceder solo al buffer
    WaitForSingleObject(mutex, INFINITE);
    int item = buffer.front();//obtengo item de buffer
    buffer.pop(); //borro el elemento que obtuve
    ReleaseMutex(mutex); //hago up(mutex) para marcar que esta disponible de nuevo

    // "Consumo" el elemeto que saque del buffer
    std::cout << "Consumidor: consumiendo elemento - " << item << std::endl;
    Sleep(1500); // Simulo espera para que sea legible lo que pasa en el programa

    // Hago un up(semVacio) para marcar que deje un espacio vacio en el buffer
    ReleaseSemaphore(semVacio, 1, NULL);
  }
  return 0;
}

//Cuerpo del programa, aca creo los semaforos, llamo a las funciones de consumidor y productot y las ejecuto en paralelo
int main() {
  // Creo los semaforos con lo que seria los init()
  //en el mutex, los null son objetos opcionales de windows y el FALSE es en que valor lo inicializo
  //en los contadores los null son los objetos opciones y los valores del medio el valor inicial / maximo (MAXIMO_BUFFER definido al inicio del programa)
  mutex = CreateMutex(NULL, FALSE, NULL);
  semVacio = CreateSemaphore(NULL, MAXIMO_BUFFER, MAXIMO_BUFFER, NULL);
  semLleno = CreateSemaphore(NULL, 0, MAXIMO_BUFFER, NULL);

  // Crea los hilos productor y consumidor para ejecutar concurrentemente (seria el parbegin / parend)
  HANDLE hiloProductor = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hiloConsumidor = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);

  // Espera la finalizacion de los hilos en x tiempo, en este caso lo dejamos en INFINITE para que sigan produciendo, si queremos dejarlo en un timepo cambioamos el INFINITE por valor en milisegundos
  WaitForSingleObject(hiloProductor, 5000);
  WaitForSingleObject(hiloConsumidor, 5000);

  // Cierro los hilos y los semaforos
  CloseHandle(hiloProductor);
  CloseHandle(hiloConsumidor);
  CloseHandle(mutex);
  CloseHandle(semVacio);
  CloseHandle(semLleno);

  return 0;
}
