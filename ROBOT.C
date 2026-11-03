#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include <math.h>
#include <dos.h>
#define MAXP 12
#define L 45
#define R 420
#define T 35
#define B 420
#define FRAMES 18
#define PI 3.14159265
#define STEP_PIX 30.0
int wx[MAXP],wy[MAXP],px[MAXP],py[MAXP];
int count=0,current=0,frameDelay=70;
double scale,ox,oy;
int viewMinX,viewMaxX,viewMinY,viewMaxY,gridStep=1;
int roundPixel(double v)
{
    if(v>=0) return (int)(v+0.5);
    return (int)(v-0.5);
}

int readNumber(char *msg)
{
    char s[8];
    int n=0,ch;
    s[0]='\0';
    for(;;) {
        setfillstyle(SOLID_FILL,BLACK); bar(2,440,638,478);
        setcolor(YELLOW); outtextxy(8,444,msg); outtextxy(8,462,s);
        ch=getch();
        if(ch==13 && n>0) break;
        if(ch==8 && n>0) s[--n]='\0';
        else if(((ch>='0'&&ch<='9')||(ch=='-'&&n==0))&&n<6) {
            s[n++]=ch; s[n]='\0';
        }
    }
    return atoi(s);
}

int yesNo(char *msg)
{
    int ch;
    setfillstyle(SOLID_FILL,BLACK); bar(2,440,638,478);
    setcolor(YELLOW); outtextxy(8,450,msg); ch=getch();
    return ch=='y'||ch=='Y';
}

void worldToScreen(double x,double y,int *sx,int *sy)
{
    *sx=roundPixel(ox+x*scale); *sy=roundPixel(oy-y*scale);
}

void mapPoints(void)
{
    int i,rx,ry;
    double sx,sy;
    viewMinX=viewMaxX=0; viewMinY=viewMaxY=0;
    for(i=0;i<count;i++) {
        if(wx[i]<viewMinX) viewMinX=wx[i];
        if(wx[i]>viewMaxX) viewMaxX=wx[i];
        if(wy[i]<viewMinY) viewMinY=wy[i];
        if(wy[i]>viewMaxY) viewMaxY=wy[i];
    }
    rx=viewMaxX-viewMinX+2; ry=viewMaxY-viewMinY+2;
    sx=(double)(R-L-100)/rx; sy=(double)(B-T-180)/ry;
    scale=sx<sy?sx:sy;
    if(scale>35) scale=35;
    ox=L+50-(viewMinX-1)*scale;
    oy=B-55+(viewMinY-1)*scale;
    gridStep=(rx>ry?rx:ry)/8;
    if(gridStep<1) gridStep=1;
    for(i=0;i<count;i++) worldToScreen(wx[i],wy[i],&px[i],&py[i]);
}

void drawGrid(void)
{
    int x,y,sx,sy,start;
    char s[12];
    setcolor(DARKGRAY); setlinestyle(DOTTED_LINE,0,NORM_WIDTH);
    start=(int)floor((double)viewMinX/gridStep)*gridStep;
    for(x=start;x<=viewMaxX;x+=gridStep) {
        worldToScreen(x,0,&sx,&sy); line(sx,T+5,sx,B-5);
        sprintf(s,"%d",x); outtextxy(sx-5,B-18,s);
    }
    start=(int)floor((double)viewMinY/gridStep)*gridStep;
    for(y=start;y<=viewMaxY;y+=gridStep) {
        worldToScreen(0,y,&sx,&sy); line(L+5,sy,R-5,sy);
        sprintf(s,"%d",y); outtextxy(L+7,sy-5,s);
    }
    setlinestyle(SOLID_LINE,0,NORM_WIDTH);
    setcolor(LIGHTGRAY); worldToScreen(0,0,&sx,&sy);
    line(L+5,sy,R-5,sy); line(sx,T+5,sx,B-5);
    outtextxy(sx+4,sy+4,"0"); outtextxy(R-18,sy+5,"X"); outtextxy(sx+5,T+5,"Y");
}

