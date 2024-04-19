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
L3 = 50.0

def FK(theta):
    p0 = [L0 * cos(theta[0]), L0 * sin(theta[0])]
    p1 = [p0[0] + L1 * cos(theta[0] + theta[1]), p0[1] + L1 * sin(theta[0] + theta[1])]
    p2 = [p1[0] + (L2 + theta[4]) * cos(theta[0] + theta[1] + theta[2]), p1[1] + (L2 + theta[4]) * sin(theta[0] + theta[1] + theta[2])]
    p3 = [p2[0] + L3 * cos(theta[0] + theta[1] + theta[2] + theta[3]), p2[1] + L3 * sin(theta[0] + theta[1] + theta[2] + theta[3])]
    return p0, p1, p2, p3


def objective(theta):
    # squared length of the line connecting robot-tip and target
    _, _, _, tip = FK(theta)
    squared_length = ((tip[0] - target[0]) ** 2 + (tip[1] - target[1]) ** 2)
    regularizer = 0.01 * theta[4] * theta[4]
    return squared_length + regularizer
    


theta = [ 0.0, 0.0, 0.0, 0.0, 0.0 ]
frame = 0
while begin_frame():
    frame_angle = frame / 20.0
    target = [150 + 100 * cos(frame_angle), 150 + 50 * sin(frame_angle)]
    # target = targets[frame_index % len(targets)]
    frame += 1

    # NOTE: freaky scipy casting np.array <--> list
    result = minimize(objective, np.array(theta), method = 'L-BFGS-B')
    theta = list(result['x'])

    p0, p1, p2, p3 = FK(theta)
    draw_line([0, 0], p0, BLUE)
    draw_line(p0, p1, BLUE)
    draw_line(p1, p2, BLUE)
    draw_line(p2, p3, BLUE)
    draw_point(p0)
    draw_point(p1)
    draw_point(p2)
    draw_point(target, GREEN)




