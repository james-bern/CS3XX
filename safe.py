from math import *
import numpy as np
from scipy.optimize import minimize

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

L0 = 50.0
L1 = 50.0
L2 = 50.0
theta = [ 0.0, 0.0, 0.0, 0.0 ]

def FK(theta):
    p1 = [L0 * cos(theta[0]), L0 * sin(theta[0])]
    p2 = [p1[0] + (L1 + theta[3]) * cos(theta[0] + theta[1]), p1[1] + (L1 + theta[3]) * sin(theta[0] + theta[1])]
    p3 = [p2[0] + L2 * cos(theta[0] + theta[1] + theta[2]), p2[1] + L2 * sin(theta[0] + theta[1] + theta[2])]
    return p1, p2, p3

def objective(theta):
    target = world_from_pygame(pygame.mouse.get_pos())
    _, _, p3 = FK(theta)
    dx = p3[0] - target[0]
    dy = p3[1] - target[1]
    return dx * dx + dy * dy + 0.1 * theta[3] * theta[3]

while begin_frame():
    mouse = world_from_pygame(pygame.mouse.get_pos())

    result = minimize(objective, np.array(theta), method = 'L-BFGS-B')
    theta = list(result['x'])

    p0 = [0.0, 0.0 ]
    p1, p2, p3 = FK(theta)
    draw_line(p0, p1, RED)
    draw_line(p1, p2, BLACK)
    draw_line(p2, p3, RED)
    draw_point(p0, BLUE)
    draw_point(p1, BLUE)
    draw_point(p2, BLUE)
    draw_point(mouse, GREEN)

