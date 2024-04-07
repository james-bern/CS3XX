// TODO: a crash should cause the window to close
// TODO: a nice kitchen sink with COW branding, maybe some 3d stuff
// TODO: turn off automatic repaint (right now only happens when you resize the window)
// TODO: protect the entire student-facing API with checks for begin_frame() already called
// TODO: bouncing balls demo

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.util.*;
import javax.swing.*;
import java.lang.Math;
import java.io.*;


class Cow {

    public static void main(String[] arguments) {
        // configure
        set_canvas_world_left_bottom_corner(-5, -5);
        set_canvas_world_size(10, 10);

        // state
        double time = 0.0;

        // loop
        while (begin_frame()) {
            if (!key_toggled('P')) { // update
                time += 0.0167;
            }

            { // draw
                set_draw_color(Color.yellow);
                draw_center_rectangle(0, 0, 4, 2, false);

                set_draw_color(Color.green);
                set_draw_width(2.0);
                draw_corner_rectangle(2, 1, 3, 3, true);

                set_draw_color(color_rainbow_swirl(time));
                set_draw_width(4.0);
                draw_line(-2, 1, 3, 3);

                for (int pass = 0; pass < 2; ++pass) {
                    if (pass == 0) {
                        set_draw_color(1.0, 0.0, 1.0, 0.5);
                    } else {
                        set_draw_color(0.0, 0.0, 0.0);
                        set_draw_width(3.0);
                    }
                    draw_circle(2.0 * Math.cos(time), 2.0 * Math.sin(time), 1, (pass == 1));
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////

    // TODO mouse and key stuff
    // TODO imgui

    static boolean key_toggled(char key) {
        return false;
    }

    /////////////////////////////////////////////////////////////////////////

    static int canvas_pixel_height = 512;
    static double canvas_world_left   = 0;
    static double canvas_world_bottom = 0;
    static double canvas_world_width  = 16;
    static double canvas_world_height = 9;
    static double canvas_aspect_ratio() { return canvas_world_width / canvas_world_height; }
    static int canvas_pixel_width() {
        return (int) (canvas_aspect_ratio() * canvas_pixel_height);
    }

    static void set_canvas_world_left_bottom_corner(double x, double y) {
        canvas_world_left = x;
        canvas_world_bottom = y;
    }

    static void set_canvas_world_size(double x, double y) {
        canvas_world_width = x;
        canvas_world_height = y;
    }


    static void set_draw_width(double w) {
        assert w >= 0;
        ((Graphics2D) _buffered_image_graphics).setStroke(new BasicStroke((float) w));
    }

    static void TODO_set_draw_color(Color color, double a) {
        // TODO
        // assert a >= 0;
        // assert a <= 1;
        // _buffered_image_graphics.setColor(new Color(color.getRed(), color.getGreen(), color.getBlue(), (float) a));
    }

    static void set_draw_color(Color color) {
        _buffered_image_graphics.setColor(color);
    }

    static void set_draw_color(double r, double g, double b, double a) {
        assert r >= 0;
        assert r <= 1;
        assert g >= 0;
        assert g <= 1;
        assert b >= 0;
        assert b <= 1;
        assert a >= 0;
        assert a <= 1;
        set_draw_color(new Color((float) r, (float) g, (float) b, (float) a));
    }

    static void set_draw_color(double r, double g, double b) {
        set_draw_color(r, g, b, 1.0);
    }

    static float _color_rainbow_swirl_helper(double time, double offset) {
        return 0.5f + 0.5f * (float) Math.cos(6.28 * (offset - time));
    }
    static Color color_rainbow_swirl(double time) {
        return new Color(_color_rainbow_swirl_helper(time, 0.0), _color_rainbow_swirl_helper(time, 0.33), _color_rainbow_swirl_helper(time, -0.33));
    }


    static void draw_line(double x1, double y1, double x2, double y2) {
        _buffered_image_graphics.drawLine(X(x1), Y(y1), X(x2), Y(y2));
    }

    static void draw_corner_rectangle(double x1, double y1, double x2, double y2, boolean outlined) {
        int Xx1 = X(x1);
        int Yy1 = Y(y1);
        int Xx2 = X(x2);
        int Yy2 = Y(y2);
        int arg1 = Math.min(Xx1, Xx2);
        int arg2 = Math.min(Yy1, Yy2);
        int arg3 = Math.abs(Xx1 - Xx2);
        int arg4 = Math.abs(Yy1 - Yy2);
        if (!outlined) {
            _buffered_image_graphics.fillRect(arg1, arg2, arg3, arg4);
        } else {
            _buffered_image_graphics.drawRect(arg1, arg2, arg3, arg4);
        }
    }

    static void draw_center_rectangle(double x, double y, double width, double height, boolean outlined) {
        assert width >= 0;
        assert height >= 0;
        double half_width = width / 2;
        double half_height = height / 2;
        draw_corner_rectangle(x - half_width, y - half_height, x + half_width, y + half_height, outlined);
    }

    static void draw_circle(double x, double y, double r, boolean outlined) {
        int arg1 = X(x - r);
        int arg2 = Y(y + r);
        int arg3 = L(2 * r);
        int arg4 = arg3;
        if (!outlined) {
            _buffered_image_graphics.fillOval(arg1, arg2, arg3, arg4);
        } else {
            _buffered_image_graphics.drawOval(arg1, arg2, arg3, arg4);
        }
    }

    // TODO: camera (ability to zoom out, pan)
    // TODO: ability to resize window
    // TODO: window decorations

    static double pixel_from_world_scale() {
        return canvas_pixel_height / canvas_world_height;
    }

    static int X(double x_world) {
        return (int) (pixel_from_world_scale() * (x_world - canvas_world_left));
    }

    static int Y(double y_world) {
        return (int) (canvas_pixel_height - pixel_from_world_scale() * ((y_world - canvas_world_bottom)));
    }

    static int L(double length_world) {
        return (int) (pixel_from_world_scale() * length_world);
    }

    static BufferedImage _buffered_image;
    static Graphics _buffered_image_graphics;
    static CowJPanelExtender _jPanel_extender;
    static JFrame _jFrame;
    static boolean _cow_initialized;
    static void _cow_initialize() {
        _buffered_image = new BufferedImage(canvas_pixel_width(), canvas_pixel_height, BufferedImage.TYPE_INT_ARGB);
        assert _buffered_image != null;

        _buffered_image_graphics = _buffered_image.createGraphics();
        assert _buffered_image_graphics != null;

        _jPanel_extender = new CowJPanelExtender(_buffered_image, _buffered_image_graphics);

        _jFrame = new JFrame("Cow.java 🙂👍");
        _jFrame.add(_jPanel_extender);
        _jFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        _jFrame.getContentPane().add(_jPanel_extender, BorderLayout.CENTER);
        _jFrame.setUndecorated(true);
        _jFrame.setSize(canvas_pixel_width(), canvas_pixel_height);
        _jFrame.setVisible(true);

    }


    static boolean begin_frame() {
        if (!_cow_initialized) {
            _cow_initialized = true;
            _cow_initialize();
        }

        _jPanel_extender.repaint();

        try { Thread.sleep(1000 / 60); } catch (Exception exception) {};

        // beginning of next frame
        Color tmp = _buffered_image_graphics.getColor();
        _buffered_image_graphics.setColor(Color.white);
        _buffered_image_graphics.fillRect(0, 0, canvas_pixel_width(), canvas_pixel_height);
        _buffered_image_graphics.setColor(tmp);
        return true;
    }
}

class CowJPanelExtender extends JPanel {
    static BufferedImage _buffered_image;
    static Graphics _buffered_image_graphics;
    CowJPanelExtender(BufferedImage _buffered_image, Graphics _buffered_image_graphics) {
        super();
        this._buffered_image = _buffered_image;
        this._buffered_image_graphics = _buffered_image_graphics;
    }
    @Override
    public void paintComponent(Graphics paintComponentGraphics) { 
        while (_buffered_image_graphics == null) {}
        while (_buffered_image == null) {}
        paintComponentGraphics.drawImage(_buffered_image, 0, 0, null);
    }
}


