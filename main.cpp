#include "Game.h"
#include "Alpha Beta.h"
#include <vector>
#include <iostream>
#include <map>


const int Infinity = INT_MAX;
const int WinValue = INT_MAX;

int main()
{
	BoardState GameBoard = GetStartState();
	GameBoard.playerTurn = 1;
	PrintBoard(GameBoard);
	
	while(!GameOver(GameBoard))
	{
		//Run(GameBoard, 1, 0, -Infinity, Infinity);
		DoMove(GameBoard);
		playMoves(GameBoard);
	}
	
	
	/*	
	 while(!GameOver(GameBoard))
	 {
	 if(GameBoard.playerTurn == 1)
	 {
	 DoMove(GameBoard);
	 GameBoard.playerTurn = 2;
	 }
	 else {
	 DoMove(GameBoard);
	 GameBoard.playerTurn = 1;
	 }
	 }
	 */
	
	return 0;
}