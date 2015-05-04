# dispatch++
A small C++11 dispatch/signals design that can be used with std::bind, lambdas, static functions, and/or member functions. It uses C# style `+=` and `-=` listener syntax. 

## Example

    struct test_signal : public signal {
        int a;
        test_signal(int _a) : signal(), a(_a) { }
    };

    void dispatch_ts(dispatcher& d, const test_signal& ts) {
        d.dispatch(ts);
    }

    void do_so(const test_signal& ts) {
        std::cout << "do_so: " << ts.a << std::endl;
    }

    void test_dispatch() {
        struct dispatch_test {
            void do_something(test_signal a) { 
                std::cout << "dispatch_test.do_something(" << a.a << ")" << std::endl; 
            }
        };

        struct static_func_test {
            static void static_do_something(test_signal& a) { 
                std::cout << "static_func_test.static_do_something(" << a.a << ")" << std::endl; 
            }
        };

        dispatch_test dt;
        test_signal tst(15);

        dispatcher d;
        auto l1 = [](test_signal ts) { 
            std::cout << "value: " << ts.a << std::endl; 
        };
        auto l2 = [](test_signal&& v) { 
            std::cout << "v: " << v.a << std::endl; 
        };
        auto l3 = [](const test_signal& t) { 
            std::cout << "t: " << t.a << std::endl; 
        };
        auto tmp = std::bind(&dispatch_test::do_something, &dt, std::placeholders::_1);
        auto tmp2 = static_func_test::static_do_something;
        auto tmp3 = std::bind(do_so, std::placeholders::_1);

        test_signal ts(323);
        test_signal tsp(23);
        test_signal tone(111);

        d += l1;
        d += l2;
        d += l3;
        d += tmp3;
        d += tmp;

        d.dispatch(ts);
        std::cout << "---\n";

        d -= l2;
        d -= tmp;

        d.dispatch(tsp);
        std::cout << "---\n";

        d -= l3;
        d += tmp2;
        d -= tmp3;

        //d.dispatch(tone);
        dispatch_ts(d, tone);
        std::cout << "---\n";
        
        d -= l1;
        d += tmp;
        d -= tmp2;

        d.dispatch(test_signal{ 5 });
        std::cout << "---\n";
    }
