#include <random>
#include <chrono>
#include <algorithm>
#include "AI.h"
#include "Controller.h"

AI::AI(int playerNum, Shade shade) : Player(playerNum, shade)
{

}

void AI::Prepare(Field& field)
{

}

void AI::Turn(View& view, Controller& controller, Field& field)
{
	std::shared_ptr<PositionPriority> bestTurn;

	//��������� ���� �� ������ ����� �� ��������� ������� ���� ����������, ���� ���� �������� �� ��� ����
	bestTurn = GetPieceNearFinishTiles(field);

	//���� ������ ��� ���� �� �������
	if (bestTurn == nullptr)
	{
		//��������� ���� �� ������ ������� ����� ������ �� ���� ����������, ���� ���� �������� �� ��� ���� 
		//��� ����� ��� ���� ����� ����������� ������ �� ���� ����������
		bestTurn = GetPieceToMoveOnFinishTiles(field);
	}

	//���� ������ ��� ���� �� �������
	if (bestTurn == nullptr)
	{
		//������� ������ ��������� �����
		ClearPositionPriorities();

		for (auto& piece : pieces)
		{
			//��������� ��� ������ ��������� �� �� ���� ����������
			std::shared_ptr<Tile> pieceTile = field.GetTile(piece->GetPosition());
			if (field.CheckEnemyBaseTile(pieceTile, playerNum))
			{
				continue;
			}

			//���������� ��������� ����� � ���������� ����� �� ���� ���������� � ��������� � ������ ��������� �����
			//(���� �������������� ��� ����� ������ �����)
			std::vector<std::shared_ptr<PositionPriority>> turnTilesPathIgnorePieces = GetTurnTilesPath(field, piece, true);
			CalcAndPushTurnTilesPriority(turnTilesPathIgnorePieces, false);
			//���������� ��������� ����� � ���������� ����� �� ���� ���������� � ��������� � ������ ��������� �����
			//(���� �������������� � ������ ������ �����)
			std::vector<std::shared_ptr<PositionPriority>> turnTilesPathNotIgnorePieces = GetTurnTilesPath(field, piece, false);
			CalcAndPushTurnTilesPriority(turnTilesPathNotIgnorePieces, true);
		}

		//������������ ������ ��������� ����� � ��������� ���� �� ����������
		SortPositionPriorities();

		//������ ��������� ��� �� ������ ����� ���������� ��� ������
		bestTurn = GetBestTurn(field);
	}

	//�����
	field.MovePiece(bestTurn->position, bestTurn->piece);

	//�������� ��� ���������� ������
	controller.NextPlayerTurn(field);
}

std::vector<std::shared_ptr<PositionPriority>> AI::GetTurnTilesPath(Field& field, std::shared_ptr<Piece> piece, bool ignorePieces)
{
	std::vector<std::shared_ptr<Tile>> turnTiles = field.GetTilesForTurn(piece->GetPosition(), true);
	std::vector<std::shared_ptr<PositionPriority>> piecePositionPriorities;

	//������� ������ �� ������� ����� �������� ������
	for (auto& turnTale : turnTiles)
	{
		std::vector<Position> path = GetBestPath(turnTale->position, field, ignorePieces);

		//���� ���� ���, �� ��������� � ��������� ������
		if (path.size() == 0)
		{
			continue;
		}

		//���������� ������ ��� ��������� ��� ����
		piecePositionPriorities.push_back(std::make_shared<PositionPriority>(piece, turnTale->position, int (path.size())));
	}

	return piecePositionPriorities;
}

void AI::CalcAndPushTurnTilesPriority(std::vector<std::shared_ptr<PositionPriority>>& turnTilesPath, bool startFromOne)
{
	int oldStepCount = 0;
	int priority = 0;

	//��� ������� ���������� ����, � ������� ������ �����, ����� �������� ������� ��������� � �������
	if (startFromOne)
	{
		priority = 1;
	}

	//��������� ��� ��������� ���� ������ �� ���������� ����� �� ���� ����������
	std::sort(turnTilesPath.begin(), turnTilesPath.end(), [](const auto& a, const auto& b) {return a->stepCounts < b->stepCounts; });

	//����������� ���� ��������� ����� ������ ���������, � ����������� �� ���������� ����� �� ���� ����������
	for (auto& positionPriority : turnTilesPath)
	{
		if (oldStepCount != positionPriority->stepCounts)
		{
			oldStepCount = positionPriority->stepCounts;
			priority += 2;
		}
		//���������� ��������� ��� ���� � ��������� ��� � ������ ��������� �����
		positionPriority->priority = priority;
		positionPriorities.push_back(positionPriority);
	}
}

