ffmpeg -i <input.mp4>  -r 10 -f image2pipe -vcodec ppm - | convert -delay 10 -loop 0 -layers Optimize - <output.gif>

