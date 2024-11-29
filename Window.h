#pragma once
#include<Windows.h>
#include<iostream>


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

	void create(int window_width, int window_height);

	void updateMouse(int x, int y);

	bool processMessages();
};

