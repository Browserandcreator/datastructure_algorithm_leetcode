#include "elevator_core.hpp"

/********** 单梯电梯结构（简单版） **********/
struct Elevator {
    int floor = LOBBY;
    Dir dir = IDLE;
    bool doorOpen = false;
    int doorTimer = 0;      // 开/关门/进出动作剩余 tick
    int moveTimer = 0;      // 移动剩余 tick
    int idleStay = 0;
    vector<Passenger> inside;
};

static inline int sgn(int x){ return (x>0)-(x<0); }

/********** 随机乘客生成器（示例） **********/
struct Generator {
    mt19937 rng; uniform_real_distribution<double> U;
    Generator(int seed=42): rng(seed), U(0.0,1.0) {}
    void maybe_spawn(int tick, vector<Passenger> waiting[], int floors, int &pid, double prob=0.03){
        if (U(rng) < prob){
            int cnt = 1 + (rng()%2);
            while(cnt--){
                int from = rng()%floors;        // 0..floors-1
                int to = rng()%floors; while(to==from) to=rng()%floors;
                Passenger p{pid++, tick, from, to};
                waiting[from].push_back(p);
                cout<<fixed<<setprecision(1)
                    <<"["<<tick*0.1<<"s] P#"<<p.id<<" 产生 F"<<from<<"->F"<<to<<"\n";
            }
        }
    }
};

/********** 单梯仿真主逻辑（5 层演示） **********/
int main(int argc, char** argv){
    ios::sync_with_stdio(false); cin.tie(nullptr);

    const int FLOORS = 5;           // 0..4, 其中 1 为大厅
    const int T_IDLE_BACK = 300;    // 超过即回 1 层候命
    int T = 5000;                   // 总 tick
    if (argc>=2) T = max(1, atoi(argv[1]));

    vector<Passenger> waiting[FLOORS];
    Elevator e;
    Generator gen(42);
    int nextPid=1;

    ArrayStack<int> st; // 手写栈：用于一次开门时的进/出临时处理

    cout<<"=== Single Elevator Simulation (ticks="<<T<<", tick=0.1s) ===\n";
    for (int tick=0; tick<=T; ++tick){
        // 1) 随机生客
        gen.maybe_spawn(tick, waiting, FLOORS, nextPid);

        // 2) 运动或动作推进
        if (e.moveTimer>0){
            if (--e.moveTimer==0){
                e.floor += (e.dir==UP?+1:-1);
                e.idleStay = 0;
                cout<<"["<<tick*0.1<<"s] 到达 F"<<e.floor<<" dir="<<dirStr(e.dir)<<"\n";
                e.doorOpen=true; e.doorTimer=T_OPEN;   // 到站开门
                cout<<"["<<tick*0.1<<"s] 开门\n";
            }
            continue;
        }

        if (e.doorOpen){
            if (e.doorTimer>0){                      // 正在开/关门或进出
                if (--e.doorTimer==0){
                    // 本 tick 处理一次“出、上、检查”的事务
                    // 出：目的层==当前层的人
                    st = ArrayStack<int>();
                    for(int i=0;i<(int)e.inside.size();++i)
                        if (e.inside[i].to==e.floor) st.push(i);
                    if(!st.empty()){
                        int idx=st.pop();
                        auto p=e.inside[idx];
                        e.inside.erase(e.inside.begin()+idx);
                        cout<<"["<<tick*0.1<<"s] P#"<<p.id<<" 出电梯@F"<<e.floor<<"\n";
                        e.doorTimer=T_INOUT; // 一人出
                    }else{
                        // 上：随便上（示例）
                        auto &Q=waiting[e.floor];
                        if(!Q.empty()){
                            auto p=Q.back(); Q.pop_back();
                            e.inside.push_back(p);
                            e.dir = (sgn(p.to-p.from)>0?UP:DOWN);
                            cout<<"["<<tick*0.1<<"s] P#"<<p.id<<" 上电梯, dir="<<dirStr(e.dir)<<"\n";
                            e.doorTimer=T_INOUT;
                        }else{
                            // 无人 → 关门
                            e.doorTimer=T_CLOSE;
                            cout<<"["<<tick*0.1<<"s] 关门\n";
                        }
                    }
                }
            }else{
                // 门已关完：出发
                e.doorOpen=false;
                if(!e.inside.empty()){
                    e.moveTimer=T_MOVE_ONE;
                    cout<<"["<<tick*0.1<<"s] 出发 dir="<<dirStr(e.dir)<<"\n";
                }else{
                    e.dir=IDLE;
                }
            }
            continue;
        }

        // 3) 决策：若有需求→移动或开门；否则 idle/back to lobby
        e.idleStay++;
        bool hasDemand=false;
        for (int f=0; f<FLOORS && !hasDemand; ++f) hasDemand |= !waiting[f].empty();
        if (!hasDemand && e.idleStay>T_IDLE_BACK && e.floor!=LOBBY){
            e.dir = (e.floor<LOBBY?UP:DOWN); e.moveTimer=T_MOVE_ONE;
            cout<<"["<<tick*0.1<<"s] 空闲过久，返回 1 层\n";
        }else if (hasDemand){
            int bestF=-1, bestDist=1e9;
            for(int f=0; f<FLOORS; ++f) if(!waiting[f].empty()){
                int d=abs(f-e.floor); if(d<bestDist){bestDist=d; bestF=f;}
            }
            if(bestF==e.floor){ e.doorOpen=true; e.doorTimer=T_OPEN; e.idleStay=0; cout<<"["<<tick*0.1<<"s] 需求层开门\n";}
            else { e.dir=(bestF>e.floor?UP:DOWN); e.moveTimer=T_MOVE_ONE; }
        }
    }
    cout<<"=== End ===\n";
    return 0;
}
