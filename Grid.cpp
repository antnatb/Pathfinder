//
// Created by anton on 03/10/2022.
//

#include "Grid.h"

Grid::Grid(int x, int y):width(x), height(y) {
    cellSide = int(sf::VideoMode::getDesktopMode().width) / (x);
    for (int i = 0; i < x; i++) {
        cells.emplace_back();
        for (int j = 0; j < y ; j++){
            Cell cell(i * cellSide, j * cellSide, cellSide);
            cells[i].push_back(cell);
        }
    }
    setStart();
    setTarget();
}

Cell *Grid::findFreeCell() {
    srand(time(0));
    bool found = false;
    Cell *cell = nullptr;
    while (!found) {
        int x = rand() % width;
        int y = rand() % height;
        cell = &cells[x][y];
        if (!cell->isStart() && !cell->isObstacle() && !cell->isTarget())
            found = true;
    }
    return cell;
}

void Grid::setStart() {
    start = findFreeCell();
    start->makeStart();
}

void Grid::setTarget() {
    target = findFreeCell();
    target->makeTarget();
}

void Grid::update(sf::RenderWindow &window) {
    for (auto &row : cells)
        for (auto &cell : row)
            cell.update(window);
}

void Grid::draw(sf::RenderWindow &window) {
    for (auto &row : cells)
        for (auto &cell : row)
            cell.draw(window);
}

void Grid::findPath() {
    reset();
    availableCells.push_back(start);
    start->setAvailable(true);
    bool path = true;
    while (current != target && path) {
        current = findLowestCostCell();
        availableCells.remove(current);
        current->setAvailable(false);
        evaluatedCells.push_back(current);
        current->evaluate();
        addNeighbors(*current);
        evaluateNeighbors(*current);
        if (availableCells.empty())
            path = false;
    }
    if (path)
        traceBackPath();
}


Cell *Grid::findLowestCostCell() {
    float lowestCost = 9999.9;
    Cell *lowestCostCell = nullptr;
    for (auto &cell: availableCells) {
        if (cell->getFCost() < lowestCost || (cell->getFCost() == lowestCost && cell->getHCost() < lowestCostCell->getHCost())) {
            lowestCostCell = cell;
            lowestCost = cell->getFCost();
        }
    }
    return lowestCostCell;
}

void Grid::addNeighbors(Cell &cell) {
    int x = cell.getX() / cellSide;
    int y = cell.getY() / cellSide;

    //top
    if (y + 1 < height)
        if (!cells[x][y + 1].isStart() && !cells[x][y + 1].isObstacle()) {
            cell.addNeighbor(cells[x][y + 1]);
        }

    //right
    if (x + 1 < width)
        if (!cells[x + 1][y].isStart() && !cells[x + 1][y].isObstacle()) {
            cell.addNeighbor(cells[x + 1][y]);
        }

    //bottom
    if (y - 1 >= 0)
        if (!cells[x][y - 1].isStart() && !cells[x][y - 1].isObstacle()) {
            cell.addNeighbor(cells[x][y - 1]);
        }

    //left
    if (x - 1 >= 0)
        if (!cells[x - 1][y].isStart() && !cells[x - 1][y].isObstacle()) {
            cell.addNeighbor(cells[x - 1][y]);
        }

    if (diagonalMovement) {
        //top right
        if (y + 1 < height && x + 1 < width)
            if (!cells[x + 1][y + 1].isStart() && !cells[x + 1][y + 1].isObstacle() && !(cells[x][y+1].isObstacle() && cells[x+1][y].isObstacle())) {
                cell.addNeighbor(cells[x + 1][y + 1]);
            }

        //bottom right
        if (y - 1 >= 0 && x + 1 < width)
            if (!cells[x + 1][y - 1].isStart() && !cells[x + 1][y - 1].isObstacle() && !(cells[x][y-1].isObstacle() && cells[x+1][y].isObstacle())) {
                cell.addNeighbor(cells[x + 1][y - 1]);
            }

        //bottom left
        if (y - 1 >= 0 && x - 1 >= 0)
            if (!cells[x - 1][y - 1].isStart() && !cells[x - 1][y - 1].isObstacle() && !(cells[x][y-1].isObstacle() && cells[x-1][y].isObstacle())) {
                cell.addNeighbor(cells[x - 1][y - 1]);
            }

        //top left
        if (x - 1 >= 0 && y + 1 < height)
            if (!cells[x - 1][y + 1].isStart() && !cells[x - 1][y + 1].isObstacle() && !(cells[x][y-1].isObstacle() && cells[x-1][y].isObstacle())) {
                cell.addNeighbor(cells[x - 1][y + 1]);
            }
    }
}

void Grid::reset() {
    for (auto &row : cells)
        for (auto &cell: row)
            cell.reset();
    availableCells.clear();
    evaluatedCells.clear();
    current = nullptr;
}

void Grid::traceBackPath() {
    current = target->getParent();
    while (current != start) {
        current->visit();
        current = current->getParent();
    }
}

void Grid::evaluateNeighbors(Cell &cell) {
    for (auto neighbor: cell.getNeighbors()) {
        if (neighbor->isObstacle() || neighbor->isEvaluated())
            continue;
        float newGCost = cell.getGCost() + sqrtf(pow(cell.getX() - neighbor->getX(), 2) +
                                                     pow(cell.getY() - neighbor->getY(), 2));
        if (newGCost < neighbor->getGCost() || !neighbor->isAvailable()) {
            neighbor->setGCost(newGCost);
            neighbor->computeHCost(*target);
            neighbor->computeFCost();
            neighbor->setParent(cell);
            if (!neighbor->isAvailable()) {
                availableCells.push_back(neighbor);
                neighbor->setAvailable(true);
            }
        }

    }
}



