////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////



#pragma once


#include <Evo.h>


namespace tigris::connect_4{


	class Board{
		public:
			Board() = default;
			~Board() = default;

			enum class Space{
				EMPTY = 0,
				X = 1,
				O = 2,
			};


			auto placeX(size_t collumn_index) -> void;
			auto placeO(size_t collumn_index) -> void;



			enum class GameStatus{ // numbers match Space
				IN_PROGRESS = 0,
				X_WIN = 1,
				O_WIN = 2,
			};
			EVO_NODISCARD auto getGameStatus() const -> GameStatus;


			EVO_NODISCARD auto toString() const -> std::string;


		private:

			template<Space PIECE>
			auto place_impl(size_t collumn_index) -> void;

			// coordinate
			struct Coordinate{
				size_t row;
				size_t collumn;
			};

			EVO_NODISCARD auto get_space(Coordinate coord) const -> Space {
				return this->spaces[coord.collumn][coord.row];
			}
			
			EVO_NODISCARD auto get_space(Coordinate coord) -> Space& {
				return this->spaces[coord.collumn][coord.row];
			}
			

			// These can detect a win of EMPTY spaces
			EVO_NODISCARD auto is_right_win(Coordinate start_coord) const -> bool;
			EVO_NODISCARD auto is_diag_left_win(Coordinate start_coord) const -> bool; // goes up
			EVO_NODISCARD auto is_diag_right_win(Coordinate start_coord) const -> bool; // goes up
			EVO_NODISCARD auto is_up_win(Coordinate start_coord) const -> bool;


			EVO_NODISCARD auto is_win(const std::array<Coordinate, 4>& coords) const -> bool;

	
		private:
			std::array<std::array<Space, 6>, 7> spaces{}; // done flipped to improve locality of the collumn
														  // 	(when checking which row the piece goes into)
	};

	
}