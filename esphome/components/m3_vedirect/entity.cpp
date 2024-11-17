#include "entity.h"

namespace esphome {
namespace m3_vedirect {

Entity::build_entity_func_t Entity::BUILD_ENTITY_FUNC[Platform_COUNT] = {};

void Entity::dynamic_init_entity_(EntityBase *entity, const char *name, const char *object_id, const char *manager_name,
                                  const char *manager_id) {
  // Helper for build_entity. Need a 'static' trick since Entity is not defined as an EntityBase
  if (manager_name) {
    char *entity_name = new char[strlen(manager_name) + strlen(name) + 2];
    sprintf(entity_name, "%s.%s", manager_name, name);
    name = entity_name;
  }
  entity->set_name(name);
  char *entity_object_id = new char[strlen(manager_id) + strlen(object_id) + 2];
  sprintf(entity_object_id, "%s_%s", manager_id, object_id);
  entity->set_object_id(entity_object_id);
}

void Entity::update_platforms() {
  // This code will run after initial setup and will check if any platform
  // is missing. When this happens, it'll try to use the most close platform
  // implementation available generally falling back to TextSensor which is
  // able to manage any kind of data though. If nothing better, it will just
  // install Entity::build_entity which will just work as a stub since Entity
  // itself doesn't implement any useful behavior for parsing
  if (!BUILD_ENTITY_FUNC[TextSensor]) {
    BUILD_ENTITY_FUNC[TextSensor] = Entity::build_entity;
  }
  if (!BUILD_ENTITY_FUNC[Select]) {
    BUILD_ENTITY_FUNC[Select] = BUILD_ENTITY_FUNC[TextSensor];
  }
  if (!BUILD_ENTITY_FUNC[BinarySensor]) {
    BUILD_ENTITY_FUNC[BinarySensor] = BUILD_ENTITY_FUNC[TextSensor];
  }
  if (!BUILD_ENTITY_FUNC[Switch]) {
    BUILD_ENTITY_FUNC[Switch] = BUILD_ENTITY_FUNC[BinarySensor];
  }
  if (!BUILD_ENTITY_FUNC[Sensor]) {
    BUILD_ENTITY_FUNC[Sensor] = BUILD_ENTITY_FUNC[TextSensor];
  }
  if (!BUILD_ENTITY_FUNC[Number]) {
    BUILD_ENTITY_FUNC[Number] = BUILD_ENTITY_FUNC[Sensor];
  }
}

const char *NumericEntity::UNIT_TO_DEVICE_CLASS[REG_DEF::UNIT::UNIT_COUNT] = {
    nullptr, "current", "voltage", "apparent_power", "power", nullptr, "energy", "battery", "duration", "temperature",
};

}  // namespace m3_vedirect
}  // namespace esphome
