#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

mutex output;
const int nFumadores = 3,
          nEstanqueros =2;



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
            mostrador;   //Variable que representa el ingrediente que ocupa el mostrador
        CondVar 
            estanqueros,                     //Variable de condicion de los estanqueros
            fumadores[nFumadores];          //Variables de condicion para fumadores

    public:
        void ponerIngrediente ( int ingrediente, int nEstanquero );      //Método para poner un ingrediente en el mostrador
        void esperarRecogidaIngrediente();  //Método que llama el estanquero para esperar se recoja un ingrediente
        void obtenerIngrediente (int nFumador);    //Método que llama un fumador para obtener su ingrediente
        Estanco();                          //Inicializacion
};

//----------------------------------------------------------------------
// Constuctor/Inicializador del monitor

Estanco :: Estanco(){
    mostrador=-1;
    estanqueros = newCondVar();
    for (unsigned i = 0; i < nFumadores; i++)
        fumadores[i] = newCondVar();
}

//----------------------------------------------------------------------
// Función que llama el estanquero para poner un ingrediente generado en
// el mostrador

void Estanco :: ponerIngrediente( int ingrediente, int nEstanquero ){
    if (mostrador!=-1)
        estanqueros.wait();
    
    mostrador = ingrediente;
    if(!fumadores[ingrediente].empty())
        fumadores[ingrediente].signal();
}

//----------------------------------------------------------------------
// Función que llama el estanquero después de poner un ingrediente generado
// para esperar a que este sea recogido

void Estanco :: esperarRecogidaIngrediente(){
    while(mostrador!=-1)
        estanqueros.wait();
}

//----------------------------------------------------------------------
// Función que llaman los fumadores para obtener su ingrediente
// y acabar la espera del estanquero

void Estanco :: obtenerIngrediente (int nFumador){
    while(mostrador!=nFumador)
        fumadores[nFumador].wait();

    assert(mostrador==nFumador);
    
    mostrador = -1;
    estanqueros.signal();
}


//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> monitor, int nEstanquero )
{
    int ingrediente;
    while (true){
        ingrediente = producirIngrediente();
        output.lock();
        cout << "Estanquero " << nEstanquero << " produce ingrediente " << ingrediente << endl;
        output.unlock();
        monitor->ponerIngrediente(ingrediente, nEstanquero);
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
void  funcion_hebra_fumador( MRef<Estanco> monitor, int num_fumador )
{
    while( true ){
        monitor->obtenerIngrediente(num_fumador);
        output.lock();
        cout << "\tEl fumador " << num_fumador << " retira su ingrediente " << endl;
        output.unlock();
        fumar(num_fumador);
    }
}

//----------------------------------------------------------------------

int main()
{
    MRef<Estanco> estanco = Create<Estanco>();

    thread  hebrasEstanqueros[nEstanqueros], hebrasFumadores[nFumadores];
    for (int i = 0; i < nEstanqueros; i++)
        hebrasEstanqueros[i] = thread( funcion_hebra_estanquero, estanco, i );
    for (int i = 0; i < nFumadores; i++)
        hebrasFumadores[i] = thread( funcion_hebra_fumador, estanco, i );

    for (int i = 0; i < nEstanqueros; i++)
        hebrasEstanqueros[i].join();
    for (int i = 0; i < nFumadores; i++)
        hebrasFumadores[i].join();
}
