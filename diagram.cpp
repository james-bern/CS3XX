// TODO (last before starting over) -- anticipate object creation in dark blue (ghost object)
// TODOLATER field value should shrink before it changes
// about to change should be orange (just on path should be blue)

// TODO: emphasis of path when writing (idea of information following the flow of pointers / references down the chain)
//       (field names need to pulse)
// TODO: different color for "data that's about to change" and on_path
// (TODO (before): the asterisk / arrow should be blue)
// DUMMY or PATH would solve this

// TODO: time_since_in_path should anticipate in blue (FORNOW: not-anticipate in yellow)

// TODO: target_size size
// TODO: target_color color

// TODO: STACK versus HEAP OBJECT
// TODO: delete object
// TODO: line dividing stack and heap
// TODO: preview (anticipate -- see animation book) changes
// TODO: consider highlighting (and anticipating?) the "path" through the valriables to the data of interest
#include "playground.cpp"
#include "easy_mode.cpp"
char *script = R""(
// CODE // let's learn about array lists in C/C++!
// CODE
CODE int i = 5;
OBJ i xxi
SET ixxi 5
CODE int j;
OBJ j xxj
SET jxxj ?
CODE j = i;
SET jxxj 5
CODE i = 6;
SET ixxi 6
//
//
//
//
CODE ArrayList list;
OBJ list .length .capacity .array
// FILL ? list
SET list.length ?
SET list.capacity ?
SET list.array ?
CODE list = {};
SET list.length 0
SET list.capacity 0
SET list.array NULL
CODE // note: we could have used "ArrayList list = {};"
.... //       instead of "ArrayList list;" and "list = {};"
CODE
CODE list.capacity = 4;
SET list.capacity 4
CODE list.array = (double *) malloc(list.capacity, sizeof(double));
OBJ xxA [0] [1] [2] [3]
SET xxA[0] ?
SET xxA[1] ?
SET xxA[2] ?
SET xxA[3] ?
SET list.array *
CODE memset(list.array, 0, list.capacity * sizeof(double));
// NOTE: dummy set
SET list.array *
// FILL xxA 0.0
SET xxA[0] 0.0
SET xxA[1] 0.0
SET xxA[2] 0.0
SET xxA[3] 0.0
CODE // note: we could have picked any initial capacity instead of 4
CODE // note: we could have called calloc(...)
.... //       instead of malloc(...) and memset(..., 0, ...) 
CODE
CODE // // as long as there are empty slots in the array, append(...) takes two steps
CODE // 1) write the element to the array
.... list.array[list.length] = 6.0;
// NOTE: dummy set
SET list.array *
SET xxA[0] 6.0
CODE // 2) increment the list's length
.... list.length++;
SET list.length 1
CODE
CODE // note: we can do both steps on one line
.... list.array[list.length++] = 7.0;
SET xxA[1] 7.0
SET list.length 2
CODE
CODE list.array[list.length++] = 8.0;
SET xxA[2] 8.0
SET list.length 3
CODE list.array[list.length++] = 9.0;
SET xxA[3] 9.0
SET list.length 4
CODE
CODE // // we're out of room now!
.... // // the next append will be trickier!
CODE list.capacity *= 2;
SET list.capacity 8
CODE double *xxB = (double *) calloc(list.capacity, sizeof(double));
OBJ xxB [0] [1] [2] [3] [4] [5] [6] [7]
SET xxB[0] 0.0
SET xxB[1] 0.0
SET xxB[2] 0.0
SET xxB[3] 0.0
SET xxB[4] 0.0
SET xxB[5] 0.0
SET xxB[6] 0.0
SET xxB[7] 0.0
CODE memcpy(xxB, list.array, list.capacity / 2 * sizeof(double));
SET xxB[0] 6.0
SET xxB[1] 7.0
SET xxB[2] 8.0
SET xxB[3] 9.0
CODE
CODE
CODE
CODE // TODO: allocate tmp, etc.
CODE
CODE
CODE
CODE
CODE
)"";









// // TODO: growing array
// CODE "capacity *= 2;"
// CODE "tmp = calloc(capacity, sizeof(double)))"";

uint substep = 0;
vec3 bottom_line_color;

