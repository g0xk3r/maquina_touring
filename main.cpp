#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

struct EstadoSimbolo {
    string estado;
    char simbolo;
    bool operator<(const EstadoSimbolo& other) const {
        if (estado != other.estado) return estado < other.estado;
        return simbolo < other.simbolo;
    }
};

struct Transicion {
    string nuevo_estado;
    char simbolo_escrito;
    char direccion;
};

struct PasoHistoria {
    string cinta_snapshot;
    int cabezal_pos;
    string estado;
};

class MaquinaTuring {
private:
    vector<char> cinta;
    int cabezal;
    string estado_actual;
    const char simbolo_blanco = 'B';
    map<EstadoSimbolo, Transicion> transiciones;
    ofstream archivo_id;
    ofstream archivo_traza;
    vector<PasoHistoria> historia_animacion;

public:
    MaquinaTuring(string cadena_entrada) {
        for (char c : cadena_entrada) cinta.push_back(c);
        cabezal = 0;
        estado_actual = "q0";
        transiciones[{"q0", '0'}] = {"q1", 'X', 'R'};
        transiciones[{"q0", 'Y'}] = {"q3", 'Y', 'R'};
        transiciones[{"q1", '0'}] = {"q1", '0', 'R'};
        transiciones[{"q1", '1'}] = {"q2", 'Y', 'L'};
        transiciones[{"q1", 'Y'}] = {"q1", 'Y', 'R'};
        transiciones[{"q2", '0'}] = {"q2", '0', 'L'};
        transiciones[{"q2", 'X'}] = {"q0", 'X', 'R'};
        transiciones[{"q2", 'Y'}] = {"q2", 'Y', 'L'};
        transiciones[{"q3", 'Y'}] = {"q3", 'Y', 'R'};
        transiciones[{"q3", 'B'}] = {"q4", 'B', 'R'};
    }

    char obtener_simbolo_actual() {
        if (cabezal >= (int)cinta.size()) return simbolo_blanco;
        if (cabezal < 0) return simbolo_blanco;
        return cinta[cabezal];
    }

    void escribir_simbolo(char simbolo) {
        if (cabezal >= (int)cinta.size()) cinta.push_back(simbolo_blanco);
        if (cabezal < 0) { cinta.insert(cinta.begin(), simbolo_blanco); cabezal = 0; }
        cinta[cabezal] = simbolo;
    }

    void guardar_descripcion_instantanea() {
        for (int i = 0; i < cabezal && i < (int)cinta.size(); ++i) archivo_id << cinta[i];
        archivo_id << estado_actual;
        if (cabezal < (int)cinta.size()) {
            for (int i = cabezal; i < (int)cinta.size(); ++i) archivo_id << cinta[i];
        } else {
            archivo_id << simbolo_blanco;
        }
        archivo_id << "\n";
    }

    void registrar_paso_animacion() {
        string snapshot = "";
        for(char c : cinta) snapshot += c;
        if (cabezal >= (int)cinta.size()) snapshot += simbolo_blanco;
        historia_animacion.push_back({snapshot, cabezal, estado_actual});
    }

    bool paso(int numero_paso) {
        char simbolo_leido = obtener_simbolo_actual();
        if (transiciones.count({estado_actual, simbolo_leido})) {
            Transicion t = transiciones[{estado_actual, simbolo_leido}];
            archivo_traza << "Paso " << numero_paso << ": " << estado_actual
                        << ", Lee: " << simbolo_leido << " -> Escribe: " << t.simbolo_escrito
                        << ", Mueve: " << t.direccion << ", Nuevo: " << t.nuevo_estado << "\n";
            escribir_simbolo(t.simbolo_escrito);
            estado_actual = t.nuevo_estado;
            if (t.direccion == 'R') cabezal++;
            else if (t.direccion == 'L') cabezal--;
            return true;
        } else {
            return false;
        }
    }

