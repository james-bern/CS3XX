void entities_load(String filename, List<Entity> *entities) {
    #if 0
    {
        FORNOW_UNUSED(filename);
        return {};
        #elif 0
        List<Entity> result = {};
        result.num_entities = 8;
        result.entities = (Entity *) calloc(result.num_entities, sizeof(Entity));
        result.entities[0] = { EntityType::Line, 0, 0.0, 0.0, 1.0, 0.0 };
        result.entities[1] = { EntityType::Line, 1, 1.0, 0.0, 1.0, 1.0 };
        result.entities[2] = { EntityType::Line, 2, 0.0, 1.0, 0.0, 0.0 };
        result.entities[3] = { EntityType::Arc,  3, 0.5, 1.0, 0.5,    0.0, 180.0 };
        result.entities[4] = { EntityType::Arc,  4, 0.5, 1.0, 0.25,   0.0, 180.0 };
        result.entities[5] = { EntityType::Arc,  5, 0.5, 1.0, 0.25, 180.0, 360.0 };
        result.entities[6] = { EntityType::Arc,  6, 0.5, 1.0, 0.1,    0.0, 180.0 };
        result.entities[7] = { EntityType::Arc,  7, 0.5, 1.0, 0.1,  180.0, 360.0 };
        return result;
    }
    #endif
    list_free_AND_zero(entities);

    FILE *file = (FILE *) FILE_OPEN(filename, "r");
    ASSERT(file);

    bool inches = true; // if none read will assume inches
    
    *entities = {}; {
        #define PARSE_NONE   0
        #define PARSE_LINE   1
        #define PARSE_ARC    2
        #define PARSE_CIRCLE 3
        #define PARSE_UNITS  4
        uint mode = 0;

        auto convert = [&](real value) {
            if (inches && !config.usingInches) return MM(value);
            else if (!inches && config.usingInches) return INCHES(value);
            return value;
        };
        
        int code = 0;
        bool code_is_hot = false;
        Entity entity = {};
        #define MAX_LINE_LENGTH 1024
        static _STRING_CALLOC(line_from_file, MAX_LINE_LENGTH);
        while (string_read_line_from_file(&line_from_file, MAX_LINE_LENGTH, file)) {
            if (mode == PARSE_NONE) {
                if (string_matches_prefix(line_from_file, STRING("$INSUNITS"))) {
                    mode = PARSE_UNITS;
                    code_is_hot = false;
                } else if (string_matches_prefix(line_from_file, STRING("LINE"))) {
                    mode = PARSE_LINE;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Line;
                } else if (string_matches_prefix(line_from_file, STRING("ARC"))) {
                    mode = PARSE_ARC;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Arc;
                } else if (string_matches_prefix(line_from_file, STRING("CIRCLE"))) {
                    mode = PARSE_CIRCLE;
                    code_is_hot = false;
                    entity = {};
                    entity.type = EntityType::Circle;
                }
            } else {
                if (!code_is_hot) {
                    sscanf(line_from_file.data, "%d", &code);
                    // NOTE this initialization is sketchy but works
                    // probably don't make a habit of it
                    if (code == 0) {
                        list_push_back(entities, entity);
                        mode = PARSE_NONE;
                        code_is_hot = false;
                    }
                } else {
                    if (code == 62) {
                        int value;
                        sscanf(line_from_file.data, "%d", &value);
                        entity.color_code = (u8) value; 
                    } else {
                        float value;
                        sscanf(line_from_file.data, "%f", &value);
                        if (mode == PARSE_LINE) {
                            if (code == 10) {
                                entity.line.start.x = convert(value);
                            } else if (code == 20) {
                                entity.line.start.y = convert(value);
                            } else if (code == 11) {
                                entity.line.end.x = convert(value);
                            } else if (code == 21) {
                                entity.line.end.y = convert(value);
                            }
                        } else if (mode == PARSE_ARC) {
                            if (code == 10) {
                                entity.arc.center.x = convert(value);
                            } else if (code == 20) {
                                entity.arc.center.y = convert(value);
                            } else if (code == 40) {
                                entity.arc.radius = convert(value);
                            } else if (code == 50) {
                                entity.arc.start_angle_in_degrees = value;
                            } else if (code == 51) {
                                entity.arc.end_angle_in_degrees = value;
                            }
                        } else if (mode == PARSE_CIRCLE) {
                            if (code == 10) {
                                entity.circle.center.x = convert(value);
                            } else if (code == 20) {
                                entity.circle.center.y = convert(value);
                            } else if (code == 40) {
                                entity.circle.radius = convert(value);
                            }
                        } else {
                            ASSERT(mode == PARSE_UNITS);
                            inches = (value == 1);
                            mode = PARSE_NONE;
                        }
                    }
                }
                code_is_hot = !code_is_hot;
            }
        }
    }

    fclose(file);
}

