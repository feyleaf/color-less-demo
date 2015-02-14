#include <SFML/Graphics.hpp>
#include "ic_math.h"

struct particle
{
	sf::Vector2f pos;
	sf::Color col;
	sf::Vector2f vel;
	float mass;
	particle(sf::Vector2f ctr) {
		pos = ctr;
		vel = sf::Vector2f(0, 0);
		col = sf::Color(0, 0, 0, 0);
		mass = 60000.0f;

	}
	particle(sf::Vector2f ctr, sf::Image& img) {
		do
		{
			pos = sf::Vector2f(sf::Vector2i(rand() % 128, rand() % 128));
			col = img.getPixel(unsigned int(pos.x), unsigned int(pos.y));
		}
		while (col.a<10 || col==sf::Color::Black);

		pos += sf::Vector2f(-64.0f,-64.0f)+ctr;
		vel = sf::Vector2f(0.0025f*(rand() % 400 - 200), -0.005f*(rand() % 300 + 100));
		mass = 0.00125f;
	}
};

void gravity(std::vector<particle>& list, particle& ob)
{
	for (int i = 0; i < int(list.size()); i++)
	{
		sf::Vector2f littlePoint = list[i].pos;
		sf::Vector2f bigPoint = ob.pos;
		float dist = calcDist(littlePoint, bigPoint);
		//newton's law of force of universal gravity, F = mass1*mass2/dist^2
		//this will be the force accelleration on the particle dust, change in velocity
		float force = (list[i].mass*ob.mass) / (dist*dist);
		sf::Vector2f newDirection = ob.pos - list[i].pos;
		sf::Vector2f accel = scalar(force, newDirection);
		if (dist>33.0f)
			list[i].vel = list[i].vel + accel;
		else
		{
			list[i].vel = scalar(0.65f, list[i].vel);
		}
	}
}

sf::Image desaturate(sf::Image& src)
{
	sf::Image ret = src;
	for (int y = 0; y < int(ret.getSize().y); y++)
	{
		for (int x = 0; x < int(ret.getSize().x); x++)
		{
			//saturation is the measure of the highest RGB value.
			//we will change all the values to match this highest one
			sf::Color rgb = ret.getPixel(x, y);
			if (rgb.a > 0)
			{
				int high = max3(int(rgb.r), int(rgb.g), int(rgb.b));
				ret.setPixel(x, y, sf::Color(high, high, high, rgb.a));
			}
		}
	}
	return ret;
}

void diffuse(std::vector<particle>& list)
{
	for (int i = 0; i < int(list.size()); i++)
	{
		if(list[i].col.a>1) list[i].col.a--;
		list[i].pos = list[i].pos + list[i].vel;
	}
}

void clip(std::vector<particle>& list)
{
	for (int i = list.size() - 1; i > 0; i--)
	{
		if (list[i].col.a<10 || (i>1000))
			list.erase(list.begin() + i);
	}
}

int main()
{
	sf::RenderWindow app(sf::VideoMode(1024, 576), "Color~Less Demo");
	app.setView(sf::View(sf::Vector2f(512, 288), sf::Vector2f(1600, 900)));
	app.setPosition(sf::Vector2i(80, 15));
	app.setFramerateLimit(60);
	//sf::Mouse themouse;
	sf::Vector2f butterPos;
	sf::Vector2f butterVel;
	sf::Vector2f centerPos;
	sf::Image butterflyImg;
	butterflyImg.loadFromFile("butterfly.png");
	sf::Image deadImg = desaturate(butterflyImg);
	sf::Texture butterflyTex;
	sf::Texture deadTex;
	butterflyTex.loadFromImage(butterflyImg);
	deadTex.loadFromImage(deadImg);
	sf::Sprite butterfly;
	butterfly.setTexture(butterflyTex);
	sf::Sprite death;
	death.setTexture(deadTex);
	std::vector<particle> particleList;
	particleList.clear();

	std::vector<particle> obeliskList;
	obeliskList.push_back(particle(sf::Vector2f(400, 250)));
	obeliskList.push_back(particle(sf::Vector2f(-200, 600)));
	obeliskList.push_back(particle(sf::Vector2f(800, 25)));
	obeliskList.push_back(particle(sf::Vector2f(1200, 400)));
	int oop = 0;
	sf::CircleShape start = sf::CircleShape(25.0f);
	start.setOrigin(sf::Vector2f(25.0f, 25.0f));
	start.setFillColor(sf::Color(96, 96, 16, 255));
	start.setOutlineThickness(2.0f);
	start.setOutlineColor(sf::Color::Yellow);
	start.setPosition(sf::Vector2f(64, 64));
	sf::CircleShape goal = sf::CircleShape(25.0f);
	goal.setOrigin(sf::Vector2f(25.0f, 25.0f));
	goal.setFillColor(sf::Color(16, 96, 16, 255));
	goal.setOutlineThickness(2.0f);
	goal.setOutlineColor(sf::Color::Green);
	goal.setPosition(sf::Vector2f(1280, -100));

	while (app.isOpen())
	{
		sf::Event event;
		while (app.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				app.close();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) butterVel.y -= 3.0f;
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) butterVel.y += 3.0f;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) butterVel.x -= 3.0f;
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) butterVel.x += 3.0f;
		butterVel.x /= 2.0f;
		butterVel.y /= 2.0f;
		butterPos += butterVel;
		centerPos = sf::Vector2f(butterPos) + sf::Vector2f(64, 64);
		for (int o = 0; o < int(obeliskList.size()); o++)
		{
			if (calcDist(centerPos, obeliskList[o].pos) < 360.0f)
			{
				for (int i = 0; i < 4; i++)
				{
					if (oop < 255)
						particleList.push_back(particle(centerPos, butterflyImg));
					else
						particleList.push_back(particle(centerPos, deadImg));
				}
				if (oop < 255) { if (rand() % 2 == 0) oop++; }
			}
			gravity(particleList, obeliskList[o]);
		}
		if (calcDist(centerPos, goal.getPosition()) < 64.0f)
		{
			oop = 0;
		}
		if (calcDist(centerPos, start.getPosition()) < 64.0f)
		{
			oop = 0;
		}
		diffuse(particleList);
		butterfly.setPosition(butterPos);
		death.setPosition(butterPos);
		death.setColor(sf::Color(255, 255, 255, oop));
		app.clear();
		for (int o = 0; o < int(obeliskList.size()); o++)
		{
			sf::CircleShape point = sf::CircleShape(25.0f);
			point.setOrigin(sf::Vector2f(25.0f, 25.0f));
			point.setFillColor(sf::Color(16, 16, 16, 255));
			point.setOutlineThickness(2.0f);
			point.setOutlineColor(sf::Color(96, 96, 96, 255));
			point.setPosition(sf::Vector2f(obeliskList[o].pos));
			app.draw(point);
		}

		app.draw(start);
		app.draw(goal);

		app.draw(butterfly);
		app.draw(death);

		for (int i = 0; i < int(particleList.size()); i++)
		{
			sf::CircleShape point = sf::CircleShape(1.0f);
			point.setFillColor(particleList[i].col);
			point.setPosition(sf::Vector2f(particleList[i].pos));
			app.draw(point);
		}
		clip(particleList);
		app.display();
	}

	return int(particleList.size());
}