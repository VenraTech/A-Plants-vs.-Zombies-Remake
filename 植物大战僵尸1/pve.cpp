//1.创建空项目模版
//2.导入素材（植物大战僵尸素材包）
//3.实现最开始的游戏场景
//4.实现游戏顶部的工具栏
//5.实现工具栏中植物卡牌

#define _CRT_SECURE_NO_WARNINGS
#include<chrono>
#include<set>
#include<thread>
#include<Windows.h>
#include<mmsystem.h>
#pragma comment(lib,"WINMM.LIB")
#include<string.h>
#include<time.h>
#include<math.h>
#include<stdio.h>
#include<graphics.h>//easyx图形库的头文件，需要安装easyx图形库
#include"tools.h"
#include"vector2.h"
#define width 900
#define height 600
#define kills 10

enum { wan_dou_she_shou, xiang_ri_kui, zhiwu_count };
enum{going,win,fail};

int zzkill;
int gamekill;
int gamestatus;

IMAGE imgbg;//加载背景图片
IMAGE imgtool;//工具栏
IMAGE imgplants[zhiwu_count];
IMAGE* imgzhiwu[zhiwu_count][20];

int cardX, cardY;//卡牌当前位置
int curzhiwu;//0没有选中，1选中第一种

struct zhiwu
{
	int type;      //0:没有植物，1：植物一......
	int frameindex;//植物当前帧数
	bool used = false;
	int blood; 
	int timer;
	int x, y;
	std::chrono::steady_clock::time_point lasttime;
	bool first;
};
struct zhiwu map[5][9];
enum{dowN,stoP,flY,creatE};


struct sunshine
{
	int x, y;//飘落过程中的xy坐标
	int frameindex;//当前图片帧数
	int destY;//飘落目标地
	bool used;
	int timer;
	float xoff;
	float yoff;
	float time;//0-1,贝塞尔曲线四个点.0起点1终点
	vector2 p1, p2, p3, p4;
	vector2 pcur;
	float speed;
	int status;
	bool light;
};

struct sunshine balls[10];
IMAGE imgsunshine[29];
int sunshine;

struct zongzi
{
	int x, y;
	int frameindex;
	bool used;
	int speed;
	int row;
	int rol;
	int blood;
	bool dead=true;
	int dps;
    bool eating;
};
struct zongzi zongzis[10];
IMAGE imgzz[22];
IMAGE imgdead[20];
IMAGE imgeat[11];
IMAGE imgstand[11];


struct bullet
{
	int x, y;
	int row;
	bool used;
	int speed;
	bool boom;//是否爆炸
	int frameindex;//帧序号
};
struct bullet bullets[64];
IMAGE imgbulletnormal;
IMAGE imgboom[4];

struct SunflowerTimestamp
{
	std::chrono::steady_clock::time_point lastSunshineTime;
};
//函数声明区
void collectsunshine(ExMessage* msg);
void drawzz();
void drawsunshine();

bool fileExist(const char* name)
{
	FILE* fp = fopen(name, "r");
	if (fp == NULL)
	{
		return false;
	}
	else
	{
		fclose(fp);
		return true;
	}
}



