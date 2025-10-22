#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>   // for va_list / va_start / vsnprintf

/*** 基本参数 ***/
static const int FLOORS = 5;         
static const int LOBBY  = 1;
static const int TICK_MS = 100;
static const int DOOR_ACT = 20;
static const int PERSON_IO = 25;
static const int MOVE_UP   = 51;
static const int MOVE_DOWN = 61;
static const int IDLE_BELL = 300;
static const int SIM_MIN_T = 500;
static const int SIM_MAX_T = 3000;

/* 可调参数 */
const int MAX_OPEN_TICKS = 120;

/* 限制（为了简单实现队列/容器） */
#define MAX_ELEVATORS 32
#define MAX_INSIDE 64
#define MAX_QUEUE_PER_FLOOR 512
#define MAX_LOG_LINES 200000

/* 随机：xorshift64* */
typedef struct {
    uint64_t s;
} RNG;
static void rng_seed(RNG *r, uint64_t seed){ r->s = seed ? seed : 114514ULL; }
static double rng_r(RNG *r){
    uint64_t x = r->s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    r->s = x;
    uint64_t res = x * 2685821657736338717ULL;
    return (double)(res >> 11) / (double)(1ULL<<53);
}
static int rng_randint(RNG *r, int l, int h){
    double u = rng_r(r);
    return l + (int)floor(u*(h-l+1));
}

/*** 数据结构 ***/
typedef struct {
    int id;
    int inFloor, outFloor;
    int giveup;
    int arriveTick;
} Passenger;

typedef enum { IDLE=0, UP=1, DOWN=-1 } Dir;

typedef struct {
    int up[FLOORS+1];
    int dn[FLOORS+1];
    int car[FLOORS+1];
} Calls;

static void calls_clear(Calls *c){
    for(int i=0;i<=FLOORS;i++){ c->up[i]=c->dn[i]=c->car[i]=0; }
}
static bool calls_anyAbove(const Calls *c, int f){
    for(int i=f+1;i<=FLOORS;i++) if(c->up[i]||c->dn[i]||c->car[i]) return true;
    return false;
}
static bool calls_anyBelow(const Calls *c, int f){
    for(int i=1;i<f;i++) if(c->up[i]||c->dn[i]||c->car[i]) return true;
    return false;
}
static int calls_nearestAbove(const Calls *c, int f){
    for(int i=f+1;i<=FLOORS;i++) if(c->up[i]||c->dn[i]||c->car[i]) return i;
    return -1;
}
static int calls_nearestBelow(const Calls *c, int f){
    for(int i=f-1;i>=1;i--) if(c->up[i]||c->dn[i]||c->car[i]) return i;
    return -1;
}

/* 电梯 */
typedef struct {
    int id;
    int floor;
    Dir dir;
    bool doorOpen;
    int doorTimer;
    int moveTimer;
    int idleStay;
    Calls call;
    Passenger inside[MAX_INSIDE];
    int insideCount;
} Elevator;

/* 简单循环队列用于每层上/下行候客 */
typedef struct {
    Passenger data[MAX_QUEUE_PER_FLOOR];
    int head, tail, count; /* head/tail are indices modulo, count 为当前元素数 */
} PQueue;
static void pq_init(PQueue *q){ q->head = q->tail = q->count = 0; }
static int pq_size(const PQueue *q){ return q->count; }
static bool pq_empty(const PQueue *q){ return q->count == 0; }
static bool pq_push(PQueue *q, Passenger p){
    if(q->count >= MAX_QUEUE_PER_FLOOR) return false;
    q->data[q->tail] = p;
    q->tail = (q->tail + 1) % MAX_QUEUE_PER_FLOOR;
    q->count++;
    return true;
}
static bool pq_pop(PQueue *q, Passenger *out){
    if(q->count == 0) return false;
    *out = q->data[q->head];
    q->head = (q->head + 1) % MAX_QUEUE_PER_FLOOR;
    q->count--;
    return true;
}
static int pq_clear_and_count(PQueue *q){
    int c = q->count;
    q->head = q->tail = q->count = 0;
    return c;
}
static int pq_count(const PQueue *q){
    return q->count;
}