void drawPath(void)
{
    int i,p[8],x,y;
    double dx,dy,len,nx,ny;
    char s[20];
    for(i=0;i<count-1;i++) {
        dx=px[i+1]-px[i]; dy=py[i+1]-py[i];
        len=sqrt(dx*dx+dy*dy); if(len<1) len=1;
        nx=-dy/len*8; ny=dx/len*8;
        p[0]=px[i]+roundPixel(nx); p[1]=py[i]+roundPixel(ny);
        p[2]=px[i+1]+roundPixel(nx); p[3]=py[i+1]+roundPixel(ny);
        p[4]=px[i+1]-roundPixel(nx); p[5]=py[i+1]-roundPixel(ny);
        p[6]=px[i]-roundPixel(nx); p[7]=py[i]-roundPixel(ny);
        setfillstyle(SOLID_FILL,BROWN); setcolor(BROWN); fillpoly(4,p);
        setcolor(LIGHTGREEN); line(px[i],py[i],px[i+1],py[i+1]);
    }
    for(i=0;i<count;i++) {
        setcolor(LIGHTGRAY); circle(px[i],py[i],8);
        setfillstyle(SOLID_FILL,i==0?LIGHTGREEN:YELLOW);
        fillellipse(px[i],py[i],4,4);
        sprintf(s,"(%d,%d)",wx[i],wy[i]);
        x=px[i]+7; y=py[i]-14;
        if(x>R-55) x=px[i]-48;
        if(y<T) y=py[i]+7;
        setcolor(WHITE); outtextxy(x,y,s);
    }
}

void localPoint(double cx,double cy,double fx,double fy,double nx,double ny,
                double lx,double ly,int *sx,int *sy)
{
    *sx=roundPixel(cx+fx*lx+nx*ly); *sy=roundPixel(cy+fy*lx+ny*ly);
}

void thickLine(int x1,int y1,int x2,int y2,int radius,int col)
{
    double dx,dy,d,nx,ny;
    int p[8];
    dx=x2-x1; dy=y2-y1; d=sqrt(dx*dx+dy*dy);
    if(d<1) d=1;
    nx=-dy*radius/d; ny=dx*radius/d;
    p[0]=x1+roundPixel(nx); p[1]=y1+roundPixel(ny);
    p[2]=x2+roundPixel(nx); p[3]=y2+roundPixel(ny);
    p[4]=x2-roundPixel(nx); p[5]=y2-roundPixel(ny);
    p[6]=x1-roundPixel(nx); p[7]=y1-roundPixel(ny);
    setfillstyle(SOLID_FILL,col); setcolor(col); fillpoly(4,p);
    fillellipse(x1,y1,radius,radius); fillellipse(x2,y2,radius,radius);
}

void joint(double cx,double cy,double fx,double fy,double nx,double ny,
            double lx,double ly,int rad,int col)
{
    int x,y;
    localPoint(cx,cy,fx,fy,nx,ny,lx,ly,&x,&y);
    setfillstyle(SOLID_FILL,col); setcolor(WHITE);
    fillellipse(x,y,rad,rad); setcolor(col); circle(x,y,rad);
}

void solveKnee(double hx,double hy,double ax,double ay,int bend,
               double *kx,double *ky)
{
    double dx,dy,d,a,h;
    dx=ax-hx; dy=ay-hy; d=sqrt(dx*dx+dy*dy);
    if(d<1) d=1;
    if(d>50) { dx=dx*50.0/d; dy=dy*50.0/d; d=50; }
    a=(26.0*26.0-25.0*25.0+d*d)/(2.0*d); h=sqrt(fabs(26.0*26.0-a*a));
    *kx=hx+dx*a/d-bend*dy*h/d;
    *ky=hy+dy*a/d+bend*dx*h/d;
}

void limb(double cx,double cy,double fx,double fy,double nx,double ny,
          double ax,double ay,double bx,double by,int thick,int col)
{
    int x1,y1,x2,y2;
    localPoint(cx,cy,fx,fy,nx,ny,ax,ay,&x1,&y1);
    localPoint(cx,cy,fx,fy,nx,ny,bx,by,&x2,&y2); thickLine(x1,y1,x2,y2,thick,col);
}

