#include "playground.cpp"
#include "easy_mode.cpp"

#pragma warning(disable : 4305 4244)

// M4_RotationAboutXAxis ...
// #define MAX_NUM_CHILD_BONES 16
// uint num_children;
// Bone *children[MAX_NUM_BONES];
// Bone *parent;

// mat4 SOUP_LINES overlay, ...




struct Bone {
    real length;
    uint index;
    uint parent_index; // if this is the same as the bone's index, the bone has no parent
    vec3 local_position;
    mat4 local_rotation;

};

struct EmmaMesh {
    uint num_vertices;
    vec3 *vertex_positions;

    uint num_triangles;
    uint3 *triangle_indices;

    uint num_bones;
    Bone *bones;
    uint4 *vertex_bone_indices;
    vec4  *vertex_bone_weights;
};

EmmaMesh mesh;

mat4 get_B(uint bone_index, bool bind = false) {
    mat4 result = M4_Identity();
    while (true) {
        mat4 T = M4_Translation(mesh.bones[bone_index].local_position);
        mat4 R = (bind) ? M4_Identity() : mesh.bones[bone_index].local_rotation;
        result = T * R * result;
        uint prev_bone_index = bone_index;
        bone_index = mesh.bones[bone_index].parent_index;
        if (prev_bone_index == bone_index) break;
    }
    return result;
}

real smoothstep(real x, real edge0, real edge1) {
   x = CLAMP(INVERSE_LERP(x, edge0, edge1), 0, 1);
   return x * x * (3.0f - 2.0f * x);
}

