#include "Headers/TCMessage.h"

TCMessage::TCMessage() {}
TCMessage::~TCMessage() {}

std::vector<int> TCMessage::serialize() {
	std::vector<int> output;
	output.push_back(ansn);

	for (auto& na : mNeighborAddresses)
		for (int i = 0; i < 14; ++i)
			output.push_back((int) na.data[i]);

	return output;
}
