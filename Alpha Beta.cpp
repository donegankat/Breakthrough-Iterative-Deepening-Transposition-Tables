/*
 *  Alpha Beta.cpp
 *  Breakthrough Alpha Beta
 *
 *  Created by K Donegan on 2/10/13.
 *  Copyright 2013 University of Denver. All rights reserved.
 *
 */

#include "Alpha Beta.h"
#include "Game.h"
#include <iostream>     // std::cout
#include <algorithm>    // std::sort
#include <vector>       // std::vector
#include <map>
#include <stdint.h>
#include <cstdlib>
#include "Timer.h"
#include "ZobristValues.h"

Timer *myTimer = new Timer();

static int alpha = Evaluation(GetStartState()); //Minimum
static int beta = 100000000; //Maximum
static PieceAction best_move, worst_move;
//static int depth = 0;
const int maxDepth = 2;
const int Infinity = INT_MAX;
const int WinValue = INT_MAX;
static int CurrentTurn = 1;

const double MaxMoveTime = 5;

static BoardState BestBS;

static int NodesSearchedLast = 0;
static int PlyDepth = 50; // Max-Ply Search
static std::vector<BoardState> ResultBoards;

static std::vector<PieceAction> moves_taken1, moves_taken2;
static std::vector<PieceAction> best_sequence, worst_sequence;

static int GetNodesSearchedLast();
bool P1Win(BoardState s);
bool P2Win(BoardState s);
bool GameOver(BoardState s);
int GetOppositePlayer(int player);
static BoardState CopyBoard(BoardState s, int player);
static int ModifyDepth(int d, int PossibleMoves);
std::vector<BoardState> SortBoards(std::vector<BoardState> MyBoards);
static std::vector<BoardState> PossibleBoards(BoardState s, int player);
static BoardState FixedDepthAlphaBeta(BoardState ExamineBoard, int MaxDepth, int playerTurn);
static BoardState AlphaBetaTTRoot(BoardState ExamineBoard, int Depth, int playerTurn);
static int AlphaBetaTT(BoardState ExamineBoard, int Depth, int Alpha, int Beta, int playerTurn);
static BoardState IterativeDeepeningAlphaBeta(BoardState ExamineBoard, int MaxDepth, int turn, bool ShowDepth);

bool updateHistory(std::vector<PieceAction> &history, PieceAction newmove);
void Run(BoardState &s, int turn, int depth, int a, int b);
void playMoves(BoardState &s);

void DoMove(BoardState s);
void test(std::vector<PieceAction> x, BoardState s);
//static BoardTT *MyBoardTT = new BoardTT();

//static std::vector<std::pair<uint64_t, TTEntry>> *myTT;
static std::map<uint64_t,TTEntry> Table;
static int Seed = 1;
void StoreEntry(uint64_t Key, TTEntry Entry);
void PrintTT();//std::map<uint64_t,TTEntry> &Table);

void BoardTT()
{
	//static std::map<uint64_t, TTEntry> Table;
	// total should be: 2*64 + 1 = 129
	static int *PRNArray = new int[129];
	srand (time(NULL));
	
	for (int i = 0; i < 129; i++)
	{
		int MT = rand() % 64 + (Seed);
		PRNArray[i] = MT;
	}
	
	//Initialize the table
	//Table = new std::map<uint64_t, TTEntry>();
}

//BoardTT::~BoardTT()
//{
//	delete[] this;
//}

void StoreEntry(uint64_t Key, TTEntry Entry)
{
	if (Table.count(Key) == 0)
	{
		Table.insert(std::pair<uint64_t, TTEntry>(Key, Entry));
	}
	else {
		Table.erase(Key);
		Table.insert(std::pair<uint64_t, TTEntry>(Key, Entry));
	}
}

static int GetNodesSearchedLast()
{
	return NodesSearchedLast;
}

int GetNum1(BoardState s)
{
	int num1;
	
	for(int i = 0; i < 64; i++)
	{
		if(s.board[i] == 1)
		{
			num1 ++;
		}
	}
	
	return num1;
}

int GetNum2(BoardState s)
{
	int num2;
	
	for(int i = 0; i < 64; i++)
	{
		if(s.board[i] == 2)
		{
			num2 ++;
		}
	}
	
	return num2;
}

bool P1Win(BoardState s)
{
	
	if(GetNum2(s) == 0)//GetNumPieces2(s) == 0)
	{
		std::cout << "Player 2 is out of pieces." << std::endl;
		return true;
	}	
	
	for(int i = 0; i < 8; i++)
	{
		if(s.board[i] == 1)
		{
			return true;
		}
	}
	
	return false;
}

