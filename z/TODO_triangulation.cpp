
        #if 0
        do_once { // selection triangulation 3d 3D
            ManifoldTriangulation *triangulation = manifold_triangulate(manifold_alloc_triangulation(), polygons, 0.0); // TODO: what is eps
            int selection_num_triangles = (int) manifold_triangulation_num_tri(triangulation);
            uint3 *selection_triangle_tuples = (uint3 *) manifold_triangulation_tri_verts(malloc(selection_num_triangles * sizeof(uint3)), triangulation);
            manifold_delete_triangulation(triangulation);
            fornow_global_selection_num_triangles = selection_num_triangles;
            fornow_global_selection_triangle_tuples = selection_triangle_tuples;

            // 1) scavenging new vertex positions from CrossSection
            // 2) converting 64bit->32bit
            {
                List<vec2> tmp = {};

                size_t tmp_num_polygons = manifold_polygons_length(polygons);
                for_(i, tmp_num_polygons) {
                    size_t tmp_num_vertices = manifold_polygons_simple_length(polygons, i);
                    for_(j, tmp_num_vertices) {
                        ManifoldVec2 _p = manifold_polygons_get_point(polygons, i, j);
                        list_push_back(&tmp, { real(_p.x), real(_p.y) });
                    }
                }

                fornow_global_selection_vertex_positions = tmp.array;
            }
        };
        #endif
