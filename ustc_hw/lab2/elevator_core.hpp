#pragma once
#include <bits/stdc++.h>
using namespace std;

/************* 手写顺序栈（不使用 std::stack） *************/
template<typename T>
class ArrayStack {
    T* buf=nullptr; size_t n=0, cap=0;
    void grow(){ size_t nc = cap? cap*2:8; T* nb=new T[nc];
        for(size_t i=0;i<n;++i) nb[i]=buf[i]; delete[] buf; buf=nb; cap=nc; }
public:
    ~ArrayStack(){ delete[] buf; }
    bool empty()const{return n==0;} size_t size()const{return n;}
    void push(const T& v){ if(n==cap) grow(); buf[n++]=v; }
    T pop(){ if(!n) throw runtime_error("pop"); return buf[--n]; }
    T& top(){ if(!n) throw runtime_error("top"); return buf[n-1]; }
};

/******************* 通用参数（两边共享） *******************/
static const int TICK_PER_SEC = 10;  // 1 tick = 0.1s
static const int T_OPEN  = 20;       // 开门  2.0s
static const int T_CLOSE = 20;       // 关门  2.0s
static const int T_INOUT = 25;       // 单人进/出 2.5s
static const int T_MOVE_ONE = 30;    // 层间 3.0s（可按需要调整）
static const int LOBBY = 1;

/******************* 基本数据结构 *******************/
struct Passenger {
    int id;
    int bornTick;        // 出现时刻
    int from, to;        // 出发层 -> 目的层
    // 性能统计（可选）
    int waitStartTick=-1, boardTick=-1, arriveTick=-1;
};

enum Dir { DOWN=-1, IDLE=0, UP=1 };

static inline string dirStr(Dir d){ return d==UP? "UP" : d==DOWN? "DOWN":"IDLE"; }
static inline string fmt_time_hhmmss(int tick, int baseH=0, int baseM=0, int baseS=0){
    int sec = tick / TICK_PER_SEC;
    int h=baseH, m=baseM, s=baseS + sec;
    m += s/60; s%=60; h+=m/60; m%=60;
    char buf[32]; sprintf(buf,"%02d:%02d:%02d",h,m,s);
    return string(buf);
}
