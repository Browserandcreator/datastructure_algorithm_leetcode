// main.cpp  -- 离散事件电梯仿真（必做 + 选做1：多梯，最小修复）
// C++17  单文件可编译运行
#include <bits/stdc++.h>
using namespace std;

/*** 基本参数（题目给定的时间常量） ***/
static constexpr int FLOORS = 5;         // 1..5
static constexpr int LOBBY  = 1;
static constexpr int TICK_MS = 100;      // 0.1 s 一个tick
static constexpr int DOOR_ACT = 20;      // 开/关门各 20t
static constexpr int PERSON_IO = 25;     // 单人进出耗时
static constexpr int MOVE_UP   = 51;     // 上一层 51t
static constexpr int MOVE_DOWN = 61;     // 下一层 61t
static constexpr int IDLE_BELL = 300;    // 原地 >=300t 回一层
static constexpr int SIM_MIN_T = 500;
static constexpr int SIM_MAX_T = 3000;

/*** 随机 ***/
struct RNG {
    mt19937_64 eng;
    uniform_real_distribution<double> U{0.0,1.0};
    RNG(uint64_t seed=114514) : eng(seed) {}
    double r(){ return U(eng); }
    int randint(int l,int r){ uniform_int_distribution<int> D(l,r); return D(eng); }
};

/*** 乘客、队列、呼叫 ***/
struct Passenger {
    int id;
    int inFloor, outFloor;
    int giveup;          // 耐心(剩余tick)，<=0 表示放弃
    int arriveTick;
};

enum Dir { IDLE=0, UP=1, DOWN=-1 };

struct Calls {
    // up[f]为当前层外呼叫标记，car[f]为当前层内下梯标记
    array<int,FLOORS+1> up{}, dn{}, car{};
    void clear(){ up.fill(0); dn.fill(0); car.fill(0); }
    // f之上是否存在任务
    bool anyAbove(int f) const {
        for(int i=f+1;i<=FLOORS;i++) if(up[i]||dn[i]||car[i]) return true;
        return false;
    }
    // f之下是否存在任务
    bool anyBelow(int f) const {
        for(int i=1;i<f;i++) if(up[i]||dn[i]||car[i]) return true;
        return false;
    }
};

struct Elevator {
    int id=1;
    int floor=LOBBY;
    Dir  dir=IDLE;
    bool doorOpen=false;
    int  doorTimer=0;     // >0 表示正在开/关门/进出
    int  moveTimer=0;     // >0 表示在楼层之间运动
    int  idleStay=0;      // 在当前楼层的静置计时
    Calls call;
    vector<Passenger> inside;
};

/*** 仿真环境 ***/
struct World {
    int E=1;                     // 电梯数量
    int totalTick = SIM_MAX_T;   // 仿真时长
    bool rush=false;             // 早高峰模式（保留但不强制）
    RNG rng;
    vector<Elevator> elev;
    // 队列数组，人多时维护每层的等待队列顺序
    array<deque<Passenger>,FLOORS+1> Qup, Qdn;
    int nextPid=1;
    vector<string> log;

    World(int elevators, int T, bool rush_mode, uint64_t seed=1)
        : E(max(1,elevators)), totalTick(T), rush(rush_mode), rng(seed) {
        elev.resize(E);
        for(int i=0;i<E;i++){ elev[i].id=i+1; elev[i].floor=LOBBY; }
    }

    static string fmt(const char* fmt, ...){
        char buf[512];
        va_list ap; va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        return string(buf);
    }
    void pushLog(const string& s){ log.emplace_back(s); }

    // 乘客到达（普通/早高峰）
    void spawnPassengers(int tick){
        // 是否高峰？
        double baseProb = rush? 0.20 : 0.04;          
        if(rng.r() >= baseProb) return;
        // batch为一tick一次生成数量数量
        int batch = rush? rng.randint(3,8) : rng.randint(1,2);
        for(int c=0;c<batch;c++){
            int from,to;
            if(rush){
                bool upflux = rng.r()<0.85;          // 高峰：多数 1到高层，少量反向
                if(upflux){ from=1; to=rng.randint(2,FLOORS); }
                else{ from=rng.randint(2,FLOORS); to=1; }
            }else{
                from=rng.randint(1,FLOORS);
                do{ to=rng.randint(1,FLOORS); }while(to==from);
            }
            Passenger p{nextPid++,from,to,rng.randint(180,600),tick};
            if(to>from) Qup[from].push_back(p);
            else        Qdn[from].push_back(p);
            // 外呼灯，辅助调度器决策
            for(auto &el: elev){
                if(to>from) el.call.up[from]=1;
                else        el.call.dn[from]=1;
            }
            // 乘客生成信息
            pushLog(fmt("T%05d: P#%d arrives at F%d -> F%d",tick,p.id,from,to));
        }
    }

