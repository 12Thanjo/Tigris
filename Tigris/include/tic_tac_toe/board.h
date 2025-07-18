////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////



#pragma once


#include <Evo.h>


namespace tigris::tic_tac_toe{


	class Board{
		public:
			Board() = default;
			~Board() = default;

			enum class Space{
				EMPTY = 0,
				X = 1,
				O = 2,
			};


			// coordinate
			struct Coordinate{
				size_t row;
				size_t collumn;
			};


			auto placeX(Coordinate coord) -> void;
			auto placeO(Coordinate coord) -> void;



			enum class GameStatus{ // numbers match Space
				IN_PROGRESS = 0,
				X_WIN = 1,
				O_WIN = 2,
			};
			EVO_NODISCARD auto getGameStatus() const -> GameStatus;


			EVO_NODISCARD auto toString() const -> std::string;


		private:
			EVO_NODISCARD auto get_space(Coordinate coord) const -> Space {
				return this->spaces[coord.row][coord.collumn];
			}
			
			EVO_NODISCARD auto get_space(Coordinate coord) -> Space& {
				return this->spaces[coord.row][coord.collumn];
			}


			EVO_NODISCARD auto is_row_win(size_t row) const -> bool;
			EVO_NODISCARD auto is_collumn_win(size_t collumn) const -> bool;

			EVO_NODISCARD auto is_win(const std::array<Coordinate, 3>& coords) const -> bool;


	
		private:
			std::array<std::array<Space, 3>, 3> spaces{};
	};

	
}