bool drawing_save_dxf(Drawing *drawing_to_save, String filename) {
    List<Entity> *entities = &drawing_to_save->entities;

    FILE* file = (FILE *) FILE_OPEN(filename, "w");
    if (!file) {
        MESSAGE_FAILURE("SaveDWF: error opening %s for writing \n", filename);
        return false;
    }

    // Write DXF header
    fprintf(file, "999\nFile generated by Conversation\n");
    fprintf(file, "0\nSECTION\n2\nHEADER\n");

    fprintf(file, "0\nSECTION\n2\nHEADER\n");
    fprintf(file, "9\n$ACADVER\n1\nAC1021\n");
    fprintf(file, "9\n$INSBASE\n10\n0.0\n20\n0.0\n30\n0.0\n");
    fprintf(file, "9\n$INSUNITS\n70\n%d\n", config.usingInches ? 1 : 4);
    fprintf(file, "9\n$EXTMIN\n10\n0.0\n20\n0.0\n30\n0.0\n");
    fprintf(file, "9\n$EXTMAX\n10\n1000.0\n20\n1000.0\n30\n0.0\n");
    fprintf(file, "0\nENDSEC\n");
    // Write TABLES section
    fprintf(file, "0\nSECTION\n2\nTABLES\n");

    // LTYPE table
    fprintf(file, "0\nTABLE\n2\nLTYPE\n70\n1\n0\nLTYPE\n2\nCONTINUOUS\n70\n64\n3\nSolid line\n72\n65\n73\n0\n40\n0.0\n0\nENDTAB\n");

    // LAYER table
    fprintf(file, "0\nTABLE\n2\nLAYER\n70\n1\n0\nLAYER\n2\n0\n70\n64\n62\n7\n6\nCONTINUOUS\n0\nENDTAB\n");

    // STYLE table
    fprintf(file, "0\nTABLE\n2\nSTYLE\n70\n1\n0\nSTYLE\n2\nSTANDARD\n70\n0\n40\n0.0\n41\n1.0\n50\n0.0\n71\n0\n42\n0.2\n3\ntxt\n4\n\n0\nENDTAB\n");

    fprintf(file, "0\nENDSEC\n");

    // Write ENTITIES section
    fprintf(file, "0\nSECTION\n2\nENTITIES\n");
    for (Entity *entity = entities->array; entity < &entities->array[entities->length]; ++entity) {

        if (entity->type == EntityType::Line) {
            fprintf(file, "0\nLINE\n");
            fprintf(file, "8\n0\n");  // Layer
            fprintf(file, "62\n%d\n", entity->color_code);
            fprintf(file, "10\n%.6f\n", entity->line.start.x);
            fprintf(file, "20\n%.6f\n", entity->line.start.y);
            fprintf(file, "30\n0.0\n");  // Z coordinate (2D)
            fprintf(file, "11\n%.6f\n", entity->line.end.x);
            fprintf(file, "21\n%.6f\n", entity->line.end.y);
            fprintf(file, "31\n0.0\n");  // Z coordinate (2D)
        } else if (entity->type == EntityType::Arc) {
            fprintf(file, "0\nARC\n");
            fprintf(file, "8\n0\n");  // Layer
            fprintf(file, "62\n%d\n", entity->color_code);
            fprintf(file, "10\n%.6f\n", entity->arc.center.x);
            fprintf(file, "20\n%.6f\n", entity->arc.center.y);
            fprintf(file, "30\n0.0\n");  // Z coordinate (2D)
            fprintf(file, "40\n%.6f\n", entity->arc.radius);
            fprintf(file, "50\n%.6f\n", _WRAP_TO_0_360_INTERVAL(entity->arc.start_angle_in_degrees));
            fprintf(file, "51\n%.6f\n", _WRAP_TO_0_360_INTERVAL(entity->arc.end_angle_in_degrees));
        } else if (entity->type == EntityType::Circle) {
            fprintf(file, "0\nCIRCLE\n");
            fprintf(file, "8\n0\n");  // Layer
            fprintf(file, "62\n%d\n", entity->color_code);
            fprintf(file, "10\n%.6f\n", entity->circle.center.x);
            fprintf(file, "20\n%.6f\n", entity->circle.center.y);
            fprintf(file, "30\n0.0\n");  // Z coordinate (2D)
            fprintf(file, "40\n%.6f\n", entity->circle.radius);
        }
    }

    fprintf(file, "0\nENDSEC\n");

    // Write EOF
    fprintf(file, "0\nEOF\n");

    fclose(file);
    return true;
}

bool mesh_save_stl(WorkMesh *mesh_to_save, String filename) {
    FILE *file = FILE_OPEN(filename, "wb");
    if (!file) {
        return false;
    }

    int num_bytes = 80 + 4 + 50 * mesh_to_save->num_triangles;
    char *buffer = (char *) calloc(num_bytes, 1); {
        int offset = 80;
        memcpy(buffer + offset, &mesh_to_save->num_triangles, 4);
        offset += 4;
        for_(i, mesh_to_save->num_triangles) {
            vec3 triangle_normal; {
                triangle_normal = mesh_to_save->triangle_normals[i];
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_normal = { triangle_normal.x, -triangle_normal.z, triangle_normal.y };
            }
            memcpy(buffer + offset, &triangle_normal, 12);
            offset += 12;
            vec3 triangle_vertex_positions[3];
            for_(j, 3) {
                triangle_vertex_positions[j] = mesh_to_save->vertex_positions[mesh_to_save->triangle_tuples[i][j]];
                // 90 degree rotation about x: (x, y, z) <- (x, -z, y)
                triangle_vertex_positions[j] = { triangle_vertex_positions[j].x, -triangle_vertex_positions[j].z, triangle_vertex_positions[j].y };

            }
            memcpy(buffer + offset, triangle_vertex_positions, 36);
            offset += 38;
        }
    }
    fwrite(buffer, 1, num_bytes, file);
    free(buffer);

    fclose(file);
    return true;
}