    // 等候耐心衰减
    void decayPatience(int tick){
        // 遍历所有楼层，更新耐心，更新外呼灯
        for(int f=1; f<=FLOORS; ++f){
            auto drop = [&](deque<Passenger>& Q, const char* tag){
                for(auto it=Q.begin(); it!=Q.end(); ){
                    if(--it->giveup<=0){
                        pushLog(fmt("T%05d: P#%d gives up at F%d (%s)",tick,it->id,f,tag));
                        it = Q.erase(it);
                        if(tag[0]=='U')
                            for(auto &el: elev) el.call.up[f] = Q.empty()?0:1;
                        else
                            for(auto &el: elev) el.call.dn[f] = Q.empty()?0:1;
                    } else ++it;
                }
            };
            drop(Qup[f],"UP"); drop(Qdn[f],"DOWN");
        }
    }

    // —— 关键的“最小修复”：大厅保底策略
    // 若 F1 上行队列有人，且没有电梯正在去 F1，则派最近的idle电梯去 F1。
    void ensureLobbyFeeder(int tick){
        if(Qup[LOBBY].empty()) return;      // 一层没人等上行就不干预
        // 是否已经有人正去 F1（向下行、或正在 F1 开门）
        for(auto &e: elev){
            if((e.floor==LOBBY && (e.doorOpen || e.doorTimer>0)) ||
               (e.dir==DOWN && e.floor>LOBBY)) return;  // 有人在服务/在路上
        }
        // 选择最近的“空闲”电梯
        int best=-1,bestDist=1e9;
        for(int i=0;i<E;i++){
            auto &e=elev[i];
            if(e.moveTimer==0 && !e.doorOpen && e.doorTimer==0){
                int d=abs(e.floor-LOBBY);
                if(d<bestDist){ bestDist=d; best=i; }
            }
        }
        if(best!=-1){
            auto &e=elev[best];
            e.dir = (e.floor>LOBBY? DOWN: UP);
            startMoveOneFloor(e, tick);
            pushLog(fmt("T%05d: E#%d assign-to-lobby", tick, e.id));
        }
    }

    // 控制器：更新状态，准备移动
    void controllerChoose(Elevator& e, int tick){
        if(e.moveTimer>0 || e.doorTimer>0) return;
        // 当前层有下客
        if(!e.doorOpen && e.call.car[e.floor]){ openDoor(e,tick); return; }
        // 当前层有与方向一致的外呼
        if(e.dir==UP   && e.call.up[e.floor])   { openDoor(e,tick); return; }
        if(e.dir==DOWN && e.call.dn[e.floor])   { openDoor(e,tick); return; }

        bool anyAbove=e.call.anyAbove(e.floor), anyBelow=e.call.anyBelow(e.floor);
        // 超时回一楼
        if(e.dir==IDLE){
            if(anyAbove||anyBelow){
                int upDist=1e9,dnDist=1e9;
                for(int i=e.floor+1;i<=FLOORS;i++) if(e.call.up[i]||e.call.dn[i]||e.call.car[i]){upDist=i-e.floor;break;}
                for(int i=e.floor-1;i>=1;i--) if(e.call.up[i]||e.call.dn[i]||e.call.car[i]){dnDist=e.floor-i;break;}
                e.dir=(upDist<=dnDist?UP:DOWN);
            }else{
                e.idleStay++;
                if(e.idleStay>=IDLE_BELL && e.floor!=LOBBY){
                    e.dir=(e.floor>LOBBY?DOWN:UP);
                    startMoveOneFloor(e,tick);
                    pushLog(fmt("T%05d: E#%d idle>300t, recall to lobby",tick,e.id,e.floor));
                }
                return;
            }
        }
        if(!e.doorOpen) startMoveOneFloor(e,tick);
    }

    void startMoveOneFloor(Elevator& e, int tick){
        if(e.dir==UP && e.floor>=FLOORS){
            pushLog(fmt("T%05d: E#%d blocked at top F%d, ignore UP",tick,e.id,e.floor));
            e.dir=IDLE; return;
        }
        if(e.dir==DOWN && e.floor<=LOBBY){
            pushLog(fmt("T%05d: E#%d blocked at bottom F%d, ignore DOWN",tick,e.id,e.floor));
            e.dir=IDLE; return;
        }
        e.idleStay=0;
        e.moveTimer=(e.dir==UP?MOVE_UP:MOVE_DOWN);
        pushLog(fmt("T%05d: E#%d F%d -> %s (move %dt)",tick,e.id,e.floor,(e.dir==UP?"UP":"DOWN"),e.moveTimer));
    }
    void openDoor(Elevator& e,int t){ 
        if(!e.doorOpen){ 
            e.doorOpen=true; 
            e.doorTimer=DOOR_ACT; 
            pushLog(fmt("T%05d: E#%d F%d OPEN (%dt)",t,e.id,e.floor,DOOR_ACT));
        }
    }
    void closeDoor(Elevator& e,int t){ 
        e.doorOpen=false; e.doorTimer=DOOR_ACT; 
        pushLog(fmt("T%05d: E#%d F%d CLOSE (%dt)",t,e.id,e.floor,DOOR_ACT));
    }