/* 日志缓冲（为与原行为一致，收集后输出） */
static char *logbuf[MAX_LOG_LINES];
static int logcnt = 0;
static void pushLogf(const char *fmt, ...){
    if(logcnt >= MAX_LOG_LINES) return;
    va_list ap;
    va_start(ap, fmt);
    char *buf = (char*)malloc(256);
    vsnprintf(buf, 256, fmt, ap);
    va_end(ap);
    logbuf[logcnt++] = buf;
}

/*** 仿真世界 ***/
typedef struct {
    int E;
    int totalTick;
    bool rush;
    RNG rng;
    Elevator elev[MAX_ELEVATORS];
    PQueue Qup[FLOORS+1], Qdn[FLOORS+1];
    int nextPid;
    /* openTime per elevator */
    int openTime[MAX_ELEVATORS];
} World;

static void world_init(World *w, int elevators, int T, bool rush_mode, uint64_t seed){
    w->E = (elevators < 1) ? 1 : (elevators > MAX_ELEVATORS ? MAX_ELEVATORS : elevators);
    w->totalTick = T;
    w->rush = rush_mode;
    rng_seed(&w->rng, seed ? seed : 20251020ULL);
    w->nextPid = 1;
    for(int i=0;i<w->E;i++){
        w->elev[i].id = i+1;
        w->elev[i].floor = LOBBY;
        w->elev[i].dir = IDLE;
        w->elev[i].doorOpen = false;
        w->elev[i].doorTimer = 0;
        w->elev[i].moveTimer = 0;
        w->elev[i].idleStay = 0;
        calls_clear(&w->elev[i].call);
        w->elev[i].insideCount = 0;
        w->openTime[i] = 0;
    }
    for(int f=0;f<=FLOORS;f++){ pq_init(&w->Qup[f]); pq_init(&w->Qdn[f]); }
}

/* 声明（后面实现） */
static void startMoveOneFloor(World *w, Elevator *e, int tick);
static void openDoor(World *w, Elevator *e, int tick);
static void closeDoor(World *w, Elevator *e, int tick);
static void arriveOneFloor(World *w, Elevator *e, int tick);
static void controllerChoose(World *w, Elevator *e, int tick);
static void handleDoorWork(World *w, Elevator *e, int tick);

/* 乘客到达 */
static void spawnPassengers(World *w, int tick){
    double baseProb = w->rush ? 0.20 : 0.04;
    if(rng_r(&w->rng) >= baseProb) return;
    int batch = w->rush ? rng_randint(&w->rng, 3, 8) : rng_randint(&w->rng, 1, 2);
    for(int c=0;c<batch;c++){
        int from,to;
        if(w->rush){
            bool upflux = rng_r(&w->rng) < 0.85;
            if(upflux){ from = 1; to = rng_randint(&w->rng, 2, FLOORS); }
            else { from = rng_randint(&w->rng, 2, FLOORS); to = 1; }
        } else {
            from = rng_randint(&w->rng, 1, FLOORS);
            do { to = rng_randint(&w->rng, 1, FLOORS); } while(to == from);
        }
        Passenger p;
        p.id = w->nextPid++;
        p.inFloor = from;
        p.outFloor = to;
        p.giveup = rng_randint(&w->rng, 180, 600);
        p.arriveTick = tick;
        if(to > from) pq_push(&w->Qup[from], p);
        else pq_push(&w->Qdn[from], p);
        /* 外呼灯 */
        for(int i=0;i<w->E;i++){
            if(to > from) w->elev[i].call.up[from] = 1;
            else w->elev[i].call.dn[from] = 1;
        }
        pushLogf("T%05d: P#%d arrives at F%d -> F%d", tick, p.id, from, to);
    }
}

