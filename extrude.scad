dxf_filename = "pacman.dxf";
extrusion_height = 3.0;
$fn = 64; // number of line segments used when discretizing a complete circle
linear_extrude(extrusion_height)
  scale([25.4, 25.4, 25.4]) // NOTE: OMAX exports in inches
    import(dxf_filename);