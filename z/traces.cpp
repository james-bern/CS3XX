#include "serial.cpp"
// workspace [0, 1]
#define PHYSICAL_POSITION_0 750
#define PHYSICAL_POSITION_1 1500
#define PHYSICAL_ANGLE_0    3775
#define PHYSICAL_ANGLE_1    2960
#define PHYSICAL_DISTANCE_0 0
#define PHYSICAL_DISTANCE_1 250
#define VIRTUAL_POSITION_0  RAD(-45.0)
#define VIRTUAL_POSITION_1  RAD( 45.0)
#define VIRTUAL_ANGLE_0     RAD(-45.0)
#define VIRTUAL_ANGLE_1     RAD( 45.0)
#define BUFFER_LENGTH 256
void traces() {
    config.tweaks_size_in_pixels_soup_draw_defaults_to_if_you_pass_0_for_size_in_pixels = 5.0;

    serial_init(3);
    serial_write("O");

    real dxl_goal_position = 0.5; // fornow

    int PLOT_DXL_POSITION_GOAL,
        PLOT_DXL_POSITION_PRESENT,
        PLOT_MAGNET_ANGLE_UNFILTERED,
        PLOT_MAGNET_ANGLE_FILTERED,
        PLOT_SENSOR_DISTANCE_UNFILTERED,
        PLOT_SENSOR_DISTANCE_FILTERED;
    Plot plot; {
        plot_init(&plot, 128);
        int k = 0;
        PLOT_DXL_POSITION_GOAL          = k++; plot_add_trace(&plot, 0.0, 1.0, .5 * monokai.red);
        PLOT_DXL_POSITION_PRESENT       = k++; plot_add_trace(&plot, 0.0, 1.0, monokai.red);
        PLOT_MAGNET_ANGLE_UNFILTERED    = k++; plot_add_trace(&plot, 0.0, 1.0, monokai.gray);
        PLOT_MAGNET_ANGLE_FILTERED      = k++; plot_add_trace(&plot, 0.0, 1.0, monokai.orange);
        PLOT_SENSOR_DISTANCE_UNFILTERED = k++; plot_add_trace(&plot, 0.0, 1.0, monokai.gray);
        PLOT_SENSOR_DISTANCE_FILTERED   = k++; plot_add_trace(&plot, 0.0, 1.0, monokai.blue);
    }

    Camera2D camera = { 1.2, 0.5, 0.5 };
    while (cow_begin_frame()) {
        camera_move(&camera);
        mat4 PV = camera_get_PV(&camera);
        static bool paused; gui_checkbox("paused", &paused, 'p');
        static bool show_unfiltered = true; gui_checkbox("show_unfiltered", &show_unfiltered, 'o');

        { // dxl
            { // goal

                // set dxl_goal_position
                bool position_dirty = false;
                {
                    real tmp = dxl_goal_position;
                    gui_slider("dxl_goal_position", &dxl_goal_position, 0.0, 1.0);
                    static bool sinusoid;
                    gui_checkbox("sinusoid", &sinusoid, 's');
                    if (sinusoid) {
                        static real time;
                        dxl_goal_position = 0.5 + 0.5 * sin(3.0 * time);
                        time += 0.0167;
                    }
                    position_dirty = (tmp != dxl_goal_position);
                }

                // plot
                if (!paused) {
                    plot_data_point(&plot, PLOT_DXL_POSITION_GOAL, dxl_goal_position);
                }

                // NOTE: could do this properly as bytes (would be a shorter message)
                //       though it's already pretty short *shrug*
                // A good project for a rainy day
                if (position_dirty) {
                    static char buffer[BUFFER_LENGTH];
                    sprintf(buffer, "<%d>", int(LINEAR_REMAP(dxl_goal_position, 0.0, 1.0, PHYSICAL_POSITION_0, PHYSICAL_POSITION_1)));
                    serial_write(buffer);
                }

            }

        }

        {
            // i2c sensors
            static char buffer[BUFFER_LENGTH];
            static int angle_valid, distance_valid;
            static real angle, distance, present_position;
            static int inspect_0;

            { // read
                if (serial_read(buffer, BUFFER_LENGTH)) {
                    int _angle, _distance, _present_position;

                    // printf((char *) buffer);
                    sscanf(buffer, "%d %d %d %d %d %d", &angle_valid, &_angle, &distance_valid, &_distance, &_present_position, &inspect_0);
                    angle = LINEAR_REMAP(_angle, PHYSICAL_ANGLE_0, PHYSICAL_ANGLE_1, 0.0, 1.0);
                    distance = LINEAR_REMAP(_distance, PHYSICAL_DISTANCE_0, PHYSICAL_DISTANCE_1, 0.0, 1.0);
                    present_position = LINEAR_REMAP(_present_position, PHYSICAL_POSITION_0, PHYSICAL_POSITION_1, 0.0, 1.0);
                }
            }

            gui_readout("inspect_0", &inspect_0);

            if (!paused) { // plot
                plot_data_point(&plot, PLOT_MAGNET_ANGLE_UNFILTERED, angle_valid ? angle : INFINITY);
                { // median filter
                    real y = MID(plot_get(&plot, PLOT_MAGNET_ANGLE_UNFILTERED, 0), plot_get(&plot, PLOT_MAGNET_ANGLE_UNFILTERED, 1), plot_get(&plot, PLOT_MAGNET_ANGLE_UNFILTERED, 2));

                    // // moving average
                    // real y_tmp = (y + plot_get(&plot, 1, 0) + plot_get(&plot, 1, 1)) / 3;
                    // if (y_tmp < HUGE) y = y_tmp;

                    plot_data_point(&plot, PLOT_MAGNET_ANGLE_FILTERED, y);
                }

                plot_data_point(&plot, PLOT_SENSOR_DISTANCE_UNFILTERED, distance_valid ? distance : INFINITY);
                { // hold
                    static real y;
                    real y_tmp = plot_get(&plot, PLOT_SENSOR_DISTANCE_UNFILTERED, 0);
                    if (y_tmp != INFINITY) {
                        y = y_tmp;
                    }
                    plot_data_point(&plot, PLOT_SENSOR_DISTANCE_FILTERED, y);
                }

                plot_data_point(&plot, PLOT_DXL_POSITION_PRESENT, present_position);
            }
        }

        { // plot
            plot.trace_colors[PLOT_MAGNET_ANGLE_UNFILTERED] = plot.trace_colors[PLOT_SENSOR_DISTANCE_UNFILTERED] = (show_unfiltered) ? monokai.gray : monokai.black;
            plot_draw(&plot, PV);
        }
    }
    serial_write("X");
    serial_exit();
}
