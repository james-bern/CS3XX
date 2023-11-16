// TODO: sync write?

#ifndef DXL_CPP
#define DXL_CPP

bool _DXL_NO_READ; // FORNOW: Hackorama in the shim to not worry about receipts, etc.

#pragma warning(push, 0)
#include "codebase/ext/dynamixel/dynamixel_sdk.h"
#pragma warning(pop)

#define PROTOCOL_VERSION 2
#define BAUDRATE 1000000

struct {
    int id;
    int _port_number;
    bool initialized;
} dxl;

void dxl_init(char *deviceName) {
    dxl._port_number = portHandler(deviceName);

    packetHandler();
    openPort(dxl._port_number);
    if (!serial->connected) {
        printf("[dxl_init] Failed to open port.\n");
        dxl.initialized = false;
        return;
    }
    dxl.initialized = true;
}

void _ASSERT_DXL_NO_ERROR() {
    if (_DXL_NO_READ) { return; }
    {
        int dxl_comm_result = COMM_TX_FAIL;
        if ((dxl_comm_result = getLastTxRxResult(dxl._port_number, PROTOCOL_VERSION)) != COMM_SUCCESS) {
            printf("%s\n", getTxRxResult(PROTOCOL_VERSION, dxl_comm_result));
            ASSERT(0);
        }
    }
    {
        uint8_t dxl_error = 0;
        if ((dxl_error = getLastRxPacketError(dxl._port_number, PROTOCOL_VERSION)) != 0) {
            printf("%s\n", getRxPacketError(PROTOCOL_VERSION, dxl_error));
            ASSERT(0);
        }
    }
}

#define DXL_ADDR_TORQUE_ENABLE     64
#define DXL_ADDR_LED               65
#define DXL_ADDR_GOAL_POSITION     116
#define DXL_ADDR_PRESENT_POSITION  132

#define DXL_MINIMUM_POSITION_VALUE 0
#define DXL_MAXIMUM_POSITION_VALUE 4095

#define DXL_BROADCAST_ID 254

void _DXL_HANDLE_STATUS_PACKET(int dxl_ID) {
    if (dxl_ID != DXL_BROADCAST_ID) {
        _ASSERT_DXL_NO_ERROR();
    }
}

void dxl_set_led(uint8_t byte, uint8_t dxl_ID = DXL_BROADCAST_ID) {
    if (!dxl.initialized) { return; }

    write1ByteTxRx(dxl._port_number, PROTOCOL_VERSION, dxl_ID, DXL_ADDR_LED, byte);
    _DXL_HANDLE_STATUS_PACKET(dxl_ID);
}

void dxl_set_torque_enable(uint8_t byte, uint8_t dxl_ID = DXL_BROADCAST_ID) {
    if (!dxl.initialized) { return; }

    write1ByteTxRx(dxl._port_number, PROTOCOL_VERSION, dxl_ID, DXL_ADDR_TORQUE_ENABLE, byte);
    _DXL_HANDLE_STATUS_PACKET(dxl_ID);
}

void dxl_set_goal_position(int byte4, uint8_t dxl_ID = DXL_BROADCAST_ID) {
    if (!dxl.initialized) { return; }

    write4ByteTxRx(dxl._port_number, PROTOCOL_VERSION, dxl_ID, DXL_ADDR_GOAL_POSITION, byte4);
    _DXL_HANDLE_STATUS_PACKET(dxl_ID);
}

int dxl_get_present_position(uint8_t dxl_ID) {
    if (!dxl.initialized) { return 0; }
    if (_DXL_NO_READ) { return 0; }

    int ret = read4ByteTxRx(dxl._port_number, PROTOCOL_VERSION, dxl_ID, DXL_ADDR_PRESENT_POSITION);
    _DXL_HANDLE_STATUS_PACKET(dxl_ID);

    return ret;
}