    void generar_archivo_html() {
        ofstream html("animacion.html");
        html << "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>"
            << "<title>Animacion Maquina de Turing</title>"
            << "<style>"
            << "body { font-family: 'Segoe UI', sans-serif; background-color: #1e1e1e; color: white; text-align: center; }"
            << ".contenedor-cinta { display: flex; justify-content: center; margin: 50px auto; overflow-x: auto; padding: 20px; }"
            << ".celda { width: 50px; height: 50px; border: 2px solid #555; display: flex; align-items: center; justify-content: center;"
            << "         font-size: 24px; margin: 2px; border-radius: 5px; background: #333; transition: all 0.3s ease; }"
            << ".cabezal { background-color: #007acc; border-color: #61dafb; box-shadow: 0 0 15px #61dafb; transform: scale(1.1); font-weight: bold; }"
            << ".info-panel { background: #252526; padding: 20px; border-radius: 10px; display: inline-block; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }"
            << "h1 { color: #61dafb; }"
            << "#estado-display { font-size: 1.5em; color: #ffeb3b; }"
            << "#paso-display { color: #aaa; }"
            << "</style></head><body>"
            << "<h1>Simulacion Grafica de Maquina de Turing</h1>"
            << "<div class='info-panel'>"
            << "<div id='paso-display'>Paso: 0</div>"
            << "<div id='estado-display'>Estado: q0</div>"
            << "</div>"
            << "<div class='contenedor-cinta' id='cinta'></div>"
            << "<script>"
            << "const historia = [";

        for(size_t i=0; i < historia_animacion.size(); ++i) {
            html << "{cinta: '" << historia_animacion[i].cinta_snapshot << "', "
                << "cabezal: " << historia_animacion[i].cabezal_pos << ", "
                << "estado: '" << historia_animacion[i].estado << "'},";
        }

        html << "];\n"
            << "let indice = 0;\n"
            << "const divCinta = document.getElementById('cinta');\n"
            << "const divEstado = document.getElementById('estado-display');\n"
            << "const divPaso = document.getElementById('paso-display');\n"
            << "function dibujar() {\n"
            << "  if (indice >= historia.length) return;\n"
            << "  const paso = historia[indice];\n"
            << "  divEstado.innerText = 'Estado: ' + paso.estado;\n"
            << "  divPaso.innerText = 'Paso: ' + indice + ' de ' + (historia.length - 1);\n"
            << "  divCinta.innerHTML = '';\n"
            << "  for (let i = 0; i < paso.cinta.length; i++) {\n"
            << "    const celda = document.createElement('div');\n"
            << "    celda.className = 'celda';\n"
            << "    celda.innerText = paso.cinta[i];\n"
            << "    if (i === paso.cabezal) celda.classList.add('cabezal');\n"
            << "    divCinta.appendChild(celda);\n"
            << "  }\n"
            << "  indice++;\n"
             << "  if(indice < historia.length) setTimeout(dibujar, 500); // 500ms delay\n"
            << "}\n"
            << "dibujar();\n"
            << "</script></body></html>";

        html.close();
        cout << "\nArchivo 'animacion.html' generado exitosamente." << endl;
    }

    void ejecutar(bool animar) {
        archivo_id.open("descripciones_instantaneas.txt");
        archivo_traza.open("traza_transiciones.txt");
        int pasos_contador = 0;
        guardar_descripcion_instantanea();
        if(animar) registrar_paso_animacion();
        cout << "Procesando... " << endl;

        while (estado_actual != "q4") {
            pasos_contador++;
            bool continua = paso(pasos_contador);
            guardar_descripcion_instantanea();
            if(animar) registrar_paso_animacion();
            if (!continua) break;
        }

        if (estado_actual == "q4") {
            cout << "\nCADENA ACEPTADA. Estado final alcanzado." << endl;
            archivo_traza << "\nCADENA ACEPTADA.\n";
        } else {
            cout << "\nLa maquina se detuvo sin aceptar." << endl;
            archivo_traza << "\nRECHAZADA.\n";
        }

        archivo_id.close();
        archivo_traza.close();
        if (animar) {
            generar_archivo_html();
        }
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    string opcion;
    cout << "Desea usar 'cadena_manual.txt'? (s/n): ";
    cin >> opcion;

    string entrada = "";
    if (opcion == "s" || opcion == "S") {
        ifstream archivo_entrada("cadena_manual.txt");
        if (archivo_entrada.is_open()) {
            archivo_entrada >> entrada;
            archivo_entrada.close();
            cout << "Cadena leida desde archivo. Longitud: " << entrada.length() << endl;
        } else {
            cerr << "ERROR FATAL: No se encontro el archivo 'cadena_manual.txt'." << endl;
            cerr << "Por favor crea el archivo en la misma carpeta que este programa." << endl;
            return 1;
        }
    } else {
        srand(time(0));
        int n = rand() % 100000 + 1;
        cout << "Generando cadena automatica aleatoria 0^" << n << " 1^" << n << endl;
        for(int i=0; i<n; ++i) entrada += '0';
        for(int i=0; i<n; ++i) entrada += '1';
    }

    if (entrada.length() > 100000) {
        cout << "Error: La cadena excede el maximo." << endl;
        return 1;
    }

    MaquinaTuring mt(entrada);
    bool modo_grafico = (entrada.length() <= 20);
    mt.ejecutar(modo_grafico);

    return 0;
}