#include <linux/if.h>
#include <linux/if_tun.h>


/* References for following dev: 
* https://backreference.org/2010/03/26/tuntap-interface-tutorial/
* https://www.kernel.org/doc/html/latest/networking/tuntap.html
*/
int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
       return tun_alloc_old(dev);

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IIF_TAP; //TAP- Ethernet frames
    /* If name was given, use it as device name. otherwise, a free name will be allocated */
    if( *dev )
       strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
       close(fd);
       return err;
    }
    strcpy(dev, ifr.ifr_name);
    return fd;
}