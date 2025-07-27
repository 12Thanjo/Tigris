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
#include <Vulkan.h>
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





using TicTacToePlayer = std::function<tigris::tic_tac_toe::Board(evo::ArrayProxy<tigris::tic_tac_toe::Board>)>;



auto play_tic_tac_toe(TicTacToePlayer x_player, TicTacToePlayer o_player) -> tigris::tic_tac_toe::Board::GameStatus {
	auto board = tigris::tic_tac_toe::Board();
	bool is_x_turn = true;

	while(board.getGameStatus() == tigris::tic_tac_toe::Board::GameStatus::IN_PROGRESS){
		const std::vector<tigris::tic_tac_toe::Board> possible_moves = [&](){
			if(is_x_turn){
				return board.getPossibleMovesForX();
			}else{
				return board.getPossibleMovesForO();
			}
		}();


		if(is_x_turn){
			board = x_player(possible_moves);

		}else{
			board = o_player(possible_moves);
		}

		is_x_turn = !is_x_turn;
	}

	return board.getGameStatus();
}



auto ai_play_tic_tac_toe(const tigris::AI& x_player, const tigris::AI& o_player)
-> tigris::tic_tac_toe::Board::GameStatus {
	return play_tic_tac_toe(
		[&](evo::ArrayProxy<tigris::tic_tac_toe::Board> possible_moves){
			auto results = std::vector<float>(possible_moves.size());

			for(size_t i = 0; i < possible_moves.size(); i+=1){
				tigris::Matrix result = x_player.calculate(possible_moves[i].getAIData());
				evo::debugAssert(result.width() == 1 && result.height() == 1);
				results[i] = result[0, 0];
			}

			return possible_moves[std::distance(results.begin(), std::max_element(results.begin(), results.end()))];
		},
		[&](evo::ArrayProxy<tigris::tic_tac_toe::Board> possible_moves){
			auto results = std::vector<float>(possible_moves.size());

			for(size_t i = 0; i < possible_moves.size(); i+=1){
				tigris::Matrix result = o_player.calculate(possible_moves[i].getAIData());
				evo::debugAssert(result.width() == 1 && result.height() == 1);
				results[i] = result[0, 0];
			}

			return possible_moves[std::distance(results.begin(), std::min_element(results.begin(), results.end()))];
		}
	);
}