/* 耐心衰减 */
static void decayPatience(World *w, int tick){
    for(int f=1; f<=FLOORS; ++f){
        /* 检查 UP 队列 */
        PQueue *Qu = &w->Qup[f];
        int size = pq_count(Qu);
        if(size>0){
            /* 采用简单重建队列方式 */
            Passenger tmp[MAX_QUEUE_PER_FLOOR];
            int newn = 0;
            while(!pq_empty(Qu)){
                Passenger p; pq_pop(Qu, &p);
                p.giveup--;
                if(p.giveup <= 0){
                    pushLogf("T%05d: P#%d gives up at F%d (UP)", tick, p.id, f);
                } else {
                    tmp[newn++] = p;
                }
            }
            for(int i=0;i<newn;i++) pq_push(Qu, tmp[i]);
            for(int i=0;i<w->E;i++) w->elev[i].call.up[f] = pq_empty(Qu) ? 0 : 1;
        }
        /* DN 队列 */
        PQueue *Qd = &w->Qdn[f];
        size = pq_count(Qd);
        if(size>0){
            Passenger tmp[MAX_QUEUE_PER_FLOOR];
            int newn = 0;
            while(!pq_empty(Qd)){
                Passenger p; pq_pop(Qd, &p);
                p.giveup--;
                if(p.giveup <= 0){
                    pushLogf("T%05d: P#%d gives up at F%d (DOWN)", tick, p.id, f);
                } else {
                    tmp[newn++] = p;
                }
            }
            for(int i=0;i<newn;i++) pq_push(Qd, tmp[i]);
            for(int i=0;i<w->E;i++) w->elev[i].call.dn[f] = pq_empty(Qd) ? 0 : 1;
        }
    }
}

/* ensureLobbyFeeder（最小修复） */
static void ensureLobbyFeeder(World *w, int tick){
    if(pq_empty(&w->Qup[LOBBY])) return;
    /* 是否已有电梯在服务 F1（在开门、门计时、或朝下并在移动） */
    for(int i=0;i<w->E;i++){
        Elevator *e = &w->elev[i];
        if((e->floor == LOBBY && (e->doorOpen || e->doorTimer > 0)) ||
           (e->dir == DOWN && e->floor > LOBBY && e->moveTimer > 0)) return;
    }
    /* 选择最近空闲电梯 */
    int best = -1;
    int bestDist = 1000000;
    for(int i=0;i<w->E;i++){
        Elevator *e = &w->elev[i];
        if(e->moveTimer == 0 && !e->doorOpen && e->doorTimer == 0 && e->dir == IDLE){
            if(e->floor == FLOORS) continue;
            int d = abs(e->floor - LOBBY);
            if(d < bestDist){ bestDist = d; best = i; }
        }
    }
    if(best != -1){
        Elevator *e = &w->elev[best];
        e->dir = (e->floor > LOBBY) ? DOWN : UP;
        startMoveOneFloor(w, e, tick);
        if(e->moveTimer > 0) pushLogf("T%05d: E#%d assign-to-lobby", tick, e->id);
    }
}

/* open/close/startMove/arrive 等 */
static void startMoveOneFloor(World *w, Elevator *e, int tick){
    if(e->dir == UP && e->floor >= FLOORS){
        pushLogf("T%05d: E#%d blocked at top F%d, ignore UP", tick, e->id, e->floor);
        e->dir = IDLE; return;
    }
    if(e->dir == DOWN && e->floor <= LOBBY){
        pushLogf("T%05d: E#%d blocked at bottom F%d, ignore DOWN", tick, e->id, e->floor);
        e->dir = IDLE; return;
    }
    e->idleStay = 0;
    e->moveTimer = (e->dir == UP ? MOVE_UP : MOVE_DOWN);
    pushLogf("T%05d: E#%d F%d -> %s (move %dt)", tick, e->id, e->floor, (e->dir==UP?"UP":"DOWN"), e->moveTimer);
}