bool P2Win(BoardState s)
{
	
	if(GetNum1(s) == 0)//GetNumPieces1(s) == 0)
	{
		std::cout << "Player 1 is out of pieces." << std::endl;
		return true;
	}	
	
	for(int i = 56; i < 64; i++)
	{
		if(s.board[i] == 2)
		{
			return true;
		}
	}
	
	return false;
}

bool GameOver(BoardState s)
{
	if(P1Win(s) || P2Win(s))
	{
		return true;
	}
	
	return false;
}

int GetOppositePlayer(int player)
{
	if(player == 1)
	{
		return 2;
	}
	
	if(player == 2)
	{
		return 1;
	}
	
	return 0;
}

static BoardState CopyBoard(BoardState s, int player)
{
	BoardState Copied;// = new BoardState;
	Copied.playerTurn = player;
	
	for(int i = 0; i < 64; i++)
	{
		Copied.board[i] = s.board[i];
	}
	
	return Copied;
}

static int ModifyDepth(int d, int PossibleMoves)
{
	if(PossibleMoves < 9)
	{
		d = d + 2;
	}
	return d;
}


static std::vector<BoardState> PossibleBoards(BoardState s, int player)
{
	std::vector<int> MovablePieces; // = new std::vector<int>;
		
	for(int i = 0; i < 64; i++)
	{
		if(s.board[i] != player)
		{
			continue;
		}
		else //if(s.board[i] == player)
		{
			MovablePieces.push_back(i);
		}
	}
	for(int i = 0; i < MovablePieces.size(); i++)
	{
		BoardState CurrentBoard = CopyBoard(s, player);
		std::vector<PieceAction> MovesForPiece;
		MovesForPiece = GetPieceActions(s, player, MovablePieces[i]);
		for(int j = 0; j < MovesForPiece.size(); j++)
		{
			CurrentBoard = ApplyAction(CurrentBoard, MovesForPiece[j]);
//			MovesForPiece.erase(MovesForPiece[j]);
//			PrintBoard(CurrentBoard);
			
			ResultBoards.push_back(CurrentBoard);
			
			CurrentBoard = UndoAction(CurrentBoard, MovesForPiece[j]);
		}
		
	}
	
//	MovablePieces.clear();
	return ResultBoards;
		
}

// Root for Fixed Depth Alpha-Beta
static BoardState FixedDepthAlphaBeta(BoardState ExamineBoard, int MaxDepth, int playerTurn)
{
	//Set AI
	CurrentTurn = playerTurn;
	
	//Trash Transposition Table
	Table.clear();// = new map<int, TTEntry>();
	
	return AlphaBetaTTRoot(ExamineBoard, MaxDepth, playerTurn);
}


bool SortSuccessors(BoardState i, BoardState j)
{
	return (Evaluation(i) > Evaluation(j));
}

// Root entry of TT Alpha-Beta algorithm (Transposition Table enhanced)
static BoardState LoSSearch(BoardState ExamineBoard, int MaxDepth, int playerTurn)
{
	std::vector<BoardState> Successors = PossibleBoards(ExamineBoard, playerTurn);
	BoardState Result;
	
	// Fill the search space
	std::vector<int> SearchSpace; // = new List<byte>();
	if (playerTurn == 1)
	{
		for (int i = 8; i < 40; i++) // 32
			if (ExamineBoard.board[i] != 0 && ExamineBoard.board[i] == 1)
				SearchSpace.push_back(i);
	}
	else
	{
		for (int i = 24; i < 56; i++) // 32
			if (ExamineBoard.board[i] == 2)
				SearchSpace.push_back(i);
	}
	
	if (SearchSpace.size() > 0)
	{
		for(int i = 0; i < SearchSpace.size(); i++)
		{
			// Reduce to Line of sight
			BoardState Last = AlphaBetaTTRoot(ExamineBoard, MaxDepth, playerTurn);
			
			if (Last.board != 0)
			{
				if (Evaluation(Last) >= WinValue)
				{ // Winning position found! 
					Result = Last;
					break;
				}
				else if (Result.board != 0 && Evaluation(Result) < Evaluation(Last))
				{ // Better one found
					Result = Last;
				}
				else if (Result.board == 0)
				{
					Result = Last;
				}
			}
		}
	}
	
	return Result;
}

