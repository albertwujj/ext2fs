#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

struct partition {
    u8 drive;             /* drive number FD=0, HD=0x80, etc. */

    u8  head;             /* starting head */
    u8  sector;           /* starting sector */
    u8  cylinder;         /* starting cylinder */

    u8  sys_type;         /* partition type: NTFS, LINUX, etc. */

    u8  end_head;         /* end head */
    u8  end_sector;       /* end sector */
    u8  end_cylinder;     /* end cylinder */

    u32 start_sector;     /* starting sector counting from 0 */
    u32 nr_sectors;       /* number of of sectors in partition */
};

int getsector(int fd, int sector, char buf[]) {
    lseek(fd, (long) (sector*512), SEEK_SET);
    read(fd, buf, 512);
}

int main(void) {
    int fd = open("vdisk", 0);
    char buf[512];
    getsector(fd, 0, buf);
    struct partition *p = &buf[0x1BE];
    int i = 0;
    int disk_num = 1;
    for(i=0; i<4; i++) {
        if (p->sys_type == 5) {
            printf("vdisk%d, start_sector: %d, type: %x\n",i,p->start_sector, p->sys_type);
            struct partition *curr;
            int base = p->start_sector;
            int offset = 0;
            int prev_partition_end = base;
            do {
                disk_num++;
                getsector(fd, base + offset, buf);
                curr = &buf[0x1BE];
                printf("vdisk%d, start_sector: %d, type: %x\n",disk_num, prev_partition_end + curr->start_sector, curr->sys_type);
                prev_partition_end = prev_partition_end + curr->start_sector + curr->nr_sectors;
                offset = (curr + 1)->start_sector;
            } while (offset != 0);
        }
        else {
            printf("vdisk%d, start_sector: %d, type: %x\n",i, p->start_sector, p->sys_type);
        }
        disk_num++;
        p++;
    }
}