static void openDoor(World *w, Elevator *e, int t){
    if(!e->doorOpen){
        e->doorOpen = true;
        e->doorTimer = DOOR_ACT;
        pushLogf("T%05d: E#%d F%d OPEN (%dt)", t, e->id, e->floor, DOOR_ACT);
    }
}
static void closeDoor(World *w, Elevator *e, int t){
    e->doorOpen = false;
    e->doorTimer = DOOR_ACT;
    pushLogf("T%05d: E#%d F%d CLOSE (%dt)", t, e->id, e->floor, DOOR_ACT);
}

static void arriveOneFloor(World *w, Elevator *e, int tick){
    if(e->dir == UP){
        if(e->floor < FLOORS) e->floor++;
        else { pushLogf("T%05d: E#%d illegal UP arrival at F%d, ignore", tick, e->id, e->floor); e->dir = IDLE; return; }
    } else if(e->dir == DOWN){
        if(e->floor > LOBBY) e->floor--;
        else { pushLogf("T%05d: E#%d illegal DOWN arrival at F%d, ignore", tick, e->id, e->floor); e->dir = IDLE; return; }
    } else return;
    pushLogf("T%05d: E#%d ARRIVE F%d", tick, e->id, e->floor);
    bool needStop = false;
    if(e->call.car[e->floor]) needStop = true;
    if(e->dir == UP && e->call.up[e->floor]) needStop = true;
    if(e->dir == DOWN && e->call.dn[e->floor]) needStop = true;
    if(needStop) openDoor(w, e, tick);
    else {
        bool ahead = (e->dir==UP ? calls_anyAbove(&e->call, e->floor) : calls_anyBelow(&e->call, e->floor));
        if(!ahead){
            bool behind = (e->dir==UP ? calls_anyBelow(&e->call, e->floor) : calls_anyAbove(&e->call, e->floor));
            e->dir = behind ? (e->dir==UP ? DOWN : UP) : IDLE;
        }
    }
}

/* 门处理：下客->上客，增加超时与每次上限 */
static void handleDoorWork(World *w, Elevator *e, int tick){
    int idx = e->id - 1;
    if(!e->doorOpen) return;
    w->openTime[idx]++;
    if(w->openTime[idx] > MAX_OPEN_TICKS){
        pushLogf("T%05d: E#%d timeout-close at F%d", tick, e->id, e->floor);
        closeDoor(w, e, tick);
        w->openTime[idx] = 0;
        return;
    }
    if(e->doorTimer > 0){ e->doorTimer--; return; }
    int dropCnt = 0;
    if(e->call.car[e->floor]){
        Passenger keep[MAX_INSIDE];
        int nk = 0;
        for(int i=0;i<e->insideCount;i++){
            Passenger *p = &e->inside[i];
            if(p->outFloor == e->floor){
                dropCnt++;
                e->doorTimer += PERSON_IO;
                pushLogf("T%05d: E#%d F%d  P#%d OUT (+%dt)", tick, e->id, e->floor, p->id, PERSON_IO);
            } else {
                keep[nk++] = *p;
            }
        }
        e->insideCount = 0;
        for(int i=0;i<nk;i++) e->inside[e->insideCount++] = keep[i];
        e->call.car[e->floor] = 0;
        if(dropCnt > 0) return;
    }
    PQueue *Q = (e->dir != DOWN) ? &w->Qup[e->floor] : &w->Qdn[e->floor];
    int take = 0;
    while(!pq_empty(Q) && take < 4 && e->insideCount < MAX_INSIDE){
        Passenger p;
        pq_pop(Q, &p);
        e->inside[e->insideCount++] = p;
        e->call.car[p.outFloor] = 1;
        e->doorTimer += PERSON_IO;
        take++;
        pushLogf("T%05d: E#%d F%d  P#%d IN ->F%d (+%dt)", tick, e->id, e->floor, p.id, p.outFloor, PERSON_IO);
    }
    if(pq_empty(Q)){
        for(int i=0;i<w->E;i++){
            if(e->dir != DOWN) w->elev[i].call.up[e->floor] = 0;
            else w->elev[i].call.dn[e->floor] = 0;
        }
    }
    if(dropCnt == 0 && take == 0){
        closeDoor(w, e, tick);
        w->openTime[idx] = 0;
    }
}

