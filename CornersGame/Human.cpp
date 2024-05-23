#include "Human.h"
#include "View.h"
#include "Controller.h"

Human::Human(int playerNum, Shade shade) : Player(playerNum, shade)
{

}

void Human::Prepare(Field& field)
{

}

void Human::Turn(View& view, Controller& controller, Field& field)
{
	Position mousePosition = view.GetMousePosition();
	//��������� ��������� �� ������ � �������� �����
	if ((mousePosition.x < 0) ||
		(mousePosition.y < 0) ||
		(mousePosition.x >= field.GetWidth() * field.GetTileSize()) ||
		(mousePosition.y >= field.GetHeight() * field.GetTileSize()))
	{
		return;
	}

	//���� ����� ������ �� �� ������, �� ���������� ����
	if ((view.GetLeftMousePressed()) && (!mouseIsDown))
	{
		mouseIsDown = true;
		mouseClick = true;
	}

	//���� ����� ������, �� ����� �������� ��� ���
	if (!view.GetLeftMousePressed())
	{
		mouseIsDown = false;
	}

	//��� ����� ������
	if (mouseClick)
	{
		mouseClick = false;

		std::shared_ptr<Tile> tileSelected = field.GetTile(mousePosition.x / field.GetTileSize(), mousePosition.y / field.GetTileSize());
		std::shared_ptr<Piece> tilePice = tileSelected->piece;

		//���� ������ ������� � �������� �� ������ ��������� ��� ����, �� ����� �������
		if (pieceSelected != nullptr)
		{
			std::vector<std::shared_ptr<Tile>> tTiles = field.GetTilesForTurn(pieceSelected->GetPosition(), false);
			for (auto& tile : tTiles)
			{
				if ((tile->position.x == tileSelected->position.x) && (tile->position.y == tileSelected->position.y))
				{
					field.MovePiece(tile->position, pieceSelected);
					pieceSelected = nullptr;
					controller.NextPlayerTurn(field);
					break;
				}
			}
		}

		//���� �������� �� ������ ������ ��� �� ������ � ��������� �������, �� ������� ��������� ������
		if ((tilePice == nullptr) || (tilePice->GetPlayerNum() != playerNum))
		{
			pieceSelected = nullptr;
		}
		//����� ���������� ������ �� ������ ��� ���������
		else
		{
			pieceSelected = tilePice;
		}

		//���� ������ �������, �� �������� ������ �� ������� ��� ����� ��������
		std::vector<std::shared_ptr<Tile>> turnTiles;
		if (pieceSelected != nullptr)
		{
			turnTiles = field.GetTilesForTurn(tileSelected->position, false);
		}

		//�������� ������� ������ �� ������� ����� �������� ������
		std::vector<Position> turnPositions;
		for (auto& tile : turnTiles)
		{
			turnPositions.push_back(tile->position);
		}

		//��������� ��� ������ �� ������� ����� �������� ������ ����� �������� �� ������
		view.ShowTurn(turnPositions);
	}
}

Human::~Human()
{

}
