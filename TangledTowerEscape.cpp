#include <bangtal>
#include <string>
#include <cmath>

#include <iostream>

using namespace std;
using namespace bangtal;
using namespace std;


typedef struct  {
	ScenePtr scene, parentScene;
	ObjectPtr backButton;
}SubScene;

typedef struct  {
	ObjectPtr object;
	int x, y;
	bool toRight;
	ScenePtr scene;
}DuplicatedObject;


int currentStage = 0;
ScenePtr currentScene = nullptr;



class Item : public Object {
public:
	static ObjectPtr create(const string& image, ScenePtr scene = nullptr, int x = 0, int y = 0, bool shown = true) {
		auto object = ObjectPtr(new Item(image, scene, x, y, shown));
		Object::add(object);
		return object;
	}

protected:
	Item(const string& image, ScenePtr scene, int x, int y, bool shown) : Object(image, scene, x, y, shown) {}

public:
	virtual bool onMouse(int x, int y, MouseAction action) {
		if (!Object::onMouse(x, y, action)) 
			pick();
		return true;
	}
};

class Button : public Object{
	ScenePtr sceneToMove;

public:
	static ObjectPtr create(const string& image, ScenePtr scene = nullptr, int x = 0, int y = 0, ScenePtr move = nullptr) {
		auto object = ObjectPtr(new Button(image, scene, x, y, move));
		object->setScale(0.3f);
		Object::add(object);
		return object;
	}

protected:
	Button(const string& image, ScenePtr scene, int x, int y, ScenePtr move) : Object(image, scene, x, y, true), sceneToMove(move) {}

public:
	virtual bool onMouse(int x, int y, MouseAction action) {
		if (!Object::onMouse(x, y, action)) {
			currentScene = sceneToMove;
			sceneToMove->enter();
		} return true;
	}
};

typedef struct {
	ObjectPtr object;
	int location;
} puzzlePiece;

class Puzzle {
public:
	ScenePtr scene;
	int num;
	puzzlePiece* board;

private:
	int x, xd, y, yd, sqrtN;
	puzzlePiece blank;
	static int count;
	const int MAX_COUNT = 20;
	ScenePtr nextScene;
	ObjectPtr reward;

public:
	Puzzle(ScenePtr s, int x, int xd, int y, int yd, int n, ScenePtr next, ObjectPtr reward) : scene(s), x(x), xd(xd), y(y), yd(yd), num(n), sqrtN(sqrt(num)), nextScene(next), reward(reward) {
		board = new puzzlePiece[num + 1];
		blank.object = Object::create("images/puzzle2/16.png", scene); blank.object->setScale(0.35f);
		blank.location = num;
		board[num] = blank;

		for (int i = 1; i < num; i++) {
			auto filename = "images/puzzle/" + to_string(i) + ".png";
			board[i].object = Object::create(filename, scene, 0, 0, false);
			locate_puzzle(board[i].object, scene, i, sqrtN, x, xd, y, yd);
			board[i].object->setScale(0.35f);
			board[i].location = i;


			board[i].object->setOnMouseCallback([&](ObjectPtr object, int, int, MouseAction)->bool {
				int n = 1;
				for (; n <= num; n++)
					if (board[n].object == object) break;


				if (isMoved(board[n], blank))
					isSolved();


				return true;
				});

		}
	}
	
	void locate_puzzle(ObjectPtr object, ScenePtr scene, int i, int n, int x, int xd, int y, int yd);
	bool isMoved(puzzlePiece& p, puzzlePiece& b);
	void mix_puzzle();
	void resetting();
	void isSolved();
};

int Puzzle::count = 0;


