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

using namespace std;

int FUN_NUM = 0;
int FUN_CNT = 0;

void compile(elt *elt, ofstream& fo)
{
    if (elt==NULL){
        return;
    }
    if (elt->val != NULL)
    {
        int pos;
        string instr = elt->val;
        //Cuando se encuentra uns instruccion INT_CONST no se hace nada, luego viene un numero
        if (instr.find("INT_CONST") != std::string::npos) {
        	cout << "Apilando numero ";
        }else if (instr.find("ADD") != std::string::npos) {
            //Al encontrar ADD se sacan dos elementos de la pila y se suman
            fo << "\tLDMFD SP!, {R0, R1}\n";
            fo << "\tADD R0, R0, R1\n";
            fo << "\tSTR R0, [SP], #-4\n";
        }else if (instr.find("SUBS") != std::string::npos) {
            //Al encontrar SUBS se quitan dos elementos de la pila y se restan
            fo << "\tLDMFD SP!, {R0, R1}\n";
            fo << "\tSUBS R0, R0, R1\n";
            fo << "\tSTR R0, [SP], #-4\n";
        } else if (instr.find("FUN") != std:string:npos) {
        	fo << "\tLDR R7, =label" << FUN_NUM << "\n";
            fo << "\tSTR R7, [SP], #-4\n";
            return compile(elt->list, fo);
        } else if (instr.find("APPLY")) {
        	if (FUN_NUM > FUN_CNT) {
        		cout << "Syntax error: No function to apply";
        		throw logic_error;
        	}
        	fo << "\tLDMFD SP!, {R0, R1}\n";
        	fo << "\tBLX R1\n";
        } else {
            // Si se encuentra un numero se guarda en la pila
            cout << instr << "\n";
            fo << "\tMOV R0, #" << instr << "\n";
            fo << "\tSTR R0, [SP], #-4\n";
        }

    }
    //Primero se compila la instruccion siguiente y luego el elemento siguiente
    compile(elt->next, fo);
    compile(elt->list, fo);
}

void pre_compile(elt *elt, ofstream& fo)
{
	if (elt==NULL){
        return;
    }
    if (elt->val != NULL)
    {
        int pos;
        string instr = elt->val;
        //Cuando se encuentra uns instruccion INT_CONST no se hace nada, luego viene un numero
        if (instr.find("INT_CONST") != std::string::npos) {
        	cout << "Apilando numero ";
        }else if (instr.find("ADD") != std::string::npos) {
            //Al encontrar ADD se sacan dos elementos de la pila y se suman
            fo << "\tLDMFD SP!, {R0, R1}\n";
            fo << "\tADD R0, R0, R1\n";
            fo << "\tSTR R0, [SP], #-4\n";
        }else if (instr.find("SUBS") != std::string::npos) {
            //Al encontrar SUBS se quitan dos elementos de la pila y se restan
            fo << "\tLDMFD SP!, {R0, R1}\n";
            fo << "\tSUBS R0, R0, R1\n";
            fo << "\tSTR R0, [SP], #-4\n";
        } else if (instr.find("FUN") != std:string:npos) {
        	fo << "\tLDR R7, =label" << FUN_NUM << "\n";
            fo << "\tSTR R7, [SP], #-4\n";
            return compile(elt->list, fo);
        } else if (instr.find("APPLY") != std::string::npos) {
        	if (FUN_NUM > FUN_CNT) {
        		cout << "Syntax error: No function to apply";
        		throw logic_error;
        	}
        	fo << "\tLDMFD SP!, {R0, R1}\n";
        	fo << "\tBLX R1\n";
        } else if (instr.find("RETURN") != std::string::npos) {
        	
        } else {
            // Si se encuentra un numero se guarda en la pila
            cout << instr << "\n";
            fo << "\tMOV R0, #" << instr << "\n";
            fo << "\tSTR R0, [SP], #-4\n";
        }

    }
    //Primero se compila la instruccion siguiente y luego el elemento siguiente
    pre_compile(elt->next, fo);
    pre_compile(elt->list, fo);
}

void compile_fun(elt *elt, ofstream& fo)
{
    if (elt==NULL){
        return;
    }
    if (elt->val != NULL)
    {
        int pos;
        string instr = elt->val;
        //Cuando se encuentra uns instruccion INT_CONST no se hace nada, luego viene un numero
        if (instr.find("FUN") != std::string::npos) {
        	cout << "Creando funcion\n";
        	fo << "label" << FUN_CNT << ":\n";
        	FUN_CNT++;

        } else if (instr.find("RETURN") != std::string::npos) {

        }
    }
    //Primero se compila la instruccion siguiente y luego el elemento siguiente
    compile_fun(elt->next, fo);
    compile_fun(elt->list, fo);
}

int main(int argc, const char * argv[])
{
    char linebuf[256];
    ifstream fp;
    ofstream fo;
    sexp_t *sx;
    
    // Se abren los archivos a manipular    
    fp.open("lista.txt");
    fo.open("out.s");

    // Se agregan las primeras instrucciones para el printf
    fo << ".LC0:\n";
	fo << "\t.string " << '"' << "%d\n" << '"' << "\n";
	fo << "\t.text\n";
	fo << "\t.global main\n";
	fo << "\t.type main, " << '%' << "function\n";

    // Mientras no se acaben las instrucciones se sigue leyendo
    // Se asume que hay una instruccion por fila y se parte haciendo las instrucciones de las funciones
    while (! fp.eof()) {
        fp.getline(linebuf, 200);
        sx = parse_sexp(linebuf,strlen(linebuf));
        compile_fun(sx->list, fo);
        destroy_sexp(sx);
    }

    // Etiqueta inicial funcion main
    fo << "main:\n";
	fo << "\tstmfd	sp!, {fp, lr}\n";

	// Se reinicia puntero en el archivo de instrucciones
	fseek(fp, 0, SEEK_SET)

	// Mientras no se acaben las instrucciones se sigue leyendo
    // Se asume que hay una instruccion por fila y se parte haciendo las instrucciones de las funciones
    while (! fp.eof()) {
        fp.getline(linebuf, 200);
        sx = parse_sexp(linebuf,strlen(linebuf));
        compile(sx->list, fo);
        destroy_sexp(sx);
    }

    cout << "fin\n";
    fo << "\tLDMFD r13!, {r0}\n";
	fo << "\tMOV r1, r0\n";
	fo << "\tLDR r0, .L1\n";
	fo << "\tBL printf\n";
	fo << "\tLDMFD sp!, {pc}\n";
	fo << ".L1:";
	fo << "\t.word .LC0\n";


    fp.close();
    fo.close();

    return 0;
}
