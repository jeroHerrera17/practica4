// netsim_simple.cpp
// Simulador de red de enrutadores (requisitos 1-4)
// - Usa solo: <iostream>, <fstream>, <vector>, <string>
// - Solo 'class'
// - Dijkstra implementado en O(n^2) (sin priority_queue)
// - Tablas y rutas almacenadas con vectores
//
// Compilar:
//   g++ -std=c++17 -O2 -o netsim_simple netsim_simple.cpp
//
// Ejecutar:
//   ./netsim_simple
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

// Valor grande para "infinito"
const int INF = 1000000000; // 1e9

// -----------------------------
// Clase Router
// - Guarda, para cada destino d:
//    dist[d] = costo mínimo desde este router hasta d (INF si no alcanzable)
//    next[d] = siguiente salto (index) para llegar a d (-1 si no definido)
//    paths[d] = vector con ruta completa desde este router hasta d (vacío si no hay)
// - El tamaño de estos vectores coincide con N (número de nodos de la red)
// -----------------------------
class Router {
private:
    int id;
    vector<int> dist;               // dist[dest]
    vector<int> nextHop;            // nextHop[dest]
    vector<vector<int>> paths;      // paths[dest] = ruta completa

public:
    Router(): id(-1) {}
    Router(int _id): id(_id) {}

    void setId(int _id) { id = _id; }

    // Asegura que la tabla tenga 'n' entradas (llena con valores por defecto)
    void ensureSize(int n) {
        dist.assign(n, INF);
        nextHop.assign(n, -1);
        paths.assign(n, vector<int>()); // vectores vacíos
    }

    // Establece la información para un destino concreto
    void setEntry(int dest, int cost, int next, const vector<int>& path) {
        if (dest < 0 || dest >= (int)dist.size()) return;
        dist[dest] = cost;
        nextHop[dest] = next;
        paths[dest] = path;
    }

    bool hasRoute(int dest) const {
        if (dest < 0 || dest >= (int)dist.size()) return false;
        return dist[dest] < INF;
    }

    int getCost(int dest) const {
        if (dest < 0 || dest >= (int)dist.size()) return -1;
        if (dist[dest] >= INF) return -1;
        return dist[dest];
    }

    vector<int> getPath(int dest) const {
        if (dest < 0 || dest >= (int)paths.size()) return vector<int>();
        return paths[dest];
    }

    // Imprime la tabla de enrutamiento de forma simple
    void printTable() const {
        cout << "Routing table for router " << id << ":\n";
        cout << "Destino\tCosto\tNextHop\tPath\n";
        for (int d = 0; d < (int)dist.size(); ++d) {
            if (d == id) {
                // opcional: mostrar ruta a sí mismo
                cout << d << "\t" << 0 << "\t" << id << "\t" << d << "\n";
                continue;
            }
            if (dist[d] >= INF) continue; // no alcanzable -> omitimos
            cout << d << "\t" << dist[d] << "\t" << nextHop[d] << "\t";
            const vector<int>& p = paths[d];
            for (int i = 0; i < (int)p.size(); ++i) {
                cout << p[i];
                if (i + 1 < (int)p.size()) cout << "->";
            }
            cout << "\n";
        }
    }
};

// -----------------------------
// Clase Network
// - Mantiene matriz de adyacencia adj (int), -1 significa sin enlace.
// - Mantiene vector<Router> routers y vector<bool> removed.
// - Implementa Dijkstra O(n^2) y reconstrucción de rutas.
// -----------------------------
class Network {
private:
    int n;                        // número de nodos (incluye indices removidos)
    vector<vector<int>> adj;      // adj[u][v] = costo o -1 si sin enlace
    vector<Router> routers;       // routers[i] corresponde al nodo i
    vector<bool> removed;         // removed[i] = true si el router fue removido

    // Dijkstra O(n^2) que devuelve dist[] y parent[]
    // - dist[i] = costo mínimo desde s hasta i
    // - parent[i] = padre de i en el árbol de caminos más cortos, o -1
    std::pair<vector<int>, vector<int>> dijkstra(int s) const {
        vector<int> dist(n, INF);
        vector<int> parent(n, -1);
        vector<bool> visited(n, false);

        if (s < 0 || s >= n || removed[s]) return std::make_pair(dist, parent);

        dist[s] = 0;

        // Repetimos n veces: seleccionar el no visitado con dist mínimo
        for (int iter = 0; iter < n; ++iter) {
            int u = -1;
            int best = INF;
            for (int i = 0; i < n; ++i) {
                if (removed[i]) continue;          // ignorar nodos removidos
                if (!visited[i] && dist[i] < best) {
                    best = dist[i];
                    u = i;
                }
            }
            if (u == -1) break; // no hay más nodos alcanzables

            visited[u] = true;

            // relajamos aristas desde u hacia todos los v
            for (int v = 0; v < n; ++v) {
                if (removed[v]) continue;
                int w = adj[u][v];
                if (w < 0) continue; // sin enlace
                if (dist[v] > dist[u] + w) {
                    dist[v] = dist[u] + w;
                    parent[v] = u;
                }
            }
        }
        return std::make_pair(dist, parent);
    }