void Puzzle::locate_puzzle(ObjectPtr object, ScenePtr scene, int i, int n, int x, int xd, int y, int yd) {
	object->locate(scene, x + ((i - 1) % n) * xd, y - ((i - 1) / n) * yd);
}
bool Puzzle::isMoved(puzzlePiece& p, puzzlePiece& b) {
	if (((p.location % sqrtN) != 1 && (p.location - 1) == b.location) ||			// left
		((p.location % sqrtN) != 0 && (p.location + 1) == b.location) ||			// right
		(p.location > sqrtN && (p.location - sqrtN) == b.location) ||				// up
		(p.location <= (num - sqrtN) && (p.location + sqrtN) == b.location))		// down
	{
		int temp = p.location;
		p.location = b.location;
		b.location = temp;

		locate_puzzle(p.object, scene, p.location, sqrtN, x, xd, y, yd);
		locate_puzzle(b.object, scene, b.location, sqrtN, x, xd, y, yd);

		return true;
	}
	return false;
}
void Puzzle::mix_puzzle() {
	blank.object->hide();

	auto timer = Timer::create(0.1f);

	timer->setOnTimerCallback([&](TimerPtr t)->bool {
		//cout << "timeout " << count << endl;
		int newBlank = 0;

		bool isCounted = false;
		do {
			do {
				switch (rand() % 4) {
				case 0:		//left
					newBlank = blank.location - 1;		break;
				case 1:		//right
					newBlank = blank.location + 1;		break;
				case 2:		//up
					newBlank = blank.location - sqrtN;		break;
				case 3:		//down
					newBlank = blank.location + sqrtN;		break;
				default:
					break;
				}
			} while (newBlank < 1 || newBlank >(sqrtN * sqrtN));

			int i = 1;
			for (; i <= (sqrtN * sqrtN); i++)
				if (board[i].location == newBlank) break;

			if (isMoved(board[i], blank))
				isCounted = true;

		} while (!isCounted);


		count++;
		if (count < MAX_COUNT) {
			t->set(0.1f);
			t->start();
		}

		return true;
		});

	timer->start();

}
void Puzzle::resetting() {
	for (int i = 1; i < num; i++) {
		board[i].location = i;
		locate_puzzle(board[i].object, scene, i, sqrtN, x, xd, y, yd);
		board[i].object->hide();
	}
	blank.location = num;
	locate_puzzle(blank.object, scene, num, sqrtN, x, xd, y, yd);
	blank.object->hide();

	return;
}
void Puzzle::isSolved() {
	for (int i = 1; i <= num; i++) {
		if (board[i].location != i) return;
	}

	blank.object->show();
	reward->show();
	showMessage("Puzzle Complete!\nPick the map.");

	

	

	return;
}

void startStage(ScenePtr nextScene) {
	currentStage++;
	currentScene = nextScene;

	currentScene->enter();
	

	string filename = "images/stage" + to_string(currentStage) + "/tutorial.jpg";
	auto tutorial = Object::create(filename, currentScene);
	tutorial->setOnMouseCallback([](ObjectPtr object, int, int, MouseAction) -> bool {
		object->hide();  return true;
		});

}

void clearStage(ScenePtr nextscene) {
	string  scene_filename = "images/stage" + to_string(currentStage) + "/clear.jpg";
	auto clearScene = Object::create(scene_filename, currentScene);

	clearScene->setOnMouseCallback([=](ObjectPtr, int x, int y, MouseAction) -> bool {
		if (currentStage == 3) {
			currentStage++;
			clearScene->setImage("images/clear.jpg");
			clearScene->setOnMouseCallback([](ObjectPtr, int x, int y, MouseAction) -> bool {
				endGame(); return true;
				});
		}
		else if (x > 1000 && x < 1200 && y > 40 && y < 100) {
			startStage(nextscene);
		}
		return true;
		});

}



