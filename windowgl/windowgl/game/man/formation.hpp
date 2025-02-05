#pragma once

#include <glm\glm\ext\vector_float3.hpp>

#include <vector>
#include <game\cmp\ia.hpp>
#include <ecs/man/entitymanager.hpp>
#include <game\icmp\pattern.hpp>
#include <game/util/log.hpp>

namespace GM {
	struct FormationManager {
		bool addCharacter(IA_t& ia, std::vector<IA_t>& assignments);
		void removeCharacter(IA_t& ia, std::vector<IA_t>& assignments);
		void updateSlots(std::vector<IA_t>& assignments);
		void updateSlot(IA_t& a);
		void ressetAnchorsToCenterOfMass(const std::vector<IA_t>& assignments, const ECS::EntityManager_t& em);

		Pattern& createPattern(ECS::EntityManager_t& em);
		Pattern& getPattern(unsigned int index) {
			if (index >= patterns.size()) {
				Log::log("ERROR: Pattern& FormationManager::getPattern(...) Index (" + std::to_string(index) + ") out of range");
			}
			else {
				return patterns[index];
			}
		}
		unsigned int patternCount = 0;

	private:
		std::vector<Pattern> patterns;
		void setTarget(Target& target, const Location& location);
	};
}