template <typename T> void JUICEIT_EASYTWEEN(T *a, T b) {
    real f = 0.1f;
    *a += f * (b - *a);
}

#define MESSAGE_MAX_LENGTH 256
#define MESSAGE_MAX_NUM_MESSAGES 64
#define MESSAGE_MAX_TIME 2.0f

struct Message {
    _STRING_CALLOC(string, MESSAGE_MAX_LENGTH);
    real time_remaining;
    real y;
    vec3 base_color;
};
Message messages[MESSAGE_MAX_NUM_MESSAGES];
uint _message_index;
void messagef(vec3 color, char *format, ...) {
    va_list arg;
    va_start(arg, format);
    Message *message = &messages[_message_index];
    message->string.length = vsnprintf(message->string.data, MESSAGE_MAX_LENGTH, format, arg);
    va_end(arg);

    message->base_color = color;
    message->time_remaining = MESSAGE_MAX_TIME;
    _message_index = (_message_index + 1) % MESSAGE_MAX_NUM_MESSAGES;
    message->y = 0.0f;

    // printf("%s\n", message->buffer); // FORNOW print to terminal as well
}
void _messages_update() {
    for_(i, MESSAGE_MAX_NUM_MESSAGES) {
        Message *message = &messages[i];
        if (message->time_remaining > 0) {
            message->time_remaining -= 0.0167f;;
        } else {
            message->time_remaining = 0.0f;
        }
    }
}
void _messages_draw() {
    real font_height_Pixel = 24.0f;
    uint i_0 = (_message_index == 0) ? (MESSAGE_MAX_NUM_MESSAGES - 1) : _message_index - 1;

    uint num_drawn = 0;
    auto draw_lambda = [&](uint message_index) {
        Message *message = &messages[message_index];

        real FADE_IN_TIME = 0.33f;
        // real FADE_OUT_TIME = 0.66f;

        real alpha; { // ramp on ramp off
            alpha = 0
                + CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME, MESSAGE_MAX_TIME - FADE_IN_TIME, 0.0f, 1.0f)
                ;// - CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, 0.0f, 0.3f);
        }

        vec3 color = message->base_color;
        if (num_drawn == 0) {
            vec3 target_bottom_line_color; {
                if (substep == 0) {
                    target_bottom_line_color = message->base_color;
                } else if (substep == 1) {
                    target_bottom_line_color = 0.3f * monokai.blue;
                    bottom_line_color = target_bottom_line_color; // FORNOW
                } else { ASSERT(substep == 2);
                    target_bottom_line_color = monokai.yellow;
                }
            }
            JUICEIT_EASYTWEEN(&bottom_line_color, target_bottom_line_color);
            color = bottom_line_color;
        }
        //CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME + FADE_IN_TIME, MESSAGE_MAX_TIME - 5.0f * FADE_IN_TIME, basic.yellow, message->base_color);
        // color = LERP(CLAMP(INVERSE_LERP(message->time_remaining, MESSAGE_MAX_TIME - FADE_OUT_TIME, 0.0f), 0.0f, 0.8f), color, V3(0.6f));

        real x = 300;
        real y_target = ++num_drawn * font_height_Pixel;
        // if (substep == 1) y_target -= font_height_Pixel / 2;
        // if (substep == 0) y_target += font_height_Pixel / 16;
        // if (message->time_remaining < FADE_OUT_TIME) y_target += CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, 0.0f, 12.0f);

        JUICEIT_EASYTWEEN(&message->y, y_target);
        // if (message->time_remaining > 0) {
        // font_height_Pixel = CLAMPED_LINEAR_REMAP(message->time_remaining, 0.0f, 0.7f, 22.0f, 20.0f);
        text_draw(OpenGL_from_Pixel, message->string, V2(x, 256.0f - message->y), V4(color, alpha), font_height_Pixel);
        // }
    };

    { // this is pretty gross
        uint i = i_0;
        while (true) {
            draw_lambda(i);

            if (i > 0) --i;
            else if (i == 0) i = MESSAGE_MAX_NUM_MESSAGES - 1;

            if (i == i_0) break;
        }
    }
}
void messages_update_and_draw() {
    _messages_update();
    _messages_draw();
    eso_begin(M4_Identity(), SOUP_QUADS, true);
    eso_color(basic.black, 0.0f);
    eso_vertex(-1.0f, 0.0f);
    eso_vertex( 1.0f, 0.0f);
    eso_color(basic.black, 1.0f);
    eso_vertex( 1.0f, 1.0f);
    eso_vertex(-1.0f, 1.0f);
    eso_end();
}