void gamestart()//加载背景1.直接整图片 2.放到内存变量
{

	loadimage(&imgbg, "res\\bg1.jpg");//一开始这样是失败的，要把字符集改为多字节字符集
	//写到这一步图片还不会显示，因为它在内存变量里面
	loadimage(&imgtool, "res\\bar5.png");//工具栏图片
	//创建游戏窗口

	zzkill = 0;
	gamekill = 0;
	gamestatus = going;
	memset(imgzhiwu, 0, sizeof(imgzhiwu));
	memset(map, 0, sizeof(map));

	char name[128];
	for (int i = 0; i < zhiwu_count; i++)
	{
		sprintf_s(name, sizeof(name), "res\\Cards\\card_%d.png", i + 1);//每张卡片路径名不一样，要自动生成文件名
		loadimage(&imgplants[i], name);
		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name, sizeof(name), "res\\zhiwu\\%d\\%d.png", i, j + 1);
			if (fileExist(name))//判断文件是否存在
			{
				imgzhiwu[i][j] = new IMAGE;
				loadimage(imgzhiwu[i][j], name);
			}
			else
			{
				break;
			}
		}

	}

	curzhiwu = 0;
	sunshine = 50;

	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++)
	{
		sprintf_s(name, sizeof(name), "res\\sunshine\\%d.png", i + 1);
		loadimage(&imgsunshine[i], name);
	}

	srand((unsigned int)time(NULL));//创建随机数

	initgraph(width, height,1);//这个函数用于初始化绘图环境。参数：width绘图环境的宽度。height绘图环境的高度。flag绘图环境的样式，默认为 NULL。可为以下值：返回值,创建的绘图窗口的句柄。

	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;//抗锯齿效果
	settextstyle(&f);//设置字体
	setbkmode(TRANSPARENT);//设置背景模式
	setcolor(BLACK);//设置字体颜色

	//初始化僵尸数据
	memset(zongzis, 0, sizeof(zongzis));
	for (int i = 0; i < 22; i++)
	{
		sprintf_s(name, sizeof(name),"res\\zm\\%d.png" , i+1);//"C:\植物大战僵尸\植物大战僵尸1\res\zm\1.png"name 植物名字
		loadimage(&imgzz[i], name);
	}


	loadimage(&imgbulletnormal, "res\\bullets\\bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	//初始化子弹爆炸帧图片
	for (int i = 0; i < 4; i++)
	{
		sprintf_s(name, sizeof(name), "res\\bullets\\PeaNormalExplode\\PeaNormalExplode_%d.png", i);
		loadimage(&imgboom[i], name);
	}
	for (int i = 0; i < 20; i++)//僵尸死亡
	{
		sprintf_s(name, sizeof(name), "res\\zm_dead\\%d.png", i+1);
		loadimage(&imgdead[i], name);
	}
	for (int i = 0; i < 11; i++)//僵尸吃
	{
		sprintf_s(name, sizeof(name), "res\\zm_eat\\%d.png", i + 1);//"C:\植物大战僵尸\植物大战僵尸1\res\zm_eat\1.png"
		loadimage(&imgeat[i], name);
	}
	for (int i = 0; i < 11; i++)
	{
		sprintf_s(name, sizeof(name), "res\\zm_stand\\%d.png", i+1);
		loadimage(&imgstand[i], name);
	}
}
void drawcard()
{
	for (int i = 0; i < zhiwu_count; i++)
	{
		int x = 338 + i * 64;
		putimage(x-112, 6, &imgplants[i]);
	}
}
void drawzhiwu()
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type > 0)
			{
				//int x = 255 + j * 80;
				//int y = 80 + i * 100 + 15;

				int zhiwutype = map[i][j].type - 1;
				int index = map[i][j].frameindex;
				putimagePNG(map[i][j].x, map[i][j].y, imgzhiwu[zhiwutype][index]);
			}
		}
	}

	if (curzhiwu > 0)//渲染拖动过程
	{
		IMAGE* img = imgzhiwu[curzhiwu - 1][0];
		putimagePNG(cardX - img->getwidth() / 2, cardY - img->getheight() / 2, img);
	}
}
void drawsuncount()
{
	char scoreText[20];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(282-112, 67, scoreText);//输出阳光数量
}
void drawbullet()
{
	int bulletcount = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletcount; i++)
	{
		if (bullets[i].used == false)
		{
			continue;
		}
		if (bullets[i].boom)
		{
			IMAGE* img = &imgboom[bullets[i].frameindex];
			putimagePNG(bullets[i].x, bullets[i].y, img);
		}
		else
		{
			putimagePNG(bullets[i].x, bullets[i].y, &imgbulletnormal);
		}
	}//渲染子弹
}
void updateWindows()
{
	BeginBatchDraw();//先双缓冲，即先打印到一块特殊内存，不直接打印到屏幕，再一次性打印出去。此处为开始缓冲。
	putimage(-112, 0, &imgbg);//三个参数：x距离最左端，y距离顶部的距离，图片地址
	putimagePNG(250-112, 0, &imgtool);
	drawcard();
	drawzhiwu();
	drawsunshine();
	drawsuncount();
	drawzz();//渲染僵尸
	drawbullet();
	EndBatchDraw();//结束双缓冲。
}
void userclick()
{
	ExMessage msg;
	static int status = 0;//添加状态变量
	if (peekmessage(&msg))
	{

		if (msg.message == WM_LBUTTONDOWN)
		{
			if (status == 1)
			{
				cardX = msg.x;
				cardY = msg.y;
				int row = (msg.y - 80) / 100;
				int col = (msg.x - 255+112) / 81;
				if (map[row][col].type == 0)
				{
					map[row][col].type = curzhiwu;
					map[row][col].frameindex = 0;
					map[row][col].blood = 5;
					map[row][col].x = 255-112 + col * 81;
					map[row][col].y = 80 + 100 * row+15;
					map[row][col].first = true;
					
				}
				status = 0;
				curzhiwu = 0;
			}
			else if (status == 0)
			{
				if (msg.x > 338-112 && msg.x < 338 + 65 * zhiwu_count-112 && msg.y < 96 && msg.y>6)//选定卡牌位置
				{
					int index = (msg.x - 338+112) / 65;
					status = 1;//此时为选定
					curzhiwu = index + 1;
					cardX = msg.x;
					cardY = msg.y;
				}
				else
				{
					collectsunshine(&msg);
				}
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)
		{
			cardX = msg.x;
			cardY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP && status == 1)
		{
			if (msg.x > 255-112&& msg.y > 80 && msg.y < 580 && msg.x < 900-112)
			{
				int row = (msg.y - 80) / 100;
				int col = (msg.x - 255+112) / 81;
				if (map[row][col].type == 0)
				{
					map[row][col].type = curzhiwu;
					map[row][col].frameindex = 0;
					map[row][col].blood = 5;
					map[row][col].x = 255-112+col*81;
					map[row][col].y = 80+100*row+15;
					map[row][col].first = true;
				}
				curzhiwu = 0;
				status = 0;
			}
		}
	}
}
void createsunshine()
{
	static int count = 0;
	count++;
	static int zhenshu = 20;
	int ballcount = sizeof(balls) / sizeof(balls[0]);
	if (count > zhenshu)
	{
		zhenshu = 200 + rand() % 200;
		count = 0;
		int i = 0;
		for (i = 0; i < ballcount && balls[i].used; i++);
		if (i >= ballcount)
			return;

		balls[i].used = true;
		balls[i].frameindex = 0;
		balls[i].status = dowN;
		balls[i].time = 0;
		balls[i].timer = 0;
		balls[i].p1 = vector2(260 + rand() % (900 - 260), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 180 + 98 * rand() % 4);
		float off = 2;
		float distance = dis(balls[i].p1 - balls[i].p4);
		balls[i].speed = 1.0 / (distance / off);
	}
			for (int v = 0; v < 5; v++)
			{
				for (int j = 0; j < 9; j++)
				{
					if (map[v][j].type == xiang_ri_kui + 1)
					{
						std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
						std::chrono::milliseconds timeSinceLastSun = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - map[v][j].lasttime);
						if (map[v][j].first==true)
						{
							map[v][j].lasttime = currentTime;
							map[v][j].first = false;
							continue;
						}
						if (timeSinceLastSun.count() >= 20000)
						{
							int k = 0;
							for (k = 0; k < ballcount && balls[k].used; k++);
							if (k >= ballcount)
							{
								return;
							}
							balls[k].used = true;
							balls[k].p1 = vector2(map[v][j].x, map[v][j].y);

							int kuan = rand() % 50 * (rand() % 2 ? 1 : -1);
							balls[k].p4 = vector2(map[v][j].x + kuan, map[v][j].y + imgzhiwu[xiang_ri_kui][0]->getheight() - imgsunshine[0].getheight());
							balls[k].p2 = vector2(balls[k].p1.x + kuan * 0.3, balls[k].p1.y - 100);
							balls[k].p3 = vector2(balls[k].p1.x + kuan * 0.7, balls[k].p1.y - 100);
							balls[k].status = creatE;
							balls[k].speed = 0.05;
							balls[k].time = 0;
							map[v][j].lasttime = currentTime;
						}
					}
				}
			}
}
void drawsunshine()
{
	int ballcount = sizeof(balls) / sizeof(balls[0]);//阳光渲染
	for (int i = 0; i < ballcount; i++)
	{
		//if (balls[i].used || balls[i].xoff)
		if(balls[i].used)
		{
			IMAGE* img = &imgsunshine[balls[i].frameindex];
			//putimagePNG(balls[i].x, balls[i].y, img);
			putimagePNG(balls[i].pcur.x, balls[i].pcur.y, img);
		}
	}
}
void updatesunshine()
{
	int ballcount = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballcount; i++)
	{
		if (balls[i].used)
		{
			balls[i].frameindex = (balls[i].frameindex + 1) % 29;
			if (balls[i].status == dowN)
			{
				struct sunshine* sun = &balls[i];
				sun->time += sun->speed;
				sun->pcur = sun->p1 + sun->time * (sun->p4 - sun->p1);
				if (sun->time >= 1)
				{
					sun->status = stoP;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == flY)
			{
				struct sunshine* sun = &balls[i];
				sun->time += sun->speed;
				sun->pcur = sun->p1 + sun->time * (sun->p4 - sun->p1);
				if (sun->time > 1)
				{
					sun->used = false;
					sunshine += 25;
				}
			}
			else if (balls[i].status == stoP)
			{
				balls[i].timer++;
				if (balls[i].timer > 200)
				{
					balls[i].timer = 0;
					balls[i].used = false;
				}
			}
			else if (balls[i].status == creatE)
			{
				struct sunshine* sun = &balls[i];
				sun->time += sun->speed;
				sun->pcur = calcBezierPoint(sun->time, sun->p1, sun->p2, sun->p3, sun->p4);
				if (balls[i].time > 1)
				{
					sun->status = stoP;
					sun->timer = 0;
				}
			}
			/*if (balls[i].timer == 0)
			{
				balls[i].y += 2;
			}
			if (balls[i].y >= balls[i].destY)
			{
				balls[i].timer++;
				if (balls[i].timer > 100)
				{
					balls[i].used = false;
				}
			}*/
		}
		/*else if (balls[i].xoff)
		{
			balls[i].x -= balls[i].xoff;
			balls[i].y -= balls[i].yoff;
			if (balls[i].x < 262 || balls[i].y < 0)
			{
				balls[i].xoff = 0;
				balls[i].yoff = 0;
				sunshine += 25;
			}
		}*/
	}
}
void sunshineplay()
{
	// 音频播放操作
	mciSendString("play res\\sunshine.mp3 ", NULL, 0, NULL);
}
void collectsunshine(ExMessage* msg)
{
	int ballcount = sizeof(balls) / sizeof(balls[0]);
	int w = imgsunshine[0].getwidth();
	int h = imgsunshine[0].getheight();
	for (int i = 0; i < ballcount; i++)
	{
		if (balls[i].used)
		{
			//int x = balls[i].x;
			//int y = balls[i].y;
			int x = balls[i].pcur.x;
			int y = balls[i].pcur.y;
			if (msg->x > x && msg->x<x + w &&
				msg->y>y && msg->y < y + h)
			{
				//balls[i].used = false;
				balls[i].status = flY;
				//mciSendString("play res\\sunshine.mp3 ", NULL, 0, NULL);
				std::thread audioThread(sunshineplay);
				audioThread.detach();
				//float destx = 262;
				//float desty = 0;
				//float angle = atan((balls[i].y - desty) / (balls[i].x - destx));//反正切求角度，不懂用正切啥的也行
				//balls[i].xoff = 40 * cos(angle);
				//balls[i].yoff = 40 * sin(angle);
				balls[i].p1 = balls[i].pcur;
				balls[i].p4 = vector2(262-112, 0);
				balls[i].time = 0;
				float distance = dis(balls[i].p1- balls[i].p4);
				float off = 16;
				balls[i].speed = 1.0 / (distance / off);
			}

		}
	}
}
void createzz()
{
	if (gamekill > kills)
	{
		return;
	}
	static int  zmfre = 20;
	static int count = 0;
	count++;
	if (count > zmfre)
	{
		count = 0;
		zmfre = rand() % 20 + 30;
		int i;
		int zzs = sizeof(zongzis) / sizeof(zongzis[0]);
		for (i = 0; i < zzs && zongzis[i].used; i++);
		if (i < zzs)
		{
			zongzis[i].dead = false;
			zongzis[i].used = true;
			zongzis[i].x = width;
			zongzis[i].row = rand() % 5;
			zongzis[i].y = 173 + zongzis[i].row * 100;
			zongzis[i].speed = 1;
			zongzis[i].blood = 10;
			zongzis[i].eating = false;
			zongzis[i].dps = 1;
			gamekill++;
		}
	}
	
}
void updatezz()
{
	//更新僵尸位置
	int zzs = sizeof(zongzis) / sizeof(zongzis[0]);
	static int count = 0;
	count++;
	if (count >= 4)
	{
		count = 0;
		for (int i = 0; i < zzs; i++)
		{
			if (zongzis[i].used)
			{
				zongzis[i].x = zongzis[i].x - zongzis[i].speed;
				if (zongzis[i].x < 150-112)
				{
					gamestatus = fail;
				}
			}
		}
	}
	static int count2 = 0;
	count2++;
	if (count2 >= 2)
	{
		count2 = 0;
		for (int i = 0; i < zzs; i++)
		{
			if (zongzis[i].used)
			{
				if (zongzis[i].dead)
				{
					zongzis[i].frameindex = zongzis[i].frameindex + 1;
					if (zongzis[i].frameindex >= 20)
					{
						zongzis[i].used = false;
						zzkill++;
						if (zzkill > kills)
						{
							gamestatus = win;
						}
					}
				}
				else if (zongzis[i].eating)
				{
					zongzis[i].frameindex = zongzis[i].frameindex + 1;
					if (zongzis[i].frameindex > 10)
					{
						map[zongzis[i].row][zongzis[i].rol].blood -= zongzis[i].dps;
						zongzis[i].frameindex = 0;
					}
				}
				else
				{
					zongzis[i].frameindex = (zongzis[i].frameindex + 1) % 22;
				}
			}
		}
	}
}
void drawzz()
{
	int zzcount = sizeof(zongzis) / sizeof(zongzis[0]);
	for (int i = 0; i < zzcount; i++)
	{
		if (zongzis[i].used)
		{
			//IMAGE* img = &imgzz[zongzis[i].frameindex];
			//IMAGE* img = (zongzis[i].dead) ? imgdead : imgzz;
			IMAGE* img = NULL;
			if (zongzis[i].dead)
			{
				img = imgdead;
			}
			else if (zongzis[i].eating)
			{
				img = imgeat;
			}
			else
			{
				img = imgzz;
			}
			img += zongzis[i].frameindex;
			putimagePNG(zongzis[i].x, zongzis[i].y - img->getheight(), img);
		}
	}
}
std::chrono::steady_clock::time_point lastShotTime = std::chrono::steady_clock::now();
void shoot()
{
	int zzcount = sizeof(zongzis) / sizeof(zongzis[0]);
	int bulletcount = sizeof(bullets) / sizeof(bullets[0]);

	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	std::chrono::milliseconds timeSinceLastShot = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastShotTime);

	if (timeSinceLastShot.count() >= 2000)
	{
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				if (map[i][j].type == wan_dou_she_shou + 1 && map[i][j].used)
				{

					int k = 0;
					for (k = 0; k < bulletcount && bullets[k].used; k++);
					if (k < bulletcount)
					{
						bullets[k].row = i;
						bullets[k].used = true;
						bullets[k].speed = 30;
						int zhiwuX = 255-112 + j * 80;
						int zhiwuY = 80 + i * 100 + 15;
						bullets[k].x = zhiwuX + imgzhiwu[map[i][j].type - 1][0]->getwidth() - 20;
						bullets[k].y = zhiwuY + 10;
						bullets[k].boom = false;
						bullets[k].frameindex = 0;
					}
				}
			}
		}
		lastShotTime = currentTime;
	}

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type != wan_dou_she_shou + 1)
			{
				continue;
			}
			bool hasLiveZombie = false; // 添加一个标志来记录是否存在活着的僵尸
			for (int v = 0; v < zzcount; v++)
			{
				if (zongzis[v].row == i && zongzis[v].used && !zongzis[v].dead)
				{
					int danger = (zongzis[v].x - 255 - j * 80+112) / 80; // 预警范围
					if (danger < 6 && danger > 0)
					{
						hasLiveZombie = true; // 存在活着的僵尸
						break;
					}
				}
			}
			map[i][j].used = hasLiveZombie; // 根据是否存在活着的僵尸来更新豌豆射手的射击状态
		}
	}
}
void updatebullet()
{
	int bulletcount = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletcount; i++)
	{
		if (bullets[i].used)
		{
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > 1500)
			{
				bullets[i].used = false;
			}

			//子弹爆炸
			if (bullets[i].boom)
			{
				bullets[i].frameindex++;
				if (bullets[i].frameindex >= 5)
				{
					bullets[i].boom = false;
					bullets[i].used = false;
				}
			}
		}
	}

}
void bulletcheck()
{
	int zzcount = sizeof(zongzis) / sizeof(zongzis[0]);
	int bulletcount = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletcount; i++)
	{
		if (bullets[i].used == false || bullets[i].boom)
		{
			continue;
		}
		for (int j = 0; j < zzcount; j++)
		{
			if (zongzis[j].used == false)
			{
				continue;
			}
			int x1 = zongzis[j].x + 80;
			int x2 = zongzis[j].y + 100;
			if (zongzis[j].dead == false && bullets[i].x > x1 && bullets[i].y < x2 && zongzis[j].row == bullets[i].row)
			{
				zongzis[j].blood -= 1;
				bullets[i].boom = true;
				bullets[i].speed = 0;
				if (zongzis[j].blood <= 0)
				{
					zongzis[j].dead = true;
					zongzis[j].speed = 0;
					zongzis[j].frameindex = 0;
				}
			}
		}
	}
}
void eatcheck()
{
	int zzcount = sizeof(zongzis) / sizeof(zongzis[0]);
	for (int i = 0; i < zzcount; i++)
	{
		if (zongzis[i].used == false)
		{
			continue;
		}
		
		for (int v = 0; v < 9; v++)
		{
			if (map[zongzis[i].row][v].type==0)
			{
				continue;
			}
			int zhuwux1 = 255-112 + v * 80 + 10;
			int zhiwux2 = 255-112 + v * 80+70;
			int x3 = zongzis[i].x + 80;
			if (x3 > zhuwux1 && x3 < zhiwux2)
			{
				zongzis[i].eating = true;
				zongzis[i].speed = 0;
				zongzis[i].rol = v;
				if (map[zongzis[i].row][v].blood <= 0)
				{
					map[zongzis[i].row][v].type = 0;
					for (int j = 0; j < zzcount; j++)
					{
						if(x3 > zhuwux1 && x3 < zhiwux2&& zongzis[j].rol == v)
						{ 
						zongzis[j].eating = false;
						zongzis[j].speed = 1;

						zongzis[j].rol = 0;
						}
					}
				}
			}
		}
		
	}
}
void checkboom()
{
	bulletcheck();//子弹对僵尸的碰撞检测
	eatcheck();//僵尸对植物的吃检测
}
void updatecontrol()
{
	static int count;
	count++;
	if (count < 2)
	{
		return;
	}
	count = 0;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type > 0)
			{
				map[i][j].frameindex++;
				int zhiwutype = map[i][j].type - 1;
				int index = map[i][j].frameindex;

				if (imgzhiwu[zhiwutype][index] == NULL)
				{
					map[i][j].frameindex = 0;
				}
			}
		}
	}
}
void updategame()
{
	updatecontrol();
	createsunshine();
	updatesunshine();

	createzz();
	updatezz();

	shoot();//发射子弹
	updatebullet();//更新位置
	checkboom();
}
void startUI()//游戏前界面
{
	IMAGE imgmenu, imgmenu1, imgmenu2;
	loadimage(&imgmenu, "res\\menu.png");
	loadimage(&imgmenu1, "res\\menu1.png");//"C:\Users\31703\OneDrive\c语言\植物大战僵尸\res\menu1.png"
	loadimage(&imgmenu2, "res\\menu2.png");

	int flag = 0;
	while (1)
	{
		BeginBatchDraw();
		putimage(0, 0, &imgmenu);
		putimagePNG(470, 75, flag ? &imgmenu2 : &imgmenu1);

		ExMessage msg;
		if (peekmessage(&msg))
		{
			if (msg.message == WM_LBUTTONDOWN && msg.x > 470 && msg.x < 770 && msg.y>75 && msg.y < 215 && flag == 0)
			{
				flag = 1;
			}
			else if (msg.message == WM_LBUTTONUP && flag == 1)
			{
				EndBatchDraw();
				break;
			}
		}
		EndBatchDraw();




	}

}
void startall()
{
	vector2 point[9] = { {550,80},{530,160} ,{630,170} ,{530,200} ,{515,270 } ,{565,370}, {605,340}, {705,280}, {690,340} };
	int xmin = width - imgbg.getwidth();
	int index[9];
	for (int i = 0; i < 9; i++)
	{
		index[i] = rand() % 11;
	}
	int count = 0;
		for (int x = 0; x >= xmin; x -= 2)
		{
			BeginBatchDraw();
			putimage(x, 0, &imgbg);
			count++;
			for (int k = 0; k < 9; k++)
			{
				putimagePNG(point[k].x - xmin + x, point[k].y, &imgstand[index[k]]);
				if (count >= 10)
				{
					index[k] = (index[k] + 1) % 11;
				}
			}
			if (count > 10)
			{
				count = 0;
			}
			Sleep(6);
			EndBatchDraw();
		}
		for (int i = 0; i < 100; i++)
		{
			BeginBatchDraw();
			putimage(xmin, 0, &imgbg);
			for (int k = 0; k < 9; k++)
			{
				putimagePNG(point[k].x, point[k].y, &imgstand[index[k]]);
				index[k] = (index[k] + 1) % 11;
			}
			EndBatchDraw();
			Sleep(5);
	     }
		for (int x = xmin; x <= -112; x += 2)
		{
			BeginBatchDraw();
			putimage(x, 0, &imgbg);
			count++;
			for (int k = 0; k < 9; k++)
			{
				putimagePNG(point[k].x - xmin + x, point[k].y, &imgstand[index[k]]);
				if (count >= 10)
				{
					index[k] = (index[k] + 1) % 11;
				}
			}
			if (count > 10)
			{
				count = 0;
			}
			Sleep(6);
			EndBatchDraw();
		}
}
void tooldown()
{
	int heighT = imgtool.getheight();
	for (int y = -height; y < 0; y++)
	{
		BeginBatchDraw();

		putimage(-112, 0, &imgbg);
		putimage(250 - 112, y, &imgtool);
		for (int i = 0; i < zhiwu_count; i++)
		{
			int x = 338 - 112 + i * 65;
			int z = 6+y;
			putimage(x, y, &imgplants[i]);
		}
		
		EndBatchDraw();
		Sleep(5);
	}
}
bool checkover()
{
	int ret = false;
	IMAGE imgwin, imgfail;
	if (gamestatus == win)
	{
		loadimage(&imgwin, "res\\gameWin.png");
		putimage(0, 0, &imgwin);
		Sleep(3000);
		ret = true;
	}
	else if (gamestatus == fail)
	{
		loadimage(&imgfail, "res\\gameFail.png");
		putimage(0, 0, &imgfail);
		Sleep(3000);
		ret = true;
	}
	return ret;
}
int main()
{

	gamestart();//游戏初始化
	startUI();//启动菜单
	startall();//巡场片头
	tooldown();
	int timer = 0;
	bool flag = true;
	while (1)
	{
		userclick();

		timer += getDelay();
		if (timer > 20)
		{
			flag = true;
			timer = 0;
		}
		if (flag)
		{
			flag = false;
			updateWindows();//桌面函数
			updategame();
			if (checkover()) {
				break;
			}

		}



	}


	return 0;
}