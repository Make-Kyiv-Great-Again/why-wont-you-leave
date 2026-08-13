#pragma once

class Entity {
public:
    virtual ~Entity() = default;
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
};
