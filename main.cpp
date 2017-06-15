//
//  main.cpp
//  readList
//
//  Created by Pablo Guerrero on 10/8/14.
//  Copyright (c) 2014 a. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <stdio.h>
#include "sexpr-1.3/src/sexp.h"
#include "sexpr-1.3/src/sexp_ops.h"
#include <string.h>
#include <exception>
#include <stack>
#include <sstream>

using namespace std;

int FUN_NUM = 0;
int FUN_CNT = 0;
int IF_CNT = 0;

void compile(stack<string>& s, ofstream& fo)
{
    while (!s.empty())
    {
        int pos;
        string instr(s.top());
        s.pop();
        if (instr.find("INT_CONST") != std::string::npos) {
            //Cuando se encuentra uns instruccion INT_CONST no se hace nada, luego viene un numero
            cout << "Apilando numero ";
        }else if (instr.find("ADD") != std::string::npos) {
            //Al encontrar ADD se sacan dos elementos de la pila y se suman
            fo << "\tLDR R0, [SP], #4\n";
            fo << "\tLDR R1, [SP], #4\n";
            fo << "\tADD R0, R0, R1\n";
            fo << "\tSTR R0, [SP, #-4]!\n";
        }else if (instr.find("SUB") != std::string::npos) {
            //Al encontrar SUBS se quitan dos elementos de la pila y se restan
            fo << "\tLDR R0, [SP], #4\n";
            fo << "\tLDR R1, [SP], #4\n";
            fo << "\tSUB R0, R0, R1\n";
            fo << "\tSTR R0, [SP, #-4]!\n";
        } else if (instr.find("FUN") != std::string::npos) {
            fo << "\tLDR R7, =" << s.top() << "\n";
            s.pop();
            fo << "\tSTR R7, [SP, #-4]!\n";
            string next = s.top();
            s.pop();
            FUN_NUM = 0;
            while (true) {
                if (next.find("FUN") != std::string::npos) {
                    FUN_NUM++;
                }
                if (next.find("RETURN") != std::string::npos) {
                    if (FUN_NUM == 0) {
                        break;
                    } else {
                        FUN_NUM--;
                    }
                }
                if (s.empty()) {
                    printf("Syntax error: No return found");
                    return;
                }
                next = s.top();
                s.pop();
            }
        } else if (instr.find("APPLY")!= std::string::npos) {
            // Se desapila el argumento
            fo << "\tLDR R0, [SP], #4\n";
            // Se desapila el puntero a la función
            fo << "\tLDR R7, [SP], #4\n";
            // Se ejecuta la función con el argumento en R0
            fo << "\tBLX R7\n";
            // Se guarda el resultado en la pila

            fo << "\tSTR R0, [SP, #-4]!\n";
        } else if (instr.find("RETURN") != std::string::npos) {
            fo << "\tLDR R3, [SP], #4\n";
            fo << "\tMOV R0, R3\n";
            fo << "\tSUB SP, FP, #4\n";
            fo << "\tLDMFD SP!, {FP, PC}\n";
            return;
        } else if (instr.find("IF") != std::string::npos) {
            fo << "\t" << instr << ":\n";
            fo << "\tLDR R0, [SP], #4\n";
            fo << "\tCMP R0, #0\n";
            fo << "\tBNE false" << instr.substr(2, 3) << "\n";
        } else if (instr.find("end") != std::string::npos) {
            fo << "\t" << instr << ":\n";
        } else {
            // Si se encuentra un numero se guarda en la pila
            cout << instr << "\n";
            fo << "\tMOV R0, #" << instr << "\n";
            fo << "\tSTR R0, [SP, #-4]!\n";
        }
        if (!s.empty()) {
            string instr2(s.top());
            if (instr2.find("false") != std::string::npos) {
                fo << "\tB end" << instr2.substr(5, 3) << "\n";
                fo << "\t" << instr2 << ":\n";
                s.pop();
            }
        }
    }
}

