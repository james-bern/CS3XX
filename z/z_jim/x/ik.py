import pygame
from math import *

# Initialize pygame
pygame.init()

# Set the height and width of the screen
size = [512, 512]
screen = pygame.display.set_mode(size)

pygame.display.set_caption("Example code for the draw module")

done = False
clock = pygame.time.Clock()

theta_1 = 0
theta_2 = 0
L_1 = 100
L_2 = 100

while not done:
    clock.tick(60)
    for event in pygame.event.get():
        if event.type == pygame.QUIT:  # If user clicked close
            done = True  # Flag that we are done so we exit this loop

    screen.fill("white")

    theta_1, theta_2 = [-pi + pi * x / 256.0 for x in pygame.mouse.get_pos()]
    
    x_0 = 256
    y_0 = 256

    x_1 = x_0 + L_1 * cos(theta_1)
    y_1 = y_0 + L_1 * sin(theta_1)

    x_2 = x_1 + L_2 * cos(theta_1 + theta_2)
    y_2 = y_1 + L_2 * sin(theta_1 + theta_2)

    pygame.draw.line(screen, (60, 179, 113), [x_0, y_0], [x_1, y_1], 5)
    pygame.draw.line(screen, (179, 60, 113), [x_1, y_1], [x_2, y_2], 5)

    pygame.display.flip()

pygame.quit()
