/* Examen P3:
 * Sobre el problema de los filósofos con camarero. En este caso cada filósofo al
 * solicitar sentarse envía al camarero un vector de tamaño aleatorio entre 1 y 5
 * (da igual el contenido, importa la capacidad), y el camarero debe recibir este
 * vector a modo de solicitud, almacenando la suma de los tamaños de los distintos
 * vectores de los distintos filosofos que se van sentando y cada vez que dicha suma
 * supera 10 unidades el camarero debe imprimir un mensaje diciendo:
 * "He alcanzado ya (...) unidades en total de propina!!!"
 * Por ejemplo, si recibe un vector de tamaño 3, uno de tamaño 5 y uno de tamaño 4
 * llevando 0 unidades de propina imprimirá
 * "He alcanzado ya 12 unidades en total de propina!!!"
 * Si después se reciben dos vectores de tamaño 5 se imprimirá (pues se ha superado el 20)
 * "He alcanzado ya 22 unidades en total de propina!!!"
 */ 

// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

#define ETIQ_SENTARSE 0
#define ETIQ_LEVANTARSE 1

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos  = 2*num_filosofos + 1,
   id_camarero   = num_procesos-1;


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

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
    int id_ten_izq = (id+1)              % (num_procesos-1), //id. tenedor izq.
        id_ten_der = (id+num_procesos-2) % (num_procesos-1), //id. tenedor der.
        id_camarero = num_procesos-1,
        valor = 0,
        tam      ;
    int * vec;

    while ( true )
    {
        // Vector de tamaño aleatorio
        tam = aleatorio<1, 5>();
        vec = new int[tam];

        // El filosofo solicita sentarse mediante el vector declarado
        cout << "Filosofo " << id/2 << " solicita sentarse" << endl;
        MPI_Ssend(&vec, tam, MPI_INT, id_camarero, ETIQ_SENTARSE, MPI_COMM_WORLD);

        delete vec;

        cout <<"Filósofo " << id/2 << " se sienta y solicita ten. izq." <<id_ten_izq <<endl;
        // ... solicitar tenedor izquierdo
        MPI_Ssend(&valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

        cout <<"Filósofo " << id/2 <<" solicita ten. der." <<id_ten_der <<endl;
        // ... solicitar tenedor derecho
        MPI_Ssend(&valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

        cout <<"Filósofo " << id/2 <<" comienza a comer" <<endl ;
        sleep_for( milliseconds( aleatorio<10,100>() ) );

        cout <<"Filósofo " << id/2 <<" suelta ten. izq. " <<id_ten_izq <<endl;
        // ... soltar el tenedor izquierdo
        MPI_Ssend(&valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

        cout<< "Filósofo " << id/2 <<" suelta ten. der. " <<id_ten_der <<endl;
        // ... soltar el tenedor derecho
        MPI_Ssend(&valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

        cout << " Filosofo " << id/2 << " se levanta de la mesa" << endl;
        MPI_Ssend(&valor, 1, MPI_INT, id_camarero, ETIQ_LEVANTARSE, MPI_COMM_WORLD);

        cout << "Filosofo " << id/2 << " comienza a pensar" << endl;
        sleep_for( milliseconds( aleatorio<10,100>() ) );
    }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
    int valor, id_filosofo ;  // valor recibido, identificador del filósofo
    MPI_Status estado ;       // metadatos de las dos recepciones

    while ( true )
    {
        // ...... recibir petición de cualquier filósofo (completar)
        MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado);
        // ...... guardar en 'id_filosofo' el id. del emisor (completar)
        id_filosofo = estado.MPI_SOURCE;
        cout <<"Ten. " << id <<" ha sido cogido por filo. " << id_filosofo/2 <<endl;

        // ...... recibir liberación de filósofo 'id_filosofo' (completar)
        MPI_Recv(&valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado);
        cout <<"Ten. "<< id << " ha sido liberado por filo. " << id_filosofo/2 <<endl ;
    }
}
// ---------------------------------------------------------------------

void funcion_camarero(){
    int valor,                  // Variable que se envia en caso de levantarse
        s               = 0,    // Numero de filosofos sentados
        etiq_aceptable     ,    // Etiqueta de peticion aceptable
        propinaTotal    = 0,    // Propina recibida en total
        propinaRecibida    ,    // Propina recibida en cada iteracion
        cuenta          = 1;    // Proxima decena de propina a superar

    MPI_Status estado;

    while (true)
    {
        // 1. Busco la etiqueta aceptable y si es un filosofo que quiere sentarse guardo metadatos del mensaje

        if(s<num_filosofos-1){          // Se pueden sentar filosofos
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);    // Sondeo el proximo mensaje
            if(estado.MPI_TAG == ETIQ_SENTARSE){                                // Si es un filosofo que quiere sentarse
                MPI_Get_count( &estado, MPI_INT, &propinaRecibida );            // Guardo el tamaño del vector que envia (propina)
                propinaTotal += propinaRecibida;                                // Incremento la propina total
            }
            etiq_aceptable = estado.MPI_TAG;                                    // Guardo la etiqueta del mensaje
        }
        else                            // Solo pueden levantarse filosofos
            etiq_aceptable = ETIQ_LEVANTARSE;

        // 2. Imprimo mensaje en pantalla si se cumple la condicion requerida

        if(propinaTotal > cuenta *10){      // Si se ha superado el proximo multiplo de 10 se imprime el mensaje
            cuenta++;
            cout << "\tCamarero: He alcanzado ya " << propinaTotal << " unidades en total de propina!!!" << endl;
        }

        // 3. Recibo el mensaje

        if(etiq_aceptable == ETIQ_SENTARSE){            // Si es un filosofo que quiere sentarse recibo el vector
            int vec[propinaRecibida];
            MPI_Recv ( &vec, propinaRecibida, MPI_INT, estado.MPI_SOURCE , etiq_aceptable, MPI_COMM_WORLD,&estado );
        }
        else                                            // Si es un filosofo que quiere levantarse recibo un valor
            MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_TAG, etiq_aceptable, MPI_COMM_WORLD, &estado);
        

        // 4. Actualizo el numero de filosofos sentados segun la etiqueta del mensaje recibido

        switch(estado.MPI_TAG){
            case ETIQ_SENTARSE:
                s++;
                break;
            case ETIQ_LEVANTARSE:
                s--;
                break;
        }
    }
}


// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
    int id_propio, num_procesos_actual ;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
    MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


    if ( num_procesos == num_procesos_actual )
    {
        // ejecutar la función correspondiente a 'id_propio'
        if ( id_propio == id_camarero)     // si es el último
            funcion_camarero();             //   es el camarero
        else if ( id_propio % 2 == 0 )      // si es par
            funcion_filosofos( id_propio ); //   es un filósofo
        else                               // si es impar
            funcion_tenedores( id_propio ); //   es un tenedor 
    }
    else
    {
        if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
        { cout  << "el número de procesos esperados es:    " << num_procesos << endl
                << "el número de procesos en ejecución es: " << num_procesos_actual << endl
                << "(programa abortado)" << endl ;
        }
    }

    MPI_Finalize( );
    return 0;
}

// ---------------------------------------------------------------------
