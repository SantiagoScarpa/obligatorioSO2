#include <windows.h> //libreria para windows para la utilizacion de semaforos
#include <iostream>
#include <queue>//libreria para la utilizacion de la estructura de cola para el buffer (FIFO)
using namespace std;


#define MAXIMO_BUFFER 5 //defino el tama;o del buffer

//HANDLE hace referencia a la utilizacion de objetos del kernel que se utilizaran como semaforos
HANDLE mutex;
HANDLE semVacio;
HANDLE semLleno;

int thread_id = 0 ; //variable global de thread ID, para identificar diferentes productores/consumidores en test
int msSleepProd = 1000; //variable global para el sleep de productor
int msSleepCons = 1000; //variable global para el sleep de consumidor

// Queue de int para el buffer, la misma es FIFO
queue<int> buffer;

//Funcion productora
//DWORD es un tipo de dato de windows que se utiliza como INT, utilizamos esto en vez de int, porque el manejo de procesos concurrentes lo espera. Este tiene una mejor compatibilidad y consistencia que INT en windows
//WINAPI es utilizado para procesos que van a correr en hilo de windows, tiene una serie de llamadas estandarizadas para llamadas del sistema de windows
//LPVOID, al igual que DWORD es un tipo de dato que es necesario para la programacion en hilos de windows, este es un puntero .
DWORD WINAPI Productor(LPVOID lpParam) {
  int item = 1;
  int pid = thread_id; //ID del productor a mostrar en consola
  thread_id++;

  cout << "\n"<<endl;
  //Creo elementos mientras sea posible
  while (true) {
    cout << "Productor ID-"<<pid<<" : creando elemento - " << item <<endl;

    Sleep(msSleepProd); // Simulo una espera para que sea legible en ejecucion

    // Es una funcion de windows.h que espera un objeto HANDLE el cual debe estar disponible, en este caso, el semaforo de vacia para marcar que hay espacio en el buffer
    //el INFINITE hace referencia a cuandos milisegundos debe esperar por este semaforo, se pone infinite ya que no se desea que de time out
    //es basicamente hacer el down(semVacio)
    WaitForSingleObject(semVacio, INFINITE);

    //down(mutex) para la mutua exclusion del buffer
    WaitForSingleObject(mutex, INFINITE);
    buffer.push(item);
    item++;

    //up(mutex)
    ReleaseMutex(mutex);

    //Up(semLlano) para marcar que hay un mensaje nuevo
    //el primer argumento es el semaforo, el segundo cuanto incremento, el tercero es una variable opcional del procedimiento
    ReleaseSemaphore(semLleno, 1, NULL);
  }
  return 0;
}


// Funcion Consumidor
DWORD WINAPI Consumidor(LPVOID lpParam) {
  int pid = thread_id; //ID del consumidor
  thread_id++;

  while (true) {
    // down(semLleno) para ver que haya por lo menos un mensaje en el buffer, si no espera
    WaitForSingleObject(semLleno, INFINITE);

    //down(mutex) para acceder solo al buffer
    WaitForSingleObject(mutex, INFINITE);
    int item = buffer.front();//obtengo item de buffer
    buffer.pop(); //borro el elemento que obtuve
    ReleaseMutex(mutex); //up(mutex)

    // "Consumo" el elemeto que saque del buffer
    cout << "Consumidor ID-"<<pid<<" : consumiendo elemento - " << item << endl;
    Sleep(msSleepCons); // Simulo espera con variable

    // Hago un up(semVacio) para marcar que deje un espacio vacio en el buffer
    ReleaseSemaphore(semVacio, 1, NULL);
  }
  return 0;
}

void ejecucionNormal(){
  // Crea los hilos productor y consumidor para ejecutar concurrentemente (seria el parbegin / parend)
  HANDLE hiloProductor = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hiloConsumidor = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);

  // Espera la finalizacion de los hilos en x tiempo, en este caso lo dejamos en INFINITE para que sigan produciendo, si queremos dejarlo en un timepo cambioamos el INFINITE por valor en milisegundos
  WaitForSingleObject(hiloProductor, INFINITE);
  WaitForSingleObject(hiloConsumidor, INFINITE);

  // Cierro los hilos y los semaforos
  CloseHandle(hiloProductor);
  CloseHandle(hiloConsumidor);
}


//A continuacion dejo algos procedimientos para hacer preubas con diferentes combinaciones de Productores - Consumidores
//Test de 2 productores 1 consumidor
void testProcedure2_1(){
  HANDLE hiloProductor1 = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hiloProductor2 = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hiloConsumidor = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);

  WaitForSingleObject(hiloProductor1, INFINITE);
  WaitForSingleObject(hiloProductor2, INFINITE);
  WaitForSingleObject(hiloConsumidor, INFINITE);

  CloseHandle(hiloProductor1);
  CloseHandle(hiloProductor2);
  CloseHandle(hiloConsumidor);
}

