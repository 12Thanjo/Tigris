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


			auto placeO(Coordinate coord) -> void;
			auto placeX(Coordinate coord) -> void;


			EVO_NODISCARD auto getPossibleMovesForX() const -> std::vector<Board>;
			EVO_NODISCARD auto getPossibleMovesForO() const -> std::vector<Board>;



			enum class GameStatus{ // numbers match Space
				IN_PROGRESS = 0,
				X_WIN = 1,
				O_WIN = 2,
				DRAW = 3,
			};
			EVO_NODISCARD auto getGameStatus() const -> GameStatus;


			EVO_NODISCARD auto getAIData() const -> std::vector<float>;

			EVO_NODISCARD auto toString() const -> std::string;


		private:
			EVO_NODISCARD auto operator[](Coordinate coord) const -> const Space& {
				return this->spaces[coord.row][coord.collumn];
			}
			
			EVO_NODISCARD auto operator[](Coordinate coord) -> Space& {
				return this->spaces[coord.row][coord.collumn];
			}


			template<Space PIECE>
			EVO_NODISCARD auto get_possible_moves() const -> std::vector<Board>;


			EVO_NODISCARD auto is_row_win(size_t row) const -> bool;
			EVO_NODISCARD auto is_collumn_win(size_t collumn) const -> bool;

			EVO_NODISCARD auto is_win(const std::array<Coordinate, 3>& coords) const -> bool;


	
		private:
			std::array<std::array<Space, 3>, 3> spaces{};
	};

	
}




template<>
struct std::formatter<tigris::tic_tac_toe::Board>{
    constexpr auto parse(std::format_parse_context& ctx) const -> auto {
        return ctx.begin();
    }

    auto format(const tigris::tic_tac_toe::Board& board, std::format_context& ctx) const -> auto {
        return std::format_to(ctx.out(), "{}", board.toString());
    }
};