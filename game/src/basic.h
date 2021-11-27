#include <stdlib.h>
struct res
{
    int w;
    int h;
};

typedef struct vec2
{
    float x;
    float y;
} vec2;

typedef struct allocator
{
    void *Start;
    void *End;
} allocator;

void scale_vector(vec2 *vector, float scaler)
{
    vector->x *= scaler;
    vector->y *= scaler;
}

typedef struct atlas_member
{
    float x;
    float y;
    float w;
    float h;
} atlas_member; 

typedef struct rectangle
{
    int w;
    int h;
} rectangle;

