#include "hexregister.h"
#include "manager.h"

namespace esphome {
namespace m3_vedirect {

HexRegister *HexRegister::cascade_dispatcher_(HexRegister *hex_register) {
  auto dispatcher = new HexRegisterDispatcher();
  dispatcher->cascade_dispatcher_(this);
  dispatcher->cascade_dispatcher_(hex_register);
  return dispatcher;
}

}  // namespace m3_vedirect
}  // namespace esphome