#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace
{
constexpr int screenWidth = 1280;
constexpr int screenHeight = 720;
constexpr float playerSide = 1.0F;
constexpr float playerHalfHeight = playerSide * 0.5F;
constexpr float moveSpeed = 5.0F;
constexpr float cameraDistance = 7.0F;
constexpr float cameraHeight = 3.4F;
constexpr float cityChunkSize = 42.0F;
constexpr int cityChunkRadius = 1;

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

struct BoundsXZ
{
    float minX = 0.0F;
    float maxX = 0.0F;
    float minZ = 0.0F;
    float maxZ = 0.0F;
};

struct BuildingInstance
{
    Model* model = nullptr;
    Vector3 position{};
    float scale = 1.0F;
    float rotation = 0.0F;
    BoundsXZ collision{};
};

struct CityChunk
{
    int gridX = 0;
    int gridZ = 0;
    std::vector<BuildingInstance> buildings;
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
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), button.bounds))
    {
        return true;
    }

    const int touchCount = GetTouchPointCount();
    for (int touchIndex = 0; touchIndex < touchCount; ++touchIndex)
    {
        if (CheckCollisionPointRec(GetTouchPosition(touchIndex), button.bounds))
        {
            return true;
        }
    }
    return false;
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
    DrawText("MOVE / TOUCH", screenWidth - 204, screenHeight - 184, 16, {178, 201, 227, 255});

    DrawControlButton(up, IsControlHeld(up) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP));
    DrawControlButton(down, IsControlHeld(down) || IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN));
    DrawControlButton(left, IsControlHeld(left) || IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
    DrawControlButton(right, IsControlHeld(right) || IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT));
}

std::string FindCityAsset(const char* fileName)
{
    const std::array<std::string, 3> roots = {
        "android/app/src/main/assets/city/",
        "city/",
        "assets/city/",
    };

    for (const std::string& root : roots)
    {
        const std::string candidate = root + fileName;
        if (FileExists(candidate.c_str()))
        {
            return candidate;
        }
    }

    // Android's asset manager exposes the app assets from the "city" folder.
    return std::string("city/") + fileName;
}

bool IsUsableModel(const Model& model)
{
    return model.meshCount > 0 && model.materialCount > 0;
}

struct CityAssets
{
    Model largeBuilding{};
    Model mediumBuilding{};
    Model smallBuilding{};
    Model street{};
    Model intersection{};
    Model sidewalk{};
    bool ready = false;

    bool Load()
    {
        struct AssetToLoad
        {
            Model* model;
            const char* fileName;
            const char* label;
        };

        const std::array<AssetToLoad, 6> assets = {{
            {&largeBuilding, "Building_Large_2.gltf", "large building"},
            {&mediumBuilding, "Building_Medium_2_001.gltf", "medium building"},
            {&smallBuilding, "Building_Small_1.gltf", "small building"},
            {&street, "Street_2Lane.gltf", "street"},
            {&intersection, "Street_4WayIntersection.gltf", "intersection"},
            {&sidewalk, "Sidewalk_Straight_3m.gltf", "sidewalk"},
        }};

        ready = true;
        for (const AssetToLoad& asset : assets)
        {
            const std::string path = FindCityAsset(asset.fileName);
            *asset.model = LoadModel(path.c_str());
            if (!IsUsableModel(*asset.model))
            {
                TraceLog(LOG_WARNING, "City asset failed to load: %s (%s)", asset.label, path.c_str());
                ready = false;
            }
            else
            {
                TraceLog(LOG_INFO, "City asset loaded: %s", path.c_str());
            }
        }
        return ready;
    }

    void Unload()
    {
        Model* models[] = {
            &largeBuilding,
            &mediumBuilding,
            &smallBuilding,
            &street,
            &intersection,
            &sidewalk,
        };

        for (Model* model : models)
        {
            if (IsUsableModel(*model))
            {
                UnloadModel(*model);
            }
        }
    }
};

