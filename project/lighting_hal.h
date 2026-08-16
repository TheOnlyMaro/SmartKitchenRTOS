#ifndef LIGHTING_HAL_H
#define LIGHTING_HAL_H

#include <stdbool.h>
#include <stdint.h>

void LightingHal_Init(void);
uint32_t LightingHal_ReadAmbient(void);
void LightingHal_SetOutput(bool on);

#endif /* LIGHTING_HAL_H */
