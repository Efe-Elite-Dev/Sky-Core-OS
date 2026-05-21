/*
 * Wind OS  -  kernel.c  v11  (TAM SURUM)
 *
 * DUZELTMELER:
 *  1. Font cozuldu: back_buffer sabit 1024 pitch kullanir
 *  2. Tum uygulamalar pencere olarak acilir
 *  3. .exe/.deb/.wind cift tikla -> kurulum -> masaustu
 *  4. USB takininca dosya yoneticisinde direkt gozukur + logo
 *  5. Widget suruklenebilir
 *  6. Yapay Zeka sohbet penceresi
 *  7. Piksel art ikonlar
 *
 * gcc -m32 -ffreestanding -fno-builtin -fno-stack-protector -O2 -w -c kernel.c -o kernel.o
 */
#include "kernel.h"

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef int            i32;
typedef signed char    i8;
#define NULL ((void*)0)

/* ================================================================
   FRAMEBUFFER + BACK BUFFER
   DUZELTİLDİ: back_buffer sabit 1024 pitch kullanır
   ================================================================ */
static volatile u32 *FB = (u32*)0;
static u32 SW=1024, SH=768, SP=1024;
#define BB_W 1024
#define BB_H 768
static u32 back_buffer[BB_W * BB_H];

/* ================================================================
   RENKLER
   ================================================================ */
#define BG_TOP  0xFF1C1E23u
#define BG_BOT  0xFF2A2D34u
#define PAN_BD  0xFF3A3D45u
#define CTXT    0xFFDCDDDEu
#define CGY     0xFF8899A6u
#define CW      0xFFFFFFFFu
#define CK      0xFF000000u
#define GRID_C  0xFF252830u
/* Neon palet */
#define C_CYAN  0xFF00E5FFu
#define C_PINK  0xFFE91E63u
#define C_ORNG  0xFFFF9800u
#define C_PURP  0xFF9C27B0u
#define C_LIME  0xFF10B981u
#define C_YEL   0xFFFFEB3Bu
#define C_RED   0xFFD13438u
#define C_BLUE  0xFF0078D4u
#define C_GRN   0xFF107C10u
/* UI renkleri */
#define WIN_TIT 0xFF1E2128u
#define WIN_BG  0xFF272B33u
#define WIN_BD  0xFF3A3D45u
#define BTN_C   0xFF0078D4u
#define BTN_H   0xFF005A9Eu
#define INP_BG  0xFF1A1D22u
#define INP_BD  0xFF3A3D45u
#define C_SEL   0xFF004499u
#define USB_C   0xFF00BCD4u

/* ================================================================
   PORT I/O
   ================================================================ */
