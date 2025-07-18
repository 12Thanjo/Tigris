////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include <connect_4/board.h>

namespace tigris::connect_4{


	auto Board::placeX(size_t collumn_index) -> void {
		this->place_impl<Space::X>(collumn_index);
	}

	auto Board::placeO(size_t collumn_index) -> void {
		this->place_impl<Space::O>(collumn_index);
	}



	auto Board::getGameStatus() const -> GameStatus {
		// check right wins
		for(size_t row = 0; row < 6; row+=1){
			for(size_t collumn = 0; collumn < 4; collumn+=1){
				const Space space = this->get_space(Coordinate(row, collumn));

				if(space == Space::EMPTY){ continue; }
				if(this->is_right_win(Coordinate(row, collumn))){ return evo::bitCast<GameStatus>(space); }
			}
		}


		// check diag left wins
		for(size_t row = 0; row < 2; row+=1){
			for(size_t collumn = 3; collumn < 7; collumn+=1){
				const Space space = this->get_space(Coordinate(row, collumn));

				if(space == Space::EMPTY){ continue; }
				if(this->is_diag_left_win(Coordinate(row, collumn))){ return evo::bitCast<GameStatus>(space); }
			}
		}


		// check diag right wins
		for(size_t row = 0; row < 2; row+=1){
			for(size_t collumn = 0; collumn < 4; collumn+=1){
				const Space space = this->get_space(Coordinate(row, collumn));

				if(space == Space::EMPTY){ continue; }
				if(this->is_diag_right_win(Coordinate(row, collumn))){ return evo::bitCast<GameStatus>(space); }
			}
		}

		// check up wins
		for(size_t row = 0; row < 2; row+=1){
			for(size_t collumn = 0; collumn < 7; collumn+=1){
				const Space space = this->get_space(Coordinate(row, collumn));

				if(space == Space::EMPTY){ continue; }
				if(this->is_up_win(Coordinate(row, collumn))){ return evo::bitCast<GameStatus>(space); }
			}
		}

		return GameStatus::IN_PROGRESS;
	}




	auto Board::toString() const -> std::string {
		auto output = std::string();

		for(size_t row = this->spaces[0].size() - 1; row < this->spaces[0].size(); row-=1){ // go backwards
			for(size_t collumn = 0; collumn < this->spaces.size(); collumn+=1){
				switch(this->get_space(Coordinate(row, collumn))){
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
	auto Board::place_impl(size_t collumn_index) -> void {
		static_assert(PIECE != Space::EMPTY, "Not a space");
		evo::debugAssert(collumn_index < 7, "Invalid row");

		std::array<Space, 6>& row = this->spaces[collumn_index];

		for(size_t i = 0; i < 6; i+=1){
			if(row[i] == Space::EMPTY){
				row[i] = PIECE;
				return;
			}
		}

		evo::debugFatalBreak("No spot in the collumn");
	}



	auto Board::is_right_win(Coordinate start_coord) const -> bool {
		auto array = std::array<Coordinate, 4>{
			start_coord,
			Coordinate(start_coord.row, start_coord.collumn + 1),
			Coordinate(start_coord.row, start_coord.collumn + 2),
			Coordinate(start_coord.row, start_coord.collumn + 3),
		};
		return this->is_win(array);
	}

	auto Board::is_diag_left_win(Coordinate start_coord) const -> bool {
		auto array = std::array<Coordinate, 4>{
			start_coord,
			Coordinate(start_coord.row + 1, start_coord.collumn - 1),
			Coordinate(start_coord.row + 2, start_coord.collumn - 2),
			Coordinate(start_coord.row + 3, start_coord.collumn - 3),
		};
		return this->is_win(array);
	}

	auto Board::is_diag_right_win(Coordinate start_coord) const -> bool {
		auto array = std::array<Coordinate, 4>{
			start_coord,
			Coordinate(start_coord.row + 1, start_coord.collumn + 1),
			Coordinate(start_coord.row + 2, start_coord.collumn + 2),
			Coordinate(start_coord.row + 3, start_coord.collumn + 3),
		};
		return this->is_win(array);
	}

	auto Board::is_up_win(Coordinate start_coord) const -> bool {
		auto array = std::array<Coordinate, 4>{
			start_coord,
			Coordinate(start_coord.row + 1, start_coord.collumn),
			Coordinate(start_coord.row + 2, start_coord.collumn),
			Coordinate(start_coord.row + 3, start_coord.collumn),
		};
		return this->is_win(array);
	}



	auto Board::is_win(const std::array<Coordinate, 4>& coords) const -> bool {
		const Space start_space = this->get_space(coords[0]);

		for(size_t i = 1; i < 4; i+=1){
			if(this->get_space(coords[i]) != start_space){ return false; }
		}

		return true;
	}

	
}