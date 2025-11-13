
#include "enrutador.h"
#include <iostream>
#include <iomanip>
using namespace std;

// ============================
// Constructor
// ============================
Router::Router(int id) : id(id) {}

// ============================
// Métodos básicos
// ============================
void Router::nuevoVecino(Router* vecino, int costo) {
    if (!vecino || vecino == this) return; // evita punteros nulos o bucles
    vecinos[vecino] = costo;
}

void Router::eliminarVecino(Router* vecino) {
    if (!vecino) return;
    vecinos.erase(vecino);
}

// ============================
// Métodos nuevos
// ============================
string Router::getNombre() const {
    return "R" + to_string(id);
}

map<string, int> Router::getTabla() const {
    map<string, int> tabla;
    for (auto& [v, c] : vecinos)
        if (v) tabla[v->getNombre()] = c;
    return tabla;
}

void Router::mostrarConexiones() const {
    cout << getNombre() << " -> ";
    if (vecinos.empty()) {
        cout << "sin conexiones";
    } else {
        bool first = true;
        for (auto& [v, c] : vecinos) {
            if (!first) cout << ", ";
            cout << v->getNombre() << "(" << c << ")";
            first = false;
        }
    }
    cout << endl;
}

// ============================
// Métodos adicionales
// ============================
void Router::agregarConexion(const string& nombreVecino, int costo) {
    // este método solo tiene sentido si ya existen objetos Router reales
    // por eso normalmente no se usa fuera de Red
    cout << " [aviso] agregarConexion(" << nombreVecino << ") debe hacerse desde Red.\n";
}

void Router::eliminarConexion(const string& nombreVecino) {
    for (auto it = vecinos.begin(); it != vecinos.end(); ) {
        if (it->first && it->first->getNombre() == nombreVecino)
            it = vecinos.erase(it);
        else
            ++it;
    }
}

void Router::mostrarTablaEnrutamiento(const map<string, pair<int, string>>& tabla) const {
    cout << "\nTabla de enrutamiento de " << getNombre() << ":\n";
    cout << left << setw(10) << "Destino" << setw(10) << "Costo" << "Siguiente salto\n";
    cout << string(40, '-') << "\n";
    for (auto& [dest, info] : tabla)
        cout << setw(10) << dest << setw(10) << info.first << info.second << "\n";
}
