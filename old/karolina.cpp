const int bufferLength = 255;
char buffer[bufferLength];

#include "playground.cpp"
#include "easy_mode.cpp"

int main() {

    uint numVertices;
    uint numFaces;
    {
        numVertices = 0;
        numFaces = 0;
        FILE* filePointer = fopen("teapot.obj", "r");
        while(fgets(buffer, bufferLength, filePointer)) {
            if(buffer[0] == 'v') {
                numVertices++;
            } else if (buffer[0] ==  'f') {
                numFaces++;
            }
        }
        fclose(filePointer);
    }

    vec3 *vertices;
    uint3 *faces;
    {
        vertices = (vec3 *) malloc(numVertices * sizeof(vec3));
        faces = (uint3 *) malloc(numFaces * sizeof(uint3));

        FILE *filePointer = fopen("teapot.obj", "r");
        int vertexIndex = 0;
        int faceIndex = 0;
        while(fgets(buffer, bufferLength, filePointer)) {
            if(buffer[0] == 'v') {
                real v1, v2, v3;
                sscanf(buffer, "%*c %f %f %f", &v1, &v2, &v3);
                vertices[vertexIndex++] = 30.0f * V3(v1, v2, v3);
            } else if (buffer[0] ==  'f') {
                uint f1, f2, f3;
                sscanf(buffer, "%*c %d %d %d", &f1, &f2, &f3);
                faces[faceIndex++] = {f1 - 1, f2 - 1, f3 - 1}; // fix it to be 0-indexed
            }
        }
        fclose(filePointer);
    }

    Camera camera_2D = make_Camera2D(256.0f);
    Camera orbit_camera_3D = make_OrbitCamera3D(0.5f * 256.0f / TAN(RAD(30.0f)), RAD(60.0f));
    Camera first_person_camera_3D = make_FirstPersonCamera3D({ 0.0f, 16.0f, 0.5f * 256.0f / TAN(RAD(30.0f)) }, RAD(60.0f));
    Camera *camera = &orbit_camera_3D;
    real time = 0.0f;
    while (begin_frame(camera)) {
        time += 0.0167f;

        // switch cameras
        {
            if (key_pressed['1']) {
                camera = &camera_2D;
                pointer_unlock();
            }

            if (key_pressed['2']) {
                camera = &orbit_camera_3D;
                pointer_unlock();
            }

            if (key_pressed['3']) {
                camera = &first_person_camera_3D;
                pointer_lock();
            }
        }

        mat4 PV = camera->get_PV();

        { // draw ground
            eso_begin(PV, SOUP_QUADS);
            real r = 128.0f;
            eso_color(basic.white);
            eso_vertex(-r, 0.0f, -r);
            eso_vertex(-r, 0.0f,  r);
            eso_vertex( r, 0.0f,  r);
            eso_vertex( r, 0.0f, -r);
            eso_end();
        }

        { // draw a pillar
            eso_begin(PV, SOUP_LINE_LOOP);
            real r = 200.0f;
            eso_color(basic.red);
            
            eso_vertex( 0.0f, 0.0f, 0.0f);
            eso_vertex( 0.0f, 300.0f, 0.0f);
            eso_end();
        }

        { // draw lines
            eso_begin(PV, SOUP_LINE_LOOP);
            real r = 200.0f;
            eso_color(basic.red);
            eso_vertex(-r, 0.0f, -r);
            eso_vertex(-r, 0.0f,  r);
            eso_vertex( r, 0.0f,  r);
            eso_vertex( r, 0.0f, -r);
            eso_vertex( 0.0f, 0.0f, 0.0f);
            eso_end();
        }

        { // draw pyramid
            eso_begin(PV, SOUP_QUADS);
            real r = 30.0f;
            eso_color(basic.blue);
            eso_vertex(-r, 1.0f, -r);
            eso_vertex(-r, 1.0f,  r);
            eso_vertex( r, 1.0f,  r);
            eso_vertex( r, 1.0f, -r);
            eso_end();

            eso_begin(PV, SOUP_TRIANGLES);
            eso_color(basic.green, 0.5f);
            eso_vertex(-r + 100, 1.0f, -r);
            eso_vertex(-r + 100, 1.0f,  r);
            eso_vertex(0.0f, 2*r, 0.0f);
            eso_vertex(-r + 100, 1.0f,  r);
            eso_vertex( r + 100, 1.0f,  r);
            eso_vertex(0.0f, 2*r, 0.0f);
            eso_vertex( r + 100, 1.0f,  r);
            eso_vertex( r + 100, 1.0f, -r);
            eso_vertex(0.0f, 2*r, 0.0f);
            eso_vertex(-r + 100, 1.0f, -r);
            eso_vertex( r + 100, 1.0f, -r);
            eso_vertex(0.0f, 2*r, 0.0f);
            eso_end();
        }

        { // draw teapot
            eso_begin(PV, SOUP_TRIANGLES);
            eso_color(color_rainbow_swirl(time / 10.0f), 0.5f);
            for_(index, numFaces) {
                for_(d, 3) {
                    eso_vertex(vertices[faces[index][d]]);
                }
            }
            eso_end();
        }
    }
}