void dxl_exit() {
    if (!dxl.initialized) { return; }

    dxl_set_led(0);
    dxl_set_torque_enable(0);
    closePort(dxl._port_number);
}

void hello_dxl() {
    glfwSwapInterval(1); // FORNOW

    _DXL_NO_READ = false;
    {
        dxl_init("COM4");
        dxl_set_torque_enable(1);
    }
    uint8_t dxl_ID = 1;

    int dxl_goal_position = dxl_get_present_position(dxl_ID);

    Plot plot; {
        plot_init(&plot, 256);
        plot_add_trace(&plot, DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE, monokai.yellow, 24.0);
        plot_add_trace(&plot, DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE, (!_DXL_NO_READ) ? monokai.blue : monokai.gray, 12.0);
    }

    Camera2D camera = { 2.0 };
    while (cow_begin_frame()) {
        mat4 PV = camera_get_PV(&camera);
        camera_move(&camera);

        { // dxl
            real tmp = dxl_goal_position;
            gui_slider("dxl_goal_position", &dxl_goal_position, DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE);
            if (tmp != dxl_goal_position) {
                dxl_set_goal_position(dxl_goal_position, dxl_ID);
            }
        }

        { // plot
            plot_data_point(&plot, 0, dxl_goal_position);
            plot_data_point(&plot, 1, dxl_get_present_position(dxl_ID));
            plot_draw(&plot, PV);
        }
    }
    dxl_exit();
}

// --

// TODO click conversion
// TODO extended position mode
// TODO bring reading back

#define MAX_NUM_MOTORS 64
struct {
    int num_motors;
    int motor_ids[MAX_NUM_MOTORS];
    int motor_signs[MAX_NUM_MOTORS];
    real pulley_radius_in_meters;
    char commPortString[64];
    int CLICKS_FROM_METERS(real contraction_in_meters) {
        return int((contraction_in_meters / pulley_radius_in_meters) * (4096 / TAU));
    }
    real _top_3_motors_max_meters =  50 / 1000.0;
    real _other_motors_max_meters = 100 / 1000.0;
    int top_3_motors_max_clicks() { return CLICKS_FROM_METERS(_top_3_motors_max_meters); }
    int other_motors_max_clicks() { return CLICKS_FROM_METERS(_other_motors_max_meters); }
} kaa_dxl;


void kaa_dxl_init_FORNOW_ASSUMES_9_MOTORS(char *filename) {
    _DXL_NO_READ = true;
    { // parse config
        FILE *fp = fopen(filename, "r");
        ASSERT(fp);

        char buffer[2048];

        fgets(buffer, _COUNT_OF(buffer), fp); // COMX
        sscanf(buffer, "%s", kaa_dxl.commPortString);

        kaa_dxl.num_motors = 9;

        fgets(buffer, _COUNT_OF(buffer), fp); // IDS
        sscanf(buffer, "%d %d %d   %d %d %d   %d %d %d",
                &kaa_dxl.motor_ids[0],
                &kaa_dxl.motor_ids[1],
                &kaa_dxl.motor_ids[2],
                &kaa_dxl.motor_ids[3],
                &kaa_dxl.motor_ids[4],
                &kaa_dxl.motor_ids[5],
                &kaa_dxl.motor_ids[6],
                &kaa_dxl.motor_ids[7],
                &kaa_dxl.motor_ids[8]);

        fgets(buffer, _COUNT_OF(buffer), fp); // signs
        sscanf(buffer, "%d %d %d   %d %d %d   %d %d %d",
                &kaa_dxl.motor_signs[0],
                &kaa_dxl.motor_signs[1],
                &kaa_dxl.motor_signs[2],
                &kaa_dxl.motor_signs[3],
                &kaa_dxl.motor_signs[4],
                &kaa_dxl.motor_signs[5],
                &kaa_dxl.motor_signs[6],
                &kaa_dxl.motor_signs[7],
                &kaa_dxl.motor_signs[8]);

        fgets(buffer, _COUNT_OF(buffer), fp); // pulley radius
        sscanf(buffer, "%lf", &kaa_dxl.pulley_radius_in_meters);

        fclose(fp);
    }
    dxl_init(kaa_dxl.commPortString);
    dxl_set_torque_enable(1);
    dxl_set_led(1);
}

