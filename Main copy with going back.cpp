#include "API.h"
#include <vector>
#include <cstdlib>
#include <utility>
#include <iostream>
#include <queue>
#include <array>
#include <algorithm>
#include <cassert>
#include <string>

namespace Mouse {

enum Heading {N=0, E=1, S=2, Wd=3};

static const int MAX_MAZE = 32;
static const unsigned WALL_N = 1u<<0;
static const unsigned WALL_E = 1u<<1;
static const unsigned WALL_S = 1u<<2;
static const unsigned WALL_W = 1u<<3;
static const unsigned KNOWN_N = 1u<<4;
static const unsigned KNOWN_E = 1u<<5;
static const unsigned KNOWN_S = 1u<<6;
static const unsigned KNOWN_W = 1u<<7;

inline unsigned wallMask(Heading h){switch(h){case N:return WALL_N;case E:return WALL_E;case S:return WALL_S;default:return WALL_W;}}
inline unsigned knownMask(Heading h){switch(h){case N:return KNOWN_N;case E:return KNOWN_E;case S:return KNOWN_S;default:return KNOWN_W;}}
inline Heading opp(Heading h){return Heading((int(h)+2)&3);} 

struct Cell {unsigned bits; Cell():bits(0){}};

static int mazeW=16, mazeH=16;
static Cell maze[MAX_MAZE][MAX_MAZE];
static int dist[MAX_MAZE][MAX_MAZE];
static int x_=0,y_=0;
static Heading facing_=N;
static std::vector<std::pair<int,int> > goals;

inline bool inBounds(int x,int y){return x>=0 && x<mazeW && y>=0 && y<mazeH;}
void showWall(int x,int y,Heading h){char d='n'; if(h==E)d='e'; else if(h==S)d='s'; else if(h==Wd)d='w'; API::setWall(x,y,d);} 
void showDist(int x,int y,int d){API::setText(x,y,std::to_string(d));}

bool haveWall(int x,int y,Heading h){return maze[x][y].bits & wallMask(h);}        
bool knowSide(int x,int y,Heading h){return maze[x][y].bits & knownMask(h);}       

void setWallKnown(int x,int y,Heading h,bool wall){
    if(!inBounds(x,y)) return;
    Cell &c=maze[x][y];
    if(wall) c.bits |= wallMask(h); else c.bits &= ~wallMask(h);
    c.bits |= knownMask(h);
    int nx=x,ny=y; Heading oh=opp(h);
    if(h==N) ny++; else if(h==E) nx++; else if(h==S) ny--; else nx--;
    if(inBounds(nx,ny)){
        Cell &n=maze[nx][ny];
        if(wall) n.bits |= wallMask(oh); else n.bits &= ~wallMask(oh);
        n.bits |= knownMask(oh);
    }
    if(wall) showWall(x,y,h);
}

bool senseRelative(Heading rel){switch(rel){case N: return API::wallFront();case E: return API::wallRight();case S: return API::wallFront();case Wd: return API::wallLeft();} return false;}
void senseAllSides(){bool wf=senseRelative(N);bool wr=senseRelative(E);bool wb=false;bool wl=senseRelative(Wd);setWallKnown(x_,y_,facing_,wf);setWallKnown(x_,y_,Heading((int(facing_)+1)&3),wr);setWallKnown(x_,y_,opp(facing_),wb);setWallKnown(x_,y_,Heading((int(facing_)+3)&3),wl);} 

void turnLeft(){API::turnLeft(); facing_=Heading((int(facing_)+3)&3);} 
void turnRight(){API::turnRight(); facing_=Heading((int(facing_)+1)&3);} 
void face(Heading h){int dt=((int)h-(int)facing_)&3; if(dt==1) turnRight(); else if(dt==3) turnLeft(); else if(dt==2){turnRight();turnRight();}}
void stepForward(){API::moveForward(); if(facing_==N) y_++; else if(facing_==E) x_++; else if(facing_==S) y_--; else x_--; if(!inBounds(x_,y_)) { std::cerr<<"Out of bounds: ("<<x_<<","<<y_<<")"<<std::endl; std::terminate(); }}

void recomputeDistances(){
    const int INF=1000000000;
    for(int x=0;x<mazeW;x++) for(int y=0;y<mazeH;y++) dist[x][y]=INF;
    std::queue<std::pair<int,int> > q;
    for(size_t i=0;i<goals.size();++i){int gx=goals[i].first;int gy=goals[i].second;if(inBounds(gx,gy)){dist[gx][gy]=0;q.push(goals[i]);}}
    while(!q.empty()){
        int cx=q.front().first, cy=q.front().second; q.pop(); int cd=dist[cx][cy];
        static const int dx[4]={0,1,0,-1}; static const int dy[4]={1,0,-1,0}; static const Heading hh[4]={N,E,S,Wd};
        for(int k=0;k<4;k++){
            int nx=cx+dx[k], ny=cy+dy[k]; Heading h=hh[k];
            if(!inBounds(nx,ny)) continue;
            if(knowSide(cx,cy,h)&&haveWall(cx,cy,h)) continue;
            if(dist[nx][ny]==INF){dist[nx][ny]=cd+1; q.push(std::make_pair(nx,ny));}
        }
    }
    for(int x=0;x<mazeW;x++) for(int y=0;y<mazeH;y++) if(dist[x][y]<INF) showDist(x,y,dist[x][y]);
}

void computeToStart(){
    goals.clear(); goals.push_back(std::make_pair(0,0));
    recomputeDistances();
}

bool chooseNextCell(int &tx,int &ty,Heading &th){
    int best=dist[x_][y_]; bool found=false;
    static const int dx[4]={0,1,0,-1}; static const int dy[4]={1,0,-1,0}; static const Heading hh[4]={N,E,S,Wd};
    for(int k=0;k<4;k++){
        int nx=x_+dx[k], ny=y_+dy[k]; Heading h=hh[k];
        if(!inBounds(nx,ny)) continue;
        if(knowSide(x_,y_,h)&&haveWall(x_,y_,h)) continue;
        int d=dist[nx][ny];
        if(d<best){best=d;tx=nx;ty=ny;th=h;found=true;}
    }
    return found;
}

bool atGoal(){for(size_t i=0;i<goals.size();++i) if(x_==goals[i].first && y_==goals[i].second) return true; return false;}

void init(){
    mazeW=API::mazeWidth(); mazeH=API::mazeHeight();
    if(mazeW>MAX_MAZE) mazeW=MAX_MAZE; if(mazeH>MAX_MAZE) mazeH=MAX_MAZE;
    for(int x=0;x<mazeW;x++){setWallKnown(x,0,S,true);setWallKnown(x,mazeH-1,N,true);} 
    for(int y=0;y<mazeH;y++){setWallKnown(0,y,Wd,true);setWallKnown(mazeW-1,y,E,true);} 
    int cx0=(mazeW-1)/2, cx1=mazeW/2, cy0=(mazeH-1)/2, cy1=mazeH/2;
    goals.clear(); goals.push_back(std::make_pair(cx0,cy0));
    if(cx1!=cx0||cy1!=cy0){goals.push_back(std::make_pair(cx1,cy0));goals.push_back(std::make_pair(cx0,cy1));goals.push_back(std::make_pair(cx1,cy1));}
    recomputeDistances(); senseAllSides(); recomputeDistances();
}

void returnToStart(){
    computeToStart();
    while(!atGoal()){
        int tx=x_, ty=y_; Heading th=facing_;
        if(!chooseNextCell(tx,ty,th)){std::cerr<<"No path to start!"<<std::endl; break;}
        face(th); stepForward(); senseAllSides(); recomputeDistances();
    }
    std::cerr << "Returned to start." << std::endl;
}

void solve(){
    init();
    while(true){
        if(atGoal()){std::cerr<<"Reached goal at ("<<x_<<","<<y_<<")!"<<std::endl; break;}
        int tx=x_,ty=y_; Heading th=facing_;
        if(!chooseNextCell(tx,ty,th)){std::cerr<<"No path!"<<std::endl; break;}
        face(th); stepForward(); senseAllSides(); recomputeDistances();
    }
    returnToStart();
}

} // namespace Mouse

int main(){ Mouse::solve(); return 0; }
