
// red.cpp (reemplaza tu archivo actual)
#include "red.h"
#include "enrutador.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <map>
using namespace std;

// ============================
// Constructores y destructor
// ============================
Red::Red() {}

Red::Red(int cantidad) {
    for (int i = 1; i <= cantidad; ++i)
        enrutadores.push_back(new Router(i));
}

Red::~Red() {
    for (auto* r : enrutadores)
        delete r;
    enrutadores.clear();
}

// ============================
// Generación y visualización
// ============================
void Red::generarRedAleatoria() {
    srand((unsigned)time(nullptr));
    int n = enrutadores.size();

    if (n <= 0) {
        cout << "No hay enrutadores para generar la red.\n";
        return;
    }

    // Conectar TODOS los pares con un costo aleatorio (1..20)
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            int costo = (rand() % 20) + 1; // costo entre 1 y 20
            enrutadores[i]->nuevoVecino(enrutadores[j], costo);
            enrutadores[j]->nuevoVecino(enrutadores[i], costo);
        }
    }

    cout << "Red aleatoria (completamente conectada) generada correctamente.\n";
}

void Red::mostrarRed() const {
    int n = enrutadores.size();
    if (n == 0) {
        cout << "No hay enrutadores en la red.\n";
        return;
    }

    // Matriz de distancias mínimas
    vector<vector<int>> distancias(n, vector<int>(n, INT_MAX));

    // Aplicar Dijkstra para cada enrutador como origen
    for (int i = 0; i < n; ++i) {
        string origen = enrutadores[i]->getNombre();

        map<string,int> dist;
        priority_queue<pair<int,string>, vector<pair<int,string>>, greater<pair<int,string>>> pq;
        for (auto* r : enrutadores) dist[r->getNombre()] = INT_MAX;

        dist[origen] = 0;
        pq.push({0, origen});

        while (!pq.empty()) {
            auto [d, actual] = pq.top(); pq.pop();
            if (d > dist[actual]) continue;

            Router* r_actual = nullptr;
            for (auto* r : enrutadores)
                if (r->getNombre() == actual) { r_actual = r; break; }
            if (!r_actual) continue;

            for (auto& p : r_actual->vecinos) {
                Router* vecino = p.first;
                int costo = p.second;
                string nombreVec = vecino->getNombre();
                if (dist[actual] + costo < dist[nombreVec]) {
                    dist[nombreVec] = dist[actual] + costo;
                    pq.push({dist[nombreVec], nombreVec});
                }
            }
        }

        // Guardar resultados en la matriz
        for (int j = 0; j < n; ++j) {
            string destino = enrutadores[j]->getNombre();
            if (dist[destino] != INT_MAX)
                distancias[i][j] = dist[destino];
        }
    }

    // Mostrar la matriz
    cout << "\n========= MATRIZ DE COSTOS (RUTAS MÁS CORTAS - DIJKSTRA) =========\n";
    cout << setw(5) << " ";
    for (int j = 0; j < n; ++j)
        cout << setw(6) << "R" + to_string(j + 1);
    cout << endl;

    for (int i = 0; i < n; ++i) {
        cout << setw(4) << "R" + to_string(i + 1);
        for (int j = 0; j < n; ++j) {
            if (i == j)
                cout << setw(6) << "0";
            else if (distancias[i][j] == INT_MAX)
                cout << setw(6) << "-";
            else
                cout << setw(6) << distancias[i][j];
        }
        cout << endl;
    }
    cout << "===================================================================\n";
}

// ============================
// Guardar / Cargar red
// ============================