    // 电梯到站后的判断：落客/接客触发点
    void arriveOneFloor(Elevator& e, int tick){
        // 更新楼层
        if(e.dir==UP) e.floor++; 
        else if(e.dir==DOWN) e.floor--; 
        else return;
        pushLog(fmt("T%05d: E#%d ARRIVE F%d",tick,e.id,e.floor));
        
        // 到站判断是否开门
        bool needStop=false;
        if(e.call.car[e.floor]) needStop=true;
        if(e.dir==UP   && e.call.up[e.floor]) needStop=true;
        if(e.dir==DOWN && e.call.dn[e.floor]) needStop=true;
        if(needStop) openDoor(e,tick);
        
        // 若不开门，下一步 
        else{
            bool ahead=(e.dir==UP?e.call.anyAbove(e.floor):e.call.anyBelow(e.floor));
            if(!ahead){
                bool behind=(e.dir==UP?e.call.anyBelow(e.floor):e.call.anyAbove(e.floor));
                e.dir = behind? (e.dir==UP?DOWN:UP) : IDLE;
            }
        }
    }

    // 门开期间：先下客，再同向上客
    void handleDoorWork(Elevator& e, int tick){
        if(!e.doorOpen) return;
        if(e.doorTimer>0){ e.doorTimer--; return; } // 开门动画/累积的上下客时间

        int dropCnt=0;
        if(e.call.car[e.floor]){
            vector<Passenger> keep;
            for(auto &p: e.inside){
                if(p.outFloor==e.floor){
                    dropCnt++; e.doorTimer+=PERSON_IO;
                    pushLog(fmt("T%05d: E#%d F%d  P#%d OUT (+%dt)",tick,e.id,e.floor,p.id,PERSON_IO));
                }else keep.push_back(p);
            }
            e.inside.swap(keep);
            e.call.car[e.floor]=0;
            if(dropCnt>0) return; // 先把“下客时间”走完
        }

        // 同向上客
        deque<Passenger> &Q = (e.dir!=DOWN? Qup[e.floor] : Qdn[e.floor]);
        int take=0;
        while(!Q.empty()){
            Passenger p = Q.front(); Q.pop_front();
            e.inside.push_back(p);
            e.call.car[p.outFloor]=1;
            e.doorTimer += PERSON_IO;
            take++;
            pushLog(fmt("T%05d: E#%d F%d  P#%d IN ->F%d (+%dt)",tick,e.id,e.floor,p.id,p.outFloor,PERSON_IO));
        }
        // 若该方向队列清空则熄灭该方向外呼状态
        if(e.dir!=DOWN){ for(auto &el: elev) el.call.up[e.floor]=Qup[e.floor].empty()?0:1; }
        else           { for(auto &el: elev) el.call.dn[e.floor]=Qdn[e.floor].empty()?0:1; }

        // 无人上下，关门
        if(dropCnt==0 && take==0) closeDoor(e,tick);
    }

    // 推进一台电梯
    void stepElevator(Elevator& e, int tick){
        if(e.moveTimer>0){ 
            e.moveTimer--; 
            if(e.moveTimer==0) 
            arriveOneFloor(e,tick); return; 
        }
        if(e.doorOpen){ handleDoorWork(e,tick); return; }
        if(e.doorTimer>0){ 
            e.doorTimer--; 
            if(e.doorTimer>0) 
            return; 
        }
        controllerChoose(e,tick);
    }

    // 主循环
    void run(){
        pushLog("===== Simulation Start =====");
        for(int t=0;t<totalTick;t++){
            spawnPassengers(t);
            decayPatience(t);
            ensureLobbyFeeder(t);              // 最小干预：保证有人回一层接客
            for(int i=0;i<E;i++) stepElevator(elev[i],t);
        }
        pushLog("===== Simulation End =====");
        for(auto &s: log) cout<<s<<"\n";
        int onboard=0, waiting=0;
        for(auto &e: elev) onboard += (int)e.inside.size();
        for(int f=1;f<=FLOORS;f++) waiting += (int)Qup[f].size()+(int)Qdn[f].size();
        cerr<<"[Stats] elevators="<<E
            <<" onboard="<<onboard<<" waiting="<<waiting
            <<" ticks="<<totalTick<<"\n";
    }
};

/*** 命令行解析 ***/
int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int E = 1;
    int T = SIM_MAX_T;
    bool rush=false;
    for(int i=1;i<argc;i++){
        string a = argv[i];
        if(a=="--elevators" && i+1<argc) { E = max(1, atoi(argv[++i])); }
        else if(a=="--ticks" && i+1<argc) { T = max(SIM_MIN_T, atoi(argv[++i])); }
        else if(a=="--rush") rush=true;
        else if(a=="-h"||a=="--help"){
            cerr<<"Usage: ./elevator [--elevators N] [--ticks T] [--rush]\n";
            return 0;
        }
    }
    World w(E,T,rush, 20251020);
    w.run();
    return 0;
}
