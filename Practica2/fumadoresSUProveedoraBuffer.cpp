/* Problema para practicar:
 * Consiste en el problema de los fumadores incluyendo una hebra proveedora de packs.
 * La proveedora y el estanquero comparten un buffer de M ingredientes, de forma que mientras
 * el buffer no esté completo la proveedora insertará un pack y mientras no esté
 * vacío el estanquero podrá sacar un pack y producir un ingrediente (Política FIFO).
 * Nota: Muy parecido al examen de la P1 pero implementado con monitores.
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

const int nFumadores = 3,
          M = 3;
int buffer[M];

mutex mtx;


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
// funcion que produce un ingrediente aleatorio
int producirIngrediente( )
{
    return aleatorio<0, nFumadores-1>();
}

//----------------------------------------------------------------------
// Declaración e implementación del monitor:

class Estanco : public HoareMonitor{
    private:
        int 
            mostrador,   //Variable que representa el ingrediente que ocupa el mostrador
            primera_libre;
        CondVar 
            estanquero,                     //Variable de condicion del estanquero
            fumadores[nFumadores],          //Variables de condicion para fumadores
            proveedora,
            estanqueroRepone;

    public:
        void reponer();
        void esperarReponer();
        void obtenerLote();
        void ponerIngrediente (int i);      //Método para poner un ingrediente en el mostrador
        void esperarRecogidaIngrediente();  //Método que llama el estanquero para esperar se recoja un ingrediente
        void obtenerIngrediente (int i);    //Método que llama un fumador para obtener su ingrediente
        Estanco();                          //Inicializacion
};

//----------------------------------------------------------------------
// Constuctor/Inicializador del monitor

Estanco :: Estanco(){
    mostrador = -1;
    primera_libre = 0;
    estanquero = newCondVar();
    proveedora = newCondVar();
    estanqueroRepone = newCondVar();
    for (unsigned i = 0; i < nFumadores; i++)
        fumadores[i] = newCondVar();
}

void Estanco :: reponer(){
    assert(primera_libre<M);
    buffer[primera_libre]=-1;   //-1 simboliza un lote
    primera_libre++;    
    estanqueroRepone.signal();
}

void Estanco :: esperarReponer(){
    if(primera_libre==M)
        proveedora.wait();
}


void Estanco :: obtenerLote(){
    int lote;
    if(primera_libre==0){
        proveedora.signal();
        estanqueroRepone.wait();
    }
    else
        proveedora.signal();
    
    assert(primera_libre>0);
    primera_libre--;
    lote = buffer[primera_libre];
}

//----------------------------------------------------------------------
// Función que llama el estanquero para poner un ingrediente generado en
// el mostrador

void Estanco :: ponerIngrediente(int i){
    mostrador = i;
    if(!fumadores[i].empty())
        fumadores[i].signal();
}

//----------------------------------------------------------------------
// Función que llama el estanquero después de poner un ingrediente generado
// para esperar a que este sea recogido

void Estanco :: esperarRecogidaIngrediente(){
    if(mostrador!=-1)
        estanquero.wait();
}

//----------------------------------------------------------------------
// Función que llaman los fumadores para obtener su ingrediente
// y acabar la espera del estanquero

void Estanco :: obtenerIngrediente (int i){
    if(mostrador!=i)
        fumadores[i].wait();
    mostrador = -1;
    estanquero.signal();
}


//----------------------------------------------------------------------
void funcion_hebra_proveedora( MRef<Estanco> monitor){
    while (true){
        monitor->esperarReponer();
        monitor->reponer();
        mtx.lock();
        cout << "\t\tLa proveedora repone un lote de ingredientes" << endl;
        mtx.unlock();
    }
}


//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
    int ingrediente;
    while (true){
        monitor->obtenerLote();
        mtx.lock();
        cout << "Estanquero obtiene un lote " << endl;
        mtx.unlock();
        ingrediente = producirIngrediente();
        mtx.lock();
        cout << "Estanquero produce ingrediente " << ingrediente << endl;
        mtx.unlock();
        monitor->ponerIngrediente(ingrediente);
        monitor->esperarRecogidaIngrediente();
    }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    mtx.unlock();
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
    mtx.unlock();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( MRef<Estanco> monitor, int num_fumador )
{
   while( true ){
      monitor->obtenerIngrediente(num_fumador);
      mtx.lock();
      cout << "\tEl fumador " << num_fumador << " retira su ingrediente" << endl;
      mtx.unlock();
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
    MRef<Estanco> estanco = Create<Estanco>();

    thread  hebraEstanquero, hebrasFumadores[nFumadores], hebraProveedora;
    hebraProveedora = thread( funcion_hebra_proveedora, estanco);
    hebraEstanquero = thread( funcion_hebra_estanquero, estanco);
    for (int i = 0; i < nFumadores; i++)
        hebrasFumadores[i] = thread( funcion_hebra_fumador, estanco, i );

    hebraProveedora.join();
    hebraEstanquero.join();
    for (int i = 0; i < nFumadores; i++)
        hebrasFumadores[i].join();
}