static BoardState AlphaBetaTTRoot(BoardState ExamineBoard, int Depth, int playerTurn)
{
	// Perform variable initializations
	int BestValue = -Infinity;
	bool FirstCall = true;
	NodesSearchedLast = 0;
	Depth--;
	
	// Init hash (just first, next hashes will be rolled)
	ExamineBoard = GetStartState();
	
	// Get all possible boards
	std::vector<BoardState> Successors = PossibleBoards(ExamineBoard, playerTurn);

	// Evaluate all and order the root
	std::sort(Successors.begin(), Successors.end(), SortSuccessors);
	
	// Result value initialization
	BoardState BestBoard = Successors[0];
	
	for (int i = 0; i < Successors.size(); i++)
	{
		BoardState BoardToEvaluate = Successors[i];
		int Value;
		if (FirstCall)
		{ // First call, alpha = -infinity and beta = +infinity
			Value = -AlphaBetaTT(BoardToEvaluate, ModifyDepth(Depth, Successors.size()), -Infinity, -BestValue, GetOppositePlayer(playerTurn));
		}
		else
		{ // Better value found
			Value = -AlphaBetaTT(BoardToEvaluate, ModifyDepth(Depth, Successors.size()), -BestValue - 1, -BestValue, GetOppositePlayer(playerTurn));
			if (Value > BestValue)
			{
				Value = -AlphaBetaTT(BoardToEvaluate, ModifyDepth(Depth, Successors.size()), -Infinity, -BestValue, GetOppositePlayer(playerTurn));
			}
		}
		
		if (Value > BestValue) // value is better
		{
			BestValue = Value;
			BestBoard = BoardToEvaluate;
			FirstCall = false;
		}
		
	}
		
	
	Successors.clear();
	return BestBoard;
}

static BoardState IterativeDeepeningAlphaBeta(BoardState ExamineBoard, int MaxDepth, int turn, bool ShowDepth)
{
	//Set AI
	ExamineBoard.playerTurn = turn;
	
	//Empty the Transposition Table
	Table.clear();
	BoardState BestBoard;
	BoardState LastBoard;
	
	int Depth = 1;
	
	myTimer->start();
	for (Depth = 1; Depth < MaxDepth; Depth++)
	{
		double duration = myTimer->TimeElapsed(*myTimer);
		LastBoard = AlphaBetaTTRoot(ExamineBoard, Depth, turn);
		if (duration >= MaxMoveTime || GetHashFromState(LastBoard) == 0)
			break; // timeout
		BestBoard = LastBoard;
	}
	myTimer->stop();
	
	if (ShowDepth)
		std::cout << "AI> Depth searched = " << Depth << std::endl;
	return BestBoard;
}


 /// Alpha-Beta algorithm iteration