//Test de 1 productor 0 consumidores
void testProcedure1_0(){
  HANDLE hiloProductor1 = CreateThread(NULL, 0, Productor, NULL, 0, NULL);

  WaitForSingleObject(hiloProductor1, INFINITE);

  CloseHandle(hiloProductor1);
}

//Test de 0 productores 1 consumidor
void testProcedure0_1(){
  HANDLE hiloConsumidor = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);

  WaitForSingleObject(hiloConsumidor, INFINITE);

  CloseHandle(hiloConsumidor);
}

//Test de 1 productor 2 consumidores
void testProcedure1_2(){
  HANDLE hiloProductor1 = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hiloConsumidor1 = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);
  HANDLE hiloConsumidor2 = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);

  WaitForSingleObject(hiloProductor1, INFINITE);
  WaitForSingleObject(hiloConsumidor1, INFINITE);
  WaitForSingleObject(hiloConsumidor2, INFINITE);

  CloseHandle(hiloProductor1);
  CloseHandle(hiloConsumidor1);
  CloseHandle(hiloConsumidor2);
}

//Test de 2 productores 2 consumidores
void testProcedure2_2(){
  HANDLE hiloProductor1 = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hiloProductor2 = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hiloConsumidor1 = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);
  HANDLE hiloConsumidor2 = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);

  WaitForSingleObject(hiloProductor1, INFINITE);
  WaitForSingleObject(hiloProductor2, INFINITE);
  WaitForSingleObject(hiloConsumidor1, INFINITE);
  WaitForSingleObject(hiloConsumidor2, INFINITE);

  CloseHandle(hiloProductor1);
  CloseHandle(hiloProductor2);
  CloseHandle(hiloConsumidor1);
  CloseHandle(hiloConsumidor2);
}

//Cambio tiempo de variables de sleep para Productor
void testProcedureTiemposP(){
  msSleepProd = 500;
  msSleepCons = 2000;
  HANDLE hiloProductor = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hiloConsumidor = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);

  WaitForSingleObject(hiloProductor, INFINITE);
  WaitForSingleObject(hiloConsumidor, INFINITE);

  CloseHandle(hiloProductor);
  CloseHandle(hiloConsumidor);
}

//Cambio tiempo de variables de sleep para Consumidor
void testProcedureTiemposC(){
  msSleepProd = 2000;
  msSleepCons = 500;
  HANDLE hiloProductor = CreateThread(NULL, 0, Productor, NULL, 0, NULL);
  HANDLE hiloConsumidor = CreateThread(NULL, 0, Consumidor, NULL, 0, NULL);

  WaitForSingleObject(hiloProductor, INFINITE);
  WaitForSingleObject(hiloConsumidor, INFINITE);

  CloseHandle(hiloProductor);
  CloseHandle(hiloConsumidor);
}


//Cuerpo del programa, aca creo los semaforos, llamo a las funciones de consumidor y productot y las ejecuto en paralelo
int main() {
    cout<<"INICIO APPLICACION PRODUCTOR - CONSUMIDOR CON SEMAFOROS "<<endl;

  // Creo los semaforos con lo que seria los init()
  //en el mutex, los null son objetos opcionales de windows y el FALSE es en que valor lo inicializo
  //en los contadores los null son los objetos opciones y los valores del medio el valor inicial / maximo (MAXIMO_BUFFER definido al inicio del programa)
  mutex = CreateMutex(NULL, FALSE, NULL);
  semVacio = CreateSemaphore(NULL, MAXIMO_BUFFER, MAXIMO_BUFFER, NULL);
  semLleno = CreateSemaphore(NULL, 0, MAXIMO_BUFFER, NULL);

  //seteamos la llamada de los hilos en un procedimiento aparte para poder hacer diferentes pruebas
  ejecucionNormal();


  //Ahora se llaman diferente procedures para hacer test con diferentes variables, en caso de querer correr uno o el otro, comentar el "ejecucionNormal" y los demas Test procedure
  //testProcedure2_1(); //2 Prod 1 Consu

  //testProcedure1_0(); // 1 Prod 0 Consu

  //testProcedure0_1(); // 0 Prod 1 Consu

  //testProcedure1_2(); // 1 Prod 2 Consu

  //testProcedure2_2(); // 2 Prod 2 consu

  //testProcedureTiemposP(); //Productor mas rapido que consumidor
  //testProcedureTiemposC(); //Consumidor mas rapido que Productor

  CloseHandle(mutex);
  CloseHandle(semVacio);
  CloseHandle(semLleno);

  return 0;
}
