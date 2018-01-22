#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/input.h>
#include <linux/soundcard.h>

#define MIXER_DEVICE "/dev/mixer"

int mixer = -1;

void daemonize(char *dir)
{
  if (fork() != 0)
    exit(0);
  setsid();
  if (fork() != 0)
    exit(0);
  chdir(dir);
  umask(0);
  close(0);
  close(1);
  close(2);
  /*STDIN*/
  open("/dev/null", O_RDONLY);
  /*STDOUT*/
  open("/dev/null", O_WRONLY);
  /*STDERR*/
  open("/dev/null", O_WRONLY);
}

void setVolume(int level)
{ 

  int oss_volume = level | (level << 8); // set volume for both channels

  if (ioctl(mixer, SOUND_MIXER_WRITE_VOLUME, &oss_volume) == -1)
  {
    fprintf(stderr, "Failed opening mixer for write - VOLUME\n");
  }
}

int getVolume()
{
  if (mixer == -1)
  {
    fprintf(stderr, "Key mixer not open\n");
    return -1;
  }

  int outvol = 0;
  if (ioctl(mixer, SOUND_MIXER_READ_VOLUME, &outvol))
  {
    fprintf(stderr, "Failed opening mixer for read - VOLUME\n");
    return -1;
  }
  return outvol & 0xff;
}

void setDmenuVol(int vol)
{  

  char cmd[128];
  sprintf(cmd, "sed -i 's/SndVol =.*/SndVol = %i/g' /mnt/mmc/dmenu/dmenu.ini", vol);
  system(cmd);
}

void increaseVol() {
  int vol = getVolume();
  vol = vol + 10;

  if(vol > 100) {
    vol = 100;
  }

  setDmenuVol(vol);
  setVolume(vol);
}

void decreaseVol() {
  int vol = getVolume();
  vol = vol - 10;
  if(vol < 0) {
    vol = 0;
  }

  setDmenuVol(vol);
  setVolume(vol);
}

int main()
{
  mixer = open(MIXER_DEVICE, O_WRONLY);
  char devname[] = "/dev/event0";
  int device = open(devname, O_RDONLY);
  struct input_event ev;

  //daemonize("/");

  while (1)
  {
    read(device, &ev, sizeof(ev));
    if (ev.type == 1 && ev.value == 1)
    {

      if(ev.code == 3) {
        increaseVol();
      } else if(ev.code == 2) {
        decreaseVol();
      }
      
      int vol = getVolume();
      printf("Key: %i State: %i volume is %d\n", ev.code, ev.value, vol);
    }
  }
  close(mixer);
}
