import os, time, pygame
rate = 48000
buf  = int(os.environ.get("VM_BUFFER","4096"))
pygame.mixer.pre_init(rate, -16, 2, buf)
pygame.init()
pygame.mixer.set_num_channels(16)
snd = pygame.mixer.Sound("click.wav")
for i in range(300):
    snd.play()
    time.sleep(0.03)
print("Done at", rate, "Hz buffer", buf)
