#include "red.h"
#include <iostream>
#include <limits>
#include <QDir>
#include <QFile>

using namespace std;

void mostrarMenu() {
    cout << "\n=========================================\n";
    cout << "||    SIMULADOR DE REDES DE ENRUTADORES ||\n";
    cout << "=========================================\n";
    cout << "1. Mostrar red actual\n";
    cout << "2. Calcular ruta más corta\n";
    cout << "3. Agregar enrutador\n";
    cout << "4. Eliminar enrutador\n";
    cout << "5. Agregar enlace\n";
    cout << "6. Eliminar enlace\n";
    cout << "7. Guardar red\n";
    cout << "8. Mostrar tablas de enrutamiento\n";
    cout << "9. Salir\n";
    cout << "=========================================\n";
    cout << "Seleccione una opción: ";
}

/**
 * @brief Cuenta cuántos archivos red_*.txt existen en la carpeta.
 */
int contarRedesDisponibles(const string& carpeta) {
    QDir dir(QString::fromStdString(carpeta));

    if (!dir.exists()) {
        dir.mkpath(".");
        return 0;
    }

    QStringList filtros;
    filtros << "red_*.txt";
    dir.setNameFilters(filtros);
    dir.setFilter(QDir::Files);

    return dir.entryList().size();
}

/**
 * @brief Lista las redes disponibles en la carpeta.
 */
vector<string> listarRedesDisponibles(const string& carpeta) {
    vector<string> archivos;
    int cantidad = contarRedesDisponibles(carpeta);

    if (cantidad == 0) {
        cout << "No hay redes guardadas disponibles.\n";
        return archivos;
    }

    cout << "\nRedes disponibles:\n";

    // Listar red_1.txt, red_2.txt, ... red_n.txt
    QDir dir(QString::fromStdString(carpeta));
    for (int i = 1; i <= cantidad; ++i) {
        string nombreArchivo = "red_" + to_string(i) + ".txt";
        QString rutaCompleta = dir.filePath(QString::fromStdString(nombreArchivo));

        // Verificar si el archivo existe
        if (QFile::exists(rutaCompleta)) {
            archivos.push_back(nombreArchivo);
            cout << "  [" << i << "] " << nombreArchivo << "\n";
        }
    }

    return archivos;
}

/**
 * @brief Solicita al usuario cómo desea guardar la red.
 * @return Ruta completa del archivo donde guardar
 */
string solicitarRutaGuardado(const string& carpeta) {
    cout << "\n¿Cómo desea guardar la red?\n";
    cout << "1. Crear archivo nuevo\n";
    cout << "2. Sobreescribir archivo existente\n";
    cout << "Seleccione una opción: ";

    int opcion;
    cin >> opcion;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    QDir dir(QString::fromStdString(carpeta));

    if (opcion == 1) {
        // Crear archivo nuevo con el siguiente número disponible
        int cantidadActual = contarRedesDisponibles(carpeta);
        int nuevoNumero = cantidadActual + 1;

        string nombreArchivo = "red_" + to_string(nuevoNumero) + ".txt";
        QString rutaCompleta = dir.filePath(QString::fromStdString(nombreArchivo));

        cout << "Nueva red será guardada como: " << nombreArchivo << "\n";
        return rutaCompleta.toStdString();

    } else if (opcion == 2) {
        // Sobreescribir archivo existente
        vector<string> disponibles = listarRedesDisponibles(carpeta);

        if (disponibles.empty()) {
            cout << "No hay archivos existentes para sobreescribir.\n";
            cout << "Creando archivo nuevo...\n";

            string nombreArchivo = "red_1.txt";
            QString rutaCompleta = dir.filePath(QString::fromStdString(nombreArchivo));
            return rutaCompleta.toStdString();
        }

        cout << "Seleccione el número del archivo a sobreescribir: ";
        int eleccion;
        cin >> eleccion;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (eleccion > 0 && eleccion <= (int)disponibles.size()) {
            cout << "¿Está seguro de sobreescribir '" << disponibles[eleccion - 1]
                 << "'? (s/n): ";
            char confirmar;
            cin >> confirmar;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (confirmar == 's' || confirmar == 'S') {
                QString rutaCompleta = dir.filePath(QString::fromStdString(disponibles[eleccion - 1]));
                return rutaCompleta.toStdString();
            } else {
                cout << "Operación cancelada.\n";
                return "";
            }
        } else {
            cout << "Opción inválida. Operación cancelada.\n";
            return "";
        }
    } else {
        cout << "Opción inválida. Operación cancelada.\n";
        return "";
    }
}

