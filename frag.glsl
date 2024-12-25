char *frag = R""( 
// TODO: shwoopy color band on extrude
// TODO: highlight faces you're about to click on (behaves just like a button)

#version 330 core
in BLOCK {
    vec3 position_World;
    vec3 normal_World;
    flat uint patch_index;
} fs_in;

uniform vec3 eye_World;
uniform float dark_light_tween;

uniform int feature_plane_is_active;
uniform vec3 feature_plane_normal;
uniform float feature_plane_signed_distance_to_world_origin;

uniform int   hover_plane_is_active;
uniform vec3  hover_plane_normal;
uniform float hover_plane_signed_distance_to_world_origin;

uniform int mode;

out vec4 _gl_FragColor;

void main() {
    vec3 N = normalize(fs_in.normal_World);
    vec3 rgb = vec3(0.0);
    float a = 1.0;


    // TODO: sparkly color shwoop up the extrude axis when it extudes (TODO: see olllld commit)
    // *shwing*
    if (mode == 0) {

        vec3 rgb_gooch3 = vec3(0);
        if (true) { // three way rainbow gooch
            for (int d = 0; d < 3; ++d) {
                vec3 warm_color = vec3(0.0);
                warm_color[d] = 1.0;

                vec3 cool_color = (vec3(1.0) - warm_color) / 1.5;

                vec3 L = vec3(0.0);
                L[d] = 1.0;

                float LN = dot(L, N);
                float t = LN; // 0.5 + 0.5 * LN;
                rgb_gooch3 += 0.3 * mix(vec3(1.0), mix(cool_color, warm_color, t), 0.8);
            }
        }


        if (false) { // sunlight gooch
            vec3 warm_color = vec3(1.0, 0.7, 0.3);
            vec3 cool_color = vec3(0.3, 0.7, 1.0);
            vec3 L = vec3(0.0, 1.0, 0.0);
            float LN = dot(L, N);
            float t = 0.5 + 0.5 * LN;
            rgb += 0.7 * mix(cool_color, warm_color, t);
        }

        if (false) { // eyelight gooch
            vec3 warm_color = vec3(1.0, 0.7, 0.3);
            vec3 cool_color = vec3(0.3, 0.7, 1.0);
            vec3 L = normalize(eye_World - fs_in.position_World);
            float LN = dot(L, N);
            float t = LN;
            rgb += 0.7 * mix(cool_color, warm_color, t);
        }

        if (true) { // eye light phong fresnel
            vec3 L = normalize(eye_World - fs_in.position_World);
            vec3 E = normalize(eye_World - fs_in.position_World);
vec3 H = normalize(L + E);
float LN = dot(L, N);
float F0 = 0.05;
float diffuse = max(0.0, LN);
float specular = pow(max(0.0, dot(N, H)), 256);
float fresnel = F0 + (1 - F0) * pow(1.0 - max(0.0, dot(N, H)), 5);
vec3 ambient = vec3(1.0);
rgb += mix(0.15, 0.4, dark_light_tween) * ambient;
rgb += mix(0.30, 0.55, dark_light_tween) * diffuse;
rgb += mix(0.3 , 0.2, dark_light_tween) * specular;
rgb += mix(0.15, 0.3, dark_light_tween) * fresnel;
rgb = mix(rgb, vec3(dark_light_tween), 0.6f);
}

if (feature_plane_is_active != 0) { // feature plane override
    if (dot(fs_in.normal_World, feature_plane_normal) > 0.99) {
        if (abs(dot(fs_in.position_World, feature_plane_normal) - feature_plane_signed_distance_to_world_origin) < 0.01) {
            rgb = mix(rgb, vec3(1.0 - dark_light_tween), 0.05);
        }
    }
}

if (hover_plane_is_active != 0) { // hover plane override
    if (dot(fs_in.normal_World, hover_plane_normal) > 0.99) {
        if (abs(dot(fs_in.position_World, hover_plane_normal) - hover_plane_signed_distance_to_world_origin) < 0.01) {
            rgb = mix(rgb, vec3(1.0 - dark_light_tween), 0.12);
        }
    }
}

// rgb = clamp(vec3(0.0f), vec3(0.9f), rgb);
} else if ((mode == 1) || (mode == 3)) {
    int i = (mode == 1) ? int(fs_in.patch_index) : gl_PrimitiveID;
    rgb.r = (i % 256);
    rgb.g = ((i / 256) % 256);
    rgb.b = ((i / (256 * 256)) % 256);
    rgb /= 255.0;
}

_gl_FragColor = vec4(rgb, a);
}
)"";