static int AlphaBetaTT(BoardState ExamineBoard, int Depth, int Alpha, int Beta, int playerTurn)
{

	uint64_t HashValue = GetHashFromState(ExamineBoard);
	TTEntry theEntry; 
	std::map<uint64_t,TTEntry>::iterator it;
	for(std::map<uint64_t,TTEntry>::iterator it=Table.begin(); it!=Table.end(); ++it)
	{
		
	//}
	//for(uint64_t i = 0; i < GetMaxHashValue(); i++)
	//{
		if(it->first == HashValue)
		{
			theEntry = it->second;
		}
		
	}

	//CHECK TT TO SEE IF CURRENT STATE IS IN THERE
	

	if (theEntry.Depth >= Depth)
	{
		TTEntryType theType = theEntry.Type;
		
		if (theType == ExactValue) // stored value is exact
		{
			return theEntry.Value;
		}
		if (theType == LowerBound && theEntry.Value > Alpha)
		{
			Alpha = theEntry.Value; // update LowerBound alpha if needed
		}
		else if (theType == UpperBound && theEntry.Value < Beta)
		{
			Beta = theEntry.Value; // update UpperBound beta if needed
		}
		if (Alpha >= Beta)
		{
			return theEntry.Value; // if LowerBound surpasses UpperBound
		}
	}
	
	if (Depth == 0 || GameOver(ExamineBoard))
	{
		//-------- BoardEvaluation.GetValue(ExamineBoard,playerTurn);
		int value = Evaluation(ExamineBoard) + Depth; // add depth (since it's inverse)
		TTEntry newEntry;
		newEntry.Value = value;
		newEntry.Depth = Depth;
		if (value <= Alpha) // a LowerBound value
		{
			newEntry.Type = LowerBound;
		}
		else if (value >= Beta) // an UpperBound value
			newEntry.Type = UpperBound;
		else // a true minimax value
			newEntry.Type = ExactValue;
		
		StoreEntry(HashValue, newEntry);
		return value;
	}
	
	NodesSearchedLast++;
	
	//Apply null move restrictions

 if (Depth >= 2 && Beta < Infinity && (GetNum1(ExamineBoard) + GetNum2(ExamineBoard)) > 15)
	{
		// Try null move
		int r = 1;
		if (Depth >= 4) r = 2;
		else if (Depth >= 7) r = 3;
		
		int value = -AlphaBetaTT(ExamineBoard, (int)(Depth - r - 1), -Beta, -Beta + 1, playerTurn);
		
		TTEntry UpperTTEntry;
		UpperTTEntry.Value = value;
		UpperTTEntry.Depth = Depth;
		UpperTTEntry.Type = UpperBound;
		
		if (value >= Beta)
		{
			StoreEntry(HashValue, UpperTTEntry);
			return value;
		}
	}
	
	
	std::vector<BoardState> Successors = PossibleBoards(ExamineBoard, playerTurn);
	int totalBoards = Successors.size();
	
	std::cout << "Number of successor boards at depth: " << Depth << " is: " << totalBoards << std::endl;
	
	if (totalBoards == 0)
		return Evaluation(ExamineBoard);
	
//	std::cout << "Unsorted: " << std::endl;
//	for(int i = 0; i < Successors.size(); i++)
//	{
//		std::cout << Evaluation(Successors[i]) << std::endl;
//	}
	
	// sort the boards in order to have better pruning
	std::sort(Successors.begin(), Successors.end(), SortSuccessors);
	
//	std::cout << "Sorted: " << std::endl;
//	for(int i = 0; i < Successors.size(); i++)
//	{
//		std::cout << Evaluation(Successors[i]) << std::endl;
//	}
	
	// Evaluate all and order the root
	Depth--;
	
	int BestScore = -WinValue - 1;
	int BestIndex = 0;

	BoardState BoardToEvaluate;
	for (int i = 0; i < totalBoards; i++)
	{
		BoardToEvaluate = Successors[i];
		int value = -AlphaBetaTT(BoardToEvaluate, Depth, -Beta, -Alpha, GetOppositePlayer(playerTurn));
		
		if (value > BestScore)
		{
			BestScore = value;
			BestIndex = i;
		}
			
		if (BestScore > Alpha)
			Alpha = BestScore;
		if (BestScore >= Beta)
			break;
	}
	
	TTEntry BestTTEntry;
	BestTTEntry.Value = BestScore;
	BestTTEntry.Depth = Depth;
	
	if (BestScore <= Alpha) // a LowerBound value
		BestTTEntry.Type = LowerBound;
	else if (BestScore >= Beta) // an UpperBound value
		BestTTEntry.Type = UpperBound;
	else // a true minimax value
		BestTTEntry.Type = ExactValue;
	StoreEntry(HashValue, BestTTEntry);
	BestBS = Successors[BestIndex];
	Successors.clear();
	return BestScore;
}

int NodesSearched;
std::vector<PieceAction> MoveHistory; // = new Stack<MoveContent>();

BoardState theBoard = GetStartState();
BoardState previousBoard;
int currentPlayer = 1;

void DoMove(BoardState s)
{
	std::cout << "Player: " << s.playerTurn << std::endl;
	
	//only if not winning board
	if (s.board != 0 && !GameOver(s))
	{
		std::cout << "Finding best board." << std::endl;
		myTimer->start();
		std::vector<BoardState> Tree;// = new List<Board>();
		
		NodesSearched = 0;
		BoardState BestBoard; // = new Board();
		BestBoard = IterativeDeepeningAlphaBeta(theBoard, PlyDepth, s.playerTurn, true);
		//BestBoard = FixedDepthAlphaBeta(s, 2, s.playerTurn);
		//BestBoard = MoveSearchTree.AlphaBetaRoot(ChessBoard, 7, WhosMove);
		
		for(int i = 0; i < MoveHistory.size(); i++)
		{
			std::cout << MoveHistory[i].from << " " << MoveHistory[i].to << std::endl;
		}
		
		if (BestBoard.board != 0 && !MoveHistory.empty())
		{
			std::cout << "Applying actions." << std::endl;
			PieceAction LastMove = MoveHistory.back();
			BestBoard = ApplyAction(s, LastMove);
			
			PrintBoard(BestBoard);
			
			int BestValue = Evaluation(BestBoard);
		}
		else
		{
		}
		
		NodesSearched = GetNodesSearchedLast();
	}
}