MeshesReadOnly stl_load(String filename) {
    // history_record_state(history, manifold_manifold, mesh); // FORNOW

    { // mesh
        uint num_triangles;
        vec3 *triangle_soup;
        defer { free(triangle_soup); };
        {
            #define MAX_LINE_LENGTH 1024
            static _STRING_CALLOC(line_of_file, MAX_LINE_LENGTH);

            #define STL_FILETYPE_UNKNOWN 0
            #define STL_FILETYPE_ASCII   1
            #define STL_FILETYPE_BINARY  2
            uint filetype; {
                FILE *file = FILE_OPEN(filename, "r");
                string_read_line_from_file(&line_of_file, 80, file);
                filetype = (string_matches_prefix(line_of_file, STRING("solid"))) ? STL_FILETYPE_ASCII : STL_FILETYPE_BINARY;
                fclose(file);
            }

            if (filetype == STL_FILETYPE_ASCII) {
                char ascii_scan_dummy[64];
                real ascii_scan_p[3];
                List<real> ascii_data = {};

                FILE *file = FILE_OPEN(filename, "r");
                while (string_read_line_from_file(&line_of_file, MAX_LINE_LENGTH, file)) {
                    if (string_matches_prefix(line_of_file, STRING("vertex"))) {
                        sscanf(line_of_file.data, "%s %f %f %f", ascii_scan_dummy, &ascii_scan_p[0], &ascii_scan_p[1], &ascii_scan_p[2]);
                        for_(d, 3) list_push_back(&ascii_data, ascii_scan_p[d]);
                    }
                }
                fclose(file);
                num_triangles = ascii_data.length / 9;
                uint size = ascii_data.length * sizeof(real);
                triangle_soup = (vec3 *) malloc(size);
                memcpy(triangle_soup, ascii_data.array, size);
                list_free_AND_zero(&ascii_data);
            } else {
                ASSERT(filetype == STL_FILETYPE_BINARY);
                char *entire_file; {
                    FILE *file = FILE_OPEN(filename, "rb");
                    fseek(file, 0, SEEK_END);
                    long fsize = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    entire_file = (char *) malloc(fsize + 1);
                    fread(entire_file, fsize, 1, file);
                    fclose(file);
                    entire_file[fsize] = 0;
                }
                uint offset = 80;
                memcpy(&num_triangles, entire_file + offset, 4);
                offset += 4;
                uint size = num_triangles * 36;
                triangle_soup = (vec3 *) calloc(1, size);
                for_(i, num_triangles) {
                    offset += 12;
                    memcpy(&triangle_soup[3 * i], entire_file + offset, 36);
                    offset += 38;
                }
            }
            { // -90 degree rotation about x: (x, y, z) <- (x, z, -y)
                uint num_vertices = 3 * num_triangles;
                for_(i, num_vertices) {
                    triangle_soup[i] = { triangle_soup[i].x, triangle_soup[i].z, -triangle_soup[i].y };
                }
            }
        }

        uint num_vertices;
        vec3 *vertex_positions;
        uint3 *triangle_tuples;
        Arena *arena = ARENA_ACQUIRE();
        { // merge vertices (NOTE: only merges vertices that overlap exactly)
            num_vertices = 0;
            Map<vec3, uint> map = {};
            uint _3__times__num_triangles = 3 * num_triangles;
            uint default_value = _3__times__num_triangles + 1;
            {
                List<vec3> list = {};
                for_(i, _3__times__num_triangles) {
                    vec3 p = triangle_soup[i];
                    uint j = map_get(&map, p, default_value);
                    if (j == default_value) {
                        map_put(&map, p, num_vertices++);
                        list_push_back(&list, p);
                    }
                }
                {
                    uint size = list.length * sizeof(vec3);
                    vertex_positions = (vec3 *) arena->malloc(size);
                    memcpy(vertex_positions, list.array, size);
                }
                list_free_AND_zero(&list);
            }
            triangle_tuples = (uint3 *) arena->malloc(num_triangles * sizeof(uint3));
            for_(k, _3__times__num_triangles) triangle_tuples[k / 3][k % 3] = map_get(&map, triangle_soup[k]);
            map_free_and_zero(&map);
        }


        MeshesReadOnly result = build_meshes(arena, num_vertices, vertex_positions, num_triangles, triangle_tuples);
        result.M_3D_from_2D = M4_xyzo(V3(1,0,0), V3(0,0,-1), V3(0,1,0), V3(0,0,0)); // FORNOW; TODO: this should call get_M_3D_from_2D
        return result;
    }
}