    // Reconstruye ruta de s a t usando parent[].
    // Si no hay ruta válida devuelve vector vacío.
    vector<int> reconstructPath(const vector<int>& parent, int s, int t) const {
        vector<int> path;
        if (t < 0 || t >= (int)parent.size()) return path;
        if (parent[t] == -1 && s != t) {
            if (s == t) { path.push_back(s); return path; }
            return vector<int>(); // no alcanzable
        }
        int cur = t;
        while (cur != -1) {
            path.push_back(cur);
            if (cur == s) break;
            cur = parent[cur];
        }
        // invertir manualmente el vector 'path'
        int left = 0, right = (int)path.size() - 1;
        while (left < right) {
            int tmp = path[left];
            path[left] = path[right];
            path[right] = tmp;
            ++left; --right;
        }
        // verificar que empieza en s
        if (!path.empty() && path[0] == s) return path;
        return vector<int>();
    }

public:
    Network(): n(0) {}

    // Inicializa red vacía con size nodos
    void initEmpty(int size) {
        n = size;
        adj.assign(n, vector<int>(n, -1));
        for (int i = 0; i < n; ++i) adj[i][i] = 0;
        routers.clear();
        routers.reserve(n);
        for (int i = 0; i < n; ++i) {
            Router r(i);
            r.ensureSize(n);
            routers.push_back(r);
        }
        removed.assign(n, false);
    }

    // Carga topología desde archivo (N seguido de matriz N x N)
    bool loadFromFile(const string& path) {
        ifstream in(path.c_str());
        if (!in.is_open()) return false;
        int N;
        in >> N;
        if (!in) return false;
        initEmpty(N);
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                int v;
                in >> v;
                adj[i][j] = v;
            }
        }
        // asegurar simetría mínima (interpretamos red como no dirigida)
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                int a = adj[i][j];
                int b = adj[j][i];
                if (a < 0 && b < 0) {
                    adj[i][j] = adj[j][i] = -1;
                } else {
                    // si uno es -1, tomamos el otro; si ambos >=0 tomamos el menor
                    if (a < 0) adj[i][j] = adj[j][i];
                    else if (b < 0) adj[j][i] = adj[i][j];
                    else {
                        // ambos válidos: tomar el menor
                        adj[i][j] = adj[j][i] = (a < b ? a : b);
                    }
                }
            }
        }
        recomputeAllRoutingTables();
        return true;
    }

    // Agrega router al final (id = n), expande matriz
    void addRouter() {
        int newId = n;
        ++n;
        for (int i = 0; i < (int)adj.size(); ++i) adj[i].push_back(-1);
        adj.push_back(vector<int>(n, -1));
        for (int i = 0; i < n; ++i) {
            adj[newId][i] = adj[i][newId] = -1;
        }
        adj[newId][newId] = 0;
        Router r(newId);
        r.ensureSize(n);
        routers.push_back(r);
        removed.push_back(false);
        recomputeAllRoutingTables();
    }

    // Marca router como removido y borra sus enlaces
    void removeRouter(int id) {
        if (id < 0 || id >= n) return;
        if (removed[id]) return;
        removed[id] = true;
        for (int i = 0; i < n; ++i) {
            adj[id][i] = -1;
            adj[i][id] = -1;
        }
        // limpiar tabla
        routers[id].ensureSize(n); // tablas vacías/inf
        recomputeAllRoutingTables();
    }

    // Agrega o actualiza enlace no dirigido
    void addOrUpdateLink(int u, int v, int cost) {
        if (u < 0 || u >= n || v < 0 || v >= n) return;
        if (removed[u] || removed[v]) return;
        if (u == v) return;
        adj[u][v] = cost;
        adj[v][u] = cost;
        recomputeAllRoutingTables();
    }

    // Elimina enlace
    void removeLink(int u, int v) {
        if (u < 0 || u >= n || v < 0 || v >= n) return;
        adj[u][v] = -1;
        adj[v][u] = -1;
        recomputeAllRoutingTables();
    }

    // Recalcula todas las tablas: para cada s ejecuta Dijkstra y guarda dist/next/path
    void recomputeAllRoutingTables() {
        // preparar tamaño de tablas en cada router
        for (int i = 0; i < n; ++i) {
            routers[i].ensureSize(n);
        }

        for (int s = 0; s < n; ++s) {
            if (removed[s]) continue;
            std::pair<vector<int>, vector<int>> res = dijkstra(s);
            const vector<int>& dist = res.first;
            const vector<int>& parent = res.second;

            for (int t = 0; t < n; ++t) {
                if (removed[t]) continue;
                if (dist[t] >= INF) continue; // no alcanzable
                // calcular nextHop:
                int next = -1;
                if (s == t) {
                    next = s;
                } else {
                    int cur = t;
                    int prev = parent[cur];
                    while (prev != -1 && prev != s) {
                        cur = prev;
                        prev = parent[cur];
                    }
                    if (prev == -1) {
                        if (parent[t] == -1) next = -1;
                        else next = cur;
                    } else {
                        next = cur;
                    }
                }
                vector<int> path = reconstructPath(parent, s, t);
                routers[s].setEntry(t, dist[t], next, path);
            }
        }
    }

    // Consultas
    int getCost(int s, int t) const {
        if (s < 0 || s >= n || t < 0 || t >= n) return -1;
        if (removed[s] || removed[t]) return -1;
        return routers[s].getCost(t);
    }

    vector<int> getPath(int s, int t) const {
        if (s < 0 || s >= n || t < 0 || t >= n) return vector<int>();
        if (removed[s] || removed[t]) return vector<int>();
        return routers[s].getPath(t);
    }

    void printAdjMatrix() const {
        cout << "Adjacency matrix (-1 = no link):\n";
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                cout << adj[i][j];
                if (j + 1 < n) cout << " ";
            }
            cout << "\n";
        }
    }

    void printRoutingTable(int id) const {
        if (id < 0 || id >= n) { cout << "Router invalido\n"; return; }
        if (removed[id]) { cout << "Router " << id << " esta removido\n"; return; }
        routers[id].printTable();
    }

    int size() const { return n; }
    bool isRemoved(int id) const {
        if (id < 0 || id >= n) return true;
        return removed[id];
    }
};