void compile_fun(stack<string>& s, ofstream& fo)
{
    while (!s.empty())
    {
        int pos;
        string instr(s.top());
        s.pop();
        //Cuando se encuentra una instruccion INT_CONST no se hace nada, luego viene un numero
        if (instr.find("FUN") != std::string::npos) {
        	cout << "Creando funcion\n";
            // Se agrega label y se apila el argumento en R0
        	fo << s.top() << ":\n";
            s.pop();
            fo << "\tSTMFD SP!, {FP, LR}\n";
            fo << "\tADD FP, SP, #4\n";
            fo << "\tSTR R0, [SP, #-4]!\n";
            stack<string> s2(s);
            compile(s2, fo);
        }
    }
}

int cnt = 0;
int cnt2 = 0;
void turn(stack<string>& s, elt *elt) {
    if (elt == NULL) {
        return;
    }
    /*
    cnt++;
    cout << "next " << cnt << ":\n";
    */
    if (elt->val != NULL) {
        string instr(elt->val);
        if (instr.find("IF0") != std::string::npos) {
            stringstream ss1;
            ss1 << "end" << to_string(IF_CNT);
            s.push(ss1.str());
            turn(s, elt->next->next->list);
            stringstream ss2;
            ss2 << "false" << to_string(IF_CNT);
            s.push(ss2.str());
            cout << s.top() << "\n";
            turn(s, elt->next->list);
            cout << s.top() << "\n";
            stringstream ss4;
            ss4 << "IF" << to_string(IF_CNT);
            s.push(ss4.str());
            cout << s.top() << "\n";
            IF_CNT++;
            return;
        }
    }
    turn(s, elt->next);
    /*
    cout << "out next " << cnt << "\n";
    cnt--;
    */
    if (elt->val != NULL) {
        string instr(elt->val);
        if (instr.find("FUN") != std::string::npos) {
            stringstream ss;
            ss << "label" << to_string(FUN_CNT);
            s.push(ss.str());
            FUN_CNT++;
            cout << s.top() << "\n";
        }
        s.push(instr);
        cout << s.top() << "\n";
    }
    //cout << "list " << ++cnt2 << ":\n";
    turn(s, elt->list);
    //cout << "out list " << cnt2-- << "\n";
}

int main(int argc, const char * argv[])
{
    string linebuf;
    int pos;
    ifstream fp;
    ofstream fo;
    std::fstream edit;
    sexp_t *sx, *sx2;
    stack<string> s, s2;
    
    // Se abren los archivos a manipular
    fp.open("listas.txt");
    fo.open("out.s");

    // Se agregan las primeras instrucciones para el printf
    fo << ".LC0:\n";
	fo << "\t.string " << '"' << "Result: %d\\n" << '"' << "\n";
	fo << "\t.text\n";
	fo << "\t.global main\n";
	fo << "\t.type main, " << '%' << "function\n";

    // Mientras no se acaben las instrucciones se sigue leyendo
    // Se asume que hay una instruccion por fila y se parte haciendo las instrucciones de las funciones
    while (getline(fp, linebuf)) {
        cout << "Starting with found FUN definitions\n";
        char *cstr = new char[linebuf.length() + 1];
        strcpy(cstr, linebuf.c_str()); 
        sx = parse_sexp(cstr,linebuf.length());
        turn(s, sx);
        compile_fun(s, fo);
        destroy_sexp(sx);
    }

    // Etiqueta inicial funcion main
    fo << "main:\n";
	fo << "\tSTMFD SP!, {LR}\n";
    FUN_CNT = 0;
	// Mientras no se acaben las instrucciones se sigue leyendo
    // Se asume que hay una instruccion por fila y se parte haciendo las instrucciones de las funciones
    cout << "Compiling main\n";
    char *cstr = new char[linebuf.length() + 1];
    strcpy(cstr, linebuf.c_str()); 
    sx2 = parse_sexp(cstr,linebuf.length());
    turn(s2, sx2);
    compile(s2, fo);
    destroy_sexp(sx2);

    cout << "fin\n";
    fo << "\tLDR R0, [SP], #4\n";
	fo << "\tMOV R1, R0\n";
	fo << "\tLDR R0, .L1\n";
	fo << "\tBL printf\n";
	fo << "\tLDMFD SP!, {PC}\n";
	fo << ".L1:\n";
	fo << "\t.word .LC0\n";


    fp.close();
    fo.close();

    return 0;
}
