// -----------------------------------------------------------------------------
// Sistemas concurrentes y Distribuidos.
// Seminario 1. Programación Multihebra y Semáforos.
// -----------------------------------------------------------------------------
// Juan Antonio Villegas Recio
// ejemplo9.cpp
// Compilar con la orden: g++ -std=c++11 ejemplo9.cpp -o ejemplo9 -lpthread
// -----------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <chrono>  // incluye now, time\_point, duration
#include <future>
#include <vector>
#include <cmath>
#include <cstring> // Para comparar strcmp()

using namespace std ;
using namespace std::chrono;

const long m  = 1024l*1024l*1024l,
           n  = 4 ;

bool ciclica;  //Variable booleana global que define el tipo de asignación según los argumentos


// -----------------------------------------------------------------------------
// evalua la función $f$ a integrar ($f(x)=4/(1+x^2)$)
double f( double x )
{
  return 4.0/(1.0+x*x) ;
}

// -----------------------------------------------------------------------------
// calcula la integral de forma secuencial, devuelve resultado:
double calcular_integral_secuencial(  )
{
   double suma = 0.0 ;                        // inicializar suma
   for( long i = 0 ; i < m ; i++ )            // para cada $i$ entre $0$ y $m-1$:
      suma += f( (i+double(0.5)) /m );         //   $~$ añadir $f(x_i)$ a la suma actual
   return suma/m ;                            // devolver valor promedio de $f$
}

// -----------------------------------------------------------------------------
// función que ejecuta cada hebra: recibe $i$ ==índice de la hebra, ($0\leq i<n$)

// Función que asigna las iteraciones de forma cíclica
// Se ejecuta sólo si ciclica=true
double funcion_hebra_ciclica( long i )
{
   double suma_parcial = 0.0;                              // Inicializar suma

   for( double x = (double)i/m; x < 1.0 ; x += (double)n/m )  // Asignación cíclica a las hebras
      suma_parcial += f(x)/m;                              // Añadir f(x)*(1/m)=f(x)/m a la suma parcial

   return suma_parcial;                                    // Devolver suma parcial 
}

// Función que asigna las iteraciones mediante bloques contiguos
// Se ejecuta sólo si ciclica=false
double funcion_hebra_bloques( long i )
{
   double suma_parcial = 0.0;                             // Inicializar suma

   for( double x = (double)i/n; x < (double)(i+1)/n; x += 1.0/m )  // Asignación a las hebras por bloques contiguos
      suma_parcial += f(x)/m;                              // Añadir f(x)*(1/m)=f(x)/m a la suma parcial

   return suma_parcial;                                    // Devolver suma parcial 
}

// -----------------------------------------------------------------------------
// calculo de la integral de forma concurrente
double calcular_integral_concurrente( )
{
   future<double> futuros[n];                   // Creación de un vector de tantos futuros como hebras
   double resultado = 0.0;                      // Inicialización del resultado

   if(ciclica)   
      for( int i = 0; i < n ; i++ )                
         futuros[i] = async( launch::async, funcion_hebra_ciclica, i );      // Cada hebra ejecuta funcion_hebra_ciclica
   else
      for( int i = 0; i < n ; i++ )                
         futuros[i] = async( launch::async, funcion_hebra_bloques, i );      // Cada hebra ejecuta funcion_hebra_bloques

   for( int i = 0; i < n ; i++ )
       resultado += futuros[i].get();           // Cuando cada hebra ha acabado, se suma el resultado de la ejecución
   
   return resultado;                            // Se devuelve la suma calculada
}
// -----------------------------------------------------------------------------



int main(int argc, char ** argv)
{
   //Comprobamos argumentos
   if( argc != 2 || ( strcmp(argv[1], "-c" )!=0 &&  strcmp(argv[1], "-b" )!=0 ) ){
      cout << "Error en los argumentos. Uso: " << endl <<
               argv[0] << " -c | -b" << endl <<
              "Use la opción \'-c\' para calcular pi mediante asignación cíclica." << endl <<
              "Use la opción \'-b\' para calcular pi mediante asignación por bloques." << endl;
      exit(-1);
   }

   //Asignamos true o false a ciclica según los argumentos
   if( !strcmp(argv[1], "-c" ) )
      ciclica = true;
   else 
      ciclica = false;
   
  // Ejecutamos y calculamos tiempos de ejecución secuencial y concurrente
  time_point<steady_clock> inicio_sec  = steady_clock::now() ;
  const double             result_sec  = calcular_integral_secuencial(  );
  time_point<steady_clock> fin_sec     = steady_clock::now() ;
  double x = sin(0.4567);
  time_point<steady_clock> inicio_conc = steady_clock::now() ;
  const double             result_conc = calcular_integral_concurrente(  );
  time_point<steady_clock> fin_conc    = steady_clock::now() ;
  duration<float,milli>    tiempo_sec  = fin_sec  - inicio_sec ,
                           tiempo_conc = fin_conc - inicio_conc ;
  const float              porc        = 100.0*tiempo_conc.count()/tiempo_sec.count() ;


  constexpr double pi = 3.14159265358979323846l ;

  //Imprimimos resultados
  if(ciclica)     cout << "\t\tAsignación cíclica:" << endl;
  else            cout << "\tAsignación por bloques contiguos:"<< endl;
  cout << "Número de muestras (m)   : " << m << endl
       << "Número de hebras (n)     : " << n << endl
       << setprecision(18)
       << "Valor de PI              : " << pi << endl
       << "Resultado secuencial     : " << result_sec  << endl
       << "Resultado concurrente    : " << result_conc << endl
       << setprecision(5)
       << "Tiempo secuencial        : " << tiempo_sec.count()  << " milisegundos. " << endl
       << "Tiempo concurrente       : " << tiempo_conc.count() << " milisegundos. " << endl
       << setprecision(4)
       << "Porcentaje t.conc/t.sec. : " << porc << "%" << endl;
}
