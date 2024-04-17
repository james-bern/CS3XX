from math import *

##################################################
# PYGAME #########################################
##################################################

import pygame

pygame.init()
clock = pygame.time.Clock()

def begin_frame():
    pygame.display.flip()
    clock.tick(60)
    for event in pygame.event.get(): 
        if event.type == pygame.QUIT:
            return False
    screen.fill("white")
    return True


##################################################
# GRAPHICS #######################################
##################################################

BLACK = (  0,   0,   0)
RED   = (255,   0,   0)
GREEN = (  0, 255,   0)
BLUE  = (  0,   0, 255)

screen = pygame.display.set_mode([512, 512])

def pygame_from_world(p_world):
    return [256 + p_world[0], 256 - p_world[1]]

def world_from_pygame(p_pygame):
    return [-256 + p_pygame[0], 256 - p_pygame[1]]

def draw_line(a_world, b_world, color = BLACK):
    pygame.draw.line(screen, color, pygame_from_world(a_world), pygame_from_world(b_world), 5)

def draw_point(p_world, color = BLACK):
    pygame.draw.circle(screen, color, pygame_from_world(p_world), 8)


##################################################
# APP ############################################
##################################################

L1 = 100.0
L2 = 100.0
while begin_frame():
    mouse = world_from_pygame(pygame.mouse.get_pos())
    theta1 = radians(mouse[0])
    theta2 = radians(mouse[1])
    p1 = [L1 * cos(theta1), L1 * sin(theta1)]
    p2 = [p1[0] + L2 * cos(theta1 + theta2), p1[1] + L2 * sin(theta1 + theta2)]
    draw_line([0, 0], p1, RED)
    draw_line(p1, p2, BLUE)
    draw_point(mouse, [128 + 128 * cos(time), 0, 0)