// -----------------------------
// Interfaz de consola (menú)
// -----------------------------
void showMenu() {
    cout << "\n--- MENU (requisitos 1-4) ---\n";
    cout << "1) Cargar topologia desde archivo\n";
    cout << "2) Agregar enrutador\n";
    cout << "3) Remover enrutador\n";
    cout << "4) Agregar/Actualizar enlace (u v cost)\n";
    cout << "5) Remover enlace (u v)\n";
    cout << "6) Imprimir matriz de adyacencia\n";
    cout << "7) Imprimir tabla de enrutamiento de un router\n";
    cout << "8) Costo minimo entre dos routers\n";
    cout << "9) Camino minimo entre dos routers\n";
    cout << "0) Salir\n";
    cout << "Selecciona opcion: ";
}

int main() {
    Network net;
    cout << "Simulador de red (solo requisitos 1-4). Includes limitados.\n";

    while (true) {
        showMenu();
        int opt;
        if (!(cin >> opt)) break;
        if (opt == 0) break;

        if (opt == 1) {
            string path; cout << "Ruta del archivo: "; cin >> path;
            if (net.loadFromFile(path)) cout << "Topologia cargada OK.\n";
            else cout << "Error al cargar archivo.\n";
        } else if (opt == 2) {
            net.addRouter();
            cout << "Router agregado. Nuevo tamaño = " << net.size() << "\n";
        } else if (opt == 3) {
            int id; cout << "Id a remover: "; cin >> id;
            net.removeRouter(id);
            cout << "Operacion (si existia) aplicada.\n";
        } else if (opt == 4) {
            int u,v,c; cout << "u v cost: "; cin >> u >> v >> c;
            net.addOrUpdateLink(u,v,c);
            cout << "Enlace agregado/actualizado.\n";
        } else if (opt == 5) {
            int u,v; cout << "u v: "; cin >> u >> v;
            net.removeLink(u,v);
            cout << "Enlace removido (si existia).\n";
        } else if (opt == 6) {
            net.printAdjMatrix();
        } else if (opt == 7) {
            int id; cout << "Id router: "; cin >> id;
            net.printRoutingTable(id);
        } else if (opt == 8) {
            int s,t; cout << "origen destino: "; cin >> s >> t;
            int cost = net.getCost(s,t);
            if (cost < 0) cout << "No alcanzable o router invalido.\n";
            else cout << "Costo minimo: " << cost << "\n";
        } else if (opt == 9) {
            int s,t; cout << "origen destino: "; cin >> s >> t;
            vector<int> path = net.getPath(s,t);
            if (path.empty()) cout << "No hay camino o router invalido.\n";
            else {
                cout << "Camino: ";
                for (int i = 0; i < (int)path.size(); ++i) {
                    cout << path[i];
                    if (i + 1 < (int)path.size()) cout << "->";
                }
                cout << "\n";
            }
        } else {
            cout << "Opcion no valida.\n";
        }
    }

    cout << "Saliendo...\n";
    return 0;
}
