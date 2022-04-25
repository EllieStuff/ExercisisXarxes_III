#include "SceneManager.h"

int main()
{
	srand(time(NULL));

	SceneManager* scene = new SceneManager();

	scene->Update();
	return 0;
}