void robotSegment(double cx,double cy,double fx,double fy,double nx,double ny,
                  double phase)
{
    double hy,sx,sy,ax,ay,kx,ky,ex,ey,wx,wy,legPhase,swing,lift;
    int poly[8];
    hy=48;
    /* Far arm and far leg */
    sx=-12; sy=77; ex=sx-5+sin(phase+PI)*8; ey=60;
    wx=ex-4+sin(phase+PI)*3; wy=43;
    limb(cx,cy,fx,fy,nx,ny,sx,sy,ex,ey,4,DARKGRAY);
    limb(cx,cy,fx,fy,nx,ny,ex,ey,wx,wy,3,DARKGRAY);
    joint(cx,cy,fx,fy,nx,ny,ex,ey,4,BLUE);
    legPhase=phase+PI; swing=cos(legPhase)*8;
    lift=sin(legPhase)<0?-sin(legPhase)*11:0;
    ax=swing; ay=lift+3; solveKnee(-6,hy,ax,ay,1,&kx,&ky);
    limb(cx,cy,fx,fy,nx,ny,-6,hy,kx,ky,5,DARKGRAY);
    limb(cx,cy,fx,fy,nx,ny,kx,ky,ax,ay,4,DARKGRAY);
    limb(cx,cy,fx,fy,nx,ny,ax-7,ay+1,ax+12,ay+1,4,DARKGRAY);
    joint(cx,cy,fx,fy,nx,ny,kx,ky,4,BLUE);
    /* Near leg and near arm: legs alternate their swing phase */
    swing=cos(phase)*8; lift=sin(phase)<0?-sin(phase)*11:0;
    ax=swing; ay=lift+3; solveKnee(6,hy,ax,ay,1,&kx,&ky);
    limb(cx,cy,fx,fy,nx,ny,6,hy,kx,ky,6,LIGHTGRAY);
    limb(cx,cy,fx,fy,nx,ny,kx,ky,ax,ay,5,LIGHTGRAY);
    limb(cx,cy,fx,fy,nx,ny,ax-7,ay+2,ax+13,ay+2,5,LIGHTGRAY);
    joint(cx,cy,fx,fy,nx,ny,kx,ky,5,LIGHTBLUE);
    joint(cx,cy,fx,fy,nx,ny,ax+11,ay+2,3,CYAN);
    sx=12; sy=77; ex=sx+5+sin(phase)*8; ey=60;
    wx=ex+4+sin(phase)*3; wy=43;
    limb(cx,cy,fx,fy,nx,ny,sx,sy,ex,ey,5,LIGHTGRAY);
    limb(cx,cy,fx,fy,nx,ny,ex,ey,wx,wy,4,LIGHTGRAY);
    joint(cx,cy,fx,fy,nx,ny,ex,ey,4,LIGHTBLUE);
    joint(cx,cy,fx,fy,nx,ny,wx,wy,4,CYAN);
    /* Pelvis and chest armour */
    localPoint(cx,cy,fx,fy,nx,ny,-12,50,&poly[0],&poly[1]);
    localPoint(cx,cy,fx,fy,nx,ny,12,50,&poly[2],&poly[3]);
    localPoint(cx,cy,fx,fy,nx,ny,12,57,&poly[4],&poly[5]);
    localPoint(cx,cy,fx,fy,nx,ny,-12,57,&poly[6],&poly[7]);
    setfillstyle(SOLID_FILL,LIGHTBLUE); setcolor(CYAN); fillpoly(4,poly);
    localPoint(cx,cy,fx,fy,nx,ny,-13,55,&poly[0],&poly[1]);
    localPoint(cx,cy,fx,fy,nx,ny,-11,79,&poly[2],&poly[3]);
    localPoint(cx,cy,fx,fy,nx,ny,11,79,&poly[4],&poly[5]);
    localPoint(cx,cy,fx,fy,nx,ny,13,55,&poly[6],&poly[7]);
    setfillstyle(SOLID_FILL,LIGHTGRAY); setcolor(WHITE); fillpoly(4,poly);
    limb(cx,cy,fx,fy,nx,ny,-7,59,7,59,3,CYAN);
    joint(cx,cy,fx,fy,nx,ny,0,68,6,YELLOW);
    joint(cx,cy,fx,fy,nx,ny,-13,77,5,LIGHTBLUE);
    joint(cx,cy,fx,fy,nx,ny,13,77,5,LIGHTBLUE);
    /* Neck, helmet, visor and antenna */
    limb(cx,cy,fx,fy,nx,ny,0,79,0,85,4,LIGHTBLUE);
    localPoint(cx,cy,fx,fy,nx,ny,0,99,&poly[0],&poly[1]);
    setfillstyle(SOLID_FILL,LIGHTGRAY); setcolor(WHITE);
    fillellipse(poly[0],poly[1],12,13);
    localPoint(cx,cy,fx,fy,nx,ny,5,100,&poly[0],&poly[1]);
    setfillstyle(SOLID_FILL,BLUE); setcolor(CYAN);
    fillellipse(poly[0],poly[1],6,4);
    joint(cx,cy,fx,fy,nx,ny,8,100,2,YELLOW);
    limb(cx,cy,fx,fy,nx,ny,-3,110,-3,116,1,LIGHTGRAY);
    joint(cx,cy,fx,fy,nx,ny,-3,117,2,RED);
}

