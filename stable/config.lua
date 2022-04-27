-- Config file for 3d game

-- Screen Size

ScreenWidth = 1024
ScreenHeight = 768

WorldWidth = 30
WorldHeight = 15

-- World Map as a string

WorldMap = {}

WorldMap[-7] = '##############################'
WorldMap[-6] = '##############################'
WorldMap[-5] = '##############################'
WorldMap[-4] = '########432112344321##########'
WorldMap[-3] = '########+----=-----+##########'
WorldMap[-2] = '########|##########|##########'
WorldMap[-1] = '########"##########|##########'
WorldMap[0] =  '########|##########|##########'
WorldMap[1] =  '########+-------=--+##########'
WorldMap[2] =  '##############################'
WorldMap[3] =  '##############################'
WorldMap[4] =  '##############################'
WorldMap[5] =  '##############################'
WorldMap[6] =  '##############################'
WorldMap[7] =  '##############################'

-- Textures

Textures =
{
	Grass = 'sprites/roads/grass.png',
	RoadTile = 'sprites/roads/road_tile.jpg',
	Road = 'sprites/roads/horizontal.jpg',
	RoadCrosswalk = 'sprites/roads/horizontal_crosswalk.jpg',
	Intersection = 'sprites/roads/intersection.jpg',
	RightRoad = 'sprites/roads/right.jpg',
	Car = 'sprites/cars/car.png',
	Building1Floor = 'sprites/buildings/windows.png',
	Building1Roof = 'sprites/buildings/roof.png',
}