void AI::ClearPositionPriorities()
{
	//������� ������ ��������� �����
	positionPriorities.clear();
}

void AI::SortPositionPriorities()
{
	//������������ ������ �����
	auto seed = unsigned int(std::chrono::system_clock::now().time_since_epoch().count());
	std::shuffle(std::begin(positionPriorities), std::end(positionPriorities), std::default_random_engine(seed));

	//��������� ��� ��������� ���� ���� ����� �� ����������
	std::sort(positionPriorities.begin(), positionPriorities.end(), [](const auto& a, const auto& b) {return a->priority < b->priority; });
}

std::shared_ptr<PositionPriority> AI::GetBestTurn(Field& field) const
{
	std::shared_ptr<PositionPriority> bestTurn;

	//���������� ������ ���
	for (auto& positionPriority : positionPriorities)
	{
		//���� �� ������ ��� ���� ��������� ������, �� ��������� � �������� ���������� ����
		std::shared_ptr<Tile> tile = field.GetTile(positionPriority->position);
		if (tile->piece != nullptr)
		{
			continue;
		}

		//���� ������ ���� �� ���� ������ �� ������� ����, �� ��������� � �������� ���������� ����
		Position oldPosition = positionPriority->piece->GetOldPosition();
		if ((positionPriority->position.x == oldPosition.x) && (positionPriority->position.y == oldPosition.y))
		{
			continue;
		}

		//���������� ������� ��� ��� ������
		bestTurn = positionPriority;
		break;
	}

	return bestTurn;
}

std::shared_ptr<PositionPriority> AI::GetPieceNearFinishTiles(Field& field) const
{
	std::shared_ptr<PositionPriority> bestTurn;

	for (auto& piece : pieces)
	{
		//���� ������ ��������� �� ���� ����������, �� ��������� � ��������� ������
		std::shared_ptr<Tile> pieceTile = field.GetTile(piece->GetPosition());
		if (field.CheckEnemyBaseTile(pieceTile, playerNum))
		{
			continue;
		}

		//���� ������ ���� ���������� ����� � �������
		std::vector<std::shared_ptr<Tile>> turnTiles = field.GetTilesForTurn(piece->GetPosition(), false);
		for (auto& turnTile : turnTiles)
		{
			if (field.CheckEnemyBaseTile(turnTile, playerNum))
			{
				bestTurn = std::make_shared<PositionPriority>(piece, turnTile->position);
				break;
			}
		}
	}

	return bestTurn;
}

std::shared_ptr<PositionPriority> AI::GetPieceToMoveOnFinishTiles(Field& field) const
{
	std::shared_ptr<PositionPriority> bestTurn;

	for (auto& piece : pieces)
	{
		//���� ������ ��������� �� �� ���� ���������� �� ��������� � ��������� ������
		std::shared_ptr<Tile> tile = field.GetTile(piece->GetPosition());
		if (!field.CheckEnemyBaseTile(tile, playerNum))
		{
			continue;
		}

		//��������� ��������� ������ ��� �������
		int currentRespawnPriority = tile->respawnPriority;

		std::vector<std::shared_ptr<Tile>> turnTiles = field.GetTilesForTurn(piece->GetPosition(), true);
		//���������� ���������� �������� ������ � ��� �� ������� ��������� ������
		for (auto& turnTile : turnTiles)
		{
			//���� �������� ������ �� ������ ��������� � ��������� ������ 
			if (turnTile->piece != nullptr)
			{
				continue;
			}

			//���� ��������� ������ ���� ��� ��� ��� � ������ ��� ������� ���������� ������ � ������� ������ ��� ����
			if (turnTile->respawnPriority > currentRespawnPriority)
			{
				bestTurn = std::make_shared<PositionPriority>(piece, turnTile->position);
				continue;
			}
		}
	}

	return bestTurn;
}

