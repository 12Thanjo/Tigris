////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////



#include <iostream>
#include <Evo.h>

#include <tigris.h>
#include "../os/Windows/include/windows.h"



auto tic_tac_toe_test() -> void {
	evo::printlnCyan("Tic Tac Toe");

	auto board = tigris::tic_tac_toe::Board();

	board.placeX(tigris::tic_tac_toe::Board::Coordinate(1, 1));
	board.placeO(tigris::tic_tac_toe::Board::Coordinate(1, 2));
	board.placeO(tigris::tic_tac_toe::Board::Coordinate(2, 2));
	board.placeO(tigris::tic_tac_toe::Board::Coordinate(0, 2));

	evo::print(board.toString());

	switch(board.getGameStatus()){
		break; case tigris::tic_tac_toe::Board::GameStatus::IN_PROGRESS: evo::println("IN_PROGRESS");
		break; case tigris::tic_tac_toe::Board::GameStatus::X_WIN:       evo::println("X_WIN");
		break; case tigris::tic_tac_toe::Board::GameStatus::O_WIN:       evo::println("O_WIN");
	}
}



auto connect_4_test() -> void {
	evo::printlnCyan("Connect 4");

	auto board = tigris::connect_4::Board();

	board.placeX(3);

	board.placeO(4);
	board.placeX(4);

	board.placeO(5);
	board.placeO(5);
	board.placeX(5);

	board.placeO(6);
	board.placeO(6);
	board.placeO(6);
	// board.placeX(6);

	evo::print(board.toString());

	switch(board.getGameStatus()){
		break; case tigris::connect_4::Board::GameStatus::IN_PROGRESS: evo::println("IN_PROGRESS");
		break; case tigris::connect_4::Board::GameStatus::X_WIN:       evo::println("X_WIN");
		break; case tigris::connect_4::Board::GameStatus::O_WIN:       evo::println("O_WIN");
	}
}





auto main(int argc, const char* argv[]) -> int {
	#if defined(EVO_PLATFORM_WINDOWS)
		os::windows::setConsoleToUTF8Mode();
	#endif

	#if defined(EVO_CONFIG_DEBUG)
		evo::log::setDefaultThreadSaferCallback();
	#endif

	#if defined(EVO_CONFIG_DEBUG) && defined(EVO_PLATFORM_WINDOWS)
		if(os::windows::isDebuggerPresent()){
			std::atexit([]() -> void {
				evo::printGray("Press [Enter] to close...");
				std::cin.get();
			});
		}
	#endif


	tic_tac_toe_test();
	connect_4_test();


	return 0;
}