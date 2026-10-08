#include "../include/RecipeController.h"
#include <cassert>
int main() {
  RecipeController r;
  // Nonempty start is rejected.
  r.update(0, 1.0f, true, true);
  assert(!r.start(10, 5, 100, 1.0f, true));
  r.update(200, 0, true, true);
  r.update(3300, 0, true, true);
  assert(r.emptyConfirmed());
  assert(r.start(10, 5, 3400, 0, true));
  assert(r.state()==RecipeController::State::Filling && r.relay1() && !r.relay2());
  r.update(4000, 10, true, true);
  assert(!r.relay1() && !r.relay2() && r.state()==RecipeController::State::Settle);
  r.update(6100, 10, true, true);
  assert(r.relay2() && !r.relay1());
  r.update(11101, 10, true, true);
  assert(r.state()==RecipeController::State::Completed && !r.relay1() && !r.relay2());
  assert(!r.start(10, 2, 12000, 10, true));
  assert(!r.acknowledge(10, true));
  assert(r.acknowledge(0, true));
  // Sensor loss stops filling.
  r.update(13000, 0, true, true);
  r.update(16100, 0, true, true);
  assert(r.start(10, 2, 16200, 0, true));
  r.update(16400, 0, false, true);
  assert(r.state()==RecipeController::State::Aborted && !r.relay1() && !r.relay2());
  // Reject recipes beyond tank capacity.
  RecipeController x;
  x.update(0,0,true,true);
  x.update(3100,0,true,true);
  assert(!x.start(20, 1, 3200, 0, true));
  return 0;
}
