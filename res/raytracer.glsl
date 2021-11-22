#version 430

#define WORLD_SIZE_X 512
#define WORLD_SIZE_Y 64
#define WORLD_SIZE_Z 512

uniform vec2 screenSize;
uniform mat4 cameraMatrix;
uniform vec3 cameraPos;
uniform vec3 lightPos;

#define MAX_ITERATIONS 128
#define GROUP_SIZE 30
#define ENABLE_SHADOWS 1
#define ENABLE_REFLECTIONS 1

const float fov = 70.0f;

#define FACE_POSX 0
#define FACE_NEGX 1

#define FACE_POSY 2
#define FACE_NEGY 3

#define FACE_POSZ 4
#define FACE_NEGZ 5

const vec3 surfaceNormals[] = vec3[6](
vec3(1, 0, 0),
vec3(-1, 0, 0),
vec3(0, 1, 0),
vec3(0, -1, 0),
vec3(0, 0, 1),
vec3(0, 0, -1)
);

layout (binding = 0, rgba32f) uniform image2D destTex;
layout (local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE) in;
layout (std430, binding = 3) buffer voxelBuffer
{
    uint voxels[];// glsl doesnt support types smaller than 32-bit
};



vec3 calc_ray(float x, float y) {
    float sc = tan(radians(fov * 0.5));
    float aspect = screenSize.x / screenSize.y;

    float Px = float(2 * (x + 0.5) / screenSize.x - 1) * sc * aspect;
    float Py = float(1 - 2 * (y + 0.5) / screenSize.y) * sc;

    vec4 rayPWorld = cameraMatrix * vec4(Px, Py, 1, 1);
    return normalize(rayPWorld.xyz);
}

int getCurrent(float location, float floor, int orientation) {
    if (location == floor && orientation == -1) {
        return int(floor - 1);
    } else {
        return int(floor);
    }
}

float getNextR(float location, float floor, int orientation, float sr) {
    if (location == floor)
    {
        // This returns infinity when sr is infinity.
        return 1 * sr;
    }
    else
    {
        // The first term will always be greater than zero and therefore returns inifnity when sr is infinity.
        return abs(floor + (orientation == 1 ? 1 : 0) - location) * sr;
    }
}

int lookup(uint x, uint y, uint z) {
    if (x >= WORLD_SIZE_X || y >= WORLD_SIZE_Y || z >= WORLD_SIZE_Z) {
        return 0;
    } else {
        uint idx = x + WORLD_SIZE_X * (y + WORLD_SIZE_Y * z);
        uint vox = voxels[idx];
        return int(vox);
    }
}

vec4 getColor(uint x, uint y, uint z, int o) {
    vec3 color = vec3(0.8, 0.8, 0.8);

    if (o == 0||o==1) color *= 0.77;
    else if (o == 2||o==3) color *= 0.95;
    else color *= 0.69;

    if (y < 48) {
        color = mix(color, vec3(0, 0.35, 0.65), 0.85);
    }

    return vec4(color, 1.0);
}

int trace(in vec3 loc, in vec3 ray, in int iterations, out ivec3 intersectionBlock, out vec3 intersection, out int face) {
    ivec3 orientation = ivec3(sign(ray.x), sign(ray.y), sign(ray.z));
    vec3 sr = vec3(abs(1/ray.x), abs(1/ray.y), abs(1/ray.z));
    vec3 f = vec3(floor(loc.x), floor(loc.y), floor(loc.z));
    ivec3 c = ivec3(getCurrent(loc.x, f.x, orientation.x), getCurrent(loc.y, f.y, orientation.y), getCurrent(loc.z, f.z, orientation.z));
    vec3 nr = vec3(getNextR(loc.x, f.x, orientation.x, sr.x), getNextR(loc.y, f.y, orientation.y, sr.y), getNextR(loc.z, f.z, orientation.z, sr.z));

    for (;;) {
        if (nr.x < nr.y && nr.x < nr.z)// xside
        {
            if (nr.x > iterations) return 0;
            int hit = lookup(c.x += orientation.x, c.y, c.z);
            if (hit != 0) {
                face = FACE_NEGX - clamp(orientation.x, 0, 1);
                intersectionBlock = c;
                intersection = vec3(c.x, loc.y + nr.x * ray.y, loc.z + nr.x * ray.z);
                return hit;
            }
            nr.x += sr.x;
        }
        else if (nr.y < nr.x && nr.y < nr.z)// yside
        {
            if (nr.y > iterations) return 0;
            c.y += orientation.y;
            if (c.y < 0 || c.y > WORLD_SIZE_Y) return 0;
            int hit = lookup(c.x, c.y, c.z);
            if (hit != 0) {
                face = FACE_NEGY - clamp(orientation.y, 0, 1);
                intersectionBlock = c;
                intersection = vec3(loc.x + nr.y * ray.x, c.y, loc.z + nr.y * ray.z);
                return hit;
            }
            nr.y += sr.y;
        }
        else // zside
        {
            if (nr.z > iterations) return 0;
            int hit = lookup(c.x, c.y, c.z += orientation.z);
            if (hit != 0) {
                face = FACE_NEGZ - clamp(orientation.z, 0, 1);
                intersectionBlock = c;
                intersection = vec3(loc.x + nr.z * ray.x, loc.y + nr.z * ray.y, c.z);
                return hit;
            }
            nr.z += sr.z;
        }
    }

    return 0;
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 pixelPosition = gl_GlobalInvocationID.xy;

    vec3 ray = calc_ray(pixelPosition.x, pixelPosition.y);

    ivec3 intersectionBlock;
    vec3 intersection;
    int face;
    int block = trace(cameraPos, ray, MAX_ITERATIONS, intersectionBlock, intersection, face);

    vec4 color = vec4(0.3, 0.7, 1.0, 1);
    if (block != 0) {


#if ENABLE_REFLECTIONS
        if (intersectionBlock.y < 48) {
            ray = reflect(ray, surfaceNormals[face]);
            ivec3 ib2;
            vec3  b2;
            block = trace(intersection + vec3(0, 1, 0), ray, 64, ib2, b2, face);
            if (block != 0) {
                color = mix(getColor(ib2.x, ib2.y, ib2.z, face), getColor(intersectionBlock.x, intersectionBlock.y, intersectionBlock.z, face), 0.5);
            } else {
                color = mix(color, getColor(intersectionBlock.x, intersectionBlock.y, intersectionBlock.z, face), 0.5);
            }
        } else
#endif
        {
            color = getColor(intersectionBlock.x, intersectionBlock.y, intersectionBlock.z, face);
        }

#if ENABLE_SHADOWS
        // TODO Doesn't work very well
        vec3 shadowRaySrc = intersection + vec3(0, 1, 0);
        vec3 shadowRayDir = normalize(lightPos - shadowRaySrc);
        block = trace(shadowRaySrc, shadowRayDir, 64, intersectionBlock, intersection, face);
        if (block != 0) {
            color *= 0.4;
        }
#endif
    }

    imageStore(destTex, ivec2(pixelPosition.x, pixelPosition.y), color);
}