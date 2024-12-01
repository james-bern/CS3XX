import pygame_widgets
import pygame
from pygame_widgets.slider import Slider
from pygame_widgets.textbox import TextBox
from math import pi, acos, cos, atan2, sin, radians

pygame.init()
# origin (0, 0) is top left corner
clock = pygame.time.Clock()
size = [512, 512]
screen = pygame.display.set_mode(size)

slider1 = Slider(screen, 60, 50, 100, 8, min=-90, max=90, step=1)
slider2 = Slider(screen, 60, 80, 100, 8, min=-90, max=90, step=1)
output1 = TextBox(screen, 10, 40, 35, 25, fontSize=15)
output2 = TextBox(screen, 10, 70, 35, 25, fontSize=15)
output1.disable()  # Act as label instead of textbox
output2.disable()

def begin_frame():
    pygame.display.flip()
    clock.tick(360)
    for event in pygame.event.get(): 
        if event.type == pygame.QUIT:
            return False
        elif event.type == pygame.MOUSEBUTTONDOWN:
            print(mouse)
    screen.fill("white")
    draw_history(points, black)
    return True

def draw_line(_p0, _p1, color):
    p0 = [256 + _p0[0] , 256 - _p0[1]]
    p1 = [256 + _p1[0], 256 - _p1[1]]
    pygame.draw.line(screen, color, p0, p1, 4)

def draw_point(p, color):
    _p = [256 + p[0], 256 - p[1]]
    pygame.draw.circle(screen, color, _p, 4)

def draw_history(points, color):
    for p in points:
        _p = [256 + p[0], 256 - p[1]]
        pygame.draw.circle(screen, color, _p, 2)

def world_from_pygame(p):
    return [-256 + p[0], 256 - p[1]]



green = (30, 179, 113)
purple = (179, 30, 113)
black = (0, 0, 0)


EVE = [[80, 115], [155, 115], [155, 50], [115, 50], [115, 95], [135, 95], [135, 75], [115, 75], [115, 95], [105, 95], [105, 50], [80, 50], [80, 115], [80, 40],
       [155, 40], [155, 20], [105, 20], [105, 0], [155, 0], [155, -20], [80, -20], [80, 40], [80, -30],
       [155, -30], [155, -95], [115, -95], [115, -50], [135, -50], [135, -70], [115, -70], [115, -50], [105, -50], [105, -95], [80, -95], [80, -30], [80, -95]]

L0 = 100
L1 = 100

name = EVE
wait = 0
line_count = len(name)
p1 = [80, 115]
points = []


while begin_frame():
    events = pygame.event.get()
    mouse = world_from_pygame(pygame.mouse.get_pos())
    
    output1.setText(slider1.getValue())
    output2.setText(slider2.getValue())

    if line_count == 0:
        pass
    elif wait > 0.99:
        wait = 0
        line_count -= 1
    elif wait < 1:
        x = name[len(name) - line_count][0] * wait
        y = name[len(name) - line_count][1] * wait
        x += ((1 - wait) * p1[0])
        y += ((1 - wait) * p1[1])
        cosine = (x**2 + y**2 - L0**2 - L1**2) / (2*L0*L1)
        theta1 = acos(min(max(cosine, -1.0), 1.0))
        theta0 = atan2(y, x) - atan2((L1*sin(theta1)), (L0+L1*cos(theta1)))
        p0 = [L0 * cos(theta0), L0 * sin(theta0)]
        p1 = [p0[0] + L1 * cos(theta0+theta1), p0[1] + L1 * sin(theta0+theta1)]
        points += [p1]
        wait += 0.01

    draw_line([0,0], p0, green)
    draw_line(p0, p1, purple)
    draw_point(p1, black)
    pygame_widgets.update(events)

pygame.quit()