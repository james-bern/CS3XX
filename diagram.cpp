#include "playground.cpp"
#include "easy_mode.cpp"
char *script = R""(
CODE // let's learn about array lists in C/C++!
CODE
CODE ArrayList list;
OBJ list .length .capacity .array
// FILL ? list
PUT list.length ?
PUT list.capacity ?
PUT list.array ?
CODE list = {};
PUT list.length 0
PUT list.capacity 0
PUT list.array NULL
CODE list.capacity = 3;
PUT list.capacity 3
CODE list.array = (double *) malloc(list.capacity, sizeof(double));
OBJ foo [0] [1] [2]
PUT foo[0] ?
PUT foo[1] ?
PUT foo[2] ?
PUT foo list.array
CODE memset(list.array, 0, list.capacity * sizeof(double));
// FILL foo 0.0
PUT foo[0] 0.0
PUT foo[1] 0.0
PUT foo[2] 0.0
CODE // note: we could have used "ArrayList list = {};"
.... //       instead of "ArrayList list;" and "list = {};"
CODE // note: we could have picked any initial capacity instead of 3
CODE // note: we could have called calloc(...)
.... //       instead of malloc(...) and memset(..., 0, ...) 
CODE
CODE // // as long as there are empty slots in the array, append(...) takes two steps
CODE // 1) write the element to the array
CODE list.array[list.length] = 1.0;
PUT foo[0] 1.0
CODE // 2) increment the list's length
CODE list.length++;
PUT list.length 1
CODE
CODE // note: we can do both steps on one line using "length++"
CODE list.array[list.length++] = 1.0;
PUT foo[1] 1.0
PUT list.length 2
CODE
CODE list.array[list.length++] = 2.0;
PUT foo[2] 2.0
PUT list.length 3
CODE
CODE // // we're out of room now, so the next append will be trickier!
CODE
CODE
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

        vec3 color = message->base_color;//CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME + FADE_IN_TIME, MESSAGE_MAX_TIME - 5.0f * FADE_IN_TIME, basic.yellow, message->base_color);
                                         // color = LERP(CLAMP(INVERSE_LERP(message->time_remaining, MESSAGE_MAX_TIME - FADE_OUT_TIME, 0.0f), 0.0f, 0.8f), color, V3(0.6f));

        real x = 16 + font_height_Pixel;
        real y_target = ++num_drawn * font_height_Pixel;
        // if (message->time_remaining < FADE_OUT_TIME) y_target += CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, 0.0f, 12.0f);

        JUICEIT_EASYTWEEN(&message->y, y_target);
        // if (message->time_remaining > 0) {
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
};

#define MAX_NUM_FIELDS 8
struct Object {
    bool is_live;
    _STRING_CALLOC(name, 64);
    uint num_fields;
    Field fields[MAX_NUM_FIELDS];
};

#define MAX_NUM_OBJECTS 8
uint num_objects;
Object objects[MAX_NUM_OBJECTS];


#define _for_each_object_ for (Object *object = objects; object < objects + MAX_NUM_OBJECTS; ++object)
#define for_each_live_object_ _for_each_object_ if (object->is_live)

// TODO: parse line into words (for CODE we can wrap the argument in "..." fornow)
// This will make everything way easier

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
            ASSERT(string_matches_prefix(*current_line, "CODE"));
            do {
                if (current_line->length > 4) {
                    vec3 color = monokai.white;
                    if ((current_line->length > 6) && (current_line->data[5] == '/') && (current_line->data[6] == '/')) {
                        color = monokai.blue;
                    }
                    messagef(color, "%s", &current_line->data[5]);
                } else {
                    messagef({}, "");
                }
                ++current_line;
            } while (string_matches_prefix(*current_line, "...."));

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
                } else if (string_matches_prefix(*current_line, "PUT")) {
                    bool consumed_dest = false;

                    ASSERT(current_line->length > strlen("PUT"));
                    char *start = &current_line->data[strlen("PUT") + 1];
                    char *read_head = start;
                    String *dest_string;
                    while (true) {
                        ++read_head;
                        bool is_space = (*read_head == ' ');
                        bool is_end = !string_pointer_is_valid(*current_line, read_head);
                        if (is_space || is_end) {
                            if (!consumed_dest) {
                                consumed_dest = true;
                                String key = { start, uint(read_head - start) };
                                for_(object_index, num_objects) {
                                    Object *object = &objects[object_index];
                                    if (string_matches_prefix(key, object->name)) {
                                        dest_string = &objects->fields[0].value;
                                    }
                                }
                            } else {
                                // FORNOW
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
        } else if (key_pressed[GLFW_KEY_LEFT]) {
            messagef(basic.red, "TODO: implement undo");
        }

        messages_update_and_draw();

        EasyTextPen pen = { 16.0f, 300.0f, 16.0f, monokai.purple, true};

        for_each_live_object_ {
            easy_text_drawf(&pen, "%s", object->name.data);
            for (Field *field = object->fields; field < object->fields + object->num_fields; ++field) {
                easy_text_drawf(&pen, "    %s = %s", field->name.data, field->value.data);
            }
            easy_text_drawf(&pen, "");
        }
    }
}
