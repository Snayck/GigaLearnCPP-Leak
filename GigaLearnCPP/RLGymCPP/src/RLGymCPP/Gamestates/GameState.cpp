#include "GameState.h"

#include "../Math.h"

using namespace RLGC;

// No longer needed - we use arena boost pads directly in their natural order

void RLGC::GameState::ResetBeforeStep() {
	for (auto& player : players)
		player.ResetBeforeStep();
}

void RLGC::GameState::UpdateFromArena(Arena* arena, const std::vector<Action>& actions, GameState* prev) {
	this->prev = prev;
	if (prev)
		prev->prev = NULL;

	lastArena = arena;
	int tickSkip = RS_MAX(arena->tickCount - lastTickCount, 0);
	deltaTime = tickSkip * (1 / 120.f);

	ball = arena->ball->GetState();

	players.resize(arena->_cars.size());

	auto carItr = arena->_cars.begin();
	for (int i = 0; i < players.size(); i++) {
		auto& player = players[i];
		player.index = i;
		player.UpdateFromCar(*carItr, arena->tickCount, tickSkip, actions[i], prev ? &prev->players[i] : NULL);
		if (player.ballTouchedStep)
			lastTouchCarID = player.carId;

		carItr++;
	}

	// Update boost pads - use arena's natural ordering
	int numBoostPads = arena->_boostPads.size();
	boostPads.resize(numBoostPads);
	boostPadsInv.resize(numBoostPads);
	boostPadTimers.resize(numBoostPads);
	boostPadTimersInv.resize(numBoostPads);

	for (int i = 0; i < numBoostPads; i++) {
		// Normal order for blue team
		auto state = arena->_boostPads[i]->GetState();
		boostPads[i] = state.isActive;
		boostPadTimers[i] = state.cooldown;

		// Inverted order for orange team (reverse index)
		auto stateInv = arena->_boostPads[numBoostPads - i - 1]->GetState();
		boostPadsInv[i] = stateInv.isActive;
		boostPadTimersInv[i] = stateInv.cooldown;
	}

	// Update goal scoring
	// If you don't have a GoalScoreCondition then that's not my problem lmao
	goalScored = arena->IsBallScored();

	lastTickCount = arena->tickCount;
}