BoundsXZ TransformBounds(
    BoundsXZ localBounds,
    Vector3 position,
    float scale,
    float rotationDegrees)
{
    const float radians = rotationDegrees * DEG2RAD;
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    BoundsXZ result{
        position.x + 100000.0F,
        position.x - 100000.0F,
        position.z + 100000.0F,
        position.z - 100000.0F,
    };

    const float xs[] = {localBounds.minX, localBounds.maxX};
    const float zs[] = {localBounds.minZ, localBounds.maxZ};
    for (float x : xs)
    {
        for (float z : zs)
        {
            const float rotatedX = (x * cosine - z * sine) * scale + position.x;
            const float rotatedZ = (x * sine + z * cosine) * scale + position.z;
            result.minX = std::min(result.minX, rotatedX);
            result.maxX = std::max(result.maxX, rotatedX);
            result.minZ = std::min(result.minZ, rotatedZ);
            result.maxZ = std::max(result.maxZ, rotatedZ);
        }
    }
    return result;
}

class CityWorld
{
public:
    explicit CityWorld(CityAssets& assets)
        : assets_(assets)
    {
    }

    void Update(Vector3 playerPosition)
    {
        const int nextGridX = static_cast<int>(std::floor(playerPosition.x / cityChunkSize));
        const int nextGridZ = static_cast<int>(std::floor(playerPosition.z / cityChunkSize));
        if (nextGridX == centerGridX_ && nextGridZ == centerGridZ_ && !chunks_.empty())
        {
            return;
        }

        centerGridX_ = nextGridX;
        centerGridZ_ = nextGridZ;
        chunks_.clear();

        // Keep a compact 3x3 neighborhood around the player. The asset models
        // are shared, while these lightweight chunk instances are rebuilt as
        // the player crosses a 42m boundary.
        for (int z = -cityChunkRadius; z <= cityChunkRadius; ++z)
        {
            for (int x = -cityChunkRadius; x <= cityChunkRadius; ++x)
            {
                LoadChunk(centerGridX_ + x, centerGridZ_ + z);
            }
        }
    }

    bool Collides(Vector3 candidate, float radius) const
    {
        for (const CityChunk& chunk : chunks_)
        {
            for (const BuildingInstance& building : chunk.buildings)
            {
                if (candidate.x + radius > building.collision.minX &&
                    candidate.x - radius < building.collision.maxX &&
                    candidate.z + radius > building.collision.minZ &&
                    candidate.z - radius < building.collision.maxZ)
                {
                    return true;
                }
            }
        }
        return false;
    }

    void Draw() const
    {
        DrawPlane({0.0F, -0.02F, 0.0F}, {cityChunkSize * 3.0F, cityChunkSize * 3.0F}, {37, 47, 57, 255});

        for (const CityChunk& chunk : chunks_)
        {
            const Vector3 center{
                static_cast<float>(chunk.gridX) * cityChunkSize,
                0.15F,
                static_cast<float>(chunk.gridZ) * cityChunkSize,
            };

            DrawModelEx(assets_.intersection, center, {0.0F, 1.0F, 0.0F}, 0.0F, {1.0F, 1.0F, 1.0F}, WHITE);
            DrawModelEx(assets_.street, {center.x, 0.15F, center.z - 15.0F}, {0.0F, 1.0F, 0.0F}, 0.0F, {1.0F, 1.0F, 1.0F}, WHITE);
            DrawModelEx(assets_.street, {center.x, 0.15F, center.z + 15.0F}, {0.0F, 1.0F, 0.0F}, 0.0F, {1.0F, 1.0F, 1.0F}, WHITE);
            DrawModelEx(assets_.street, {center.x - 15.0F, 0.15F, center.z}, {0.0F, 1.0F, 0.0F}, 90.0F, {1.0F, 1.0F, 1.0F}, WHITE);
            DrawModelEx(assets_.street, {center.x + 15.0F, 0.15F, center.z}, {0.0F, 1.0F, 0.0F}, 90.0F, {1.0F, 1.0F, 1.0F}, WHITE);

            DrawModelEx(assets_.sidewalk, {center.x - 5.0F, 0.15F, center.z - 5.0F}, {0.0F, 1.0F, 0.0F}, 0.0F, {1.0F, 1.0F, 1.0F}, WHITE);
            DrawModelEx(assets_.sidewalk, {center.x + 5.0F, 0.15F, center.z + 5.0F}, {0.0F, 1.0F, 0.0F}, 90.0F, {1.0F, 1.0F, 1.0F}, WHITE);

            for (const BuildingInstance& building : chunk.buildings)
            {
                DrawModelEx(
                    *building.model,
                    building.position,
                    {0.0F, 1.0F, 0.0F},
                    building.rotation,
                    {building.scale, building.scale, building.scale},
                    WHITE);
            }
        }
    }

