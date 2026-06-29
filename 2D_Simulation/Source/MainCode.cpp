#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <algorithm> // For std::max and std::min

using namespace std;

const float DEG2RAD = 3.14159f / 180.0f;

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE, PADDLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
	float red, green, blue;
	float x, y;
	float width;   // total width
	float height;  // total height
	BRICKTYPE brick_type;
	ONOFF onoff;
	int hitsLeft;  // Durability count (3 for DESTRUCTABLE, 1 for others)

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float hh, float rr, float gg, float bb)
	{
		brick_type = bt;
		x = xx;
		y = yy;
		width = ww;
		height = hh;
		red = rr;
		green = gg;
		blue = bb;
		onoff = ON;
		hitsLeft = (bt == DESTRUCTABLE) ? 3 : 1;
	}

	void drawBrick()
	{
		if (onoff == ON)
		{
			float halfW = width / 2.0f;
			float halfH = height / 2.0f;

			// Determine color based on hits left if destructible
			float r = red;
			float g = green;
			float b = blue;

			if (brick_type == DESTRUCTABLE)
			{
				if (hitsLeft == 3)
				{
					// Green (full health)
					r = 0.0f; g = 0.8f; b = 0.0f;
				}
				else if (hitsLeft == 2)
				{
					// Yellow (damaged)
					r = 0.8f; g = 0.8f; b = 0.0f;
				}
				else if (hitsLeft == 1)
				{
					// Red (near destruction)
					r = 0.8f; g = 0.0f; b = 0.0f;
				}
			}

			// Draw solid brick body
			glColor3f(r, g, b);
			glBegin(GL_POLYGON);
			glVertex2d(x + halfW, y + halfH);
			glVertex2d(x + halfW, y - halfH);
			glVertex2d(x - halfW, y - halfH);
			glVertex2d(x - halfW, y + halfH);
			glEnd();

			// Draw black border around the brick to make them visually distinct
			glColor3f(0.0f, 0.0f, 0.0f);
			glLineWidth(2.0f);
			glBegin(GL_LINE_LOOP);
			glVertex2d(x + halfW, y + halfH);
			glVertex2d(x + halfW, y - halfH);
			glVertex2d(x - halfW, y - halfH);
			glVertex2d(x - halfW, y + halfH);
			glEnd();

			// Draw cracks for damaged destructible bricks
			if (brick_type == DESTRUCTABLE && hitsLeft < 3)
			{
				glColor3f(0.0f, 0.0f, 0.0f); // black crack lines
				glLineWidth(2.0f);
				glBegin(GL_LINES);
				if (hitsLeft <= 2)
				{
					// First crack line (diagonal from bottom-left to top-right)
					glVertex2d(x - halfW + 0.02f, y - halfH + 0.02f);
					glVertex2d(x + halfW - 0.02f, y + halfH - 0.02f);
				}
				if (hitsLeft == 1)
				{
					// Second crack line (diagonal from top-left to bottom-right)
					glVertex2d(x - halfW + 0.02f, y + halfH - 0.02f);
					glVertex2d(x + halfW - 0.02f, y - halfH + 0.02f);
				}
				glEnd();
			}
		}
	}
};

