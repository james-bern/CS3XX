
    if (0) {
        // messagef(pallete.red, "TODO: Center snap should visualize the entity (same for all of them i think)");
        // messagef(pallete.red, "TODO: expand scripting to allow SHIFT+SPACE (just use what vimrc does)");
        // messagef(pallete.red, "TODO: measure should populate the active Mesh field; this would be really nice");
        // messagef(pallete.red, "TODO: popup->lcopy_num_additional_copies should default to 1 (ZERO_OUT is problem)");
        // messagef(pallete.red, "TODO: make negative extrude and revolves work");
        // messagef(pallete.red, "TODO: blue move/rotate/copy (certainly) still needs work");
        // // messagef(pallete.red, "TODO: move shouldn't snap to entities being moved");
        // // messagef(pallete.blue, "TODO: EXCLUDE_SELECTED_ENTITIES_FROM_SECOND_CLICK_SNAP flag");
        // messagef(pallete.red, "TODO: rotate about origin bumps the mouse unnecessarily (or like...wrong?)");
        // messagef(pallete.yellow, "TODO: CIRCLE type entity");
        // messagef(pallete.yellow, "TODO: - Select Connected");
        // messagef(pallete.yellow, "TODO: - TwoClickDivide");
        // messagef(pallete.green, "Offset broken clicking here *------* ");
        // messagef(pallete.green, "TODO (fun): Offset visualization and tweening");


    }
    // messagef(pallete.red, "?TODO: possible to get into a state where Escape no longer cancels Select, Connected (just popups)");


    // messagef(pallete.red, "TODO: rename pallete -> pallete (allow for pallete swaps later)");
    #ifdef SHIP
    // messagef(pallete.light_gray, "press ? for help");
    #endif
    /*
       messagef(pallete.red, "TODO: Intersection snap");
       messagef(pallete.red, "TODO: SHIP should disable all the commands without without without without without without without without buttons");
       messagef(pallete.red, "TODO: Save/Load need buttons");
       messagef(pallete.red, "TODO: Camera clip planes still jacked (including ortho)");
       messagef(pallete.red, "TODO: Camera hotkeys ;, ' need buttons");
       messagef(pallete.red, "TODO: Rezoom camera needs button");
       messagef(pallete.red, "TODO: Beatiful button presses");
       messagef(pallete.red, "TODO: config needs inches vs. mm");
       messagef(pallete.red, "TODO: config needs bool to hide gui");
       messagef(pallete.red, "TODO: Select/deselect snaps");
       messagef(pallete.red, "TODO: Push power fillet to beta");
       messagef(pallete.red, "TODO: Push power offset (shell) to beta");
       messagef(pallete.red, "TODO: Save/Load DXF broken for some arcs if you load\n      and save the dxf in LAYOUT in the middle.");
       */
// #define NATE
#if 0

#include "basics.cpp"
#include "arena.cpp"

int main() {
    Arena arena = arena_create();
    void *foo = arena_malloc(&arena, 123);
    void *bar = arena_malloc(&arena, 456);
    void *baz = arena_malloc(&arena, 10001);
    FORNOW_UNUSED(foo);
    FORNOW_UNUSED(bar);
    FORNOW_UNUSED(baz);


    ArenaList<uint> list = { &arena };
    for_(i, 10000) list_push_back(&list, i);

    ArenaMap<uint, uint> map = { &arena };
    map_put(&map, 1U, 5U);
    // printf("%d\n", map_get(1));

    arena_free(&arena);
}


#else

// BETA
// TODO: first and second clicks of FGHI should be hinted (even first!)
// TODO: Pressing enter should spawn a ghost of the pink or blue crosshairs
// TODO: the white box should fade in
// TODO: second click of fillets needs to be previewed in blue; some sort of cool bezier curve interpolation could be nice (or even just lerp between the central arcs?--polygon will be good practice (this could be fun for Nate)
// TODO: cool flash and then fade out with beep (don't actually need beep) for enter with crosshairs
// TODO: use UP and DOWN arrow keys to increase or decrease the field by one if it's a pure number
// TODO: preview fields if mouse not moving
// -- or actually, do both, but have them fade or something if the mouse is/isn't moving (MouseDrawColor, EnterDrawColor)
// -- or actually actually, time_since_current_popup_typed_in (and maybe also a boolean on mouse movement if they change their mind)
// TODO: fillet preview

// TODO: BETA
// - divide shows the point that was divided (animation)

// TODO (Jim): slow mo keybind

