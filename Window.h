#pragma once
#define NOMINMAX
#include<Windows.h>
#include<iostream>
#include"Object.h"

class Player;

class Window
{
public:
	HWND hwnd;
	HINSTANCE hinstance;

	int width = 0;
	int height = 0;
	bool fullscreen = false;

	//input variables
	bool keys[256] = { 0 };
	int mousex = 0;
	int mousey = 0;
	bool mouseButtons[3] = { 0 };
	float sensitivity = 0.005f;

	Player* player = nullptr;

	void create(int window_width, int window_height);

	void updateMouse(int x, int y);

	bool processMessages();
};

