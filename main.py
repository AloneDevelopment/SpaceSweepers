import time

import pygame

running = True

screen = pygame.display.set_mode((320, 240), pygame.RESIZABLE)
pygame.display.set_caption("Space Sweepers")


class Player:
    x = 0
    y = 0
    score = 0

player = Player()

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.KEYDOWN:
            done = False
            while not done:
                keys = pygame.key.get_pressed()
                if keys[pygame.K_DOWN]:
                    player.y -= 1
                if keys[pygame.K_UP]:
                    player.y += 1
                if keys[pygame.K_LEFT]:
                    player.x -= 1
                if keys[pygame.K_RIGHT]:
                    player.x += 1
                print(player.x)
                print(player.y)
                time.sleep(0.5)
                for e in pygame.event.get():
                    pass



pygame.quit()
