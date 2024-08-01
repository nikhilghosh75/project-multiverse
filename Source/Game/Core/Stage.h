#pragma once
#include <vector>

/*
* Stages are the entry point of the codebase, and represents what happens in a single update call
* Stages exist on a stack (i.e. a pause menu might go on top of a combat state)
*/
class Stage
{
public:
	char* name;

	virtual void Update() = 0;

	virtual void OnStateAdd() {}
	virtual void OnStateRemove() {}
};

class StageManager
{
	static const int MAX_STAGES = 32;

public:
	static void Initialize();

	static void AddStage(Stage* stage);

	static void PopStage();

	static void Update();

private:
	static inline std::vector<Stage*> stages;

	static inline Stage* nextStage;

	static inline int popBackCount = 0;
};