// TODO (Jim): cookbook_delete should throw a warning if you're trying to delete the same entity twice (run a O(n) pass on the sorted list)
// TODO (Jim): fillet
// TODO (Jim): power fillet
// TODO (Jim): revolve++ (with same cool animation for partial revolves
// TODO (Jim): dog ear
// TODO (Jim): power dog ear


// TODO (Jim): upgrade test bed
// TODO (Jim): switch everything from radians to turns

// XXXX: stippled lines
// TODO: reset everything in eso on begin

// XXXX: basic 3D grid with lines (have grids on by default)
// ////: camera reset needs to take into account divider position
// XXXX: switch entity_*(...) over to vec2(...) instead of void(..., real *, real *)
// XXXX: consider cookbook_lambdas -> Cookbook cookbook = cookbook(...)
// TODO: (re)write soup.cpp
// TODOFIRST: eso_size(...) (eso_begin doesn't take size)
// TODO: memcmp to see if should record
// TODO: timer to see if should snapshot

#if 0
#define Color0 Green
#define Color1 Red
#define Color2 Pink
#define Color3 Magenta
#define Color4 Purple
#define Color5 Blue
#define Color6 Gray
#define Color7 LightGray
#define Color8 Cyan
#define Color9 Orange
#elif 0
#define Color0 LightGray
#define Color1 Red
#define Color2 Orange
#define Color3 Yellow
#define Color4 Green
#define Color5 Blue
#define Color6 Purple
#define Color7 Brown
#define Color8 DarkGray
#define Color9 XXX
#endif

#if 0 // fornow
run_before_main {
startup_script = "^.^odemo.dxf\nsao";
};
#endif

// ISSUE: negative extrude crashes program`;
// ISSUE: undo graphics broken again but not sure why

#if 0 // talk
run_before_main {
    startup_script =
        ""
        "^."
        // "cz30\ncx10\n3.4\ns<m2d 10 0>R6\nsay[3\n"
        "^odemo.dxf\nysq1[5\nyn5\nsq3[30\n"
        // "bz10\t10\nysa[10\n"
        // "^Odemo.stl\n"
        ;
};
#endif











#if 0 // flip
run_before_main {
    startup_script =
        "^."
        // "^G"
        "^odemo.dxf\n"
        "sq1m"
        // "sq1y[5\n"
        // "<m3d 0 100 15 0 -1 0>"
        // "sc<m2d 30 15><m2d 30 -15>"
        ;
};
#endif

#if 0 // splash
run_before_main {
    // #define DEBUG_HISTORY_CHECKPOINT_EVERYTHING // TODO: why this broken?
    startup_script =
        "^.^G"
        "^osplash.dxf\n"
        "y"
        "sc"
        // "<m2d 16 16>"
        // "<m2d -16 16>"
        // "<m2d -16 -16>"
        // "<m2d 16 -16>"
        // "c"
        // "<m2d 100 0>"
        // "[50\n"
        // "<m3d 0 100 0 0 -1 0>"
        // "<m2d 17 0>"
        // "{47\n"
        // ";"
        ;
};
#endif

#if 0 // snap
run_before_main {
    startup_script =
        "^."
        "^odemo.dxf\n"
        "ysq1[5\n"
        ;
};
#endif

#if 0 // circle
run_before_main {
    startup_script =
        // "^..cz32\nlq<m2d 10 0>zs<m2d 0 0>\b<esc>le<m2d 10 0>q<m2d -10 0>"
        // "^.cz32\nlq<m2d 10 0>zs<m2d 0 0>\b<esc>"
        // "cz16\n"
        // "s<m2d> 0 0>\b"
        // "lq<m2d 8 0>q<m2d 16 0>"
        // "sc"
        // "^odemo.dxf\n"
        // "bz<m2d 10 10>"
        // "cz<m2d 10 10>"
        // "ysq1[5\n"
        // "^G"
        // "^.^Obuddha.stl\n"
        // "^.^Ocylinder.stl\n"
        // "^odemo.dxf\nsq1y[5\n<m3d 0 10 15 0 -1 0>"
        ;
};
#endif

#if 0 // bug.dxf load
run_before_main {
    startup_script = "^.^obug.dxf\n";
};
#endif

#if 0 // glorbo
run_before_main {
    startup_script = "^.^oglorbo.dxf\n"
        // "^Oglorbo.stl\n"
        ;
};
#endif

#if 0 // Box tweening
run_before_main {
    startup_script = "bz5";
};
#endif

