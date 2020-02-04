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
          M = 5;


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
            estanquero,                     //Variable de condicion del estanquero
            fumadores[nFumadores];          //Variables de condicion para fumadores

    public:
        void ponerIngrediente (int i);      //Método para poner un ingrediente en el mostrador
        void esperarRecogidaIngrediente();  //Método que llama el estanquero para esperar se recoja un ingrediente
        void obtenerIngrediente (int i);    //Método que llama un fumador para obtener su ingrediente
        Estanco();                          //Inicializacion
};

//----------------------------------------------------------------------
// Constuctor/Inicializador del monitor

Estanco :: Estanco(){
    mostrador = -1;
    estanquero = newCondVar();
    for (unsigned i = 0; i < nFumadores; i++)
        fumadores[i] = newCondVar();
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

//--------------------------------------------------------------------
// Monitor para la proveedora

class Proveedor : public HoareMonitor{
    private:
        int restantes;
    CondVar
        proveedora,
        estanquero;
    public:
        void reponer();
        void esperarReponer();
        void obtenerGastarRecursos();
        Proveedor();
};

void Proveedor :: reponer(){
    restantes = M;
    estanquero.signal();
}

void Proveedor :: esperarReponer(){
    if(restantes > 0)
        proveedora.wait();
}

void Proveedor :: obtenerGastarRecursos(){
    restantes--;
    if (restantes == 0){
        proveedora.signal();
        estanquero.wait();
    }
}

Proveedor :: Proveedor(){
    restantes = 0;
    estanquero = newCondVar();
    proveedora = newCondVar();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del proveedor
void funcion_hebra_proveedor( MRef<Proveedor> proveedor ){
    while (true){
        proveedor->esperarReponer();
        cout << "\t\tLa proveedora repone " << M << " ingredientes" << endl;
        proveedor->reponer();
    }
    
}


//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> estanco, MRef<Proveedor> proveedor )
{
    int ingrediente;
    while (true){
        proveedor->obtenerGastarRecursos();
        ingrediente = producirIngrediente();
        cout << "Estanquero produce ingrediente " << ingrediente << endl;
        estanco->ponerIngrediente(ingrediente);
        estanco->esperarRecogidaIngrediente();
    }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( MRef<Estanco> estanco, int num_fumador )
{
   while( true ){
      estanco->obtenerIngrediente(num_fumador);
      cout << "\tEl fumador " << num_fumador << " retira su ingrediente" << endl;
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
    MRef<Estanco> estanco = Create<Estanco>();
    MRef<Proveedor> proveedor = Create<Proveedor>();

    thread hebraProveedor, hebraEstanquero, hebrasFumadores[nFumadores];
    hebraProveedor  = thread( funcion_hebra_proveedor, proveedor );
    hebraEstanquero = thread( funcion_hebra_estanquero, estanco, proveedor );
    for (int i = 0; i < nFumadores; i++)
        hebrasFumadores[i] = thread( funcion_hebra_fumador, estanco, i );

    hebraProveedor.join();
    hebraEstanquero.join();
    for (int i = 0; i < nFumadores; i++)
        hebrasFumadores[i].join();
}