auto run_tic_tac_toe_training() -> void {
	static constexpr size_t POPULATION = 200;
	static constexpr size_t NUM_ITERS_PER_EPOCH = 10;
	static constexpr float MUTATION_RATE = 0.01f;
	static constexpr float NUM_NEW_RANDOM = 0;
	static constexpr size_t NUM_RUNS_AGAINST_RANDOM = 50;


	auto environment = tigris::Environment(POPULATION, {9, 64, 1});
	environment.initRandom();

	size_t last_num_losses = 0;
	size_t num_epochs = 0;

	evo::printlnGray("             w/ d/ l");

	while(true){
		///////////////////////////////////
		// epoch

		for(size_t i = 0; i < NUM_ITERS_PER_EPOCH; i+=1){
			environment.beginGame();

			for(size_t x_player_i = 0; x_player_i < environment.population.size() - 1; x_player_i+=1){
				for(size_t o_player_i = x_player_i + 1; o_player_i < environment.population.size(); o_player_i+=1){
					{
						const tigris::tic_tac_toe::Board::GameStatus game_result = ai_play_tic_tac_toe(
							environment.population[x_player_i], environment.population[o_player_i]
						);

						switch(game_result){
							case tigris::tic_tac_toe::Board::GameStatus::IN_PROGRESS: {
								evo::debugFatalBreak("Invalid Result");
							} break;

							case tigris::tic_tac_toe::Board::GameStatus::X_WIN: {
								environment.scores[x_player_i] += 1.0f;
							} break;

							case tigris::tic_tac_toe::Board::GameStatus::O_WIN: {
								environment.scores[o_player_i] += 1.0f;
							} break;

							case tigris::tic_tac_toe::Board::GameStatus::DRAW: {
								environment.scores[x_player_i] += 0.5f;
								environment.scores[o_player_i] += 0.5f;
							} break;
						}
					}
					
					{
						const tigris::tic_tac_toe::Board::GameStatus game_result = ai_play_tic_tac_toe(
							environment.population[o_player_i], environment.population[x_player_i]
						);

						switch(game_result){
							case tigris::tic_tac_toe::Board::GameStatus::IN_PROGRESS: {
								evo::debugFatalBreak("Invalid Result");
							} break;

							case tigris::tic_tac_toe::Board::GameStatus::X_WIN: {
								environment.scores[o_player_i] += 1.0f;
							} break;

							case tigris::tic_tac_toe::Board::GameStatus::O_WIN: {
								environment.scores[x_player_i] += 1.0f;
							} break;

							case tigris::tic_tac_toe::Board::GameStatus::DRAW: {
								environment.scores[o_player_i] += 0.5f;
								environment.scores[x_player_i] += 0.5f;
							} break;
						}
					}
				}
			}

			environment.setScoresToReproductionChance();

			environment.createNewPopulation(MUTATION_RATE, NUM_NEW_RANDOM);
		}




		///////////////////////////////////
		// run against random

		using TicTacToeStatus = tigris::tic_tac_toe::Board::GameStatus;

		const tigris::AI& best_ai = environment.population[
			std::distance(environment.scores.begin(), std::ranges::max_element(environment.scores))
		];


		std::srand(12);

		size_t num_wins = 0;
		size_t num_draws = 0;
		size_t num_losses = 0;
		for(size_t i = 0; i < NUM_RUNS_AGAINST_RANDOM; i+=1){
			{
				const TicTacToeStatus game_result = play_tic_tac_toe(
					[&](evo::ArrayProxy<tigris::tic_tac_toe::Board> possible_moves){
						auto results = std::vector<float>(possible_moves.size());

						for(size_t i = 0; i < possible_moves.size(); i+=1){
							tigris::Matrix result = best_ai.calculate(possible_moves[i].getAIData());
							evo::debugAssert(result.width() == 1 && result.height() == 1);
							results[i] = result[0, 0];
						}

						return possible_moves[
							std::distance(results.begin(), std::max_element(results.begin(), results.end()))
						];
					},
					[&](evo::ArrayProxy<tigris::tic_tac_toe::Board> possible_moves) -> tigris::tic_tac_toe::Board {
						// return possible_moves[evo::random(possible_moves.size()-1)];
						if(possible_moves.size() == 1){
							return possible_moves[0];
						}else{
							return possible_moves[std::rand() % (possible_moves.size() - 1)];
						}
					}
				);

				switch(game_result){
					break; case TicTacToeStatus::IN_PROGRESS: evo::debugFatalBreak("Invalid Result");
					break; case TicTacToeStatus::X_WIN: num_wins += 1;
					break; case TicTacToeStatus::O_WIN: num_losses += 1;
					break; case TicTacToeStatus::DRAW:  num_draws += 1;
				}
			}
			
			{
				const TicTacToeStatus game_result = play_tic_tac_toe(
					[&](evo::ArrayProxy<tigris::tic_tac_toe::Board> possible_moves) -> tigris::tic_tac_toe::Board {
						// return possible_moves[evo::random(possible_moves.size()-1)];
						if(possible_moves.size() == 1){
							return possible_moves[0];
						}else{
							return possible_moves[std::rand() % (possible_moves.size() - 1)];
						}
					},
					[&](evo::ArrayProxy<tigris::tic_tac_toe::Board> possible_moves){
						auto results = std::vector<float>(possible_moves.size());

						for(size_t i = 0; i < possible_moves.size(); i+=1){
							tigris::Matrix result = best_ai.calculate(possible_moves[i].getAIData());
							evo::debugAssert(result.width() == 1 && result.height() == 1);
							results[i] = result[0, 0];
						}

						return possible_moves[
							std::distance(results.begin(), std::min_element(results.begin(), results.end()))
						];
					}
				);

				switch(game_result){
					break; case TicTacToeStatus::IN_PROGRESS: evo::debugFatalBreak("Invalid Result");
					break; case TicTacToeStatus::X_WIN: num_losses += 1;
					break; case TicTacToeStatus::O_WIN: num_wins += 1;
					break; case TicTacToeStatus::DRAW:  num_draws += 1;
				}
			}
		}


		///////////////////////////////////
		// done

		evo::printGray("epoch {:<5} ", num_epochs);
		evo::printWhite("{:2}/{:2}/{:2} ", num_wins, num_draws, num_losses);

		if(last_num_losses == 0){
			evo::println();

		}else if(last_num_losses < num_losses){
			evo::printlnRed("+{}", num_losses - last_num_losses);

		}else if(last_num_losses > num_losses){
			evo::printlnGreen("-{}", last_num_losses - num_losses);

		}else{
			evo::printlnCyan("0");
		}

		last_num_losses = num_losses;
		num_epochs += 1;

		if(num_losses == 0){ break; }
	}
		
	evo::printlnGreen("Done");
}







auto main(int argc, const char* argv[]) -> int {
	auto args = std::vector<std::string_view>(argv, argv + argc);


	#if defined(EVO_PLATFORM_WINDOWS)
		os::windows::setConsoleToUTF8Mode();
	#endif

	evo::log::setDefaultThreadSaferCallback();

	#if defined(EVO_PLATFORM_WINDOWS)
		if(os::windows::isDebuggerPresent()){
			std::atexit([]() -> void {
				evo::printGray("Press [Enter] to close...");
				std::cin.get();
			});
		}
	#endif


	// /run_tic_tac_toe_training();

	vulkan::test();



	return 0;
}