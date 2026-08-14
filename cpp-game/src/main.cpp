#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace
{
constexpr int screenWidth = 1280;
constexpr int screenHeight = 720;
constexpr float playerSide = 1.0F;
constexpr float playerHalfHeight = playerSide * 0.5F;
constexpr float moveSpeed = 5.0F;
constexpr float cameraDistance = 7.0F;
constexpr float cameraHeight = 3.4F;

struct MovementInput
{
    float x = 0.0F;
    float z = 0.0F;
};

struct Player
{
    Vector3 position{0.0F, playerHalfHeight, 0.0F};
    Vector3 forward{0.0F, 0.0F, 1.0F};
};

struct ControlButton
{
    Rectangle bounds{};
    const char* label = "";
};

float LengthSquared(Vector3 value)
{
    return value.x * value.x + value.y * value.y + value.z * value.z;
}

Vector3 NormalizeXZ(Vector3 value)
{
    const float length = std::sqrt(value.x * value.x + value.z * value.z);
    if (length <= 0.0001F)
    {
        return {0.0F, 0.0F, 1.0F};
    }
    return {value.x / length, 0.0F, value.z / length};
}

bool IsControlHeld(const ControlButton& button)
{
    return IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
           CheckCollisionPointRec(GetMousePosition(), button.bounds);
}

bool IsControlPressed(const ControlButton& button)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
           CheckCollisionPointRec(GetMousePosition(), button.bounds);
}

void AddAxisFromKey(float& axis, KeyboardKey negative, KeyboardKey positive)
{
    if (IsKeyDown(negative))
    {
        axis -= 1.0F;
    }
    if (IsKeyDown(positive))
    {
        axis += 1.0F;
    }
}

MovementInput ReadMovementInput(
    const ControlButton& up,
    const ControlButton& down,
    const ControlButton& left,
    const ControlButton& right)
{
    MovementInput input;
    AddAxisFromKey(input.x, KEY_A, KEY_D);
    AddAxisFromKey(input.x, KEY_LEFT, KEY_RIGHT);
    AddAxisFromKey(input.z, KEY_S, KEY_W);
    AddAxisFromKey(input.z, KEY_DOWN, KEY_UP);

    if (IsControlHeld(left))
    {
        input.x -= 1.0F;
    }
    if (IsControlHeld(right))
    {
        input.x += 1.0F;
    }
    if (IsControlHeld(down))
    {
        input.z -= 1.0F;
    }
    if (IsControlHeld(up))
    {
        input.z += 1.0F;
    }

    const float length = std::sqrt(input.x * input.x + input.z * input.z);
    if (length > 1.0F)
    {
        input.x /= length;
        input.z /= length;
    }
    return input;
}

void DrawControlButton(const ControlButton& button, bool active)
{
    const Color fill = active ? Color{58, 126, 226, 255} : Color{28, 39, 59, 235};
    const Color border = active ? Color{161, 210, 255, 255} : Color{99, 121, 153, 255};

    DrawRectangleRounded(button.bounds, 0.22F, 8, fill);
    DrawRectangleRoundedLinesEx(button.bounds, 0.22F, 8, 2.0F, border);

    const int textWidth = MeasureText(button.label, 24);
    DrawText(
        button.label,
        static_cast<int>(button.bounds.x + (button.bounds.width - textWidth) * 0.5F),
        static_cast<int>(button.bounds.y + (button.bounds.height - 24) * 0.5F),
        24,
        RAYWHITE);
}

void DrawControls(
    const ControlButton& up,
    const ControlButton& down,
    const ControlButton& left,
    const ControlButton& right)
{
    DrawRectangleRounded({screenWidth - 236.0F, screenHeight - 196.0F, 212.0F, 178.0F}, 0.08F, 8, {8, 14, 25, 205});
    DrawText("MOVE", screenWidth - 204, screenHeight - 184, 16, {178, 201, 227, 255});

    DrawControlButton(up, IsControlHeld(up) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP));
    DrawControlButton(down, IsControlHeld(down) || IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN));
    DrawControlButton(left, IsControlHeld(left) || IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
    DrawControlButton(right, IsControlHeld(right) || IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT));
}

