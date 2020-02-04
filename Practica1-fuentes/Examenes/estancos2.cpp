// PRIMER EXAMEN DE PRÁCTICAS
// Juan Antonio Villegas Recio
/*
 * Crear sobre el arhivo fumadores.cpp una nueva hebra (proveedora) que genera 
 * lotes de ingredientes y los pone en un pale, cada lote encima de los anteriores
 * pero no puede haber nunca más de 3 lotes en el pale.
 * El estanquero por su parte coge el lote de ingredientes que está en la cima del
 * palé, elige (aleatoriamente) uno solo de los ingredientes del lote y ese es el 
 * que pone a la venta para los clientes, desechando los demás.
 * 
 * Nota: Mi solución está mal porque era necesario implementar el pale mediante
 * un vector de variables inutiles que no servian de nada y garantizar la EM en
 * el acceso a dicho vector mediante semaforos. Como el vector era inutil no lo
 * implementé aunque el comportamiento del programa es el deseado, así que me
 * quedé con un 7.
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

const int tam_pale=3,  
          nFumadores=3;

Semaphore   fumadores[nFumadores] = { 0, 0, 0 },
            mostrador = 1,
            estanquero = 0,
            proveedora = tam_pale;

mutex output;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra proveedora

void funcion_hebra_proveedora( )
{
    while (true)
    {
      sem_wait(proveedora);

      output.lock();
      cout << "***La proveedora ha puesto un lote en el pale***" << endl;
      output.unlock();

      sem_signal(estanquero);
    }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
    int ingrediente;
    while (true){
        sem_wait(estanquero);

        output.lock();
        cout << "Estanquero recoge un lote del pale" << endl;
        output.unlock();

        sem_signal(proveedora);

        ingrediente = aleatorio<0, nFumadores-1>();
        sem_wait(mostrador);

        output.lock();
        cout << "\tEstanquero pone a la venta el ingrediente " << ingrediente << endl;
        output.unlock();

        sem_signal(fumadores[ingrediente]);  
    }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
    output.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    output.unlock();
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
    output.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
    output.unlock();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true ){
      sem_wait(fumadores[num_fumador]);

      output.lock();
      cout << "\t\tEl fumador " << num_fumador << " retira su ingrediente" << endl;
      output.unlock();

      sem_signal(mostrador);
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
    thread hebraproveedora, hebraestanquero, hebrafumadores[nFumadores];
    hebraproveedora = thread ( funcion_hebra_proveedora );
    hebraestanquero = thread ( funcion_hebra_estanquero );
    for(int i = 0; i < nFumadores; i++)
        hebrafumadores[i] = thread (funcion_hebra_fumador, i);


    hebraproveedora.join();
    hebraestanquero.join();
    for(int i = 0; i < nFumadores; i++)
        hebrafumadores[i].join();
}
