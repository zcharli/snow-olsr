#include "Headers/HelloMessage.h"

HelloMessage::HelloMessage() {}
HelloMessage::~HelloMessage() {}

uint8_t HelloMessage::getWillingness() {
    std::cout << "HelloMessage::getWillingness not yet implemented" << std::endl;
    return 0;
}

std::vector<HelloMessage::LinkMessage> HelloMessage::getLinkMessages() const {
    return mLinkMessages;
}

char* HelloMessage::serialize() {
	int total_size = 0;
	total_size += 8;
	for (auto& lm : mLinkMessages)
		total_size += (4 + 14*lm.neighborIfAddr.size());

	char* output = new char[total_size];
	output[3] = 3; // default willingness.

	int offset = 4;
	for (auto& lm : mLinkMessages) {
		output[offset] = lm.linkCode;
		output[offset+2] = 14*lm.neighborIfAddr.size();

		offset += 4;
		for (auto& nia : lm.neighborIfAddr) {
			for (int i = 0; i < 14; ++i)
				output[offset+i] = nia.data[i];
			offset += 14;
		}
	}

	return output;
}