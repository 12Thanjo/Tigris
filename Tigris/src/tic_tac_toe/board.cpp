////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include <tic_tac_toe/board.h>

namespace tigris::tic_tac_toe{


	auto Board::placeX(Coordinate coord) -> void {
		Space& space = this->operator[](coord);
		evo::debugAssert(space == Space::EMPTY, "space is not empty");
		space = Space::X;
	}

	auto Board::placeO(Coordinate coord) -> void {
		Space& space = this->operator[](coord);
		evo::debugAssert(space == Space::EMPTY, "space is not empty");
		space = Space::O;
	}



	auto Board::getPossibleMovesForX() const -> std::vector<Board> { return this->get_possible_moves<Space::X>(); }
	auto Board::getPossibleMovesForO() const -> std::vector<Board> { return this->get_possible_moves<Space::O>(); }




	auto Board::getGameStatus() const -> GameStatus {
		for(size_t i = 0; i < 3; i+=1){
			const Space space = this->operator[](Coordinate(i, 0));
			if(space == Space::EMPTY){ continue; }
			if(this->is_row_win(i)){ return evo::bitCast<GameStatus>(space); }
		}

		for(size_t i = 0; i < 3; i+=1){
			const Space space = this->operator[](Coordinate(0, i));
			if(space == Space::EMPTY){ continue; }
			if(this->is_collumn_win(i)){ return evo::bitCast<GameStatus>(space); }
		}

		{
			const Space space = this->operator[](Coordinate(0, 0));
			if(space != Space::EMPTY){
				auto space_array = std::array<Coordinate, 3>{Coordinate(0, 0), Coordinate(1, 1), Coordinate(2, 2)};
				if(this->is_win(space_array)){ return evo::bitCast<GameStatus>(space); }
			}
		}

		{
			const Space space = this->operator[](Coordinate(0, 2));
			if(space != Space::EMPTY){
				auto space_array = std::array<Coordinate, 3>{Coordinate(0, 2), Coordinate(1, 1), Coordinate(2, 0)};
				if(this->is_win(space_array)){ return evo::bitCast<GameStatus>(space); }
			}
		}


		for(size_t x = 0; x < 3; x+=1){
			for(size_t y = 0; y < 3; y+=1){
				if(this->operator[](Coordinate(x, y)) == Space::EMPTY){ return GameStatus::IN_PROGRESS; }
			}
		}

		return GameStatus::DRAW;
	}



	auto Board::getAIData() const -> std::vector<float> {
		auto output = std::vector<float>();

		for(const std::array<Space, 3>& row : this->spaces){
			for(Space space : row){
				switch(space){
					break; case Space::EMPTY: output.emplace_back(0.0f);
					break; case Space::X:     output.emplace_back(0.5f);
					break; case Space::O:     output.emplace_back(1.0f);
				}
			}
		}

		return output;
	}



	auto Board::toString() const -> std::string {
		auto output = std::string();

		for(const std::array<Space, 3>& row : this->spaces){
			for(Space space : row){
				switch(space){
					break; case Space::EMPTY: output += '.';
					break; case Space::X:     output += 'X';
					break; case Space::O:     output += 'O';
				}
			}
			output += '\n';
		}

		return output;
	}



	template<Board::Space PIECE>
	EVO_NODISCARD auto Board::get_possible_moves() const -> std::vector<Board> {
		auto output = std::vector<Board>();

		for(size_t x = 0; x < 3; x+=1){
			for(size_t y = 0; y < 3; y+=1){
				if(this->operator[](Coordinate(x, y)) == Space::EMPTY){
					if constexpr(PIECE == Space::X){
						output.emplace_back(*this).placeX(Coordinate(x, y));
					}else{
						output.emplace_back(*this).placeO(Coordinate(x, y));
					}
				}
			}
		}

		return output;
	}




	auto Board::is_row_win(size_t row) const -> bool {
		auto array = std::array<Coordinate, 3>{
			Coordinate(row, 0),
			Coordinate(row, 1),
			Coordinate(row, 2),
		};
		return this->is_win(array);
	}

	auto Board::is_collumn_win(size_t collumn) const -> bool {
		auto array = std::array<Coordinate, 3>{
			Coordinate(0, collumn),
			Coordinate(1, collumn),
			Coordinate(2, collumn),
		};
		return this->is_win(array);
	}


	auto Board::is_win(const std::array<Coordinate, 3>& coords) const -> bool {
		const Space start_space = this->operator[](coords[0]);

		if(this->operator[](coords[1]) != start_space){ return false; }
		if(this->operator[](coords[2]) != start_space){ return false; }

		return true;
	}


	
}