class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float vx; // velocity x
	float vy; // velocity y

	Circle(double xx, double yy, double rr, float vx_init, float vy_init, float r, float g, float b)
	{
		x = (float)xx;
		y = (float)yy;
		radius = (float)rr;
		red = r;
		green = g;
		blue = b;
		vx = vx_init;
		vy = vy_init;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->onoff == OFF) return;

		float halfW = brk->width / 2.0f;
		float halfH = brk->height / 2.0f;

		// Find the closest point on the brick to the circle's center
		float closestX = x;
		float leftBound = brk->x - halfW;
		float rightBound = brk->x + halfW;
		if (closestX < leftBound) closestX = leftBound;
		else if (closestX > rightBound) closestX = rightBound;

		float closestY = y;
		float bottomBound = brk->y - halfH;
		float topBound = brk->y + halfH;
		if (closestY < bottomBound) closestY = bottomBound;
		else if (closestY > topBound) closestY = topBound;

		// Distance between circle's center and closest point
		float distX = x - closestX;
		float distY = y - closestY;
		float distSq = distX * distX + distY * distY;

		if (distSq < radius * radius)
		{
			float dist = sqrt(distSq);
			float normX = 0;
			float normY = 0;

			if (dist > 0.0001f)
			{
				normX = distX / dist;
				normY = distY / dist;

				// Resolve overlap: push circle away from brick
				float overlap = radius - dist;
				x += normX * overlap;
				y += normY * overlap;
			}
			else
			{
				// Center of circle is inside brick, push out along closest side
				if (abs(x - brk->x) / halfW > abs(y - brk->y) / halfH)
				{
					normX = (x > brk->x) ? 1.0f : -1.0f;
					x += normX * radius;
				}
				else
				{
					normY = (y > brk->y) ? 1.0f : -1.0f;
					y += normY * radius;
				}
			}

			// Reflect velocity across normal: V' = V - 2 * (V . N) * N
			float dot = vx * normX + vy * normY;
			if (dot < 0) // Only reflect if moving towards the surface
			{
				if (brk->brick_type == PADDLE)
				{
					// Paddle bounce: adjust angle depending on where it hits the paddle
					float hitFactor = (x - brk->x) / halfW; // range [-1, 1]
					float speed = sqrt(vx * vx + vy * vy);

					// Restrict hitFactor to safe ranges
					if (hitFactor < -1.0f) hitFactor = -1.0f;
					if (hitFactor > 1.0f) hitFactor = 1.0f;

					vx = hitFactor * speed * 0.85f;
					// Ensure vy is directed upwards since paddle is at the bottom (positive y)
					vy = sqrt(abs(speed * speed - vx * vx));
				}
				else
				{
					vx = vx - 2.0f * dot * normX;
					vy = vy - 2.0f * dot * normY;
				}

				// Change color of circle upon hitting brick to give a nice feedback effect
				red = (float)rand() / RAND_MAX;
				green = (float)rand() / RAND_MAX;
				blue = (float)rand() / RAND_MAX;
			}

			// Apply brick damage
			if (brk->brick_type == DESTRUCTABLE)
			{
				brk->hitsLeft--;
				if (brk->hitsLeft <= 0)
				{
					brk->onoff = OFF;
				}
			}
		}
	}

	void MoveOneStep()
	{
		// Update position
		x += vx;
		y += vy;

		// Border collision - Left wall
		if (x - radius < -1.0f && vx < 0)
		{
			x = -1.0f + radius;
			vx = -vx;
		}
		// Border collision - Right wall
		if (x + radius > 1.0f && vx > 0)
		{
			x = 1.0f - radius;
			vx = -vx;
		}
		// Border collision - Top wall
		if (y + radius > 1.0f && vy > 0)
		{
			y = 1.0f - radius;
			vy = -vy;
		}
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i += 5) // Step by 5 to render faster and keep code concise
		{
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();

		// Outline for the circle
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < 360; i += 10)
		{
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};

vector<Circle> world;

void ResolveCircleCollisions(vector<Circle>& circles)
{
	for (size_t i = 0; i < circles.size(); i++)
	{
		for (size_t j = i + 1; j < circles.size(); j++)
		{
			Circle& c1 = circles[i];
			Circle& c2 = circles[j];

			float dx = c2.x - c1.x;
			float dy = c2.y - c1.y;
			float distSq = dx * dx + dy * dy;
			float minDist = c1.radius + c2.radius;

			if (distSq < minDist * minDist)
			{
				float dist = sqrt(distSq);
				if (dist < 0.0001f) continue; // prevent division by zero

				float nx = dx / dist;
				float ny = dy / dist;

				// Resolve overlap: push circles apart equally
				float overlap = minDist - dist;
				c1.x -= nx * overlap * 0.5f;
				c1.y -= ny * overlap * 0.5f;
				c2.x += nx * overlap * 0.5f;
				c2.y += ny * overlap * 0.5f;

				// Elastic collision bounce (equal mass velocity swap along normal)
				float rvx = c2.vx - c1.vx;
				float rvy = c2.vy - c1.vy;
				float velAlongNormal = rvx * nx + rvy * ny;

				if (velAlongNormal < 0) // only bounce if moving towards each other
				{
					c1.vx += velAlongNormal * nx;
					c1.vy += velAlongNormal * ny;
					c2.vx -= velAlongNormal * nx;
					c2.vy -= velAlongNormal * ny;
				}

				// Change colors to show collision feedback
				c1.red = (float)rand() / RAND_MAX;
				c1.green = (float)rand() / RAND_MAX;
				c1.blue = (float)rand() / RAND_MAX;

				c2.red = (float)rand() / RAND_MAX;
				c2.green = (float)rand() / RAND_MAX;
				c2.blue = (float)rand() / RAND_MAX;
			}
		}
	}
}

