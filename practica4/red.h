
#ifndef RED_H
#define RED_H

#include "enrutador.h"
#include <vector>
#include <string>
//#include <utility>
//#include <iostream>
//#include <queue>
//#include <climits>
//#include <iomanip>

class Red {
private:
    std::vector<Router*> enrutadores; // Lista de enrutadores de la red
    std::string rutaArchivo;          // Ruta del archivo de guardado (opcional)

public:
    // ===========================
    // Constructores y destructor
    // ===========================
    Red();                       // Constructor vacío
    Red(int cantidad);            // Constructor con número de enrutadores
    ~Red();                       // Destructor (libera memoria)

    // ===========================
    // Funciones principales
    // ===========================
    void generarRedAleatoria();   // Genera una red con enlaces aleatorios
    void mostrarRed() const;      // Muestra la matriz de costos mínimos entre todos los enrutadores

    void guardarEnArchivo(const std::string& nombreArchivo) const; // Guarda la red en un archivo
    void cargarDesdeArchivo(const std::string& nombreArchivo);     // Carga la red desde un archivo

    void calcularRutaMasCorta(int origen, int destino);            // Aplica Dijkstra entre dos enrutadores
    void mostrarTablasDeEnrutamiento();                            // Muestra la tabla de enrutamiento de cada enrutador

    // ===========================
    // Gestión de enrutadores
    // ===========================
    void agregarEnrutador();       // Agrega un nuevo enrutador
    void eliminarEnrutador(int id); // Elimina un enrutador y sus enlaces

    // ===========================
    // Gestión de enlaces
    // ===========================
    void agregarEnlace();          // Agrega un enlace entre dos enrutadores
    void eliminarEnlace();         // Elimina un enlace entre dos enrutadores
};

#endif // RED_H