/* 推进单台电梯一步 */
static void stepElevator(World *w, Elevator *e, int tick){
    if(e->moveTimer > 0){
        e->moveTimer--;
        if(e->moveTimer == 0){
            arriveOneFloor(w, e, tick);
        }
        return;
    }
    if(e->doorOpen){ handleDoorWork(w, e, tick); return; }
    if(e->doorTimer > 0){ e->doorTimer--; if(e->doorTimer > 0) return; }
    controllerChoose(w, e, tick);
}

/* 控制器选择（带边界处理与最近任务判断） */
static void controllerChoose(World *w, Elevator *e, int tick){
    if(e->moveTimer > 0 || e->doorOpen || e->doorTimer > 0) return;
    if(e->floor == FLOORS && e->dir == UP) e->dir = DOWN;
    if(e->floor == 1 && e->dir == DOWN) e->dir = UP;
    if(e->call.car[e->floor] ||
       (e->dir != DOWN && e->call.up[e->floor]) ||
       (e->dir != UP && e->call.dn[e->floor])){
        openDoor(w, e, tick); return;
    }
    bool above = calls_anyAbove(&e->call, e->floor);
    bool below = calls_anyBelow(&e->call, e->floor);
    if(e->dir == IDLE){
        if(above && !below) e->dir = UP;
        else if(below && !above) e->dir = DOWN;
        else if(above && below){
            int na = calls_nearestAbove(&e->call, e->floor);
            int nb = calls_nearestBelow(&e->call, e->floor);
            if(na != -1 && nb != -1) e->dir = (abs(e->floor - na) < abs(e->floor - nb)) ? UP : DOWN;
            else if(na != -1) e->dir = UP;
            else if(nb != -1) e->dir = DOWN;
        } else {
            e->idleStay++;
            if(e->idleStay >= IDLE_BELL && e->floor != LOBBY){
                e->dir = (e->floor > LOBBY) ? DOWN : UP;
                startMoveOneFloor(w, e, tick);
                e->idleStay = 0;
                pushLogf("T%05d: E#%d recall to lobby", tick, e->id);
            }
            return;
        }
    }
    startMoveOneFloor(w, e, tick);
}

/* 主循环 run */
static void world_run(World *w){
    pushLogf("===== Simulation Start =====");
    for(int t=0;t<w->totalTick;t++){
        spawnPassengers(w, t);
        decayPatience(w, t);
        ensureLobbyFeeder(w, t);
        for(int i=0;i<w->E;i++) stepElevator(w, &w->elev[i], t);
    }
    pushLogf("===== Simulation End =====");
    for(int i=0;i<logcnt;i++){ puts(logbuf[i]); free(logbuf[i]); }
    int onboard = 0, waiting = 0;
    for(int i=0;i<w->E;i++) onboard += w->elev[i].insideCount;
    for(int f=1; f<=FLOORS; f++){
        waiting += pq_count(&w->Qup[f]);
        waiting += pq_count(&w->Qdn[f]);
    }
    fprintf(stderr, "[Stats] elevators=%d onboard=%d waiting=%d ticks=%d\n",
            w->E, onboard, waiting, w->totalTick);
}

/* main */
int main(int argc, char **argv){
    int E = 1;
    int T = SIM_MAX_T;
    bool rush = false;
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"--elevators")==0 && i+1<argc){ E = atoi(argv[++i]); if(E<1) E=1; if(E>MAX_ELEVATORS) E=MAX_ELEVATORS; }
        else if(strcmp(argv[i],"--ticks")==0 && i+1<argc){ T = atoi(argv[++i]); if(T < SIM_MIN_T) T = SIM_MIN_T; }
        else if(strcmp(argv[i],"--rush")==0) rush = true;
        else if(strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0){
            fprintf(stderr,"Usage: ./elevator_c [--elevators N] [--ticks T] [--rush]\n");
            return 0;
        }
    }
    World w;
    world_init(&w, E, T, rush, 20251020ULL);
    world_run(&w);
    return 0;
}