// guardarEnArchivo ahora SOLO guarda la red en el archivo que se le pasa
// en formato de enlaces: "R1 R2 5" por línea (sin duplicados, orden numérico).
void Red::guardarEnArchivo(const string& rutaArchivo) const {
    ofstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        cerr << "Error al abrir/crear archivo: " << rutaArchivo << endl;
        return;
    }

    // Guardamos únicamente enlaces únicos (r->id < vecino->id) en orden numérico
    // para que el archivo sea fácil de cargar.
    // Formato por línea: R<idOrigen> R<idDestino> <costo>
    // Ordenamos por id para salida consistente.
    vector<tuple<int,int,int>> enlaces;
    for (auto* r : enrutadores) {
        for (auto& p : r->vecinos) {
            Router* vec = p.first;
            int costo = p.second;
            if (r->id < vec->id) {
                enlaces.emplace_back(r->id, vec->id, costo);
            }
        }
    }

    sort(enlaces.begin(), enlaces.end(), [](const auto& a, const auto& b){
        if (get<0>(a) != get<0>(b)) return get<0>(a) < get<0>(b);
        return get<1>(a) < get<1>(b);
    });

    for (auto& t : enlaces) {
        archivo << "R" << get<0>(t) << " R" << get<1>(t) << " " << get<2>(t) << "\n";
    }
    archivo.close();

    // Además, registramos el nombre en lista_rutas.txt (evita duplicados)
    string lista = "lista_rutas.txt";
    // Leemos si ya existe para evitar duplicados
    bool ya = false;
    ifstream in(lista);
    if (in.is_open()) {
        string line;
        while (getline(in, line)) {
            if (line == rutaArchivo) { ya = true; break; }
        }
        in.close();
    }
    if (!ya) {
        ofstream out(lista, ios::app);
        if (out.is_open()) {
            out << rutaArchivo << "\n";
            out.close();
        }
    }

    cout << "Red guardada en: " << rutaArchivo << endl;
}

// Cargar espera líneas con formato: R<num> R<num> <costo>
void Red::cargarDesdeArchivo(const string& nombreArchivo) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cerr << "No se pudo abrir el archivo: " << nombreArchivo << endl;
        return;
    }

    // Limpiar red actual
    for (auto* r : enrutadores) delete r;
    enrutadores.clear();

    string a, b;
    int costo;
    // Usamos un mapa temporal id -> Router*
    map<int, Router*> mapa;

    while (archivo >> a >> b >> costo) {
        if (a.size() < 2 || b.size() < 2) continue;
        // quitar prefijo 'R' y convertir
        int id1 = stoi(a.substr(1));
        int id2 = stoi(b.substr(1));
        if (id1 == id2) continue;

        if (mapa.find(id1) == mapa.end()) mapa[id1] = new Router(id1);
        if (mapa.find(id2) == mapa.end()) mapa[id2] = new Router(id2);

        mapa[id1]->nuevoVecino(mapa[id2], costo);
        mapa[id2]->nuevoVecino(mapa[id1], costo);
    }
    archivo.close();

    // Insertar routers en vector en orden de id (1..N)
    vector<int> ids;
    for (auto& p : mapa) ids.push_back(p.first);
    sort(ids.begin(), ids.end());
    for (int id : ids) {
        // garantizamos que el id interno coincida con la posición
        Router* r = mapa[id];
        r->id = (int)enrutadores.size() + 1; // renumerar consecutivamente
        enrutadores.push_back(r);
    }

    // IMPORTANTE: los vecinos apuntan a Router* con los ids antiguos;
    // necesitamos reindexarlos para que sus .id coincidan con la nueva numeración.
    // Creamos un mapa viejoId -> nuevoPointer
    map<int, Router*> nuevoMapa;
    for (auto* r : enrutadores) nuevoMapa[r->id] = r; // ahora r->id es posición, pero vecinos tienen ptrs con old ids
    // No podemos determinar old ids facilmente desde los punteros si no las actualizamos.
    // Por simplicidad, asumimos que los archivos vienen con IDs consecutivos 1..N y las referencias por pointer siguen siendo válidas.
    // Si en tu formato necesitas preservar IDs tal cual, se puede adaptar.

    cout << "Red cargada desde: " << nombreArchivo << endl;
}