    int ActiveChunkCount() const
    {
        return static_cast<int>(chunks_.size());
    }

private:
    void LoadChunk(int gridX, int gridZ)
    {
        CityChunk chunk;
        chunk.gridX = gridX;
        chunk.gridZ = gridZ;
        const Vector3 origin{
            static_cast<float>(gridX) * cityChunkSize,
            0.0F,
            static_cast<float>(gridZ) * cityChunkSize,
        };

        AddBuilding(
            chunk,
            assets_.largeBuilding,
            {-9.33F, 11.33F, -16.33F, 0.33F},
            {origin.x - 14.0F, 0.0F, origin.z - 4.0F},
            0.65F,
            0.0F);
        AddBuilding(
            chunk,
            assets_.mediumBuilding,
            {-7.53F, 7.53F, -12.49F, 0.57F},
            {origin.x + 10.0F, 0.0F, origin.z - 4.0F},
            0.70F,
            0.0F);
        AddBuilding(
            chunk,
            assets_.largeBuilding,
            {-9.33F, 11.33F, -16.33F, 0.33F},
            {origin.x - 12.0F, 0.0F, origin.z + 4.0F},
            0.65F,
            180.0F);
        AddBuilding(
            chunk,
            assets_.smallBuilding,
            {-7.23F, 5.23F, -12.23F, 0.23F},
            {origin.x + 10.0F, 0.0F, origin.z + 15.0F},
            0.75F,
            0.0F);

        chunks_.push_back(std::move(chunk));
    }

    static void AddBuilding(
        CityChunk& chunk,
        Model& model,
        BoundsXZ modelBounds,
        Vector3 position,
        float scale,
        float rotation)
    {
        chunk.buildings.push_back({
            &model,
            position,
            scale,
            rotation,
            TransformBounds(modelBounds, position, scale, rotation),
        });
    }

    CityAssets& assets_;
    int centerGridX_ = 999999;
    int centerGridZ_ = 999999;
    std::vector<CityChunk> chunks_;
};

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
    InitWindow(screenWidth, screenHeight, "Downtown City - C++ raylib");
    SetTargetFPS(60);

    CityAssets cityAssets;
    const bool cityReady = cityAssets.Load();
    CityWorld city(cityAssets);

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
            const Vector3 candidate{
                player.position.x + direction.x * moveSpeed * deltaTime,
                player.position.y,
                player.position.z + direction.z * moveSpeed * deltaTime,
            };

            if (!cityReady || !city.Collides(candidate, playerSide * 0.46F))
            {
                player.position = candidate;
            }
            player.forward = direction;
        }

        if (cityReady)
        {
            city.Update(player.position);
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
        if (cityReady)
        {
            city.Draw();
        }
        else
        {
            DrawTestDistrict();
        }

        // Temporary player placeholder. Animation is intentionally postponed
        // until the playable character asset is added.
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

        DrawRectangle(0, 0, screenWidth, 92, {7, 13, 24, 225});
        DrawText("DOWNTOWN CITY", 28, 14, 24, RAYWHITE);
        DrawText(
            cityReady ? "Native C++ / raylib  |  City assets loaded" : "Native C++ / raylib  |  City assets unavailable",
            30,
            46,
            16,
            cityReady ? Color{148, 221, 183, 255} : Color{255, 181, 137, 255});
        DrawText(
            TextFormat("Position  X %.1f   Z %.1f", player.position.x, player.position.z),
            30,
            72,
            16,
            {193, 216, 236, 255});
        if (cityReady)
        {
            DrawText(
                TextFormat("Chunks %d / 9", city.ActiveChunkCount()),
                326,
                72,
                16,
                {193, 216, 236, 255});
        }
        DrawText("ESC to quit", screenWidth - 116, 26, 16, {165, 188, 214, 255});

        DrawControls(up, down, left, right);
        EndDrawing();
    }

    cityAssets.Unload();
    CloseWindow();
    return 0;
}