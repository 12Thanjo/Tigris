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
		Space& space = this->get_space(coord);
		evo::debugAssert(space == Space::EMPTY, "space is not empty");
		space = Space::X;
	}

	auto Board::placeO(Coordinate coord) -> void {
		Space& space = this->get_space(coord);
		evo::debugAssert(space == Space::EMPTY, "space is not empty");
		space = Space::O;
	}




	auto Board::getGameStatus() const -> GameStatus {
		for(size_t i = 0; i < 3; i+=1){
			const Space space = this->get_space(Coordinate(i, 0));
			if(space == Space::EMPTY){ continue; }
			if(this->is_row_win(i)){ return evo::bitCast<GameStatus>(space); }
		}

		for(size_t i = 0; i < 3; i+=1){
			const Space space = this->get_space(Coordinate(0, i));
			if(space == Space::EMPTY){ continue; }
			if(this->is_collumn_win(i)){ return evo::bitCast<GameStatus>(space); }
		}

		{
			const Space space = this->get_space(Coordinate(0, 0));
			if(space != Space::EMPTY){
				auto space_array = std::array<Coordinate, 3>{Coordinate(0, 0), Coordinate(1, 1), Coordinate(2, 2)};
				if(this->is_win(space_array)){ return evo::bitCast<GameStatus>(space); }
			}
		}

		{
			const Space space = this->get_space(Coordinate(0, 2));
			if(space != Space::EMPTY){
				auto space_array = std::array<Coordinate, 3>{Coordinate(0, 2), Coordinate(1, 1), Coordinate(2, 0)};
				if(this->is_win(space_array)){ return evo::bitCast<GameStatus>(space); }
			}
		}

		return GameStatus::IN_PROGRESS;
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
		const Space start_space = this->get_space(coords[0]);

		if(this->get_space(coords[1]) != start_space){ return false; }
		if(this->get_space(coords[2]) != start_space){ return false; }

		return true;
	}


	
}