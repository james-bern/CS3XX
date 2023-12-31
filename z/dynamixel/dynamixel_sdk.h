/*******************************************************************************
 * Copyright 2017 ROBOTIS CO., LTD.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

/* Author: Ryu Woon Jung (Leon) */

#ifndef DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_DYNAMIXELSDK_C_H_
#define DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_DYNAMIXELSDK_C_H_

/*
extern "C" {
    #include "robotis_def.h"
    #include "group_bulk_read.h"
    #include "group_bulk_write.h"
    #include "group_sync_read.h"
    #include "group_sync_write.h"
    #include "packet_handler.h"
    #include "port_handler.h"
}
*/

extern "C" {
    #include "robotis_def.h"
    // #include "group_bulk_read.c"
    // #include "group_bulk_write.c"
    // #include "group_sync_read.c"
    // #include "group_sync_write.c"
    #include "protocol2_packet_handler.c"
    #include "protocol1_packet_handler.c"
    #include "packet_handler.c"
    #include "port_handler.c"
    // #include "port_handler_windows.c"
    #include "port_handler_windows_shim.c"

}


#endif /* DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_DYNAMIXELSDK_C_H_ */