std::vector<PieceAction> mv, bestaction, worstaction;
int maxd = 3;
static int result;
static std::vector<PieceAction> move_history;


//std::vector<PieceAction> updateHistory(std::vector<PieceAction> history, PieceAction newmove)
bool updateHistory(std::vector<PieceAction> &history, PieceAction newmove)
{
	for(int i = 0; i < history.size(); i++)
	{
		if(newmove.depth == history[i].depth && newmove.player == history[i].player)
		{
			history[i].from = newmove.from;
			history[i].to = newmove.to;
			
			return true;
		}
	}
	
	return false;
}

void Run(BoardState &s, int turn, int depth, int a, int b)
{
//	std::cout << "Player: " << turn << std::endl;
	s.playerTurn = turn;
	std::vector<BoardState> Boards = PossibleBoards(s, turn);
	
	if(depth == 0)
	{
		alpha = -Infinity;
		beta = Infinity;
	}
	
	if(depth == maxd)
	{
		return;// Evaluation(s);
	}
	
	std::cout << "Player: " << s.playerTurn << std::endl;
	std::cout << "Player: " << turn << std::endl;
	if(turn == 1)
	{
		s.playerTurn = 1;
		std::vector<PieceAction> Moves = GetLegalActions1(s);

		for(int i = 0; i < Moves.size(); i++)
		{
			PieceAction tempMove = PieceAction(Moves[i].from, Moves[i].to, depth, 1);
			BoardState temp = ApplyAction(s, Moves[i]);
			
			if(alpha >= beta)
			{
				bool updated = false;
				
				for(int j = 0; j < move_history.size(); j++)
				{
					if(depth == move_history[j].depth && turn == move_history[j].player)
					{
						move_history[j].from = tempMove.from;
						move_history[j].to = tempMove.to;
						
						std::cout << "Updated move from: " << tempMove.from << " to: " << tempMove.to << std::endl;
						
						updated = true;
						
						break;
					}
					
				}
				
				if(!updated)
				{
				
					move_history.push_back(tempMove);
				}
				
				result = Evaluation(temp);
				std::cout << "Player 1 Result: " << result << std::endl;
				//alpha = Evaluation(temp);
				//return Evaluation(temp);
			}
			
			if(alpha < Evaluation(temp))
			{
				bool updated = false;
				
				for(int j = 0; j < move_history.size(); j++)
				{
					if(depth == move_history[j].depth && turn == move_history[j].player)
					{
						move_history[j].from = tempMove.from;
						move_history[j].to = tempMove.to;
						
						std::cout << "Updated move from: " << tempMove.from << " to: " << tempMove.to << std::endl;
						
						updated = true;
						
						break;
					}
					
				}
				
				if(!updated)
				{
				 
					move_history.push_back(tempMove);
				}				
				alpha = Evaluation(temp);
				std::cout << "NEW ALPHA VALUE: " << alpha << std::endl;
			}
			
			else
			{
				break;
			}
			
			temp.playerTurn = 2;
			Run(temp, 2, depth + 1, alpha, beta);
			
			UndoAction(temp, Moves[i]);
		}
		
		Moves.clear();
		return;// alpha;
	}
	
	if(turn == 2)
	{
		s.playerTurn = 2;
		std::vector<PieceAction> Moves = GetLegalActions2(s);
		
		for(int i = 0; i < Moves.size(); i++)
		{
			PieceAction tempMove = PieceAction(Moves[i].from, Moves[i].to, depth, 2);
			BoardState temp = ApplyAction(s, Moves[i]);
			
			if(alpha >= beta)
			{
				bool updated = false;
				
				
				for(int j = 0; j < move_history.size(); j++)
				{
					if(depth == move_history[j].depth && turn == move_history[j].player)
					{
						move_history[j].from = tempMove.from;
						move_history[j].to = tempMove.to;
						
						std::cout << "Updated move from: " << tempMove.from << " to: " << tempMove.to << std::endl;

						updated = true;
						
						break;
					}
					
				}
				
				if(!updated)
				{
				 
					move_history.push_back(tempMove);
				}
				
				result = Evaluation(temp);
				std::cout << "Player 2 Result: " << result << std::endl;
			}
			
			if(beta > Evaluation(temp))
			{
				bool updated = false;
				
				for(int j = 0; j < move_history.size(); j++)
				{
					if(depth == move_history[j].depth && turn == move_history[j].player)
					{
						move_history[j].from = tempMove.from;
						move_history[j].to = tempMove.to;
						
						std::cout << "Updated move from: " << tempMove.from << " to: " << tempMove.to << std::endl;
						
						updated = true;
						
						break;
					}
					
				}
				
				if(!updated)
				{
					move_history.push_back(tempMove);
				}
				
				beta = Evaluation(temp);
				std::cout << "NEW BETA VALUE: " << beta << std::endl;
			}
			
			else
			{
				break;
			}
			
			temp.playerTurn = 1;
			Run(temp, 1, depth + 1, alpha, beta);
		}
		
		Moves.clear();
				
		return;// beta;
	}
	
//	if (s.board != 0 && !GameOver(s))
//	{
//		std::sort(Boards.begin(), Boards.end(), SortSuccessors);

//		if(s.playerTurn == 1)
//		{
//			P1LastScore = Evaluation(s);
//		}
//		if(s.playerTurn == 2)
//		{
//			P2LastScore = Evaluation(s);
//		}
		

		/*
			for(int j = 0; j < Boards.size(); j++)
			{
				if(s.playerTurn == 1)
				{
					if(P1LastScore == Evaluation(Boards[j]))
					{
						continue;
					}
					
					else
					{
						
						for(int i = 0; i < 64; i++)
						{
							s.board[i] = Boards[j].board[i];
						}
						P1LastScore = Evaluation(s);

					}
				}
				
				if(s.playerTurn == 2)
				{
					if(P2LastScore == Evaluation(Boards[j]))
					{
						continue;
					}
					
					else
					{
						
						for(int i = 0; i < 64; i++)
						{
							s.board[i] = Boards[j].board[i];
						}
						P2LastScore = Evaluation(s);
					}
				}
			}
		*/
		
//		int bindex = 0;
//		for(int j = 0; j < Boards.size(); j++)
//		{
//			if(Evaluation(s) == Evaluation(Boards[0]))
//			{
//				std::cout << "DUPLICATE" << std::endl;
//				return;
//				continue;
//			}
//			else {
//				bindex = j;
//				break;
//			}
//		}
		
		
//		for(int i = 0; i < 64; i++)
//		{
//			s.board[i] = Boards[bindex].board[i];
//		}

		/*
		for(int i = 0; i < Boards.size(); i++)
		{
			std::cout << "Start **************************************" << std::endl;
			PrintBoard(Boards[i]);
			std::cout << "************************************** End" << std::endl;
		}
		 */
//		std::cout << "Score of current state: " << Evaluation(s) << std::endl;
//		std::cout << "Depth: " << d << std::endl;
//		PrintBoard(s);
		
//		s.playerTurn = GetOppositePlayer(s.playerTurn);
//		Run(s, d + 1);
//	}

	if(GameOver(s))
	{
		std::cout << "Game Over. ";
		if(P1Win(s))
		{
			std::cout << "Player 1 Wins!" << std::endl;
		}
		if(P2Win(s))
		{
			std::cout << "Player 2 Wins!" << std::endl;
		}
		
		return;
	}
		
	for(int i = 0; i < move_history.size(); i++)
	{
		std::cout << move_history[i].from << " " << move_history[i].to << std::endl;
	}
	
	playMoves(s);
	
	return;// 0;
}

void playMoves(BoardState &s)
{
	for(int i = 0; i < move_history.size(); i++)
	{
		s = ApplyAction(s, move_history[i]);
		PrintBoard(s);
		
		if(s.playerTurn == 1)
		{
			s.playerTurn = 2;
		}
		else {
			s.playerTurn = 1;
		}
	}
	
	PrintTT();//Table);
}

void PrintTT()//std::map<uint64_t,TTEntry> &Table)
{
	std::cout << "Printing Transposition Table:" << std::endl;
	//std::map<uint64_t,TTEntry> (*myTable) = Table;
	std::map<uint64_t,TTEntry>::iterator it;
	
	uint64_t x = GetHashFromState(GetStartState());
	
	std::cout << Table.begin()->first << std::endl;
	//std::cout << Table[x] << std::endl;
	for(std::map<uint64_t,TTEntry>::iterator it=Table.begin(); it!=Table.end(); ++it)
	{
		std::cout << it->first << " => " <<  '\n';
	}

	while (!Table.empty())
	{
		std::cout << Table.begin()->first << " => " << '\n';
		Table.erase(Table.begin());
	}
	
	return;
	
}

