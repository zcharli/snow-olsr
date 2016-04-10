#include "Headers/TCMessage.h"

TCMessage::TCMessage() {}
TCMessage::~TCMessage() {}

char* TCMessage::serialize() {
	char* output = new char[4+14*mNeighborAddresses.size()];
	output[0] = ansn;

	int offset = 4;
	for (auto& na : mNeighborAddresses) {
		for (int i = 0; i < 14; ++i)
			output[offset+i] = na.data[i];
		offset += 14;
	}

	return output;
}