std::vector<Position> AI::GetBestPath(Position position, Field& field, bool ignorePieces) const
{
	std::vector<Position> bestPath;
	std::vector<std::shared_ptr<Tile>> finishTiles = field.GetFinishTiles(playerNum);
	std::sort(finishTiles.begin(), finishTiles.end(), [](const auto& a, const auto& b) {return a->respawnPriority < b->respawnPriority; });

	size_t stepCount = size_t (std::numeric_limits<int>::max());
	//����������� ����� ���� �� ������ ������ ���� ���������� � �������� ��������� ����
	for (auto& tile : finishTiles)
	{
		//���� �� ������ ����� ���� ������, ��������� � ��������� ������
		if ((tile->piece != nullptr) && (tile->piece->GetPlayerNum() == playerNum))
		{
			continue;
		}

		std::vector<Position> path = GetPath(position, tile->position, field, ignorePieces);
		//���� ���� �� ������ �� ������, ��������� � ��������� ������
		if (path.size() == 0)
		{
			continue;
		}

		//���� ���� �� ������ �� ������ ��� � ���������� ������, ��������� � ��������� ������
		if (path.size() > stepCount)
		{
			continue;
		}

		//���������� ���� ��� ���������
		stepCount = path.size();
		bestPath = path;
	}

	return bestPath;
}

std::vector<Position> AI::GetPath(Position startPosition, Position endPosition, Field& field, bool ignorePieces) const
{
	const int width = field.GetWidth();
	const int height = field.GetHeight();

	std::vector<std::vector<std::vector<int>>> map;

	//�������������� ����� ��� ������ ����
	map.resize(width);
	for (int i = 0; i < width; i++)
	{
		map[i].resize(height);
		for (int j = 0; j < height; j++)
		{
			map[i][j].resize(3, 0);

			std::shared_ptr<Tile> tile = field.GetTile(i, j);
			if (tile->piece != nullptr)
			{
				map[i][j][0] = -2;

				if (ignorePieces)
				{
					if (tile->piece->GetPlayerNum() != 0)
					{
						map[i][j][0] = -1;
					}
				}
			}
			else
			{
				map[i][j][0] = -1;
			}
		}
	}

	int step = 0;
	bool added = true;
	bool result = true;

	std::vector<Position> path;

	map[endPosition.x][endPosition.y][0] = -1;
	map[startPosition.x][startPosition.y][0] = 0;

	//���� ����
	while (added && map[endPosition.x][endPosition.y][0] == -1)
	{
		added = false;
		step++;
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				if (map[i][j][0] == step - 1)
				{
					int _i, _j;

					_i = i + 1; _j = j;
					if (_i >= 0 && _j >= 0 && _i < width && _j < height)
					{
						if (map[_i][_j][0] == -1 && map[_i][_j][0] != -2)
						{
							map[_i][_j][0] = step;
							map[_i][_j][1] = i;
							map[_i][_j][2] = j;
							added = true;
						}
					}
					_i = i - 1; _j = j;
					if (_i >= 0 && _j >= 0 && _i < width && _j < height)
					{
						if (map[_i][_j][0] == -1 && map[_i][_j][0] != -2)
						{
							map[_i][_j][0] = step;
							map[_i][_j][1] = i;
							map[_i][_j][2] = j;
							added = true;
						}
					}
					_i = i; _j = j + 1;
					if (_i >= 0 && _j >= 0 && _i < width && _j < height)
					{
						if (map[_i][_j][0] == -1 && map[_i][_j][0] != -2)
						{
							map[_i][_j][0] = step;
							map[_i][_j][1] = i;
							map[_i][_j][2] = j;
							added = true;
						}
					}
					_i = i; _j = j - 1;
					if (_i >= 0 && _j >= 0 && _i < width && _j < height)
					{
						if (map[_i][_j][0] == -1 && map[_i][_j][0] != -2)
						{
							map[_i][_j][0] = step;
							map[_i][_j][1] = i;
							map[_i][_j][2] = j;
							added = true;
						}
					}
				}
			}
		}
	}

	//���� �� ������
	if ((map[endPosition.x][endPosition.y][0] == -1) || (step == 0))
	{
		result = false;
	}

	//���� ���� ������, ���������� ���
	if (result) 
	{
		int _i = endPosition.x, _j = endPosition.y;

		while (map[_i][_j][0] != 0)
		{
			std::shared_ptr<Tile> tile = field.GetTile(_i, _j);
			path.push_back(tile->position);
			int li = map[_i][_j][1];
			int lj = map[_i][_j][2];
			_i = li; _j = lj;
		}
	}

	return path;
}

AI::~AI()
{

}
