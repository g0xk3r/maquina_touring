#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
    ofstream archivo("cadena_manual.txt");
    if (archivo.is_open()) {
        int n = 50000;
        for(int i = 0; i < n; i++) archivo << '0';
        for(int i = 0; i < n; i++) archivo << '1';
        archivo.close();
        cout << "Archivo 'cadena_manual.txt' creado exitosamente con " << n*2 << "caracteres." << endl;
    } else {
        cout << "No se pudo crear el archivo." << endl;
    }
    return 0;
}