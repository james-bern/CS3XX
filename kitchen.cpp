
char *kitchen_sink_script = 
"\33\33\33\33\33" // close all popups
"^n^N" // clear everything
"cz10\n" // circle
"cz\t10\n" // bigger circle
"bzx30\t30\n" // box
"ysadc<m2d 0 0>" // just select circles
"[5\t15\n" // extrude
"ysc<m2d 0 30><esc>qs3" // make square yellow
"1<m2d 30 15>0<esc>" // make right side of square red
"dasq1sq3" // deselect and select square with colors and mouse
"me<m2d 40 40>x15\t15\n" // move square
"{3\n" // cut slit with square
"ysc<m2d 0 0>Zm<m2d -50 0>" 
"sc<m2d 0 0>Am<m2d -50 0><m2d -15 5>" 
"]360\n" // do fun doughnut revolve
"^n"
"cx30\t30\n3.4\n" 
"ysaXzYzXzsa[1\n" // extrude 4 tiny disks
"^osplash.dxf\nysc<m2d 24 0><m2d 16 0>"
"[\t10\n" // extrude frame
"Ac<m2d 15.3 15.4>c<m2d -16.4 -16.3>ysc<m2d -16 16>]360\n"  // revolve thin diagonal doughnut
"^n" 
"l<m2d 0 0><m2d 0 10>l<m2d 0 10><m2d 10 0>l<m2d 10 0><m2d 0 0>" 
"yn25\n"
"sa[1\n" // tiny floating right triangle
"^n"
"cz8\n"
"<m3d 1 100 -1 0 -1 0>" 
"sa{100\n"
// take bite out of triangle corner
// ";" 
// "^odemo.dxf\n" 
// "^signore.stl\ny\n" 
// "^oignore.stl\n" 
// ".." 
// "pz\t5\n" // (Nathan) Polygon
// "cz18\nD<m2d 0 9>D<m2d 0 -9>s<m2d 2 -9><m2d -2 9>\b" // (Henok) DivideNearest
// "j2<m2d 1 7><m2d -1 -7>\n" //(Henok) Offset
// "^N^ob:wug.drawing\nysa"
;
