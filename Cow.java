// TODO: a nice kitchen sink with COW branding, maybe some 3d stuff
// TODO: turn off automatic repaint (right now only happens when you resize the window)
// TODO: protect the entire student-facing API with checks for beginFrame() already called
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
        int counter = 0;
        while (beginFrame()) {
            ++counter;
            drawLine(0, 0, counter, counter);
        }
    }

    // TODO: camera

    static void drawLine(double x1, double y1, double x2, double y2) {
        _bufferedImageGraphics.setColor(Color.blue);
        _bufferedImageGraphics.drawLine((int) x1, (int) y1, (int) x2, (int) y2);
    }

    static int CANVAS_WIDTH = 512;
    static int CANVAS_HEIGHT = 512;

    static BufferedImage _bufferedImage;
    static Graphics _bufferedImageGraphics;
    static JPanelExtender _jPanelExtender;
    static JFrame _jFrame;

    static boolean _cow_initialized;

    static void _cow_initialize() {
        _bufferedImage = new BufferedImage(CANVAS_WIDTH, CANVAS_HEIGHT, BufferedImage.TYPE_INT_ARGB);

        assert _bufferedImage != null;

        _bufferedImageGraphics = _bufferedImage.createGraphics();
        assert _bufferedImageGraphics != null;

        _jPanelExtender = new JPanelExtender(_bufferedImage, _bufferedImageGraphics);

        _jFrame = new JFrame("Cow.java 🙂👍");
        _jFrame.add(_jPanelExtender);
        _jFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        _jFrame.getContentPane().add(_jPanelExtender, BorderLayout.CENTER);
        // _jFrame.setUndecorated(true);
        _jFrame.setSize(CANVAS_WIDTH, CANVAS_HEIGHT);
        _jFrame.setVisible(true);

    }

    static long startTime = System.currentTimeMillis();

    static boolean beginFrame() {
        if (!_cow_initialized) {
            _cow_initialized = true;
            _cow_initialize();
        }
        _jPanelExtender.repaint();

        // FORNOW: very optimistic sleep
        try { Thread.sleep(1000 / 60); } catch (Exception exception) {};

        // FORNOW: spin wait
        // while ((new Date()).getTime() - startTime < 1000 / 60) { }
        // startTime = System.currentTimeMillis();

        // beginning of next frame
        _bufferedImageGraphics.setColor(Color.white);
        _bufferedImageGraphics.fillRect(0, 0, 512, 512);
        return true;
    }
}


class JPanelExtender extends JPanel {
    static BufferedImage _bufferedImage;
    static Graphics _bufferedImageGraphics;
    JPanelExtender(BufferedImage _bufferedImage, Graphics _bufferedImageGraphics) {
        super();
        this._bufferedImage = _bufferedImage;
        this._bufferedImageGraphics = _bufferedImageGraphics;
    }
    @Override
    public void paintComponent(Graphics paintComponentGraphics) { 
        while (_bufferedImageGraphics == null) {}
        while (_bufferedImage == null) {}
        paintComponentGraphics.drawImage(_bufferedImage, 0, 0, null);
    }
}






class Vector2 {
    double x;
    double y;

    public String toString() { return "(" + this.x + ", " + this.y + ")"; }

    Vector2() {} // NOTE: x and y automatically initialized to zero
    Vector2(double x, double y) {
        this.x = x;
        this.y = y;
    }
    Vector2(Vector2 other) {
        this.x = other.x;
        this.y = other.y;
    }
    Vector2(double s) {
        this.x = s;
        this.y = s;
    }

    Vector2 plus(Vector2 other) { return new Vector2(this.x + other.x, this.y + other.y); }
    Vector2 minus(Vector2 other) { return new Vector2(this.x - other.x, this.y - other.y); }
    Vector2 times(double scalar) { return new Vector2(scalar * this.x, scalar * this.y); }
    Vector2 dividedBy(double scalar) { return this.times(1.0 / scalar); }
    double squaredLength() { return this.x * this.x + this.y * this.y; }
    double length() { return Math.sqrt(this.squaredLength()); }
    Vector2 directionVector() { return this.dividedBy(this.length()); }

