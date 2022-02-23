#include "threadpool.h"
#include <thread>
#include <future>

class ThreadpoolExecution {
public:
    template<typename Func,
             typename Ret = std::invoke_result_t<Func>>
    static inline auto AsyncSubmit(ThreadPool * tp, Func&& func) {
        if constexpr(std::is_same_v<Ret, void>) {
            auto p = std::make_unique<std::promise<void>>();
            //auto p = std::make_unique<seastar::promise<>>();
            auto future = p->get_future();
            //auto coreid = seastar::engine().cpu_id();
	    auto coreid = 0;
            tp->Submit(std::move(func),
                [p = std::move(p), coreid](std::exception_ptr e) mutable {
                    //seastar::alien::submit_to(coreid, [p = std::move(p), e]() mutable {
                        if (e) {
                            p->set_exception(e);
                        } else {
                            p->set_value();
                        }
                        //return seastar::make_ready_future<>();
                   // });
                }
            );
            return future;
        } else {
            auto p = std::make_unique<std::promise<Ret>>();
            //auto p = std::make_unique<seastar::promise<Ret>>();
            auto future = p->get_future();
            //auto coreid = seastar::engine().cpu_id();
	auto coreid = 0;
            tp->Submit(std::move(func),
                [p = std::move(p), coreid](Ret r, std::exception_ptr e) mutable {
                    //seastar::alien::submit_to(coreid, [r = std::move(r), p = std::move(p), e]() mutable {
                        if (e) {
                            p->set_exception(e);
                        } else {
                            p->set_value(std::move(r));
                        }
                        //return seastar::make_ready_future<>();
                    //});
                }
            );
            return future;
        }
    }
};

class ThreadPoolSingleton {
public:

    static void Init(int n){
        GetThreadPoolSingleton().reset(new ThreadPool(n));
        std::cout <<"Init"<< std::endl;
    }
    static std::shared_ptr<ThreadPool>& GetThreadPoolSingleton(){
				std::cout <<"get static threadpool"<< std::endl;
        static std::shared_ptr<ThreadPool> tp;
        return tp;
    }

    template<typename Func,
             typename Ret = std::invoke_result_t<Func>>
    static inline auto AsyncSubmit(Func &&func) {
        auto tp = GetThreadPoolSingleton();
        return ThreadpoolExecution::AsyncSubmit(tp.get(), std::move(func));
    }
};
int main(){
    ThreadPoolSingleton::Init(10);
    auto f = ThreadPoolSingleton::AsyncSubmit([](){int a=1,b=2;return a + b;});
    std ::cout <<  f.get() << std::endl;;
    std::async([](){std::this_thread::sleep_for(std::chrono::seconds(200));});
}