int main(int argc, char *argv[]) {
    Red* red = new Red();
    string carpeta = "Datos";
    string nombreArchivo;

    // ======================================================
    // Cargar red existente o crear una nueva
    // ======================================================
    cout << "¿Desea cargar una red existente o crear una nueva? (c/n): ";
    char opcion;
    cin >> opcion;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (opcion == 'c' || opcion == 'C') {
        vector<string> disponibles = listarRedesDisponibles(carpeta);
        if (!disponibles.empty()) {
            cout << "Seleccione el número de red a cargar: ";
            int eleccion;
            cin >> eleccion;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (eleccion > 0 && eleccion <= (int)disponibles.size()) {
                QDir dir(QString::fromStdString(carpeta));
                QString rutaCompleta = dir.filePath(QString::fromStdString(disponibles[eleccion - 1]));
                nombreArchivo = rutaCompleta.toStdString();
                red->cargarDesdeArchivo(nombreArchivo);
                cout << "Red " << eleccion << " cargada correctamente.\n";
            } else {
                cout << "Opción inválida. Finalizando.\n";
                delete red;
                return 0;
            }
        } else {
            cout << "No hay redes guardadas. Debe crear una nueva.\n";
            opcion = 'n';
        }
    }

    if (opcion == 'n' || opcion == 'N') {
        int cantidad;
        cout << "Ingrese la cantidad de enrutadores para la nueva red: ";
        cin >> cantidad;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        delete red;
        red = new Red(cantidad);
        red->generarRedAleatoria();

        cout << "¿Desea guardar esta red? (s/n): ";
        char guardar;
        cin >> guardar;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (guardar == 's' || guardar == 'S') {
            nombreArchivo = solicitarRutaGuardado(carpeta);
            if (!nombreArchivo.empty()) {
                red->guardarEnArchivo(nombreArchivo);
                cout << "Red guardada exitosamente en: " << nombreArchivo << endl;
            }
        }
    }

    // ======================================================
    // Menú principal
    // ======================================================
    int opcionMenu = 0;
    do {
        mostrarMenu();
        cin >> opcionMenu;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Entrada inválida. Intente nuevamente.\n";
            continue;
        }

        switch (opcionMenu) {
        case 1:
            red->mostrarRed();
            break;
        case 2: {
            int o, d;
            cout << "Ingrese enrutador origen: ";
            cin >> o;
            cout << "Ingrese enrutador destino: ";
            cin >> d;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            red->calcularRutaMasCorta(o, d);
            break;
        }
        case 3:
            red->agregarEnrutador();
            break;
        case 4: {
            int id;
            cout << "Ingrese el ID del enrutador a eliminar: ";
            cin >> id;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            red->eliminarEnrutador(id);
            break;
        }
        case 5:
            red->agregarEnlace();
            break;
        case 6:
            red->eliminarEnlace();
            break;
        case 7: {
            string rutaGuardado = solicitarRutaGuardado(carpeta);
            if (!rutaGuardado.empty()) {
                red->guardarEnArchivo(rutaGuardado);
                nombreArchivo = rutaGuardado;
                cout << "Red guardada exitosamente.\n";
            }
            break;
        }
        case 8:
            red->mostrarTablasDeEnrutamiento();
            break;
        case 9:
            cout << "\nSaliendo del programa...\n";
            break;
        default:
            cout << "Opción no válida.\n";
        }
    } while (opcionMenu != 9);

    delete red;
    cout << "Programa finalizado correctamente.\n";
    return 0;
}