int _kaa_dxl_clicks_from_u(real *u, int j) {
    int tmp = kaa_dxl.CLICKS_FROM_METERS(kaa_dxl.motor_signs[j] * u[j]);
    int bound = (j < 3) ? kaa_dxl.top_3_motors_max_clicks() : kaa_dxl.other_motors_max_clicks();
    return MAG_CLAMP(tmp, bound);
}

void kaa_dxl_write(real *u) {
    static int HORRIFYING_HACK__FRAME;
    for_(j, kaa_dxl.num_motors) {
        if ((HORRIFYING_HACK__FRAME % 6 == 0) && (j > 3)) continue;
        if ((HORRIFYING_HACK__FRAME % 6 == 2) && (j < 3 || j >= 6)) continue;
        if ((HORRIFYING_HACK__FRAME % 6 == 4) && (j < 6)) continue;
        if (kaa_dxl.motor_ids[j] == -1) continue;
        dxl_set_goal_position(_kaa_dxl_clicks_from_u(u, j), (uint8_t) kaa_dxl.motor_ids[j]);
    }
    ++HORRIFYING_HACK__FRAME;
}


// TODO: software contraction limits


void motor_tester() {
    glfwSwapInterval(1); // FORNOW
    COW0._cow_display_fps = true; // FORNOW


    kaa_dxl_init_FORNOW_ASSUMES_9_MOTORS("./motor_config.txt");
    real u[9] = {};

    Plot plot; {
        plot_init(&plot, 256);
        for_(j, kaa_dxl.num_motors) plot_add_trace(&plot, 0.0, 2 * kaa_dxl._other_motors_max_meters, color_kelly(j));
    }

    real time = 0.0;
    Camera2D camera = { 2.0 };
    while (cow_begin_frame()) {
        camera_move(&camera);
        mat4 PV = camera_get_PV(&camera);
        time += 0.0167;

        gui_printf("README");
        gui_printf("------");
        gui_printf("Pulley Diameter %d mm", int(2000 * kaa_dxl.pulley_radius_in_meters));
        gui_printf("top 3 motors clamped to %d clicks", kaa_dxl.top_3_motors_max_clicks());
        gui_printf("other motors clamped to %d clicks", kaa_dxl.other_motors_max_clicks());
        gui_printf("");
        if (1) { // manual sliders
            real SLIDER_MAX_UNCLAMPED_CONTRACTION = 1.4 * kaa_dxl._other_motors_max_meters;
            for_(j, kaa_dxl.num_motors) {
                if (j == 0) {
                    gui_printf("top 3 motors");
                    gui_printf("------------");
                }
                if (j == 3) {
                    gui_printf("");
                    gui_printf("other motors");
                    gui_printf("------------");
                }
                char buffer[512];
                // sprintf(buffer, "(ID = %03d, sign = %02d) u[%d] =", kaa_dxl.motor_ids[j], kaa_dxl.motor_signs[j], j);
                sprintf(buffer, "%d: %d\
                        \
                        \
                        \
                        ", kaa_dxl.motor_ids[j], _kaa_dxl_clicks_from_u(u, j));
                gui_slider(buffer, &u[j], -SLIDER_MAX_UNCLAMPED_CONTRACTION, SLIDER_MAX_UNCLAMPED_CONTRACTION);

            }
        }


        kaa_dxl_write(u);

        { // plot
            for_(j, kaa_dxl.num_motors) plot_data_point(&plot, j, u[j]);
            plot_draw(&plot, PV);
        }
    }
    dxl_exit();
}

// FORNOW: assumes dynamixels have been zeroed

#endif
