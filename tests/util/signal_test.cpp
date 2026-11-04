// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/util/signal.h>
using namespace easy3d;
int main() { return test::run("signal connections, forwarding and disconnect",[] {
    Signal<int> signal,forward;int total=0,other=0;
    const int first=signal.connect([&](int v){total+=v;});
    signal.connect(&forward);forward.connect([&](int v){other+=v;});
    signal.send(3);test::require(total==3 && other==3,"signal delivery differs");
    signal.disconnect(first);signal.send(2);test::require(total==3 && other==5,"disconnect failed");
    forward.disconnect_all();signal.send(1);test::require(other==5,"disconnect_all failed");
    Signal<int> copy(signal);copy.send(7);test::require(other==5,"copied signal retained connections");
}); }