void processInput(GLFWwindow* window, Brick& paddle)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Move paddle left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		paddle.x -= 0.009f;
		if (paddle.x - paddle.width / 2.0f < -1.0f)
			paddle.x = -1.0f + paddle.width / 2.0f;
	}

	// Move paddle right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		paddle.x += 0.009f;
		if (paddle.x + paddle.width / 2.0f > 1.0f)
			paddle.x = 1.0f - paddle.width / 2.0f;
	}

	// Spawn new circle on spacebar press (debounced)
	static bool spacePressedLastFrame = false;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		if (!spacePressedLastFrame)
		{
			// Spawn circle at center moving upwards at a random angle (45 to 135 degrees)
			float angle = (45.0f + (rand() % 90)) * DEG2RAD;
			float speed = 0.008f;
			float vx_val = cos(angle) * speed;
			float vy_val = sin(angle) * speed;

			float r = (float)rand() / RAND_MAX;
			float g = (float)rand() / RAND_MAX;
			float b = (float)rand() / RAND_MAX;

			Circle newCircle(0.0, -0.5, 0.04, vx_val, vy_val, r, g, b);
			world.push_back(newCircle);

			spacePressedLastFrame = true;
		}
	}
	else
	{
		spacePressedLastFrame = false;
	}
}

int main(void) {
	srand((unsigned int)time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment: 2D Physics Animation", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Setup Brick Layout
	vector<Brick> bricks;

	// Add destructible brick rows (2 rows of 5 bricks)
	float startX = -0.8f;
	float spacingX = 0.4f;
	float brickW = 0.3f;
	float brickH = 0.1f;

	for (int i = 0; i < 5; i++)
	{
		// Row 1 at y = 0.7
		bricks.push_back(Brick(DESTRUCTABLE, startX + i * spacingX, 0.7f, brickW, brickH, 0.0f, 0.8f, 0.0f));
		// Row 2 at y = 0.5
		bricks.push_back(Brick(DESTRUCTABLE, startX + i * spacingX, 0.5f, brickW, brickH, 0.0f, 0.8f, 0.0f));
	}

	// Add side bumpers (REFLECTIVE)
	bricks.push_back(Brick(REFLECTIVE, -0.9f, 0.0f, 0.15f, 0.4f, 0.6f, 0.6f, 0.9f));
	bricks.push_back(Brick(REFLECTIVE, 0.9f, 0.0f, 0.15f, 0.4f, 0.6f, 0.6f, 0.9f));

	// Add player paddle (PADDLE) at the bottom
	size_t paddleIndex = bricks.size();
	bricks.push_back(Brick(PADDLE, 0.0f, -0.85f, 0.5f, 0.06f, 0.7f, 0.7f, 0.7f));

	while (!glfwWindowShouldClose(window)) {
		// Setup View
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window, bricks[paddleIndex]);

		// Resolve circle-circle collisions
		ResolveCircleCollisions(world);

		// Movement, collision, and cleanup for all circles (missed ball = deleted)
		for (size_t i = 0; i < world.size(); )
		{
			// Check collision with all bricks (including bumpers and paddle)
			for (size_t j = 0; j < bricks.size(); j++)
			{
				world[i].CheckCollision(&bricks[j]);
			}
			world[i].MoveOneStep();

			// If circle falls past the bottom of the screen (off-screen / missed by paddle)
			if (world[i].y + world[i].radius < -1.0f)
			{
				world.erase(world.begin() + i);
			}
			else
			{
				world[i].DrawCircle();
				i++;
			}
		}

		// Draw all active bricks and paddle
		for (size_t j = 0; j < bricks.size(); j++)
		{
			bricks[j].drawBrick();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}