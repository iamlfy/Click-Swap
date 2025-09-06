#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/**
 * 1、加载整张图片 ok
 * 2、将图片进行分割。按宽度和高度进行分割，分成4*4（后续根据宽高比进行优化） ok
 * 3、记录分块的初始状态，然后随机渲染对应的分块 ok
 * 4、支持鼠标点击两个相邻块进行交换 ok
 * 5、如果各个分块达到了初始状态，则游戏结束
 */

void shuffleIndices(int indices[], int indices_len)
{
	SetRandomSeed((unsigned int)time(NULL));

	for (int i = 0; i < indices_len; ++i)
	{
		indices[i] = i;
	}

	for (int i = indices_len - 1; i >= 0; --i)
	{
		int randomPos = GetRandomValue(0, i);
		int tmp = indices[randomPos];
		indices[randomPos] = indices[i];
		indices[i] = tmp;
	}
}

void resizeImage(Image *image)
{
	int screenHeight = GetScreenHeight();
	int screenWidth = GetScreenWidth();

	if (image->height > screenHeight)
	{
		float scale = (float)screenHeight / image->height;
		int newWidth = (int)(image->width * scale);
		int newHeight = screenHeight;
		ImageResize(image, newWidth, newHeight);
	}

	if (image->width > screenWidth)
	{
		float scale = (float)screenWidth / image->width;
		int newWidth = screenWidth;
		int newHeight = (int)(image->height * scale);
		ImageResize(image, newWidth, newHeight);
	}
}

Rectangle *shuffleSrcImage(int widthSize, int heightSize, float textureWidth, float textureHeight)
{
	Rectangle *srcRec = (Rectangle *)malloc(sizeof(Rectangle) * (size_t)(widthSize * heightSize));
	int len = widthSize * heightSize;
	float frameWidth = textureWidth / (float)widthSize;
	float frameHeight = textureHeight / (float)heightSize;
	for (int i = 0; i < len; ++i)
	{
		float x = i % widthSize * frameWidth;
		// float y = i / heightSize * frameHeight;
		float y = i / widthSize * frameHeight;
		*(srcRec + i) = (Rectangle){x, y, frameWidth, frameHeight};
	}
	return srcRec;
}

void drawImage(Rectangle srcRec[], Texture2D *texture, int indices[], int widthSize, int heightSize)
{
	int len = widthSize * heightSize;
	float frameWidth = srcRec[0].width;
	float frameHeight = srcRec[0].height;
	float x = 0;
	float y = 0;
	for (int i = 0; i < len; ++i)
	{
		int curPos = indices[i];
		x = (i % widthSize) * frameWidth;
		y = (i / heightSize) * frameHeight;
		DrawTextureRec(*texture, srcRec[curPos], (Vector2){x, y}, WHITE);
	}
}

int getChosenRec(Vector2 mousePos, int widthSize, int heightSize, float frameWidth, float frameHeight)
{
	float x = mousePos.x;
	float y = mousePos.y;
	int len = widthSize * heightSize;
	for (int i = 0; i < len; ++i)
	{
		float curx = (i % widthSize) * frameWidth;
		float cury = (i / heightSize) * frameHeight;
		if ((x >= curx && x <= curx + frameWidth) && (y >= cury && y <= cury + frameHeight))
		{
			return i;
		}
	}

	return -1;
}

// updateRec(indices, firstChosenRec, secondChosenRec);
void updateRec(int indices[], int indexa, int indexb)
{
	int tmp = indices[indexa];
	indices[indexa] = indices[indexb];
	indices[indexb] = tmp;
}

// 4 * 4
/*
0       1       2       3
4       5       6       7
8       9       10      11
12      13      14      15
*/
bool closeEnough(int indexa, int indexb, int widthSize)
{
	if (indexa == indexb)
	{
		return false;
	}

	int rowa = indexa / widthSize;
	int cola = indexa % widthSize;
	int rowb = indexb / widthSize;
	int colb = indexb % widthSize;

	if ((rowa == rowb && abs(cola - colb) == 1) || (cola == colb && abs(rowa - rowb) == 1))
	{
		return true;
	}

	return false;
}

