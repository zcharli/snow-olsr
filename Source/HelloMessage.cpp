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

std::vector<int> HelloMessage::serialize() {
	std::vector<int> output;
	output.push_back(3); // default willingness.

	for (auto& lm : mLinkMessages) {
		output.push_back((int) lm.linkCode);
		output.push_back(14*lm.neighborIfAddr.size());

		for (auto& nia : lm.neighborIfAddr) {
			for (int i = 0; i < 14; ++i)
				output.push_back((int) nia.data[i]);
		}
	}

	return output;
}
