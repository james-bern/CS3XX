#ifndef DXL_CPP
#define DXL_CPP

#pragma warning(push, 0)
#include "codebase/ext/dynamixel/dynamixel_sdk.h"
#pragma warning(pop)

#define PROTOCOL_VERSION 2
#define BAUDRATE 1000000

struct {
    int id;
    int _port_number;
    bool initialized;
    bool _NO_READ = true; // FORNOW
} dxl;

bool dxl_init(char *deviceName) {
    dxl._port_number = portHandler(deviceName);

    packetHandler();
    if (openPort(dxl._port_number)) {
        if (setBaudRate(dxl._port_number, BAUDRATE)) { // TODO remove this check once you're confident with the shim
            dxl.initialized = true;
            return true;
        }

        printf("[dxl] failed to set baud\n");
        closePort(dxl._port_number);
        return false;
    }

    printf("[dxl] failed to open port\n");
    return false;
}

void _ASSERT_DXL_NO_ERROR() {
    if (dxl._NO_READ) { return; }
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
    if (dxl._NO_READ) { return 0; }

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

    {
        dxl_init("COM4");
        dxl_set_torque_enable(1);
    }
    uint8_t dxl_ID = 31;

    int dxl_goal_position = dxl_get_present_position(dxl_ID);

    Plot plot; {
        plot_init(&plot, 256);
        plot_add_trace(&plot, DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE, monokai.yellow, 24.0);
        plot_add_trace(&plot, DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE, (!dxl._NO_READ) ? monokai.blue : monokai.gray, 12.0);
    }

    Camera2D camera = { 2.0 };
    while (cow_begin_frame()) {
        mat4 PV = camera_get_PV(&camera);
        camera_move(&camera);

        { // dxl
            real tmp = dxl_goal_position;
            gui_slider("dxl_goal_position", &dxl_goal_position, DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE);
            if (tmp != dxl_goal_position) {
                dxl_set_goal_position(dxl_goal_position);
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
    real motor_signs[MAX_NUM_MOTORS];
    real pulley_radius_in_meters;
} easy_dxl;


void easy_dxl_init_FORNOW_ASSUMES_9_MOTORS(char *filename) {
    FILE *fp = fopen(filename, "r");
    ASSERT(fp);

    char buffer[2048];

    char comName[64];
    fgets(buffer, _COUNT_OF(buffer), fp); // COMX
    sscanf(buffer, "%s", comName);
    dxl_init(comName);

    easy_dxl.num_motors = 9;

    fgets(buffer, _COUNT_OF(buffer), fp); // IDS
    sscanf(buffer, "%d %d %d   %d %d %d   %d %d %d",
            &easy_dxl.motor_ids[0],
            &easy_dxl.motor_ids[1],
            &easy_dxl.motor_ids[2],
            &easy_dxl.motor_ids[3],
            &easy_dxl.motor_ids[4],
            &easy_dxl.motor_ids[5],
            &easy_dxl.motor_ids[6],
            &easy_dxl.motor_ids[7],
            &easy_dxl.motor_ids[8]);

    fgets(buffer, _COUNT_OF(buffer), fp); // signs
    sscanf(buffer, "%lf %lf %lf   %lf %lf %lf   %lf %lf %lf",
            &easy_dxl.motor_signs[0],
            &easy_dxl.motor_signs[1],
            &easy_dxl.motor_signs[2],
            &easy_dxl.motor_signs[3],
            &easy_dxl.motor_signs[4],
            &easy_dxl.motor_signs[5],
            &easy_dxl.motor_signs[6],
            &easy_dxl.motor_signs[7],
            &easy_dxl.motor_signs[8]);

    fgets(buffer, _COUNT_OF(buffer), fp); // pulley radius
    sscanf(buffer, "%lf", &easy_dxl.pulley_radius_in_meters);

    fclose(fp);
}

void easy_dxl_write(real *u) {

}


void hello_easy_dxl() {
    easy_dxl_init_FORNOW_ASSUMES_9_MOTORS("config.txt");
    real u[9] = {};
    while (cow_begin_frame()) {
        easy_dxl_write(u);
    }
    dxl_exit();

}

#endif
