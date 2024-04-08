// TODO: a crash should cause the window to close
// TODO: a nice kitchen sink with COW branding, maybe some 3d stuff
// TODO: turn off automatic repaint (right now only happens when you resize the window)
// TODO: protect the entire student-facing API with checks for begin_frame() already called
// TODO: set_canvas_clear_color

// // TODO: demos
// TODO: paint
// TODO: tic tac toe
// TODO: flappy bird

// // NOTE: limitations
// NOTE: drops very fast press and release

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.util.*;
import javax.swing.*;
import java.lang.Math;
import java.io.*;

class DemoTicTacToe extends Cow {
    final static int NONE = 0;
    final static int X_PLAYER = 1;
    final static int O_PLAYER = 2;
    public static void main(String[] arguments) {
        // configure
        set_canvas_world_left_bottom_corner(0, 0);
        set_canvas_world_size(3, 3);
        canvas_pixel_height = 256;

        // state
        int board[][] = new int[3][3]; // board[row (y)][column (x)]
        int current_player = X_PLAYER;
        boolean game_is_over = false;

        // loop
        while (begin_frame()) {
            int hot_row = (int) Math.floor(mouse_y);
            int hot_column = (int) Math.floor(mouse_x);

            if (!game_is_over) { // update
                if (mouse_pressed) {
                    if (board[hot_row][hot_column] == NONE) {
                        board[hot_row][hot_column] = current_player;
                        current_player = (current_player == X_PLAYER) ? O_PLAYER : X_PLAYER;
                    }
                }

                { // check for game over
                    for (int row = 0; row < 3; ++row) {
                        for (int column = 0; column < 3; ++column) {
                        }
                    }
                }

            }

            { // draw
                if (!game_is_over) { // hot square
                    set_draw_color(Color.yellow, 0.5);
                    fill_corner_rectangle(hot_column, hot_row, hot_column + 1, hot_row + 1);
                }

                { // X's and O's
                    set_draw_color(Color.black);
                    for (int row = 0; row < 3; ++row) {
                        for (int column = 0; column < 3; ++column) {
                            if (board[row][column] == X_PLAYER) {
                                draw_line(column, row, column + 1, row + 1);
                                draw_line(column, row + 1, column + 1, row);
                            } else if (board[row][column] == O_PLAYER) {
                                outline_circle(column + 0.5, row + 0.5, 0.5);
                            }
                        }
                    }
                }

                { // board lines
                    set_draw_color(Color.black);
                    draw_line(1, 0, 1, 3);
                    draw_line(2, 0, 2, 3);
                    draw_line(0, 1, 3, 1);
                    draw_line(0, 2, 3, 2);
                }
            }
        }
    }
}

class DemoKitchenSink extends Cow {
    static class Particle {
        double x;
        double y;
        Color color;
    };

    public static void main(String[] arguments) {
        // configure
        set_canvas_world_left_bottom_corner(-5, -5);
        set_canvas_world_size(10, 10);

        // state
        double time = 0.0;
        ArrayList<Particle> particles = new ArrayList<>();
        double x = 0.0;
        double y = 0.0;

        // loop
        while (begin_frame()) {
            if (!key_toggled['P']) { // update
                time += 0.0167;

                if (mouse_pressed) {
                    Particle particle = new Particle();
                    particle.x = mouse_x;
                    particle.y = mouse_y;
                    particle.color = Color.orange;
                    particles.add(particle);
                }

                if (mouse_released) {
                    Particle particle = new Particle();
                    particle.x = mouse_x;
                    particle.y = mouse_y;
                    particle.color = Color.cyan;
                    particles.add(particle);
                }

                {
                    double delta = 0.1;
                    if (key_held['W']) y += delta;
                    if (key_held['A']) x -= delta;
                    if (key_held['S']) y -= delta;
                    if (key_held['D']) x += delta;
                }
            }

            { // draw

                if (!mouse_held) {
                    set_draw_color(Color.gray);
                } else {
                    set_draw_color(Color.yellow);
                }
                fill_center_rectangle(0, 0, 4, 2);

                set_draw_color(Color.green);
                set_draw_width(2.0);
                outline_corner_rectangle(-5, -5, 5, 5);

                set_draw_color(Color.magenta);
                set_draw_width(4.0);
                draw_line(mouse_x, mouse_y, 3, 3);

                for (int pass = 0; pass < 2; ++pass) {
                    if (pass == 0) {
                        set_draw_color(color_rainbow_swirl(time), 0.5);
                    } else {
                        set_draw_color(0.0, 0.0, 0.0);
                        set_draw_width(3.0);
                    }
                    _draw_circle(x, y, 1, (pass == 1));
                }

                for (int i = 0; i < particles.size(); ++i) {
                    set_draw_color(particles.get(i).color);
                    fill_circle(particles.get(i).x, particles.get(i).y, 0.1);
                }
            }
        }
    }
}