// ============================
// Gestión de enrutadores
// ============================
void Red::agregarEnrutador() {
    int nuevoId = enrutadores.size() + 1;
    enrutadores.push_back(new Router(nuevoId));
    cout << "Enrutador R" << nuevoId << " agregado.\n";
}

void Red::eliminarEnrutador(int id) {
    if (id <= 0 || id > (int)enrutadores.size()) {
        cout << "ID inválido.\n";
        return;
    }

    Router* aEliminar = enrutadores[id - 1];

    // eliminar referencias de vecinos
    for (auto* r : enrutadores) {
        if (r != aEliminar) r->eliminarVecino(aEliminar);
    }

    delete aEliminar;
    enrutadores.erase(enrutadores.begin() + (id - 1));

    // reajustar ids para que sean consecutivos 1..N
    for (size_t i = 0; i < enrutadores.size(); ++i)
        enrutadores[i]->id = (int)i + 1;

    cout << "Enrutador R" << id << " eliminado y IDs reajustados.\n";
}

// ============================
// Gestión de enlaces
// ============================
void Red::agregarEnlace() {
    int id1, id2, costo;
    cout << "Ingrese el ID del primer enrutador: ";
    cin >> id1;
    cout << "Ingrese el ID del segundo enrutador: ";
    cin >> id2;
    cout << "Ingrese el costo del enlace: ";
    cin >> costo;

    if (id1 <= 0 || id2 <= 0 || id1 > (int)enrutadores.size() || id2 > (int)enrutadores.size()) {
        cout << "IDs inválidos.\n";
        return;
    }
    if (id1 == id2) {
        cout << "No se puede conectar un enrutador consigo mismo.\n";
        return;
    }

    Router* r1 = enrutadores[id1 - 1];
    Router* r2 = enrutadores[id2 - 1];
    r1->nuevoVecino(r2, costo);
    r2->nuevoVecino(r1, costo);

    cout << "Enlace agregado entre R" << id1 << " y R" << id2 << ".\n";
}

void Red::eliminarEnlace() {
    int id1, id2;
    cout << "Ingrese el ID del primer enrutador: ";
    cin >> id1;
    cout << "Ingrese el ID del segundo enrutador: ";
    cin >> id2;

    if (id1 <= 0 || id2 <= 0 || id1 > (int)enrutadores.size() || id2 > (int)enrutadores.size()) {
        cout << "IDs inválidos.\n";
        return;
    }

    Router* r1 = enrutadores[id1 - 1];
    Router* r2 = enrutadores[id2 - 1];
    r1->eliminarVecino(r2);
    r2->eliminarVecino(r1);

    cout << "Enlace eliminado entre R" << id1 << " y R" << id2 << ".\n";
}