void drawScreen(int x,int y,double phase,double fx,double fy,
                double nx,double ny,int cx,int cy)
{
    char s[40];
    cleardevice();
    setcolor(CYAN); outtextxy(100,8,"ARTICULATED COORDINATE ROBOT");
    drawGrid(); drawPath();
    setcolor(BLUE); rectangle(435,35,635,420);
    setcolor(CYAN); outtextxy(455,48,"ROBOT CONTROL");
    setcolor(WHITE);
    sprintf(s,"WAYPOINT: %d / %d",current,count-1); outtextxy(450,88,s);
    sprintf(s,"POSITION X: %d",cx); outtextxy(450,112,s);
    sprintf(s,"POSITION Y: %d",cy); outtextxy(450,132,s);
    sprintf(s,"DELAY: %d ms",frameDelay); outtextxy(450,154,s);
    outtextxy(450,200,"MENU");
    outtextxy(450,222,"1 New route"); outtextxy(450,242,"2 Walk again");
    outtextxy(450,262,"3 Return home"); outtextxy(450,282,"4 Set speed");
    outtextxy(450,302,"5 Exit"); outtextxy(450,350,"P = Pause");
    robotSegment((double)x,(double)y,fx,fy,nx,ny,phase);
}

void animate(int back)
{
    int i,k,f,a,b,x,y,ch,cx,cy,steps;
    double dx,dy,len,fx,fy,nx,ny,t,pos,phase;
    if(count<2) return;
    if(back) { if(current==0) return; }
    else current=0;
    for(i=0;i<count-1;i++) {
        if(back) { a=count-1-i; b=a-1; }
        else { a=i; b=i+1; }
        dx=px[b]-px[a]; dy=py[b]-py[a];
        len=sqrt(dx*dx+dy*dy); if(len<1) continue;
        fx=dx/len; fy=dy/len;
        nx=fy; ny=-fx;
        if(ny>0) { nx=-nx; ny=-ny; }
        steps=(int)ceil(len/STEP_PIX); if(steps<1) steps=1;
        for(k=0;k<steps;k++) {
            for(f=0;f<=FRAMES;f++) {
                t=(double)f/FRAMES;
                pos=((double)k+t)/steps;
                x=px[a]+roundPixel(dx*pos); y=py[a]+roundPixel(dy*pos);
                cx=wx[a]+roundPixel((wx[b]-wx[a])*pos); cy=wy[a]+roundPixel((wy[b]-wy[a])*pos);
                phase=2.0*PI*t;
                current=back?b:a;
                drawScreen(x,y,phase,fx,fy,nx,ny,cx,cy);
                delay(frameDelay);
                if(kbhit()) {
                    ch=getch();
                    if(ch=='p'||ch=='P') {
                        setcolor(YELLOW); outtextxy(460,390,"PAUSED - press key");
                        getch();
                    }
                }
            }
        }
        current=b;
    }
    current=back?0:count-1;
}

void setSpeed(void)
{
    int ch;
    setfillstyle(SOLID_FILL,BLACK); bar(2,440,638,478);
    setcolor(YELLOW); outtextxy(8,444,"1 Slow  2 Normal  3 Fast");
    ch=getch();
    if(ch=='1') frameDelay=150;
    else if(ch=='2') frameDelay=70;
    else if(ch=='3') frameDelay=25;
    setfillstyle(SOLID_FILL,BLACK); bar(2,440,638,478);
    setcolor(LIGHTGREEN); outtextxy(8,450,"Speed updated - next walk uses new speed."); delay(500);
}

void newRoute(void)
{
    int x,y;
    count=1; current=0; wx[0]=0; wy[0]=0;
    while(count<MAXP) {
        x=readNumber("Destination X (-99 to 99):");
        y=readNumber("Destination Y (-99 to 99):");
        if(x>99||x< -99||y>99||y< -99) {
            setfillstyle(SOLID_FILL,BLACK); bar(2,440,638,478);
            setcolor(RED); outtextxy(8,450,"Use coordinates between -99 and 99.");
            getch(); continue;
        }
        wx[count]=x; wy[count]=y; count++;
        if(count==MAXP||!yesNo("Add another coordinate? Y/N")) break;
    }
    mapPoints(); animate(0);
}

int main(void)
{
    int gd=DETECT,gm,ch;
    initgraph(&gd,&gm,"C:\\TURBOC3\\BGI");
    if(graphresult()!=grOk) { printf("BGI initialisation failed. Check BGI path.\n"); return 1; }
    setbkcolor(BLACK);
    for(;;) {
        if(count>1) drawScreen(px[current],py[current],0,1,0,0,-1,wx[current],wy[current]);
        else {
            cleardevice(); setcolor(CYAN);
            outtextxy(160,120,"ARTICULATED HUMANOID ROBOT");
            outtextxy(150,175,"1. Create coordinate route");
            outtextxy(150,205,"4. Set walking speed");
            outtextxy(150,235,"5. Exit");
        }
        ch=getch();
        if(ch=='1') newRoute();
        else if(ch=='2'&&count>1) animate(0);
        else if(ch=='3'&&count>1) animate(1);
        else if(ch=='4') setSpeed();
        else if(ch=='5') break;
    }
    closegraph(); return 0;
}
