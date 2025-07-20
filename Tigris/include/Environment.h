////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////



#pragma once


#include "./AI.h"


namespace tigris{


	struct Environment{
		public:
			Environment(size_t total_population, evo::ArrayProxy<size_t> _dimentions)
				: totalPopulation(total_population), dimentions(_dimentions.begin(), _dimentions.end()) {}

			auto initRandom() -> void {
				this->population.reserve(this->totalPopulation);
				for(size_t i = 0; i < this->totalPopulation; i+=1){
					this->population.emplace_back(this->dimentions);
				}
			}


			auto beginGame() -> void {
				this->scores = std::vector<float>(this->totalPopulation, 0.0f);
			}

			auto setScoresToReproductionChance() -> void {
				float score_sum = 0.0f;
				for(float& score : this->scores){
					score *= score;
					score_sum += score;
				}

				for(float& score : this->scores){
					score /= score_sum;
				}
			}


			auto createNewPopulation(float mutation_rate, float num_new_random) -> void {
				auto new_population = std::vector<AI>();
				new_population.reserve(this->totalPopulation);

				const size_t best_index_from_last_run = std::distance(
					this->scores.begin(), std::max_element(this->scores.begin(), this->scores.end())
				);
				new_population.emplace_back(this->population[best_index_from_last_run]); // keep the best one

				for(size_t i = 0; i < num_new_random; i+=1){
					new_population.emplace_back(this->dimentions);
				}


				size_t target_index = 0;
				while(new_population.size() < totalPopulation){
					if(this->scores[target_index] <= evo::random01()){
						new_population.emplace_back(this->population[target_index]);

						new_population.back().mutate(mutation_rate);
					}

					target_index += 1;
					if(target_index >= totalPopulation){ target_index = 0; }
				}

				this->population = std::move(new_population);
			}


		public:
			size_t totalPopulation;
			evo::SmallVector<size_t> dimentions;
			std::vector<AI> population{};
			std::vector<float> scores{};
	};

	
}