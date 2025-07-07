#pragma once

#include <map>

#include "main.hpp"

class NameTracker {
   public:
	// Use rng for mapping names. rng will be deleted when nametracker is destroyed.
	NameTracker(Random* rng);
	~NameTracker();

	void callItem(Actor* itemActor, std::string callName);

	void identifyItem(Actor* itemActor);

	const char* getDisplayName(const Actor* itemActor);

	const char* getDescription(const Actor* itemActor);

	static constexpr auto MAX_CALL_STRING_LENGTH = 30;

   protected:
	Random* rng;

	std::map<std::string, bool> identifyStatus;
	std::map<std::string, std::string> callNames;
	std::map<char, std::map<std::string, std::string>> defaultNames;
	std::map<std::string, std::string> descriptions;
};
