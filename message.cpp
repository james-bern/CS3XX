#define MESSAGE_MAX_LENGTH 256
#define MESSAGE_MAX_NUM_MESSAGES 64
#define MESSAGE_MAX_TIME 16.0f

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
    message->y = 0.0f;//-16.0f; // FORNOW

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
    real font_height_Pixel = 12.0f;
    uint i_0 = (_message_index == 0) ? (MESSAGE_MAX_NUM_MESSAGES - 1) : _message_index - 1;

    real epsilon = font_height_Pixel;
    real x_left, y_top, x_right, y_bottom;
    bbox2 bbox;
    {
        x_left = get_x_divider_drawing_mesh_Pixel() + epsilon;
        x_right = window_get_width_Pixel() - epsilon;
        y_bottom = window_get_height_Pixel() - 2 * epsilon;
        y_top = y_bottom - 96.0f;
        bbox = { x_left, y_top, x_right, y_bottom };
    }

    uint lines_drawn = 0;
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
        color = CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, color, V3((color.x + color.y + color.z) / 3));

        real y_target = lines_drawn * font_height_Pixel;
        lines_drawn += message->height;
        // if (message->time_remaining < FADE_OUT_TIME) y_target += CLAMPED_LINEAR_REMAP(message->time_remaining, FADE_OUT_TIME, 0.0f, 0.0f, 12.0f);

        JUICEIT_EASYTWEEN(&message->y, y_target);
        if (message->time_remaining > 0) {
            text_draw(other.OpenGL_from_Pixel, message->string, ROUND(V2(x_left, y_top + message->y)), V4(color, alpha), font_height_Pixel);
        }
    };

    glEnable(GL_SCISSOR_TEST);
    gl_scissor_Pixel(bbox);
    { // this is pretty gross
        uint i = i_0;
        while (true) {
            draw_lambda(i);

            if (i > 0) --i;
            else if (i == 0) i = MESSAGE_MAX_NUM_MESSAGES - 1;

            if (i == i_0) break;
        }
    }
    glDisable(GL_SCISSOR_TEST);
    
    // TODO: stencil test to the transition in beautiful
    bbox2 inflated_bbox = bbox_inflate(bbox, epsilon / 2); 
    eso_begin(other.OpenGL_from_Pixel, SOUP_QUADS);
    eso_color(omax.black, 0.3f);
    eso_bbox_SOUP_QUADS(inflated_bbox);
    eso_end();
    eso_begin(other.OpenGL_from_Pixel, SOUP_LINE_LOOP);
    eso_color(omax.dark_gray);
    eso_bbox_SOUP_QUADS(inflated_bbox);
    eso_end();
}