bool isSolved(int indices[], int len)
{
	for (int i = 0; i < len; i++)
	{
		if (indices[i] != i)
			return false;
	}
	return true;
}

void usage(void) {
	printf("Usage: ./main <imageFilePath> <widthSize> <heightSize>\n");
}

int main(int argc, char* argv[])
{
	if (argc != 4) {
		usage();
		return 0;
	}

	const char* imageFilePath = argv[1];
	const int widthSize = atoi(argv[2]);
	const int heightSize = atoi(argv[3]);

	const int screenWidth = 1200;
	const int screenHeight = 1000;

	InitWindow(screenWidth, screenHeight, "raylib [textures] example - texture source and destination rectangles");

	// monitor width: 2560
	// monitor height: 1440
	// int monitor = GetCurrentMonitor();
	// int monitorWidth = GetMonitorWidth(monitor);
	// int monitorHeight = GetMonitorHeight(monitor);

	// image width: 1280
	// image height: 2774

	// 加载图片并和当前游戏屏幕大小进行匹配
	Image image = LoadImage(imageFilePath);
	if (image.data == NULL || image.height == 0 || image.width == 0) {
		printf("image path [%s] invalid.\n", imageFilePath);
		return -1;
	}
	
	resizeImage(&image);
	Texture2D texture = LoadTextureFromImage(image);
	UnloadImage(image);

	// 对图片进行分割，分割成 4*4 的 16个块
	Rectangle *srcRec = NULL;
	srcRec = shuffleSrcImage(widthSize, heightSize, texture.width, texture.height);

	// 初始化显示时，需要打乱 16 个块的顺序
	int recSize = widthSize * heightSize;
	int *indices = (int *)malloc(sizeof(int) * (size_t)recSize);
	shuffleIndices(indices, recSize);

	// 初始化一个变量，用于记录鼠标点击次数
	// 获取当前鼠标位置，根据鼠标位置，获取当前选中的 srcRec 块
	int mousePressedCnt = 0;
	int firstChosenRec = -1;
	int secondChosenRec = -1;

	SetTargetFPS(60);
	bool wrongChosenRec = false;
	while (!WindowShouldClose())
	{
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			mousePressedCnt++;
			if (mousePressedCnt == 3)
			{
				mousePressedCnt = 1;
			}
			Vector2 mousePos = GetMousePosition();
			int chosenRec = getChosenRec(mousePos, widthSize, heightSize, srcRec[0].width, srcRec[0].height);
			if (mousePressedCnt == 1)
			{
				firstChosenRec = chosenRec;
			}
			else
			{
				secondChosenRec = chosenRec;
			}

			if (mousePressedCnt == 2)
			{
				if (closeEnough(firstChosenRec, secondChosenRec, widthSize))
				{
					wrongChosenRec = false;
					updateRec(indices, firstChosenRec, secondChosenRec);
				}
				else
				{
					wrongChosenRec = true;
				}
			}
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);
		drawImage(srcRec, &texture, indices, widthSize, heightSize);
		DrawText(TextFormat("MouseCnt: %d", mousePressedCnt), GetScreenWidth() - 200, GetScreenHeight() - 200, 20, RED);
		if (wrongChosenRec)
		{
			DrawText(TextFormat("Please choose nearby cube"), GetScreenWidth() - 400, 200, 20, RED);
		}
		if (isSolved(indices, recSize))
		{
			DrawText("You Win!", GetScreenWidth() / 2 - 50, GetScreenHeight() / 2, 40, GREEN);
		}
		EndDrawing();
	}

	free(srcRec);
	srcRec = NULL;

	free(indices);
	indices = NULL;

	UnloadTexture(texture);

	CloseWindow();

	return 0;
}
