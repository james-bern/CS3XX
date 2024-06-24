#include "playground.cpp"
#include "easy_mode.cpp"
int main() {
    Camera camera_2D = make_Camera2D(256.0f);
    Camera orbit_camera_3D = make_OrbitCamera3D(0.5f * 256.0f / TAN(RAD(30.0f)), RAD(60.0f));
    Camera first_person_camera_3D = make_FirstPersonCamera3D({ 0.0f, 16.0f, 0.5f * 256.0f / TAN(RAD(30.0f)) }, RAD(60.0f));
    Camera *camera = &camera_2D;
    real time = 0.0f;
    //// draw teapot
    double *vertex_positions = (double *) calloc(99999, sizeof(double)); 
    int *triangle_indices = (int *) calloc(99999, sizeof(int)); 
    FILE *file = fopen("teapot.txt", "r");
    ASSERT(file);
    int num_vertices = 0;
    int num_triangles = 0;
    char point_type;
    double a, b, c;
    char line[1024];
    while (fgets(line, 1024, file) != NULL) { 
        sscanf(line,"%c %lf %lf %lf", &point_type, &a, &b, &c);
        if (point_type == 'v') {
            vertex_positions[3 * num_vertices + 0] = a;
            vertex_positions[3 * num_vertices + 1] = b;
            vertex_positions[3 * num_vertices + 2] = c;
            ++num_vertices;
        }
        if (point_type == 'f') {
            triangle_indices[3 * num_triangles + 0] = a - 1;
            triangle_indices[3 * num_triangles + 1] = b - 1;
            triangle_indices[3 * num_triangles + 2] = c - 1;
            ++num_triangles;
         }
    }
    fclose(file);
    while (begin_frame(camera)) {
        time += 0.0167f;
        int traingle_index = 0; 
        while (traingle_index < (3 * num_triangles)) {
            int i = triangle_indices[traingle_index];
            int j = triangle_indices[traingle_index + 1];
            int k = triangle_indices[traingle_index + 2];

            
            double x_i = (30.0f * vertex_positions[3 * i + 0]);
            double y_i = (30.0f * vertex_positions[3 * i + 1]);
            double z_i = (30.0f * vertex_positions[3 * i + 2]);
            double x_j = (30.0f * vertex_positions[3 * j + 0]);
            double y_j = (30.0f * vertex_positions[3 * j + 1]);
            double z_j = (30.0f * vertex_positions[3 * j + 2]);
            double x_k = (30.0f * vertex_positions[3 * k + 0]);
            double y_k = (30.0f * vertex_positions[3 * k + 1]);
            double z_k = (30.0f * vertex_positions[3 * k + 2]);
            
            
            eso_begin(camera->get_PV(), SOUP_TRIANGLES);
            eso_color(basic.white);
            eso_vertex(x_i, y_i, z_i);
            eso_vertex(x_j, y_j, z_j);
            eso_vertex(x_k, y_k, z_k);
            eso_end();
       
            traingle_index += 3;
        }
        ////
        

        { // switch cameras
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

        
    }
}
