
#include "flat_fsm.h"

namespace flat {


    void Second::onEnter() {
    count1++;
    machine_.context().is_valid(true);
    machine_.context().value(machine_.context().value() + 1);
  }

void Third::onEnter(const event2 &ev) {
    count1++;
    machine_.context().is_valid(true);
    machine_.context().value(machine_.context().value() + ev.value_);
  }

}