int main() {
{
        uint meshSegments = 20;
        real segmentLen = 20.0f;
        uint boneSegments = 3;
        real boneLen = segmentLen * meshSegments / boneSegments;

        mesh = {};

        { // vertex_positions
            mesh.num_vertices = 4 + meshSegments * 4;
            mesh.vertex_positions = (vec3 *) calloc(mesh.num_vertices, sizeof(vec3));
            uint num_layers_of_vertices = meshSegments + 1;
            uint currVert = 0;
            vec3 offset = { 0, segmentLen / 2, segmentLen / 2 };
            for_(i, num_layers_of_vertices) {
                mesh.vertex_positions[currVert++] = V3(i*segmentLen, 0, 0) - offset; // back bottom corner
                mesh.vertex_positions[currVert++] = V3(i*segmentLen, 0, segmentLen) - offset; // front bottom corner
                mesh.vertex_positions[currVert++] = V3(i*segmentLen, segmentLen, segmentLen) - offset; // front top corner
                mesh.vertex_positions[currVert++] = V3(i*segmentLen, segmentLen, 0) - offset; // back top corner
            }
            ASSERT(currVert == mesh.num_vertices);
        }

        { // triangle_indices
            mesh.num_triangles = 4 + meshSegments*8;
            mesh.triangle_indices = (uint3 *) calloc(mesh.num_triangles, sizeof(uint3));
            uint currTriangle = 0;
            // sides
            for (uint i = 0; i < mesh.num_vertices-4; i++) {
                mesh.triangle_indices[currTriangle++] = { i, i+3, i+4 };
                mesh.triangle_indices[currTriangle++] = { i, i+1, i+4 };
            }
            // ends
            mesh.triangle_indices[currTriangle++] = { 0, 1, 3 };
            mesh.triangle_indices[currTriangle++] = { 1, 2, 3 };
            mesh.triangle_indices[currTriangle++] = { mesh.num_vertices-3, mesh.num_vertices-2, mesh.num_vertices-1 };
            mesh.triangle_indices[currTriangle++] = { mesh.num_vertices-4, mesh.num_vertices-3, mesh.num_vertices-1 };
            ASSERT(currTriangle == mesh.num_triangles);
        }

        { // bones
            mesh.num_bones = boneSegments;
            mesh.bones = (Bone *) calloc(mesh.num_bones, sizeof(Bone));
            mesh.vertex_bone_indices = (uint4 *) calloc(mesh.num_vertices, sizeof(uint4));
            mesh.vertex_bone_weights = (vec4 *) calloc(mesh.num_vertices, sizeof(vec4));   
            for_(i, mesh.num_bones) {
                mesh.bones[i].index = i;
                if (i == 0) {
                    mesh.bones[i].parent_index = 0;
                    mesh.bones[i].local_position = {};
                } else {
                    mesh.bones[i].parent_index = i - 1;
                    mesh.bones[i].local_position = { mesh.bones[mesh.bones[i].parent_index].length, 0, 0 };
                }
                mesh.bones[i].local_rotation = M4_Identity();
                mesh.bones[i].length = boneLen;
            }

            for_(vertex_index, mesh.num_vertices) {
                real t = real(vertex_index / 4) * 4 / mesh.num_vertices; // [0, 1]
                real eps = 0.1;
                real w = smoothstep(t, .5 - eps, .5 + eps);
                mesh.vertex_bone_indices[vertex_index] = { 0,     1 };
                mesh.vertex_bone_weights[vertex_index] = { 1 - w, w };
                ASSERT(sum(mesh.vertex_bone_weights[vertex_index]));
            }
        }
    }

    Camera camera = make_OrbitCamera3D(0.5f * 256.0f / TAN(RAD(30.0f)), RAD(60.0f));
    #if 1
    camera.angle_of_view = 1.04719758;
    camera.euler_angles = {-1.57079637, -0.0375000238, 0};
    camera.pre_nudge_World = {156.293091, 1.06562614};
    camera.ortho_screen_height_World = 389.513611;
    #endif

    //mesh.bones[0].local_position += { 0.0, 0.0, 50.0f };

    real time = 0.0f;
    while (begin_frame(&camera)) {
        if (key_pressed['Q']) exit(1);
        if (!key_toggled['P']) {
            time += 0.0167f;
        }
        mat4 PV = camera.get_PV();

        mesh.bones[0].local_rotation = M4_RotationAboutYAxis(SIN(2.0f * time));
        mesh.bones[1].local_rotation = M4_RotationAboutYAxis(SIN(3.0f * time));

        { // render triangle mesh
            eso_begin(PV, SOUP_TRI_MESH);
            for_(triangle_index, mesh.num_triangles) {
                for_(d, 3) {
                    uint vertex_index = mesh.triangle_indices[triangle_index][d];
                    vec3 color = {};
                    vec3 s_bind = mesh.vertex_positions[vertex_index]; 
                    vec3 newVertex = {};
                    // TODO: implement bone parenting and adjusting rendering accordingly
                    for_(assigned_bone_index, 4) {
                        uint j = mesh.vertex_bone_indices[vertex_index][assigned_bone_index];
                        real bone_weight = mesh.vertex_bone_weights[vertex_index][assigned_bone_index];
                        color += get_kelly_color(j) * bone_weight;

                        newVertex += bone_weight * transformPoint(get_B(j) * inverse(get_B(j, true)), s_bind);
                    }
                    eso_color(color);
                    eso_vertex(newVertex);

                }
            }
            eso_end();
        }   

        if (!key_toggled['B']) {
            for_(pass, 2) {
                for_(bone_index, mesh.num_bones) {
                    eso_begin(PV, SOUP_LINES);
                    eso_overlay(true);
                    eso_color((pass == 0) ? monokai.black : get_kelly_color(bone_index));
                    real eps = (pass == 0) ? 4.0f : 0.0f;

                    vec3 base_local = {};
                    vec3 tip_local = { mesh.bones[bone_index].length, 0, 0};
                    mat4 W = get_B(bone_index);
                    eso_size(eps + 16);
                    eso_vertex(transformPoint(W, base_local));
                    eso_size(eps + 0);
                    eso_vertex(transformPoint(W, tip_local));
                    eso_end();
                }
            }
        }      
    }
}

// fun zone
#if 0
    
    old mesh renderer    
        // vec3 global_position = get_global_position(mesh.bones[bone_index], mesh); 
        // newVertex += (transformPoint(mesh.bones[bone_index].local_rotation, vertex_position - mesh.bones[bone_index].local_position) + mesh.bones[bone_index].local_position) * bone_weight;
                        
    old bone renderer    
         // eso_size(eps + 16);
        // eso_vertex(get_global_position(mesh.bones[bone_index], mesh));
        // eso_size(eps + 0);
        // vec3 v = transformVector(mesh.bones[bone_index].local_rotation, V3(mesh.bones[bone_index].length, 0, 0));
        // eso_vertex(get_global_position(mesh.bones[bone_index], mesh) + v);


#endif

#if 0
vec3 get_global_position(Bone bone, EmmaMesh mesh) {
    if(bone.parent_index == -1) {
        return bone.local_position;
    } else {
        return transformPoint(mesh.bones[bone.parent_index].local_rotation, bone.local_position) + get_global_position(mesh.bones[bone.parent_index], mesh);
    }
}

mat4 get_rotation_from_root(Bone bone, EmmaMesh mesh) {
    if(bone.parent_index == -1) {
        return bone.local_rotation;
    } else {
        return bone.local_rotation * get_rotation_from_root(mesh.bones[bone.parent_index], mesh);
    }
}
#endif
