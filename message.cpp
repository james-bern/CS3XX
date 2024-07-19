#define MESSAGE_MAX_LENGTH 256
#define MESSAGE_MAX_NUM_MESSAGES 64
#define MESSAGE_MAX_TIME 6.0f

struct Message {
    _STRING_CALLOC(string, MESSAGE_MAX_LENGTH);
    real time_remaining;
    real y;
    vec3 base_color;
    uint height;
};

Message messages[MESSAGE_MAX_NUM_MESSAGES];

uint _message_index;

void messagef(vec3 color, char *format, ...) {
    if (other.please_suppress_messagef) return;
    va_list arg;
    va_start(arg, format);
    Message *message = &messages[_message_index];
    message->string.length = vsnprintf(message->string.data, MESSAGE_MAX_LENGTH, format, arg);
    va_end(arg);

    message->base_color = color;
    message->time_remaining = MESSAGE_MAX_TIME;
    _message_index = (_message_index + 1) % MESSAGE_MAX_NUM_MESSAGES;
    message->y = 0.0f;

    // Count the number of newlines in the message
    message->height = 1;
    for (uint i = 0; i < message->string.length; i++) {
        if (message->string.data[i] == '\n') {
            message->height++;
        }
    }

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
    real font_height_Pixel = 16.0f;
    uint i_0 = (_message_index == 0) ? (MESSAGE_MAX_NUM_MESSAGES - 1) : _message_index - 1;

    uint num_drawn = 0;
    uint lines_drawn = 1;
    auto draw_lambda = [&](uint message_index) {
        Message *message = &messages[message_index];

        real FADE_IN_TIME = 0.33f;
        real FADE_OUT_TIME = 2.0f;

        real alpha; { // ramp on ramp off
            alpha = 0
                + CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME, MESSAGE_MAX_TIME - FADE_IN_TIME, 0.0f, 1.0f)
                - CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, 0.0f, 1.0f);
        }

        vec3 color = CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME + FADE_IN_TIME, MESSAGE_MAX_TIME - 2.5f * FADE_IN_TIME, omax.yellow, message->base_color);
        color = CLAMPED_LINEAR_REMAP(message->time_remaining, MESSAGE_MAX_TIME - FADE_OUT_TIME, 0.0f, color, V3((color.x + color.y + color.z) / 3));

        real x = get_x_divider_drawing_mesh_Pixel() + font_height_Pixel;
        real y_target = lines_drawn * font_height_Pixel;
        lines_drawn += message->height;
        // if (message->time_remaining < FADE_OUT_TIME) y_target += CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, 0.0f, 12.0f);

        JUICEIT_EASYTWEEN(&message->y, y_target);
        if (message->time_remaining > 0) {
            text_draw(other.OpenGL_from_Pixel, message->string, V2(x, message->y), V4(color, alpha), font_height_Pixel);
        }
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

