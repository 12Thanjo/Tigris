////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////



#pragma once


#include "./Matrix.h"


namespace tigris{


	inline auto sigmoid(float value) -> float {
		return 1.0f / (1.0f + std::exp(-value));
	}


	class AI{
		public:
			AI(evo::ArrayProxy<size_t> dimentions) {
				evo::debugAssert(dimentions.size() >= 2, "must have at least 2 dimentions");

				this->matrices.reserve(dimentions.size() - 1);
				for(size_t i = 0; i < dimentions.size() - 1; i+=1){
					this->matrices.emplace_back(Matrix::random(dimentions[i + 1], dimentions[i]));
				}
			}

			~AI() = default;


			auto calculate(std::vector<float>&& inputs) const -> Matrix {
				auto output = tigris::Matrix(this->matrices[0].height(), 1, std::move(inputs));

				for(size_t i = 0; const Matrix& matrix : this->matrices){
					output *= matrix;

					if(i + 1 < this->matrices.size()){
						for(float& value : output.data()){
							value = sigmoid(value);
						}
					}

					i += 1;
				}

				return output;
			}


			auto mutate(float mutation_rate) -> void {
				for(Matrix& matrix : this->matrices){
					for(float& value : matrix.data()){
						if(mutation_rate < float(evo::random01())){ continue; }
						value = float(evo::random01());
					}
				}
			}
	
		private:
			std::vector<Matrix> matrices{};
	};

	
}