void DrawTestDistrict()
{
    DrawPlane({0.0F, 0.0F, 0.0F}, {80.0F, 80.0F}, {38, 46, 59, 255});
    DrawGrid(40, 2.0F);

    const Color buildingColors[] = {
        {84, 102, 128, 255},
        {121, 91, 77, 255},
        {71, 112, 102, 255},
        {107, 99, 134, 255},
    };
    const Vector3 buildings[] = {
        {-7.0F, 2.0F, 8.0F},
        {7.0F, 3.0F, 13.0F},
        {-12.0F, 1.5F, -11.0F},
        {11.0F, 2.5F, -8.0F},
        {19.0F, 4.0F, 4.0F},
        {-20.0F, 3.0F, 1.0F},
    };
    const Vector3 buildingSizes[] = {
        {7.0F, 4.0F, 5.0F},
        {5.0F, 6.0F, 6.0F},
        {6.0F, 3.0F, 7.0F},
        {6.0F, 5.0F, 5.0F},
        {7.0F, 8.0F, 7.0F},
        {8.0F, 6.0F, 5.0F},
    };

    for (int i = 0; i < 6; ++i)
    {
        const Vector3 center = buildings[i];
        const Vector3 size = buildingSizes[i];
        DrawCube(center, size.x, size.y, size.z, buildingColors[i % 4]);
        DrawCubeWires(center, size.x, size.y, size.z, {184, 197, 214, 170});
    }
}

} // namespace

int main()
{
    InitWindow(screenWidth, screenHeight, "Third-Person Cube - C++ Prototype");
    SetTargetFPS(60);

    Player player;
    Camera3D camera{};
    camera.position = {0.0F, cameraHeight, -cameraDistance};
    camera.target = {0.0F, 1.0F, 0.0F};
    camera.up = {0.0F, 1.0F, 0.0F};
    camera.fovy = 60.0F;
    camera.projection = CAMERA_PERSPECTIVE;

    const ControlButton up{{screenWidth - 152.0F, screenHeight - 148.0F, 56.0F, 56.0F}, "W"};
    const ControlButton down{{screenWidth - 152.0F, screenHeight - 76.0F, 56.0F, 56.0F}, "S"};
    const ControlButton left{{screenWidth - 224.0F, screenHeight - 76.0F, 56.0F, 56.0F}, "A"};
    const ControlButton right{{screenWidth - 80.0F, screenHeight - 76.0F, 56.0F, 56.0F}, "D"};

    while (!WindowShouldClose())
    {
        const float deltaTime = std::min(GetFrameTime(), 0.05F);
        const MovementInput input = ReadMovementInput(up, down, left, right);
        const Vector3 movement{input.x, 0.0F, input.z};

        if (LengthSquared(movement) > 0.0001F)
        {
            const Vector3 direction = NormalizeXZ(movement);
            player.position.x += direction.x * moveSpeed * deltaTime;
            player.position.z += direction.z * moveSpeed * deltaTime;
            player.forward = direction;
        }

        const Vector3 desiredCameraPosition = {
            player.position.x - player.forward.x * cameraDistance,
            player.position.y + cameraHeight,
            player.position.z - player.forward.z * cameraDistance,
        };
        camera.position = Vector3Lerp(camera.position, desiredCameraPosition, 1.0F - std::pow(0.001F, deltaTime));
        camera.target = Vector3Lerp(
            camera.target,
            {player.position.x, player.position.y + 0.65F, player.position.z},
            1.0F - std::pow(0.001F, deltaTime));

        BeginDrawing();
        ClearBackground({18, 27, 43, 255});

        BeginMode3D(camera);
        DrawTestDistrict();

        DrawCube(player.position, playerSide, playerSide, playerSide, {61, 166, 255, 255});
        DrawCubeWires(player.position, playerSide, playerSide, playerSide, RAYWHITE);
        const Vector3 frontMarker = {
            player.position.x + player.forward.x * 0.57F,
            player.position.y + 0.12F,
            player.position.z + player.forward.z * 0.57F,
        };
        DrawCube(frontMarker, 0.16F, 0.16F, 0.16F, {255, 211, 92, 255});
        DrawLine3D(
            player.position,
            {player.position.x + player.forward.x * 1.5F, player.position.y, player.position.z + player.forward.z * 1.5F},
            {255, 211, 92, 255});

        EndMode3D();

        DrawRectangle(0, 0, screenWidth, 76, {7, 13, 24, 220});
        DrawText("THIRD-PERSON CUBE", 28, 16, 24, RAYWHITE);
        DrawText("C++ prototype  |  WASD / Arrows / on-screen buttons", 30, 46, 16, {165, 188, 214, 255});
        DrawText(
            TextFormat("Position  X %.1f   Z %.1f", player.position.x, player.position.z),
            30,
            92,
            16,
            {193, 216, 236, 255});
        DrawText("ESC to quit", screenWidth - 116, 26, 16, {165, 188, 214, 255});

        DrawControls(up, down, left, right);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}