class Cow {

    public static void main(String[] arguments) {
        // DemoKitchenSink.main(null);
        DemoTicTacToe.main(null);
    }

    /////////////////////////////////////////////////////////////////////////

    // TODO mouse and key stuff
    // TODO imgui


    static boolean mouse_held;
    static boolean _mouse_held_prev;
    static boolean mouse_pressed;
    static boolean mouse_released;
    static float mouse_x;
    static float mouse_y;

    static boolean _key_held_prev[]     = new boolean[256];
    static boolean key_held[]     = new boolean[256];
    static boolean key_released[] = new boolean[256];
    static boolean key_pressed[]  = new boolean[256];
    static boolean key_toggled[]  = new boolean[256];

    /////////////////////////////////////////////////////////////////////////

    // TODO: user-facing API takes doubles and returns floats

    static int canvas_pixel_height = 512;
    static float canvas_world_left   = 0;
    static float canvas_world_bottom = 0;
    static float canvas_world_width  = 16;
    static float canvas_world_height = 9;
    static float canvas_aspect_ratio() { return canvas_world_width / canvas_world_height; }
    static int canvas_pixel_width() {
        return (int) (canvas_aspect_ratio() * canvas_pixel_height);
    }

    static void set_canvas_world_left_bottom_corner(double x, double y) {
        canvas_world_left = (float) x;
        canvas_world_bottom = (float) y;
    }

    static void set_canvas_world_size(double x, double y) {
        canvas_world_width = (float) x;
        canvas_world_height = (float) y;
    }


    static void set_draw_width(double w) {
        assert w >= 0;
        ((Graphics2D) _buffered_image_graphics).setStroke(new BasicStroke((float) w));
    }

