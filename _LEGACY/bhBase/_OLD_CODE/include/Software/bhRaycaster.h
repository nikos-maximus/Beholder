#ifndef BH_RAYCASTER_H
#define BH_RAYCASTER_H

int bhRaycaster_Init(int width, int height);
void bhRaycaster_Destroy();
void bhRaycaster_DrawMap(struct SDL_Surface* surf);
void bhRaycaster_HandleInput();

#endif //BH_RAYCASTER_H