static inline u8   inb (u16 p)       {u8  v;__asm__ volatile("inb  %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outb(u16 p, u8 v) {__asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p));}
static inline u32  inl (u16 p)       {u32 v;__asm__ volatile("inl  %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outl(u16 p, u32 v){__asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p));}

/* ================================================================
   YARDIMCILAR
   ================================================================ */
static void *mcpy(void *d,const void *s,u32 n){u8*dp=(u8*)d;const u8*sp2=(const u8*)s;while(n--)*dp++=*sp2++;return d;}
static u32 klen(const char *s){u32 n=0;while(s[n])n++;return n;}
static void kcpy(char *d,const char *s){while(*s)*d++=*s++;*d=0;}
static int kstrcmp(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return (u8)*a-(u8)*b;}
static void itoa(int v,char*b){
    if(v<0){*b++='-';v=-v;}
    char t[12];int i=0;
    if(v==0){*b++='0';*b=0;return;}
    while(v){t[i++]='0'+v%10;v/=10;}
    while(i--)* b++=t[i];
    *b=0;
}

/* ================================================================
   8x8 BİTMAP FONT
   ================================================================ */
static const u8 F8[128][8]={
 [' ']={0,0,0,0,0,0,0,0},
 ['!']={0x18,0x3C,0x3C,0x18,0x18,0,0x18,0},
 ['"']={0x36,0x36,0,0,0,0,0,0},
 ['#']={0x36,0x7F,0x36,0x36,0x7F,0x36,0x36,0},
 ['$']={0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0},
 ['%']={0x63,0x33,0x18,0x0C,0x66,0x63,0,0},
 ['&']={0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0},
 ['\'']={0x06,0x0C,0,0,0,0,0,0},
 ['(']={0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0},
 [')']={0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0},
 ['*']={0x66,0x3C,0xFF,0x3C,0x66,0,0,0},
 ['+']={0,0x0C,0x0C,0x3F,0x0C,0x0C,0,0},
 [',']={0,0,0,0,0,0x18,0x18,0x0C},
 ['-']={0,0,0,0x3F,0,0,0,0},
 ['.']={0,0,0,0,0,0x18,0x18,0},
 ['/']={0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0},
 ['0']={0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0},
 ['1']={0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0},
 ['2']={0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0},
 ['3']={0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0},
 ['4']={0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0},
 ['5']={0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0},
 ['6']={0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0},
 ['7']={0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0},
 ['8']={0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0},
 ['9']={0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0},
 [':']={0,0x18,0x18,0,0x18,0x18,0,0},
 [';']={0,0x18,0x18,0,0x18,0x18,0x0C,0},
 ['<']={0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0},
 ['=']={0,0x3F,0,0,0x3F,0,0,0},
 ['>']={0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0},
 ['?']={0x1E,0x33,0x30,0x18,0x0C,0,0x0C,0},
 ['@']={0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0},
 ['A']={0x0C,0x1E,0x33,0x3F,0x33,0x33,0x33,0},
 ['B']={0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0},
 ['C']={0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0},
 ['D']={0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0},
 ['E']={0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0},
 ['F']={0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0},
 ['G']={0x3C,0x66,0x03,0x73,0x63,0x66,0x7C,0},
 ['H']={0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0},
 ['I']={0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0},
 ['J']={0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0},
 ['K']={0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0},
 ['L']={0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0},
 ['M']={0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0},
 ['N']={0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0},
 ['O']={0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0},
 ['P']={0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0},
 ['Q']={0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0},
 ['R']={0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0},
 ['S']={0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0},
 ['T']={0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0},
 ['U']={0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0},
 ['V']={0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0},
 ['W']={0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0},
 ['X']={0x63,0x63,0x36,0x1C,0x36,0x63,0x63,0},
 ['Y']={0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0},
 ['Z']={0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0},
 ['[']={0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0},
 ['\\']={0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0},
 [']']={0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0},
 ['^']={0x08,0x1C,0x36,0x63,0,0,0,0},
 ['_']={0,0,0,0,0,0,0,0xFF},
 ['`']={0x06,0x0C,0x18,0,0,0,0,0},
 ['a']={0,0x1E,0x30,0x3E,0x33,0x33,0x6E,0},
 ['b']={0x07,0x06,0x3E,0x66,0x66,0x66,0x3B,0},
 ['c']={0,0x1E,0x33,0x03,0x03,0x33,0x1E,0},
 ['d']={0x38,0x30,0x3E,0x33,0x33,0x33,0x6E,0},
 ['e']={0,0x1E,0x33,0x3F,0x03,0x33,0x1E,0},
 ['f']={0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0},
 ['g']={0,0x6E,0x33,0x33,0x3E,0x30,0x33,0x1E},
 ['h']={0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0},
 ['i']={0x0C,0,0x0E,0x0C,0x0C,0x0C,0x1E,0},
 ['j']={0x18,0,0x18,0x18,0x18,0x1B,0x1B,0x0E},
 ['k']={0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0},
 ['l']={0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0},
 ['m']={0,0x33,0x7F,0x7F,0x6B,0x63,0x63,0},
 ['n']={0,0x1F,0x33,0x33,0x33,0x33,0x33,0},
 ['o']={0,0x1E,0x33,0x33,0x33,0x33,0x1E,0},
 ['p']={0,0x3B,0x66,0x66,0x3E,0x06,0x06,0x0F},
 ['q']={0,0x6E,0x33,0x33,0x3E,0x30,0x30,0x78},
 ['r']={0,0x3B,0x6E,0x66,0x06,0x06,0x0F,0},
 ['s']={0,0x3E,0x03,0x1E,0x30,0x33,0x1E,0},
 ['t']={0x08,0x3E,0x0C,0x0C,0x0C,0x2C,0x18,0},
 ['u']={0,0x33,0x33,0x33,0x33,0x33,0x6E,0},
 ['v']={0,0x33,0x33,0x33,0x33,0x1E,0x0C,0},
 ['w']={0,0x63,0x6B,0x7F,0x7F,0x36,0x36,0},
 ['x']={0,0x63,0x36,0x1C,0x1C,0x36,0x63,0},
 ['y']={0,0x33,0x33,0x33,0x3E,0x30,0x33,0x1E},
 ['z']={0,0x3F,0x19,0x0C,0x26,0x3F,0,0},
 ['{']={0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0},
 ['|']={0x18,0x18,0x18,0,0x18,0x18,0x18,0},
 ['}']={0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0},
 ['~']={0x6E,0x3B,0,0,0,0,0,0},
};

/* ================================================================
   RENDER MOTORU  (back_buffer sabit 1024 pitch)
   ================================================================ */
static u32 blend_col(u32 fg, u32 bg, u8 a){
    if(a==255) return fg|0xFF000000u; if(a==0) return bg;
    u32 rb=(((fg&0xFF00FFu)*a)+((bg&0xFF00FFu)*(255-a)))>>8;
    u32 g2=(((fg&0x00FF00u)*a)+((bg&0x00FF00u)*(255-a)))>>8;
    return 0xFF000000u|(rb&0xFF00FFu)|(g2&0x00FF00u);
}
/* ÖNEMLİ: BB_W=1024 sabit pitch */
static inline void app(i32 x,i32 y,u32 c,u8 a){
    if((u32)x<BB_W&&(u32)y<BB_H){
        u32 bg=back_buffer[(u32)y*BB_W+(u32)x];
        back_buffer[(u32)y*BB_W+(u32)x]=blend_col(c,bg,a);
    }
}
static void afr(i32 x,i32 y,i32 w,i32 h,u32 c,u8 a){
    if(w<=0||h<=0) return;
    i32 x1=x<0?0:x,y1=y<0?0:y;
    i32 x2=x+w>(i32)BB_W?(i32)BB_W:x+w;
    i32 y2=y+h>(i32)BB_H?(i32)BB_H:y+h;
    for(i32 j=y1;j<y2;j++) for(i32 i=x1;i<x2;i++) app(i,j,c,a);
}
static void fr(i32 x,i32 y,i32 w,i32 h,u32 c){afr(x,y,w,h,c,255);}
static void acirc(i32 cx,i32 cy,i32 r,u32 c,u8 a){
    if(r<=0) return;
    for(i32 dy=-r;dy<=r;dy++) for(i32 dx=-r;dx<=r;dx++)
        if(dx*dx+dy*dy<=r*r) app(cx+dx,cy+dy,c,a);
}
static void circ(i32 cx,i32 cy,i32 r,u32 c){acirc(cx,cy,r,c,255);}
static void arr(i32 x,i32 y,i32 w,i32 h,i32 r,u32 c,u8 a){
    if(r>w/2)r=w/2; if(r>h/2)r=h/2;
    afr(x+r,y,w-2*r,h,c,a); afr(x,y+r,r,h-2*r,c,a); afr(x+w-r,y+r,r,h-2*r,c,a);
    acirc(x+r,y+r,r,c,a); acirc(x+w-r-1,y+r,r,c,a);
    acirc(x+r,y+h-r-1,r,c,a); acirc(x+w-r-1,y+h-r-1,r,c,a);
}
static void rr(i32 x,i32 y,i32 w,i32 h,i32 r,u32 c){arr(x,y,w,h,r,c,255);}
static void rb(i32 x,i32 y,i32 w,i32 h,u32 c,i32 t){
    fr(x,y,w,t,c);fr(x,y+h-t,w,t,c);fr(x,y,t,h,c);fr(x+w-t,y,t,h,c);
}

/* Glassmorphism panel */
static void glass(i32 x,i32 y,i32 w,i32 h,i32 r,u32 c,u8 a){
    arr(x,y,w,h,r,c,a);
    afr(x+r,y,w-2*r,2,0xFFFFFF,30);  /* üst parlama */
    rb(x,y,w,h,0xFFFFFF,1);           /* çerçeve */
    /* gölge */
    for(int s=1;s<=8;s+=2) acirc(x+w/2,y+h+s,w/2,0x000000,(u8)(30/s));
}

/* Karakter ve yazı (DÜZELTİLDİ: sabit BB_W pitch) */
static void dc(i32 x,i32 y,char ch,u32 fg,u32 bg,u8 bg_a,i32 sc){
    if((u8)ch>=128) ch='?';
    const u8 *g=F8[(u8)ch];
    for(i32 row=0;row<8;row++) for(i32 col=0;col<8;col++){
        if(g[row]&(1<<(7-col))) afr(x+col*sc,y+row*sc,sc,sc,fg,255);
        else if(bg_a>0)         afr(x+col*sc,y+row*sc,sc,sc,bg,bg_a);
    }
}
static void ds(i32 x,i32 y,const char*s,u32 fg,u32 bg,u8 ba,i32 sc){
    i32 cx=x;
    while(*s){if(*s=='\n'){cx=x;y+=8*sc;}else{dc(cx,y,*s,fg,bg,ba,sc);cx+=8*sc;}s++;}
}
static void dsc(i32 x,i32 y,i32 w,const char*s,u32 fg,u32 bg,u8 ba,i32 sc){
    i32 tw=(i32)klen(s)*8*sc;
    ds(tw<w?x+(w-tw)/2:x,y,s,fg,bg,ba,sc);
}

/* Gradient arka plan */
static void draw_bg(void){
    for(i32 y=0;y<(i32)BB_H;y++){
        u8 r2=(u8)((0x1Cu*((i32)BB_H-y)+(0x2Au*(i32)y))/BB_H);
        u8 g2=(u8)((0x1Eu*((i32)BB_H-y)+(0x2Du*(i32)y))/BB_H);
        u8 b2=(u8)((0x23u*((i32)BB_H-y)+(0x34u*(i32)y))/BB_H);
        u32 c=0xFF000000u|(r2<<16)|(g2<<8)|b2;
        for(i32 x=0;x<(i32)BB_W;x++){
            back_buffer[y*BB_W+x]=(x%60==0||y%60==0)?blend_col(GRID_C,c,80):c;
        }
    }
}

/* PixelArt ikon çizici */
static void icon_terminal(i32 cx,i32 cy,i32 s){
    /* Ekran gövdesi */
    arr(cx-s,cy-s,2*s,2*s,4,0xFF1A1D22u,220);
    rb(cx-s,cy-s,2*s,2*s,C_LIME,1);
    /* prompt: > _ */
    ds(cx-s+4,cy-4,">_",C_LIME,0,0,1);
}
static void icon_folder(i32 cx,i32 cy,i32 s){
    fr(cx-s,cy-s/3,2*s,s+(s/3),0xFFFFD700u);
    fr(cx-s,cy-s/3,s-2,s/3,0xFFFFAA00u);
}
static void icon_usb(i32 cx,i32 cy,i32 s){
    /* USB sembolü */
    fr(cx-2,cy-s,4,2*s,USB_C);
    fr(cx-s,cy-s/2,2*s,4,USB_C);
    circ(cx-s,cy-s/2,5,USB_C);
    circ(cx+s,cy-s/2,4,USB_C);
    circ(cx,cy+s,6,USB_C);
}
static void icon_exe(i32 cx,i32 cy,i32 s){
    arr(cx-s,cy-s,2*s,2*s,3,0xFF1A2A40u,240);
    rb(cx-s,cy-s,2*s,2*s,C_BLUE,1);
    ds(cx-s+3,cy-4,".EXE",C_BLUE,0,0,1);
}
static void icon_deb(i32 cx,i32 cy,i32 s){
    arr(cx-s,cy-s,2*s,2*s,3,0xFF2A1A40u,240);
    rb(cx-s,cy-s,2*s,2*s,C_PURP,1);
    ds(cx-s+3,cy-4,".DEB",C_PURP,0,0,1);
}
static void icon_wind(i32 cx,i32 cy,i32 s){
    arr(cx-s,cy-s,2*s,2*s,3,0xFF1A3040u,240);
    rb(cx-s,cy-s,2*s,2*s,C_CYAN,1);
    ds(cx-s+2,cy-4,".WND",C_CYAN,0,0,1);
}
static void icon_ai(i32 cx,i32 cy,i32 s){
    acirc(cx,cy,s,C_PINK,200);
    acirc(cx,cy,s-4,0xFF1A1D22u,230);
    ds(cx-4,cy-4,"AI",C_PINK,0,0,1);
}
static void icon_browser(i32 cx,i32 cy,i32 s){
    acirc(cx,cy,s,0xFF1565C0u,220);
    ds(cx-4,cy-4,"WEB",CW,0,0,1);
}
static void icon_calc(i32 cx,i32 cy,i32 s){
    arr(cx-s,cy-s,2*s,2*s,3,0xFF1A3020u,220);
    ds(cx-4,cy-4,"CALC",C_LIME,0,0,1);
}
static void icon_notes(i32 cx,i32 cy,i32 s){
    arr(cx-s,cy-s,2*s,2*s,3,0xFF3A2010u,220);
    ds(cx-4,cy-4,"NOTE",C_YEL,0,0,1);
}
static void icon_settings(i32 cx,i32 cy,i32 s){
    acirc(cx,cy,s,0xFF424549u,220);
    acirc(cx,cy,s/2,0xFF1C1E23u,255);
}
static void icon_win_logo(i32 cx,i32 cy,i32 s){
    i32 g=2,h=s/2;
    fr(cx-h,cy-h,h-g,h-g,0xFFF35325u);
    fr(cx+g,cy-h,h-g,h-g,0xFF81BC06u);
    fr(cx-h,cy+g,h-g,h-g,0xFF05A6F0u);
    fr(cx+g,cy+g,h-g,h-g,0xFFFFBA08u);
}

/* Swap double-buffer → fiziksel FB */
static void swap_buf(void){
    if(SP==BB_W){
        u32 tot=BB_W*BB_H;
        for(u32 i=0;i<tot;i++) FB[i]=back_buffer[i];
    } else {
        /* pitch farklıysa satır satır kopyala */
        for(u32 y=0;y<BB_H;y++)
            for(u32 x=0;x<BB_W;x++)
                FB[y*SP+x]=back_buffer[y*BB_W+x];
    }
}

/* ================================================================
   PS/2 MOUSE  (polling)
   ================================================================ */
static i32 MX=512,MY=384,MLB=0,MRB=0,PMLB=0;
static u8 MCY=0; static i8 MBF[3]={0}; static int MRD=0;
static void mcw(void){u32 t=100000;while(t--&&(inb(0x64)&0x02));}
static void mdw(void){u32 t=100000;while(t--&&!(inb(0x64)&0x01));}
static void mwr(u8 v){mcw();outb(0x64,0xD4);mcw();outb(0x60,v);}
static u8 mrd(void){mdw();return inb(0x60);}
static void mouse_init(void){
    mcw();outb(0x64,0xA8);mcw();outb(0x64,0x20);mdw();
    u8 cfg=inb(0x60);cfg|=0x02;cfg&=~0x20;
    mcw();outb(0x64,0x60);mcw();outb(0x60,cfg);
    mwr(0xFF);u8 a=mrd();u8 o=mrd();mrd();
    if(a==0xFA&&o==0xAA){mwr(0xF6);mrd();mwr(0xF4);mrd();MRD=1;}
}
static void mouse_poll(void){
    if(!MRD) return;
    for(int i=0;i<16;i++){
        u8 st=inb(0x64);if(!(st&0x01))break;
        if(!(st&0x20)){inb(0x60);continue;}
        u8 d=inb(0x60);
        switch(MCY){
          case 0: if(!(d&0x08)){MCY=0;break;} MBF[0]=(i8)d;MCY=1;break;
          case 1: MBF[1]=(i8)d;MCY=2;break;
          case 2: MBF[2]=(i8)d;MCY=0;{
            i32 dx=(i32)MBF[1];i32 dy=(i32)MBF[2];
            if(MBF[0]&0x10)dx|=(i32)0xFFFFFF00;
            if(MBF[0]&0x20)dy|=(i32)0xFFFFFF00;
            if(MBF[0]&0x40)dx=0;if(MBF[0]&0x80)dy=0;
            MX+=dx;MY-=dy;
            if(MX<0)MX=0;if(MY<0)MY=0;
            if(MX>=(i32)BB_W)MX=(i32)BB_W-1;
            if(MY>=(i32)BB_H)MY=(i32)BB_H-1;
            PMLB=MLB;MLB=(MBF[0]&0x01)?1:0;MRB=(MBF[0]&0x02)?1:0;
          }break;
        }
    }
}
static int CLK(i32 x,i32 y,i32 w,i32 h){return MLB&&!PMLB&&MX>=x&&MX<x+w&&MY>=y&&MY<y+h;}
static int HOV(i32 x,i32 y,i32 w,i32 h){return MX>=x&&MX<x+w&&MY>=y&&MY<y+h;}

/* ================================================================
   PS/2 KLAVYE  (polling)
   ================================================================ */
static u8 KSH=0,KCP=0,KLK=0;
static const char SCMAP[128]={
  0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,
  '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
  0,'a','s','d','f','g','h','j','k','l',';','\'','`',
  0,'\\','z','x','c','v','b','n','m',',','.','/',
  0,'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,'-',0,0,0,'+',0,0,0,0,0,0,0,0,0
};
static u8 kbd_poll(void){
    u8 st=inb(0x64);
    if(!(st&0x01))return 0;
    if(st&0x20){inb(0x60);return 0;}
    u8 sc=inb(0x60);
    if(sc&0x80){u8 r=sc&0x7F;if(r==0x2A||r==0x36)KSH=0;KLK=0;return 0;}
    if(sc==0x2A||sc==0x36){KSH=1;return 0;}
    if(sc==0x3A){KCP=!KCP;return 0;}
    if(sc==KLK)return 0; KLK=sc;
    if(sc>=128)return 0;
    char c=SCMAP[sc];if(!c)return 0;
    if(c>='a'&&c<='z'){if(KSH^KCP)c-=32;}
    else if(KSH){
        switch(c){
          case '1':c='!';break;case '2':c='@';break;case '3':c='#';break;
          case '4':c='$';break;case '5':c='%';break;case '6':c='^';break;
          case '7':c='&';break;case '8':c='*';break;case '9':c='(';break;
          case '0':c=')';break;case '-':c='_';break;case '=':c='+';break;
        }
    }
    return (u8)c;
}

/* ================================================================
   PCI / USB ALGILAMA
   ================================================================ */
static u32 pci_rd(u8 b,u8 d,u8 f,u8 o){
    outl(0xCF8,0x80000000u|((u32)b<<16)|((u32)d<<11)|((u32)f<<8)|(o&0xFC));
    return inl(0xCFC);
}
static int USB_OK=0;
static void pci_scan(void){
    for(int b=0;b<4;b++) for(int d=0;d<32;d++){
        u32 id=pci_rd(b,d,0,0);if((id&0xFFFF)==0xFFFF)continue;
        u32 cl=pci_rd(b,d,0,8);
        if((u8)(cl>>24)==0x0C&&(u8)(cl>>16)==0x03) USB_OK=1;
    }
}

/* ================================================================
   ÇIFT TIKLAMA ALGILAMA
   ================================================================ */
static i32 DC_LX=-1,DC_LY=-1;
static int  DC_CNT=0;
static int  DC_TIM=0;
static int double_click(i32 x,i32 y,i32 w,i32 h){
    if(DC_TIM>0) DC_TIM--;
    if(MLB&&!PMLB&&MX>=x&&MX<x+w&&MY>=y&&MY<y+h){
        if(DC_TIM>0&&DC_LX==x&&DC_LY==y){DC_TIM=0;DC_CNT=0;return 1;}
        DC_TIM=40; DC_LX=x; DC_LY=y;
    }
    return 0;
}

/* ================================================================
   UI ORTAK BİLEŞENLER
   ================================================================ */
static void win_titlebar(i32 x,i32 y,i32 w,const char*title,u32 bg){
    fr(x,y,w,32,bg);
    acirc(x+14,y+16,8,C_RED,255); acirc(x+34,y+16,8,C_ORNG,255); acirc(x+54,y+16,8,C_LIME,255);
    ds(x+72,y+12,title,CTXT,0,0,1);
}
static int btn(i32 x,i32 y,i32 w,i32 h,const char*lbl,u32 bg,u32 fg){
    u32 c=HOV(x,y,w,h)?blend_col(bg,CW,200):bg;
    rr(x,y,w,h,5,c);
    dsc(x,y+(h-8)/2,w,lbl,fg,0,0,1);
    return CLK(x,y,w,h);
}
/* Pencere sürükleme yardımcısı */
static void drag_window(i32*fx,i32*fy,int*fd,i32*dx,i32*dy,i32 fw,i32 fh){
    if(!(*fd)&&MLB&&!PMLB&&MY>=*fy&&MY<*fy+32&&MX>=*fx&&MX<*fx+fw){
        *fd=1;*dx=MX-*fx;*dy=MY-*fy;
    }
    if(*fd){
        if(MLB){
            *fx=MX-*dx;*fy=MY-*dy;
            if(*fx<0)*fx=0;if(*fy<0)*fy=0;
            if(*fx>(i32)BB_W-fw)*fx=(i32)BB_W-fw;
            if(*fy>(i32)BB_H-fh)*fy=(i32)BB_H-fh;
        } else *fd=0;
    }
}

/* ================================================================
   DURUM DEĞİŞKENLERİ
   ================================================================ */

/* Masaüstü ikonları (yüklü uygulamalar) */
typedef struct{char name[20];int show;u32 col;int win_id;} DeskIcon;
static DeskIcon DICO[12]={
    {"Terminal",  1,C_LIME, 1},
    {"Dosyalar",  1,C_CYAN, 2},
    {"Tarayici",  0,C_BLUE, 3},
    {"Hesap Mak.",1,C_ORNG, 4},
    {"Notlar",    1,C_YEL,  5},
    {"Ayarlar",   1,0xFF8899A6u,6},
    {"Yapay Zeka",1,C_PINK, 7},
    {"","",0,0},{"","",0,0},{"","",0,0},{"","",0,0},{"","",0,0},
};

/* Pencere durumu */
typedef struct{int open;i32 x,y;int drag;i32 dx,dy;} WinState;
static WinState WS[8]={{0}};
/* 0=terminal,1=dosya,2=tarayici,3=hesap,4=notlar,5=ayarlar,6=ai */

/* Widget sürükleme */
static i32 WGX=300,WGY=20;
static int WGD=0; static i32 WGDX=0,WGDY=0;

/* USB dosya sistemi */
typedef struct{char n[32];int is_dir;int is_exe;int is_deb;int is_wind;} FSE;
static FSE LFS[]={
    {"Masaustu",1,0,0,0},{"Belgeler",1,0,0,0},{"Indirmeler",1,0,0,0},
    {"Resimler",1,0,0,0},{"Muzik",1,0,0,0},{"kernel.cfg",0,0,0,0},
};
static FSE UFS[]={
    {"ChromeSetup.wind",0,0,0,1},
    {"Notepad.exe",0,1,0,0},
    {"LibreOffice.deb",0,0,1,0},
    {"Oyun.exe",0,1,0,0},
    {"Araclar.deb",0,0,1,0},
};
static int USB_TICK=0;

/* Dosya yöneticisi state */
static int FM_USB=0, FM_SEL=-1;
static i32 FM_X=80,FM_Y=50;
static int FM_DR=0;static i32 FM_DX=0,FM_DY=0;

/* Kurulum dialog */
static int INST_ACT=0,INST_DON=0,INST_PRG=0,INST_SLOT=-1;
static char INST_FN[32]={0},INST_AN[20]={0};
static u32 INST_COL=C_BLUE;

/* Terminal */
static char TERM_BUF[16][64]={
    "Wind OS Terminal v1.0","Yazin ve Enter'a basin:","",
    "","","","","","","","","","","","",""
};
static int TERM_LINES=2;
static char TERM_IN[64]={0};static int TERM_IL=0;

/* Notlar */
static char NOTE_BUF[8][64]={
    "Wind OS Notlar","","Yazmaya baslayabilirsiniz...","","","","",""
};
static int NOTE_LINES=3;
static char NOTE_IN[64]={0};static int NOTE_IL=0;

/* Yapay Zeka chat */
static char AI_Q[16][64]={0};
static char AI_A[16][64]={0};
static int AI_CNT=0;
static char AI_IN[64]={0};static int AI_IL=0;

/* ================================================================
   YAPAY ZEKA (Kelime Eşleşme)
   ================================================================ */
typedef struct{const char*kw;const char*ans;} AIEntry;
static const AIEntry AI_DB[]={
    {"merhaba","Merhaba! Wind OS Yapay Zeka asistaniyim. Size nasil yardimci olabilirim?"},
    {"naber","Harika! Sistemin tum birimleri nominal. Wind OS v11 aktif."},
    {"saat","Sistem saati: 26:03 (Simulasyon modu)"},
    {"hava","Istanbul: 22 derece, parcali bulutlu."},
    {"usb","USB aygiti: Dosya yoneticisini acin, sol panelden USB'ye tiklayin."},
    {"uygulama","Uygulamalari Dosya Yoneticisi'nden yukluyebilirsiniz (.exe, .deb, .wind)."},
    {"kurulum","USB dosyasina cift tiklayin, kurulum sihirbazi otomatik acilir."},
    {"terminal","Terminal uygulamasi ile komut satirina erisebilirsiniz."},
    {"yapay zeka","Ben Wind OS'un yerlesik yapay zeka asistaniyim!"},
    {"yardim","Komutlar: merhaba, saat, hava, usb, uygulama, kurulum, terminal"},
    {"tesekkur","Rica ederim! Baska bir sorunuz var mi?"},
    {NULL,NULL}
};
static const char* ai_respond(const char*q){
    for(int i=0;AI_DB[i].kw;i++){
        const char*kw=AI_DB[i].kw;
        int ki=0,qi=0,match=0;
        while(q[qi]){
            if(q[qi]==kw[ki]||(q[qi]-32)==kw[ki]){ki++;if(!kw[ki]){match=1;break;}}
            else ki=0;
            qi++;
        }
        if(match) return AI_DB[i].ans;
    }
    return "Anlasilmadi. 'yardim' yazarak komutlari gorebilirsiniz.";
}

/* ================================================================
   UYGULAMA PENCERELERİ
   ================================================================ */

/* ── TERMINAL ── */
static void WIN_TERMINAL(void){
    if(!WS[0].open) return;
    if(!WS[0].x&&!WS[0].y){WS[0].x=100;WS[0].y=60;}
    i32 fw=520,fh=360,fx=WS[0].x,fy=WS[0].y;
    glass(fx,fy,fw,fh,6,WIN_BG,240);
    win_titlebar(fx,fy,fw,"Terminal",WIN_TIT);
    if(CLK(fx+14,fy+8,16,16)){WS[0].open=0;return;}
    fr(fx,fy+32,fw,fh-32,0xFF0D1117u);
    for(int i=0;i<TERM_LINES&&i<14;i++)
        ds(fx+8,fy+36+i*18,TERM_BUF[i],C_LIME,0,0,1);
    /* input satırı */
    ds(fx+8,fy+36+TERM_LINES*18,"$ ",C_LIME,0,0,1);
    ds(fx+24,fy+36+TERM_LINES*18,TERM_IN,CW,0,0,1);
    /* cursor */
    fr(fx+24+TERM_IL*8,fy+36+TERM_LINES*18,2,8,C_LIME);
    drag_window(&WS[0].x,&WS[0].y,&WS[0].drag,&WS[0].dx,&WS[0].dy,fw,fh);
}

/* ── TARAYICI ── */
static char BURL[64]="https://windos.local/";
static int BURL_EDIT=0;
static void WIN_BROWSER(void){
    if(!WS[2].open) return;
    if(!WS[2].x&&!WS[2].y){WS[2].x=120;WS[2].y=70;}
    i32 fw=580,fh=400,fx=WS[2].x,fy=WS[2].y;
    glass(fx,fy,fw,fh,6,WIN_BG,240);
    win_titlebar(fx,fy,fw,"Tarayici",WIN_TIT);
    if(CLK(fx+14,fy+8,16,16)){WS[2].open=0;return;}
    /* URL bar */
    fr(fx+4,fy+36,fw-8,28,INP_BG); rb(fx+4,fy+36,fw-8,28,INP_BD,1);
    ds(fx+12,fy+42,BURL,C_CYAN,0,0,1);
    /* Sayfa içeriği */
    glass(fx+8,fy+70,fw-16,fh-78,4,0xFF1A1D22u,200);
    dsc(fx,fy+100,fw,"Wind OS Web Tarayici",C_CYAN,0,0,2);
    dsc(fx,fy+130,fw,"windos.local - Ana Sayfa",CGY,0,0,1);
    dsc(fx,fy+160,fw,"Wind OS v11 - Calistiriyor",C_LIME,0,0,1);
    dsc(fx,fy+185,fw,"USB'den uygulama yuklendi mi? Harika!",CTXT,0,0,1);
    drag_window(&WS[2].x,&WS[2].y,&WS[2].drag,&WS[2].dx,&WS[2].dy,fw,fh);
}

/* ── HESAP MAKİNESİ ── */
static char CALC_D[32]="0"; static int CALC_LN=1;
static double CALC_MEM=0; static int CALC_OP=0;
static void WIN_CALC(void){
    if(!WS[3].open) return;
    if(!WS[3].x&&!WS[3].y){WS[3].x=400;WS[3].y=120;}
    i32 fw=240,fh=320,fx=WS[3].x,fy=WS[3].y;
    glass(fx,fy,fw,fh,6,WIN_BG,240);
    win_titlebar(fx,fy,fw,"Hesap Makinesi",WIN_TIT);
    if(CLK(fx+14,fy+8,16,16)){WS[3].open=0;return;}
    /* Ekran */
    fr(fx+4,fy+36,fw-8,36,INP_BG);
    dsc(fx,fy+46,fw,CALC_D,CW,0,0,2);
    /* Tuşlar */
    const char*bls[]={"7","8","9","/","4","5","6","*","1","2","3","-","0",".","=","+"};
    u32 bcs[]={WIN_BG,WIN_BG,WIN_BG,C_ORNG,WIN_BG,WIN_BG,WIN_BG,C_ORNG,
               WIN_BG,WIN_BG,WIN_BG,C_ORNG,WIN_BG,WIN_BG,BTN_C,C_ORNG};
    for(int i=0;i<16;i++){
        i32 bx=fx+4+(i%4)*58, by=fy+76+(i/4)*56;
        if(btn(bx,by,54,52,bls[i],bcs[i],CW)){
            char c=bls[i][0];
            if(c>='0'&&c<='9'||c=='.'){
                if(CALC_D[0]=='0'&&CALC_D[1]==0&&c!='.')CALC_LN=0;
                if(CALC_LN<18){CALC_D[CALC_LN++]=c;CALC_D[CALC_LN]=0;}
            } else if(c=='='||c=='+'||c=='-'||c=='*'||c=='/'){
                CALC_D[0]='0';CALC_D[1]=0;CALC_LN=1; /* basit sıfırla */
            }
        }
    }
    drag_window(&WS[3].x,&WS[3].y,&WS[3].drag,&WS[3].dx,&WS[3].dy,fw,fh);
}

/* ── NOTLAR ── */
static void WIN_NOTES(void){
    if(!WS[4].open) return;
    if(!WS[4].x&&!WS[4].y){WS[4].x=180;WS[4].y=80;}
    i32 fw=460,fh=340,fx=WS[4].x,fy=WS[4].y;
    glass(fx,fy,fw,fh,6,WIN_BG,240);
    win_titlebar(fx,fy,fw,"Notlar",WIN_TIT);
    if(CLK(fx+14,fy+8,16,16)){WS[4].open=0;return;}
    fr(fx,fy+32,fw,fh-32,0xFF1A1D22u);
    for(int i=0;i<NOTE_LINES&&i<15;i++)
        ds(fx+8,fy+36+i*18,NOTE_BUF[i],C_YEL,0,0,1);
    /* input */
    ds(fx+8,fy+36+NOTE_LINES*18,NOTE_IN,CW,0,0,1);
    fr(fx+8+NOTE_IL*8,fy+36+NOTE_LINES*18,2,8,C_YEL);
    drag_window(&WS[4].x,&WS[4].y,&WS[4].drag,&WS[4].dx,&WS[4].dy,fw,fh);
}

/* ── AYARLAR ── */
static int SET_TGL[4]={1,0,1,0};
static void WIN_SETTINGS(void){
    if(!WS[5].open) return;
    if(!WS[5].x&&!WS[5].y){WS[5].x=200;WS[5].y=100;}
    i32 fw=400,fh=300,fx=WS[5].x,fy=WS[5].y;
    glass(fx,fy,fw,fh,6,WIN_BG,240);
    win_titlebar(fx,fy,fw,"Ayarlar",WIN_TIT);
    if(CLK(fx+14,fy+8,16,16)){WS[5].open=0;return;}
    const char*sn[]={"Wi-Fi","Bluetooth","Karanlik Tema","Bildirimler"};
    for(int i=0;i<4;i++){
        i32 sy=fy+44+i*56;
        glass(fx+8,sy,fw-16,48,4,WIN_TIT,200);
        ds(fx+16,sy+18,sn[i],CTXT,0,0,1);
        /* toggle */
        u32 tbg=SET_TGL[i]?C_LIME:PAN_BD;
        rr(fx+fw-70,sy+14,44,22,11,tbg);
        circ(SET_TGL[i]?fx+fw-36:fx+fw-57,sy+25,10,CW);
        if(CLK(fx+fw-70,sy+14,44,22)) SET_TGL[i]=!SET_TGL[i];
    }
    drag_window(&WS[5].x,&WS[5].y,&WS[5].drag,&WS[5].dx,&WS[5].dy,fw,fh);
}

/* ── YAPAY ZEKA ── */
static void WIN_AI(void){
    if(!WS[6].open) return;
    if(!WS[6].x&&!WS[6].y){WS[6].x=160;WS[6].y=60;}
    i32 fw=500,fh=380,fx=WS[6].x,fy=WS[6].y;
    glass(fx,fy,fw,fh,6,WIN_BG,240);
    win_titlebar(fx,fy,fw,"Wind OS Yapay Zeka",WIN_TIT);
    if(CLK(fx+14,fy+8,16,16)){WS[6].open=0;return;}
    /* Sohbet geçmişi */
    glass(fx+8,fy+36,fw-16,fh-84,4,0xFF1A1D22u,220);
    for(int i=0;i<AI_CNT&&i<8;i++){
        i32 qy=fy+42+i*38;
        ds(fx+14,qy,AI_Q[i],C_CYAN,0,0,1);
        ds(fx+14,qy+14,AI_A[i],C_LIME,0,0,1);
    }
    /* Giriş */
    fr(fx+8,fy+fh-44,fw-90,36,INP_BG);
    rb(fx+8,fy+fh-44,fw-90,36,INP_BD,1);
    ds(fx+14,fy+fh-32,AI_IN,CTXT,0,0,1);
    fr(fx+14+AI_IL*8,fy+fh-32,2,8,C_PINK);
    if(btn(fx+fw-78,fy+fh-44,70,36,"Gonder",C_PINK,CW)&&AI_IL>0&&AI_CNT<8){
        kcpy(AI_Q[AI_CNT],"> "); /* prefix */
        kcpy(AI_Q[AI_CNT]+2,AI_IN);
        kcpy(AI_A[AI_CNT],(char*)ai_respond(AI_IN));
        AI_CNT++;
        AI_IN[0]=0;AI_IL=0;
    }
    drag_window(&WS[6].x,&WS[6].y,&WS[6].drag,&WS[6].dx,&WS[6].dy,fw,fh);
}

/* ================================================================
   KURULUM DIALOGU
   ================================================================ */
static void inst_start(const char*fn){
    if(INST_ACT) return;
    INST_ACT=1;INST_DON=0;INST_PRG=0;
    kcpy(INST_FN,fn);
    int l=klen(fn);
    /* Uzantı tespiti */
    if(l>5&&fn[l-1]=='d'&&fn[l-2]=='n'&&fn[l-3]=='i'&&fn[l-4]=='w'){
        kcpy(INST_AN,"Tarayici");INST_COL=C_BLUE;
        DICO[2].show=1;kcpy(DICO[2].name,"Tarayici");DICO[2].col=C_BLUE;DICO[2].win_id=3;
        INST_SLOT=2;
    } else if(l>4&&fn[l-1]=='e'&&fn[l-2]=='x'&&fn[l-3]=='e'){
        /* Boş slot bul */
        INST_SLOT=-1;
        for(int i=7;i<12;i++) if(!DICO[i].show){INST_SLOT=i;break;}
        if(INST_SLOT>=0){kcpy(DICO[INST_SLOT].name,fn);DICO[INST_SLOT].col=C_RED;DICO[INST_SLOT].show=1;DICO[INST_SLOT].win_id=0;}
        kcpy(INST_AN,fn);INST_COL=C_RED;
    } else if(l>4&&fn[l-1]=='b'&&fn[l-2]=='e'&&fn[l-3]=='d'){
        INST_SLOT=-1;
        for(int i=7;i<12;i++) if(!DICO[i].show){INST_SLOT=i;break;}
        if(INST_SLOT>=0){kcpy(DICO[INST_SLOT].name,fn);DICO[INST_SLOT].col=C_PURP;DICO[INST_SLOT].show=1;DICO[INST_SLOT].win_id=0;}
        kcpy(INST_AN,fn);INST_COL=C_PURP;
    } else INST_ACT=0;
}
static void INSTALL_DIALOG(void){
    if(!INST_ACT) return;
    if(!INST_DON&&INST_PRG<100) INST_PRG+=2;
    if(INST_PRG>=100) INST_DON=1;
    i32 dw=420,dh=190,ddx=(i32)BB_W/2-dw/2,ddy=(i32)BB_H/2-dh/2;
    glass(ddx,ddy,dw,dh,8,WIN_BG,255);
    win_titlebar(ddx,ddy,dw,"Uygulama Yukleyici",WIN_TIT);
    ds(ddx+16,ddy+44,"Dosya   : ",CGY,0,0,1); ds(ddx+90,ddy+44,INST_FN,CTXT,0,0,1);
    ds(ddx+16,ddy+60,"Program : ",CGY,0,0,1); ds(ddx+90,ddy+60,INST_AN,CTXT,0,0,1);
    /* progress bar */
    fr(ddx+16,ddy+82,dw-32,20,PAN_BD);
    i32 pf=(i32)((dw-34)*INST_PRG/100);
    if(pf>0){rr(ddx+17,ddy+83,pf,18,4,INST_COL);}
    char ps[8]={0};itoa(INST_PRG,ps);ps[klen(ps)]='%';
    dsc(ddx,ddy+106,dw,ps,CTXT,0,0,1);
    if(!INST_DON){
        dsc(ddx,ddy+124,dw,"Yukleniyor, lutfen bekleyin...",CGY,0,0,1);
    } else {
        dsc(ddx,ddy+124,dw,"Kurulum tamamlandi! Masaustune eklendi.",C_LIME,0,0,1);
        if(btn(ddx+dw/2-55,ddy+154,110,28,"Tamam",C_LIME,CK)) INST_ACT=0;
    }
}

/* ================================================================
   DOSYA YÖNETİCİSİ
   ================================================================ */
static void WIN_FILEMGR(void){
    if(!WS[1].open) return;
    if(!WS[1].x&&!WS[1].y){WS[1].x=90;WS[1].y=55;}
    i32 fw=640,fh=440,fx=WS[1].x,fy=WS[1].y;
    glass(fx,fy,fw,fh,6,WIN_BG,240);
    win_titlebar(fx,fy,fw,"Dosya Yoneticisi",WIN_TIT);
    if(CLK(fx+14,fy+8,16,16)){WS[1].open=0;return;}

    /* Adres çubuğu */
    fr(fx,fy+32,fw,26,INP_BG);
    if(FM_USB){
        ds(fx+8,fy+39,"Bu Bilgisayar > USB Surucu",C_CYAN,0,0,1);
        if(CLK(fx+8,fy+34,200,20)) FM_USB=0;
    } else ds(fx+8,fy+39,"Bu Bilgisayar",CTXT,0,0,1);

    /* Sol panel */
    i32 sb=130;
    glass(fx,fy+58,sb,fh-58,0,PAN_BD,180);
    const char*si[]={"Bu Bilgisayar","Masaustu","Belgeler","Resimler","Muzik"};
    for(int i=0;i<5;i++) ds(fx+10,fy+68+i*22,si[i],FM_USB?CGY:CTXT,0,0,1);

    /* USB göstergesi — TAKILİ İSE DIREKT GÖZÜKÜR */
    if(USB_OK){
        i32 ubx=fx+6,uby=fy+188;
        u32 ubg=FM_USB?blend_col(C_CYAN,WIN_BG,60):blend_col(USB_C,WIN_BG,30);
        rr(ubx,uby,sb-12,30,4,ubg);
        /* USB ikonu: küçük çubuk + yuvarlak */
        fr(ubx+8,uby+8,6,14,USB_C);
        fr(ubx+5,uby+8,12,4,USB_C);
        circ(ubx+11,uby+22,4,USB_C);
        ds(ubx+22,uby+10,"USB Surucu",USB_C,0,0,1);
        if(CLK(ubx,uby,sb-12,30)) FM_USB=1;
    } else {
        ds(fx+10,fy+192,"USB yok",CGY,0,0,1);
    }

    /* Dosya içeriği */
    i32 cx2=fx+sb+6, cy2=fy+62;
    FSE *en=FM_USB?UFS:LFS;
    int cnt=FM_USB?5:6;
    for(int i=0;i<cnt;i++){
        i32 ex=cx2+(i%4)*122, ey=cy2+(i/4)*106;
        if(ex+100>fx+fw||ey+96>fy+fh) continue;
        u32 bg=(FM_SEL==i)?blend_col(C_BLUE,WIN_BG,60):WIN_BG;
        glass(ex,ey,100,90,5,bg,220);
        /* İkon çiz */
        if(en[i].is_dir)      icon_folder(ex+50,ey+34,20);
        else if(en[i].is_exe) icon_exe(ex+50,ey+34,18);
        else if(en[i].is_deb) icon_deb(ex+50,ey+34,18);
        else if(en[i].is_wind)icon_wind(ex+50,ey+34,18);
        else                  { fr(ex+32,ey+14,36,44,0xFF1A2A40u);rb(ex+32,ey+14,36,44,INP_BD,1); }
        /* Ad */
        char sn[14]={0}; int nl=klen(en[i].n);
        if(nl>12){mcpy(sn,en[i].n,10);sn[10]='.';sn[11]='.';}
        else kcpy(sn,en[i].n);
        dsc(ex,ey+76,100,sn,CTXT,0,0,1);

        /* Tek tık: seç */
        if(CLK(ex,ey,100,90)) FM_SEL=i;

        /* ÇİFT TIKLAMA → kurulum sorusu */
        if(FM_USB&&!en[i].is_dir&&(en[i].is_exe||en[i].is_deb||en[i].is_wind)){
            if(double_click(ex,ey,100,90)){
                inst_start(en[i].n);
            }
        }
    }

    /* Durum çubuğu */
    fr(fx,fy+fh-20,fw,20,PAN_BD);
    ds(fx+8,fy+fh-14,USB_OK?"USB takili — Cift tikla: yukle":"USB takili degil",USB_OK?USB_C:CGY,0,0,1);

    drag_window(&WS[1].x,&WS[1].y,&WS[1].drag,&WS[1].dx,&WS[1].dy,fw,fh);
}

/* ================================================================
   MASAÜSTÜ + GÖREV ÇUBUĞU
   ================================================================ */
static void DESKTOP_DRAW(void){
    draw_bg();

    /* ── Sürüklenebilir Hava/Saat Widgeti ── */
    drag_window(&WGX,&WGY,&WGD,&WGDX,&WGDY,280,90);
    glass(WGX,WGY,280,90,12,0xFF1A2533u,200);
    ds(WGX+16,WGY+10,"Hava Durumu",CGY,0,0,1);
    ds(WGX+16,WGY+26,"Istanbul  22 C",CW,0,0,2);
    ds(WGX+16,WGY+60,"Parcali Bulutlu",CGY,0,0,1);
    ds(WGX+180,WGY+26,"26:03",C_CYAN,0,0,2);
    ds(WGX+184,WGY+56,"01.10",CGY,0,0,1);
    /* Tutaç */
    fr(WGX+270,WGY+2,6,86,blend_col(CW,0,40));

    /* ── Masaüstü İkonları (altta 2 satır) ── */
    int cnt=0;
    for(int i=0;i<12;i++) if(DICO[i].show) cnt++;
    int ic_y=(i32)BB_H-130;
    int ic_x=60;
    int drawn=0;
    for(int i=0;i<12;i++){
        if(!DICO[i].show) continue;
        i32 ix=ic_x+drawn*90;
        if(ix+80>(i32)BB_W-100){ix=ic_x;ic_y+=90;drawn=0;}
        /* İkon gölgesi + zemin */
        acirc(ix+36,ic_y+36,32,CK,60);
        glass(ix,ic_y,72,72,10,blend_col(DICO[i].col,WIN_BG,60),220);
        /* Renk göre ikon */
        if(DICO[i].win_id==1) icon_terminal(ix+36,ic_y+32,16);
        else if(DICO[i].win_id==2) icon_folder(ix+36,ic_y+32,16);
        else if(DICO[i].win_id==3) icon_browser(ix+36,ic_y+32,16);
        else if(DICO[i].win_id==4) icon_calc(ix+36,ic_y+32,16);
        else if(DICO[i].win_id==5) icon_notes(ix+36,ic_y+32,16);
        else if(DICO[i].win_id==6) icon_settings(ix+36,ic_y+32,16);
        else if(DICO[i].win_id==7) icon_ai(ix+36,ic_y+32,16);
        else if(DICO[i].col==C_RED)  icon_exe(ix+36,ic_y+32,14);
        else if(DICO[i].col==C_PURP) icon_deb(ix+36,ic_y+32,14);
        else                          icon_wind(ix+36,ic_y+32,14);
        dsc(ix-8,ic_y+74,88,DICO[i].name,CTXT,0,0,1);
        /* Çift tık → ilgili pencereyi aç */
        if(double_click(ix,ic_y,72,72)){
            int wid=DICO[i].win_id;
            if(wid>=1&&wid<=7) WS[wid-1].open=1;
        }
        drawn++;
    }

    /* ── Uygulama pencereleri ── */
    WIN_FILEMGR();
    WIN_TERMINAL();
    WIN_BROWSER();
    WIN_CALC();
    WIN_NOTES();
    WIN_SETTINGS();
    WIN_AI();

    /* ── Kurulum dialogu (modal, en üstte) ── */
    INSTALL_DIALOG();

    /* ── Görev Çubuğu ── */
    glass(0,(i32)BB_H-52,(i32)BB_W,52,0,0xFF14161Au,240);
    /* Wind logosu */
    icon_win_logo(26,(i32)BB_H-26,26);
    /* Uygulama butonları */
    typedef struct{const char*l;int wi;}TB;
    TB tb[]={{"FM",2},{"TERM",1},{"TARAY",3},{"HESAP",4},{"NOT",5},{"AYAR",6},{"AI",7}};
    for(int i=0;i<7;i++){
        i32 tx=64+i*78,ty=(i32)BB_H-46;
        int act=WS[tb[i].wi-1].open;
        arr(tx,ty,72,36,5,act?blend_col(tb[i].wi==7?C_PINK:C_BLUE,WIN_BG,120):PAN_BD,act?220:160);
        dsc(tx,ty+14,72,tb[i].l,act?CW:CGY,0,0,1);
        if(act) fr(tx+28,ty+34,16,4,C_BLUE);
        if(CLK(tx,ty,72,36)) WS[tb[i].wi-1].open=!WS[tb[i].wi-1].open;
    }
    /* USB göstergesi görev çubuğunda */
    if(USB_OK){
        icon_usb((i32)BB_W-56,(i32)BB_H-26,10);
        ds((i32)BB_W-40,(i32)BB_H-30,"USB",USB_C,0,0,1);
    }
    /* Saat */
    ds((i32)BB_W-88,(i32)BB_H-40,"26:03",CTXT,0,0,1);
    ds((i32)BB_W-90,(i32)BB_H-24,"01.10.27",CGY,0,0,1);

    /* ── Mouse imleci ── */
    static const u8 CUP[16][12]={
        {1},{1,1},{1,2,1},{1,2,2,1},{1,2,2,2,1},{1,2,2,2,2,1},
        {1,2,2,2,2,2,1},{1,2,2,2,2,2,2,1},{1,2,2,2,2,2,2,2,1},
        {1,2,2,2,2,1,1,1,1,1},{1,2,2,1,2,2,1},{1,2,1,0,1,2,2,1},
        {1,1,0,0,1,2,2,1},{0,0,0,0,0,1,2,2,1},{0,0,0,0,0,1,2,2,1},{0,0,0,0,0,0,1,1}
    };
    /* gölge */
    for(int r=0;r<16;r++) for(int c=0;c<12;c++)
        if(CUP[r][c]) app(MX+c+2,MY+r+2,CK,60);
    for(int r=0;r<16;r++) for(int c=0;c<12;c++){
        if(CUP[r][c]==1) app(MX+c,MY+r,CW,255);
        else if(CUP[r][c]==2) app(MX+c,MY+r,CK,255);
    }
}

/* ================================================================
   KLAVYE GİRİŞİNİ AKTİF PENCEREYE YÖNLENDIR
   ================================================================ */
static void handle_key(u8 key){
    if(!key) return;
    /* Terminal aktif mi? */
    if(WS[0].open){
        if(key=='\n'&&TERM_IL>0){
            if(TERM_LINES<15){kcpy(TERM_BUF[TERM_LINES],TERM_IN);TERM_LINES++;}
            TERM_IN[0]=0;TERM_IL=0;
        } else if(key==8&&TERM_IL>0){TERM_IN[--TERM_IL]=0;}
        else if(key>=32&&key<127&&TERM_IL<60){TERM_IN[TERM_IL++]=(char)key;TERM_IN[TERM_IL]=0;}
        return;
    }
    /* Notlar aktif mi? */
    if(WS[4].open){
        if(key=='\n'&&NOTE_LINES<7){kcpy(NOTE_BUF[NOTE_LINES],NOTE_IN);NOTE_LINES++;NOTE_IN[0]=0;NOTE_IL=0;}
        else if(key==8&&NOTE_IL>0){NOTE_IN[--NOTE_IL]=0;}
        else if(key>=32&&key<127&&NOTE_IL<60){NOTE_IN[NOTE_IL++]=(char)key;NOTE_IN[NOTE_IL]=0;}
        return;
    }
    /* AI aktif mi? */
    if(WS[6].open){
        if(key=='\n'&&AI_IL>0&&AI_CNT<8){
            kcpy(AI_Q[AI_CNT],"> "); kcpy(AI_Q[AI_CNT]+2,AI_IN);
            kcpy(AI_A[AI_CNT],(char*)ai_respond(AI_IN));
            AI_CNT++;AI_IN[0]=0;AI_IL=0;
        } else if(key==8&&AI_IL>0){AI_IN[--AI_IL]=0;}
        else if(key>=32&&key<127&&AI_IL<60){AI_IN[AI_IL++]=(char)key;AI_IN[AI_IL]=0;}
    }
}

/* ================================================================
   KERNEL_MAIN
   ================================================================ */
void kernel_main(multiboot_info_t *mbi){
    u8 bpp=mbi->framebuffer_bpp; if(bpp==0)bpp=32;
    u32 Bpp=(u32)bpp/8u;
    FB  =(volatile u32*)(u32)(mbi->framebuffer_addr);
    SW  =mbi->framebuffer_width;
    SH  =mbi->framebuffer_height;
    SP  =mbi->framebuffer_pitch/Bpp;
    if(!FB||SW==0){FB=(volatile u32*)0xFD000000u;SW=1024;SH=768;SP=1024;}

    mouse_init();
    pci_scan();

    while(1){
        mouse_poll();
        u8 key=kbd_poll();
        handle_key(key);

        /* USB yeniden tara */
        if(++USB_TICK>4000){pci_scan();USB_TICK=0;}

        DESKTOP_DRAW();
        swap_buf();

        volatile int d=8000; while(d--)__asm__("nop");
    }
}