int main() {

	setGameOption(GameOption::GAME_OPTION_ROOM_TITLE, false);
	setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);



	const int totalStage = 3;
	const int MaxScene = 2;
	const int MaxItem = 8;



	ScenePtr sceneARR[totalStage + 1];

	/*=================================================== stage 1 ===============================================================*/

	// main scene
	auto scene1_1 = Scene::create("Stage 1", "images/stage1/main scene1.jpg");
	auto scene1_1_ob = Object::create("images/stage1/main scene1.jpg", scene1_1); scene1_1_ob->setScale(0.7f);
	sceneARR[1] = scene1_1;

	auto scene1_2 = Scene::create("Stage 1", "images/stage1/main scene2.jpg");
	auto scene1_2_ob = Object::create("images/stage1/main scene2.jpg", scene1_2); scene1_2_ob->setScale(0.7f);

	// subscene, 세부 장소 확대
	const int MaxSubScene = 5;
	SubScene* subSceneARR = new SubScene[MaxSubScene];
	for (int i = 0; i < MaxSubScene; i++) {
		string filename = "images/stage1/subscene" + to_string(i + 1) + ".jpg";
		subSceneARR[i].scene = Scene::create("subscene", filename);

		(i < 3) ? subSceneARR[i].parentScene = scene1_1 : subSceneARR[i].parentScene = scene1_2;
	}

	// item list 
	DuplicatedObject list;
	list.x = 1180; list.y = 30; list.scene = scene1_1;
	list.object = Object::create("images/stage1/list.png", list.scene, list.x, list.y); list.object->setScale(0.3f);


	// items
	ObjectPtr itemARR[MaxItem];
	static int item_count = 0;
	auto fryingPan = Item::create("images/item/pan.png", subSceneARR[0].scene, 975, 340);				fryingPan->setScale(0.4f);		itemARR[0] = fryingPan;
	auto apple = Item::create("images/item/apple.png", subSceneARR[1].scene, 630, 200);					apple->setScale(0.5f);			itemARR[1] = apple;
	auto bread = Item::create("images/item/bread.png", subSceneARR[1].scene, 800, 300);					bread->setScale(0.5f);			itemARR[2] = bread;
	auto match = Item::create("images/item/matches.png", subSceneARR[1].scene, 300, 400);				match->setScale(0.2f);			itemARR[3] = match;
	auto candle = Item::create("images/item/candle_off.png", subSceneARR[2].scene, 500, 535);			candle->setScale(0.2f);			itemARR[4] = candle;
	auto blanket = Item::create("images/item/blanket.png", subSceneARR[2].scene, 330, 60);				blanket->setScale(0.35f);		itemARR[5] = blanket;
	auto hairbrush = Item::create("images/item/hairbrush_behind.png", subSceneARR[3].scene, 500, 265);  hairbrush->setScale(0.15f);		itemARR[6] = hairbrush;
	auto tiara = Item::create("images/item/tiara.png.", subSceneARR[4].scene, 550, 490);				tiara->setScale(0.2f);			itemARR[7] = tiara;

	// godel
	DuplicatedObject godel;
	godel.x = 0; godel.y = 0; godel.scene = scene1_1;
	godel.object = Object::create("images/godel_R.png", godel.scene, godel.x, godel.y);	godel.object->setScale(0.45f);
	godel.toRight = true;

	// buttons
	auto leftButton = Button::create("images/leftbutton.png", scene1_1, 50, 300, scene1_2);
	auto rightButton = Button::create("images/rightbutton.png", scene1_2, 1200, 300, scene1_1);
	for (int i = 0; i < MaxSubScene; i++) {
		subSceneARR[i].backButton = Button::create("images/backbutton.png", subSceneARR[i].scene, 650, 20, subSceneARR[i].parentScene);
		subSceneARR[i].backButton->setOnMouseCallback([=](ObjectPtr, int, int, MouseAction) -> bool {
			list.object->locate(subSceneARR[i].parentScene, list.x, list.y); return false;
			});
	}

	//-----------------------------------------------------------------------------------------------------------------------------------//

	// enter subScene
	scene1_1_ob->setOnMouseCallback([&](ObjectPtr, int x, int y, MouseAction) -> bool {
		if (x > 790 && x < 970 && y >500 && y < 640) {
			subSceneARR[0].scene->enter();
			list.object->locate(subSceneARR[0].scene, list.x, list.y);
		}
		else if (x > 1000 && x < 1200 && y > 400 && y < 550) {
			subSceneARR[1].scene->enter();
			list.object->locate(subSceneARR[1].scene, list.x, list.y);
		}
		else if (x > 120 && x < 470 && y > 400 && y < 800) {
			subSceneARR[2].scene->enter();
			list.object->locate(subSceneARR[2].scene, list.x, list.y);
		}
		return true;
		});

	scene1_2_ob->setOnMouseCallback([&](ObjectPtr, int x, int y, MouseAction) -> bool {
		if (x > 190 && x < 540 && y >390 && y < 690) {
			subSceneARR[3].scene->enter();
			list.object->locate(subSceneARR[3].scene, list.x, list.y);
		}

		else if (x > 1000 && x < 1300 && y >350 && y < 600) {
			subSceneARR[4].scene->enter();
			list.object->locate(subSceneARR[4].scene, list.x, list.y);
		}

		return true;
		});

	// list
	list.object->setOnMouseCallback([](ObjectPtr, int, int, MouseAction) -> bool {
		showMessage("빗, 양초, 성냥, 담요, 프라이팬, 빵, 사과, 티아라");
		return true;
		});

	// count item
	for (int i = 0; i < MaxItem; i++) {
		itemARR[i]->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction) -> bool {
			item_count++;
			if (item_count == MaxItem) {
				godel.object->hide();
				list.object->hide();
				clearStage(sceneARR[2]);

			}

			return false;
			});
	}

	// set godel& at current scene
	leftButton->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction) -> bool {
		list.object->locate(scene1_2, list.x, list.y);
		godel.scene = scene1_2;
		return false;
		});

	rightButton->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction) -> bool {
		list.object->locate(scene1_1, list.x, list.y);
		godel.scene = scene1_1;
		return false;
		});



	// godel moving
	auto godel_Timer = Timer::create(0.1f);
	godel_Timer->setOnTimerCallback([&](TimerPtr t)->bool {
		if (godel.x > 900) {
			godel.object->setImage("images/godel_l.png");
			godel.object->setScale(0.45f);
			godel.toRight = false;
		}
		else if (godel.x < 0) {
			godel.object->setImage("images/godel_R.png");
			godel.toRight = true;
		}

		if (godel.toRight)	godel.object->locate(godel.scene, godel.x += 10, godel.y);
		else				godel.object->locate(godel.scene, godel.x -= 10, godel.y);

		t->set(0.1f);
		t->start();

		return true;
		});
	godel_Timer->start();

	// godel trap
	godel.object->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction) -> bool {
		auto trapScene = Scene::create("Stage 1", "images/stage1/trap.jpg");
		trapScene->enter();

		ObjectPtr A[4];
		for (int i = 0; i < 4; i++) {
			string name = "images/stage1/answer" + to_string(i + 1) + ".png";
			A[i] = Object::create(name, trapScene, 720, (380 - i * 90));
			A[i]->setScale(0.9f);

			A[i]->setOnMouseCallback([=](ObjectPtr, int, int, MouseAction) -> bool {
				if (i == 3) {
					(godel.scene == scene1_1) ? scene1_1->enter() : scene1_2->enter();
				}
				else {
					auto failScene = Scene::create("Stage 1", "images/fail.png");
					failScene->enter();
				}  return true;
				});
		}  return true;
		});



	/*=================================================== stage 2 ===============================================================*/

	//main scene
	auto scene2 = Scene::create("Stage 2", "images/stage2/main scene.png");
	auto scene2_ob = Object::create("images/stage2/main scene.png", scene2);
	sceneARR[2] = scene2;

	ObjectPtr paskal[2];
	paskal[0] = Object::create("images/stage2/paskal1.png", scene2, 800, 250); paskal[0]->setScale(0.15f);
	paskal[1] = Object::create("images/stage2/paskal2.png", scene2, 420, 200); paskal[1]->setScale(0.05f);

	auto foundCheck1 = Object::create("images/stage2/found.png", scene2, 800, 230, false); foundCheck1->setScale(0.1f);
	auto foundCheck2 = Object::create("images/stage2/found.png", scene2, 400, 180, false); foundCheck2->setScale(0.1f);


	static int paskalCount = 0;
	bool check1 = false, check2 = false;
	for (int i = 0; i < 2; i++) {
		paskal[i]->setOnMouseCallback([=, &check1, &check2](ObjectPtr, int, int, MouseAction) -> bool {
			if (i == 0 && !check1) {
				check1 = true;
				paskalCount++;
				foundCheck1->show();
			}
			else if (i == 1 && !check2) {
				check2 = true;
				paskalCount++;
				foundCheck2->show();
			}
			return true;
			});
	}

	bool windowCheck = false;
	scene2_ob->setOnMouseCallback([&](ObjectPtr, int x, int y, MouseAction) -> bool {
		if (windowCheck)
			clearStage(sceneARR[3]);
		else if (paskalCount == 2 && x > 180 && x < 310 && y>250 && y < 450) {
			scene2_ob->setImage("images/stage2/open.jpg");
			windowCheck = true;
		}
		return true;
		});


	/*=================================================== stage 3 ===============================================================*/

		//main scene
	auto scene3 = Scene::create("Stage 3", "images/stage3/main scene.png");
	auto puzzleScene = Scene::create("Puzzle", "images/puzzle/background.jpg");
	
	auto start = Object::create("images/puzzle/start.png", puzzleScene, 550, 350);
	auto restart = Object::create("images/puzzle/restartbutton.jpg", puzzleScene, 550, 330, false);
	restart->setScale(0.7f);

	auto map = Item::create("images/item/map.png", puzzleScene, 530, 300, false); map->setScale(0.2f);

	Puzzle puzzle(puzzleScene, 713, 106, 519, 158, 16, scene3, map);
	sceneARR[3] = puzzleScene;

	start->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)->bool {
		for (int j = 1; j < puzzle.num; j++)
			puzzle.board[j].object->show();
		puzzle.mix_puzzle();
		start->hide();
		restart->show();
		return true;
		});

	restart->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
		puzzle.resetting();
		restart->hide();
		start->show();
		return true;
		});


	static bool zoomMap = false;
	map->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {

		if (currentScene == puzzleScene) {
			map->locate(scene3, 1180, 30); map->setScale(0.1f);
			scene3->enter();
			currentScene = scene3;
		}
		
		else if (currentScene == scene3) {
			if (!zoomMap) {
				map->locate(scene3, 500, 0);
				map->setScale(1.f);
				zoomMap = true;
			}
			else {
				map->locate(scene3, 1180, 30);
				map->setScale(0.1f);
				zoomMap = false;
			}
		}
		
		return true;
		});

	auto timer = Timer::create(100.f);

	const int directionInfo[8][8] = { {6, 12, 4, 6, 14, 10, 12, 4},
										{5, 5, 7, 9, 5, 2, 11, 13},
										{5, 3, 13, 4, 5, 6, 12, 5},
										{3, 12, 5, 3, 15, 13, 5, 1},
										{4, 5, 3, 12, 1, 5, 7, 12},
										{7, 9, 6, 9, 6, 9, 1, 5},
										{5, 4, 5, 4, 1, 6, 12, 7},
										{3, 9, 1, 3, 10, 9, 3, 9} };

	static int player_x = 282, player_y = 550, player_location = -1;
	auto player = Object::create("images/stage3/player.png", scene3, player_x, player_y); player->setScale(0.3f);

	static int light_x = -920, light_y = -180;
	ObjectPtr candleLight = Object::create("images/stage3/lightOff.jpg", scene3, light_x, light_y); candleLight->setScale(1.3f);

	auto candleOn = Object::create("images/item/candle_on.png");
	candleOn->defineCombination(match, candle);
	candleOn->setOnCombineCallback([&](ObjectPtr) -> bool {
		candleLight->setImage("images/stage3/lightOn.png");
		showTimer(timer);
		timer->start();

		return true;
		});

	timer->setOnTimerCallback([](TimerPtr)->bool {
		auto failScene = Object::create("images/fail.png", currentScene);
		failScene->setOnMouseCallback([](ObjectPtr, int, int, MouseAction) -> bool {
			endGame();
			return true;
			});
		return true;
		});


	ObjectPtr directionButton[4];
	for (int i = 0; i < 4; i++) {
		string filename = "images/stage3/button" + to_string(i) + ".png";
		directionButton[i] = Object::create(filename, scene3);
		directionButton[i]->setScale(0.2f);

		directionButton[i]->setOnMouseCallback([&, i](ObjectPtr, int, int, MouseAction) -> bool {
			int c = player_location / 10;
			int r = player_location - c * 10;


			switch (i) {
			case 0:	// up(1)
				if ((directionInfo[c][r] & 1) == 1) {
					cout << (directionInfo[c][r] & 1) << "True" << endl;
					if (player_y + 80 < 650)
						player_y += 80; light_y += 80;
					player_location -= 10;
				} break;
			case 1:	// right(2)
				if (player_location == -1) {
					if (player_x + 78 < 920)
						player_x += 78; light_x += 78;
					player_location = 10;
				}
				else if ((directionInfo[c][r] & 2) == 2) {
					cout << (directionInfo[c][r] & 2) << "True" << endl;
					if (player_x + 78 < 920)
						player_x += 78; light_x += 78;
					if (player_location == 67) player_x = player_x + 78;
					player_location += 1;
				} break;
			case 2: // down(4)
				if ((directionInfo[c][r] & 4) == 4) {
					cout << (directionInfo[c][r] & 4) << "True" << endl;
					if (player_y - 80 > 50)
						player_y -= 80; light_y -= 80;
					player_location += 10;
				} break;
			case 3:	// left(8)
				if ((directionInfo[c][r] & 8) == 8) {
					cout << (directionInfo[c][r] & 8) << "True" << endl;
					if (player_x - 78 > 350)
						player_x -= 78; light_x -= 78;
					player_location -= 1;
				} break;
			default:
				break;
			}
			player->locate(scene3, player_x, player_y);
			candleLight->locate(scene3, light_x, light_y);
			if (player_location == 68) {
				timer->stop();
				hideTimer();
				clearStage(sceneARR[0]);
			}
			return true;
			});
	}
	directionButton[0]->locate(scene3, 1100, 400);
	directionButton[1]->locate(scene3, 1185, 315);
	directionButton[2]->locate(scene3, 1100, 270);
	directionButton[3]->locate(scene3, 1050, 315);












	/*=================================================== home ===============================================================*/

	auto home = Scene::create("Tangled Towe Escape", "images/home.jpg");
	currentScene = home;

	auto startButton = Object::create("images/startgame_button.jpg", home, 1080, 120);  startButton->setScale(0.15f);
	startButton->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction) -> bool {
		auto story = Object::create("images/story.jpg", home);
		story->setOnMouseCallback([&](ObjectPtr object, int, int, MouseAction) -> bool {
			object->hide();
			startStage(sceneARR[1]);  return true;
			});  return true;
		});

	auto endButton = Object::create("images/endgame_button.jpg", home, 1095, 80);  endButton->setScale(0.15f);
	endButton->setOnMouseCallback([](ObjectPtr, int, int, MouseAction) -> bool {
		endGame();	return true;
		});



	startGame(home);



	return 0;
}