struct Field {
    _STRING_CALLOC(name, 64);
    _STRING_CALLOC(value, 64);

    real time_since_in_path;
    real time_since_changed;
};

#define MAX_NUM_FIELDS 8
struct Object {
    bool is_live;
    _STRING_CALLOC(name, 64);
    uint num_fields;
    Field fields[MAX_NUM_FIELDS];

    real time_since_in_path;
    real time_since_created;
};

#define MAX_NUM_OBJECTS 8
uint num_objects;
Object objects[MAX_NUM_OBJECTS];


#define _for_each_object_ for (Object *object = objects; object < objects + MAX_NUM_OBJECTS; ++object)
#define for_each_live_object_ _for_each_object_ if (object->is_live)
#define for_each_field_(object) for (Field *field = object->fields; field < object->fields + object->num_fields; ++field)

// TODO: parse line into words (for CODE we can wrap the argument in "..." fornow)
// This will make everything way easier

// TODO: checkbox for no comments
// TODO: checkbox for no substepping

int main() {

    // TODO: draw all lines of script
    // TODO: draw CODE lines

    uint num_lines;
    String *lines;
    {
        num_lines = 0;
        lines = (String *) calloc(1024, sizeof(String)); // FORNOW
        char *start = script;
        for (char *read_head = script; (*read_head != '\0'); ++read_head) {
            if (*read_head == '\n') {
                bool is_empty_line = (read_head <= start);
                bool is_comment_to_jim = (start[0] == '/' && start[1] == '/');
                if ((!is_empty_line) && (!is_comment_to_jim)) {
                    String *command = &lines[num_lines++];
                    command->length = (read_head - start);
                    command->data = (char *) malloc(command->length + 1);
                    memcpy(command->data, start, command->length);
                    command->data[command->length] = '\0';
                }
                start = read_head + 1;
            }
        }
    }


    String *current_line = lines;
    while (begin_frame(NULL)) {
        if (key_pressed[GLFW_KEY_RIGHT]) {
            if (substep == 0) {
                ASSERT(string_matches_prefix(*current_line, "CODE"));
                do {
                    if (current_line->length > 4) {
                        vec3 color;
                        if ((current_line->length > 6) && (current_line->data[5] == '/') && (current_line->data[6] == '/')) {
                            color = monokai.purple;
                            substep = 0;
                        } else {
                            color = monokai.white;
                            substep = 1;
                        }
                        messagef(color, "%s", &current_line->data[5]);
                    } else {
                        substep = 0;
                        messagef({}, "");
                    }
                    ++current_line;
                } while (string_matches_prefix(*current_line, "...."));

                // FORNOW horrifying copypasta to make blue highlight hit a substep earlier

                String *ptr = current_line;
                while (!string_matches_prefix(*ptr, "CODE")) {
                    if (string_matches_prefix(*ptr, "SET")) {
                        bool consumed_dest = false;

                        ASSERT(ptr->length > strlen("SET"));
                        char *start = &ptr->data[strlen("SET") + 1];
                        char *read_head = start;
                        // String *dest_string = NULL;
                        while (true) {
                            ++read_head;
                            bool is_space = (*read_head == ' ');
                            bool is_end = !string_pointer_is_valid(*ptr, read_head);
                            if (is_space || is_end) {
                                if (!consumed_dest) {
                                    consumed_dest = true;
                                    String key = { start, uint(read_head - start) };
                                    for_each_live_object_ {
                                        if (string_matches_prefix(key, object->name)) {
                                            object->time_since_in_path = 0.0f;
                                            key.data += object->name.length;
                                            key.length -= object->name.length;
                                            bool found = false;
                                            for_each_field_(object) {
                                                if (string_matches_prefix(key, field->name)) {
                                                    field->time_since_in_path = 0.0f;
                                                    // field->time_since_changed = 0.0f;
                                                    // dest_string = &field->value;
                                                    found = true;
                                                    // break;
                                                }
                                            }
                                            ASSERT(found);
                                            break;
                                        }
                                    }
                                } else {
                                    // FORNOW
                                    // ASSERT(dest_string);
                                    // dest_string->length = (read_head - start);
                                    // memcpy(dest_string->data, start, dest_string->length);
                                    // dest_string->data[dest_string->length] = '\0';
                                }

                                start = ++read_head;
                            }
                            if (is_end) break;
                        }
                    }
                    ++ptr;
                }

            } else if (substep == 1) {
                while (!string_matches_prefix(*current_line, "CODE")) {
                    if (string_matches_prefix(*current_line, "OBJ")) {
                        Object *object = &objects[num_objects++]; // FORNOW
                        object->is_live = true;
                        bool consumed_object_name = false;

                        ASSERT(current_line->length > strlen("OBJ"));
                        char *start = &current_line->data[strlen("OBJ") + 1];
                        char *read_head = start;
                        while (true) {
                            ++read_head;
                            bool is_space = (*read_head == ' ');
                            bool is_end = !string_pointer_is_valid(*current_line, read_head);
                            if (is_space || is_end) {
                                String *dest_string;
                                if (!consumed_object_name) {
                                    consumed_object_name = true;
                                    dest_string = &object->name;
                                } else {
                                    dest_string = &object->fields[object->num_fields++].name;
                                }
                                dest_string->length = (read_head - start);
                                memcpy(dest_string->data, start, dest_string->length); // FORNOW
                                dest_string->data[dest_string->length] = '\0'; // FORNOW

                                start = ++read_head;
                            }
                            if (is_end) break;
                        }
                    } else if (string_matches_prefix(*current_line, "SET")) {
                        bool consumed_dest = false;

                        ASSERT(current_line->length > strlen("SET"));
                        char *start = &current_line->data[strlen("SET") + 1];
                        char *read_head = start;
                        String *dest_string = NULL;
                        while (true) {
                            ++read_head;
                            bool is_space = (*read_head == ' ');
                            bool is_end = !string_pointer_is_valid(*current_line, read_head);
                            if (is_space || is_end) {
                                if (!consumed_dest) {
                                    consumed_dest = true;
                                    String key = { start, uint(read_head - start) };
                                    for_each_live_object_ {
                                        if (string_matches_prefix(key, object->name)) {
                                            object->time_since_in_path = 0.0f;
                                            key.data += object->name.length;
                                            key.length -= object->name.length;
                                            bool found = false;
                                            for_each_field_(object) {
                                                if (string_matches_prefix(key, field->name)) {
                                                    // field->time_since_in_path = 0.0f;
                                                    field->time_since_changed = 0.0f;
                                                    dest_string = &field->value;
                                                    found = true;
                                                    break;
                                                }
                                            }
                                            ASSERT(found);
                                            break;
                                        }
                                    }
                                } else {
                                    // FORNOW
                                    ASSERT(dest_string);
                                    dest_string->length = (read_head - start);
                                    memcpy(dest_string->data, start, dest_string->length);
                                    dest_string->data[dest_string->length] = '\0';
                                }

                                start = ++read_head;
                            }
                            if (is_end) break;
                        }
                    } else if (string_matches_prefix(*current_line, "FILL")) {
                        // messagef(monokai.purple, "%s", *current_line);
                    }
                    ++current_line;
                }
                substep = 2;
            } else if (substep == 2) {
                substep = 0;
            }
        } else if (key_pressed[GLFW_KEY_LEFT]) {
            messagef(basic.red, "TODO: implement undo");
        }

        messages_update_and_draw();

        // EasyTextPen pen = { 16.0f, 300.0f, 20.0f, monokai.purple, true};
        // for_each_live_object_ {
        //     easy_text_drawf(&pen, "%s", object->name.data);
        //     for_each_field_(object) {
        //         easy_text_drawf(&pen, "    %s = %s", field->name.data, field->value.data);
        //     }
        //     easy_text_drawf(&pen, "");
        // }

        if (!substep) {
            for_each_live_object_ {
                object->time_since_created += 0.0167f;
                object->time_since_in_path += 0.0167f;
                for_each_field_(object) {
                    field->time_since_changed += 0.0167f;
                    field->time_since_in_path += 0.0167f;
                }
            }
        }

        real field_spacing = 72.0f;
        EasyTextPen pen = { V2(16.0f, 64.0f), 0.0f, basic.magenta, false};
        for_each_live_object_ {
            pen.color = CLAMPED_LINEAR_REMAP(object->time_since_in_path, 0.0f, 1.0f, monokai.blue, basic.gray);
            pen.color = CLAMPED_LINEAR_REMAP(object->time_since_created, 0.0f, 1.0f, monokai.yellow, pen.color);
            if (!string_matches_prefix(object->name, "xx")) {
                pen.font_height_Pixel = 24.0f;
                {
                    pen.offset_Pixel.x = 0.5f * ((object->num_fields * field_spacing) - _easy_text_dx(&pen, object->name));
                    easy_text_drawf(&pen, "%s", object->name.data);
                }
                pen.offset_Pixel = { 0, pen.offset_Pixel.y + 28.0f };
            }
            {
                pen.font_height_Pixel = 16.0f;
                uint field_index = 0;
                bool any = false;
                for_each_field_(object) {
                    if (!string_matches_prefix(field->name, "xx")) {
                        pen.color = CLAMPED_LINEAR_REMAP(field->time_since_in_path, 0.0f, 1.0f, monokai.blue, basic.gray);
                        pen.color = CLAMPED_LINEAR_REMAP(object->time_since_created, 0.0f, 1.0f, monokai.yellow, pen.color);
                        any = true;
                        pen.offset_Pixel.x = field_index * field_spacing;
                        pen.offset_Pixel.x += 0.5f * (field_spacing - _easy_text_dx(&pen, field->name));
                        easy_text_drawf(&pen, "%s", field->name.data);
                        ++field_index;
                    }
                }
                if (any) pen.offset_Pixel = { 0, pen.offset_Pixel.y + 26.0f };
            }
            {
                real y_top = pen.get_position_Pixel().y - 4.0f;
                real y_bottom = y_top + 24.0f; // FORNOW
                real x_left = pen.origin.x;
                real dx = field_spacing;
                real x_right = x_left + (object->num_fields * dx);
                eso_begin(OpenGL_from_Pixel, SOUP_LINES, true);
                eso_size(CLAMPED_LINEAR_REMAP(object->time_since_created, 0.0f, 1.0f, 3.0f, 1.0f));
                vec3 color = CLAMPED_LINEAR_REMAP(object->time_since_created, 0.0f, 1.0f, monokai.yellow, monokai.white);
                eso_color(color);
                eso_vertex(x_left, y_top);
                eso_vertex(x_right, y_top);
                eso_vertex(x_left, y_bottom);
                eso_vertex(x_right, y_bottom);
                for_(i, object->num_fields + 1) {
                    eso_vertex(x_left + (i * dx), y_top);
                    eso_vertex(x_left + (i * dx), y_bottom);
                }
                eso_end();
            }
            {
                uint field_index = 0;
                for_each_field_(object) {
                    pen.color = CLAMPED_LINEAR_REMAP(field->time_since_in_path, 0.0f, 1.0f, monokai.blue, monokai.white);
                    pen.color = CLAMPED_LINEAR_REMAP(field->time_since_changed, 0.0f, 1.0f, monokai.yellow, pen.color);
                    pen.font_height_Pixel = CLAMPED_LINEAR_REMAP(field->time_since_changed, 0.0f, 0.7f, 22.0f, 20.0f);
                    pen.offset_Pixel.x = field_index * field_spacing;
                    pen.offset_Pixel.x += 0.5f * (field_spacing - _easy_text_dx(&pen, field->value));
                    // pen.offset_Pixel.y -= CLAMPED_LINEAR_REMAP(field->time_since_changed, 0.0f, 0.7f, 0.5f, 0.0f);
                    easy_text_drawf(&pen, "%s", field->value.data);
                    ++field_index;
                }
            }
            pen.offset_Pixel = { 0, pen.offset_Pixel.y + 24.0f };
            pen.offset_Pixel = { 0, pen.offset_Pixel.y + 24.0f };
        }
    }
}