    static double distanceBetween(Vector2 a, Vector2 b) { return (b.minus(a)).length(); }
    static Vector2 directionVectorFrom(Vector2 a, Vector2 b) { return (b.minus(a)).directionVector(); }
    static Vector2 lerp(double t, Vector2 a, Vector2 b) { return a.times(1.0 - t).plus(b.times(t)); }

    static final Vector2 right = new Vector2( 1.0,  0.0);
    static final Vector2 left  = new Vector2(-1.0,  0.0);
    static final Vector2 up    = new Vector2( 0.0,  1.0);
    static final Vector2 down  = new Vector2( 0.0, -1.0);
}

class Vector3 {
    double x;
    double y;
    double z;

    public String toString() { return "(" + this.x + ", " + this.y + ", " + this.z + ")"; }

    Vector3() { }

    Vector3(double x, double y, double z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    Vector3(Vector3 p) {
        this.x = p.x;
        this.y = p.y;
        this.z = p.z;
    }
    Vector3(double s) {
        this.x = s;
        this.y = s;
        this.z = s;
    }

    Vector3 plus(Vector3 other) { return new Vector3(this.x + other.x, this.y + other.y, this.z + other.z); }
    Vector3 minus(Vector3 other) { return new Vector3(this.x - other.x, this.y - other.y, this.z - other.z); }
    Vector3 times(double scalar) { return new Vector3(scalar * this.x, scalar * this.y, scalar * this.z); }
    Vector3 dividedBy(double scalar) { return this.times(1.0 / scalar); }
    double squaredLength() { return this.x * this.x + this.y * this.y + this.z * this.z; }
    double length() { return Math.sqrt(this.squaredLength()); }
    Vector3 directionVector() { return this.dividedBy(this.length()); }

    static double distanceBetween(Vector3 a, Vector3 b) { return (b.minus(a)).length(); }
    static Vector3 directionVectorFrom(Vector3 a, Vector3 b) { return (b.minus(a)).directionVector(); }
    static Vector3 lerp(double t, Vector3 a, Vector3 b) { return a.times(1.0 - t).plus(b.times(t)); }

    static final Vector3 white     = new Vector3(1.0 , 1.0 , 1.0 );
    static final Vector3 lightGray = new Vector3(0.75, 0.75, 0.75);
    static final Vector3 gray      = new Vector3(0.5 , 0.5 , 0.5 );
    static final Vector3 darkGray  = new Vector3(0.25, 0.25, 0.25);
    static final Vector3 black     = new Vector3(0.0 , 0.0 , 0.0 );
    static final Vector3 red       = new Vector3(1.0 , 0.0 , 0.0 );
    static final Vector3 orange    = new Vector3(1.0 , 0.5 , 0.0 );
    static final Vector3 yellow    = new Vector3(1.0 , 1.0 , 0.0 );
    static final Vector3 green     = new Vector3(0.0 , 1.0 , 0.0 );
    static final Vector3 cyan      = new Vector3(0.0 , 1.0 , 1.0 );
    static final Vector3 blue      = new Vector3(0.0 , 0.0 , 1.0 );
    static final Vector3 magenta   = new Vector3(1.0 , 0.0 , 1.0 );
    static Vector3 rainbowSwirl(double _DEBUG_time) {
        return new Vector3(_rainbowSwirlHelper(_DEBUG_time, 0.0), _rainbowSwirlHelper(_DEBUG_time, 0.33), _rainbowSwirlHelper(_DEBUG_time, -0.33));
    }

    ////////////////////////////////////////////////////////////////////////////

    static double _rainbowSwirlHelper(double _DEBUG_time, double offset) {
        return 0.5 + 0.5 * Math.cos(6.28 * (offset - _DEBUG_time));
    }
}
