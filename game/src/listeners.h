/* date = March 3rd 2022 3:33 pm */

#ifndef LISTENERS_H
#define LISTENERS_H

void
OnKey(vp_keys Key, bool32 IsDown);

void
OnMouse(vp_buttons Button, bool32 IsDown);

void
ToggleCameraLock(b32 toggle);

b32
CameraLockedIsLocked();

void
HandleInput();

void
HandlePlayerMovement(vp_keys Key);

void
LockCamera();

#endif //LISTENERS_H
