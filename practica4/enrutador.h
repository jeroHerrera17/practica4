
#ifndef ENRUTADOR_H
#define ENRUTADOR_H

#include <map>
#include <string>
using namespace std;

class Router {
public:
    int id;
    map<Router*, int> vecinos;

    Router(int id);
    void nuevoVecino(Router* vecino, int costo);
    void eliminarVecino(Router* vecino);

    // Métodos nuevos:
    string getNombre() const;                       // Devuelve "R" + id
    map<string, int> getTabla() const;              // Convierte vecinos a <string,int>
    void mostrarConexiones() const;

    // Métodos adicionales necesarios:
    void agregarConexion(const string& nombreVecino, int costo);
    void eliminarConexion(const string& nombreVecino);
    void mostrarTablaEnrutamiento(const map<string, pair<int, string>>& tabla) const;
};

#endif