#if 0 // UP DOWN polygon
run_before_main {
    startup_script = "pz";
};
#endif

#if 0 // hotkeys not working bug
run_before_main {
    startup_script = "cx20\n10\nsay]90\t30";
};
#endif

#if 0 // hotkeys not working bug
run_before_main {
    startup_script = "^odemo.dxf\n";
};
#endif

#if 0 // divide
run_before_main {
    startup_script = 
        "cz10\n"
        "s<m2d 0 -10>\b"
        "lzm<m2d 0 10>"
        // "cz10\n"
        // "s<m2d 0 -10>\b"
        // "lzm<m2d 0 10>"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "UUUUUUUUUU"
        ;
    #if 0
    "bz10\t10\n"
        "lm<m2d 5 0>m<m2d 5 10>"
        "lm<m2d 0 5>m<m2d 10 5>"
        "sa\b"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        "uuuuuuuuuu"
        "UUUUUUUUUU"
        #endif
};
#endif

#if 0 // multi-popup
run_before_main {
    startup_script = 
        // "[lz"
        // "[10lz"
        // "f123[456"
        "lz12\t34x56\t78[98\t76"
        // "lzx["
        ;
};
#endif

#if 0 // revolve
run_before_main {
    startup_script = \
                     "y"
                     "cz10\n"
                     "sa[10\n"
                     // "Zx-5\n"
                     "y"
                     "samzx5\n"
                     // "Ax5\nx5\t5\n"
                     "sa]\n"
                     ; };
#endif

#if 0 // fillet
run_before_main {
    startup_script = \
                     "Bz40\t20\nf5<m2d 15 10><m2d 20 5>";
};
#endif

#if 0 // dogear
run_before_main {
    startup_script = \
                     "Bz40\t20\ng5<m2d 15 10><m2d 20 5>";
};
#endif

#endif
l
/*
enum class ColorCode {
    Traverse,
    Quality1,
    Quality2,
    Quality3,
    Quality4,
    Quality5,
    Etch,
    Unknown,
    _WaterOnly,
    LeadIO,
    QualitySlit1 = 21,
    QualitySlit2,
    QualitySlit3,
    QualitySlit4,
    QualitySlit5,
    Selection = 255,
    Emphasis = 254,
};
*/
// TODO: re-read the paper to see what it says about curvy polygons (maybe they just didn't consider this case)
// -- TODO: can pass corresponding triangle normals and see if that helps (the WorkMesh already has these computed; will have to do some work to hook them up, but not too much)
// TODO: wait, does this even make sense? i don't think so. our triangles aren't actually exploded. we're just using ebos to go over the edges
// there will be some weird geometry shader solution, but it's non-obvious fornow (passing around the third vertex

// // !!!
// NO this bad bad (see problem.stl) -- consider throwing away patch version and just using all edges but turning off some of them (modulating thickness) based on whether they're a patch edge. could pass into for each one to say whether it's a hard half edge or not
// this is going to be an annoying change because i don't think you can pass whether it is a hard edge as a vertex attribute with the current pipeline
// right now we just have all our vertices and we pass a triangle explode EBO as lines
// we probably need to the data actually exploded into triangle soup labeled with "opposing edge is hard" or something like that, which isn't hard, just mildly annoying; or we could just go really inefficient and throw labeled line soup at the problem, which would require minimal modification to the current edge shader pipeline. this is probably the easiest thing to do; it's also a very straight forward idea. just upload a bunch of lines to the GPU, labeled with whether they're hard. no need for a patch ID buffer. and we can definitely single pass shade it. it's bad in terms of space, but that's also probably fine. so we need a massive bool *all_half_edges_soup_per_vertex_is_hard_half_edge; and then we'll also need all_half_edges_soup_vertex_positions; using an EBO with the same data used for smooth-shading the mesh was a cute idea, but I think we need to let it go
// or actually, i think you could just use both textures. if it matches the patch id and it matches the face id, then you're golden

// I think the obvious approach is backface culling (we need to look up when in the pipeline that happens)
// - if it's early, then we rewrite this crap to make the geometry shader emit 3 lines per triangle (just a for loop)--but skip this if the triangle is facing away from us.
// we might be able to condense the hard edges and all edges into a single pass if we do this, but let's not worry about that fornow. priority now is any hard edge solution that doesn't exhibit the problem we have now


// TODO: this doesn't work if you resize the window
// TODO: get rid of STENCIL
// NOTE: VAO and VBO are about mesh data (don't connect them to shaders)
