#include "Headers/Message.h"

Message::Message() {

}

Message::~Message() {}

char* Message::serialize() {}

unsigned char Message::getType() {
    return type;
}