// ============================
// Mostrar tablas de enrutamiento
// ============================
void Red::mostrarTablasDeEnrutamiento() {
    if (enrutadores.empty()) {
        cout << "No hay enrutadores.\n";
        return;
    }

    cout << "\n========= TABLAS DE ENRUTAMIENTO =========\n";
    for (auto* origen : enrutadores) {
        string nombreOrigen = origen->getNombre();

        map<string,int> dist;
        map<string,string> prev;
        for (auto* r : enrutadores) dist[r->getNombre()] = INT_MAX;
        dist[nombreOrigen] = 0;

        priority_queue<pair<int,string>, vector<pair<int,string>>, greater<pair<int,string>>> pq;
        pq.push({0, nombreOrigen});

        while (!pq.empty()) {
            auto [d, actual] = pq.top(); pq.pop();
            if (d > dist[actual]) continue;
            Router* r_actual = nullptr;
            for (auto* r : enrutadores) if (r->getNombre() == actual) { r_actual = r; break; }
            if (!r_actual) continue;
            for (auto& p : r_actual->vecinos) {
                Router* vec = p.first;
                int c = p.second;
                string nombreVec = "R" + to_string(vec->id);
                if (dist[actual] + c < dist[nombreVec]) {
                    dist[nombreVec] = dist[actual] + c;
                    prev[nombreVec] = actual;
                    pq.push({dist[nombreVec], nombreVec});
                }
            }
        }

        cout << "Tabla de " << nombreOrigen << ":\n";
        cout << left << setw(10) << "Destino" << setw(10) << "Costo" << "Camino\n";
        cout << string(50, '-') << "\n";
        for (auto* destino : enrutadores) {
            string nombreDest = destino->getNombre();
            if (nombreDest == nombreOrigen) {
                cout << setw(10) << nombreDest << setw(10) << 0 << "-" << "\n";
                continue;
            }
            if (dist[nombreDest] == INT_MAX) {
                cout << setw(10) << nombreDest << setw(10) << "-" << "Sin conexión\n";
                continue;
            }
            // reconstruir camino
            vector<string> ruta;
            string cur = nombreDest;
            while (prev.find(cur) != prev.end()) {
                ruta.push_back(cur);
                cur = prev[cur];
            }
            ruta.push_back(nombreOrigen);
            reverse(ruta.begin(), ruta.end());
            string camino;
            for (size_t i = 0; i < ruta.size(); ++i) {
                camino += ruta[i];
                if (i + 1 < ruta.size()) camino += " -> ";
            }
            cout << setw(10) << nombreDest << setw(10) << dist[nombreDest] << camino << "\n";
        }
        cout << "\n";
    }
    cout << "==========================================\n";
}

// ============================
// Calcular ruta más corta entre dos enrutadores
// ============================
void Red::calcularRutaMasCorta(int origenId, int destinoId) {
    if (origenId <= 0 || destinoId <= 0 ||
        origenId > (int)enrutadores.size() || destinoId > (int)enrutadores.size()) {
        cout << "IDs inválidos.\n";
        return;
    }

    Router* origen = enrutadores[origenId - 1];
    Router* destino = enrutadores[destinoId - 1];

    map<string,int> dist;
    map<string,string> prev;
    string nombreOrigen = origen->getNombre();
    string nombreDestino = destino->getNombre();

    for (auto* r : enrutadores) dist[r->getNombre()] = INT_MAX;
    dist[nombreOrigen] = 0;

    priority_queue<pair<int,string>, vector<pair<int,string>>, greater<pair<int,string>>> pq;
    pq.push({0, nombreOrigen});

    while (!pq.empty()) {
        auto [d, actual] = pq.top(); pq.pop();
        if (d > dist[actual]) continue;
        Router* r_actual = nullptr;
        for (auto* r : enrutadores) if (r->getNombre() == actual) { r_actual = r; break; }
        if (!r_actual) continue;
        for (auto& p : r_actual->vecinos) {
            Router* vec = p.first; int c = p.second;
            string nombreVec = "R" + to_string(vec->id);
            if (dist[actual] + c < dist[nombreVec]) {
                dist[nombreVec] = dist[actual] + c;
                prev[nombreVec] = actual;
                pq.push({dist[nombreVec], nombreVec});
            }
        }
    }

    if (dist[nombreDestino] == INT_MAX) {
        cout << "No existe ruta entre " << nombreOrigen << " y " << nombreDestino << ".\n";
        return;
    }

    vector<string> ruta;
    string cur = nombreDestino;
    while (prev.find(cur) != prev.end()) {
        ruta.push_back(cur);
        cur = prev[cur];
    }
    ruta.push_back(nombreOrigen);
    reverse(ruta.begin(), ruta.end());

    cout << "Ruta mas corta: ";
    for (size_t i = 0; i < ruta.size(); ++i) {
        cout << ruta[i];
        if (i + 1 < ruta.size()) cout << " -> ";
    }
    cout << " | Costo total: " << dist[nombreDestino] << "\n";
}
