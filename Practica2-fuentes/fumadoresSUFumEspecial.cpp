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
const int nFumadores = 4;



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
            nCigarrosEspecial;
        CondVar 
            estanquero,                     //Variable de condicion del estanquero
            fumadores[nFumadores];          //Variables de condicion para fumadores

    public:
        void ponerIngrediente (int i);      //Método para poner un ingrediente en el mostrador
        void esperarRecogidaIngrediente();  //Método que llama el estanquero para esperar se recoja un ingrediente
        void obtenerIngrediente (int i);    //Método que llama un fumador para obtener su ingrediente
        bool puedeFumar();
        Estanco();                          //Inicializacion
};

//----------------------------------------------------------------------
// Constuctor/Inicializador del monitor

Estanco :: Estanco(){
    mostrador = -1;
    estanquero = newCondVar();
    for (unsigned i = 0; i < nFumadores; i++)
        fumadores[i] = newCondVar();
    nCigarrosEspecial = 0;
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
    if(i==nFumadores-1){
        nCigarrosEspecial++;
        if(!puedeFumar()){
            output.lock();
            cout << "El fumador especial ha fumado ya mucho y prefiere controlarse" << endl;
            output.unlock();
        }
    }
    mostrador = -1;
    estanquero.signal();
}

bool Estanco :: puedeFumar(){
    return (nCigarrosEspecial%4==0 ? false : true);
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
    int ingrediente;
    while (true){
        ingrediente = producirIngrediente();
        output.lock();
        cout << "Estanquero produce ingrediente " << ingrediente << endl;
        output.unlock();
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
      cout << "\tEl fumador " << num_fumador << " retira su ingrediente" << endl;
      output.unlock();
      if(num_fumador!=nFumadores-1 || monitor->puedeFumar())
        fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
    MRef<Estanco> estanco = Create<Estanco>();

    thread  hebraEstanquero, hebrasFumadores[nFumadores];
    hebraEstanquero = thread( funcion_hebra_estanquero, estanco);
    for (int i = 0; i < nFumadores; i++)
        hebrasFumadores[i] = thread( funcion_hebra_fumador, estanco, i );

    hebraEstanquero.join();
    for (int i = 0; i < nFumadores; i++)
        hebrasFumadores[i].join();
}
