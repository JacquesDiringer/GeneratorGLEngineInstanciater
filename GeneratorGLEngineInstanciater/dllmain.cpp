// dllmain.cpp : Définit le point d'entrée pour l'application DLL.
#include "stdafx.h"

#include <GLEngine.h>

int main()
{
	GLEngine::GLEngine engine = GLEngine::GLEngine();

	engine.InitializeContext();
}