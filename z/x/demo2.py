
from math import *
import random

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

SCREEN_SIZE = [1024, 800]
screen = pygame.display.set_mode([SCREEN_SIZE[0], SCREEN_SIZE[1]])

def pygame_from_world(p_world):
    return [SCREEN_SIZE[0]/2 + p_world[0], SCREEN_SIZE[1]/2 - p_world[1]]

def world_from_pygame(p_pygame):
    return [-SCREEN_SIZE[0]/2 + p_pygame[0], SCREEN_SIZE[1]/2 - p_pygame[1]]

def draw_line(a_world, b_world, color = BLACK):
    pygame.draw.line(screen, color, pygame_from_world(a_world), pygame_from_world(b_world), 5)

def draw_point(p_world, color = BLACK):
    pygame.draw.circle(screen, color, pygame_from_world(p_world), 8)


##################################################
# HELPERS ########################################
##################################################

def clamp(x, A, B):
    return min(max(x, A), B)


##################################################
# APP ############################################
##################################################

# TODO: review: Newton's second law F = m d2xdt2
# TODO: review: limit definition of the derivative
# TODO: review: physics update
# TODO: modeling the system
# TODO: actuator bounds
# TODO: noisy sensor # + 5 * random.randint(-1, 1)
# ----
# TODO: weight of spaceship changing as fuel is burned

timestep = 0.1
spaceship_mass = 2
gravitational_constant = 10
current_position = 350
current_velocity = 0
target_position = 100 # target

controller_proportional_gain = 5.0
controller_derivative_gain = 0.0

time_wind = 0

error = 0.0
while begin_frame():

    previous_error = error
    error = (target_position - current_position)
    derivative_of_error = (error - previous_error) / timestep

    thruster_force = controller_proportional_gain * error \
            + controller_derivative_gain * derivative_of_error

    thruster_force = clamp(thruster_force, 0, 100)

    # F = ma
    current_acceleration = -gravitational_constant + thruster_force / spaceship_mass
    # semi-implicit euler
    current_velocity += timestep * (current_acceleration)
    current_position += timestep * current_velocity

    draw_line([-1000, 0], [1000, 0])
    draw_line([-100, target_position], [100, target_position], GREEN)
    draw_line([0, current_position], [0, current_position - thruster_force], RED)
    draw_point([0, current_position], BLUE)

