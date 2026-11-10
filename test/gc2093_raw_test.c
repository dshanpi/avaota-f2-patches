#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <linux/videodev2.h>

static int xi(int f, unsigned long op, void *p) { int r; do {r=ioctl(f,op,p);}while(r<0 && errno==EINTR); return r; }
#define CALL(op,p) do { if(xi(fd,op,p)<0){perror(#op);goto done;} }while(0)
int main(int argc,char **argv) {
 int fd=-1,rc=1,stream=0,input=0,want=argc>1?atoi(argv[1]):300,n=0,errors=0,gaps=0;
 void *mem[4]={0}; size_t len[4]={0}; unsigned prev=0; double first=0,last=0;
 enum v4l2_buf_type type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
 struct v4l2_requestbuffers req={.count=3,.type=type,.memory=V4L2_MEMORY_MMAP};
 struct v4l2_streamparm sp={.type=type}; struct v4l2_format fmt={.type=type};
 setvbuf(stdout,NULL,_IOLBF,0);
 fd=open("/dev/video0",O_RDWR|O_NONBLOCK); if(fd<0){perror("open");return 1;}
 CALL(VIDIOC_S_INPUT,&input);
 sp.parm.capture.capturemode=2; sp.parm.capture.timeperframe.numerator=1;sp.parm.capture.timeperframe.denominator=30;
 CALL(VIDIOC_S_PARM,&sp);
 fmt.fmt.pix_mp.width=1920;fmt.fmt.pix_mp.height=1080;fmt.fmt.pix_mp.pixelformat=V4L2_PIX_FMT_SBGGR10;fmt.fmt.pix_mp.field=V4L2_FIELD_NONE;
 CALL(VIDIOC_S_FMT,&fmt);CALL(VIDIOC_G_FMT,&fmt);
 printf("FORMAT %ux%u fourcc=%c%c%c%c planes=%u stride=%u size=%u\n",fmt.fmt.pix_mp.width,fmt.fmt.pix_mp.height,fmt.fmt.pix_mp.pixelformat&255,(fmt.fmt.pix_mp.pixelformat>>8)&255,(fmt.fmt.pix_mp.pixelformat>>16)&255,(fmt.fmt.pix_mp.pixelformat>>24)&255,fmt.fmt.pix_mp.num_planes,fmt.fmt.pix_mp.plane_fmt[0].bytesperline,fmt.fmt.pix_mp.plane_fmt[0].sizeimage);
 if(fmt.fmt.pix_mp.num_planes!=1){fprintf(stderr,"Expected one RAW plane\n");goto done;}
 CALL(VIDIOC_REQBUFS,&req);if(req.count>4||req.count<2)goto done;
 for(unsigned i=0;i<req.count;i++) {struct v4l2_plane p[VIDEO_MAX_PLANES]={0};struct v4l2_buffer b={.type=type,.memory=V4L2_MEMORY_MMAP,.index=i,.length=VIDEO_MAX_PLANES,.m.planes=p};
 CALL(VIDIOC_QUERYBUF,&b);len[i]=p[0].length;mem[i]=mmap(NULL,len[i],PROT_READ|PROT_WRITE,MAP_SHARED,fd,p[0].m.mem_offset);if(mem[i]==MAP_FAILED){mem[i]=NULL;perror("mmap");goto done;}CALL(VIDIOC_QBUF,&b);}
 CALL(VIDIOC_STREAMON,&type);stream=1;
 for(n=0;n<want;n++) {fd_set s;FD_ZERO(&s);FD_SET(fd,&s);struct timeval tv={.tv_sec=5};int r=select(fd+1,&s,NULL,NULL,&tv);if(r<=0){fprintf(stderr,"FRAME_TIMEOUT frame=%d rc=%d\n",n,r);goto done;}
 struct v4l2_plane p[VIDEO_MAX_PLANES]={0};struct v4l2_buffer b={.type=type,.memory=V4L2_MEMORY_MMAP,.length=VIDEO_MAX_PLANES,.m.planes=p};CALL(VIDIOC_DQBUF,&b);
 if(b.index>=req.count||p[0].bytesused>len[b.index])goto done;
 double t=b.timestamp.tv_sec+b.timestamp.tv_usec*1e-6;if(n==0)first=t;else if(b.sequence>prev+1)gaps+=b.sequence-prev-1;last=t;prev=b.sequence;
 if(b.flags&V4L2_BUF_FLAG_ERROR)errors++;
 if(n==10 && argc>2){FILE *o=fopen(argv[2],"wb");if(!o){perror("save");goto done;}size_t z=fwrite(mem[b.index],1,p[0].bytesused,o);fclose(o);if(z!=p[0].bytesused)goto done;printf("SAVED %s bytes=%u\n",argv[2],p[0].bytesused);}
 if(n%30==0)printf("FRAME %d sequence=%u timestamp=%.6f bytes=%u flags=0x%x\n",n,b.sequence,t,p[0].bytesused,b.flags);
 CALL(VIDIOC_QBUF,&b);}
 printf("RESULT frames=%d elapsed=%.6f fps=%.4f error_flags=%d sequence_gaps=%d\n",n,last-first,last>first?(n-1)/(last-first):0,errors,gaps);rc=errors||gaps?2:0;
 done:if(stream && xi(fd,VIDIOC_STREAMOFF,&type)<0){perror("STREAMOFF");rc=1;}for(int i=0;i<4;i++)if(mem[i])munmap(mem[i],len[i]);if(fd>=0)close(fd);return rc;
}