    static void set_draw_color(Color color, double a) {
        assert a >= 0;
        assert a <= 1;
        _buffered_image_graphics.setColor(new Color(color.getRed() / 255.0f, color.getGreen() / 255.0f, color.getBlue() / 255.0f, (float) a));
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

    static Color color_rainbow_swirl(double time) {
        double TAU   = 6.283;
        double red   = 0.5f + 0.5f * (float) Math.cos(TAU * ( 0.000 - time));
        double green = 0.5f + 0.5f * (float) Math.cos(TAU * ( 0.333 - time));
        double blue  = 0.5f + 0.5f * (float) Math.cos(TAU * (-0.333 - time));
        return new Color((float) red, (float) green, (float) blue);
    }


    static void draw_line(double x1, double y1, double x2, double y2) {
        _buffered_image_graphics.drawLine(X_pixel_from_world(x1), Y_pixel_from_world(y1), X_pixel_from_world(x2), Y_pixel_from_world(y2));
    }

    static void fill_corner_rectangle(double x1, double y1, double x2, double y2) { _draw_corner_rectangle(x1, y1, x2, y2, false); }
    static void outline_corner_rectangle(double x1, double y1, double x2, double y2) { _draw_corner_rectangle(x1, y1, x2, y2, true); }
    static void _draw_corner_rectangle(double x1, double y1, double x2, double y2, boolean outlined) {
        int Xx1 = X_pixel_from_world(x1);
        int Yy1 = Y_pixel_from_world(y1);
        int Xx2 = X_pixel_from_world(x2);
        int Yy2 = Y_pixel_from_world(y2);
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

    static void fill_center_rectangle(double x, double y, double width, double height) { _draw_center_rectangle(x, y, width, height, false); }
    static void outline_center_rectangle(double x, double y, double width, double height) { _draw_center_rectangle(x, y, width, height, true); }
    static void _draw_center_rectangle(double x, double y, double width, double height, boolean outlined) {
        assert width >= 0;
        assert height >= 0;
        double half_width = width / 2;
        double half_height = height / 2;
        _draw_corner_rectangle(x - half_width, y - half_height, x + half_width, y + half_height, outlined);
    }

    static void fill_circle(double x, double y, double r) { _draw_circle(x, y, r, false); }
    static void outline_circle(double x, double y, double r) { _draw_circle(x, y, r, true); }
    static void _draw_circle(double x, double y, double r, boolean outlined) {
        int arg1 = X_pixel_from_world(x - r);
        int arg2 = Y_pixel_from_world(y + r);
        int arg3 = L_pixel_from_world(2 * r);
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

    static float pixel_from_world_scale() {
        return canvas_pixel_height / canvas_world_height;
    }

    static int X_pixel_from_world(double x_world) {
        return (int) (pixel_from_world_scale() * (x_world - canvas_world_left));
    }

    static int Y_pixel_from_world(double y_world) {
        return (int) (canvas_pixel_height - (pixel_from_world_scale() * (y_world - canvas_world_bottom)));
    }

    static int L_pixel_from_world(double length_world) {
        return (int) (pixel_from_world_scale() * length_world);
    }

    static float X_world_from_pixel(int x_pixel) {
        return (float) ((x_pixel / pixel_from_world_scale()) + canvas_world_left);
    }

    static float Y_world_from_pixel(int y_pixel) {
        return (float) (((canvas_pixel_height - y_pixel) / pixel_from_world_scale()) + canvas_world_bottom);
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

        _jPanel_extender = new CowJPanelExtender();
        _jPanel_extender.setPreferredSize(new Dimension(canvas_pixel_width(), canvas_pixel_height));

        _jFrame = new JFrame("Cow.java 🙂👍");
        _jFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        _jFrame.setLocation(0, 0);
        _jFrame.getContentPane().add(_jPanel_extender, BorderLayout.CENTER);
        _jFrame.pack();
        _jFrame.setVisible(true);

    }


    static boolean begin_frame() {
        if (!_cow_initialized) {
            _cow_initialized = true;
            _cow_initialize();
        }

        { // *_pressed, *_released
            { // mouse_pressed, mouse_released
                mouse_pressed =  (!_mouse_held_prev && mouse_held);
                mouse_released = (_mouse_held_prev && !mouse_held);
                _mouse_held_prev = mouse_held;
            }
            { // keyboard
                for (int i = 0; i < 256; ++i) {
                    key_pressed[i]  = (!_key_held_prev[i] && key_held[i]);
                    key_released[i] = (_key_held_prev[i] && !key_held[i]);
                    if (key_released[i]) key_toggled[i] = !key_toggled[i];
                }
                System.arraycopy(key_held, 0, _key_held_prev, 0, key_held.length); // NOTE: silly order of arguments
            }
        }

        _jPanel_extender.repaint();

        try { Thread.sleep(1000 / 60); } catch (Exception exception) {};

        // beginning of next frame

        { // mouse
            Point point;
            {
                point = MouseInfo.getPointerInfo().getLocation();
                SwingUtilities.convertPointFromScreen(point, _jPanel_extender);
            }
            mouse_x = X_world_from_pixel(point.x);
            mouse_y = Y_world_from_pixel(point.y);
        }


        { // canvas
            Color tmp = _buffered_image_graphics.getColor();
            _buffered_image_graphics.setColor(Color.white);
            _buffered_image_graphics.fillRect(0, 0, canvas_pixel_width(), canvas_pixel_height);
            _buffered_image_graphics.setColor(tmp);
        }


        return true;
    }
}

class CowJPanelExtender extends JPanel {
    CowJPanelExtender() {
        super();

        this.addMouseListener( 
                new MouseAdapter() {
                    @Override
                    public void mousePressed(MouseEvent e) {
                        Cow.mouse_held = true;
                    }

                    @Override
                    public void mouseReleased(MouseEvent e) {
                        Cow.mouse_held = false;
                    }
                }
                );

        KeyboardFocusManager.getCurrentKeyboardFocusManager().addKeyEventDispatcher(event -> {
            synchronized (Cow.class) {
                int _key = event.getKeyCode();
                if (_key >= 256) return false;
                char key = (char) _key;

                if (event.getID() == KeyEvent.KEY_PRESSED) {
                    Cow.key_held[key] = true;
                } else if (event.getID() == KeyEvent.KEY_RELEASED) {
                    Cow.key_held[key] = false;
                }

                return false;
            }
        });
    }

    @Override
    public void paintComponent(Graphics paintComponentGraphics) { 
        super.paintComponent(paintComponentGraphics);
        while (Cow._buffered_image_graphics == null) {}
        while (Cow._buffered_image == null) {}
        paintComponentGraphics.drawImage(Cow._buffered_image, 0, 0, null);
    }
}


